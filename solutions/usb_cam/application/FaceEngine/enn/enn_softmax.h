
#ifndef _ENN_SOFTMAX_H__INCLUDED_
#define _ENN_SOFTMAX_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_softmax							sub_000900

    void enn_softmax(float* inout, int count);

#ifdef __cplusplus
}
#endif

#endif //_ENN_SOFTMAX_H__INCLUDED_
