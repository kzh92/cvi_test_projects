
#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <math.h>

#include "enn_activation.h"

namespace ENN
{
	void relu(enn_blob* inout_blob)
	{
		int inch = inout_blob->c;
		int w = inout_blob->w;
		int h = inout_blob->h;
		int size = w * h * inout_blob->pack;
		int cstep = inout_blob->cstep;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < inch; c++)
		{
			float* ptrc = ((float*)inout_blob->mem) + cstep * c;
#if __ARM_NEON
			int nn = size >> 2;
			int remain = size - (nn << 2);
			if (nn > 0)
			{
				asm volatile(
					"veor       q1, q0, q0          \n"
					"0:                             \n"
					"pld        [%1, #128]          \n"
					"vld1.f32   {d0-d1}, [%1]		\n"
					"vmax.f32   q0, q0, q1          \n"
					"subs       %0, #1              \n"
					"vst1.f32   {d0-d1}, [%1]!		\n"
					"bne        0b                  \n"
					: "=r"(nn),     // %0
					"=r"(ptrc)     // %1
					: "0"(nn),
					"1"(ptrc)
					: "cc", "memory", "q0", "q1"
					);
			}
#else
			int remain = size;
#endif // __ARM_NEON
			for (; remain > 0; remain--)
			{
				float v = *ptrc;
				if (v < 0.0f) v = 0.0f;
				*ptrc++ = v;
			}
		}
	}

	void sigmoid(enn_blob* inout_blob)
	{
		int inch = inout_blob->c;
		int w = inout_blob->w;
		int h = inout_blob->h;
		int size = w * h * inout_blob->pack;
		int cstep = inout_blob->cstep;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < inch; c++)
		{
			float* ptrc = ((float*)inout_blob->mem) + cstep * c;
			int remain = size;
			for (; remain > 0; remain--)
			{
				ptrc[0] = (float)(1 / (1 + exp(-ptrc[0])));
				ptrc++;
			}
		}
	}
}
