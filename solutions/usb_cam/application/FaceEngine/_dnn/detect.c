
// VERSION MNLite
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

void Detect_Detect(Detect* ts)
{
    ts->dic_data = 0;
    ts->mem_data = 0;
    ts->mem_blk0 = 0;
    ts->mem_blk1 = 0;
    ts->mem_blk2 = 0;
    ts->mem_blk3 = 0;
    ts->mem_blk4 = 0;
    ts->mem_blk5 = 0;
    ts->mem_blk6 = 0;
    ts->mem_blk7 = 0;
    ts->g_nEngineLoaded = 0;
}

void Detect_Detect_(Detect* ts)
{
    Detect_dnn_free(ts);
}

int Detect_dnn_dic_size()
{
    return 90496;
}

int Detect_dnn_mem_size()
{
    //return 921616 + 1228816 + 1264912 + 325392 + 258064 + 76816 + 96272 + 30736 + 38416 + 11280; // (240 x 320)
    return 248848 + 331792 + 351248 + 92944 + 79888 + 24592 + 32784 + 17936 + 15376 + 5136; // (108 x 192)
}

int Detect_dnn_create(Detect* ts, const char* fn, unsigned char* pMemBuf) // unsigned char* pMemBuf = 0
{
    if (ts->g_nEngineLoaded) return 0;
    int nDicSize = Detect_dnn_dic_size();

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

    int ret = Detect_dnn_create_(ts, local_dicdata, nDicSize, pMemBuf);
    if (ret)
    {
        enn_aligned_free(local_dicdata);
        return 1;
    }

    ts->dic_data = local_dicdata;
    ts->g_nEngineLoaded = 1;

    return 0;
}

int Detect_dnn_create_(Detect* ts, unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf) // unsigned char* pMemBuf = 0
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
        mem_all = (unsigned char*)enn_aligned_malloc(Detect_dnn_mem_size(), 16);
        if (!mem_all) return 1;
        ts->mem_data = mem_all;
    }
    ts->dic_data = 0;

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
    ts->mem_blk0 = (enn_blob*)mem_all; mem_all += 248848;
    ts->mem_blk1 = (enn_blob*)mem_all; mem_all += 331792;
    ts->mem_blk2 = (enn_blob*)mem_all; mem_all += 351248;
    ts->mem_blk3 = (enn_blob*)mem_all; mem_all += 92944;
    ts->mem_blk4 = (enn_blob*)mem_all; mem_all += 79888;
    ts->mem_blk5 = (enn_blob*)mem_all; mem_all += 24592;
    ts->mem_blk6 = (enn_blob*)mem_all; mem_all += 32784;
    ts->mem_blk7 = (enn_blob*)mem_all; mem_all += 17936;
    ts->mem_blk8 = (enn_blob*)mem_all; mem_all += 15376;
    ts->mem_blk9 = (enn_blob*)mem_all;

    ts->Conv_0 = (unsigned short*)(gdata + 0); // [216]
    ts->Conv_0_bs = (unsigned short*)(gdata + 432); // [8]
    ts->Conv_2 = (unsigned short*)(gdata + 448); // [72]
    ts->Conv_2_bs = (unsigned short*)(gdata + 592); // [8]
    ts->Conv_4 = (unsigned short*)(gdata + 608); // [128]
    ts->Conv_4_bs = (unsigned short*)(gdata + 864); // [16]
    ts->Conv_6 = (unsigned short*)(gdata + 896); // [144]
    ts->Conv_6_bs = (unsigned short*)(gdata + 1184); // [16]
    ts->Conv_8 = (unsigned short*)(gdata + 1216); // [128]
    ts->Conv_8_bs = (unsigned short*)(gdata + 1472); // [8]
    ts->Conv_9 = (unsigned short*)(gdata + 1488); // [128]
    ts->Conv_9_bs = (unsigned short*)(gdata + 1744); // [16]
    ts->Conv_11 = (unsigned short*)(gdata + 1776); // [144]
    ts->Conv_11_bs = (unsigned short*)(gdata + 2064); // [16]
    ts->Conv_13 = (unsigned short*)(gdata + 2096); // [128]
    ts->Conv_13_bs = (unsigned short*)(gdata + 2352); // [8]
    ts->Conv_15 = (unsigned short*)(gdata + 2368); // [256]
    ts->Conv_15_bs = (unsigned short*)(gdata + 2880); // [32]
    ts->Conv_17 = (unsigned short*)(gdata + 2944); // [288]
    ts->Conv_17_bs = (unsigned short*)(gdata + 3520); // [32]
    ts->Conv_19 = (unsigned short*)(gdata + 3584); // [512]
    ts->Conv_19_bs = (unsigned short*)(gdata + 4608); // [16]
    ts->Conv_20 = (unsigned short*)(gdata + 4640); // [512]
    ts->Conv_20_bs = (unsigned short*)(gdata + 5664); // [32]
    ts->Conv_22 = (unsigned short*)(gdata + 5728); // [288]
    ts->Conv_22_bs = (unsigned short*)(gdata + 6304); // [32]
    ts->Conv_24 = (unsigned short*)(gdata + 6368); // [512]
    ts->Conv_24_bs = (unsigned short*)(gdata + 7392); // [16]
    ts->Conv_26 = (unsigned short*)(gdata + 7424); // [512]
    ts->Conv_26_bs = (unsigned short*)(gdata + 8448); // [32]
    ts->Conv_28 = (unsigned short*)(gdata + 8512); // [288]
    ts->Conv_28_bs = (unsigned short*)(gdata + 9088); // [32]
    ts->Conv_30 = (unsigned short*)(gdata + 9152); // [512]
    ts->Conv_30_bs = (unsigned short*)(gdata + 10176); // [16]
    ts->Conv_32 = (unsigned short*)(gdata + 10208); // [512]
    ts->Conv_32_bs = (unsigned short*)(gdata + 11232); // [32]
    ts->Conv_34 = (unsigned short*)(gdata + 11296); // [288]
    ts->Conv_34_bs = (unsigned short*)(gdata + 11872); // [32]
    ts->Conv_36 = (unsigned short*)(gdata + 11936); // [512]
    ts->Conv_36_bs = (unsigned short*)(gdata + 12960); // [16]
    ts->Conv_38 = (unsigned short*)(gdata + 12992); // [144]
    ts->Conv_38_bs = (unsigned short*)(gdata + 13280); // [16]
    ts->Conv_40 = (unsigned short*)(gdata + 13312); // [192]
    ts->Conv_40_bs = (unsigned short*)(gdata + 13696); // [8]
    ts->Conv_43 = (unsigned short*)(gdata + 13712); // [144]
    ts->Conv_43_bs = (unsigned short*)(gdata + 14000); // [16]
    ts->Conv_45 = (unsigned short*)(gdata + 14032); // [192]
    ts->Conv_45_bs = (unsigned short*)(gdata + 14416); // [12]
    ts->Conv_48 = (unsigned short*)(gdata + 14440); // [768]
    ts->Conv_48_bs = (unsigned short*)(gdata + 15976); // [48]
    ts->Conv_50 = (unsigned short*)(gdata + 16072); // [432]
    ts->Conv_50_bs = (unsigned short*)(gdata + 16936); // [48]
    ts->Conv_52 = (unsigned short*)(gdata + 17032); // [1536]
    ts->Conv_52_bs = (unsigned short*)(gdata + 20104); // [32]
    ts->Conv_53 = (unsigned short*)(gdata + 20168); // [1536]
    ts->Conv_53_bs = (unsigned short*)(gdata + 23240); // [48]
    ts->Conv_55 = (unsigned short*)(gdata + 23336); // [432]
    ts->Conv_55_bs = (unsigned short*)(gdata + 24200); // [48]
    ts->Conv_57 = (unsigned short*)(gdata + 24296); // [1536]
    ts->Conv_57_bs = (unsigned short*)(gdata + 27368); // [32]
    ts->Conv_59 = (unsigned short*)(gdata + 27432); // [1536]
    ts->Conv_59_bs = (unsigned short*)(gdata + 30504); // [48]
    ts->Conv_61 = (unsigned short*)(gdata + 30600); // [432]
    ts->Conv_61_bs = (unsigned short*)(gdata + 31464); // [48]
    ts->Conv_63 = (unsigned short*)(gdata + 31560); // [1536]
    ts->Conv_63_bs = (unsigned short*)(gdata + 34632); // [32]
    ts->Conv_65 = (unsigned short*)(gdata + 34696); // [288]
    ts->Conv_65_bs = (unsigned short*)(gdata + 35272); // [32]
    ts->Conv_67 = (unsigned short*)(gdata + 35336); // [128]
    ts->Conv_67_bs = (unsigned short*)(gdata + 35592); // [4]
    ts->Conv_70 = (unsigned short*)(gdata + 35600); // [288]
    ts->Conv_70_bs = (unsigned short*)(gdata + 36176); // [32]
    ts->Conv_72 = (unsigned short*)(gdata + 36240); // [256]
    ts->Conv_72_bs = (unsigned short*)(gdata + 36752); // [8]
    ts->Conv_75 = (unsigned short*)(gdata + 36768); // [2048]
    ts->Conv_75_bs = (unsigned short*)(gdata + 40864); // [64]
    ts->Conv_77 = (unsigned short*)(gdata + 40992); // [576]
    ts->Conv_77_bs = (unsigned short*)(gdata + 42144); // [64]
    ts->Conv_79 = (unsigned short*)(gdata + 42272); // [3072]
    ts->Conv_79_bs = (unsigned short*)(gdata + 48416); // [48]
    ts->Conv_80 = (unsigned short*)(gdata + 48512); // [3072]
    ts->Conv_80_bs = (unsigned short*)(gdata + 54656); // [64]
    ts->Conv_82 = (unsigned short*)(gdata + 54784); // [576]
    ts->Conv_82_bs = (unsigned short*)(gdata + 55936); // [64]
    ts->Conv_84 = (unsigned short*)(gdata + 56064); // [3072]
    ts->Conv_84_bs = (unsigned short*)(gdata + 62208); // [48]
    ts->Conv_86 = (unsigned short*)(gdata + 62304); // [432]
    ts->Conv_86_bs = (unsigned short*)(gdata + 63168); // [48]
    ts->Conv_88 = (unsigned short*)(gdata + 63264); // [576]
    ts->Conv_88_bs = (unsigned short*)(gdata + 64416); // [8]
    ts->Conv_91 = (unsigned short*)(gdata + 64432); // [432]
    ts->Conv_91_bs = (unsigned short*)(gdata + 65296); // [48]
    ts->Conv_93 = (unsigned short*)(gdata + 65392); // [576]
    ts->Conv_93_bs = (unsigned short*)(gdata + 66544); // [12]
    ts->Conv_96 = (unsigned short*)(gdata + 66568); // [3840]
    ts->Conv_96_bs = (unsigned short*)(gdata + 74248); // [80]
    ts->Conv_98 = (unsigned short*)(gdata + 74408); // [720]
    ts->Conv_98_bs = (unsigned short*)(gdata + 75848); // [80]
    ts->Conv_100 = (unsigned short*)(gdata + 76008); // [5120]
    ts->Conv_100_bs = (unsigned short*)(gdata + 86248); // [64]
    ts->Conv_101 = (unsigned short*)(gdata + 86376); // [576]
    ts->Conv_101_bs = (unsigned short*)(gdata + 87528); // [64]
    ts->Conv_103 = (unsigned short*)(gdata + 87656); // [256]
    ts->Conv_103_bs = (unsigned short*)(gdata + 88168); // [4]
    ts->Conv_106 = (unsigned short*)(gdata + 88176); // [576]
    ts->Conv_106_bs = (unsigned short*)(gdata + 89328); // [64]
    ts->Conv_108 = (unsigned short*)(gdata + 89456); // [512]
    ts->Conv_108_bs = (unsigned short*)(gdata + 90480); // [8]

    ts->g_nEngineLoaded = 1;

    return 0;
}

void Detect_dnn_free(Detect* ts)
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

int Detect_getEngineLoaded(Detect* ts)
{
    return ts->g_nEngineLoaded;
}

// void save_bin(enn_blob* blob);
// void save_bin_(float* blob, int sz);

int Detect_dnn_forward(Detect* ts, unsigned char* in, int width, int height, float** pprScore, float** pprBox, int isSimple) // bool isSimple = false
{
    IF_FLAG_STOP; enn_trans(in, width, height, 1, ts->mem_blk0, 3, -128, 0.0078125f);
    // save_bin(ts->mem_blk0); // 0
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
    // save_bin(ts->mem_blk2); // 1
    IF_FLAG_STOP; enn_conv3x3s2_pack1to4_neon(ts->mem_blk2, ts->mem_blk1, 8, 3, 2, ts->Conv_0, ts->Conv_0_bs);
    // save_bin(ts->mem_blk1); // 2
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(ts->mem_blk1); // 3
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
    // save_bin(ts->mem_blk2); // 4
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk2, ts->mem_blk0, 8, 3, 1, ts->Conv_2, ts->Conv_2_bs);
    // save_bin(ts->mem_blk0); // 5
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(ts->mem_blk0); // 6
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 16, 1, 1, ts->Conv_4, ts->Conv_4_bs, (float*)ts->mem_blk2);
    // save_bin(ts->mem_blk1); // 7
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(ts->mem_blk1); // 8
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
    // save_bin(ts->mem_blk2); // 9
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk0, 16, 3, 2, ts->Conv_6, ts->Conv_6_bs);
    // save_bin(ts->mem_blk0); // 10
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(ts->mem_blk0); // 11
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 8, 1, 1, ts->Conv_8, ts->Conv_8_bs, (float*)ts->mem_blk2);
    // save_bin(ts->mem_blk1); // 12
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 16, 1, 1, ts->Conv_9, ts->Conv_9_bs, (float*)ts->mem_blk2);
    // save_bin(ts->mem_blk0); // 13
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(ts->mem_blk0); // 14
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(ts->mem_blk3); // 15
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 16, 3, 1, ts->Conv_11, ts->Conv_11_bs);
    // save_bin(ts->mem_blk2); // 16
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 17
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 8, 1, 1, ts->Conv_13, ts->Conv_13_bs, (float*)ts->mem_blk3);
    // save_bin(ts->mem_blk0); // 18
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 19
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 32, 1, 1, ts->Conv_15, ts->Conv_15_bs, (float*)ts->mem_blk1);
    // save_bin(ts->mem_blk0); // 20
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(ts->mem_blk0); // 21
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
    // save_bin(ts->mem_blk2); // 22
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk1, 32, 3, 2, ts->Conv_17, ts->Conv_17_bs);
    // save_bin(ts->mem_blk1); // 23
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(ts->mem_blk1); // 24
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 16, 1, 1, ts->Conv_19, ts->Conv_19_bs, (float*)ts->mem_blk2);
    // save_bin(ts->mem_blk0); // 25
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 32, 1, 1, ts->Conv_20, ts->Conv_20_bs, (float*)ts->mem_blk2);
    // save_bin(ts->mem_blk1); // 26
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(ts->mem_blk1); // 27
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk3, 1);
    // save_bin(ts->mem_blk3); // 28
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 32, 3, 1, ts->Conv_22, ts->Conv_22_bs);
    // save_bin(ts->mem_blk2); // 29
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 30
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk1, 16, 1, 1, ts->Conv_24, ts->Conv_24_bs, (float*)ts->mem_blk3);
    // save_bin(ts->mem_blk1); // 31
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk0, ts->mem_blk1, ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 32
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 32, 1, 1, ts->Conv_26, ts->Conv_26_bs, (float*)ts->mem_blk1);
    // save_bin(ts->mem_blk0); // 33
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(ts->mem_blk0); // 34
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(ts->mem_blk3); // 35
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 32, 3, 1, ts->Conv_28, ts->Conv_28_bs);
    // save_bin(ts->mem_blk1); // 36
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(ts->mem_blk1); // 37
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 16, 1, 1, ts->Conv_30, ts->Conv_30_bs, (float*)ts->mem_blk3);
    // save_bin(ts->mem_blk0); // 38
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
    // save_bin(ts->mem_blk1); // 39
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 32, 1, 1, ts->Conv_32, ts->Conv_32_bs, (float*)ts->mem_blk2);
    // save_bin(ts->mem_blk0); // 40
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(ts->mem_blk0); // 41
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(ts->mem_blk3); // 42
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 32, 3, 1, ts->Conv_34, ts->Conv_34_bs);
    // save_bin(ts->mem_blk2); // 43
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 44
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 16, 1, 1, ts->Conv_36, ts->Conv_36_bs, (float*)ts->mem_blk3);
    // save_bin(ts->mem_blk0); // 45
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 46
    IF_FLAG_STOP; enn_padding(ts->mem_blk2, ts->mem_blk1, 1);
    // save_bin(ts->mem_blk1); // 47
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk1, ts->mem_blk0, 16, 3, 1, ts->Conv_38, ts->Conv_38_bs);
    // save_bin(ts->mem_blk0); // 48
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(ts->mem_blk0); // 49
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4to1_neon(ts->mem_blk0, ts->mem_blk1, 6, 1, 1, ts->Conv_40, ts->Conv_40_bs, (float*)ts->mem_blk3);
    // save_bin(ts->mem_blk1); // 50
    IF_FLAG_STOP;
    int p0 = enn_permute_(ts->mem_blk1, (float*)ts->mem_blk0);
    // save_bin((float*)mem_blk0, p0); // 51
    IF_FLAG_STOP; enn_padding(ts->mem_blk2, ts->mem_blk3, 1);
    // save_bin(ts->mem_blk3); // 52
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 16, 3, 1, ts->Conv_43, ts->Conv_43_bs);
    // save_bin(ts->mem_blk1); // 53
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(ts->mem_blk1); // 54
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk3, 12, 1, 1, ts->Conv_45, ts->Conv_45_bs, (float*)ts->mem_blk4);
    // save_bin(ts->mem_blk3); // 55
    IF_FLAG_STOP;
    int p1 = enn_permute_(ts->mem_blk3, (float*)ts->mem_blk1);
    // save_bin((float*)mem_blk1, p1); // 56
    if (isSimple)
    {
        enn_softmax((float*)ts->mem_blk0, p0 / 2);
        *pprScore = (float*)ts->mem_blk0;
        *pprBox = (float*)ts->mem_blk1;
        return p0 / 2;
    }
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk3, 48, 1, 1, ts->Conv_48, ts->Conv_48_bs, (float*)ts->mem_blk4);
    // save_bin(ts->mem_blk3); // 57
    IF_FLAG_STOP; enn_relu(ts->mem_blk3);
    // save_bin(ts->mem_blk3); // 58
    IF_FLAG_STOP; enn_padding(ts->mem_blk3, ts->mem_blk4, 1);
    // save_bin(ts->mem_blk4); // 59
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk4, ts->mem_blk2, 48, 3, 2, ts->Conv_50, ts->Conv_50_bs);
    // save_bin(ts->mem_blk2); // 60
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 61
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk3, 32, 1, 1, ts->Conv_52, ts->Conv_52_bs, (float*)ts->mem_blk4);
    // save_bin(ts->mem_blk3); // 62
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk3, ts->mem_blk2, 48, 1, 1, ts->Conv_53, ts->Conv_53_bs, (float*)ts->mem_blk4);
    // save_bin(ts->mem_blk2); // 63
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 64
    IF_FLAG_STOP; enn_padding(ts->mem_blk2, ts->mem_blk5, 1);
    // save_bin(ts->mem_blk5); // 65
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk5, ts->mem_blk4, 48, 3, 1, ts->Conv_55, ts->Conv_55_bs);
    // save_bin(ts->mem_blk4); // 66
    IF_FLAG_STOP; enn_relu(ts->mem_blk4);
    // save_bin(ts->mem_blk4); // 67
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk4, ts->mem_blk2, 32, 1, 1, ts->Conv_57, ts->Conv_57_bs, (float*)ts->mem_blk5);
    // save_bin(ts->mem_blk2); // 68
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk3, ts->mem_blk2, ts->mem_blk4);
    // save_bin(ts->mem_blk4); // 69
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk4, ts->mem_blk2, 48, 1, 1, ts->Conv_59, ts->Conv_59_bs, (float*)ts->mem_blk3);
    // save_bin(ts->mem_blk2); // 70
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 71
    IF_FLAG_STOP; enn_padding(ts->mem_blk2, ts->mem_blk5, 1);
    // save_bin(ts->mem_blk5); // 72
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk5, ts->mem_blk3, 48, 3, 1, ts->Conv_61, ts->Conv_61_bs);
    // save_bin(ts->mem_blk3); // 73
    IF_FLAG_STOP; enn_relu(ts->mem_blk3);
    // save_bin(ts->mem_blk3); // 74
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk3, ts->mem_blk2, 32, 1, 1, ts->Conv_63, ts->Conv_63_bs, (float*)ts->mem_blk5);
    // save_bin(ts->mem_blk2); // 75
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk4, ts->mem_blk2, ts->mem_blk3);
    // save_bin(ts->mem_blk3); // 76
    IF_FLAG_STOP; enn_padding(ts->mem_blk3, ts->mem_blk4, 1);
    // save_bin(ts->mem_blk4); // 77
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk4, ts->mem_blk2, 32, 3, 1, ts->Conv_65, ts->Conv_65_bs);
    // save_bin(ts->mem_blk2); // 78
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(ts->mem_blk2); // 79
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk4, 4, 1, 1, ts->Conv_67, ts->Conv_67_bs, (float*)ts->mem_blk5);
    // save_bin(ts->mem_blk4); // 80
    IF_FLAG_STOP;
    int p2 = enn_permute_(ts->mem_blk4, ((float*)ts->mem_blk0) + p0);
    // save_bin(((float*)mem_blk0) + p0, p2); // 81
    IF_FLAG_STOP; enn_padding(ts->mem_blk3, ts->mem_blk5, 1);
    // save_bin(ts->mem_blk5); // 82
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk5, ts->mem_blk4, 32, 3, 1, ts->Conv_70, ts->Conv_70_bs);
    // save_bin(ts->mem_blk4); // 83
    IF_FLAG_STOP; enn_relu(ts->mem_blk4);
    // save_bin(ts->mem_blk4); // 84
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk4, ts->mem_blk5, 8, 1, 1, ts->Conv_72, ts->Conv_72_bs, (float*)ts->mem_blk6);
    // save_bin(ts->mem_blk5); // 85
    IF_FLAG_STOP;
    int p3 = enn_permute_(ts->mem_blk5, ((float*)ts->mem_blk1) + p1);
    // save_bin(((float*)mem_blk1) + p1, p3); // 86
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk3, ts->mem_blk5, 64, 1, 1, ts->Conv_75, ts->Conv_75_bs, (float*)ts->mem_blk6);
    // save_bin(ts->mem_blk5); // 87
    IF_FLAG_STOP; enn_relu(ts->mem_blk5);
    // save_bin(ts->mem_blk5); // 88
    IF_FLAG_STOP; enn_padding(ts->mem_blk5, ts->mem_blk6, 1);
    // save_bin(ts->mem_blk6); // 89
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk6, ts->mem_blk3, 64, 3, 2, ts->Conv_77, ts->Conv_77_bs);
    // save_bin(ts->mem_blk3); // 90
    IF_FLAG_STOP; enn_relu(ts->mem_blk3);
    // save_bin(ts->mem_blk3); // 91
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk3, ts->mem_blk5, 48, 1, 1, ts->Conv_79, ts->Conv_79_bs, (float*)ts->mem_blk6);
    // save_bin(ts->mem_blk5); // 92
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk5, ts->mem_blk3, 64, 1, 1, ts->Conv_80, ts->Conv_80_bs, (float*)ts->mem_blk6);
    // save_bin(ts->mem_blk3); // 93
    IF_FLAG_STOP; enn_relu(ts->mem_blk3);
    // save_bin(ts->mem_blk3); // 94
    IF_FLAG_STOP; enn_padding(ts->mem_blk3, ts->mem_blk7, 1);
    // save_bin(ts->mem_blk7); // 95
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk7, ts->mem_blk6, 64, 3, 1, ts->Conv_82, ts->Conv_82_bs);
    // save_bin(ts->mem_blk6); // 96
    IF_FLAG_STOP; enn_relu(ts->mem_blk6);
    // save_bin(ts->mem_blk6); // 97
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk6, ts->mem_blk3, 48, 1, 1, ts->Conv_84, ts->Conv_84_bs, (float*)ts->mem_blk7);
    // save_bin(ts->mem_blk3); // 98
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk5, ts->mem_blk3, ts->mem_blk6);
    // save_bin(ts->mem_blk6); // 99
    IF_FLAG_STOP; enn_padding(ts->mem_blk6, ts->mem_blk5, 1);
    // save_bin(ts->mem_blk5); // 100
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk5, ts->mem_blk3, 48, 3, 1, ts->Conv_86, ts->Conv_86_bs);
    // save_bin(ts->mem_blk3); // 101
    IF_FLAG_STOP; enn_relu(ts->mem_blk3);
    // save_bin(ts->mem_blk3); // 102
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4to1_neon(ts->mem_blk3, ts->mem_blk5, 6, 1, 1, ts->Conv_88, ts->Conv_88_bs, (float*)ts->mem_blk7);
    // save_bin(ts->mem_blk5); // 103
    IF_FLAG_STOP;
    int p4 = enn_permute_(ts->mem_blk5, ((float*)ts->mem_blk0) + p0 + p2);
    // save_bin(((float*)mem_blk0) + p0 + p2, p4); // 104
    IF_FLAG_STOP; enn_padding(ts->mem_blk6, ts->mem_blk7, 1);
    // save_bin(ts->mem_blk7); // 105
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk7, ts->mem_blk5, 48, 3, 1, ts->Conv_91, ts->Conv_91_bs);
    // save_bin(ts->mem_blk5); // 106
    IF_FLAG_STOP; enn_relu(ts->mem_blk5);
    // save_bin(ts->mem_blk5); // 107
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk5, ts->mem_blk7, 12, 1, 1, ts->Conv_93, ts->Conv_93_bs, (float*)ts->mem_blk8);
    // save_bin(ts->mem_blk7); // 108
    IF_FLAG_STOP;
    int p5 = enn_permute_(ts->mem_blk7, ((float*)ts->mem_blk1) + p1 + p3);
    // save_bin(((float*)mem_blk1) + p1 + p3, p5); // 109
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk6, ts->mem_blk7, 80, 1, 1, ts->Conv_96, ts->Conv_96_bs, (float*)ts->mem_blk8);
    // save_bin(ts->mem_blk7); // 110
    IF_FLAG_STOP; enn_relu(ts->mem_blk7);
    // save_bin(ts->mem_blk7); // 111
    IF_FLAG_STOP; enn_padding(ts->mem_blk7, ts->mem_blk8, 1);
    // save_bin(ts->mem_blk8); // 112
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk8, ts->mem_blk6, 80, 3, 2, ts->Conv_98, ts->Conv_98_bs);
    // save_bin(ts->mem_blk6); // 113
    IF_FLAG_STOP; enn_relu(ts->mem_blk6);
    // save_bin(ts->mem_blk6); // 114
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk6, ts->mem_blk7, 64, 1, 1, ts->Conv_100, ts->Conv_100_bs, (float*)ts->mem_blk8);
    // save_bin(ts->mem_blk7); // 115
    IF_FLAG_STOP; enn_padding(ts->mem_blk7, ts->mem_blk8, 1);
    // save_bin(ts->mem_blk8); // 116
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk8, ts->mem_blk6, 64, 3, 1, ts->Conv_101, ts->Conv_101_bs);
    // save_bin(ts->mem_blk6); // 117
    IF_FLAG_STOP; enn_relu(ts->mem_blk6);
    // save_bin(ts->mem_blk6); // 118
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk6, ts->mem_blk8, 4, 1, 1, ts->Conv_103, ts->Conv_103_bs, (float*)ts->mem_blk9);
    // save_bin(ts->mem_blk8); // 119
    IF_FLAG_STOP;
    int p6 = enn_permute_(ts->mem_blk8, ((float*)ts->mem_blk0) + p0 + p2 + p4);
    // save_bin(((float*)mem_blk0) + p0 + p2 + p4, p6); // 120
    IF_FLAG_STOP; enn_padding(ts->mem_blk7, ts->mem_blk9, 1);
    // save_bin(ts->mem_blk9); // 121
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk9, ts->mem_blk8, 64, 3, 1, ts->Conv_106, ts->Conv_106_bs);
    // save_bin(ts->mem_blk8); // 122
    IF_FLAG_STOP; enn_relu(ts->mem_blk8);
    // save_bin(ts->mem_blk8); // 123
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk8, ts->mem_blk7, 8, 1, 1, ts->Conv_108, ts->Conv_108_bs, (float*)ts->mem_blk9);
    // save_bin(ts->mem_blk7); // 124
    IF_FLAG_STOP;
    /*int p7 = */enn_permute_(ts->mem_blk7, ((float*)ts->mem_blk1) + p1 + p3 + p5);
    // save_bin(((float*)mem_blk8) + p1 + p3 + p5, p7); // 125
    // softmax((float*)mem_blk0, (p0 + p2 + p4 + p6) / 2);
    *pprScore = (float*)ts->mem_blk0;
    *pprBox = (float*)ts->mem_blk1;

    return (p0 + p2 + p4 + p6) / 2;
}
