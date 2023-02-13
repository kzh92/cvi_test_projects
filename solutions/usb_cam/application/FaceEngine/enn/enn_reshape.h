
#ifndef _ENN_RESHAPE_H__INCLUDED_
#define _ENN_RESHAPE_H__INCLUDED_

#include "enn_global.h"

#define		reshape							sub_000800
#define		flatten							sub_000801

namespace ENN
{
	void reshape(enn_blob* in_blob, enn_blob* out_blob);
	void flatten(enn_blob* inout_blob);
}

#endif //_ENN_RESHAPE_H__INCLUDED_
