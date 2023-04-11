
#ifndef _ENN_ELTWISE_H__INCLUDED_
#define _ENN_ELTWISE_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_slice_eltwise					sub_000300
#define		enn_eltwise_sum_pack4_neon			sub_000301

    void enn_slice_eltwise(enn_blob* in_blob, enn_blob* out_blob);
    void enn_eltwise_sum_pack4_neon(enn_blob* in_blob, enn_blob* in_blob1, enn_blob* out_blob);

#ifdef __cplusplus
}
#endif

#endif //_ENN_ELTWISE_H__INCLUDED_
