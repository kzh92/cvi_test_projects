
#ifndef _ENNQ_PAD_H__INCLUDED_
#define _ENNQ_PAD_H__INCLUDED_

#include "ennq_global.h"

#define		padding_nn						sub_010400
#define		padding_packed					sub_010401

namespace ENNQ
{
	void padding_nn(signed char* in, int ch, int dim, signed char* out, int pad);
	void padding_packed(ennq_blob *in_blob, ennq_blob* out_blob, int pad);
}

#endif //_NQMFNR_PAD_H__INCLUDED_
