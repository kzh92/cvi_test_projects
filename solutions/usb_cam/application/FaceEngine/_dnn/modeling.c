
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

void Modeling_Modeling(Modeling* ts, int isHandModeling)
{
	ts->m_isHandModeling = isHandModeling;
    ts->dic_data = 0;
    ts->mem_data = 0;
    ts->g_nEngineLoaded = 0;
}

void Modeling_Modeling_(Modeling* ts)
{
    Modeling_dnn_free(ts);
}

int Modeling_dnn_dic_size(int isHandModeling)
{
	return (isHandModeling) ? 451356 : 576528;
}

int Modeling_dnn_mem_size()
{
    return 131088 + 262160 + 295952 + 82960;
}

int Modeling_dnn_create(Modeling* ts, const char* fn, unsigned char* pMemBuf) // unsigned char* pMemBuf = 0
{
    if (ts->g_nEngineLoaded) return 0;
    int nDicSize = Modeling_dnn_dic_size(ts->m_isHandModeling);

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

    int ret = Modeling_dnn_create_(ts, local_dicdata, nDicSize, pMemBuf);
    if (ret)
    {
        enn_aligned_free(local_dicdata);
        return 1;
    }

    ts->dic_data = local_dicdata;
    ts->g_nEngineLoaded = 1;

    return 0;
}

int Modeling_dnn_create_(Modeling* ts, unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf) // unsigned char* pMemBuf = 0
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
        mem_all = (unsigned char*)enn_aligned_malloc(Modeling_dnn_mem_size(), 16);
        if (!mem_all) return 1;
        ts->mem_data = mem_all;
    }
    ts->dic_data = 0;

    unsigned char* gdata = pDicData;

    ts->mem_blk0 = (enn_blob*)mem_all; mem_all += 131088;
    ts->mem_blk1 = (enn_blob*)mem_all; mem_all += 262160;
    ts->mem_blk2 = (enn_blob*)mem_all; mem_all += 295952;
    ts->mem_blk3 = (enn_blob*)mem_all;

    ts->Conv_0 = (unsigned short*)(gdata + 0); // [288]
    ts->Conv_0_bs = (unsigned short*)(gdata + 576); // [32]
    ts->Conv_2 = (unsigned short*)(gdata + 640); // [288]
    ts->Conv_2_bs = (unsigned short*)(gdata + 1216); // [32]
    ts->Conv_4 = (unsigned short*)(gdata + 1280); // [2048]
    ts->Conv_4_bs = (unsigned short*)(gdata + 5376); // [64]
    ts->Conv_6 = (unsigned short*)(gdata + 5504); // [576]
    ts->Conv_6_bs = (unsigned short*)(gdata + 6656); // [64]
    ts->Conv_8 = (unsigned short*)(gdata + 6784); // [2048]
    ts->Conv_8_bs = (unsigned short*)(gdata + 10880); // [32]
    ts->Conv_9 = (unsigned short*)(gdata + 10944); // [2048]
    ts->Conv_9_bs = (unsigned short*)(gdata + 15040); // [64]
    ts->Conv_11 = (unsigned short*)(gdata + 15168); // [576]
    ts->Conv_11_bs = (unsigned short*)(gdata + 16320); // [64]
    ts->Conv_13 = (unsigned short*)(gdata + 16448); // [2048]
    ts->Conv_13_bs = (unsigned short*)(gdata + 20544); // [32]
    ts->Conv_15 = (unsigned short*)(gdata + 20608); // [2048]
    ts->Conv_15_bs = (unsigned short*)(gdata + 24704); // [64]
    ts->Conv_17 = (unsigned short*)(gdata + 24832); // [576]
    ts->Conv_17_bs = (unsigned short*)(gdata + 25984); // [64]
    ts->Conv_19 = (unsigned short*)(gdata + 26112); // [2048]
    ts->Conv_19_bs = (unsigned short*)(gdata + 30208); // [32]
    ts->Conv_21 = (unsigned short*)(gdata + 30272); // [2048]
    ts->Conv_21_bs = (unsigned short*)(gdata + 34368); // [64]
    ts->Conv_23 = (unsigned short*)(gdata + 34496); // [576]
    ts->Conv_23_bs = (unsigned short*)(gdata + 35648); // [64]
    ts->Conv_25 = (unsigned short*)(gdata + 35776); // [2048]
    ts->Conv_25_bs = (unsigned short*)(gdata + 39872); // [32]
    ts->Conv_27 = (unsigned short*)(gdata + 39936); // [4096]
    ts->Conv_27_bs = (unsigned short*)(gdata + 48128); // [128]
    ts->Conv_29 = (unsigned short*)(gdata + 48384); // [1152]
    ts->Conv_29_bs = (unsigned short*)(gdata + 50688); // [128]
    ts->Conv_31 = (unsigned short*)(gdata + 50944); // [8192]
    ts->Conv_31_bs = (unsigned short*)(gdata + 67328); // [64]
    ts->Conv_32 = (unsigned short*)(gdata + 67456); // [8192]
    ts->Conv_32_bs = (unsigned short*)(gdata + 83840); // [128]
    ts->Conv_34 = (unsigned short*)(gdata + 84096); // [1152]
    ts->Conv_34_bs = (unsigned short*)(gdata + 86400); // [128]
    ts->Conv_36 = (unsigned short*)(gdata + 86656); // [8192]
    ts->Conv_36_bs = (unsigned short*)(gdata + 103040); // [64]
    ts->Conv_38 = (unsigned short*)(gdata + 103168); // [8192]
    ts->Conv_38_bs = (unsigned short*)(gdata + 119552); // [128]
    ts->Conv_40 = (unsigned short*)(gdata + 119808); // [1152]
    ts->Conv_40_bs = (unsigned short*)(gdata + 122112); // [128]
    ts->Conv_42 = (unsigned short*)(gdata + 122368); // [8192]
    ts->Conv_42_bs = (unsigned short*)(gdata + 138752); // [64]
    ts->Conv_44 = (unsigned short*)(gdata + 138880); // [8192]
    ts->Conv_44_bs = (unsigned short*)(gdata + 155264); // [128]
    ts->Conv_46 = (unsigned short*)(gdata + 155520); // [1152]
    ts->Conv_46_bs = (unsigned short*)(gdata + 157824); // [128]
    ts->Conv_48 = (unsigned short*)(gdata + 158080); // [8192]
    ts->Conv_48_bs = (unsigned short*)(gdata + 174464); // [64]
    ts->Conv_50 = (unsigned short*)(gdata + 174592); // [8192]
    ts->Conv_50_bs = (unsigned short*)(gdata + 190976); // [128]
    ts->Conv_52 = (unsigned short*)(gdata + 191232); // [1152]
    ts->Conv_52_bs = (unsigned short*)(gdata + 193536); // [128]
    ts->Conv_54 = (unsigned short*)(gdata + 193792); // [8192]
    ts->Conv_54_bs = (unsigned short*)(gdata + 210176); // [64]
    ts->Conv_56 = (unsigned short*)(gdata + 210304); // [16384]
    ts->Conv_56_bs = (unsigned short*)(gdata + 243072); // [256]
    ts->Conv_58 = (unsigned short*)(gdata + 243584); // [2304]
    ts->Conv_58_bs = (unsigned short*)(gdata + 248192); // [256]
    ts->Conv_60 = (unsigned short*)(gdata + 248704); // [16384]
    ts->Conv_60_bs = (unsigned short*)(gdata + 281472); // [64]
    ts->Conv_61 = (unsigned short*)(gdata + 281600); // [8192]
    ts->Conv_61_bs = (unsigned short*)(gdata + 297984); // [128]
    ts->Conv_63 = (unsigned short*)(gdata + 298240); // [1152]
    ts->Conv_63_bs = (unsigned short*)(gdata + 300544); // [128]
    ts->Conv_65 = (unsigned short*)(gdata + 300800); // [8192]
    ts->Conv_65_bs = (unsigned short*)(gdata + 317184); // [64]
    ts->Conv_67 = (unsigned short*)(gdata + 317312); // [8192]
    ts->Conv_67_bs = (unsigned short*)(gdata + 333696); // [128]
    ts->Conv_69 = (unsigned short*)(gdata + 333952); // [1152]
    ts->Conv_69_bs = (unsigned short*)(gdata + 336256); // [128]
    ts->Conv_71 = (unsigned short*)(gdata + 336512); // [8192]
    ts->Conv_71_bs = (unsigned short*)(gdata + 352896); // [64]
    ts->Conv_73 = (unsigned short*)(gdata + 353024); // [32768]
    ts->Conv_73_bs = (unsigned short*)(gdata + 418560); // [512]
    ts->Conv_75 = (unsigned short*)(gdata + 419584); // [8192]
    ts->Conv_75_bs = (unsigned short*)(gdata + 435968); // [512]
    ts->Gemm_77 = (unsigned short*)(gdata + 436992); // [69632] // [7168]
	ts->Gemm_77_bs = (unsigned short*)(gdata + ((ts->m_isHandModeling) ? 451328 : 576256)); // [136] // [14]

    ts->g_nEngineLoaded = 1;

    return 0;
}

void Modeling_dnn_free(Modeling* ts)
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

int Modeling_getEngineLoaded(Modeling* ts)
{
    return ts->g_nEngineLoaded;
}

// void save_bin(enn_blob* blob);
// void save_bin(float* blob, int sz);

float* Modeling_dnn_forward(Modeling* ts, unsigned char* in, int width, int height)
{
    IF_FLAG_STOP; enn_trans(in, 64, 64, 1, ts->mem_blk0, 1, 0, 0.00390625f);
    // save_bin(mem_blk0); // 0
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
    // save_bin(mem_blk2); // 1
    IF_FLAG_STOP; enn_conv3x3s2_pack1to4_neon(ts->mem_blk2, ts->mem_blk1, 32, 3, 2, ts->Conv_0, ts->Conv_0_bs);
    // save_bin(mem_blk1); // 2
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 3
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
    // save_bin(mem_blk2); // 4
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk2, ts->mem_blk0, 32, 3, 1, ts->Conv_2, ts->Conv_2_bs);
    // save_bin(mem_blk0); // 5
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 6
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 64, 1, 1, ts->Conv_4, ts->Conv_4_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk1); // 7
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 8
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk2, 1);
    // save_bin(mem_blk2); // 9
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk0, 64, 3, 2, ts->Conv_6, ts->Conv_6_bs);
    // save_bin(mem_blk0); // 10
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 11
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 32, 1, 1, ts->Conv_8, ts->Conv_8_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk1); // 12
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_9, ts->Conv_9_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk0); // 13
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 14
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 15
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 64, 3, 1, ts->Conv_11, ts->Conv_11_bs);
    // save_bin(mem_blk2); // 16
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(mem_blk2); // 17
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 32, 1, 1, ts->Conv_13, ts->Conv_13_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk0); // 18
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
    // save_bin(mem_blk2); // 19
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 64, 1, 1, ts->Conv_15, ts->Conv_15_bs, (float*)ts->mem_blk1);
    // save_bin(mem_blk0); // 20
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 21
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 22
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 64, 3, 1, ts->Conv_17, ts->Conv_17_bs);
    // save_bin(mem_blk1); // 23
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 24
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 32, 1, 1, ts->Conv_19, ts->Conv_19_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk0); // 25
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
    // save_bin(mem_blk1); // 26
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_21, ts->Conv_21_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk0); // 27
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 28
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 29
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 64, 3, 1, ts->Conv_23, ts->Conv_23_bs);
    // save_bin(mem_blk2); // 30
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(mem_blk2); // 31
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 32, 1, 1, ts->Conv_25, ts->Conv_25_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk0); // 32
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
    // save_bin(mem_blk2); // 33
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 1, 1, ts->Conv_27, ts->Conv_27_bs, (float*)ts->mem_blk1);
    // save_bin(mem_blk0); // 34
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 35
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
    // save_bin(mem_blk2); // 36
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk1, 128, 3, 2, ts->Conv_29, ts->Conv_29_bs);
    // save_bin(mem_blk1); // 37
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 38
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_31, ts->Conv_31_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk0); // 39
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 128, 1, 1, ts->Conv_32, ts->Conv_32_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk1); // 40
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 41
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 42
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 128, 3, 1, ts->Conv_34, ts->Conv_34_bs);
    // save_bin(mem_blk2); // 43
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(mem_blk2); // 44
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk1, 64, 1, 1, ts->Conv_36, ts->Conv_36_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk1); // 45
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk0, ts->mem_blk1, ts->mem_blk2);
    // save_bin(mem_blk2); // 46
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 1, 1, ts->Conv_38, ts->Conv_38_bs, (float*)ts->mem_blk1);
    // save_bin(mem_blk0); // 47
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 48
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 49
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 128, 3, 1, ts->Conv_40, ts->Conv_40_bs);
    // save_bin(mem_blk1); // 50
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 51
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_42, ts->Conv_42_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk0); // 52
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
    // save_bin(mem_blk1); // 53
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 128, 1, 1, ts->Conv_44, ts->Conv_44_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk0); // 54
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 55
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 56
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 128, 3, 1, ts->Conv_46, ts->Conv_46_bs);
    // save_bin(mem_blk2); // 57
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(mem_blk2); // 58
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 64, 1, 1, ts->Conv_48, ts->Conv_48_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk0); // 59
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk1, ts->mem_blk0, ts->mem_blk2);
    // save_bin(mem_blk2); // 60
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 1, 1, ts->Conv_50, ts->Conv_50_bs, (float*)ts->mem_blk1);
    // save_bin(mem_blk0); // 61
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 62
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 63
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 128, 3, 1, ts->Conv_52, ts->Conv_52_bs);
    // save_bin(mem_blk1); // 64
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 65
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_54, ts->Conv_54_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk0); // 66
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
    // save_bin(mem_blk1); // 67
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 256, 1, 1, ts->Conv_56, ts->Conv_56_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk0); // 68
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 69
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk2, 1);
    // save_bin(mem_blk2); // 70
    IF_FLAG_STOP; enn_convdw3x3s2_pack4_neon(ts->mem_blk2, ts->mem_blk1, 256, 3, 2, ts->Conv_58, ts->Conv_58_bs);
    // save_bin(mem_blk1); // 71
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 72
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_60, ts->Conv_60_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk0); // 73
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk0, ts->mem_blk1, 128, 1, 1, ts->Conv_61, ts->Conv_61_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk1); // 74
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 75
    IF_FLAG_STOP; enn_padding(ts->mem_blk1, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 76
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk2, 128, 3, 1, ts->Conv_63, ts->Conv_63_bs);
    // save_bin(mem_blk2); // 77
    IF_FLAG_STOP; enn_relu(ts->mem_blk2);
    // save_bin(mem_blk2); // 78
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk1, 64, 1, 1, ts->Conv_65, ts->Conv_65_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk1); // 79
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk0, ts->mem_blk1, ts->mem_blk2);
    // save_bin(mem_blk2); // 80
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk2, ts->mem_blk0, 128, 1, 1, ts->Conv_67, ts->Conv_67_bs, (float*)ts->mem_blk1);
    // save_bin(mem_blk0); // 81
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 82
    IF_FLAG_STOP; enn_padding(ts->mem_blk0, ts->mem_blk3, 1);
    // save_bin(mem_blk3); // 83
    IF_FLAG_STOP; enn_convdw3x3s1_pack4_neon(ts->mem_blk3, ts->mem_blk1, 128, 3, 1, ts->Conv_69, ts->Conv_69_bs);
    // save_bin(mem_blk1); // 84
    IF_FLAG_STOP; enn_relu(ts->mem_blk1);
    // save_bin(mem_blk1); // 85
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 64, 1, 1, ts->Conv_71, ts->Conv_71_bs, (float*)ts->mem_blk3);
    // save_bin(mem_blk0); // 86
    IF_FLAG_STOP; enn_eltwise_sum_pack4_neon(ts->mem_blk2, ts->mem_blk0, ts->mem_blk1);
    // save_bin(mem_blk1); // 87
    IF_FLAG_STOP; enn_conv1x1s1_sgemm_pack4_neon(ts->mem_blk1, ts->mem_blk0, 512, 1, 1, ts->Conv_73, ts->Conv_73_bs, (float*)ts->mem_blk2);
    // save_bin(mem_blk0); // 88
    IF_FLAG_STOP; enn_relu(ts->mem_blk0);
    // save_bin(mem_blk0); // 89
    IF_FLAG_STOP; enn_convdw_gdc_pack4_neon(ts->mem_blk0, ts->mem_blk1, 512, 4, 1, ts->Conv_75, ts->Conv_75_bs);
    // save_bin(mem_blk1); // 90
    ts->mem_blk1->h = ts->mem_blk1->c * ts->mem_blk1->w * ts->mem_blk1->h * ts->mem_blk1->pack;
    ts->mem_blk1->w = 1;
    ts->mem_blk1->c = 1;
    ts->mem_blk1->pack = 1;
    ts->mem_blk1->cstep = ts->mem_blk1->h;
    // save_bin(mem_blk1); // 91
    IF_FLAG_STOP; enn_innerproduct(ts->mem_blk1, ts->mem_blk0, (ts->m_isHandModeling) ? 14 : 136, ts->Gemm_77, ts->Gemm_77_bs);
    // save_bin(mem_blk0); // 92
    return (float*)ts->mem_blk0->mem;
}
