
// VERSION vfl_CODE_1.10

#include <stdio.h>

#include "modeling.h"

#include "enn_conv.h"
#include "enn_inner.h"
#include "enn_pad.h"
#include "enn_trans.h"
#include "enn_activation.h"
#include "enn_pool.h"
#include "enn_reshape.h"
#include "enn_eltwise.h"

extern int g_nStopEngine;
#define IF_FLAG_STOP if (g_nStopEngine) return 0

Modeling::Modeling()
{
	dic_data = 0;
	mem_data = 0;
	g_nEngineLoaded = 0;
}

Modeling::~Modeling()
{
	dnn_free();
}

int Modeling::dnn_dic_size()
{
	return 1153056;
}

int Modeling::dnn_mem_size()
{
	return 131088 + 262160 + 295952 + 82960;
}

int Modeling::dnn_create(const char* fn, unsigned char* pMemBuf)
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

int Modeling::dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf)
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

	mem_blk0 = (enn_blob*)mem_all; mem_all += 131088;
	mem_blk1 = (enn_blob*)mem_all; mem_all += 262160;
	mem_blk2 = (enn_blob*)mem_all; mem_all += 295952;
	mem_blk3 = (enn_blob*)mem_all;

	Conv_0 = (float*)(gdata + 0); // [288]
	Conv_0_bs = (float*)(gdata + 1152); // [32]
	Conv_2 = (float*)(gdata + 1280); // [288]
	Conv_2_bs = (float*)(gdata + 2432); // [32]
	Conv_4 = (float*)(gdata + 2560); // [2048]
	Conv_4_bs = (float*)(gdata + 10752); // [64]
	Conv_6 = (float*)(gdata + 11008); // [576]
	Conv_6_bs = (float*)(gdata + 13312); // [64]
	Conv_8 = (float*)(gdata + 13568); // [2048]
	Conv_8_bs = (float*)(gdata + 21760); // [32]
	Conv_9 = (float*)(gdata + 21888); // [2048]
	Conv_9_bs = (float*)(gdata + 30080); // [64]
	Conv_11 = (float*)(gdata + 30336); // [576]
	Conv_11_bs = (float*)(gdata + 32640); // [64]
	Conv_13 = (float*)(gdata + 32896); // [2048]
	Conv_13_bs = (float*)(gdata + 41088); // [32]
	Conv_15 = (float*)(gdata + 41216); // [2048]
	Conv_15_bs = (float*)(gdata + 49408); // [64]
	Conv_17 = (float*)(gdata + 49664); // [576]
	Conv_17_bs = (float*)(gdata + 51968); // [64]
	Conv_19 = (float*)(gdata + 52224); // [2048]
	Conv_19_bs = (float*)(gdata + 60416); // [32]
	Conv_21 = (float*)(gdata + 60544); // [2048]
	Conv_21_bs = (float*)(gdata + 68736); // [64]
	Conv_23 = (float*)(gdata + 68992); // [576]
	Conv_23_bs = (float*)(gdata + 71296); // [64]
	Conv_25 = (float*)(gdata + 71552); // [2048]
	Conv_25_bs = (float*)(gdata + 79744); // [32]
	Conv_27 = (float*)(gdata + 79872); // [4096]
	Conv_27_bs = (float*)(gdata + 96256); // [128]
	Conv_29 = (float*)(gdata + 96768); // [1152]
	Conv_29_bs = (float*)(gdata + 101376); // [128]
	Conv_31 = (float*)(gdata + 101888); // [8192]
	Conv_31_bs = (float*)(gdata + 134656); // [64]
	Conv_32 = (float*)(gdata + 134912); // [8192]
	Conv_32_bs = (float*)(gdata + 167680); // [128]
	Conv_34 = (float*)(gdata + 168192); // [1152]
	Conv_34_bs = (float*)(gdata + 172800); // [128]
	Conv_36 = (float*)(gdata + 173312); // [8192]
	Conv_36_bs = (float*)(gdata + 206080); // [64]
	Conv_38 = (float*)(gdata + 206336); // [8192]
	Conv_38_bs = (float*)(gdata + 239104); // [128]
	Conv_40 = (float*)(gdata + 239616); // [1152]
	Conv_40_bs = (float*)(gdata + 244224); // [128]
	Conv_42 = (float*)(gdata + 244736); // [8192]
	Conv_42_bs = (float*)(gdata + 277504); // [64]
	Conv_44 = (float*)(gdata + 277760); // [8192]
	Conv_44_bs = (float*)(gdata + 310528); // [128]
	Conv_46 = (float*)(gdata + 311040); // [1152]
	Conv_46_bs = (float*)(gdata + 315648); // [128]
	Conv_48 = (float*)(gdata + 316160); // [8192]
	Conv_48_bs = (float*)(gdata + 348928); // [64]
	Conv_50 = (float*)(gdata + 349184); // [8192]
	Conv_50_bs = (float*)(gdata + 381952); // [128]
	Conv_52 = (float*)(gdata + 382464); // [1152]
	Conv_52_bs = (float*)(gdata + 387072); // [128]
	Conv_54 = (float*)(gdata + 387584); // [8192]
	Conv_54_bs = (float*)(gdata + 420352); // [64]
	Conv_56 = (float*)(gdata + 420608); // [16384]
	Conv_56_bs = (float*)(gdata + 486144); // [256]
	Conv_58 = (float*)(gdata + 487168); // [2304]
	Conv_58_bs = (float*)(gdata + 496384); // [256]
	Conv_60 = (float*)(gdata + 497408); // [16384]
	Conv_60_bs = (float*)(gdata + 562944); // [64]
	Conv_61 = (float*)(gdata + 563200); // [8192]
	Conv_61_bs = (float*)(gdata + 595968); // [128]
	Conv_63 = (float*)(gdata + 596480); // [1152]
	Conv_63_bs = (float*)(gdata + 601088); // [128]
	Conv_65 = (float*)(gdata + 601600); // [8192]
	Conv_65_bs = (float*)(gdata + 634368); // [64]
	Conv_67 = (float*)(gdata + 634624); // [8192]
	Conv_67_bs = (float*)(gdata + 667392); // [128]
	Conv_69 = (float*)(gdata + 667904); // [1152]
	Conv_69_bs = (float*)(gdata + 672512); // [128]
	Conv_71 = (float*)(gdata + 673024); // [8192]
	Conv_71_bs = (float*)(gdata + 705792); // [64]
	Conv_73 = (float*)(gdata + 706048); // [32768]
	Conv_73_bs = (float*)(gdata + 837120); // [512]
	Conv_75 = (float*)(gdata + 839168); // [8192]
	Conv_75_bs = (float*)(gdata + 871936); // [512]
	Gemm_77 = (float*)(gdata + 873984); // [69632]
	Gemm_77_bs = (float*)(gdata + 1152512); // [136]

	g_nEngineLoaded = 1;

	return 0;
}

void Modeling::dnn_free()
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

int Modeling::getEngineLoaded()
{
	return g_nEngineLoaded;
}

// void save_bin(enn_blob* blob);
// void save_bin(float* blob, int sz);

float* Modeling::dnn_forward(unsigned char* in, int width, int height)
{
	IF_FLAG_STOP; trans(in, 64, 64, 1, mem_blk0, 1);
	// save_bin(mem_blk0); // 0 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 1 
	IF_FLAG_STOP; conv3x3s2_pack1to4_neon(mem_blk2, mem_blk1, 32, 3, 2, Conv_0, Conv_0_bs);
	// save_bin(mem_blk1); // 2 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 3 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 4 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk2, mem_blk0, 32, 3, 1, Conv_2, Conv_2_bs);
	// save_bin(mem_blk0); // 5 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 6 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 64, 1, 1, Conv_4, Conv_4_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 7 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 8 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 9 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk0, 64, 3, 2, Conv_6, Conv_6_bs);
	// save_bin(mem_blk0); // 10 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 11 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 32, 1, 1, Conv_8, Conv_8_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 12 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 64, 1, 1, Conv_9, Conv_9_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 13 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 14 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 15 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 64, 3, 1, Conv_11, Conv_11_bs);
	// save_bin(mem_blk2); // 16 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 17 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 32, 1, 1, Conv_13, Conv_13_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 18 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 19 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 64, 1, 1, Conv_15, Conv_15_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 20 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 21 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 22 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 64, 3, 1, Conv_17, Conv_17_bs);
	// save_bin(mem_blk1); // 23 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 24 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 32, 1, 1, Conv_19, Conv_19_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 25 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 26 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 64, 1, 1, Conv_21, Conv_21_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 27 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 28 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 29 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 64, 3, 1, Conv_23, Conv_23_bs);
	// save_bin(mem_blk2); // 30 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 31 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 32, 1, 1, Conv_25, Conv_25_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 32 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 33 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 128, 1, 1, Conv_27, Conv_27_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 34 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 35 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 36 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk1, 128, 3, 2, Conv_29, Conv_29_bs);
	// save_bin(mem_blk1); // 37 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 38 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 64, 1, 1, Conv_31, Conv_31_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 39 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 128, 1, 1, Conv_32, Conv_32_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 40 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 41 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 42 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 128, 3, 1, Conv_34, Conv_34_bs);
	// save_bin(mem_blk2); // 43 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 44 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk1, 64, 1, 1, Conv_36, Conv_36_bs, (float*)mem_blk3);
	// save_bin(mem_blk1); // 45 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 46 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 128, 1, 1, Conv_38, Conv_38_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 47 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 48 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 49 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 128, 3, 1, Conv_40, Conv_40_bs);
	// save_bin(mem_blk1); // 50 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 51 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 64, 1, 1, Conv_42, Conv_42_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 52 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 53 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 128, 1, 1, Conv_44, Conv_44_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 54 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 55 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 56 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 128, 3, 1, Conv_46, Conv_46_bs);
	// save_bin(mem_blk2); // 57 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 58 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 64, 1, 1, Conv_48, Conv_48_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 59 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 60 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 128, 1, 1, Conv_50, Conv_50_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 61 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 62 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 63 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 128, 3, 1, Conv_52, Conv_52_bs);
	// save_bin(mem_blk1); // 64 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 65 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 64, 1, 1, Conv_54, Conv_54_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 66 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 67 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 256, 1, 1, Conv_56, Conv_56_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 68 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 69 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 70 
	IF_FLAG_STOP; convdw3x3s2_pack4_neon(mem_blk2, mem_blk1, 256, 3, 2, Conv_58, Conv_58_bs);
	// save_bin(mem_blk1); // 71 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 72 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 64, 1, 1, Conv_60, Conv_60_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 73 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk0, mem_blk1, 128, 1, 1, Conv_61, Conv_61_bs, (float*)mem_blk2);
	// save_bin(mem_blk1); // 74 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 75 
	IF_FLAG_STOP; padding(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 76 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk2, 128, 3, 1, Conv_63, Conv_63_bs);
	// save_bin(mem_blk2); // 77 
	IF_FLAG_STOP; relu(mem_blk2);
	// save_bin(mem_blk2); // 78 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk1, 64, 1, 1, Conv_65, Conv_65_bs, (float*)mem_blk3);
	// save_bin(mem_blk1); // 79 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 80 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk2, mem_blk0, 128, 1, 1, Conv_67, Conv_67_bs, (float*)mem_blk1);
	// save_bin(mem_blk0); // 81 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 82 
	IF_FLAG_STOP; padding(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 83 
	IF_FLAG_STOP; convdw3x3s1_pack4_neon(mem_blk3, mem_blk1, 128, 3, 1, Conv_69, Conv_69_bs);
	// save_bin(mem_blk1); // 84 
	IF_FLAG_STOP; relu(mem_blk1);
	// save_bin(mem_blk1); // 85 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 64, 1, 1, Conv_71, Conv_71_bs, (float*)mem_blk3);
	// save_bin(mem_blk0); // 86 
	IF_FLAG_STOP; eltwise_sum_pack4_neon(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 87 
	IF_FLAG_STOP; conv1x1s1_sgemm_pack4_neon(mem_blk1, mem_blk0, 512, 1, 1, Conv_73, Conv_73_bs, (float*)mem_blk2);
	// save_bin(mem_blk0); // 88 
	IF_FLAG_STOP; relu(mem_blk0);
	// save_bin(mem_blk0); // 89 
	IF_FLAG_STOP; convdw_gdc_pack4_neon(mem_blk0, mem_blk1, 512, 4, 1, Conv_75, Conv_75_bs);
	// save_bin(mem_blk1); // 90 
	mem_blk1->h = mem_blk1->c * mem_blk1->w * mem_blk1->h * mem_blk1->pack;
	mem_blk1->w = 1;
	mem_blk1->c = 1;
	mem_blk1->pack = 1;
	mem_blk1->cstep = mem_blk1->h;
	// save_bin(mem_blk1); // 91 
	IF_FLAG_STOP; innerproduct(mem_blk1, mem_blk0, 136, Gemm_77, Gemm_77_bs);
	// save_bin(mem_blk0); // 92 
	return (float*)mem_blk0->mem;
}
