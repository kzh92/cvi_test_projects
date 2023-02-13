
// VERSION MNLite

#include <algorithm>
#include <vector>
#include <memory.h>
#include <math.h>
#include <stdio.h>
#include "detect.h"

#include "enn_trans.h"
#include "enn_pad.h"
#include "enn_conv.h"
#include "enn_activation.h"
#include "enn_permute.h"
#include "enn_softmax.h"
#include "enn_eltwise.h"

extern int g_nStopEngine;
#define IF_FLAG_STOP if (g_nStopEngine) return 0

Detect::Detect()
{
	dic_data = 0;
	mem_data = 0;
	mem_blk0 = 0;
	mem_blk1 = 0;
	mem_blk2 = 0;
	mem_blk3 = 0;
	mem_blk4 = 0;
	mem_blk5 = 0;
	mem_blk6 = 0;
	mem_blk7 = 0;
	g_nEngineLoaded = 0;
}

Detect::~Detect()
{
	dnn_free();
}

int Detect::dnn_dic_size()
{
	return 180992;
}

int Detect::dnn_mem_size()
{
    //return 921616 + 1228816 + 1264912 + 325392 + 258064 + 76816 + 96272 + 30736 + 38416 + 11280; // (240 x 320)
    return 248848 + 331792 + 351248 + 92944 + 79888 + 24592 + 32784 + 17936 + 15376 + 5136; // (108 x 192)
}

int Detect::dnn_create(const char* fn, unsigned char* pMemBuf)
{
	if (g_nEngineLoaded) return 0;
	int nDicSize = dnn_dic_size();

	unsigned char* local_dicdata = (unsigned char*)aligned_malloc(nDicSize);
	if (!local_dicdata) return 1;

	FILE *f = fopen(fn, "rb");
	if (!f)
	{
		aligned_free(local_dicdata);
		return 1;
	}

	int nReadBytes = fread(local_dicdata, 1, nDicSize, f);
	fclose(f);

	if (nReadBytes < nDicSize)
	{
		aligned_free(local_dicdata);
		return 1;
	}

	int ret = dnn_create(local_dicdata, nDicSize, pMemBuf);
	if (ret)
	{
		aligned_free(local_dicdata);
		return 1;
	}

	dic_data = local_dicdata;
	g_nEngineLoaded = 1;

	return 0;
}

int Detect::dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf)
{
	if (g_nEngineLoaded) return 0;

	unsigned char* mem_all;
	if (pMemBuf)
	{
		mem_all = pMemBuf;
		mem_data = 0;
	}
	else
	{
		mem_all = (unsigned char*)aligned_malloc(dnn_mem_size());
		if (!mem_all) return 1;
		mem_data = mem_all;
	}
	dic_data = 0;

	unsigned char* gdata = pDicData;
//// 240 x 320
//	mem_blk0 = (enn_blob*)mem_all; mem_all += 921616;
//	mem_blk1 = (enn_blob*)mem_all; mem_all += 1228816;
//	mem_blk2 = (enn_blob*)mem_all; mem_all += 1264912;
//	mem_blk3 = (enn_blob*)mem_all; mem_all += 325392;
//	mem_blk4 = (enn_blob*)mem_all; mem_all += 258064;
//	mem_blk5 = (enn_blob*)mem_all; mem_all += 76816;
//	mem_blk6 = (enn_blob*)mem_all; mem_all += 96272;
//	mem_blk7 = (enn_blob*)mem_all; mem_all += 30736;
//	mem_blk8 = (enn_blob*)mem_all; mem_all += 38416;
//	mem_blk9 = (enn_blob*)mem_all;
//// 108 x 192
    mem_blk0 = (enn_blob*)mem_all; mem_all += 248848;
    mem_blk1 = (enn_blob*)mem_all; mem_all += 331792;
    mem_blk2 = (enn_blob*)mem_all; mem_all += 351248;
    mem_blk3 = (enn_blob*)mem_all; mem_all += 92944;
    mem_blk4 = (enn_blob*)mem_all; mem_all += 79888;
    mem_blk5 = (enn_blob*)mem_all; mem_all += 24592;
    mem_blk6 = (enn_blob*)mem_all; mem_all += 32784;
    mem_blk7 = (enn_blob*)mem_all; mem_all += 17936;
    mem_blk8 = (enn_blob*)mem_all; mem_all += 15376;
    mem_blk9 = (enn_blob*)mem_all;

	Conv_0 = (float*)(gdata + 0); // [216]
	Conv_0_bs = (float*)(gdata + 864); // [8]
	Conv_2 = (float*)(gdata + 896); // [72]
	Conv_2_bs = (float*)(gdata + 1184); // [8]
	Conv_4 = (float*)(gdata + 1216); // [128]
	Conv_4_bs = (float*)(gdata + 1728); // [16]
	Conv_6 = (float*)(gdata + 1792); // [144]
	Conv_6_bs = (float*)(gdata + 2368); // [16]
	Conv_8 = (float*)(gdata + 2432); // [128]
	Conv_8_bs = (float*)(gdata + 2944); // [8]
	Conv_9 = (float*)(gdata + 2976); // [128]
	Conv_9_bs = (float*)(gdata + 3488); // [16]
	Conv_11 = (float*)(gdata + 3552); // [144]
	Conv_11_bs = (float*)(gdata + 4128); // [16]
	Conv_13 = (float*)(gdata + 4192); // [128]
	Conv_13_bs = (float*)(gdata + 4704); // [8]
	Conv_15 = (float*)(gdata + 4736); // [256]
	Conv_15_bs = (float*)(gdata + 5760); // [32]
	Conv_17 = (float*)(gdata + 5888); // [288]
	Conv_17_bs = (float*)(gdata + 7040); // [32]
	Conv_19 = (float*)(gdata + 7168); // [512]
	Conv_19_bs = (float*)(gdata + 9216); // [16]
	Conv_20 = (float*)(gdata + 9280); // [512]
	Conv_20_bs = (float*)(gdata + 11328); // [32]
	Conv_22 = (float*)(gdata + 11456); // [288]
	Conv_22_bs = (float*)(gdata + 12608); // [32]
	Conv_24 = (float*)(gdata + 12736); // [512]
	Conv_24_bs = (float*)(gdata + 14784); // [16]
	Conv_26 = (float*)(gdata + 14848); // [512]
	Conv_26_bs = (float*)(gdata + 16896); // [32]
	Conv_28 = (float*)(gdata + 17024); // [288]
	Conv_28_bs = (float*)(gdata + 18176); // [32]
	Conv_30 = (float*)(gdata + 18304); // [512]
	Conv_30_bs = (float*)(gdata + 20352); // [16]
	Conv_32 = (float*)(gdata + 20416); // [512]
	Conv_32_bs = (float*)(gdata + 22464); // [32]
	Conv_34 = (float*)(gdata + 22592); // [288]
	Conv_34_bs = (float*)(gdata + 23744); // [32]
	Conv_36 = (float*)(gdata + 23872); // [512]
	Conv_36_bs = (float*)(gdata + 25920); // [16]
	Conv_38 = (float*)(gdata + 25984); // [144]
	Conv_38_bs = (float*)(gdata + 26560); // [16]
	Conv_40 = (float*)(gdata + 26624); // [192]
	Conv_40_bs = (float*)(gdata + 27392); // [8]
	Conv_43 = (float*)(gdata + 27424); // [144]
	Conv_43_bs = (float*)(gdata + 28000); // [16]
	Conv_45 = (float*)(gdata + 28064); // [192]
	Conv_45_bs = (float*)(gdata + 28832); // [12]
	Conv_48 = (float*)(gdata + 28880); // [768]
	Conv_48_bs = (float*)(gdata + 31952); // [48]
	Conv_50 = (float*)(gdata + 32144); // [432]
	Conv_50_bs = (float*)(gdata + 33872); // [48]
	Conv_52 = (float*)(gdata + 34064); // [1536]
	Conv_52_bs = (float*)(gdata + 40208); // [32]
	Conv_53 = (float*)(gdata + 40336); // [1536]
	Conv_53_bs = (float*)(gdata + 46480); // [48]
	Conv_55 = (float*)(gdata + 46672); // [432]
	Conv_55_bs = (float*)(gdata + 48400); // [48]
	Conv_57 = (float*)(gdata + 48592); // [1536]
	Conv_57_bs = (float*)(gdata + 54736); // [32]
	Conv_59 = (float*)(gdata + 54864); // [1536]
	Conv_59_bs = (float*)(gdata + 61008); // [48]
	Conv_61 = (float*)(gdata + 61200); // [432]
	Conv_61_bs = (float*)(gdata + 62928); // [48]
	Conv_63 = (float*)(gdata + 63120); // [1536]
	Conv_63_bs = (float*)(gdata + 69264); // [32]
	Conv_65 = (float*)(gdata + 69392); // [288]
	Conv_65_bs = (float*)(gdata + 70544); // [32]
	Conv_67 = (float*)(gdata + 70672); // [128]
	Conv_67_bs = (float*)(gdata + 71184); // [4]
	Conv_70 = (float*)(gdata + 71200); // [288]
	Conv_70_bs = (float*)(gdata + 72352); // [32]
	Conv_72 = (float*)(gdata + 72480); // [256]
	Conv_72_bs = (float*)(gdata + 73504); // [8]
	Conv_75 = (float*)(gdata + 73536); // [2048]
	Conv_75_bs = (float*)(gdata + 81728); // [64]
	Conv_77 = (float*)(gdata + 81984); // [576]
	Conv_77_bs = (float*)(gdata + 84288); // [64]
	Conv_79 = (float*)(gdata + 84544); // [3072]
	Conv_79_bs = (float*)(gdata + 96832); // [48]
	Conv_80 = (float*)(gdata + 97024); // [3072]
	Conv_80_bs = (float*)(gdata + 109312); // [64]
	Conv_82 = (float*)(gdata + 109568); // [576]
	Conv_82_bs = (float*)(gdata + 111872); // [64]
	Conv_84 = (float*)(gdata + 112128); // [3072]
	Conv_84_bs = (float*)(gdata + 124416); // [48]
	Conv_86 = (float*)(gdata + 124608); // [432]
	Conv_86_bs = (float*)(gdata + 126336); // [48]
	Conv_88 = (float*)(gdata + 126528); // [576]
	Conv_88_bs = (float*)(gdata + 128832); // [8]
	Conv_91 = (float*)(gdata + 128864); // [432]
	Conv_91_bs = (float*)(gdata + 130592); // [48]
	Conv_93 = (float*)(gdata + 130784); // [576]
	Conv_93_bs = (float*)(gdata + 133088); // [12]
	Conv_96 = (float*)(gdata + 133136); // [3840]
	Conv_96_bs = (float*)(gdata + 148496); // [80]
	Conv_98 = (float*)(gdata + 148816); // [720]
	Conv_98_bs = (float*)(gdata + 151696); // [80]
	Conv_100 = (float*)(gdata + 152016); // [5120]
	Conv_100_bs = (float*)(gdata + 172496); // [64]
	Conv_101 = (float*)(gdata + 172752); // [576]
	Conv_101_bs = (float*)(gdata + 175056); // [64]
	Conv_103 = (float*)(gdata + 175312); // [256]
	Conv_103_bs = (float*)(gdata + 176336); // [4]
	Conv_106 = (float*)(gdata + 176352); // [576]
	Conv_106_bs = (float*)(gdata + 178656); // [64]
	Conv_108 = (float*)(gdata + 178912); // [512]
	Conv_108_bs = (float*)(gdata + 180960); // [8]

	g_nEngineLoaded = 1;

	return 0;
}

void Detect::dnn_free()
{
	if (!g_nEngineLoaded) return;

	if (dic_data)
	{
		aligned_free(dic_data);
		dic_data = 0;
	}

	if (mem_data)
	{
		aligned_free(mem_data);
		mem_data = 0;
	}

	g_nEngineLoaded = 0;
}

int Detect::getEngineLoaded()
{
    return g_nEngineLoaded;
}

// void save_bin(enn_blob* blob);
// void save_bin(float* blob, int sz);

int Detect::dnn_forward(unsigned char* in, int width, int height, float** pprScore, float** pprBox, bool isSimple)
{
	IF_FLAG_STOP; trans(in, width, height, 1, mem_blk0, 3, -128, 0.0078125f);
	// save_bin(mem_blk0); // 0 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 1 
	IF_FLAG_STOP; conv3x3s2_pack1to4_neon(mem_blk2, mem_blk1, 8, 3, 2, Conv_0, Conv_0_bs);
	// save_bin(mem_blk1); // 2 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 3 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 4 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk2, mem_blk0, 8, 3, 1, Conv_2, Conv_2_bs);
	// save_bin(mem_blk0); // 5 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 6 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 16, 1, 1, Conv_4, Conv_4_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 7 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 8 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 9 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk0, 16, 3, 2, Conv_6, Conv_6_bs);
	// save_bin(mem_blk0); // 10 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 11 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 8, 1, 1, Conv_8, Conv_8_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 12 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 16, 1, 1, Conv_9, Conv_9_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 13 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 14 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 15 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 16, 3, 1, Conv_11, Conv_11_bs);
	// save_bin(mem_blk2); // 16 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 17 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 8, 1, 1, Conv_13, Conv_13_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 18 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 19 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 32, 1, 1, Conv_15, Conv_15_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 20 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 21 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 22 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk1, 32, 3, 2, Conv_17, Conv_17_bs);
	// save_bin(mem_blk1); // 23 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 24 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 16, 1, 1, Conv_19, Conv_19_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 25 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 32, 1, 1, Conv_20, Conv_20_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 26 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 27 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 28 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 32, 3, 1, Conv_22, Conv_22_bs);
	// save_bin(mem_blk2); // 29 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 30 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk1, 16, 1, 1, Conv_24, Conv_24_bs, (float*)mem_blk3);
	// save_bin(mem_blk1); // 31 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 32 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 32, 1, 1, Conv_26, Conv_26_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 33 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 34 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 35 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 32, 3, 1, Conv_28, Conv_28_bs);
	// save_bin(mem_blk1); // 36 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 37 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 16, 1, 1, Conv_30, Conv_30_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 38 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 39 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 32, 1, 1, Conv_32, Conv_32_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 40 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 41 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 42 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 32, 3, 1, Conv_34, Conv_34_bs);
	// save_bin(mem_blk2); // 43 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 44 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 16, 1, 1, Conv_36, Conv_36_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 45 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 46 
	IF_FLAG_STOP; padding(mem_blk2, mem_blk1, 1);
	// save_bin(mem_blk1); // 47 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk1, mem_blk0, 16, 3, 1, Conv_38, Conv_38_bs);
	// save_bin(mem_blk0); // 48 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 49 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4to1_neon(mem_blk0, mem_blk1, 6, 1, 1, Conv_40, Conv_40_bs, (float*)mem_blk3);
	// save_bin(mem_blk1); // 50 
	IF_FLAG_STOP;
	int p0 = permute(mem_blk1, (float*)mem_blk0);
	// save_bin((float*)mem_blk0, p0); // 51 
	IF_FLAG_STOP; padding(mem_blk2, mem_blk3, 1);
	// save_bin(mem_blk3); // 52 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 16, 3, 1, Conv_43, Conv_43_bs);
	// save_bin(mem_blk1); // 53 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 54 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk3, 12, 1, 1, Conv_45, Conv_45_bs, (float*)mem_blk4);
	// save_bin(mem_blk3); // 55 
	IF_FLAG_STOP;
	// int p1 = permute(mem_blk3, (float*)mem_blk1);
	// save_bin((float*)mem_blk1, p1); // 56 
	if (isSimple)
	{
		softmax((float*)mem_blk0, p0 / 2);
		*pprScore = (float*)mem_blk0;
		*pprBox = (float*)mem_blk1;
		return p0 / 2;
	}
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk3, 48, 1, 1, Conv_48, Conv_48_bs, (float*)mem_blk4);
	// save_bin(mem_blk3); // 57 
	IF_FLAG_STOP; relu(mem_blk3);
	// save_bin(mem_blk3); // 58 
	IF_FLAG_STOP; padding(mem_blk3, mem_blk4, 1);
	// save_bin(mem_blk4); // 59 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk4, mem_blk2, 48, 3, 2, Conv_50, Conv_50_bs);
	// save_bin(mem_blk2); // 60 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 61 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk3, 32, 1, 1, Conv_52, Conv_52_bs, (float*)mem_blk4);
	// save_bin(mem_blk3); // 62 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk3, mem_blk2, 48, 1, 1, Conv_53, Conv_53_bs, (float*)mem_blk4);
	// save_bin(mem_blk2); // 63 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 64 
	IF_FLAG_STOP; padding(mem_blk2, mem_blk5, 1);
	// save_bin(mem_blk5); // 65 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk5, mem_blk4, 48, 3, 1, Conv_55, Conv_55_bs);
	// save_bin(mem_blk4); // 66 
	IF_FLAG_STOP; relu(mem_blk4);
	// save_bin(mem_blk4); // 67 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk4, mem_blk2, 32, 1, 1, Conv_57, Conv_57_bs, (float*)mem_blk5);
	// save_bin(mem_blk2); // 68 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk3, mem_blk2, mem_blk4);
	// save_bin(mem_blk4); // 69 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk4, mem_blk2, 48, 1, 1, Conv_59, Conv_59_bs, (float*)mem_blk3);
	// save_bin(mem_blk2); // 70 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 71 
	IF_FLAG_STOP; padding(mem_blk2, mem_blk5, 1);
	// save_bin(mem_blk5); // 72 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk5, mem_blk3, 48, 3, 1, Conv_61, Conv_61_bs);
	// save_bin(mem_blk3); // 73 
	IF_FLAG_STOP; relu(mem_blk3);
	// save_bin(mem_blk3); // 74 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk3, mem_blk2, 32, 1, 1, Conv_63, Conv_63_bs, (float*)mem_blk5);
	// save_bin(mem_blk2); // 75 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk4, mem_blk2, mem_blk3);
	// save_bin(mem_blk3); // 76 
	IF_FLAG_STOP; padding(mem_blk3, mem_blk4, 1);
	// save_bin(mem_blk4); // 77 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk4, mem_blk2, 32, 3, 1, Conv_65, Conv_65_bs);
	// save_bin(mem_blk2); // 78 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 79 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk4, 4, 1, 1, Conv_67, Conv_67_bs, (float*)mem_blk5);
	// save_bin(mem_blk4); // 80 
	IF_FLAG_STOP;
	int p2 = permute(mem_blk4, ((float*)mem_blk0) + p0);
	// save_bin(((float*)mem_blk0) + p0, p2); // 81 
	IF_FLAG_STOP; padding(mem_blk3, mem_blk5, 1);
	// save_bin(mem_blk5); // 82 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk5, mem_blk4, 32, 3, 1, Conv_70, Conv_70_bs);
	// save_bin(mem_blk4); // 83 
	IF_FLAG_STOP; relu(mem_blk4);
	// save_bin(mem_blk4); // 84 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk4, mem_blk5, 8, 1, 1, Conv_72, Conv_72_bs, (float*)mem_blk6);
	// save_bin(mem_blk5); // 85 
	IF_FLAG_STOP;
	// int p3 = permute(mem_blk5, ((float*)mem_blk1) + p1);
	// save_bin(((float*)mem_blk1) + p1, p3); // 86 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk3, mem_blk5, 64, 1, 1, Conv_75, Conv_75_bs, (float*)mem_blk6);
	// save_bin(mem_blk5); // 87 
	IF_FLAG_STOP; relu(mem_blk5);
	// save_bin(mem_blk5); // 88 
	IF_FLAG_STOP; padding(mem_blk5, mem_blk6, 1);
	// save_bin(mem_blk6); // 89 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk6, mem_blk3, 64, 3, 2, Conv_77, Conv_77_bs);
	// save_bin(mem_blk3); // 90 
	IF_FLAG_STOP; relu(mem_blk3);
	// save_bin(mem_blk3); // 91 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk3, mem_blk5, 48, 1, 1, Conv_79, Conv_79_bs, (float*)mem_blk6);
	// save_bin(mem_blk5); // 92 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk5, mem_blk3, 64, 1, 1, Conv_80, Conv_80_bs, (float*)mem_blk6);
	// save_bin(mem_blk3); // 93 
	IF_FLAG_STOP; relu(mem_blk3);
	// save_bin(mem_blk3); // 94 
	IF_FLAG_STOP; padding(mem_blk3, mem_blk7, 1);
	// save_bin(mem_blk7); // 95 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk7, mem_blk6, 64, 3, 1, Conv_82, Conv_82_bs);
	// save_bin(mem_blk6); // 96 
	IF_FLAG_STOP; relu(mem_blk6);
	// save_bin(mem_blk6); // 97 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk6, mem_blk3, 48, 1, 1, Conv_84, Conv_84_bs, (float*)mem_blk7);
	// save_bin(mem_blk3); // 98 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk5, mem_blk3, mem_blk6);
	// save_bin(mem_blk6); // 99 
	IF_FLAG_STOP; padding(mem_blk6, mem_blk5, 1);
	// save_bin(mem_blk5); // 100 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk5, mem_blk3, 48, 3, 1, Conv_86, Conv_86_bs);
	// save_bin(mem_blk3); // 101 
	IF_FLAG_STOP; relu(mem_blk3);
	// save_bin(mem_blk3); // 102 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4to1_neon(mem_blk3, mem_blk5, 6, 1, 1, Conv_88, Conv_88_bs, (float*)mem_blk7);
	// save_bin(mem_blk5); // 103 
	IF_FLAG_STOP;
	int p4 = permute(mem_blk5, ((float*)mem_blk0) + p0 + p2);
	// save_bin(((float*)mem_blk0) + p0 + p2, p4); // 104 
	IF_FLAG_STOP; padding(mem_blk6, mem_blk7, 1);
	// save_bin(mem_blk7); // 105 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk7, mem_blk5, 48, 3, 1, Conv_91, Conv_91_bs);
	// save_bin(mem_blk5); // 106 
	IF_FLAG_STOP; relu(mem_blk5);
	// save_bin(mem_blk5); // 107 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk5, mem_blk7, 12, 1, 1, Conv_93, Conv_93_bs, (float*)mem_blk8);
	// save_bin(mem_blk7); // 108 
	IF_FLAG_STOP;
	// int p5 = permute(mem_blk7, ((float*)mem_blk1) + p1 + p3);
	// save_bin(((float*)mem_blk1) + p1 + p3, p5); // 109 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk6, mem_blk7, 80, 1, 1, Conv_96, Conv_96_bs, (float*)mem_blk8);
	// save_bin(mem_blk7); // 110 
	IF_FLAG_STOP; relu(mem_blk7);
	// save_bin(mem_blk7); // 111 
	IF_FLAG_STOP; padding(mem_blk7, mem_blk8, 1);
	// save_bin(mem_blk8); // 112 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk8, mem_blk6, 80, 3, 2, Conv_98, Conv_98_bs);
	// save_bin(mem_blk6); // 113 
	IF_FLAG_STOP; relu(mem_blk6);
	// save_bin(mem_blk6); // 114 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk6, mem_blk7, 64, 1, 1, Conv_100, Conv_100_bs, (float*)mem_blk8);
	// save_bin(mem_blk7); // 115 
	IF_FLAG_STOP; padding(mem_blk7, mem_blk8, 1);
	// save_bin(mem_blk8); // 116 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk8, mem_blk6, 64, 3, 1, Conv_101, Conv_101_bs);
	// save_bin(mem_blk6); // 117 
	IF_FLAG_STOP; relu(mem_blk6);
	// save_bin(mem_blk6); // 118 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk6, mem_blk8, 4, 1, 1, Conv_103, Conv_103_bs, (float*)mem_blk9);
	// save_bin(mem_blk8); // 119 
	IF_FLAG_STOP;
	int p6 = permute(mem_blk8, ((float*)mem_blk0) + p0 + p2 + p4);
	// save_bin(((float*)mem_blk0) + p0 + p2 + p4, p6); // 120 
	IF_FLAG_STOP; padding(mem_blk7, mem_blk9, 1);
	// save_bin(mem_blk9); // 121 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk9, mem_blk8, 64, 3, 1, Conv_106, Conv_106_bs);
	// save_bin(mem_blk8); // 122 
	IF_FLAG_STOP; relu(mem_blk8);
	// save_bin(mem_blk8); // 123 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk8, mem_blk7, 8, 1, 1, Conv_108, Conv_108_bs, (float*)mem_blk9);
	// save_bin(mem_blk7); // 124 
	IF_FLAG_STOP;
	// int p7 = permute(mem_blk7, ((float*)mem_blk1) + p1 + p3 + p5);
	// save_bin(((float*)mem_blk8) + p1 + p3 + p5, p7); // 125 
	// softmax((float*)mem_blk0, (p0 + p2 + p4 + p6) / 2);
	*pprScore = (float*)mem_blk0;
	*pprBox = (float*)mem_blk1;

	return (p0 + p2 + p4 + p6) / 2;
}
