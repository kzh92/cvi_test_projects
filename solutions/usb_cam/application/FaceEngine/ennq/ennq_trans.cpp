
#include <string.h>

#include "ennq_trans.h"

// #define NCNN_INPUT_EQUAL

namespace ENNQ
{
	void transpose1(unsigned char* in, int dim, signed char* out)
	{
		unsigned short y;
		int sz = dim * dim;

		const unsigned char* imin_iter = in;
		signed char* imout_iter = out;

		for (y = sz; y > 0; y--)
		{
#ifdef NCNN_INPUT_EQUAL
			int val = *imin_iter;
			val = (val + ((val & 2) >> 1)) / 2;
			if (val > 127) val = 127;
			*imout_iter = val;
			imin_iter++;
			imout_iter++;
#else
			*imout_iter++ = *imin_iter++ / 2;
#endif
		}
	}

	void transpose3(unsigned char* in, int dim, signed char* out, int fIsColor)
	{
		unsigned short y;
		int sz = dim * dim;

		if (fIsColor)
			sz = sz * 3;

		const unsigned char* imin_iter = in;
		signed char* imout_iter = out;

		for (y = sz; y > 0; y--)
		{
#ifdef NCNN_INPUT_EQUAL
			int val = *imin_iter;
			val = (val + ((val & 2) >> 1)) / 2;
			if (val > 127) val = 127;
			*imout_iter = val;
			imin_iter++;
			imout_iter++;
#else
			*imout_iter++ = *imin_iter++ / 2;
#endif
		}

		if (!fIsColor)
		{
			memcpy(out + sz, out, sz);
			memcpy(out + (sz << 1), out, sz);
		}
	}

	void transform_private(unsigned char* in_it, signed char* out_it, int size)
	{
		for (int y = size; y > 0; y--)
		{
#ifdef NCNN_INPUT_EQUAL
			int val = *in_it;
			val = (val + ((val & 2) >> 1)) / 2;
			if (val > 127) val = 127;
			*out_it = val;
			in_it++;
			out_it++;
#else
			*out_it++ = *in_it++ / 2;
#endif
		}
	}

	void transpose1_packed(unsigned char* in, int width, int height, ennq_blob* out_blob)
	{
		int size = width * height;
		int cstep = get_blob_size(size);

		transform_private(in, (signed char*)out_blob->mem, size);

		out_blob->c = 1;
		out_blob->w = width;
		out_blob->h = height;
		out_blob->packp = 1;
		out_blob->elemc = 1;
		out_blob->cstep = cstep;
	}

	void transpose3_packed(unsigned char* in, int width, int height, ennq_blob* out_blob, int fIsColor)
	{
		int size = width * height;
		int cstep = get_blob_size(size);

		signed char* out_mem = (signed char*)out_blob->mem;

		transform_private(in, out_mem, size);

		if (fIsColor)
		{
			transform_private(in + size, out_mem + cstep, size);
			transform_private(in + (size << 1), out_mem + (cstep << 1), size);
		}
		else
		{
			memcpy(out_mem + cstep, out_mem, size);
			memcpy(out_mem + (cstep << 1), out_mem, size);
		}

		out_blob->c = 3;
		out_blob->w = width;
		out_blob->h = height;
		out_blob->packp = 1;
		out_blob->elemc = 1;
		out_blob->cstep = cstep;
	}
}
