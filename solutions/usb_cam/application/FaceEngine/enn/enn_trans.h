
#ifndef _ENN_TRANS_H__INCLUDED_
#define _ENN_TRANS_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_trans							sub_001000
#define		enn_multiply						sub_001001
#define		enn_multiply_spatial				sub_001002

    void enn_trans(unsigned char* in, int width, int height, int inch, enn_blob* out_blob, int outch, int bias, float scale); // int bias = 0, float scale = 0.00390625f
    void enn_multiply(enn_blob* in_blob0, enn_blob* in_blob1, enn_blob* out_blob);
    void enn_multiply_spatial(enn_blob* in_blob0, enn_blob* in_blob1, enn_blob* out_blob);

#ifdef __cplusplus
}
#endif

#endif //_ENN_TRANS_H__INCLUDED_
