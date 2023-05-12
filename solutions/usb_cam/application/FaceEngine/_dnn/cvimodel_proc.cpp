#include <stdio.h>
#include <cviruntime.h>
#include <string.h>
#include <cvimodel_proc.h>


void cvimodel_init_private(Cvimodel* model)
{
    CVI_NN_GetInputOutputTensors(model->m_handle, &model->m_input_tensors, &model->m_input_num, &model->m_output_tensors, &model->m_output_num);
    model->m_input_ptr = (float*)CVI_NN_TensorPtr(model->m_input_tensors);
    model->m_output_ptr = (float*)CVI_NN_TensorPtr(model->m_output_tensors);

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

void cvimodel_forward(Cvimodel *model, unsigned char* img, int width, int height, int type, float** retVal, float** retVal1)
{
    int i, n = width * height;
    float* prInput = model->m_input_ptr;
    unsigned char* pnImage = img;

    if (type == 0) // detect
    {
        for (i = 0; i < n; i++)
        {
            *prInput = (*pnImage - 128) * 0.0078125f;
            prInput++;
            pnImage++;
        }
        memcpy(model->m_input_ptr + n, model->m_input_ptr, n * 4);
        memcpy(model->m_input_ptr + 2*n, model->m_input_ptr, n * 4);
    }

    if (type == 1) // feature
    {
        int8_t* pnInput = (int8_t*)prInput;
        for (i = 0; i < n; i++)
        {
            int val = (int)(*pnImage * 0.00390625f * model->m_qscale);
            if (val > 127) val = 127;
            if (val < -128) val = -128;
            *pnInput = val;
            pnInput++;
            pnImage++;
        }
        memcpy(((int8_t*)model->m_input_ptr) + n, model->m_input_ptr, n);
        memcpy(((int8_t*)model->m_input_ptr) + 2*n, model->m_input_ptr, n);
    }

    if (type == 2) // other
    {
        for (i = 0; i < n; i++)
        {
            *prInput = *pnImage * 0.00390625f;
            prInput++;
            pnImage++;
        }
    }

    CVI_NN_Forward(model->m_handle, model->m_input_tensors, model->m_input_num, model->m_output_tensors, model->m_output_num);

    *retVal = model->m_output_ptr;
    if (model->m_multiOutput) *retVal1 = model->m_output_ptr1;
}
