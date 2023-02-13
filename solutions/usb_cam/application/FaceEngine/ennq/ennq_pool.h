
#ifndef _ENNQ_POOL_H__INCLUDED_
#define _ENNQ_POOL_H__INCLUDED_

#include "ennq_global.h"

#define		global_pooling					sub_010500
#define		pooling_max_nn					sub_010501

namespace ENNQ
{
	void global_pooling(const float* data_in, int ch_in, int dim_in, float* data_out);
	void pooling_max_nn(const signed char* data_in, int ch, int width, signed char* data_out);
}

#endif //_ENNQ_POOL_H__INCLUDED_
