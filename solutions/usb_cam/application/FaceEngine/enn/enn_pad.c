
#include <string.h>

#include "enn_pad.h"

extern int g_nThreadCount;

    void enn_padding(enn_blob* in_blob, enn_blob* out_blob, int pad)
	{
		int width = in_blob->w;
		int height = in_blob->h;
		int inch = in_blob->c;
		int pack = in_blob->pack;
		int pack_4 = pack * 4;
		int pack_4_p = pack_4 * pad;
		int cstep_4 = in_blob->cstep * 4;

		int out_width = width + pad * 2;
		int out_height = height + pad * 2;
		int out_size = out_width * out_height * pack;
        int out_cstep = enn_get_blob_size(out_size, 4);

		out_blob->w = out_width;
		out_blob->h = out_height;
		out_blob->c = in_blob->c;
		out_blob->pack = pack;
		out_blob->cstep = out_cstep;

		int out_cstep_4 = out_cstep * 4;
		int nTB = out_width * pack_4_p;
		int nPad4 = pack_4_p;
		int nWidth4 = width * pack_4;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
        for (int c = 0; c < inch; c++)
		{
			const unsigned char* imin_iter = ((unsigned char*)in_blob->mem) + cstep_4 * c;
			unsigned char* imout_iter = ((unsigned char*)out_blob->mem) + out_cstep_4 * c;

			memset(imout_iter, 0, nTB);
			imout_iter += nTB;

			int y;
			for (y = 0; y < height; y++)
			{
				memset(imout_iter, 0, nPad4);
				imout_iter += nPad4;

				memcpy(imout_iter, imin_iter, nWidth4);
				imout_iter += nWidth4;
				imin_iter += nWidth4;

				memset(imout_iter, 0, nPad4);
				imout_iter += nPad4;
			}

			memset(imout_iter, 0, nTB);
		}
	}

