
#ifndef _ENNQ_INNER_H__INCLUDED_
#define _ENNQ_INNER_H__INCLUDED_

#include "ennq_global.h"

#define		inner_nn_r6						sub_010200
#define		inner_nf						sub_010201
#define		inner_nn_r						sub_010202
#define		inner_ff_packed					sub_010202

namespace ENNQ
{
	void inner_nn_r6(const signed char* data_in, int dim_in, signed char* data_out, int dim_out, const signed char* kernel, const float* mem_scale, const float* mem_bias, const float* prev_scale);
	void inner_nf(const signed char* data_in, int dim_in, float* data_out, int dim_out, const signed char* kernel, const float* mem_scale, const float* mem_bias);
	void inner_nn_r(const signed char* data_in, int dim_in, signed char* data_out, int dim_out, const signed char* kernel, const float* mem_scale, const float* mem_bias);
	void inner_ff_packed(ennq_blob* in_blob, ennq_blob* out_blob, int dim_out, const float* kernel, const float* mem_scale, const float* mem_bias);
}

#endif //_ENNQ_INNER_H__INCLUDED_
