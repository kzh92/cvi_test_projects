
#ifndef _ENNQ_QUANTIZE_H__INCLUDED_
#define _ENNQ_QUANTIZE_H__INCLUDED_

#include "ennq_global.h"

#define		quantize						sub_010600
#define		quantize_packed					sub_010601

namespace ENNQ
{
	void quantize(float* data_in, int size, signed char* data_out, const float* pNextQuantScale);
	void quantize_packed(ennq_blob* in_blob, ennq_blob* out_blob, const float* pNextQuantScale);
}

#endif //_ENNQ_QUANTIZE_H__INCLUDED_
