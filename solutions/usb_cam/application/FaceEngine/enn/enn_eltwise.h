
#ifndef _ENN_ELTWISE_H__INCLUDED_
#define _ENN_ELTWISE_H__INCLUDED_

#include "enn_global.h"

#define		slice_eltwise					sub_000300
#define		eltwise_sum_pack4_neon			sub_000301

namespace ENN
{
	void slice_eltwise(enn_blob* in_blob, enn_blob* out_blob);
	void eltwise_sum_pack4_neon(enn_blob* in_blob, enn_blob* in_blob1, enn_blob* out_blob);
}

#endif //_ENN_ELTWISE_H__INCLUDED_
