
#ifndef _OCCLUSION_H__INCLUDED_
#define _OCCLUSION_H__INCLUDED_

class Occlusion
{
private:
	signed char* Conv_0; // [72]
	float* Conv_0_sc; // [8]
	float* Conv_0_bs; // [8]
	signed char* Conv_4; // [1152]
	float* Conv_4_sc; // [16]
	float* Conv_4_bs; // [16]
	signed char* Conv_8; // [4608]
	float* Conv_8_sc; // [32]
	float* Conv_8_bs; // [32]
	signed char* Conv_12; // [18432]
	float* Conv_12_sc; // [64]
	float* Conv_12_bs; // [64]
	signed char* Conv_16; // [73728]
	float* Conv_16_sc; // [128]
	float* Conv_16_bs; // [128]
	signed char* Gemm_25; // [262144]
	float* Gemm_25_sc; // [128]
	float* Gemm_25_bs; // [128]
	signed char* Gemm_27; // [256]
	float* Gemm_27_sc; // [2]
	float* Gemm_27_bs; // [2]

	unsigned char* dic_data;
	unsigned char* mem_data;

	signed char* mem_blk0;
	signed char* mem_blk1;
	signed char* mem_blk2;

	int g_nEngineLoaded;

public:
	Occlusion();
	~Occlusion();

    static int dnn_dic_size();
    static int dnn_mem_size();
	int dnn_create(const char* fn, unsigned char* pMemBuf = 0);
	int dnn_create(unsigned char* pDicData, int nDicDataSize, unsigned char* pMemBuf = 0);
	void dnn_free();
	float* dnn_forward(unsigned char* in);
};

#endif //_OCCLUSION_H__INCLUDED_
