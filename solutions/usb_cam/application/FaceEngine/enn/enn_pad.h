
#ifndef _ENN_PAD_H__INCLUDED_
#define _ENN_PAD_H__INCLUDED_

#ifdef __cplusplus
extern "C"{
#endif

#include "enn_global.h"

#define		enn_padding							sub_000500

    void enn_padding(enn_blob* in_blob, enn_blob* out_blob, int pad);

#ifdef __cplusplus
}
#endif

#endif //_ENN_PAD_H__INCLUDED_
