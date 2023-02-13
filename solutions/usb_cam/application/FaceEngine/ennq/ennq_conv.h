
#ifndef _ENNQ_CONV_H__INCLUDED_
#define _ENNQ_CONV_H__INCLUDED_

#include "ennq_global.h"

#define		conv_k1s1_nf					sub_010100
#define		conv_k1s1_nn_r					sub_010101
#define		conv_k1s1_nn					sub_010102
#define		convd_k3s1_nn_r					sub_010103
#define		convd_k3s1_nf					sub_010104
#define		convd_k3s1_nn					sub_010105
#define		convd_k3s2_nf					sub_010106
#define		convd_k3s2_nn_r					sub_010107
#define		conv_kxsx_nf					sub_010108
#define		conv_kxsx_nf_r					sub_010109
#define		conv_kxsx_nn_r					sub_010110
#define		conv_kxsx_nn					sub_010111
#define		conv_nf							sub_010112
#define		conv_nf_r						sub_010113
#define		conv_nn_r						sub_010114
#define		conv_nn							sub_010115
#define		convd_nf						sub_010116
#define		convd_nn_r						sub_010117
#define		convd_nn						sub_010118
#define		conv_k1s1_nn_r_pack8			sub_010119
#define		conv_k1s1_nf_pack8				sub_010120
#define		conv_kxsx_nn_r_pack1to8			sub_010121
#define		convd_k3s1_nn_r_pack8			sub_010122
#define		convd_k3s2_nn_r_pack8			sub_010123
#define		convd_k8s1_nf_pack8				sub_010124
#define		conv_k1s1_nn_p_pack8			sub_010125
#define     conv_kxsx_nn_p_pack1to8			sub_010126
#define     convd_k3s1_nn_p_pack8			sub_010127
#define     convd_k3s2_nn_p_pack8			sub_010128

namespace ENNQ
{
	void conv_k1s1_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_k1s1_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_k1s1_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_k3s1_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_k3s1_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_k3s1_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_k3s2_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_k3s2_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_kxsx_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_kxsx_nf_r(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_kxsx_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_kxsx_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_nf_r(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void convd_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_k1s1_nn_r_pack8(ennq_blob* in_blob, ennq_blob* out_blob, int outch, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);
	void conv_k1s1_nn_p_pack8(ennq_blob* in_blob, ennq_blob* out_blob, int outch, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu, void* mem_tm);

	void conv_k1s1_nf_pack8(ennq_blob* in_blob, ennq_blob* out_blob, int outch, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);

	void conv_kxsx_nn_r_pack1to8(ennq_blob* in_blob, ennq_blob* out_blob, int ch_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm);
	void conv_kxsx_nn_p_pack1to8(ennq_blob* in_blob, ennq_blob* out_blob, int ch_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu, void* mem_tm);

	void convd_k3s1_nn_r_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias);
	void convd_k3s1_nn_p_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu);

	void convd_k3s2_nn_r_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias);
	void convd_k3s2_nn_p_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu);

	void convd_k8s1_nf_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias);
}

#endif //_ENNQ_CONV_H__INCLUDED_
