
#ifndef _LIVEMN_H__INCLUDED_
#define _LIVEMN_H__INCLUDED_

#include "enn_global.h"

using namespace ENN;

class LiveMn
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
	float* Conv_46; // [2304]
	float* Conv_46_bs; // [96]
	float* Conv_49; // [864]
	float* Conv_49_bs; // [96]
	float* Conv_51; // [4608]
	float* Conv_51_bs; // [48]
	float* Conv_52; // [4608]
	float* Conv_52_bs; // [96]
	float* Conv_55; // [864]
	float* Conv_55_bs; // [96]
	float* Conv_57; // [4608]
	float* Conv_57_bs; // [48]
	float* Conv_59; // [4608]
	float* Conv_59_bs; // [96]
	float* Conv_62; // [864]
	float* Conv_62_bs; // [96]
	float* Conv_64; // [4608]
	float* Conv_64_bs; // [48]
	float* Conv_66; // [9216]
	float* Conv_66_bs; // [192]
	float* Conv_69; // [1728]
	float* Conv_69_bs; // [192]
	float* Conv_71; // [18432]
	float* Conv_71_bs; // [96]
	float* Conv_72; // [18432]
	float* Conv_72_bs; // [192]
	float* Conv_75; // [1728]
	float* Conv_75_bs; // [192]
	float* Conv_77; // [18432]
	float* Conv_77_bs; // [96]
	float* Conv_79; // [18432]
	float* Conv_79_bs; // [192]
	float* Conv_82; // [1728]
	float* Conv_82_bs; // [192]
	float* Conv_84; // [18432]
	float* Conv_84_bs; // [96]
	float* Conv_86; // [36864]
	float* Conv_86_bs; // [384]
	float* Conv_88; // [6144]
	float* Conv_88_bs; // [384]
	float* Gemm_95; // [768]
	float* Gemm_95_bs; // [2]
	float  Gemm_95_bs_mem[2]; // [2]

	unsigned char* dic_data;
	unsigned char* mem_data;

	enn_blob* mem_blk0;
	enn_blob* mem_blk1;
	enn_blob* mem_blk2;
	enn_blob* mem_blk3;

	int g_nEngineLoaded;

public:
	LiveMn();
	~LiveMn();

    static int dnn_dic_size();
    static int dnn_mem_size();
	int dnn_create(const char* fn, float rConfig = 75, unsigned char* pMemBuf = 0);
	int dnn_create(unsigned char* pDicData, int nDicDataSize, float rConfig = 75, unsigned char* pMemBuf = 0);
	void dnn_free();
	float* dnn_forward(unsigned char* img);
    int getEngineLoaded();
};

#endif //_LIVEMN_H__INCLUDED_
