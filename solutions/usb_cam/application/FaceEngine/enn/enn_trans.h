
#ifndef _ENN_TRANS_H__INCLUDED_
#define _ENN_TRANS_H__INCLUDED_

#include "enn_global.h"

#define		trans							sub_001000
#define		multiply						sub_001001
#define		multiply_spatial				sub_001002

namespace ENN
{
	void trans(unsigned char* in, int width, int height, int inch, enn_blob* out_blob, int outch, int bias = 0, float scale = 0.00390625f);
	void multiply(enn_blob* in_blob0, enn_blob* in_blob1, enn_blob* out_blob);
	void multiply_spatial(enn_blob* in_blob0, enn_blob* in_blob1, enn_blob* out_blob);
}

#endif //_ENN_TRANS_H__INCLUDED_
