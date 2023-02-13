
#ifndef _ESN_H__INCLUDED_
#define _ESN_H__INCLUDED_

class ESN
{
private:
	signed char* Conv_0; // [144]
	float* Conv_0_sc; // [16]
	float* Conv_0_bs; // [16]
	signed char* Conv_2; // [768]
	float* Conv_2_q; // [1]
	float* Conv_2_sc; // [48]
	float* Conv_2_bs; // [48]
	signed char* Conv_4; // [432]
	float* Conv_4_sc; // [48]
	float* Conv_4_bs; // [48]
	signed char* Conv_6; // [768]
	float* Conv_6_sc; // [16]
	float* Conv_6_bs; // [16]
	signed char* Conv_8; // [1024]
	float* Conv_8_q; // [1]
	float* Conv_8_sc; // [64]
	float* Conv_8_bs; // [64]
	signed char* Conv_10; // [576]
	float* Conv_10_sc; // [64]
	float* Conv_10_bs; // [64]
	signed char* Conv_12; // [2048]
	float* Conv_12_sc; // [32]
	float* Conv_12_bs; // [32]
	signed char* Conv_13; // [3072]
	float* Conv_13_q; // [1]
	float* Conv_13_sc; // [96]
	float* Conv_13_bs; // [96]
	signed char* Conv_15; // [864]
	float* Conv_15_sc; // [96]
	float* Conv_15_bs; // [96]
	signed char* Conv_17; // [3072]
	float* Conv_17_sc; // [32]
	float* Conv_17_bs; // [32]
	signed char* Conv_19; // [3840]
	float* Conv_19_q; // [1]
	float* Conv_19_sc; // [120]
	float* Conv_19_bs; // [120]
	signed char* Conv_21; // [1080]
	float* Conv_21_sc; // [120]
	float* Conv_21_bs; // [120]
	signed char* Conv_23; // [3840]
	float* Conv_23_sc; // [32]
	float* Conv_23_bs; // [32]
	signed char* Conv_25; // [4608]
	float* Conv_25_q; // [1]
	float* Conv_25_sc; // [144]
	float* Conv_25_bs; // [144]
	signed char* Conv_27; // [1296]
	float* Conv_27_sc; // [144]
	float* Conv_27_bs; // [144]
	signed char* Conv_29; // [9216]
	float* Conv_29_sc; // [64]
	float* Conv_29_bs; // [64]
	signed char* Conv_30; // [9216]
	float* Conv_30_q; // [1]
	float* Conv_30_sc; // [144]
	float* Conv_30_bs; // [144]
	signed char* Conv_32; // [1296]
	float* Conv_32_sc; // [144]
	float* Conv_32_bs; // [144]
	signed char* Conv_34; // [9216]
	float* Conv_34_sc; // [64]
	float* Conv_34_bs; // [64]
	signed char* Conv_36; // [15360]
	float* Conv_36_q; // [1]
	float* Conv_36_sc; // [240]
	float* Conv_36_bs; // [240]
	signed char* Conv_38; // [2160]
	float* Conv_38_sc; // [240]
	float* Conv_38_bs; // [240]
	signed char* Conv_40; // [30720]
	float* Conv_40_sc; // [128]
	float* Conv_40_bs; // [128]
	signed char* Conv_42; // [1152]
	float* Conv_42_sc; // [128]
	float* Conv_42_bs; // [128]
	signed char* Gemm_44; // [256]
	float* Gemm_44_sc; // [2]
	float* Gemm_44_bs; // [2]

	unsigned char* dic_data;
	unsigned char* mem_data;

	signed char* mem_blk0;
	signed char* mem_blk1;
	signed char* mem_blk2;
	signed char* mem_blk3;

	int g_nEngineLoaded;

public:
	ESN();
	~ESN();

    static int dnn_dic_size();
    static int dnn_mem_size();
	int dnn_create(const char* fn, unsigned char* pMemBuf = 0);
	int dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf = 0);
	void dnn_free();
	float* dnn_forward(unsigned char* in);
};

#endif //_ESN_H__INCLUDED_
