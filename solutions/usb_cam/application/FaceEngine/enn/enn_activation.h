
#ifndef _ENN_ACTIVATION_H__INCLUDED_
#define _ENN_ACTIVATION_H__INCLUDED_

#include "enn_global.h"

#define 	relu 							sub_000100
#define 	sigmoid 						sub_000101

namespace ENN
{
	void relu(enn_blob* inout_blob);
	void sigmoid(enn_blob* inout_blob);
}

#endif //_ENN_ACTIVATION_H__INCLUDED_
