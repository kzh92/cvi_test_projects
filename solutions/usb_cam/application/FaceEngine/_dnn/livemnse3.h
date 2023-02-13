
#ifndef _LIVEMNSE3_H__INCLUDED_
#define _LIVEMNSE3_H__INCLUDED_

#include "enn_global.h"

using namespace ENN;

class LiveMnSE3
{
private:
	float* Conv_1; // [108]
	float* Conv_1_bs; // [12]
	float* Conv_4; // [108]
	float* Conv_4_bs; // [12]
	float* Conv_6; // [288]
	float* Conv_6_bs; // [24]
	float* Conv_9; // [216]
	float* Conv_9_bs; // [24]
	float* Conv_11; // [288]
	float* Conv_11_bs; // [12]
	float* Conv_12; // [288]
	float* Conv_12_bs; // [24]
	float* Conv_15; // [216]
	float* Conv_15_bs; // [24]
	float* Conv_17; // [288]
	float* Conv_17_bs; // [12]
	float* Conv_19; // [288]
	float* Conv_19_bs; // [24]
	float* Conv_22; // [216]
	float* Conv_22_bs; // [24]
	float* Conv_24; // [288]
	float* Conv_24_bs; // [12]
	float* Conv_26; // [576]
	float* Conv_26_bs; // [48]
	float* Conv_29; // [432]
	float* Conv_29_bs; // [48]
	float* Conv_31; // [1152]
	float* Conv_31_bs; // [24]
	float* Conv_32; // [1152]
	float* Conv_32_bs; // [48]
	float* Conv_35; // [432]
	float* Conv_35_bs; // [48]
	float* Conv_37; // [1152]
	float* Conv_37_bs; // [24]
	float* Conv_39; // [1152]
	float* Conv_39_bs; // [48]
	float* Conv_42; // [432]
	float* Conv_42_bs; // [48]
	float* Conv_44; // [1152]
	float* Conv_44_bs; // [24]
	float* Conv_47; // [144]
	float* Conv_47_bs; // [6]
	float* Conv_49; // [144]
	float* Conv_49_bs; // [24]
	float* Conv_53; // [2304]
	float* Conv_53_bs; // [96]
	float* Conv_56; // [864]
	float* Conv_56_bs; // [96]
	float* Conv_58; // [4608]
	float* Conv_58_bs; // [48]
	float* Conv_59; // [4608]
	float* Conv_59_bs; // [96]
	float* Conv_62; // [864]
	float* Conv_62_bs; // [96]
	float* Conv_64; // [4608]
	float* Conv_64_bs; // [48]
	float* Conv_66; // [4608]
	float* Conv_66_bs; // [96]
	float* Conv_69; // [864]
	float* Conv_69_bs; // [96]
	float* Conv_71; // [4608]
	float* Conv_71_bs; // [48]
	float* Conv_74; // [576]
	float* Conv_74_bs; // [12]
	float* Conv_76; // [576]
	float* Conv_76_bs; // [48]
	float* Conv_80; // [9216]
	float* Conv_80_bs; // [192]
	float* Conv_82; // [9216]
	float* Conv_82_bs; // [192]
	float* Gemm_84; // [1920]
	float* Gemm_84_bs; // [10]
	float* Gemm_86; // [20]
	float* Gemm_86_bs; // [2]

	float  Gemm_86_bs_mem[2]; // [2]

	unsigned char* dic_data;
	unsigned char* mem_data;

	enn_blob* mem_blk0;
	enn_blob* mem_blk1;
	enn_blob* mem_blk2;
	enn_blob* mem_blk3;

	int g_nEngineLoaded;

public:
	LiveMnSE3();
	~LiveMnSE3();

    static int dnn_dic_size();
    static int dnn_mem_size();
	int dnn_create(const char* fn, float rConfig = 75, unsigned char* pMemBuf = 0);
	int dnn_create(unsigned char* pDicData, int nDicDataSize, float rConfig = 75, unsigned char* pMemBuf = 0);
	void dnn_free();
	float* dnn_forward(unsigned char* img);

	int getEngineLoaded();
};

#endif //_LIVEMNSE3_H__INCLUDED_
