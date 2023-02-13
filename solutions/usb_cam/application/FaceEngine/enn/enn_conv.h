
#ifndef _ENN_CONV_H__INCLUDED_
#define _ENN_CONV_H__INCLUDED_

#include "enn_global.h"

#define		conv1x1s1_sgemm_pack4to1_neon	sub_000200
#define		conv1x1s1_sgemm_pack4_neon	    sub_000201
#define		conv3x3s1_pack4_neon		    sub_000202
#define		conv3x3s1_pack4to1_neon		    sub_000203
#define		conv3x3s2_pack1to4_neon		    sub_000204
#define		conv5x5s1_pack4_neon		    sub_000205
#define		conv5x5s1_pack1to4_neon		    sub_000206
#define		convdw3x3s1_pack4_neon		    sub_000207
#define		convdw3x3s2_pack4_neon		    sub_000208
#define		conv5x5s1_neon			     	sub_000209
#define		conv1x1s1_neon			     	sub_000210
#define		conv3x3s1_neon			     	sub_000211
#define		conv1x1s1_pack1to4_neon		    sub_000212
#define		convdw_gdc_pack4_neon		    sub_000213

namespace ENN
{
	void conv1x1s1_sgemm_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int outch, int dim_kernel, int stride, float* kernel, float* bias, float* tmp);

	void conv1x1s1_sgemm_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int outch, int dim_kernel, int stride, float* kernel, float* bias, float* tmp);

	void conv3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias);

	void conv3x3s1_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias);

	void conv3x3s2_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int outch, int dim_kernel, int stride, float* kernel, float* bias);

	void conv5x5s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias);

	void conv5x5s1_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias);

	void convdw3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int outch, int dim_kernel, int stride, float* kernel, float* bias);

	void convdw3x3s2_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int outch, int dim_kernel, int stride, float* kernel, float* bias);

	void conv5x5s1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp);

	void conv1x1s1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp);

	void conv3x3s1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp);

	void conv1x1s1_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias);

	void convdw_gdc_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias);

	void convdw_gdc_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel_w, int dim_kernel_h, int stride, float* kernel, float* bias);
}

#endif // _ENN_CONV_H__INCLUDED_
