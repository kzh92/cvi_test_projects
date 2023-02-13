
#ifndef _ENN_INNER_H__INCLUDED_
#define _ENN_INNER_H__INCLUDED_

#include "enn_global.h"

#define		innerproduct					sub_000400

namespace ENN
{
	void innerproduct(enn_blob* in_blob, enn_blob* out_blob, int num_output, float* kernel, float* bias);
}

#endif // _ENN_INNER_H__INCLUDED_
