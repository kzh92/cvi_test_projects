
#ifndef _CVIMODEL_PROC_MN_H__INCLUDED_
#define _CVIMODEL_PROC_MN_H__INCLUDED_
#include <cviruntime.h>

typedef struct Cvimodel_tag
{
    CVI_MODEL_HANDLE    m_handle;
    int                 m_input_num;
    int                 m_output_num;
    CVI_TENSOR*         m_input_tensors;
    CVI_TENSOR*         m_output_tensors;
    float*              m_input_ptr;
    float*              m_output_ptr;
    float*              m_output_ptr1;
    float               m_qscale;
    int                 m_multiOutput;
    int                 m_input_is_int8;
    int                 m_input_channels;
    int                 m_loaded;
} Cvimodel;


#define        DIC_LEN_FACE_DETECT      (129320)
#define        DIC_LEN_FACE_MODELING    (624696)
#define        DIC_LEN_FACE_LIVE_A1     (442504)
#define        DIC_LEN_FACE_LIVE_A2     (442504)
#define        DIC_LEN_FACE_LIVE_B      (442504)
#define        DIC_LEN_FACE_LIVE_B2     (152920)
#define        DIC_LEN_FACE_LIVE_C      (442504)
#define        DIC_LEN_FACE_FEATURE     (1543872)

int cvimodel_init(const char* fn, Cvimodel* model);
int cvimodel_init(const unsigned char* pDicData, int nDicLen, Cvimodel* model);
void cvimodel_release(Cvimodel* model);
void cvimodel_forward(Cvimodel *model, unsigned char* img, int width, int height, float** retVal, float** retVal1 = 0, int mean = 0, float std = 0.00390625);
// if detect, retVal(box), retVal1(score), mean = 128, std = 0.0078125

#endif // _CVIMODEL_PROC_MN_H__INCLUDED_

