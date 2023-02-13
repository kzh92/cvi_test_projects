
#ifndef _ENNQ_SLEL_H__INCLUDED_
#define _ENNQ_SLEL_H__INCLUDED_

#include "ennq_global.h"

#define		slice_eltwise_ff				sub_010800
#define		eltwise_sum_ff					sub_010801
#define		eltwise_sum_ff_packed			sub_010802

namespace ENNQ
{
	void slice_eltwise_ff(const float* in, int size, float* out);
	void eltwise_sum_ff(const float* data_in1, const float* data_in2, int size, float* data_out);
	void eltwise_sum_ff_packed(ennq_blob* in_blob1, ennq_blob* in_blob2, ennq_blob* out_blob);
}

#endif //_ENNQ_SLEL_H__INCLUDED_
