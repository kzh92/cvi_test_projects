
#ifndef _ENNQ_TRANS_H__INCLUDED_
#define _ENNQ_TRANS_H__INCLUDED_

#include "ennq_global.h"

#define		transpose1						sub_010900
#define		transpose3						sub_010901
#define		transpose1_packed				sub_010902
#define		transpose3_packed				sub_010903

namespace ENNQ
{
	void transpose1(unsigned char* in, int dim, signed char* out);
	void transpose3(unsigned char* in, int dim, signed char* out, int fIsColor = false);

	void transpose1_packed(unsigned char* in, int width, int height, ennq_blob* out_blob);
	void transpose3_packed(unsigned char* in, int width, int height, ennq_blob* out_blob, int fIsColor = false);
}

#endif //_ENNQ_TRANS_H__INCLUDED_
