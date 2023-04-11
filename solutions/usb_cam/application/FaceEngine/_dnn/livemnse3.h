
#ifndef _LIVEMNSE3_H__INCLUDED_
#define _LIVEMNSE3_H__INCLUDED_

#include "enn_global.h"

#ifdef __cplusplus
extern	"C"
{
#endif

typedef struct LiveMnSE3_tag
{
	unsigned short* Conv_1; // [108]
	unsigned short* Conv_1_bs; // [12]
	unsigned short* Conv_4; // [108]
	unsigned short* Conv_4_bs; // [12]
	unsigned short* Conv_6; // [288]
	unsigned short* Conv_6_bs; // [24]
	unsigned short* Conv_9; // [216]
	unsigned short* Conv_9_bs; // [24]
	unsigned short* Conv_11; // [288]
	unsigned short* Conv_11_bs; // [12]
	unsigned short* Conv_12; // [288]
	unsigned short* Conv_12_bs; // [24]
	unsigned short* Conv_15; // [216]
	unsigned short* Conv_15_bs; // [24]
	unsigned short* Conv_17; // [288]
	unsigned short* Conv_17_bs; // [12]
	unsigned short* Conv_19; // [288]
	unsigned short* Conv_19_bs; // [24]
	unsigned short* Conv_22; // [216]
	unsigned short* Conv_22_bs; // [24]
	unsigned short* Conv_24; // [288]
	unsigned short* Conv_24_bs; // [12]
	unsigned short* Conv_26; // [576]
	unsigned short* Conv_26_bs; // [48]
	unsigned short* Conv_29; // [432]
	unsigned short* Conv_29_bs; // [48]
	unsigned short* Conv_31; // [1152]
	unsigned short* Conv_31_bs; // [24]
	unsigned short* Conv_32; // [1152]
	unsigned short* Conv_32_bs; // [48]
	unsigned short* Conv_35; // [432]
	unsigned short* Conv_35_bs; // [48]
	unsigned short* Conv_37; // [1152]
	unsigned short* Conv_37_bs; // [24]
	unsigned short* Conv_39; // [1152]
	unsigned short* Conv_39_bs; // [48]
	unsigned short* Conv_42; // [432]
	unsigned short* Conv_42_bs; // [48]
	unsigned short* Conv_44; // [1152]
	unsigned short* Conv_44_bs; // [24]
	unsigned short* Conv_47; // [144]
	unsigned short* Conv_47_bs; // [6]
	unsigned short* Conv_49; // [144]
	unsigned short* Conv_49_bs; // [24]
	unsigned short* Conv_53; // [2304]
	unsigned short* Conv_53_bs; // [96]
	unsigned short* Conv_56; // [864]
	unsigned short* Conv_56_bs; // [96]
	unsigned short* Conv_58; // [4608]
	unsigned short* Conv_58_bs; // [48]
	unsigned short* Conv_59; // [4608]
	unsigned short* Conv_59_bs; // [96]
	unsigned short* Conv_62; // [864]
	unsigned short* Conv_62_bs; // [96]
	unsigned short* Conv_64; // [4608]
	unsigned short* Conv_64_bs; // [48]
	unsigned short* Conv_66; // [4608]
	unsigned short* Conv_66_bs; // [96]
	unsigned short* Conv_69; // [864]
	unsigned short* Conv_69_bs; // [96]
	unsigned short* Conv_71; // [4608]
	unsigned short* Conv_71_bs; // [48]
	unsigned short* Conv_74; // [576]
	unsigned short* Conv_74_bs; // [12]
	unsigned short* Conv_76; // [576]
	unsigned short* Conv_76_bs; // [48]
	unsigned short* Conv_80; // [9216]
	unsigned short* Conv_80_bs; // [192]
	unsigned short* Conv_82; // [9216]
	unsigned short* Conv_82_bs; // [192]
	unsigned short* Gemm_84; // [1920]
	unsigned short* Gemm_84_bs; // [10]
	unsigned short* Gemm_86; // [20]
	unsigned short* Gemm_86_bs; // [2]

	unsigned short Gemm_86_bs_mem[2]; // [2]

	unsigned char* dic_data;
	unsigned char* mem_data;

	enn_blob* mem_blk0;
	enn_blob* mem_blk1;
	enn_blob* mem_blk2;
	enn_blob* mem_blk3;

	int g_nEngineLoaded;
} LiveMnSE3;

void LiveMnSE3_LiveMnSE3(LiveMnSE3 *ts);
void LiveMnSE3_LiveMnSE3_(LiveMnSE3 *ts);

int LiveMnSE3_dnn_dic_size();
int LiveMnSE3_dnn_mem_size();
int LiveMnSE3_dnn_create(LiveMnSE3 *ts, const char* fn, float rConfig, unsigned char* pMemBuf); // float rConfig = 75, unsigned char* pMemBuf = 0
int LiveMnSE3_dnn_create_(LiveMnSE3* ts, unsigned char* pDicData, int nDicDataSize, float rConfig, unsigned char* pMemBuf); // float rConfig = 75, unsigned char* pMemBuf = 0
void LiveMnSE3_dnn_free(LiveMnSE3 *ts);
float* LiveMnSE3_dnn_forward(LiveMnSE3* ts, unsigned char* img);

int LiveMnSE3_getEngineLoaded(LiveMnSE3 *ts);

#ifdef __cplusplus
}
#endif

#endif //_LIVEMNSE3_H__INCLUDED_
