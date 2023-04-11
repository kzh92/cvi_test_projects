
// VERSION Live_CODE_1.00

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

void LiveMnSE3_LiveMnSE3(LiveMnSE3* ts)
{
    ts->dic_data = 0;
    ts->mem_data = 0;
    ts->mem_blk0 = 0;
    ts->mem_blk1 = 0;
    ts->mem_blk2 = 0;
    ts->g_nEngineLoaded = 0;
}

void LiveMnSE3_LiveMnSE3_(LiveMnSE3* ts)
{
    LiveMnSE3_dnn_free(ts);
}

int LiveMnSE3_dnn_dic_size()
{
	return 123484;
}

int LiveMnSE3_dnn_mem_size()
{
	return 135184 + 270352 + 291472 + 78352;
}

int LiveMnSE3_dnn_create(LiveMnSE3* ts, const char* fn, float rConfig, unsigned char* pMemBuf) // float rConfig = 75, unsigned char* pMemBuf = 0
{
    if (ts->g_nEngineLoaded) return 0;
    int nDicSize = LiveMnSE3_dnn_dic_size();

    unsigned char* local_dicdata = (unsigned char*)enn_aligned_malloc(nDicSize, 16);
	if (!local_dicdata) return 1;

	FILE *f = fopen(fn, "rb");
	if (!f)
	{
        enn_aligned_free(local_dicdata);
		return 1;
	}

	int nReadBytes = fread(local_dicdata, 1, nDicSize, f);
	fclose(f);

	if (nReadBytes < nDicSize)
	{
        enn_aligned_free(local_dicdata);
		return 1;
	}

    int ret = LiveMnSE3_dnn_create_(ts, local_dicdata, nDicSize, rConfig, pMemBuf);
	if (ret)
	{
        enn_aligned_free(local_dicdata);
		return 1;
	}

    ts->dic_data = local_dicdata;
    ts->g_nEngineLoaded = 1;

	return 0;
}

int LiveMnSE3_dnn_create_(LiveMnSE3* ts, unsigned char* pDicData, int nDicDataSize, float rConfig, unsigned char* pMemBuf) // float rConfig = 75, unsigned char* pMemBuf = 0
{
    if (ts->g_nEngineLoaded) return 0;

	unsigned char* mem_all;
	if (pMemBuf)
	{
		mem_all = pMemBuf;
        ts->mem_data = 0;
	}
	else
	{
        mem_all = (unsigned char*)enn_aligned_malloc(LiveMnSE3_dnn_mem_size(), 16);
		if (!mem_all) return 1;
        ts->mem_data = mem_all;
	}
    ts->dic_data = 0;

	unsigned char* gdata = pDicData;

    ts->mem_blk0 = (enn_blob*)mem_all; mem_all += 135184;
    ts->mem_blk1 = (enn_blob*)mem_all; mem_all += 270352;
    ts->mem_blk2 = (enn_blob*)mem_all; mem_all += 291472;
    ts->mem_blk3 = (enn_blob*)mem_all;

	ts->Conv_1 = (unsigned short*)(gdata + 0); // [108]
	ts->Conv_1_bs = (unsigned short*)(gdata + 216); // [12]
	ts->Conv_4 = (unsigned short*)(gdata + 240); // [108]
	ts->Conv_4_bs = (unsigned short*)(gdata + 456); // [12]
	ts->Conv_6 = (unsigned short*)(gdata + 480); // [288]
	ts->Conv_6_bs = (unsigned short*)(gdata + 1056); // [24]
	ts->Conv_9 = (unsigned short*)(gdata + 1104); // [216]
	ts->Conv_9_bs = (unsigned short*)(gdata + 1536); // [24]
	ts->Conv_11 = (unsigned short*)(gdata + 1584); // [288]
	ts->Conv_11_bs = (unsigned short*)(gdata + 2160); // [12]
	ts->Conv_12 = (unsigned short*)(gdata + 2184); // [288]
	ts->Conv_12_bs = (unsigned short*)(gdata + 2760); // [24]
	ts->Conv_15 = (unsigned short*)(gdata + 2808); // [216]
	ts->Conv_15_bs = (unsigned short*)(gdata + 3240); // [24]
	ts->Conv_17 = (unsigned short*)(gdata + 3288); // [288]
	ts->Conv_17_bs = (unsigned short*)(gdata + 3864); // [12]
	ts->Conv_19 = (unsigned short*)(gdata + 3888); // [288]
	ts->Conv_19_bs = (unsigned short*)(gdata + 4464); // [24]
	ts->Conv_22 = (unsigned short*)(gdata + 4512); // [216]
	ts->Conv_22_bs = (unsigned short*)(gdata + 4944); // [24]
	ts->Conv_24 = (unsigned short*)(gdata + 4992); // [288]
	ts->Conv_24_bs = (unsigned short*)(gdata + 5568); // [12]
	ts->Conv_26 = (unsigned short*)(gdata + 5592); // [576]
	ts->Conv_26_bs = (unsigned short*)(gdata + 6744); // [48]
	ts->Conv_29 = (unsigned short*)(gdata + 6840); // [432]
	ts->Conv_29_bs = (unsigned short*)(gdata + 7704); // [48]
	ts->Conv_31 = (unsigned short*)(gdata + 7800); // [1152]
	ts->Conv_31_bs = (unsigned short*)(gdata + 10104); // [24]
	ts->Conv_32 = (unsigned short*)(gdata + 10152); // [1152]
	ts->Conv_32_bs = (unsigned short*)(gdata + 12456); // [48]
	ts->Conv_35 = (unsigned short*)(gdata + 12552); // [432]
	ts->Conv_35_bs = (unsigned short*)(gdata + 13416); // [48]
	ts->Conv_37 = (unsigned short*)(gdata + 13512); // [1152]
	ts->Conv_37_bs = (unsigned short*)(gdata + 15816); // [24]
	ts->Conv_39 = (unsigned short*)(gdata + 15864); // [1152]
	ts->Conv_39_bs = (unsigned short*)(gdata + 18168); // [48]
	ts->Conv_42 = (unsigned short*)(gdata + 18264); // [432]
	ts->Conv_42_bs = (unsigned short*)(gdata + 19128); // [48]
	ts->Conv_44 = (unsigned short*)(gdata + 19224); // [1152]
	ts->Conv_44_bs = (unsigned short*)(gdata + 21528); // [24]
	ts->Conv_47 = (unsigned short*)(gdata + 21576); // [144]
	ts->Conv_47_bs = (unsigned short*)(gdata + 21864); // [6]
	ts->Conv_49 = (unsigned short*)(gdata + 21876); // [144]
	ts->Conv_49_bs = (unsigned short*)(gdata + 22164); // [24]
	ts->Conv_53 = (unsigned short*)(gdata + 22212); // [2304]
	ts->Conv_53_bs = (unsigned short*)(gdata + 26820); // [96]
	ts->Conv_56 = (unsigned short*)(gdata + 27012); // [864]
	ts->Conv_56_bs = (unsigned short*)(gdata + 28740); // [96]
	ts->Conv_58 = (unsigned short*)(gdata + 28932); // [4608]
	ts->Conv_58_bs = (unsigned short*)(gdata + 38148); // [48]
	ts->Conv_59 = (unsigned short*)(gdata + 38244); // [4608]
	ts->Conv_59_bs = (unsigned short*)(gdata + 47460); // [96]
	ts->Conv_62 = (unsigned short*)(gdata + 47652); // [864]
	ts->Conv_62_bs = (unsigned short*)(gdata + 49380); // [96]
	ts->Conv_64 = (unsigned short*)(gdata + 49572); // [4608]
	ts->Conv_64_bs = (unsigned short*)(gdata + 58788); // [48]
	ts->Conv_66 = (unsigned short*)(gdata + 58884); // [4608]
	ts->Conv_66_bs = (unsigned short*)(gdata + 68100); // [96]
	ts->Conv_69 = (unsigned short*)(gdata + 68292); // [864]
	ts->Conv_69_bs = (unsigned short*)(gdata + 70020); // [96]
	ts->Conv_71 = (unsigned short*)(gdata + 70212); // [4608]
	ts->Conv_71_bs = (unsigned short*)(gdata + 79428); // [48]
	ts->Conv_74 = (unsigned short*)(gdata + 79524); // [576]
	ts->Conv_74_bs = (unsigned short*)(gdata + 80676); // [12]
	ts->Conv_76 = (unsigned short*)(gdata + 80700); // [576]
	ts->Conv_76_bs = (unsigned short*)(gdata + 81852); // [48]
	ts->Conv_80 = (unsigned short*)(gdata + 81948); // [9216]
	ts->Conv_80_bs = (unsigned short*)(gdata + 100380); // [192]
	ts->Conv_82 = (unsigned short*)(gdata + 100764); // [9216]
	ts->Conv_82_bs = (unsigned short*)(gdata + 119196); // [192]
	ts->Gemm_84 = (unsigned short*)(gdata + 119580); // [1920]
	ts->Gemm_84_bs = (unsigned short*)(gdata + 123420); // [10]
	ts->Gemm_86 = (unsigned short*)(gdata + 123440); // [20]
	ts->Gemm_86_bs = (unsigned short*)(gdata + 123480); // [2]

    ts->Gemm_86_bs_mem[0] = ts->Gemm_86_bs[0];
    ts->Gemm_86_bs_mem[1] = ts->Gemm_86_bs[1];
    ts->Gemm_86_bs = ts->Gemm_86_bs_mem;
	ts->Gemm_86_bs[1] = _gnu_f2h_internal_f(_gnu_h2f_internal_f(ts->Gemm_86_bs[1]) + rConfig - 75);

    ts->g_nEngineLoaded = 2;

	return 0;
}

void LiveMnSE3_dnn_free(LiveMnSE3* ts)
{
    if (!ts->g_nEngineLoaded) return;

    if (ts->dic_data)
	{
        enn_aligned_free(ts->dic_data);
        ts->dic_data = 0;
	}

    if (ts->mem_data)
	{
        enn_aligned_free(ts->mem_data);
        ts->mem_data = 0;
	}

    ts->g_nEngineLoaded = 0;
}

int LiveMnSE3_getEngineLoaded(LiveMnSE3* ts)
{
    return ts->g_nEngineLoaded;
}

//void save_bin(enn_blob* blob);
//void save_bin(float* blob, int sz);

float* LiveMnSE3_dnn_forward(LiveMnSE3* ts, unsigned char* in)
{
    IF_FLAG_STOP; enn_trans(in, 88, 128, 1, ts->mem_blk0, 1, 0, 0.003921568627451f);
	// save_bin(mem_blk0); // 0 
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
	// save_bin(mem_blk2); // 1 
    IF_FLAG_STOP; enn_conv3x3s2_pack1to4_neon(ts->mem_blk2, ts->mem_blk1, 12, 3, 2, ts->Conv_1, ts->Conv_1_bs);
	// save_bin(mem_blk1); // 2 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 3 
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
	// save_bin(mem_blk2); // 4 
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk2, ts->mem_blk0, 12, 3, 1, ts->Conv_4, ts->Conv_4_bs);
	// save_bin(mem_blk0); // 5 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 6 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 24, 1, 1, ts->Conv_6, ts->Conv_6_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk1); // 7 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 8 
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
	// save_bin(mem_blk2); // 9 
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk0, 24, 3, 2, ts->Conv_9, ts->Conv_9_bs);
	// save_bin(mem_blk0); // 10 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 11 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 12, 1, 1, ts->Conv_11, ts->Conv_11_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk1); // 12 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 24, 1, 1, ts->Conv_12, ts->Conv_12_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk0); // 13 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 14 
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(mem_blk3); // 15 
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 24, 3, 1, ts->Conv_15, ts->Conv_15_bs);
	// save_bin(mem_blk2); // 16 
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
	// save_bin(mem_blk2); // 17 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 12, 1, 1, ts->Conv_17, ts->Conv_17_bs, (float*)ts->mem_blk3);
	// save_bin(mem_blk0); // 18 
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
	// save_bin(mem_blk2); // 19 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 24, 1, 1, ts->Conv_19, ts->Conv_19_bs, (float*)ts->mem_blk1);
	// save_bin(mem_blk0); // 20 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 21 
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(mem_blk3); // 22 
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 24, 3, 1, ts->Conv_22, ts->Conv_22_bs);
	// save_bin(mem_blk1); // 23 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 24 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 12, 1, 1, ts->Conv_24, ts->Conv_24_bs, (float*)ts->mem_blk3);
	// save_bin(mem_blk0); // 25 
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
	// save_bin(mem_blk1); // 26 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 48, 1, 1, ts->Conv_26, ts->Conv_26_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk0); // 27 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 28 
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
	// save_bin(mem_blk2); // 29 
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk1, 48, 3, 2, ts->Conv_29, ts->Conv_29_bs);
	// save_bin(mem_blk1); // 30 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 31 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 24, 1, 1, ts->Conv_31, ts->Conv_31_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk0); // 32 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 48, 1, 1, ts->Conv_32, ts->Conv_32_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk1); // 33 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 34 
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk3, 1);
	// save_bin(mem_blk3); // 35 
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 48, 3, 1, ts->Conv_35, ts->Conv_35_bs);
	// save_bin(mem_blk2); // 36 
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
	// save_bin(mem_blk2); // 37 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk1, 24, 1, 1, ts->Conv_37, ts->Conv_37_bs, (float*)ts->mem_blk3);
	// save_bin(mem_blk1); // 38 
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk0, ts->mem_blk1, ts->mem_blk2);
	// save_bin(mem_blk2); // 39 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 48, 1, 1, ts->Conv_39, ts->Conv_39_bs, (float*)ts->mem_blk1);
	// save_bin(mem_blk0); // 40 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 41 
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(mem_blk3); // 42 
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 48, 3, 1, ts->Conv_42, ts->Conv_42_bs);
	// save_bin(mem_blk1); // 43 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 44 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 24, 1, 1, ts->Conv_44, ts->Conv_44_bs, (float*)ts->mem_blk3);
	// save_bin(mem_blk0); // 45 
    IF_FLAG_STOP; enn_pooling_global_aver_pack4_neon(ts->mem_blk0, ts->mem_blk1);
	// save_bin(mem_blk1); // 46 
    IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(mem_blk1); // 47 
    IF_FLAG_STOP; enn_innerproduct(ts->mem_blk1, ts->mem_blk3, 6, ts->Conv_47, ts->Conv_47_bs);
	// save_bin(mem_blk3); // 48 
    IF_FLAG_STOP; enn_relu(ts->mem_blk3);
	// save_bin(mem_blk3); // 49 
    IF_FLAG_STOP; enn_innerproduct(ts->mem_blk3, ts->mem_blk1, 24, ts->Conv_49, ts->Conv_49_bs);
	// save_bin(mem_blk1); // 50 
    IF_FLAG_STOP; enn_sigmoid(ts->mem_blk1);
	// save_bin(mem_blk1); // 51 
    IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(mem_blk1); // 52 
    IF_FLAG_STOP; enn_multiply(ts->mem_blk0, ts->mem_blk1, ts->mem_blk3);
	// save_bin(mem_blk3); // 53 
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk3, ts->mem_blk0);
	// save_bin(mem_blk0); // 54 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 96, 1, 1, ts->Conv_53, ts->Conv_53_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk1); // 55 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 56 
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
	// save_bin(mem_blk2); // 57 
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk0, 96, 3, 2, ts->Conv_56, ts->Conv_56_bs);
	// save_bin(mem_blk0); // 58 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 59 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 48, 1, 1, ts->Conv_58, ts->Conv_58_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk1); // 60 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 96, 1, 1, ts->Conv_59, ts->Conv_59_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk0); // 61 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 62 
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(mem_blk3); // 63 
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 96, 3, 1, ts->Conv_62, ts->Conv_62_bs);
	// save_bin(mem_blk2); // 64 
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
	// save_bin(mem_blk2); // 65 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 48, 1, 1, ts->Conv_64, ts->Conv_64_bs, (float*)ts->mem_blk3);
	// save_bin(mem_blk0); // 66 
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
	// save_bin(mem_blk2); // 67 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 96, 1, 1, ts->Conv_66, ts->Conv_66_bs, (float*)ts->mem_blk1);
	// save_bin(mem_blk0); // 68 
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
	// save_bin(mem_blk0); // 69 
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(mem_blk3); // 70 
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 96, 3, 1, ts->Conv_69, ts->Conv_69_bs);
	// save_bin(mem_blk1); // 71 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 72 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 48, 1, 1, ts->Conv_71, ts->Conv_71_bs, (float*)ts->mem_blk3);
	// save_bin(mem_blk0); // 73 
    IF_FLAG_STOP; enn_pooling_global_aver_pack4_neon(ts->mem_blk0, ts->mem_blk1);
	// save_bin(mem_blk1); // 74 
    IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(mem_blk1); // 75 
    IF_FLAG_STOP; enn_innerproduct(ts->mem_blk1, ts->mem_blk3, 12, ts->Conv_74, ts->Conv_74_bs);
	// save_bin(mem_blk3); // 76 
    IF_FLAG_STOP; enn_relu(ts->mem_blk3);
	// save_bin(mem_blk3); // 77 
    IF_FLAG_STOP; enn_innerproduct(ts->mem_blk3, ts->mem_blk1, 48, ts->Conv_76, ts->Conv_76_bs);
	// save_bin(mem_blk1); // 78 
    IF_FLAG_STOP; enn_sigmoid(ts->mem_blk1);
	// save_bin(mem_blk1); // 79 
    IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(mem_blk1); // 80 
    IF_FLAG_STOP; enn_multiply(ts->mem_blk0, ts->mem_blk1, ts->mem_blk3);
	// save_bin(mem_blk3); // 81 
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk3, ts->mem_blk0);
	// save_bin(mem_blk0); // 82 
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 192, 1, 1, ts->Conv_80, ts->Conv_80_bs, (float*)ts->mem_blk2);
	// save_bin(mem_blk1); // 83 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 84 
    IF_FLAG_STOP; enn_convdw_gdc_pack4_neon_(ts->mem_blk1, ts->mem_blk0, 192, 6, 8, 1, ts->Conv_82, ts->Conv_82_bs);
	// save_bin(mem_blk0); // 85 
    IF_FLAG_STOP; enn_flatten(ts->mem_blk0);
	// save_bin(mem_blk0); // 86 
    IF_FLAG_STOP; enn_innerproduct(ts->mem_blk0, ts->mem_blk1, 10, ts->Gemm_84, ts->Gemm_84_bs);
	// save_bin(mem_blk1); // 87 
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
	// save_bin(mem_blk1); // 88 
    IF_FLAG_STOP; enn_innerproduct(ts->mem_blk1, ts->mem_blk0, 2, ts->Gemm_86, ts->Gemm_86_bs);
	// save_bin(mem_blk0); // 89 
    return (float*)ts->mem_blk0->mem;
}
