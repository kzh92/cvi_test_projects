
#ifndef _ENN_POOL_H__INCLUDED_
#define _ENN_POOL_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_pooling2x2s2_max_pack4_neon		sub_000700
#define		enn_pooling2x2s2_max_neon			sub_000701
#define		enn_pooling_global_aver_pack4_neon	sub_000702

    void enn_pooling2x2s2_max_pack4_neon(enn_blob* in_blob, enn_blob* out_blob);
    void enn_pooling2x2s2_max_neon(enn_blob* in_blob, enn_blob* out_blob);
    void enn_pooling_global_aver_pack4_neon(enn_blob* in_blob, enn_blob* out_blob);

#ifdef __cplusplus
}
#endif

#endif //_ENN_POOL_H__INCLUDED_
