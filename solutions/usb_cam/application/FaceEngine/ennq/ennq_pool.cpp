
#include <algorithm>

#include "ennq_pool.h"

namespace ENNQ
{
	void global_pooling(const float* data_in, int ch_in, int dim_in, float* data_out)
	{
		int size = dim_in * dim_in;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int q = 0; q < ch_in; q++)
		{
			const float* ptr = data_in + q * size;

			float sum = 0.f;
			for (int i = 0; i < size; i++)
			{
				sum += *ptr;
				ptr++;
			}

			data_out[q] = sum / size;
		}
	}

	void pooling_max_nn(const signed char* data_in, int ch, int width, signed char* data_out) // max 2
	{
		int dim_out = width >> 1;

		int out_sz = dim_out * dim_out;
		int in_sz = out_sz << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int q = 0; q < ch; q++)
		{
			signed char* outptr = data_out + q * out_sz;
			const signed char* r0 = data_in + q * in_sz;
			const signed char* r1 = r0 + width;

			for (int i = 0; i < dim_out; i++)
			{
#if __ARM_NEON
				int nn = dim_out >> 3;
				int remain = dim_out - (nn << 3);
				if (nn > 0)
				{
					asm volatile(
						"0:                             \n"
						"pld        [%1, #256]          \n"
						"pld        [%2, #256]          \n"
						"vld1.s8    {d0-d1}, [%1]!      \n"
						"vld1.s8    {d2-d3}, [%2]!      \n"
						"vmax.s8    q0, q0, q1          \n"
						"vpmax.s8   d2, d0, d1          \n"
						"subs       %0, #1              \n"
						"vst1.s8    {d2}, [%3]!         \n"
						"bne        0b                  \n"
						: "+r"(nn),     // %0
						"+r"(r0),     // %1
						"+r"(r1),     // %2
						"+r"(outptr)  // %3
						:
						: "cc", "memory", "q0", "q1", "q2", "q3"
						);
				}
#else
				int remain = dim_out;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					signed char max0 = std::max(r0[0], r0[1]);
					signed char max1 = std::max(r1[0], r1[1]);

					*outptr = std::max(max0, max1);

					r0 += 2;
					r1 += 2;
					outptr++;
				}

				r0 += width;
				r1 += width;
			}
		}
	}
}
