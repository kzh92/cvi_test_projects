
#ifndef _HANDFEAT_H__INCLUDED_
#define _HANDFEAT_H__INCLUDED_

#include "enn_global.h"

#ifdef __cplusplus
extern	"C"
{
#endif

typedef struct HandFeat_tag
{
	unsigned short* Conv_1; // [576]
	unsigned short* Conv_1_bs; // [64]
	unsigned short* PRelu_2; // [64]
	unsigned short* Conv_4; // [576]
	unsigned short* Conv_4_bs; // [64]
	unsigned short* PRelu_5; // [64]
	unsigned short* Conv_6; // [8192]
	unsigned short* Conv_6_bs; // [128]
	unsigned short* PRelu_7; // [128]
	unsigned short* Conv_9; // [1152]
	unsigned short* Conv_9_bs; // [128]
	unsigned short* PRelu_10; // [128]
	unsigned short* Conv_11; // [8192]
	unsigned short* Conv_11_bs; // [64]
	unsigned short* Conv_12; // [8192]
	unsigned short* Conv_12_bs; // [128]
	unsigned short* PRelu_13; // [128]
	unsigned short* Conv_15; // [1152]
	unsigned short* Conv_15_bs; // [128]
	unsigned short* PRelu_16; // [128]
	unsigned short* Conv_17; // [8192]
	unsigned short* Conv_17_bs; // [64]
	unsigned short* Conv_19; // [8192]
	unsigned short* Conv_19_bs; // [128]
	unsigned short* PRelu_20; // [128]
	unsigned short* Conv_22; // [1152]
	unsigned short* Conv_22_bs; // [128]
	unsigned short* PRelu_23; // [128]
	unsigned short* Conv_24; // [8192]
	unsigned short* Conv_24_bs; // [64]
	unsigned short* Conv_26; // [8192]
	unsigned short* Conv_26_bs; // [128]
	unsigned short* PRelu_27; // [128]
	unsigned short* Conv_29; // [1152]
	unsigned short* Conv_29_bs; // [128]
	unsigned short* PRelu_30; // [128]
	unsigned short* Conv_31; // [8192]
	unsigned short* Conv_31_bs; // [64]
	unsigned short* Conv_33; // [8192]
	unsigned short* Conv_33_bs; // [128]
	unsigned short* PRelu_34; // [128]
	unsigned short* Conv_36; // [1152]
	unsigned short* Conv_36_bs; // [128]
	unsigned short* PRelu_37; // [128]
	unsigned short* Conv_38; // [8192]
	unsigned short* Conv_38_bs; // [64]
	unsigned short* Conv_40; // [16384]
	unsigned short* Conv_40_bs; // [256]
	unsigned short* PRelu_41; // [256]
	unsigned short* Conv_43; // [2304]
	unsigned short* Conv_43_bs; // [256]
	unsigned short* PRelu_44; // [256]
	unsigned short* Conv_45; // [32768]
	unsigned short* Conv_45_bs; // [128]
	unsigned short* Conv_48; // [4096]
	unsigned short* Conv_48_bs; // [32]
	unsigned short* Conv_50; // [4096]
	unsigned short* Conv_50_bs; // [128]
	unsigned short* Conv_53; // [32768]
	unsigned short* Conv_53_bs; // [256]
	unsigned short* PRelu_54; // [256]
	unsigned short* Conv_56; // [2304]
	unsigned short* Conv_56_bs; // [256]
	unsigned short* PRelu_57; // [256]
	unsigned short* Conv_58; // [32768]
	unsigned short* Conv_58_bs; // [128]
	unsigned short* Conv_61; // [4096]
	unsigned short* Conv_61_bs; // [32]
	unsigned short* Conv_63; // [4096]
	unsigned short* Conv_63_bs; // [128]
	unsigned short* Conv_67; // [32768]
	unsigned short* Conv_67_bs; // [256]
	unsigned short* PRelu_68; // [256]
	unsigned short* Conv_70; // [2304]
	unsigned short* Conv_70_bs; // [256]
	unsigned short* PRelu_71; // [256]
	unsigned short* Conv_72; // [32768]
	unsigned short* Conv_72_bs; // [128]
	unsigned short* Conv_75; // [4096]
	unsigned short* Conv_75_bs; // [32]
	unsigned short* Conv_77; // [4096]
	unsigned short* Conv_77_bs; // [128]
	unsigned short* Conv_81; // [32768]
	unsigned short* Conv_81_bs; // [256]
	unsigned short* PRelu_82; // [256]
	unsigned short* Conv_84; // [2304]
	unsigned short* Conv_84_bs; // [256]
	unsigned short* PRelu_85; // [256]
	unsigned short* Conv_86; // [32768]
	unsigned short* Conv_86_bs; // [128]
	unsigned short* Conv_89; // [4096]
	unsigned short* Conv_89_bs; // [32]
	unsigned short* Conv_91; // [4096]
	unsigned short* Conv_91_bs; // [128]
	unsigned short* Conv_95; // [65536]
	unsigned short* Conv_95_bs; // [512]
	unsigned short* PRelu_96; // [512]
	unsigned short* Conv_98; // [4608]
	unsigned short* Conv_98_bs; // [512]
	unsigned short* PRelu_99; // [512]
	unsigned short* Conv_100; // [65536]
	unsigned short* Conv_100_bs; // [128]
	unsigned short* Conv_101; // [65536]
	unsigned short* Conv_101_bs; // [512]
	unsigned short* PRelu_102; // [512]
	unsigned short* Conv_103; // [32768]
	unsigned short* Conv_103_bs; // [512]
	unsigned short* MatMul_105; // [262144]
	unsigned short* MatMul_105_bs; // [512]

	unsigned char* dic_data;
	unsigned char* mem_data;

	enn_blob* mem_blk0;
	enn_blob* mem_blk1;
	enn_blob* mem_blk2;
	enn_blob* mem_blk3;

	int g_nEngineLoaded;
} HandFeat;

void HandFeat_HandFeat(HandFeat* ts);
void HandFeat_HandFeat_(HandFeat* ts);

int HandFeat_dnn_dic_size();
int HandFeat_dnn_mem_size();
int HandFeat_dnn_create(HandFeat* ts, const char* fn, float rConfig, unsigned char* pMemBuf); // float rConfig = 75, unsigned char* pMemBuf = 0
int HandFeat_dnn_create_(HandFeat* ts, unsigned char* pDicData, int nDicDataSize, float rConfig, unsigned char* pMemBuf); // float rConfig = 75, unsigned char* pMemBuf = 0
void HandFeat_dnn_free(HandFeat* ts);
float* HandFeat_dnn_forward(HandFeat* ts, unsigned char* img);
int HandFeat_getEngineLoaded(HandFeat* ts);

#ifdef __cplusplus
}
#endif

#endif //_HANDFEAT_H__INCLUDED_
