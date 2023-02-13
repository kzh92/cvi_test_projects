
#include <string.h>

#include "ennq_pad.h"

namespace ENNQ
{
	void padding_nn(signed char* in, int ch, int dim, signed char* out, int pad)
	{
		int c;
		int odim = dim + pad * 2;
		int nTB = odim * pad;
		int sz = dim * dim;
		int osz = odim * odim;;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (c = 0; c < ch; c++)
		{
			const signed char* imin_iter = in + c * sz;
			signed char* imout_iter = out + c * osz;

			memset(imout_iter, 0, nTB);
			imout_iter += nTB;
			for (int y = 0; y < dim; y++)
			{
				memset(imout_iter, 0, pad);
				imout_iter += pad;

				memcpy(imout_iter, imin_iter, dim);
				imout_iter += dim;
				imin_iter += dim;

				memset(imout_iter, 0, pad);
				imout_iter += pad;
			}
			memset(imout_iter, 0, nTB);
			imout_iter += nTB;
		}
	}

	void padding_packed(ennq_blob *in_blob, ennq_blob* out_blob, int pad)
	{
		int width = in_blob->w;
		int height = in_blob->h;
		int ch = in_blob->c;
		int cstep = in_blob->cstep;
		int pack = in_blob->packp;
		int elem = in_blob->elemc;

		int packsize = pack * elem;
		int nnc = ch / pack;

		int out_width = width + pad * 2;
		int out_height = height + pad * 2;
		int out_size = out_width * out_height;
		int out_cstep = get_blob_size(out_size * packsize) / elem;

		out_blob->w = out_width;
		out_blob->h = out_height;
		out_blob->c = ch;
		out_blob->cstep = out_cstep;
		out_blob->packp = pack;
		out_blob->elemc = elem;

		int pad_sz = pad * packsize;
		int nTB_sz = (width + pad * 2) * pad_sz;
		int wid_sz = width * packsize;

		const signed char* in_mem = (const signed char*)in_blob->mem;
		signed char* out_mem = (signed char*)out_blob->mem;

		int cstep_elem = cstep * elem;
		int out_cstep_elem = out_cstep * elem;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < nnc; c++)
		{
			const signed char* in_it = in_mem + c * cstep_elem;
			signed char* out_it = out_mem + c * out_cstep_elem;

			memset(out_it, 0, nTB_sz); out_it += nTB_sz;

			for (int y = 0; y < height; y++)
			{
				memset(out_it, 0, pad_sz); out_it += pad_sz;
				memcpy(out_it, in_it, wid_sz); out_it += wid_sz; in_it += wid_sz;
				memset(out_it, 0, pad_sz); out_it += pad_sz;
			}

			memset(out_it, 0, nTB_sz); out_it += nTB_sz;
		}
	}
}
