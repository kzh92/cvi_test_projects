
#ifndef _ENN_PAD_H__INCLUDED_
#define _ENN_PAD_H__INCLUDED_

#include "enn_global.h"

#define		padding							sub_000500

namespace ENN
{
	void padding(enn_blob* in_blob, enn_blob* out_blob, int pad);
}

#endif //_ENN_PAD_H__INCLUDED_
