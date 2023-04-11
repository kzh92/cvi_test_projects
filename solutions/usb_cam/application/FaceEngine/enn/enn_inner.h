
#ifndef _ENN_INNER_H__INCLUDED_
#define _ENN_INNER_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_innerproduct					sub_000400

    void enn_innerproduct(enn_blob* in_blob, enn_blob* out_blob, int num_output, unsigned short* kernel, unsigned short* bias);

#ifdef __cplusplus
}
#endif

#endif // _ENN_INNER_H__INCLUDED_
