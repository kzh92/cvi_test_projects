
#ifndef _ENNQ_ACTIVATION_H__INCLUDED_
#define _ENNQ_ACTIVATION_H__INCLUDED_

#define		hardswish						sub_010000
#define		relu6							sub_010001
#define		hardsigmoid						sub_010002

namespace ENNQ
{
	void hardswish(float* data_inout, int size);
	void relu6(float* data_inout, int size);
	void hardsigmoid(float* data_inout, int size);
}

#endif //_ENNQ_ACTIVATION_H__INCLUDED_
