
// VERSION Live_CODE_1.00

#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <vector>
#include <memory.h>
#include <math.h>
#include <stdio.h>

#include "livemnse3.h"

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

LiveMnSE3::LiveMnSE3()
{
	dic_data = 0;
	mem_data = 0;
	mem_blk0 = 0;
	mem_blk1 = 0;
	mem_blk2 = 0;
	g_nEngineLoaded = 0;
}

LiveMnSE3::~LiveMnSE3()
{
	dnn_free();
}

int LiveMnSE3::dnn_dic_size()
{
	return 246968;
}

int LiveMnSE3::dnn_mem_size()
{
	return 135184 + 270352 + 291472 + 78352;
}

int LiveMnSE3::dnn_create(const char* fn, float rConfig, unsigned char* pMemBuf)
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

int LiveMnSE3::dnn_create(unsigned char* pDicData, int nDicDataSize, float rConfig, unsigned char* pMemBuf)
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

	mem_blk0 = (enn_blob*)mem_all; mem_all += 135184;
	mem_blk1 = (enn_blob*)mem_all; mem_all += 270352;
	mem_blk2 = (enn_blob*)mem_all; mem_all += 291472;
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
	Conv_47 = (float*)(gdata + 43152); // [144]
	Conv_47_bs = (float*)(gdata + 43728); // [6]
	Conv_49 = (float*)(gdata + 43752); // [144]
	Conv_49_bs = (float*)(gdata + 44328); // [24]
	Conv_53 = (float*)(gdata + 44424); // [2304]
	Conv_53_bs = (float*)(gdata + 53640); // [96]
	Conv_56 = (float*)(gdata + 54024); // [864]
	Conv_56_bs = (float*)(gdata + 57480); // [96]
	Conv_58 = (float*)(gdata + 57864); // [4608]
	Conv_58_bs = (float*)(gdata + 76296); // [48]
	Conv_59 = (float*)(gdata + 76488); // [4608]
	Conv_59_bs = (float*)(gdata + 94920); // [96]
	Conv_62 = (float*)(gdata + 95304); // [864]
	Conv_62_bs = (float*)(gdata + 98760); // [96]
	Conv_64 = (float*)(gdata + 99144); // [4608]
	Conv_64_bs = (float*)(gdata + 117576); // [48]
	Conv_66 = (float*)(gdata + 117768); // [4608]
	Conv_66_bs = (float*)(gdata + 136200); // [96]
	Conv_69 = (float*)(gdata + 136584); // [864]
	Conv_69_bs = (float*)(gdata + 140040); // [96]
	Conv_71 = (float*)(gdata + 140424); // [4608]
	Conv_71_bs = (float*)(gdata + 158856); // [48]
	Conv_74 = (float*)(gdata + 159048); // [576]
	Conv_74_bs = (float*)(gdata + 161352); // [12]
	Conv_76 = (float*)(gdata + 161400); // [576]
	Conv_76_bs = (float*)(gdata + 163704); // [48]
	Conv_80 = (float*)(gdata + 163896); // [9216]
	Conv_80_bs = (float*)(gdata + 200760); // [192]
	Conv_82 = (float*)(gdata + 201528); // [9216]
	Conv_82_bs = (float*)(gdata + 238392); // [192]
	Gemm_84 = (float*)(gdata + 239160); // [1920]
	Gemm_84_bs = (float*)(gdata + 246840); // [10]
	Gemm_86 = (float*)(gdata + 246880); // [20]
	Gemm_86_bs = (float*)(gdata + 246960); // [2]

	Gemm_86_bs_mem[0] = Gemm_86_bs[0];
	Gemm_86_bs_mem[1] = Gemm_86_bs[1];
	Gemm_86_bs = Gemm_86_bs_mem;
	Gemm_86_bs[1] += (rConfig - 75);

	g_nEngineLoaded = 2;

	return 0;
}

void LiveMnSE3::dnn_free()
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

int LiveMnSE3::getEngineLoaded()
{
    return g_nEngineLoaded;
}

void save_bin(enn_blob* blob);
void save_bin(float* blob, int sz);

float* LiveMnSE3::dnn_forward(unsigned char* in)
{
	IF_FLAG_STOP; trans(in, 88, 128, 1, mem_blk0, 1, 0, 0.003921568627451f);
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
	IF_FLAG_STOP; pooling_global_aver_pack4_neon(mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 46 
	IF_FLAG_STOP; flatten(mem_blk1);
	// save_bin(mem_blk1); // 47 
	IF_FLAG_STOP; innerproduct(mem_blk1, mem_blk3, 6, Conv_47, Conv_47_bs);
	// save_bin(mem_blk3); // 48 
	IF_FLAG_STOP; relu(mem_blk3);
	// save_bin(mem_blk3); // 49 
	IF_FLAG_STOP; innerproduct(mem_blk3, mem_blk1, 24, Conv_49, Conv_49_bs);
	// save_bin(mem_blk1); // 50 
	IF_FLAG_STOP; sigmoid(mem_blk1);
	// save_bin(mem_blk1); // 51 
	IF_FLAG_STOP; flatten(mem_blk1);
	// save_bin(mem_blk1); // 52 
	IF_FLAG_STOP; multiply(mem_blk0, mem_blk1, mem_blk3);
	// save_bin(mem_blk3); // 53 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk3, mem_blk0);
	// save_bin(mem_blk0); // 54 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 96, 1, 1, Conv_53, Conv_53_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 55 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 56 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 57 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk0, 96, 3, 2, Conv_56, Conv_56_bs);
	// save_bin(mem_blk0); // 58 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 59 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 48, 1, 1, Conv_58, Conv_58_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 60 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 96, 1, 1, Conv_59, Conv_59_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 61 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 62 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 63 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 96, 3, 1, Conv_62, Conv_62_bs);
	// save_bin(mem_blk2); // 64 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 65 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 48, 1, 1, Conv_64, Conv_64_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 66 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 67 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 96, 1, 1, Conv_66, Conv_66_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 68 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 69 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 70 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 96, 3, 1, Conv_69, Conv_69_bs);
	// save_bin(mem_blk1); // 71 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 72 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 48, 1, 1, Conv_71, Conv_71_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 73 
	IF_FLAG_STOP; pooling_global_aver_pack4_neon(mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 74 
	IF_FLAG_STOP; flatten(mem_blk1);
	// save_bin(mem_blk1); // 75 
	IF_FLAG_STOP; innerproduct(mem_blk1, mem_blk3, 12, Conv_74, Conv_74_bs);
	// save_bin(mem_blk3); // 76 
	IF_FLAG_STOP; relu(mem_blk3);
	// save_bin(mem_blk3); // 77 
	IF_FLAG_STOP; innerproduct(mem_blk3, mem_blk1, 48, Conv_76, Conv_76_bs);
	// save_bin(mem_blk1); // 78 
	IF_FLAG_STOP; sigmoid(mem_blk1);
	// save_bin(mem_blk1); // 79 
	IF_FLAG_STOP; flatten(mem_blk1);
	// save_bin(mem_blk1); // 80 
	IF_FLAG_STOP; multiply(mem_blk0, mem_blk1, mem_blk3);
	// save_bin(mem_blk3); // 81 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk3, mem_blk0);
	// save_bin(mem_blk0); // 82 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 192, 1, 1, Conv_80, Conv_80_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 83 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 84 
	IF_FLAG_STOP; convdw_gdc_pack4_neon(mem_blk1, mem_blk0, 192, 6, 8, 1, Conv_82, Conv_82_bs);
	// save_bin(mem_blk0); // 85 
	IF_FLAG_STOP; flatten(mem_blk0);
	// save_bin(mem_blk0); // 86 
	IF_FLAG_STOP; innerproduct(mem_blk0, mem_blk1, 10, Gemm_84, Gemm_84_bs);
	// save_bin(mem_blk1); // 87 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 88 
	IF_FLAG_STOP; innerproduct(mem_blk1, mem_blk0, 2, Gemm_86, Gemm_86_bs);
	// save_bin(mem_blk0); // 89 
	return (float*)mem_blk0->mem;
}
