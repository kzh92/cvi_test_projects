
#ifndef _ENN_PERMUTE_H__INCLUDED_
#define _ENN_PERMUTE_H__INCLUDED_

#include "enn_global.h"

#define		permute							sub_000600

namespace ENN
{
	void permute(enn_blob* in_blob, enn_blob* out_blob);

	int permute(enn_blob* in_blob, float* out_mem);
}

#endif //_ENN_PERMUTE_H__INCLUDED_
