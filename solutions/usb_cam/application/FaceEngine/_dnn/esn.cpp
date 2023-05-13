
// VERSION ESN_CODE_1.00 // 10ms

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "esn.h"
// #include "ennq_global.h"
// #include "ennq_trans.h"
// #include "ennq_pad.h"
// #include "ennq_conv.h"
// #include "ennq_inner.h"
// #include "ennq_slel.h"
// #include "ennq_pool.h"
// #include "ennq_normal.h"
// #include "ennq_activation.h"
// #include "ennq_quantize.h"
// #include "ennq_seblock.h"

//using namespace ENNQ;

/*
extern int g_nStopEngine;
#define IF_FLAG_STOP if (g_nStopEngine) return 0

ESN::ESN()
{
	
	dic_data = 0;
	mem_data = 0;
	g_nEngineLoaded = 0;
	
}

ESN::~ESN()
{
	
	dnn_free();
	
}

int ESN::dnn_dic_size()
{
	return 123860;
}

int ESN::dnn_mem_size()
{
	return 36864 + 36864 + 43264 + 32448;
}

int ESN::dnn_create(const char* fn, unsigned char* pMemBuf)
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

int ESN::dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf)
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

	mem_blk0 = (signed char*)mem_all; mem_all += 36864;
	mem_blk1 = (signed char*)mem_all; mem_all += 36864;
	mem_blk2 = (signed char*)mem_all; mem_all += 43264;
	mem_blk3 = (signed char*)mem_all;

	Conv_0 = (signed char*)(gdata + 0); // [144]
	Conv_0_sc = (float*)(gdata + 144); // [16]
	Conv_0_bs = (float*)(gdata + 208); // [16]
	Conv_2 = (signed char*)(gdata + 272); // [768]
	Conv_2_q = (float*)(gdata + 1040); // [1]
	Conv_2_sc = (float*)(gdata + 1044); // [48]
	Conv_2_bs = (float*)(gdata + 1236); // [48]
	Conv_4 = (signed char*)(gdata + 1428); // [432]
	Conv_4_sc = (float*)(gdata + 1860); // [48]
	Conv_4_bs = (float*)(gdata + 2052); // [48]
	Conv_6 = (signed char*)(gdata + 2244); // [768]
	Conv_6_sc = (float*)(gdata + 3012); // [16]
	Conv_6_bs = (float*)(gdata + 3076); // [16]
	Conv_8 = (signed char*)(gdata + 3140); // [1024]
	Conv_8_q = (float*)(gdata + 4164); // [1]
	Conv_8_sc = (float*)(gdata + 4168); // [64]
	Conv_8_bs = (float*)(gdata + 4424); // [64]
	Conv_10 = (signed char*)(gdata + 4680); // [576]
	Conv_10_sc = (float*)(gdata + 5256); // [64]
	Conv_10_bs = (float*)(gdata + 5512); // [64]
	Conv_12 = (signed char*)(gdata + 5768); // [2048]
	Conv_12_sc = (float*)(gdata + 7816); // [32]
	Conv_12_bs = (float*)(gdata + 7944); // [32]
	Conv_13 = (signed char*)(gdata + 8072); // [3072]
	Conv_13_q = (float*)(gdata + 11144); // [1]
	Conv_13_sc = (float*)(gdata + 11148); // [96]
	Conv_13_bs = (float*)(gdata + 11532); // [96]
	Conv_15 = (signed char*)(gdata + 11916); // [864]
	Conv_15_sc = (float*)(gdata + 12780); // [96]
	Conv_15_bs = (float*)(gdata + 13164); // [96]
	Conv_17 = (signed char*)(gdata + 13548); // [3072]
	Conv_17_sc = (float*)(gdata + 16620); // [32]
	Conv_17_bs = (float*)(gdata + 16748); // [32]
	Conv_19 = (signed char*)(gdata + 16876); // [3840]
	Conv_19_q = (float*)(gdata + 20716); // [1]
	Conv_19_sc = (float*)(gdata + 20720); // [120]
	Conv_19_bs = (float*)(gdata + 21200); // [120]
	Conv_21 = (signed char*)(gdata + 21680); // [1080]
	Conv_21_sc = (float*)(gdata + 22760); // [120]
	Conv_21_bs = (float*)(gdata + 23240); // [120]
	Conv_23 = (signed char*)(gdata + 23720); // [3840]
	Conv_23_sc = (float*)(gdata + 27560); // [32]
	Conv_23_bs = (float*)(gdata + 27688); // [32]
	Conv_25 = (signed char*)(gdata + 27816); // [4608]
	Conv_25_q = (float*)(gdata + 32424); // [1]
	Conv_25_sc = (float*)(gdata + 32428); // [144]
	Conv_25_bs = (float*)(gdata + 33004); // [144]
	Conv_27 = (signed char*)(gdata + 33580); // [1296]
	Conv_27_sc = (float*)(gdata + 34876); // [144]
	Conv_27_bs = (float*)(gdata + 35452); // [144]
	Conv_29 = (signed char*)(gdata + 36028); // [9216]
	Conv_29_sc = (float*)(gdata + 45244); // [64]
	Conv_29_bs = (float*)(gdata + 45500); // [64]
	Conv_30 = (signed char*)(gdata + 45756); // [9216]
	Conv_30_q = (float*)(gdata + 54972); // [1]
	Conv_30_sc = (float*)(gdata + 54976); // [144]
	Conv_30_bs = (float*)(gdata + 55552); // [144]
	Conv_32 = (signed char*)(gdata + 56128); // [1296]
	Conv_32_sc = (float*)(gdata + 57424); // [144]
	Conv_32_bs = (float*)(gdata + 58000); // [144]
	Conv_34 = (signed char*)(gdata + 58576); // [9216]
	Conv_34_sc = (float*)(gdata + 67792); // [64]
	Conv_34_bs = (float*)(gdata + 68048); // [64]
	Conv_36 = (signed char*)(gdata + 68304); // [15360]
	Conv_36_q = (float*)(gdata + 83664); // [1]
	Conv_36_sc = (float*)(gdata + 83668); // [240]
	Conv_36_bs = (float*)(gdata + 84628); // [240]
	Conv_38 = (signed char*)(gdata + 85588); // [2160]
	Conv_38_sc = (float*)(gdata + 87748); // [240]
	Conv_38_bs = (float*)(gdata + 88708); // [240]
	Conv_40 = (signed char*)(gdata + 89668); // [30720]
	Conv_40_sc = (float*)(gdata + 120388); // [128]
	Conv_40_bs = (float*)(gdata + 120900); // [128]
	Conv_42 = (signed char*)(gdata + 121412); // [1152]
	Conv_42_sc = (float*)(gdata + 122564); // [128]
	Conv_42_bs = (float*)(gdata + 123076); // [128]
	Gemm_44 = (signed char*)(gdata + 123588); // [256]
	Gemm_44_sc = (float*)(gdata + 123844); // [2]
	Gemm_44_bs = (float*)(gdata + 123852); // [2]

	g_nEngineLoaded = 1;
	
	return 0;
}

void ESN::dnn_free()
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

// void save_bin(void* buf, int size);
float* ESN::dnn_forward(unsigned char* in)
{
	IF_FLAG_STOP; transpose1(in, 48, mem_blk0);
	// save_bin(mem_blk0, 2304); // 0 
	IF_FLAG_STOP; padding_nn(mem_blk0, 1, 48, mem_blk2, 1);
	// save_bin(mem_blk2, 2500); // 1 
	IF_FLAG_STOP; conv_nf_r(mem_blk2, 1, 50, (float*)mem_blk1, 16, 24, 3, 2, Conv_0, Conv_0_sc, Conv_0_bs, 0);
	// save_bin(mem_blk1, 36864); // 2 
	IF_FLAG_STOP; quantize((float*)mem_blk1, 9216, mem_blk2, Conv_2_q);
	// save_bin(mem_blk2, 9216); // 3 
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 16, 24, mem_blk0, 48, 24, 1, 1, Conv_2, Conv_2_sc, Conv_2_bs, mem_blk3);
	// save_bin(mem_blk0, 27648); // 4 
	IF_FLAG_STOP; padding_nn(mem_blk0, 48, 24, mem_blk3, 1);
	// save_bin(mem_blk3, 32448); // 5 
	IF_FLAG_STOP; convd_nn_r(mem_blk3, 48, 26, mem_blk2, 48, 24, 3, 1, Conv_4, Conv_4_sc, Conv_4_bs, 0);
	// save_bin(mem_blk2, 27648); // 6 
	IF_FLAG_STOP; conv_nf(mem_blk2, 48, 24, (float*)mem_blk0, 16, 24, 1, 1, Conv_6, Conv_6_sc, Conv_6_bs, mem_blk3);
	// save_bin(mem_blk0, 36864); // 7 
	IF_FLAG_STOP; eltwise_sum_ff((float*)mem_blk1, (float*)mem_blk0, 9216, (float*)mem_blk2);
	// save_bin(mem_blk2, 36864); // 8 
	IF_FLAG_STOP; quantize((float*)mem_blk2, 9216, mem_blk1, Conv_8_q);
	// save_bin(mem_blk1, 9216); // 9 
	IF_FLAG_STOP; conv_nn_r(mem_blk1, 16, 24, mem_blk0, 64, 24, 1, 1, Conv_8, Conv_8_sc, Conv_8_bs, mem_blk2);
	// save_bin(mem_blk0, 36864); // 10 
	IF_FLAG_STOP; padding_nn(mem_blk0, 64, 24, mem_blk2, 1);
	// save_bin(mem_blk2, 43264); // 11 
	IF_FLAG_STOP; convd_nn_r(mem_blk2, 64, 26, mem_blk1, 64, 12, 3, 2, Conv_10, Conv_10_sc, Conv_10_bs, 0);
	// save_bin(mem_blk1, 9216); // 12 
	IF_FLAG_STOP; conv_nf(mem_blk1, 64, 12, (float*)mem_blk0, 32, 12, 1, 1, Conv_12, Conv_12_sc, Conv_12_bs, mem_blk2);
	// save_bin(mem_blk0, 18432); // 13 
	IF_FLAG_STOP; quantize((float*)mem_blk0, 4608, mem_blk2, Conv_13_q);
	// save_bin(mem_blk2, 4608); // 14 
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 32, 12, mem_blk1, 96, 12, 1, 1, Conv_13, Conv_13_sc, Conv_13_bs, mem_blk3);
	// save_bin(mem_blk1, 13824); // 15 
	IF_FLAG_STOP; padding_nn(mem_blk1, 96, 12, mem_blk3, 1);
	// save_bin(mem_blk3, 18816); // 16 
	IF_FLAG_STOP; convd_nn_r(mem_blk3, 96, 14, mem_blk2, 96, 12, 3, 1, Conv_15, Conv_15_sc, Conv_15_bs, 0);
	// save_bin(mem_blk2, 13824); // 17 
	IF_FLAG_STOP; conv_nf(mem_blk2, 96, 12, (float*)mem_blk1, 32, 12, 1, 1, Conv_17, Conv_17_sc, Conv_17_bs, mem_blk3);
	// save_bin(mem_blk1, 18432); // 18 
	IF_FLAG_STOP; eltwise_sum_ff((float*)mem_blk0, (float*)mem_blk1, 4608, (float*)mem_blk2);
	// save_bin(mem_blk2, 18432); // 19 
	IF_FLAG_STOP; quantize((float*)mem_blk2, 4608, mem_blk1, Conv_19_q);
	// save_bin(mem_blk1, 4608); // 20 
	IF_FLAG_STOP; conv_nn_r(mem_blk1, 32, 12, mem_blk0, 120, 12, 1, 1, Conv_19, Conv_19_sc, Conv_19_bs, mem_blk3);
	// save_bin(mem_blk0, 17280); // 21 
	IF_FLAG_STOP; padding_nn(mem_blk0, 120, 12, mem_blk3, 1);
	// save_bin(mem_blk3, 23520); // 22 
	IF_FLAG_STOP; convd_nn_r(mem_blk3, 120, 14, mem_blk1, 120, 12, 3, 1, Conv_21, Conv_21_sc, Conv_21_bs, 0);
	// save_bin(mem_blk1, 17280); // 23 
	IF_FLAG_STOP; conv_nf(mem_blk1, 120, 12, (float*)mem_blk0, 32, 12, 1, 1, Conv_23, Conv_23_sc, Conv_23_bs, mem_blk3);
	// save_bin(mem_blk0, 18432); // 24 
	IF_FLAG_STOP; eltwise_sum_ff((float*)mem_blk2, (float*)mem_blk0, 4608, (float*)mem_blk1);
	// save_bin(mem_blk1, 18432); // 25 
	IF_FLAG_STOP; quantize((float*)mem_blk1, 4608, mem_blk2, Conv_25_q);
	// save_bin(mem_blk2, 4608); // 26 
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 32, 12, mem_blk0, 144, 12, 1, 1, Conv_25, Conv_25_sc, Conv_25_bs, mem_blk1);
	// save_bin(mem_blk0, 20736); // 27 
	IF_FLAG_STOP; padding_nn(mem_blk0, 144, 12, mem_blk2, 1);
	// save_bin(mem_blk2, 28224); // 28 
	IF_FLAG_STOP; convd_nn_r(mem_blk2, 144, 14, mem_blk1, 144, 6, 3, 2, Conv_27, Conv_27_sc, Conv_27_bs, 0);
	// save_bin(mem_blk1, 5184); // 29 
	IF_FLAG_STOP; conv_nf(mem_blk1, 144, 6, (float*)mem_blk0, 64, 6, 1, 1, Conv_29, Conv_29_sc, Conv_29_bs, mem_blk2);
	// save_bin(mem_blk0, 9216); // 30 
	IF_FLAG_STOP; quantize((float*)mem_blk0, 2304, mem_blk2, Conv_30_q);
	// save_bin(mem_blk2, 2304); // 31 
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 64, 6, mem_blk1, 144, 6, 1, 1, Conv_30, Conv_30_sc, Conv_30_bs, mem_blk3);
	// save_bin(mem_blk1, 5184); // 32 
	IF_FLAG_STOP; padding_nn(mem_blk1, 144, 6, mem_blk3, 1);
	// save_bin(mem_blk3, 9216); // 33 
	IF_FLAG_STOP; convd_nn_r(mem_blk3, 144, 8, mem_blk2, 144, 6, 3, 1, Conv_32, Conv_32_sc, Conv_32_bs, 0);
	// save_bin(mem_blk2, 5184); // 34 
	IF_FLAG_STOP; conv_nf(mem_blk2, 144, 6, (float*)mem_blk1, 64, 6, 1, 1, Conv_34, Conv_34_sc, Conv_34_bs, mem_blk3);
	// save_bin(mem_blk1, 9216); // 35 
	IF_FLAG_STOP; eltwise_sum_ff((float*)mem_blk0, (float*)mem_blk1, 2304, (float*)mem_blk2);
	// save_bin(mem_blk2, 9216); // 36 
	IF_FLAG_STOP; quantize((float*)mem_blk2, 2304, mem_blk1, Conv_36_q);
	// save_bin(mem_blk1, 2304); // 37 
	IF_FLAG_STOP; conv_nn_r(mem_blk1, 64, 6, mem_blk0, 240, 6, 1, 1, Conv_36, Conv_36_sc, Conv_36_bs, mem_blk2);
	// save_bin(mem_blk0, 8640); // 38 
	IF_FLAG_STOP; padding_nn(mem_blk0, 240, 6, mem_blk2, 1);
	// save_bin(mem_blk2, 15360); // 39 
	IF_FLAG_STOP; convd_nn_r(mem_blk2, 240, 8, mem_blk1, 240, 3, 3, 2, Conv_38, Conv_38_sc, Conv_38_bs, 0);
	// save_bin(mem_blk1, 2160); // 40 
	IF_FLAG_STOP; conv_nn_r(mem_blk1, 240, 3, mem_blk0, 128, 3, 1, 1, Conv_40, Conv_40_sc, Conv_40_bs, mem_blk2);
	// save_bin(mem_blk0, 1152); // 41 
	IF_FLAG_STOP; convd_nn(mem_blk0, 128, 3, mem_blk1, 128, 1, 3, 1, Conv_42, Conv_42_sc, Conv_42_bs, 0);
	// save_bin(mem_blk1, 128); // 42 
	IF_FLAG_STOP; inner_nf(mem_blk1, 128, (float*)mem_blk0, 2, Gemm_44, Gemm_44_sc, Gemm_44_bs);
	// save_bin(mem_blk0, 8); // 43 
	return (float*)mem_blk0;
}
*/