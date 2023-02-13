
#ifndef _ENN_POOL_H__INCLUDED_
#define _ENN_POOL_H__INCLUDED_

#include "enn_global.h"

#define		pooling2x2s2_max_pack4_neon		sub_000700
#define		pooling2x2s2_max_neon			sub_000701
#define		pooling_global_aver_pack4_neon	sub_000702

namespace ENN
{
	void pooling2x2s2_max_pack4_neon(enn_blob* in_blob, enn_blob* out_blob);
	void pooling2x2s2_max_neon(enn_blob* in_blob, enn_blob* out_blob);
	void pooling_global_aver_pack4_neon(enn_blob* in_blob, enn_blob* out_blob);
}

#endif //_ENN_POOL_H__INCLUDED_
