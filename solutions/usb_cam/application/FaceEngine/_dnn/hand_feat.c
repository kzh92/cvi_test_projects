
#include <memory.h>
#include <math.h>
#include <stdio.h>

#include "hand_feat.h"

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

void HandFeat_HandFeat(HandFeat* ts)
{
    ts->dic_data = 0;
    ts->mem_data = 0;
    ts->mem_blk0 = 0;
    ts->mem_blk1 = 0;
    ts->mem_blk2 = 0;
    ts->g_nEngineLoaded = 0;
}

void HandFeat_HandFeat_(HandFeat* ts)
{
	HandFeat_dnn_free(ts);
}

int HandFeat_dnn_dic_size()
{
	return 1770624;
}

int HandFeat_dnn_mem_size()
{
	return 1048592 + 2097168 + 2230288 + 591888;
}

int HandFeat_dnn_create(HandFeat* ts, const char* fn, float rConfig, unsigned char* pMemBuf) // float rConfig = 75, unsigned char* pMemBuf = 0
{
    if (ts->g_nEngineLoaded) return 0;
    int nDicSize = HandFeat_dnn_dic_size();

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

    int ret = HandFeat_dnn_create_(ts, local_dicdata, nDicSize, rConfig, pMemBuf);
	if (ret)
	{
        enn_aligned_free(local_dicdata);
		return 1;
	}

    ts->dic_data = local_dicdata;
    ts->g_nEngineLoaded = 1;

	return 0;
}

int HandFeat_dnn_create_(HandFeat* ts, unsigned char* pDicData, int nDicDataSize, float rConfig, unsigned char* pMemBuf) // float rConfig = 75, unsigned char* pMemBuf = 0
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
        mem_all = (unsigned char*)enn_aligned_malloc(HandFeat_dnn_mem_size(), 16);
		if (!mem_all) return 1;
        ts->mem_data = mem_all;
	}
    ts->dic_data = 0;

	unsigned char* gdata = pDicData;

    ts->mem_blk0 = (enn_blob*)mem_all; mem_all += 1048592;
    ts->mem_blk1 = (enn_blob*)mem_all; mem_all += 2097168;
    ts->mem_blk2 = (enn_blob*)mem_all; mem_all += 2230288;
    ts->mem_blk3 = (enn_blob*)mem_all;

	ts->Conv_1 = (unsigned short*)(gdata + 0); // [576]
	ts->Conv_1_bs = (unsigned short*)(gdata + 1152); // [64]
	ts->PRelu_2 = (unsigned short*)(gdata + 1280); // [64]
	ts->Conv_4 = (unsigned short*)(gdata + 1408); // [576]
	ts->Conv_4_bs = (unsigned short*)(gdata + 2560); // [64]
	ts->PRelu_5 = (unsigned short*)(gdata + 2688); // [64]
	ts->Conv_6 = (unsigned short*)(gdata + 2816); // [8192]
	ts->Conv_6_bs = (unsigned short*)(gdata + 19200); // [128]
	ts->PRelu_7 = (unsigned short*)(gdata + 19456); // [128]
	ts->Conv_9 = (unsigned short*)(gdata + 19712); // [1152]
	ts->Conv_9_bs = (unsigned short*)(gdata + 22016); // [128]
	ts->PRelu_10 = (unsigned short*)(gdata + 22272); // [128]
	ts->Conv_11 = (unsigned short*)(gdata + 22528); // [8192]
	ts->Conv_11_bs = (unsigned short*)(gdata + 38912); // [64]
	ts->Conv_12 = (unsigned short*)(gdata + 39040); // [8192]
	ts->Conv_12_bs = (unsigned short*)(gdata + 55424); // [128]
	ts->PRelu_13 = (unsigned short*)(gdata + 55680); // [128]
	ts->Conv_15 = (unsigned short*)(gdata + 55936); // [1152]
	ts->Conv_15_bs = (unsigned short*)(gdata + 58240); // [128]
	ts->PRelu_16 = (unsigned short*)(gdata + 58496); // [128]
	ts->Conv_17 = (unsigned short*)(gdata + 58752); // [8192]
	ts->Conv_17_bs = (unsigned short*)(gdata + 75136); // [64]
	ts->Conv_19 = (unsigned short*)(gdata + 75264); // [8192]
	ts->Conv_19_bs = (unsigned short*)(gdata + 91648); // [128]
	ts->PRelu_20 = (unsigned short*)(gdata + 91904); // [128]
	ts->Conv_22 = (unsigned short*)(gdata + 92160); // [1152]
	ts->Conv_22_bs = (unsigned short*)(gdata + 94464); // [128]
	ts->PRelu_23 = (unsigned short*)(gdata + 94720); // [128]
	ts->Conv_24 = (unsigned short*)(gdata + 94976); // [8192]
	ts->Conv_24_bs = (unsigned short*)(gdata + 111360); // [64]
	ts->Conv_26 = (unsigned short*)(gdata + 111488); // [8192]
	ts->Conv_26_bs = (unsigned short*)(gdata + 127872); // [128]
	ts->PRelu_27 = (unsigned short*)(gdata + 128128); // [128]
	ts->Conv_29 = (unsigned short*)(gdata + 128384); // [1152]
	ts->Conv_29_bs = (unsigned short*)(gdata + 130688); // [128]
	ts->PRelu_30 = (unsigned short*)(gdata + 130944); // [128]
	ts->Conv_31 = (unsigned short*)(gdata + 131200); // [8192]
	ts->Conv_31_bs = (unsigned short*)(gdata + 147584); // [64]
	ts->Conv_33 = (unsigned short*)(gdata + 147712); // [8192]
	ts->Conv_33_bs = (unsigned short*)(gdata + 164096); // [128]
	ts->PRelu_34 = (unsigned short*)(gdata + 164352); // [128]
	ts->Conv_36 = (unsigned short*)(gdata + 164608); // [1152]
	ts->Conv_36_bs = (unsigned short*)(gdata + 166912); // [128]
	ts->PRelu_37 = (unsigned short*)(gdata + 167168); // [128]
	ts->Conv_38 = (unsigned short*)(gdata + 167424); // [8192]
	ts->Conv_38_bs = (unsigned short*)(gdata + 183808); // [64]
	ts->Conv_40 = (unsigned short*)(gdata + 183936); // [16384]
	ts->Conv_40_bs = (unsigned short*)(gdata + 216704); // [256]
	ts->PRelu_41 = (unsigned short*)(gdata + 217216); // [256]
	ts->Conv_43 = (unsigned short*)(gdata + 217728); // [2304]
	ts->Conv_43_bs = (unsigned short*)(gdata + 222336); // [256]
	ts->PRelu_44 = (unsigned short*)(gdata + 222848); // [256]
	ts->Conv_45 = (unsigned short*)(gdata + 223360); // [32768]
	ts->Conv_45_bs = (unsigned short*)(gdata + 288896); // [128]
	ts->Conv_48 = (unsigned short*)(gdata + 289152); // [4096]
	ts->Conv_48_bs = (unsigned short*)(gdata + 297344); // [32]
	ts->Conv_50 = (unsigned short*)(gdata + 297408); // [4096]
	ts->Conv_50_bs = (unsigned short*)(gdata + 305600); // [128]
	ts->Conv_53 = (unsigned short*)(gdata + 305856); // [32768]
	ts->Conv_53_bs = (unsigned short*)(gdata + 371392); // [256]
	ts->PRelu_54 = (unsigned short*)(gdata + 371904); // [256]
	ts->Conv_56 = (unsigned short*)(gdata + 372416); // [2304]
	ts->Conv_56_bs = (unsigned short*)(gdata + 377024); // [256]
	ts->PRelu_57 = (unsigned short*)(gdata + 377536); // [256]
	ts->Conv_58 = (unsigned short*)(gdata + 378048); // [32768]
	ts->Conv_58_bs = (unsigned short*)(gdata + 443584); // [128]
	ts->Conv_61 = (unsigned short*)(gdata + 443840); // [4096]
	ts->Conv_61_bs = (unsigned short*)(gdata + 452032); // [32]
	ts->Conv_63 = (unsigned short*)(gdata + 452096); // [4096]
	ts->Conv_63_bs = (unsigned short*)(gdata + 460288); // [128]
	ts->Conv_67 = (unsigned short*)(gdata + 460544); // [32768]
	ts->Conv_67_bs = (unsigned short*)(gdata + 526080); // [256]
	ts->PRelu_68 = (unsigned short*)(gdata + 526592); // [256]
	ts->Conv_70 = (unsigned short*)(gdata + 527104); // [2304]
	ts->Conv_70_bs = (unsigned short*)(gdata + 531712); // [256]
	ts->PRelu_71 = (unsigned short*)(gdata + 532224); // [256]
	ts->Conv_72 = (unsigned short*)(gdata + 532736); // [32768]
	ts->Conv_72_bs = (unsigned short*)(gdata + 598272); // [128]
	ts->Conv_75 = (unsigned short*)(gdata + 598528); // [4096]
	ts->Conv_75_bs = (unsigned short*)(gdata + 606720); // [32]
	ts->Conv_77 = (unsigned short*)(gdata + 606784); // [4096]
	ts->Conv_77_bs = (unsigned short*)(gdata + 614976); // [128]
	ts->Conv_81 = (unsigned short*)(gdata + 615232); // [32768]
	ts->Conv_81_bs = (unsigned short*)(gdata + 680768); // [256]
	ts->PRelu_82 = (unsigned short*)(gdata + 681280); // [256]
	ts->Conv_84 = (unsigned short*)(gdata + 681792); // [2304]
	ts->Conv_84_bs = (unsigned short*)(gdata + 686400); // [256]
	ts->PRelu_85 = (unsigned short*)(gdata + 686912); // [256]
	ts->Conv_86 = (unsigned short*)(gdata + 687424); // [32768]
	ts->Conv_86_bs = (unsigned short*)(gdata + 752960); // [128]
	ts->Conv_89 = (unsigned short*)(gdata + 753216); // [4096]
	ts->Conv_89_bs = (unsigned short*)(gdata + 761408); // [32]
	ts->Conv_91 = (unsigned short*)(gdata + 761472); // [4096]
	ts->Conv_91_bs = (unsigned short*)(gdata + 769664); // [128]
	ts->Conv_95 = (unsigned short*)(gdata + 769920); // [65536]
	ts->Conv_95_bs = (unsigned short*)(gdata + 900992); // [512]
	ts->PRelu_96 = (unsigned short*)(gdata + 902016); // [512]
	ts->Conv_98 = (unsigned short*)(gdata + 903040); // [4608]
	ts->Conv_98_bs = (unsigned short*)(gdata + 912256); // [512]
	ts->PRelu_99 = (unsigned short*)(gdata + 913280); // [512]
	ts->Conv_100 = (unsigned short*)(gdata + 914304); // [65536]
	ts->Conv_100_bs = (unsigned short*)(gdata + 1045376); // [128]
	ts->Conv_101 = (unsigned short*)(gdata + 1045632); // [65536]
	ts->Conv_101_bs = (unsigned short*)(gdata + 1176704); // [512]
	ts->PRelu_102 = (unsigned short*)(gdata + 1177728); // [512]
	ts->Conv_103 = (unsigned short*)(gdata + 1178752); // [32768]
	ts->Conv_103_bs = (unsigned short*)(gdata + 1244288); // [512]
	ts->MatMul_105 = (unsigned short*)(gdata + 1245312); // [262144]
	ts->MatMul_105_bs = (unsigned short*)(gdata + 1769600); // [512]

//     ts->Gemm_116_bs_mem[0] = ts->Gemm_116_bs[0];
//     ts->Gemm_116_bs_mem[1] = ts->Gemm_116_bs[1];
//     ts->Gemm_116_bs = ts->Gemm_116_bs_mem;
// 	ts->Gemm_116_bs[1] = _gnu_f2h_internal_f(_gnu_h2f_internal_f(ts->Gemm_116_bs[1]) + rConfig - 75);

    ts->g_nEngineLoaded = 2;

	return 0;
}

void HandFeat_dnn_free(HandFeat* ts)
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

int HandFeat_getEngineLoaded(HandFeat* ts)
{
    return ts->g_nEngineLoaded;
}


// void save_bin(enn_blob* blob);
// void save_bin_(float* blob, int sz);

float* HandFeat_dnn_forward(HandFeat* ts, unsigned char* in)
{
	IF_FLAG_STOP; enn_trans(in, 128, 128, 1, ts->mem_blk0, 1, 0, 0.0039215686274509803921568627451f);
	// save_bin(ts->mem_blk0); // 0 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
	// save_bin(ts->mem_blk2); // 1 
	IF_FLAG_STOP; enn_conv3x3s2_pack1to4_neon(ts->mem_blk2, ts->mem_blk1, 64, 3, 2, ts->Conv_1, ts->Conv_1_bs);
	// save_bin(ts->mem_blk1); // 2 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_2);
	// save_bin(ts->mem_blk1); // 3 
	IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
	// save_bin(ts->mem_blk2); // 4 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk2, ts->mem_blk0, 64, 3, 1, ts->Conv_4, ts->Conv_4_bs);
	// save_bin(ts->mem_blk0); // 5 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_5);
	// save_bin(ts->mem_blk0); // 6 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 128, 1, 1, ts->Conv_6, ts->Conv_6_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk1); // 7 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_7);
	// save_bin(ts->mem_blk1); // 8 
	IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
	// save_bin(ts->mem_blk2); // 9 
	IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 3, 2, ts->Conv_9, ts->Conv_9_bs);
	// save_bin(ts->mem_blk0); // 10 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_10);
	// save_bin(ts->mem_blk0); // 11 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 64, 1, 1, ts->Conv_11, ts->Conv_11_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk1); // 12 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 128, 1, 1, ts->Conv_12, ts->Conv_12_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk0); // 13 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_13);
	// save_bin(ts->mem_blk0); // 14 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(ts->mem_blk3); // 15 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 128, 3, 1, ts->Conv_15, ts->Conv_15_bs);
	// save_bin(ts->mem_blk2); // 16 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk2, ts->PRelu_16);
	// save_bin(ts->mem_blk2); // 17 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 64, 1, 1, ts->Conv_17, ts->Conv_17_bs, (float*)ts->mem_blk3);
	// save_bin(ts->mem_blk0); // 18 
	IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 19 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 1, 1, ts->Conv_19, ts->Conv_19_bs, (float*)ts->mem_blk1);
	// save_bin(ts->mem_blk0); // 20 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_20);
	// save_bin(ts->mem_blk0); // 21 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(ts->mem_blk3); // 22 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 128, 3, 1, ts->Conv_22, ts->Conv_22_bs);
	// save_bin(ts->mem_blk1); // 23 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_23);
	// save_bin(ts->mem_blk1); // 24 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_24, ts->Conv_24_bs, (float*)ts->mem_blk3);
	// save_bin(ts->mem_blk0); // 25 
	IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 26 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 128, 1, 1, ts->Conv_26, ts->Conv_26_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk0); // 27 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_27);
	// save_bin(ts->mem_blk0); // 28 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(ts->mem_blk3); // 29 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 128, 3, 1, ts->Conv_29, ts->Conv_29_bs);
	// save_bin(ts->mem_blk2); // 30 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk2, ts->PRelu_30);
	// save_bin(ts->mem_blk2); // 31 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 64, 1, 1, ts->Conv_31, ts->Conv_31_bs, (float*)ts->mem_blk3);
	// save_bin(ts->mem_blk0); // 32 
	IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 33 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 1, 1, ts->Conv_33, ts->Conv_33_bs, (float*)ts->mem_blk1);
	// save_bin(ts->mem_blk0); // 34 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_34);
	// save_bin(ts->mem_blk0); // 35 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(ts->mem_blk3); // 36 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 128, 3, 1, ts->Conv_36, ts->Conv_36_bs);
	// save_bin(ts->mem_blk1); // 37 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_37);
	// save_bin(ts->mem_blk1); // 38 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_38, ts->Conv_38_bs, (float*)ts->mem_blk3);
	// save_bin(ts->mem_blk0); // 39 
	IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 40 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 256, 1, 1, ts->Conv_40, ts->Conv_40_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk0); // 41 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_41);
	// save_bin(ts->mem_blk0); // 42 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
	// save_bin(ts->mem_blk2); // 43 
	IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk1, 256, 3, 2, ts->Conv_43, ts->Conv_43_bs);
	// save_bin(ts->mem_blk1); // 44 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_44);
	// save_bin(ts->mem_blk1); // 45 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 128, 1, 1, ts->Conv_45, ts->Conv_45_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk0); // 46 
	IF_FLAG_STOP; enn_pooling_global_aver_pack4_neon(ts->mem_blk0, ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 47 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 48 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk1, ts->mem_blk2, 32, ts->Conv_48, ts->Conv_48_bs);
	// save_bin(ts->mem_blk2); // 49 
	IF_FLAG_STOP; enn_relu(ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 50 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk2, ts->mem_blk1, 128, ts->Conv_50, ts->Conv_50_bs);
	// save_bin(ts->mem_blk1); // 51 
	IF_FLAG_STOP; enn_sigmoid(ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 52 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 53 
	IF_FLAG_STOP; enn_multiply(ts->mem_blk0, ts->mem_blk1, ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 54 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 256, 1, 1, ts->Conv_53, ts->Conv_53_bs, (float*)ts->mem_blk1);
	// save_bin(ts->mem_blk0); // 55 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_54);
	// save_bin(ts->mem_blk0); // 56 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(ts->mem_blk3); // 57 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 256, 3, 1, ts->Conv_56, ts->Conv_56_bs);
	// save_bin(ts->mem_blk1); // 58 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_57);
	// save_bin(ts->mem_blk1); // 59 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 128, 1, 1, ts->Conv_58, ts->Conv_58_bs, (float*)ts->mem_blk3);
	// save_bin(ts->mem_blk0); // 60 
	IF_FLAG_STOP; enn_pooling_global_aver_pack4_neon(ts->mem_blk0, ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 61 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 62 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk1, ts->mem_blk3, 32, ts->Conv_61, ts->Conv_61_bs);
	// save_bin(ts->mem_blk3); // 63 
	IF_FLAG_STOP; enn_relu(ts->mem_blk3);
	// save_bin(ts->mem_blk3); // 64 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk3, ts->mem_blk1, 128, ts->Conv_63, ts->Conv_63_bs);
	// save_bin(ts->mem_blk1); // 65 
	IF_FLAG_STOP; enn_sigmoid(ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 66 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 67 
	IF_FLAG_STOP; enn_multiply(ts->mem_blk0, ts->mem_blk1, ts->mem_blk3);
	// save_bin(ts->mem_blk3); // 68 
	IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk3, ts->mem_blk0);
	// save_bin(ts->mem_blk0); // 69 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 256, 1, 1, ts->Conv_67, ts->Conv_67_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk1); // 70 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_68);
	// save_bin(ts->mem_blk1); // 71 
	IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk3, 1);
	// save_bin(ts->mem_blk3); // 72 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 256, 3, 1, ts->Conv_70, ts->Conv_70_bs);
	// save_bin(ts->mem_blk2); // 73 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk2, ts->PRelu_71);
	// save_bin(ts->mem_blk2); // 74 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk1, 128, 1, 1, ts->Conv_72, ts->Conv_72_bs, (float*)ts->mem_blk3);
	// save_bin(ts->mem_blk1); // 75 
	IF_FLAG_STOP; enn_pooling_global_aver_pack4_neon(ts->mem_blk1, ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 76 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 77 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk2, ts->mem_blk3, 32, ts->Conv_75, ts->Conv_75_bs);
	// save_bin(ts->mem_blk3); // 78 
	IF_FLAG_STOP; enn_relu(ts->mem_blk3);
	// save_bin(ts->mem_blk3); // 79 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk3, ts->mem_blk2, 128, ts->Conv_77, ts->Conv_77_bs);
	// save_bin(ts->mem_blk2); // 80 
	IF_FLAG_STOP; enn_sigmoid(ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 81 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 82 
	IF_FLAG_STOP; enn_multiply(ts->mem_blk1, ts->mem_blk2, ts->mem_blk3);
	// save_bin(ts->mem_blk3); // 83 
	IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk0, ts->mem_blk3, ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 84 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 256, 1, 1, ts->Conv_81, ts->Conv_81_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk0); // 85 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_82);
	// save_bin(ts->mem_blk0); // 86 
	IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
	// save_bin(ts->mem_blk3); // 87 
	IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 256, 3, 1, ts->Conv_84, ts->Conv_84_bs);
	// save_bin(ts->mem_blk2); // 88 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk2, ts->PRelu_85);
	// save_bin(ts->mem_blk2); // 89 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 1, 1, ts->Conv_86, ts->Conv_86_bs, (float*)ts->mem_blk3);
	// save_bin(ts->mem_blk0); // 90 
	IF_FLAG_STOP; enn_pooling_global_aver_pack4_neon(ts->mem_blk0, ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 91 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 92 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk2, ts->mem_blk3, 32, ts->Conv_89, ts->Conv_89_bs);
	// save_bin(ts->mem_blk3); // 93 
	IF_FLAG_STOP; enn_relu(ts->mem_blk3);
	// save_bin(ts->mem_blk3); // 94 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk3, ts->mem_blk2, 128, ts->Conv_91, ts->Conv_91_bs);
	// save_bin(ts->mem_blk2); // 95 
	IF_FLAG_STOP; enn_sigmoid(ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 96 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk2);
	// save_bin(ts->mem_blk2); // 97 
	IF_FLAG_STOP; enn_multiply(ts->mem_blk0, ts->mem_blk2, ts->mem_blk3);
	// save_bin(ts->mem_blk3); // 98 
	IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk3, ts->mem_blk0);
	// save_bin(ts->mem_blk0); // 99 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 512, 1, 1, ts->Conv_95, ts->Conv_95_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk1); // 100 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk1, ts->PRelu_96);
	// save_bin(ts->mem_blk1); // 101 
	IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
	// save_bin(ts->mem_blk2); // 102 
	IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk0, 512, 3, 2, ts->Conv_98, ts->Conv_98_bs);
	// save_bin(ts->mem_blk0); // 103 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_99);
	// save_bin(ts->mem_blk0); // 104 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 128, 1, 1, ts->Conv_100, ts->Conv_100_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk1); // 105 
	IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 512, 1, 1, ts->Conv_101, ts->Conv_101_bs, (float*)ts->mem_blk2);
	// save_bin(ts->mem_blk0); // 106 
	IF_FLAG_STOP; enn_prelu(ts->mem_blk0, ts->PRelu_102);
	// save_bin(ts->mem_blk0); // 107 
	IF_FLAG_STOP; enn_convdw_gdc_pack4_neon(ts->mem_blk0, ts->mem_blk1, 512, 8, 1, ts->Conv_103, ts->Conv_103_bs);
	// save_bin(ts->mem_blk1); // 108 
	IF_FLAG_STOP; enn_flatten(ts->mem_blk1);
	// save_bin(ts->mem_blk1); // 109 
	IF_FLAG_STOP; enn_innerproduct(ts->mem_blk1, ts->mem_blk0, 512, ts->MatMul_105, ts->MatMul_105_bs);
	// save_bin(ts->mem_blk0); // 110 
	IF_FLAG_STOP; enn_slice_eltwise(ts->mem_blk0, ts->mem_blk1);

	return (float*)ts->mem_blk1->mem;
}
