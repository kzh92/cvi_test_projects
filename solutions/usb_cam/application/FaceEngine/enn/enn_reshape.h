
#ifndef _ENN_RESHAPE_H__INCLUDED_
#define _ENN_RESHAPE_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_reshape							sub_000800
#define		enn_flatten							sub_000801

    void enn_reshape(enn_blob* in_blob, enn_blob* out_blob);
    void enn_flatten(enn_blob* inout_blob);

#ifdef __cplusplus
}
#endif

#endif //_ENN_RESHAPE_H__INCLUDED_
