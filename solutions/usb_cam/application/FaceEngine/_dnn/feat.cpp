
// VERSION MFN_1.01
// Network MFN_PRelu

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include "feat.h"

#include "ennq_trans.h"
#include "ennq_pad.h"
#include "ennq_conv.h"
#include "ennq_quantize.h"
#include "ennq_slel.h"
#include "ennq_inner.h"

extern int g_nStopEngine;
#define IF_FLAG_STOP if (g_nStopEngine) return 0

Feature::Feature()
{
	dic_data = 0;
	mem_data = 0;
	g_nEngineLoaded = 0;
}

Feature::~Feature()
{
	dnn_free();
}

int Feature::dnn_dic_size()
{
	return 1292988;
}

int Feature::dnn_mem_size()
{
	return 262160 + 524304 + 557584 + 147984;
}

int Feature::dnn_create(const char* fn, float rConfig, unsigned char* pMemBuf)
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

	int ret = dnn_create(local_dicdata, nDicSize, rConfig, pMemBuf);
	if (ret)
	{
		aligned_free(local_dicdata);
		return 1;
	}

	dic_data = local_dicdata;
	g_nEngineLoaded = 1;

	return 0;
}

int Feature::dnn_create(unsigned char* pDicData, int nDicDataSize, float rConfig, unsigned char* pMemBuf)
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

	mem_blk0 = (ennq_blob*)mem_all; mem_all += 262160;
	mem_blk1 = (ennq_blob*)mem_all; mem_all += 524304;
	mem_blk2 = (ennq_blob*)mem_all; mem_all += 557584;
	mem_blk3 = (ennq_blob*)mem_all; mem_all += 147984;

	fc1_512 = (float*)aligned_malloc(262144 * 4);

	conv1 = (signed char*)(gdata + 0); // [1728]
	conv1_sc = (float*)(gdata + 1728); // [64]
	conv1_bs = (float*)(gdata + 1984); // [64]
	conv1_prelu = (float*)(gdata + 2240); // [64]
	conv1_dw = (signed char*)(gdata + 2496); // [576]
	conv1_dw_sc = (float*)(gdata + 3072); // [64]
	conv1_dw_bs = (float*)(gdata + 3328); // [64]
	conv1_dw_prelu = (float*)(gdata + 3584); // [64]
	conv2_ex = (signed char*)(gdata + 3840); // [8192]
	conv2_ex_sc = (float*)(gdata + 12032); // [128]
	conv2_ex_bs = (float*)(gdata + 12544); // [128]
	conv2_ex_prelu = (float*)(gdata + 13056); // [128]
	conv2_dw = (signed char*)(gdata + 13568); // [1152]
	conv2_dw_sc = (float*)(gdata + 14720); // [128]
	conv2_dw_bs = (float*)(gdata + 15232); // [128]
	conv2_dw_prelu = (float*)(gdata + 15744); // [128]
	conv2_em = (signed char*)(gdata + 16256); // [8192]
	conv2_em_sc = (float*)(gdata + 24448); // [64]
	conv2_em_bs = (float*)(gdata + 24704); // [64]
	conv2_1_ex = (signed char*)(gdata + 24960); // [8192]
	conv2_1_ex_sc = (float*)(gdata + 33152); // [128]
	conv2_1_ex_bs = (float*)(gdata + 33664); // [128]
	conv2_1_ex_prelu = (float*)(gdata + 34176); // [128]
	conv2_1_dw = (signed char*)(gdata + 34688); // [1152]
	conv2_1_dw_sc = (float*)(gdata + 35840); // [128]
	conv2_1_dw_bs = (float*)(gdata + 36352); // [128]
	conv2_1_dw_prelu = (float*)(gdata + 36864); // [128]
	conv2_1_em = (signed char*)(gdata + 37376); // [8192]
	conv2_1_em_sc = (float*)(gdata + 45568); // [64]
	conv2_1_em_bs = (float*)(gdata + 45824); // [64]
	conv2_2_ex = (signed char*)(gdata + 46080); // [8192]
	conv2_2_ex_sc = (float*)(gdata + 54272); // [128]
	conv2_2_ex_bs = (float*)(gdata + 54784); // [128]
	conv2_2_ex_prelu = (float*)(gdata + 55296); // [128]
	conv2_2_dw = (signed char*)(gdata + 55808); // [1152]
	conv2_2_dw_sc = (float*)(gdata + 56960); // [128]
	conv2_2_dw_bs = (float*)(gdata + 57472); // [128]
	conv2_2_dw_prelu = (float*)(gdata + 57984); // [128]
	conv2_2_em = (signed char*)(gdata + 58496); // [8192]
	conv2_2_em_sc = (float*)(gdata + 66688); // [64]
	conv2_2_em_bs = (float*)(gdata + 66944); // [64]
	conv2_3_ex = (signed char*)(gdata + 67200); // [8192]
	conv2_3_ex_sc = (float*)(gdata + 75392); // [128]
	conv2_3_ex_bs = (float*)(gdata + 75904); // [128]
	conv2_3_ex_prelu = (float*)(gdata + 76416); // [128]
	conv2_3_dw = (signed char*)(gdata + 76928); // [1152]
	conv2_3_dw_sc = (float*)(gdata + 78080); // [128]
	conv2_3_dw_bs = (float*)(gdata + 78592); // [128]
	conv2_3_dw_prelu = (float*)(gdata + 79104); // [128]
	conv2_3_em = (signed char*)(gdata + 79616); // [8192]
	conv2_3_em_sc = (float*)(gdata + 87808); // [64]
	conv2_3_em_bs = (float*)(gdata + 88064); // [64]
	conv2_4_ex = (signed char*)(gdata + 88320); // [8192]
	conv2_4_ex_sc = (float*)(gdata + 96512); // [128]
	conv2_4_ex_bs = (float*)(gdata + 97024); // [128]
	conv2_4_ex_prelu = (float*)(gdata + 97536); // [128]
	conv2_4_dw = (signed char*)(gdata + 98048); // [1152]
	conv2_4_dw_sc = (float*)(gdata + 99200); // [128]
	conv2_4_dw_bs = (float*)(gdata + 99712); // [128]
	conv2_4_dw_prelu = (float*)(gdata + 100224); // [128]
	conv2_4_em = (signed char*)(gdata + 100736); // [8192]
	conv2_4_em_sc = (float*)(gdata + 108928); // [64]
	conv2_4_em_bs = (float*)(gdata + 109184); // [64]
	conv3_ex = (signed char*)(gdata + 109440); // [16384]
	conv3_ex_sc = (float*)(gdata + 125824); // [256]
	conv3_ex_bs = (float*)(gdata + 126848); // [256]
	conv3_ex_prelu = (float*)(gdata + 127872); // [256]
	conv3_dw = (signed char*)(gdata + 128896); // [2304]
	conv3_dw_sc = (float*)(gdata + 131200); // [256]
	conv3_dw_bs = (float*)(gdata + 132224); // [256]
	conv3_dw_prelu = (float*)(gdata + 133248); // [256]
	conv3_em = (signed char*)(gdata + 134272); // [32768]
	conv3_em_sc = (float*)(gdata + 167040); // [128]
	conv3_em_bs = (float*)(gdata + 167552); // [128]
	conv3_1_ex = (signed char*)(gdata + 168064); // [32768]
	conv3_1_ex_sc = (float*)(gdata + 200832); // [256]
	conv3_1_ex_bs = (float*)(gdata + 201856); // [256]
	conv3_1_ex_prelu = (float*)(gdata + 202880); // [256]
	conv3_1_dw = (signed char*)(gdata + 203904); // [2304]
	conv3_1_dw_sc = (float*)(gdata + 206208); // [256]
	conv3_1_dw_bs = (float*)(gdata + 207232); // [256]
	conv3_1_dw_prelu = (float*)(gdata + 208256); // [256]
	conv3_1_em = (signed char*)(gdata + 209280); // [32768]
	conv3_1_em_sc = (float*)(gdata + 242048); // [128]
	conv3_1_em_bs = (float*)(gdata + 242560); // [128]
	conv3_2_ex = (signed char*)(gdata + 243072); // [32768]
	conv3_2_ex_sc = (float*)(gdata + 275840); // [256]
	conv3_2_ex_bs = (float*)(gdata + 276864); // [256]
	conv3_2_ex_prelu = (float*)(gdata + 277888); // [256]
	conv3_2_dw = (signed char*)(gdata + 278912); // [2304]
	conv3_2_dw_sc = (float*)(gdata + 281216); // [256]
	conv3_2_dw_bs = (float*)(gdata + 282240); // [256]
	conv3_2_dw_prelu = (float*)(gdata + 283264); // [256]
	conv3_2_em = (signed char*)(gdata + 284288); // [32768]
	conv3_2_em_sc = (float*)(gdata + 317056); // [128]
	conv3_2_em_bs = (float*)(gdata + 317568); // [128]
	conv3_3_ex = (signed char*)(gdata + 318080); // [32768]
	conv3_3_ex_sc = (float*)(gdata + 350848); // [256]
	conv3_3_ex_bs = (float*)(gdata + 351872); // [256]
	conv3_3_ex_prelu = (float*)(gdata + 352896); // [256]
	conv3_3_dw = (signed char*)(gdata + 353920); // [2304]
	conv3_3_dw_sc = (float*)(gdata + 356224); // [256]
	conv3_3_dw_bs = (float*)(gdata + 357248); // [256]
	conv3_3_dw_prelu = (float*)(gdata + 358272); // [256]
	conv3_3_em = (signed char*)(gdata + 359296); // [32768]
	conv3_3_em_sc = (float*)(gdata + 392064); // [128]
	conv3_3_em_bs = (float*)(gdata + 392576); // [128]
	conv3_4_ex = (signed char*)(gdata + 393088); // [32768]
	conv3_4_ex_sc = (float*)(gdata + 425856); // [256]
	conv3_4_ex_bs = (float*)(gdata + 426880); // [256]
	conv3_4_ex_prelu = (float*)(gdata + 427904); // [256]
	conv3_4_dw = (signed char*)(gdata + 428928); // [2304]
	conv3_4_dw_sc = (float*)(gdata + 431232); // [256]
	conv3_4_dw_bs = (float*)(gdata + 432256); // [256]
	conv3_4_dw_prelu = (float*)(gdata + 433280); // [256]
	conv3_4_em = (signed char*)(gdata + 434304); // [32768]
	conv3_4_em_sc = (float*)(gdata + 467072); // [128]
	conv3_4_em_bs = (float*)(gdata + 467584); // [128]
	conv3_5_ex = (signed char*)(gdata + 468096); // [32768]
	conv3_5_ex_sc = (float*)(gdata + 500864); // [256]
	conv3_5_ex_bs = (float*)(gdata + 501888); // [256]
	conv3_5_ex_prelu = (float*)(gdata + 502912); // [256]
	conv3_5_dw = (signed char*)(gdata + 503936); // [2304]
	conv3_5_dw_sc = (float*)(gdata + 506240); // [256]
	conv3_5_dw_bs = (float*)(gdata + 507264); // [256]
	conv3_5_dw_prelu = (float*)(gdata + 508288); // [256]
	conv3_5_em = (signed char*)(gdata + 509312); // [32768]
	conv3_5_em_sc = (float*)(gdata + 542080); // [128]
	conv3_5_em_bs = (float*)(gdata + 542592); // [128]
	conv3_6_ex = (signed char*)(gdata + 543104); // [32768]
	conv3_6_ex_sc = (float*)(gdata + 575872); // [256]
	conv3_6_ex_bs = (float*)(gdata + 576896); // [256]
	conv3_6_ex_prelu = (float*)(gdata + 577920); // [256]
	conv3_6_dw = (signed char*)(gdata + 578944); // [2304]
	conv3_6_dw_sc = (float*)(gdata + 581248); // [256]
	conv3_6_dw_bs = (float*)(gdata + 582272); // [256]
	conv3_6_dw_prelu = (float*)(gdata + 583296); // [256]
	conv3_6_em = (signed char*)(gdata + 584320); // [32768]
	conv3_6_em_sc = (float*)(gdata + 617088); // [128]
	conv3_6_em_bs = (float*)(gdata + 617600); // [128]
	conv4_ex = (signed char*)(gdata + 618112); // [65536]
	conv4_ex_sc = (float*)(gdata + 683648); // [512]
	conv4_ex_bs = (float*)(gdata + 685696); // [512]
	conv4_ex_prelu = (float*)(gdata + 687744); // [512]
	conv4_dw = (signed char*)(gdata + 689792); // [4608]
	conv4_dw_sc = (float*)(gdata + 694400); // [512]
	conv4_dw_bs = (float*)(gdata + 696448); // [512]
	conv4_dw_prelu = (float*)(gdata + 698496); // [512]
	conv4_em = (signed char*)(gdata + 700544); // [65536]
	conv4_em_sc = (float*)(gdata + 766080); // [128]
	conv4_em_bs = (float*)(gdata + 766592); // [128]
	conv4_1_ex = (signed char*)(gdata + 767104); // [32768]
	conv4_1_ex_sc = (float*)(gdata + 799872); // [256]
	conv4_1_ex_bs = (float*)(gdata + 800896); // [256]
	conv4_1_ex_prelu = (float*)(gdata + 801920); // [256]
	conv4_1_dw = (signed char*)(gdata + 802944); // [2304]
	conv4_1_dw_sc = (float*)(gdata + 805248); // [256]
	conv4_1_dw_bs = (float*)(gdata + 806272); // [256]
	conv4_1_dw_prelu = (float*)(gdata + 807296); // [256]
	conv4_1_em = (signed char*)(gdata + 808320); // [32768]
	conv4_1_em_sc = (float*)(gdata + 841088); // [128]
	conv4_1_em_bs = (float*)(gdata + 841600); // [128]
	conv4_2_ex = (signed char*)(gdata + 842112); // [32768]
	conv4_2_ex_sc = (float*)(gdata + 874880); // [256]
	conv4_2_ex_bs = (float*)(gdata + 875904); // [256]
	conv4_2_ex_prelu = (float*)(gdata + 876928); // [256]
	conv4_2_dw = (signed char*)(gdata + 877952); // [2304]
	conv4_2_dw_sc = (float*)(gdata + 880256); // [256]
	conv4_2_dw_bs = (float*)(gdata + 881280); // [256]
	conv4_2_dw_prelu = (float*)(gdata + 882304); // [256]
	conv4_2_em = (signed char*)(gdata + 883328); // [32768]
	conv4_2_em_sc = (float*)(gdata + 916096); // [128]
	conv4_2_em_bs = (float*)(gdata + 916608); // [128]
	conv5_ex = (signed char*)(gdata + 917120); // [65536]
	conv5_ex_sc = (float*)(gdata + 982656); // [512]
	conv5_ex_bs = (float*)(gdata + 984704); // [512]
	conv5_ex_prelu = (float*)(gdata + 986752); // [512]
	conv5_dw = (signed char*)(gdata + 988800); // [32768]
	conv5_dw_sc = (float*)(gdata + 1021568); // [512]
	conv5_dw_bs = (float*)(gdata + 1023616); // [512]
	fc1_512_table = (float*)(gdata + 1025664); // [256]
	fc1_512_index = (unsigned char*)(gdata + 1026688); // [262144]
	fc1_512_sc = (float*)(gdata + 1288832); // [512]
	fc1_512_bs = (float*)(gdata + 1290880); // [512]
	conv2_1_ex_q = (float*)(gdata + 1292928); // [1]
	conv2_2_ex_q = (float*)(gdata + 1292932); // [1]
	conv2_3_ex_q = (float*)(gdata + 1292936); // [1]
	conv2_4_ex_q = (float*)(gdata + 1292940); // [1]
	conv3_ex_q = (float*)(gdata + 1292944); // [1]
	conv3_1_ex_q = (float*)(gdata + 1292948); // [1]
	conv3_2_ex_q = (float*)(gdata + 1292952); // [1]
	conv3_3_ex_q = (float*)(gdata + 1292956); // [1]
	conv3_4_ex_q = (float*)(gdata + 1292960); // [1]
	conv3_5_ex_q = (float*)(gdata + 1292964); // [1]
	conv3_6_ex_q = (float*)(gdata + 1292968); // [1]
	conv4_ex_q = (float*)(gdata + 1292972); // [1]
	conv4_1_ex_q = (float*)(gdata + 1292976); // [1]
	conv4_2_ex_q = (float*)(gdata + 1292980); // [1]
	conv5_ex_q = (float*)(gdata + 1292984); // [1]
	fc1_512_bs[128] += (rConfig - 75);

	unsigned char* fc_index_iter = fc1_512_index;
	float* fc_table_iter = fc1_512;
	for (int i = 0; i < 262144; i++)
	{
		*fc_table_iter++ = fc1_512_table[*fc_index_iter++];
	}

	g_nEngineLoaded = 1;

	return 0;
}

void Feature::dnn_free()
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

	if (fc1_512)
	{
		aligned_free(fc1_512);
		fc1_512 = 0;
	}

	g_nEngineLoaded = 0;
}

int Feature::getEngineLoaded()
{
	return g_nEngineLoaded;
}

void save_bin(ennq_blob* blob);
void save_bin(void* blob, int sz);
double now_ms();

float* Feature::dnn_forward(unsigned char* in, int fIsColor)
{
	IF_FLAG_STOP; transpose3_packed(in, 128, 128, mem_blk0, fIsColor);
	// save_bin(mem_blk0); // 0 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 1 
	IF_FLAG_STOP; conv_kxsx_nn_p_pack1to8(mem_blk2, mem_blk1, 64, 3, 2, conv1, conv1_sc, conv1_bs, conv1_prelu, mem_blk0);
	// save_bin(mem_blk1); // 2 
	IF_FLAG_STOP; padding_packed(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 3 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk2, mem_blk0, conv1_dw, conv1_dw_sc, conv1_dw_bs, conv1_dw_prelu);
	// save_bin(mem_blk0); // 4 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk0, mem_blk1, 128, conv2_ex, conv2_ex_sc, conv2_ex_bs, conv2_ex_prelu, mem_blk2);
	// save_bin(mem_blk1); // 5 
	IF_FLAG_STOP; padding_packed(mem_blk1, mem_blk2, 1);
	// save_bin(mem_blk2); // 6 
	IF_FLAG_STOP; convd_k3s2_nn_p_pack8(mem_blk2, mem_blk0, conv2_dw, conv2_dw_sc, conv2_dw_bs, conv2_dw_prelu);
	// save_bin(mem_blk0); // 7 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk0, mem_blk1, 64, conv2_em, conv2_em_sc, conv2_em_bs, mem_blk2);
	// save_bin(mem_blk1); // 8 
	IF_FLAG_STOP; quantize_packed(mem_blk1, mem_blk2, conv2_1_ex_q);
	// save_bin(mem_blk2); // 9 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk0, 128, conv2_1_ex, conv2_1_ex_sc, conv2_1_ex_bs, conv2_1_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 10 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 11 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk2, conv2_1_dw, conv2_1_dw_sc, conv2_1_dw_bs, conv2_1_dw_prelu);
	// save_bin(mem_blk2); // 12 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk2, mem_blk0, 64, conv2_1_em, conv2_1_em_sc, conv2_1_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 13 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 14 
	IF_FLAG_STOP; quantize_packed(mem_blk2, mem_blk1, conv2_2_ex_q);
	// save_bin(mem_blk1); // 15 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk1, mem_blk0, 128, conv2_2_ex, conv2_2_ex_sc, conv2_2_ex_bs, conv2_2_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 16 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 17 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk1, conv2_2_dw, conv2_2_dw_sc, conv2_2_dw_bs, conv2_2_dw_prelu);
	// save_bin(mem_blk1); // 18 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 64, conv2_2_em, conv2_2_em_sc, conv2_2_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 19 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 20 
	IF_FLAG_STOP; quantize_packed(mem_blk1, mem_blk2, conv2_3_ex_q);
	// save_bin(mem_blk2); // 21 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk0, 128, conv2_3_ex, conv2_3_ex_sc, conv2_3_ex_bs, conv2_3_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 22 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 23 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk2, conv2_3_dw, conv2_3_dw_sc, conv2_3_dw_bs, conv2_3_dw_prelu);
	// save_bin(mem_blk2); // 24 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk2, mem_blk0, 64, conv2_3_em, conv2_3_em_sc, conv2_3_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 25 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 26 
	IF_FLAG_STOP; quantize_packed(mem_blk2, mem_blk1, conv2_4_ex_q);
	// save_bin(mem_blk1); // 27 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk1, mem_blk0, 128, conv2_4_ex, conv2_4_ex_sc, conv2_4_ex_bs, conv2_4_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 28 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 29 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk1, conv2_4_dw, conv2_4_dw_sc, conv2_4_dw_bs, conv2_4_dw_prelu);
	// save_bin(mem_blk1); // 30 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 64, conv2_4_em, conv2_4_em_sc, conv2_4_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 31 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 32 
	IF_FLAG_STOP; quantize_packed(mem_blk1, mem_blk2, conv3_ex_q);
	// save_bin(mem_blk2); // 33 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk0, 256, conv3_ex, conv3_ex_sc, conv3_ex_bs, conv3_ex_prelu, mem_blk1);
	// save_bin(mem_blk0); // 34 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 35 
	IF_FLAG_STOP; convd_k3s2_nn_p_pack8(mem_blk2, mem_blk1, conv3_dw, conv3_dw_sc, conv3_dw_bs, conv3_dw_prelu);
	// save_bin(mem_blk1); // 36 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 128, conv3_em, conv3_em_sc, conv3_em_bs, mem_blk2);
	// save_bin(mem_blk0); // 37 
	IF_FLAG_STOP; quantize_packed(mem_blk0, mem_blk2, conv3_1_ex_q);
	// save_bin(mem_blk2); // 38 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk1, 256, conv3_1_ex, conv3_1_ex_sc, conv3_1_ex_bs, conv3_1_ex_prelu, mem_blk3);
	// save_bin(mem_blk1); // 39 
	IF_FLAG_STOP; padding_packed(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 40 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk2, conv3_1_dw, conv3_1_dw_sc, conv3_1_dw_bs, conv3_1_dw_prelu);
	// save_bin(mem_blk2); // 41 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk2, mem_blk1, 128, conv3_1_em, conv3_1_em_sc, conv3_1_em_bs, mem_blk3);
	// save_bin(mem_blk1); // 42 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 43 
	IF_FLAG_STOP; quantize_packed(mem_blk2, mem_blk1, conv3_2_ex_q);
	// save_bin(mem_blk1); // 44 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk1, mem_blk0, 256, conv3_2_ex, conv3_2_ex_sc, conv3_2_ex_bs, conv3_2_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 45 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 46 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk1, conv3_2_dw, conv3_2_dw_sc, conv3_2_dw_bs, conv3_2_dw_prelu);
	// save_bin(mem_blk1); // 47 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 128, conv3_2_em, conv3_2_em_sc, conv3_2_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 48 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 49 
	IF_FLAG_STOP; quantize_packed(mem_blk1, mem_blk2, conv3_3_ex_q);
	// save_bin(mem_blk2); // 50 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk0, 256, conv3_3_ex, conv3_3_ex_sc, conv3_3_ex_bs, conv3_3_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 51 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 52 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk2, conv3_3_dw, conv3_3_dw_sc, conv3_3_dw_bs, conv3_3_dw_prelu);
	// save_bin(mem_blk2); // 53 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk2, mem_blk0, 128, conv3_3_em, conv3_3_em_sc, conv3_3_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 54 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 55 
	IF_FLAG_STOP; quantize_packed(mem_blk2, mem_blk1, conv3_4_ex_q);
	// save_bin(mem_blk1); // 56 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk1, mem_blk0, 256, conv3_4_ex, conv3_4_ex_sc, conv3_4_ex_bs, conv3_4_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 57 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 58 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk1, conv3_4_dw, conv3_4_dw_sc, conv3_4_dw_bs, conv3_4_dw_prelu);
	// save_bin(mem_blk1); // 59 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 128, conv3_4_em, conv3_4_em_sc, conv3_4_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 60 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 61 
	IF_FLAG_STOP; quantize_packed(mem_blk1, mem_blk2, conv3_5_ex_q);
	// save_bin(mem_blk2); // 62 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk0, 256, conv3_5_ex, conv3_5_ex_sc, conv3_5_ex_bs, conv3_5_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 63 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 64 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk2, conv3_5_dw, conv3_5_dw_sc, conv3_5_dw_bs, conv3_5_dw_prelu);
	// save_bin(mem_blk2); // 65 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk2, mem_blk0, 128, conv3_5_em, conv3_5_em_sc, conv3_5_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 66 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk1, mem_blk0, mem_blk2);
	// save_bin(mem_blk2); // 67 
	IF_FLAG_STOP; quantize_packed(mem_blk2, mem_blk1, conv3_6_ex_q);
	// save_bin(mem_blk1); // 68 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk1, mem_blk0, 256, conv3_6_ex, conv3_6_ex_sc, conv3_6_ex_bs, conv3_6_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 69 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 70 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk1, conv3_6_dw, conv3_6_dw_sc, conv3_6_dw_bs, conv3_6_dw_prelu);
	// save_bin(mem_blk1); // 71 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 128, conv3_6_em, conv3_6_em_sc, conv3_6_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 72 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 73 
	IF_FLAG_STOP; quantize_packed(mem_blk1, mem_blk2, conv4_ex_q);
	// save_bin(mem_blk2); // 74 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk0, 512, conv4_ex, conv4_ex_sc, conv4_ex_bs, conv4_ex_prelu, mem_blk1);
	// save_bin(mem_blk0); // 75 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk2, 1);
	// save_bin(mem_blk2); // 76 
	IF_FLAG_STOP; convd_k3s2_nn_p_pack8(mem_blk2, mem_blk1, conv4_dw, conv4_dw_sc, conv4_dw_bs, conv4_dw_prelu);
	// save_bin(mem_blk1); // 77 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 128, conv4_em, conv4_em_sc, conv4_em_bs, mem_blk2);
	// save_bin(mem_blk0); // 78 
	IF_FLAG_STOP; quantize_packed(mem_blk0, mem_blk2, conv4_1_ex_q);
	// save_bin(mem_blk2); // 79 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk1, 256, conv4_1_ex, conv4_1_ex_sc, conv4_1_ex_bs, conv4_1_ex_prelu, mem_blk3);
	// save_bin(mem_blk1); // 80 
	IF_FLAG_STOP; padding_packed(mem_blk1, mem_blk3, 1);
	// save_bin(mem_blk3); // 81 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk2, conv4_1_dw, conv4_1_dw_sc, conv4_1_dw_bs, conv4_1_dw_prelu);
	// save_bin(mem_blk2); // 82 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk2, mem_blk1, 128, conv4_1_em, conv4_1_em_sc, conv4_1_em_bs, mem_blk3);
	// save_bin(mem_blk1); // 83 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk0, mem_blk1, mem_blk2);
	// save_bin(mem_blk2); // 84 
	IF_FLAG_STOP; quantize_packed(mem_blk2, mem_blk1, conv4_2_ex_q);
	// save_bin(mem_blk1); // 85 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk1, mem_blk0, 256, conv4_2_ex, conv4_2_ex_sc, conv4_2_ex_bs, conv4_2_ex_prelu, mem_blk3);
	// save_bin(mem_blk0); // 86 
	IF_FLAG_STOP; padding_packed(mem_blk0, mem_blk3, 1);
	// save_bin(mem_blk3); // 87 
	IF_FLAG_STOP; convd_k3s1_nn_p_pack8(mem_blk3, mem_blk1, conv4_2_dw, conv4_2_dw_sc, conv4_2_dw_bs, conv4_2_dw_prelu);
	// save_bin(mem_blk1); // 88 
	IF_FLAG_STOP; conv_k1s1_nf_pack8(mem_blk1, mem_blk0, 128, conv4_2_em, conv4_2_em_sc, conv4_2_em_bs, mem_blk3);
	// save_bin(mem_blk0); // 89 
	IF_FLAG_STOP; eltwise_sum_ff_packed(mem_blk2, mem_blk0, mem_blk1);
	// save_bin(mem_blk1); // 90 
	IF_FLAG_STOP; quantize_packed(mem_blk1, mem_blk2, conv5_ex_q);
	// save_bin(mem_blk2); // 91 
	IF_FLAG_STOP; conv_k1s1_nn_p_pack8(mem_blk2, mem_blk0, 512, conv5_ex, conv5_ex_sc, conv5_ex_bs, conv5_ex_prelu, mem_blk1);
	// save_bin(mem_blk0); // 92 
	IF_FLAG_STOP; convd_k8s1_nf_pack8(mem_blk0, mem_blk1, conv5_dw, conv5_dw_sc, conv5_dw_bs);
	// save_bin(mem_blk1); // 93 
	IF_FLAG_STOP; inner_ff_packed(mem_blk1, mem_blk0, 512, fc1_512, fc1_512_sc, fc1_512_bs);
	// save_bin(mem_blk0); // 94 
	IF_FLAG_STOP; slice_eltwise_ff((float*)mem_blk0->mem, 512, (float*)mem_blk1);
	// save_bin(mem_blk1, 1024); // 95 
	return (float*)mem_blk1;
}
