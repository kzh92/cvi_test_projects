
#ifndef _ENN_PERMUTE_H__INCLUDED_
#define _ENN_PERMUTE_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_permute							sub_000600
#define		enn_permute_						sub_000601

    void enn_permute(enn_blob* in_blob, enn_blob* out_blob);
    int enn_permute_(enn_blob* in_blob, float* out_mem);

#ifdef __cplusplus
}
#endif

#endif //_ENN_PERMUTE_H__INCLUDED_
