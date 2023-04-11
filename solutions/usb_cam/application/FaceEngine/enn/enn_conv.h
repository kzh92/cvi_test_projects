
#ifndef _ENN_CONV_H__INCLUDED_
#define _ENN_CONV_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_conv1x1s1_sgemm_pack4to1_neon	sub_000200
#define		enn_conv1x1s1_sgemm_pack4_neon	    sub_000201
#define		enn_conv3x3s1_pack4_neon		    sub_000202
#define		enn_conv3x3s1_pack4to1_neon		    sub_000203
#define		enn_conv3x3s2_pack1to4_neon		    sub_000204
#define		enn_convdw3x3s1_pack4_neon		    sub_000205
#define		enn_convdw3x3s2_pack4_neon		    sub_000206
#define		enn_convdw_gdc_pack4_neon		    sub_000207
#define		enn_convdw_gdc_pack4_neon_		    sub_000208

	void enn_conv1x1s1_sgemm_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias, float* tmp);

	void enn_conv1x1s1_sgemm_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias, float* tmp);

	void enn_conv3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias);

	void enn_conv3x3s1_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias);

	void enn_conv3x3s2_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias);

	void enn_convdw3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias);

	void enn_convdw3x3s2_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias);

	void enn_convdw_gdc_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias);

    void enn_convdw_gdc_pack4_neon_(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel_w, int dim_kernel_h, int stride, unsigned short* kernel, unsigned short* bias);

#ifdef __cplusplus
}
#endif

#endif // _ENN_CONV_H__INCLUDED_
