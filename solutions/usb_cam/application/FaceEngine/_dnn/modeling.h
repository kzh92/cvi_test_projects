
#ifndef _Modeling_H__INCLUDED_
#define _Modeling_H__INCLUDED_

#include "enn_global.h"

using namespace ENN;

class Modeling
{
private:
	float* Conv_0; // [288]
	float* Conv_0_bs; // [32]
	float* Conv_2; // [288]
	float* Conv_2_bs; // [32]
	float* Conv_4; // [2048]
	float* Conv_4_bs; // [64]
	float* Conv_6; // [576]
	float* Conv_6_bs; // [64]
	float* Conv_8; // [2048]
	float* Conv_8_bs; // [32]
	float* Conv_9; // [2048]
	float* Conv_9_bs; // [64]
	float* Conv_11; // [576]
	float* Conv_11_bs; // [64]
	float* Conv_13; // [2048]
	float* Conv_13_bs; // [32]
	float* Conv_15; // [2048]
	float* Conv_15_bs; // [64]
	float* Conv_17; // [576]
	float* Conv_17_bs; // [64]
	float* Conv_19; // [2048]
	float* Conv_19_bs; // [32]
	float* Conv_21; // [2048]
	float* Conv_21_bs; // [64]
	float* Conv_23; // [576]
	float* Conv_23_bs; // [64]
	float* Conv_25; // [2048]
	float* Conv_25_bs; // [32]
	float* Conv_27; // [4096]
	float* Conv_27_bs; // [128]
	float* Conv_29; // [1152]
	float* Conv_29_bs; // [128]
	float* Conv_31; // [8192]
	float* Conv_31_bs; // [64]
	float* Conv_32; // [8192]
	float* Conv_32_bs; // [128]
	float* Conv_34; // [1152]
	float* Conv_34_bs; // [128]
	float* Conv_36; // [8192]
	float* Conv_36_bs; // [64]
	float* Conv_38; // [8192]
	float* Conv_38_bs; // [128]
	float* Conv_40; // [1152]
	float* Conv_40_bs; // [128]
	float* Conv_42; // [8192]
	float* Conv_42_bs; // [64]
	float* Conv_44; // [8192]
	float* Conv_44_bs; // [128]
	float* Conv_46; // [1152]
	float* Conv_46_bs; // [128]
	float* Conv_48; // [8192]
	float* Conv_48_bs; // [64]
	float* Conv_50; // [8192]
	float* Conv_50_bs; // [128]
	float* Conv_52; // [1152]
	float* Conv_52_bs; // [128]
	float* Conv_54; // [8192]
	float* Conv_54_bs; // [64]
	float* Conv_56; // [16384]
	float* Conv_56_bs; // [256]
	float* Conv_58; // [2304]
	float* Conv_58_bs; // [256]
	float* Conv_60; // [16384]
	float* Conv_60_bs; // [64]
	float* Conv_61; // [8192]
	float* Conv_61_bs; // [128]
	float* Conv_63; // [1152]
	float* Conv_63_bs; // [128]
	float* Conv_65; // [8192]
	float* Conv_65_bs; // [64]
	float* Conv_67; // [8192]
	float* Conv_67_bs; // [128]
	float* Conv_69; // [1152]
	float* Conv_69_bs; // [128]
	float* Conv_71; // [8192]
	float* Conv_71_bs; // [64]
	float* Conv_73; // [32768]
	float* Conv_73_bs; // [512]
	float* Conv_75; // [8192]
	float* Conv_75_bs; // [512]
	float* Gemm_77; // [69632]
	float* Gemm_77_bs; // [136]

	unsigned char* dic_data;
	unsigned char* mem_data;

	enn_blob* mem_blk0;
	enn_blob* mem_blk1;
	enn_blob* mem_blk2;
	enn_blob* mem_blk3;

	int g_nEngineLoaded;

public:
    Modeling();
    ~Modeling();

    static int dnn_dic_size();
    static int dnn_mem_size();
	int dnn_create(const char* fn, unsigned char* pMemBuf = 0);
	int dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf = 0);
	void dnn_free();
	float* dnn_forward(unsigned char* img, int width, int height);
    int getEngineLoaded();
};

#endif //_Modeling_H__INCLUDED_
