
// VERSION LiveMn_CODE_1.00

#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <vector>
#include <memory.h>
#include <math.h>
#include <stdio.h>

#include "livemn.h"

#include "enn_trans.h"
#include "enn_pad.h"
#include "enn_conv.h"
#include "enn_activation.h"
#include "enn_permute.h"
#include "enn_softmax.h"
#include "enn_eltwise.h"
#include "enn_pool.h"
#include "enn_inner.h"
#include "enn_reshape.h"

extern int g_nStopEngine;
#define IF_FLAG_STOP if (g_nStopEngine) return 0

LiveMn::LiveMn()
{
	dic_data = 0;
	mem_data = 0;
	mem_blk0 = 0;
	mem_blk1 = 0;
	mem_blk2 = 0;
	g_nEngineLoaded = 0;
}

LiveMn::~LiveMn()
{
	dnn_free();
}

int LiveMn::dnn_dic_size()
{
	return 767960;
}

int LiveMn::dnn_mem_size()
{
	return 196624 + 393232 + 418192 + 110992;
}

int LiveMn::dnn_create(const char* fn, float rConfig, unsigned char* pMemBuf)
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

	int ret = dnn_create(local_dicdata, nDicSize, rConfig, pMemBuf);
	if (ret)
	{
		aligned_free(local_dicdata);
		return 1;
	}

	dic_data = local_dicdata;
	g_nEngineLoaded = 1;

	return 0;
}

int LiveMn::dnn_create(unsigned char* pDicData, int nDicDataSize, float rConfig, unsigned char* pMemBuf)
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

	mem_blk0 = (enn_blob*)mem_all; mem_all += 196624;
	mem_blk1 = (enn_blob*)mem_all; mem_all += 393232;
	mem_blk2 = (enn_blob*)mem_all; mem_all += 418192;
	mem_blk3 = (enn_blob*)mem_all;

	Conv_1 = (float*)(gdata + 0); // [108]
	Conv_1_bs = (float*)(gdata + 432); // [12]
	Conv_4 = (float*)(gdata + 480); // [108]
	Conv_4_bs = (float*)(gdata + 912); // [12]
	Conv_6 = (float*)(gdata + 960); // [288]
	Conv_6_bs = (float*)(gdata + 2112); // [24]
	Conv_9 = (float*)(gdata + 2208); // [216]
	Conv_9_bs = (float*)(gdata + 3072); // [24]
	Conv_11 = (float*)(gdata + 3168); // [288]
	Conv_11_bs = (float*)(gdata + 4320); // [12]
	Conv_12 = (float*)(gdata + 4368); // [288]
	Conv_12_bs = (float*)(gdata + 5520); // [24]
	Conv_15 = (float*)(gdata + 5616); // [216]
	Conv_15_bs = (float*)(gdata + 6480); // [24]
	Conv_17 = (float*)(gdata + 6576); // [288]
	Conv_17_bs = (float*)(gdata + 7728); // [12]
	Conv_19 = (float*)(gdata + 7776); // [288]
	Conv_19_bs = (float*)(gdata + 8928); // [24]
	Conv_22 = (float*)(gdata + 9024); // [216]
	Conv_22_bs = (float*)(gdata + 9888); // [24]
	Conv_24 = (float*)(gdata + 9984); // [288]
	Conv_24_bs = (float*)(gdata + 11136); // [12]
	Conv_26 = (float*)(gdata + 11184); // [576]
	Conv_26_bs = (float*)(gdata + 13488); // [48]
	Conv_29 = (float*)(gdata + 13680); // [432]
	Conv_29_bs = (float*)(gdata + 15408); // [48]
	Conv_31 = (float*)(gdata + 15600); // [1152]
	Conv_31_bs = (float*)(gdata + 20208); // [24]
	Conv_32 = (float*)(gdata + 20304); // [1152]
	Conv_32_bs = (float*)(gdata + 24912); // [48]
	Conv_35 = (float*)(gdata + 25104); // [432]
	Conv_35_bs = (float*)(gdata + 26832); // [48]
	Conv_37 = (float*)(gdata + 27024); // [1152]
	Conv_37_bs = (float*)(gdata + 31632); // [24]
	Conv_39 = (float*)(gdata + 31728); // [1152]
	Conv_39_bs = (float*)(gdata + 36336); // [48]
	Conv_42 = (float*)(gdata + 36528); // [432]
	Conv_42_bs = (float*)(gdata + 38256); // [48]
	Conv_44 = (float*)(gdata + 38448); // [1152]
	Conv_44_bs = (float*)(gdata + 43056); // [24]
	Conv_46 = (float*)(gdata + 43152); // [2304]
	Conv_46_bs = (float*)(gdata + 52368); // [96]
	Conv_49 = (float*)(gdata + 52752); // [864]
	Conv_49_bs = (float*)(gdata + 56208); // [96]
	Conv_51 = (float*)(gdata + 56592); // [4608]
	Conv_51_bs = (float*)(gdata + 75024); // [48]
	Conv_52 = (float*)(gdata + 75216); // [4608]
	Conv_52_bs = (float*)(gdata + 93648); // [96]
	Conv_55 = (float*)(gdata + 94032); // [864]
	Conv_55_bs = (float*)(gdata + 97488); // [96]
	Conv_57 = (float*)(gdata + 97872); // [4608]
	Conv_57_bs = (float*)(gdata + 116304); // [48]
	Conv_59 = (float*)(gdata + 116496); // [4608]
	Conv_59_bs = (float*)(gdata + 134928); // [96]
	Conv_62 = (float*)(gdata + 135312); // [864]
	Conv_62_bs = (float*)(gdata + 138768); // [96]
	Conv_64 = (float*)(gdata + 139152); // [4608]
	Conv_64_bs = (float*)(gdata + 157584); // [48]
	Conv_66 = (float*)(gdata + 157776); // [9216]
	Conv_66_bs = (float*)(gdata + 194640); // [192]
	Conv_69 = (float*)(gdata + 195408); // [1728]
	Conv_69_bs = (float*)(gdata + 202320); // [192]
	Conv_71 = (float*)(gdata + 203088); // [18432]
	Conv_71_bs = (float*)(gdata + 276816); // [96]
	Conv_72 = (float*)(gdata + 277200); // [18432]
	Conv_72_bs = (float*)(gdata + 350928); // [192]
	Conv_75 = (float*)(gdata + 351696); // [1728]
	Conv_75_bs = (float*)(gdata + 358608); // [192]
	Conv_77 = (float*)(gdata + 359376); // [18432]
	Conv_77_bs = (float*)(gdata + 433104); // [96]
	Conv_79 = (float*)(gdata + 433488); // [18432]
	Conv_79_bs = (float*)(gdata + 507216); // [192]
	Conv_82 = (float*)(gdata + 507984); // [1728]
	Conv_82_bs = (float*)(gdata + 514896); // [192]
	Conv_84 = (float*)(gdata + 515664); // [18432]
	Conv_84_bs = (float*)(gdata + 589392); // [96]
	Conv_86 = (float*)(gdata + 589776); // [36864]
	Conv_86_bs = (float*)(gdata + 737232); // [384]
	Conv_88 = (float*)(gdata + 738768); // [6144]
	Conv_88_bs = (float*)(gdata + 763344); // [384]
	Gemm_95 = (float*)(gdata + 764880); // [768]
	Gemm_95_bs = (float*)(gdata + 767952); // [2]
	Gemm_95_bs_mem[0] = Gemm_95_bs[0];
	Gemm_95_bs_mem[1] = Gemm_95_bs[1];
	Gemm_95_bs = Gemm_95_bs_mem;
	Gemm_95_bs[1] += (rConfig - 75);

	g_nEngineLoaded = 2;

	return 0;
}

void LiveMn::dnn_free()
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

int LiveMn::getEngineLoaded()
{
    return g_nEngineLoaded;
}

// void save_bin(enn_blob* blob);
// void save_bin(float* blob, int sz);

float* LiveMn::dnn_forward(unsigned char* in)
{
	IF_FLAG_STOP; trans(in, 128, 128, 1, mem_blk0, 1, 0, 0.003921568627451f);
	// save_bin(mem_blk0); // 0 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 1 
	IF_FLAG_STOP; conv3x3s2_pack1to4_neon(mem_blk2, mem_blk1, 12, 3, 2, Conv_1, Conv_1_bs);
	// save_bin(mem_blk1); // 2 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 3 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 4 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk2, mem_blk0, 12, 3, 1, Conv_4, Conv_4_bs);
	// save_bin(mem_blk0); // 5 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 6 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 24, 1, 1, Conv_6, Conv_6_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 7 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 8 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 9 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk0, 24, 3, 2, Conv_9, Conv_9_bs);
	// save_bin(mem_blk0); // 10 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 11 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 12, 1, 1, Conv_11, Conv_11_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 12 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 24, 1, 1, Conv_12, Conv_12_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 13 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 14 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 15 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 24, 3, 1, Conv_15, Conv_15_bs);
	// save_bin(mem_blk2); // 16 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 17 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 12, 1, 1, Conv_17, Conv_17_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 18 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 19 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 24, 1, 1, Conv_19, Conv_19_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 20 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 21 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 22 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 24, 3, 1, Conv_22, Conv_22_bs);
	// save_bin(mem_blk1); // 23 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 24 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 12, 1, 1, Conv_24, Conv_24_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 25 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 26 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 48, 1, 1, Conv_26, Conv_26_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 27 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 28 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 29 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk1, 48, 3, 2, Conv_29, Conv_29_bs);
	// save_bin(mem_blk1); // 30 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 31 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 24, 1, 1, Conv_31, Conv_31_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 32 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 48, 1, 1, Conv_32, Conv_32_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 33 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 34 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 35 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 48, 3, 1, Conv_35, Conv_35_bs);
	// save_bin(mem_blk2); // 36 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 37 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk1, 24, 1, 1, Conv_37, Conv_37_bs, (float*)mem_blk3);
	// save_bin(mem_blk1); // 38 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 39 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 48, 1, 1, Conv_39, Conv_39_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 40 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 41 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 42 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 48, 3, 1, Conv_42, Conv_42_bs);
	// save_bin(mem_blk1); // 43 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 44 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 24, 1, 1, Conv_44, Conv_44_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 45 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 46 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 96, 1, 1, Conv_46, Conv_46_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 47 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 48 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 49 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk1, 96, 3, 2, Conv_49, Conv_49_bs);
	// save_bin(mem_blk1); // 50 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 51 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 48, 1, 1, Conv_51, Conv_51_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 52 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 96, 1, 1, Conv_52, Conv_52_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 53 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 54 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 55 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 96, 3, 1, Conv_55, Conv_55_bs);
	// save_bin(mem_blk2); // 56 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 57 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk1, 48, 1, 1, Conv_57, Conv_57_bs, (float*)mem_blk3);
	// save_bin(mem_blk1); // 58 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 59 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 96, 1, 1, Conv_59, Conv_59_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 60 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 61 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 62 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 96, 3, 1, Conv_62, Conv_62_bs);
	// save_bin(mem_blk1); // 63 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 64 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 48, 1, 1, Conv_64, Conv_64_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 65 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 66 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 192, 1, 1, Conv_66, Conv_66_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 67 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 68 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 69 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk1, 192, 3, 2, Conv_69, Conv_69_bs);
	// save_bin(mem_blk1); // 70 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 71 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 96, 1, 1, Conv_71, Conv_71_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 72 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 192, 1, 1, Conv_72, Conv_72_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 73 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 74 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 75 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 192, 3, 1, Conv_75, Conv_75_bs);
	// save_bin(mem_blk2); // 76 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 77 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk1, 96, 1, 1, Conv_77, Conv_77_bs, (float*)mem_blk3);
	// save_bin(mem_blk1); // 78 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 79 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 192, 1, 1, Conv_79, Conv_79_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 80 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 81 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 82 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 192, 3, 1, Conv_82, Conv_82_bs);
	// save_bin(mem_blk1); // 83 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 84 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 96, 1, 1, Conv_84, Conv_84_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 85 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 86 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 384, 1, 1, Conv_86, Conv_86_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 87 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 88 
	IF_FLAG_STOP; convdw_gdc_pack4_neon(mem_blk0, mem_blk1, 384, 4, 1, Conv_88, Conv_88_bs);
	// save_bin(mem_blk1); // 89 
	IF_FLAG_STOP; flatten(mem_blk1);
	// save_bin(mem_blk1); // 90 
	IF_FLAG_STOP; innerproduct(mem_blk1, mem_blk0, 2, Gemm_95, Gemm_95_bs);
	// save_bin(mem_blk0); // 91 
	return (float*)mem_blk0->mem;
}
