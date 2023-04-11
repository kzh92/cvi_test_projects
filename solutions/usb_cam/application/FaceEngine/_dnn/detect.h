
#ifndef _DETECT_MN_H__INCLUDED_
#define _DETECT_MN_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

typedef struct Detect_tag
{
    unsigned short* Conv_0; // [216]
    unsigned short* Conv_0_bs; // [8]
    unsigned short* Conv_2; // [72]
    unsigned short* Conv_2_bs; // [8]
    unsigned short* Conv_4; // [128]
    unsigned short* Conv_4_bs; // [16]
    unsigned short* Conv_6; // [144]
    unsigned short* Conv_6_bs; // [16]
    unsigned short* Conv_8; // [128]
    unsigned short* Conv_8_bs; // [8]
    unsigned short* Conv_9; // [128]
    unsigned short* Conv_9_bs; // [16]
    unsigned short* Conv_11; // [144]
    unsigned short* Conv_11_bs; // [16]
    unsigned short* Conv_13; // [128]
    unsigned short* Conv_13_bs; // [8]
    unsigned short* Conv_15; // [256]
    unsigned short* Conv_15_bs; // [32]
    unsigned short* Conv_17; // [288]
    unsigned short* Conv_17_bs; // [32]
    unsigned short* Conv_19; // [512]
    unsigned short* Conv_19_bs; // [16]
    unsigned short* Conv_20; // [512]
    unsigned short* Conv_20_bs; // [32]
    unsigned short* Conv_22; // [288]
    unsigned short* Conv_22_bs; // [32]
    unsigned short* Conv_24; // [512]
    unsigned short* Conv_24_bs; // [16]
    unsigned short* Conv_26; // [512]
    unsigned short* Conv_26_bs; // [32]
    unsigned short* Conv_28; // [288]
    unsigned short* Conv_28_bs; // [32]
    unsigned short* Conv_30; // [512]
    unsigned short* Conv_30_bs; // [16]
    unsigned short* Conv_32; // [512]
    unsigned short* Conv_32_bs; // [32]
    unsigned short* Conv_34; // [288]
    unsigned short* Conv_34_bs; // [32]
    unsigned short* Conv_36; // [512]
    unsigned short* Conv_36_bs; // [16]
    unsigned short* Conv_38; // [144]
    unsigned short* Conv_38_bs; // [16]
    unsigned short* Conv_40; // [192]
    unsigned short* Conv_40_bs; // [8]
    unsigned short* Conv_43; // [144]
    unsigned short* Conv_43_bs; // [16]
    unsigned short* Conv_45; // [192]
    unsigned short* Conv_45_bs; // [12]
    unsigned short* Conv_48; // [768]
    unsigned short* Conv_48_bs; // [48]
    unsigned short* Conv_50; // [432]
    unsigned short* Conv_50_bs; // [48]
    unsigned short* Conv_52; // [1536]
    unsigned short* Conv_52_bs; // [32]
    unsigned short* Conv_53; // [1536]
    unsigned short* Conv_53_bs; // [48]
    unsigned short* Conv_55; // [432]
    unsigned short* Conv_55_bs; // [48]
    unsigned short* Conv_57; // [1536]
    unsigned short* Conv_57_bs; // [32]
    unsigned short* Conv_59; // [1536]
    unsigned short* Conv_59_bs; // [48]
    unsigned short* Conv_61; // [432]
    unsigned short* Conv_61_bs; // [48]
    unsigned short* Conv_63; // [1536]
    unsigned short* Conv_63_bs; // [32]
    unsigned short* Conv_65; // [288]
    unsigned short* Conv_65_bs; // [32]
    unsigned short* Conv_67; // [128]
    unsigned short* Conv_67_bs; // [4]
    unsigned short* Conv_70; // [288]
    unsigned short* Conv_70_bs; // [32]
    unsigned short* Conv_72; // [256]
    unsigned short* Conv_72_bs; // [8]
    unsigned short* Conv_75; // [2048]
    unsigned short* Conv_75_bs; // [64]
    unsigned short* Conv_77; // [576]
    unsigned short* Conv_77_bs; // [64]
    unsigned short* Conv_79; // [3072]
    unsigned short* Conv_79_bs; // [48]
    unsigned short* Conv_80; // [3072]
    unsigned short* Conv_80_bs; // [64]
    unsigned short* Conv_82; // [576]
    unsigned short* Conv_82_bs; // [64]
    unsigned short* Conv_84; // [3072]
    unsigned short* Conv_84_bs; // [48]
    unsigned short* Conv_86; // [432]
    unsigned short* Conv_86_bs; // [48]
    unsigned short* Conv_88; // [576]
    unsigned short* Conv_88_bs; // [8]
    unsigned short* Conv_91; // [432]
    unsigned short* Conv_91_bs; // [48]
    unsigned short* Conv_93; // [576]
    unsigned short* Conv_93_bs; // [12]
    unsigned short* Conv_96; // [3840]
    unsigned short* Conv_96_bs; // [80]
    unsigned short* Conv_98; // [720]
    unsigned short* Conv_98_bs; // [80]
    unsigned short* Conv_100; // [5120]
    unsigned short* Conv_100_bs; // [64]
    unsigned short* Conv_101; // [576]
    unsigned short* Conv_101_bs; // [64]
    unsigned short* Conv_103; // [256]
    unsigned short* Conv_103_bs; // [4]
    unsigned short* Conv_106; // [576]
    unsigned short* Conv_106_bs; // [64]
    unsigned short* Conv_108; // [512]
    unsigned short* Conv_108_bs; // [8]

    unsigned char* dic_data;
    unsigned char* mem_data;

    enn_blob* mem_blk0;
    enn_blob* mem_blk1;
    enn_blob* mem_blk2;
    enn_blob* mem_blk3;
    enn_blob* mem_blk4;
    enn_blob* mem_blk5;
    enn_blob* mem_blk6;
    enn_blob* mem_blk7;
    enn_blob* mem_blk8;
    enn_blob* mem_blk9;

    int g_nEngineLoaded;
} Detect;

void Detect_Detect(Detect* ts);
void Detect_Detect_(Detect* ts);

int Detect_dnn_dic_size();
int Detect_dnn_mem_size();
int Detect_dnn_create(Detect* ts, const char* fn, unsigned char* pMemBuf); // unsigned char* pMemBuf = 0
int Detect_dnn_create_(Detect* ts, unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf); // unsigned char* pMemBuf = 0
void Detect_dnn_free(Detect* ts);
int Detect_dnn_forward(Detect* ts, unsigned char* img, int width, int height, float** pprScore, float** pprBox, int isSimple); // bool isSimple = false
int Detect_getEngineLoaded(Detect* ts);

#ifdef __cplusplus
}
#endif

#endif //_DETECT_MN_H__INCLUDED_
