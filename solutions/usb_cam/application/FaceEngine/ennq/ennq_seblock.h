
#ifndef _ENNQ_SEBLOCK_H__INCLUDED_
#define _ENNQ_SEBLOCK_H__INCLUDED_

#include "ennq_global.h"

#define		seblock							sub_010700
#define		multiply						sub_010701

namespace ENNQ
{
	void seblock(float* data_inout, int ch_in, int dim_in, int ch_mid, const signed char* fc1, const float* fc1_q, const float* fc1_sc, const float* fc1_bs, const signed char* fc2, const float* fc2_q, const float* fc2_sc, const float* fc2_bs, float* mem_tmp);
	void multiply(float* data_inout, int ch_in, int dim_in, float* data_scale);
}

#endif //_ENNQ_SEBLOCK_H__INCLUDED_
