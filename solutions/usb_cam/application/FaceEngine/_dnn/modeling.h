
#ifndef _Modeling_H__INCLUDED_
#define _Modeling_H__INCLUDED_

#include "enn_global.h"

#ifdef __cplusplus
extern	"C"
{
#endif

typedef struct Modeling_tag
{
    unsigned short* Conv_0; // [288]
    unsigned short* Conv_0_bs; // [32]
    unsigned short* Conv_2; // [288]
    unsigned short* Conv_2_bs; // [32]
    unsigned short* Conv_4; // [2048]
    unsigned short* Conv_4_bs; // [64]
    unsigned short* Conv_6; // [576]
    unsigned short* Conv_6_bs; // [64]
    unsigned short* Conv_8; // [2048]
    unsigned short* Conv_8_bs; // [32]
    unsigned short* Conv_9; // [2048]
    unsigned short* Conv_9_bs; // [64]
    unsigned short* Conv_11; // [576]
    unsigned short* Conv_11_bs; // [64]
    unsigned short* Conv_13; // [2048]
    unsigned short* Conv_13_bs; // [32]
    unsigned short* Conv_15; // [2048]
    unsigned short* Conv_15_bs; // [64]
    unsigned short* Conv_17; // [576]
    unsigned short* Conv_17_bs; // [64]
    unsigned short* Conv_19; // [2048]
    unsigned short* Conv_19_bs; // [32]
    unsigned short* Conv_21; // [2048]
    unsigned short* Conv_21_bs; // [64]
    unsigned short* Conv_23; // [576]
    unsigned short* Conv_23_bs; // [64]
    unsigned short* Conv_25; // [2048]
    unsigned short* Conv_25_bs; // [32]
    unsigned short* Conv_27; // [4096]
    unsigned short* Conv_27_bs; // [128]
    unsigned short* Conv_29; // [1152]
    unsigned short* Conv_29_bs; // [128]
    unsigned short* Conv_31; // [8192]
    unsigned short* Conv_31_bs; // [64]
    unsigned short* Conv_32; // [8192]
    unsigned short* Conv_32_bs; // [128]
    unsigned short* Conv_34; // [1152]
    unsigned short* Conv_34_bs; // [128]
    unsigned short* Conv_36; // [8192]
    unsigned short* Conv_36_bs; // [64]
    unsigned short* Conv_38; // [8192]
    unsigned short* Conv_38_bs; // [128]
    unsigned short* Conv_40; // [1152]
    unsigned short* Conv_40_bs; // [128]
    unsigned short* Conv_42; // [8192]
    unsigned short* Conv_42_bs; // [64]
    unsigned short* Conv_44; // [8192]
    unsigned short* Conv_44_bs; // [128]
    unsigned short* Conv_46; // [1152]
    unsigned short* Conv_46_bs; // [128]
    unsigned short* Conv_48; // [8192]
    unsigned short* Conv_48_bs; // [64]
    unsigned short* Conv_50; // [8192]
    unsigned short* Conv_50_bs; // [128]
    unsigned short* Conv_52; // [1152]
    unsigned short* Conv_52_bs; // [128]
    unsigned short* Conv_54; // [8192]
    unsigned short* Conv_54_bs; // [64]
    unsigned short* Conv_56; // [16384]
    unsigned short* Conv_56_bs; // [256]
    unsigned short* Conv_58; // [2304]
    unsigned short* Conv_58_bs; // [256]
    unsigned short* Conv_60; // [16384]
    unsigned short* Conv_60_bs; // [64]
    unsigned short* Conv_61; // [8192]
    unsigned short* Conv_61_bs; // [128]
    unsigned short* Conv_63; // [1152]
    unsigned short* Conv_63_bs; // [128]
    unsigned short* Conv_65; // [8192]
    unsigned short* Conv_65_bs; // [64]
    unsigned short* Conv_67; // [8192]
    unsigned short* Conv_67_bs; // [128]
    unsigned short* Conv_69; // [1152]
    unsigned short* Conv_69_bs; // [128]
    unsigned short* Conv_71; // [8192]
    unsigned short* Conv_71_bs; // [64]
    unsigned short* Conv_73; // [32768]
    unsigned short* Conv_73_bs; // [512]
    unsigned short* Conv_75; // [8192]
    unsigned short* Conv_75_bs; // [512]
    unsigned short* Gemm_77; // [69632] // [7168]
    unsigned short* Gemm_77_bs; // [136] // [14]

    unsigned char* dic_data;
    unsigned char* mem_data;

    enn_blob* mem_blk0;
    enn_blob* mem_blk1;
    enn_blob* mem_blk2;
    enn_blob* mem_blk3;

	int m_isHandModeling;
    int g_nEngineLoaded;
} Modeling;

void Modeling_Modeling(Modeling* ts, int isHandModeling); // 0(FaceModeling), 1(HandModeling)
void Modeling_Modeling_(Modeling* ts);

int Modeling_dnn_dic_size(int isHandModeling); // 0(FaceModeling), 1(HandModeling)
int Modeling_dnn_mem_size();
int Modeling_dnn_create(Modeling* ts, const char* fn, unsigned char* pMemBuf); // unsigned char* pMemBuf = 0
int Modeling_dnn_create_(Modeling* ts, unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf); // unsigned char* pMemBuf = 0
void Modeling_dnn_free(Modeling* ts);
float* Modeling_dnn_forward(Modeling* ts, unsigned char* img, int width, int height);
int Modeling_getEngineLoaded(Modeling* ts);

#ifdef __cplusplus
}
#endif

#endif //_Modeling_H__INCLUDED_
