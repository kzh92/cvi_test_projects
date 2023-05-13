
// VERSION OCC_CODE_1.00

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "occ.h"
// #include "ennq_global.h"
// #include "ennq_trans.h"
// #include "ennq_pad.h"
// #include "ennq_conv.h"
// #include "ennq_inner.h"
// #include "ennq_slel.h"
// #include "ennq_pool.h"

#if 0
using namespace ENNQ;

extern int g_nStopEngine;
#define IF_FLAG_STOP if (g_nStopEngine) return 0

Occlusion::Occlusion()
{
	dic_data = 0;
	mem_data = 0;
	g_nEngineLoaded = 0;
}

Occlusion::~Occlusion()
{
	dnn_free();
}

int Occlusion::dnn_dic_size()
{
	return 363416;
}

int Occlusion::dnn_mem_size()
{
	return 73728 + 32768 + 9248;
}

int Occlusion::dnn_create(const char* fn, unsigned char* pMemBuf)
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

int Occlusion::dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf)
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

	mem_blk0 = (signed char*)mem_all; mem_all += 73728;
	mem_blk1 = (signed char*)mem_all; mem_all += 32768;
	mem_blk2 = (signed char*)mem_all;

	Conv_0 = (signed char*)(gdata + 0); // [72]
	Conv_0_sc = (float*)(gdata + 72); // [8]
	Conv_0_bs = (float*)(gdata + 104); // [8]
	Conv_4 = (signed char*)(gdata + 136); // [1152]
	Conv_4_sc = (float*)(gdata + 1288); // [16]
	Conv_4_bs = (float*)(gdata + 1352); // [16]
	Conv_8 = (signed char*)(gdata + 1416); // [4608]
	Conv_8_sc = (float*)(gdata + 6024); // [32]
	Conv_8_bs = (float*)(gdata + 6152); // [32]
	Conv_12 = (signed char*)(gdata + 6280); // [18432]
	Conv_12_sc = (float*)(gdata + 24712); // [64]
	Conv_12_bs = (float*)(gdata + 24968); // [64]
	Conv_16 = (signed char*)(gdata + 25224); // [73728]
	Conv_16_sc = (float*)(gdata + 98952); // [128]
	Conv_16_bs = (float*)(gdata + 99464); // [128]
	Gemm_25 = (signed char*)(gdata + 99976); // [262144]
	Gemm_25_sc = (float*)(gdata + 362120); // [128]
	Gemm_25_bs = (float*)(gdata + 362632); // [128]
	Gemm_27 = (signed char*)(gdata + 363144); // [256]
	Gemm_27_sc = (float*)(gdata + 363400); // [2]
	Gemm_27_bs = (float*)(gdata + 363408); // [2]

	g_nEngineLoaded = 2;

	return 0;
}

void Occlusion::dnn_free()
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
float* Occlusion::dnn_forward(unsigned char* in)
{
	IF_FLAG_STOP;  transpose1(in, 64, mem_blk0);
	// save_bin(mem_blk0, 1 * 64 * 64);
	IF_FLAG_STOP; padding_nn(mem_blk0, 1, 64, mem_blk2, 1);
	// save_bin(mem_blk2, 1 * 66 * 66);
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 1, 66, mem_blk1, 8, 64, 3, 1, Conv_0, Conv_0_sc, Conv_0_bs, mem_blk0);
	// save_bin(mem_blk1, 8 * 64 * 64);
	IF_FLAG_STOP; pooling_max_nn(mem_blk1, 8, 64, mem_blk0);
	// save_bin(mem_blk0, 8 * 32 * 32);
	IF_FLAG_STOP; padding_nn(mem_blk0, 8, 32, mem_blk2, 1);
	// save_bin(mem_blk2, 8 * 34 * 34);
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 8, 34, mem_blk1, 16, 32, 3, 1, Conv_4, Conv_4_sc, Conv_4_bs, mem_blk0);
	// save_bin(mem_blk1, 16 * 32 * 32);
	IF_FLAG_STOP; pooling_max_nn(mem_blk1, 16, 32, mem_blk0);
	// save_bin(mem_blk0, 16 * 16 * 16);
	IF_FLAG_STOP; padding_nn(mem_blk0, 16, 16, mem_blk2, 1);
	// save_bin(mem_blk2, 16 * 18 * 18);
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 16, 18, mem_blk1, 32, 16, 3, 1, Conv_8, Conv_8_sc, Conv_8_bs, mem_blk0);
	// save_bin(mem_blk1, 32 * 16 * 16);
	IF_FLAG_STOP; pooling_max_nn(mem_blk1, 32, 16, mem_blk0);
	// save_bin(mem_blk0, 32 * 8 * 8);
	IF_FLAG_STOP; padding_nn(mem_blk0, 32, 8, mem_blk2, 1);
	// save_bin(mem_blk2, 32 * 10 * 10);
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 32, 10, mem_blk1, 64, 8, 3, 1, Conv_12, Conv_12_sc, Conv_12_bs, mem_blk0);
	// save_bin(mem_blk1, 64 * 8 * 8);
	IF_FLAG_STOP; pooling_max_nn(mem_blk1, 64, 8, mem_blk0);
	// save_bin(mem_blk0, 64 * 4 * 4);
	IF_FLAG_STOP; padding_nn(mem_blk0, 64, 4, mem_blk2, 1);
	// save_bin(mem_blk2, 64 * 6 * 6);
	IF_FLAG_STOP; conv_nn_r(mem_blk2, 64, 6, mem_blk1, 128, 4, 3, 1, Conv_16, Conv_16_sc, Conv_16_bs, mem_blk0);
	// save_bin(mem_blk1, 128 * 4 * 4);
	IF_FLAG_STOP; inner_nn_r(mem_blk1, 2048, mem_blk0, 128, Gemm_25, Gemm_25_sc, Gemm_25_bs);
	// save_bin(mem_blk0, 128);
	IF_FLAG_STOP; inner_nf(mem_blk0, 128, (float*)mem_blk1, 2, Gemm_27, Gemm_27_sc, Gemm_27_bs);
	// save_bin(mem_blk1, 2 * 4);
	return (float*)mem_blk1;
}
#endif
