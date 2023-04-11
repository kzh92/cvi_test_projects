
#if __ARM_NEON
#include <arm_neon.h>
#endif

#include "enn_reshape.h"

extern int g_nThreadCount;

    void enn_reshape(enn_blob* in_blob, enn_blob* out_blob)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;

		float* in_mem = (float*)in_blob->mem;
		int size = w * h;
		int size_4 = size * 4;

		out_blob->c = 1;
		out_blob->w = 1;
		out_blob->h = inch * w * h * in_blob->pack;
		out_blob->cstep = out_blob->h;
		out_blob->pack = 1;

		float* out_mem = (float*)out_blob->mem;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
        for (int p = 0; p < inch; p++)
		{
			float* in_mem_p = in_mem + size_4 * p;

			float* out_mem_p0 = out_mem + size_4 * p;
			float* out_mem_p1 = out_mem_p0 + size;
			float* out_mem_p2 = out_mem_p1 + size;
			float* out_mem_p3 = out_mem_p2 + size;

#if __ARM_NEON
			int nn = size >> 1;
			int remain = size - (nn << 1);

			if (nn > 0)
			{
				asm volatile(
					"0:							\n"

					"pld		[%0, #256]		\n"
					"vld4.f32	{d28-d31}, [%0 :128]! \n"

					"subs       %5, %5, #1      \n"

					"vst1.f32   {d28}, [%1 :64]! \n"
					"vst1.f32   {d29}, [%2 :64]! \n"
					"vst1.f32   {d30}, [%3 :64]! \n"
					"vst1.f32   {d31}, [%4 :64]! \n"

					"bne        0b              \n"

					: "+r"(in_mem_p),
					"+r"(out_mem_p0),
					"+r"(out_mem_p1),
					"+r"(out_mem_p2),
					"+r"(out_mem_p3),
					"+r"(nn)
					:
					: "cc", "memory", "q14", "q15"
					);
			}
#else
			int remain = size;
#endif
			for (; remain > 0; remain--)
			{
				*out_mem_p0++ = in_mem_p[0];
				*out_mem_p1++ = in_mem_p[1];
				*out_mem_p2++ = in_mem_p[2];
				*out_mem_p3++ = in_mem_p[3];
				in_mem_p += 4;
			}
		}
	}

    void enn_flatten(enn_blob* inout_blob)
	{
		int w = inout_blob->w;
		int h = inout_blob->h;
		int c = inout_blob->c;
		int pack = inout_blob->pack;
		int nh = c * w * h * pack;

		inout_blob->c = 1;
		inout_blob->w = 1;
		inout_blob->h = nh;
		inout_blob->pack = 1;
		inout_blob->cstep = nh;
	}

