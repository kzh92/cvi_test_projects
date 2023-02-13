
#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <string.h>

#include "enn_permute.h"

namespace ENN
{
	void permute(enn_blob* in_blob, enn_blob* out_blob)
	{
		out_blob->c = in_blob->h;
		out_blob->h = in_blob->w;
		out_blob->w = in_blob->c * in_blob->pack;
		out_blob->pack = 1;
		int out_cstep = out_blob->w * out_blob->h;
		out_blob->cstep = out_cstep;

		float* in_mem = (float*)in_blob->mem;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < out_blob->c; c++)
		{
			float* out_mem = ((float*)out_blob->mem) + out_cstep * c;
			for (int h = 0; h < out_blob->h; h++)
			{
				for (int w = 0; w < out_blob->w; w += in_blob->pack)
				{
					int ic = w / in_blob->pack;
					int ih = c;
					int iw = h;

					memcpy(out_mem, in_mem + ic * in_blob->cstep + ih * in_blob->w * in_blob->pack + iw * in_blob->pack, in_blob->pack * 4);
					out_mem += in_blob->pack;
				}
			}
		}

		out_blob->h = out_blob->w * out_blob->h * out_blob->c / 2;
		out_blob->w = 2;
		out_blob->c = 1;
		out_blob->pack = 1;
		out_blob->cstep = out_blob->w * out_blob->h;
	}

	int permute(enn_blob* in_blob, float* out_mem)
	{
		int oc = in_blob->h;
		int oh = in_blob->w;
		int out_cstep = oh * in_blob->c * in_blob->pack;

		float* in_mem = (float*)in_blob->mem;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < oc; c++)
		{
			float* out_mem_it = out_mem + out_cstep * c;
			for (int h = 0; h < oh; h++)
			{
				for (int w = 0; w < in_blob->c; w++)
				{
					memcpy(out_mem_it, in_mem + w * in_blob->cstep + c * in_blob->w * in_blob->pack + h * in_blob->pack, in_blob->pack * 4);
					out_mem_it += in_blob->pack;
				}
			}
		}

		return oc * out_cstep;
	}
}
