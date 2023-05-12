#include <stdio.h>
#include <cviruntime.h>
#include <string.h>
#include <cvimodel_proc.h>


void cvimodel_init_private(Cvimodel* model)
{
    CVI_NN_GetInputOutputTensors(model->m_handle, &model->m_input_tensors, &model->m_input_num, &model->m_output_tensors, &model->m_output_num);
    model->m_input_ptr = (float*)CVI_NN_TensorPtr(model->m_input_tensors);
    model->m_output_ptr = (float*)CVI_NN_TensorPtr(model->m_output_tensors);
    model->m_input_is_int8 = (model->m_input_tensors->fmt == CVI_FMT_INT8);
    model->m_input_channels = model->m_input_tensors->shape.dim[1];

    model->m_multiOutput = (model->m_output_num > 1) ? 1 : 0;
    if (model->m_output_num > 1)
    {
        model->m_output_ptr = (float*)CVI_NN_TensorPtr(CVI_NN_GetTensorByName("boxes_Concat_dequant", model->m_output_tensors, model->m_output_num));
        model->m_output_ptr1 = (float*)CVI_NN_TensorPtr(CVI_NN_GetTensorByName("scores_Concat_dequant", model->m_output_tensors, model->m_output_num));
    }
}

int cvimodel_init(const char* fn, Cvimodel* model)
{
    if (model->m_loaded) return 0;

    int ret = CVI_NN_RegisterModel(fn, &model->m_handle);
    if (CVI_RC_SUCCESS != ret) {
        printf("CVI_NN_RegisterModel failed, err %d\n", ret);
        return 1;
    }
    cvimodel_init_private(model);

    model->m_loaded = 1;

    return 0;
}

int cvimodel_init(const unsigned char* pDicData, int nDicLen, Cvimodel* model)
{
    if (model->m_loaded) return 0;

    int ret = CVI_NN_RegisterModelFromBuffer((const int8_t *)pDicData, nDicLen, &model->m_handle);
    if (CVI_RC_SUCCESS != ret) {
        printf("CVI_NN_RegisterModel failed, err %d\n", ret);
        return 1;
    }
    cvimodel_init_private(model);

    model->m_loaded = 1;

    return 0;
}

void cvimodel_release(Cvimodel* model)
{
    if (model->m_loaded == 0) return;

    CVI_NN_CleanupModel(model->m_handle);

    model->m_loaded = 0;
}

void cvimodel_forward(Cvimodel *model, unsigned char* img, int width, int height, float** retVal, float** retVal1, int mean, float std) // if detect, retVal(box), retVal1(score)
{
    int i, n = width * height;
    unsigned char* pnImage = img;

    if (model->m_input_is_int8)
    {
        int8_t* pnInput = (int8_t*)model->m_input_ptr;
        for (i = 0; i < n; i++)
        {
            int val = (int)((*pnImage - mean) * std * model->m_qscale);
            if (val > 127) val = 127;
            if (val < -128) val = -128;
            *pnInput = val;
            pnInput++;
            pnImage++;
        }
    }
    else
    {
        float* prInput = model->m_input_ptr;
        for (i = 0; i < n; i++)
        {
            *prInput = (*pnImage - mean) * std;
            prInput++;
            pnImage++;
        }
    }

    if (model->m_input_channels > 1)
    {
        if (model->m_input_is_int8)
        {
            int8_t* pnInput = (int8_t*)model->m_input_ptr;
            memcpy(pnInput + n, pnInput, n);
            memcpy(pnInput + n * 2, pnInput, n);
        }
        else
        {
            float* prInput = model->m_input_ptr;
            memcpy(prInput + n, prInput, n * 4);
            memcpy(prInput + n * 2, prInput, n * 4);
        }
    }

    CVI_NN_Forward(model->m_handle, model->m_input_tensors, model->m_input_num, model->m_output_tensors, model->m_output_num);

    *retVal = model->m_output_ptr;
    if (model->m_multiOutput) *retVal1 = model->m_output_ptr1;
}
