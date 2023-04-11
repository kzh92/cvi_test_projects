
#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <math.h>

#include "enn_activation.h"

extern int g_nThreadCount;

    void enn_relu(enn_blob* inout_blob)
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

	void enn_prelu(enn_blob* inout_blob, unsigned short* slope)
	{
		int inch = inout_blob->c;
		int w = inout_blob->w;
		int h = inout_blob->h;
		int cstep = inout_blob->cstep;
		int pack = inout_blob->pack;
		int size = w * h;

		if (pack == 4)
		{
#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int c = 0; c < inch; c++)
			{
				float fslope[4], *pfslope;
				pfslope = fslope;

				float* ptrc = ((float*)inout_blob->mem) + cstep * c;
				_gnu_h2f_internal_vector(slope + 4 * c, fslope, 4);
#if __ARM_NEON
				int nn = size;
				asm volatile(
					"veor       q1, q0, q0          \n"
					"vld1.f32    {d4-d5}, [%2]      \n"

					"0:                             \n"
					"pld        [%1, #128]          \n"
					"vld1.f32   {d0-d1}, [%1]		\n"

					"vcle.f32	q4, q0, q1			\n"
					"vmul.f32   q3, q0, q2			\n"
					"vbit.f32	q0, q3, q4			\n"

					"subs       %0, #1              \n"
					"vst1.f32   {d0-d1}, [%1]!		\n"
					"bne        0b                  \n"
					: "=r"(nn),     // %0
					"=r"(ptrc),     // %1
					"=r"(pfslope)   // %2
					: "0"(nn),
					"1"(ptrc),
					"2"(pfslope)
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4"
					);
#else
				int remain = size;
				for (; remain > 0; remain--)
				{
					float v;
					v = *ptrc; if (v < 0.0f) v *= fslope[0]; *ptrc++ = v;
					v = *ptrc; if (v < 0.0f) v *= fslope[1]; *ptrc++ = v;
					v = *ptrc; if (v < 0.0f) v *= fslope[2]; *ptrc++ = v;
					v = *ptrc; if (v < 0.0f) v *= fslope[3]; *ptrc++ = v;
				}
#endif // __ARM_NEON
			}
		}
		else
		{
#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int c = 0; c < inch; c++)
			{
				float* ptrc = ((float*)inout_blob->mem) + cstep * c;
				float fslope = _gnu_h2f_internal_f(slope[c]);
#if __ARM_NEON
				int nn = size >> 2;
				int remain = size - (nn << 2);
				if (nn > 0)
				{
					asm volatile(
						"veor       q1, q0, q0          \n"
						"vdup.32    q2, %2              \n"

						"0:                             \n"
						"pld        [%1, #128]          \n"
						"vld1.f32   {d0-d1}, [%1]		\n"

						"vcle.f32	q4, q0, q1			\n"
						"vmul.f32   q3, q0, q2			\n"
						"vbit.f32	q0, q3, q4			\n"

						"subs       %0, #1              \n"
						"vst1.f32   {d0-d1}, [%1]!		\n"
						"bne        0b                  \n"
						: "=r"(nn),     // %0
						"=r"(ptrc),   // %1
						"=r"(fslope)  // %2
						: "0"(nn),
						"1"(ptrc),
						"2"(fslope)
						: "cc", "memory", "q0", "q1", "q2", "q3", "q4"
						);
				}
#else
				int remain = size;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					float v = *ptrc;
					if (v < 0.0f) v *= fslope;
					*ptrc++ = v;
				}
			}
		}
	}

    void enn_sigmoid(enn_blob* inout_blob)
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
