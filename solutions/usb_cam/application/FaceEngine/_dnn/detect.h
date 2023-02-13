
#ifndef _DETECT_MN_H__INCLUDED_
#define _DETECT_MN_H__INCLUDED_

#include "enn_global.h"

using namespace ENN;

class Detect
{
private:
	float* Conv_0; // [216]
	float* Conv_0_bs; // [8]
	float* Conv_2; // [72]
	float* Conv_2_bs; // [8]
	float* Conv_4; // [128]
	float* Conv_4_bs; // [16]
	float* Conv_6; // [144]
	float* Conv_6_bs; // [16]
	float* Conv_8; // [128]
	float* Conv_8_bs; // [8]
	float* Conv_9; // [128]
	float* Conv_9_bs; // [16]
	float* Conv_11; // [144]
	float* Conv_11_bs; // [16]
	float* Conv_13; // [128]
	float* Conv_13_bs; // [8]
	float* Conv_15; // [256]
	float* Conv_15_bs; // [32]
	float* Conv_17; // [288]
	float* Conv_17_bs; // [32]
	float* Conv_19; // [512]
	float* Conv_19_bs; // [16]
	float* Conv_20; // [512]
	float* Conv_20_bs; // [32]
	float* Conv_22; // [288]
	float* Conv_22_bs; // [32]
	float* Conv_24; // [512]
	float* Conv_24_bs; // [16]
	float* Conv_26; // [512]
	float* Conv_26_bs; // [32]
	float* Conv_28; // [288]
	float* Conv_28_bs; // [32]
	float* Conv_30; // [512]
	float* Conv_30_bs; // [16]
	float* Conv_32; // [512]
	float* Conv_32_bs; // [32]
	float* Conv_34; // [288]
	float* Conv_34_bs; // [32]
	float* Conv_36; // [512]
	float* Conv_36_bs; // [16]
	float* Conv_38; // [144]
	float* Conv_38_bs; // [16]
	float* Conv_40; // [192]
	float* Conv_40_bs; // [8]
	float* Conv_43; // [144]
	float* Conv_43_bs; // [16]
	float* Conv_45; // [192]
	float* Conv_45_bs; // [12]
	float* Conv_48; // [768]
	float* Conv_48_bs; // [48]
	float* Conv_50; // [432]
	float* Conv_50_bs; // [48]
	float* Conv_52; // [1536]
	float* Conv_52_bs; // [32]
	float* Conv_53; // [1536]
	float* Conv_53_bs; // [48]
	float* Conv_55; // [432]
	float* Conv_55_bs; // [48]
	float* Conv_57; // [1536]
	float* Conv_57_bs; // [32]
	float* Conv_59; // [1536]
	float* Conv_59_bs; // [48]
	float* Conv_61; // [432]
	float* Conv_61_bs; // [48]
	float* Conv_63; // [1536]
	float* Conv_63_bs; // [32]
	float* Conv_65; // [288]
	float* Conv_65_bs; // [32]
	float* Conv_67; // [128]
	float* Conv_67_bs; // [4]
	float* Conv_70; // [288]
	float* Conv_70_bs; // [32]
	float* Conv_72; // [256]
	float* Conv_72_bs; // [8]
	float* Conv_75; // [2048]
	float* Conv_75_bs; // [64]
	float* Conv_77; // [576]
	float* Conv_77_bs; // [64]
	float* Conv_79; // [3072]
	float* Conv_79_bs; // [48]
	float* Conv_80; // [3072]
	float* Conv_80_bs; // [64]
	float* Conv_82; // [576]
	float* Conv_82_bs; // [64]
	float* Conv_84; // [3072]
	float* Conv_84_bs; // [48]
	float* Conv_86; // [432]
	float* Conv_86_bs; // [48]
	float* Conv_88; // [576]
	float* Conv_88_bs; // [8]
	float* Conv_91; // [432]
	float* Conv_91_bs; // [48]
	float* Conv_93; // [576]
	float* Conv_93_bs; // [12]
	float* Conv_96; // [3840]
	float* Conv_96_bs; // [80]
	float* Conv_98; // [720]
	float* Conv_98_bs; // [80]
	float* Conv_100; // [5120]
	float* Conv_100_bs; // [64]
	float* Conv_101; // [576]
	float* Conv_101_bs; // [64]
	float* Conv_103; // [256]
	float* Conv_103_bs; // [4]
	float* Conv_106; // [576]
	float* Conv_106_bs; // [64]
	float* Conv_108; // [512]
	float* Conv_108_bs; // [8]

	unsigned char* dic_data;
	unsigned char* mem_data;

	enn_blob* mem_blk0;
	enn_blob* mem_blk1;
	enn_blob* mem_blk2;
	enn_blob* mem_blk3;
	enn_blob* mem_blk4;
	enn_blob* mem_blk5;
	enn_blob* mem_blk6;
	enn_blob* mem_blk7;
	enn_blob* mem_blk8;
	enn_blob* mem_blk9;

	int g_nEngineLoaded;

public:
	Detect();
	~Detect();

	static int dnn_dic_size();
	static int dnn_mem_size();
	int dnn_create(const char* fn, unsigned char* pMemBuf = 0);
	int dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf = 0);
	void dnn_free();
	int dnn_forward(unsigned char* img, int width, int height, float** pprScore, float** pprBox, bool isSimple = false);
    int getEngineLoaded();
};

#endif //_DETECT_MN_H__INCLUDED_
