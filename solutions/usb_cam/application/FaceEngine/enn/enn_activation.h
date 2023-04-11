
#ifndef _ENN_ACTIVATION_H__INCLUDED_
#define _ENN_ACTIVATION_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define 	enn_relu 							sub_000100
#define 	enn_prelu 							sub_000101
#define 	enn_sigmoid 						sub_000102

    void enn_relu(enn_blob* inout_blob);
	void enn_prelu(enn_blob* inout_blob, unsigned short* slope);
    void enn_sigmoid(enn_blob* inout_blob);

#ifdef __cplusplus
}
#endif

#endif //_ENN_ACTIVATION_H__INCLUDED_
