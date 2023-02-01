
#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <string.h>
#include <math.h>

#include "enn_trans.h"

namespace ENN
{
	void trans(unsigned char* in, int width, int height, int inch, enn_blob* out_blob, int outch, int bias, float scale)
	{
		// (inch, outch) = (1,1), (1,3), (3,3)
		int size = width * height;
		float* out = (float*)out_blob->mem;
		int cstep = get_blob_size(size);

		out_blob->c = outch;
		out_blob->pack = 1;
		out_blob->w = width;
		out_blob->h = height;
		out_blob->cstep = cstep;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < inch; c++)
		{
			const unsigned char* imin_iter = in + size * c;
			float* imout_iter = out + cstep * c;

			for (int y = size; y > 0; y--)
			{
				int val = *imin_iter;
				*imout_iter = (val + bias) * scale;
				imout_iter++;
				imin_iter++;
			}
		}

		if (inch != outch) // 1 != 3
		{
			memcpy(out + cstep, out, size * sizeof(float));
			memcpy(out + cstep * 2, out, size * sizeof(float));
		}
	}

	void multiply(enn_blob* in_blob0, enn_blob* in_blob1, enn_blob* out_blob)
	{
		int w = in_blob0->w;
		int h = in_blob0->h;
		int c = in_blob0->c;
		int cstep = in_blob0->cstep;
		int pack = in_blob0->pack;
		float* in_mem = (float*)in_blob0->mem;

		float* sc = (float*)in_blob1->mem;
		
		out_blob->w = w;
		out_blob->h = h;
		out_blob->c = c;
		out_blob->cstep = cstep;
		out_blob->pack = pack;
		float* out_mem = (float*)out_blob->mem;

		int size = w * h;

		if (pack == 1)
		{
#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = 0; i < c; i++)
			{
				float* in_mit = in_mem + cstep * i;
				float* out_mit = out_mem + cstep * i;
				float scale = sc[i];

#if __ARM_NEON
				int nn = size >> 2;
				int remain = size & 3;
				if (nn > 0)
				{
					asm volatile(
						"vdup.f32    q3, %3				\n"
						"0:								\n"
						"pld        [%0, #128]			\n"
						"vld1.f32   {d8-d9}, [%0 :128]!	\n"
						"vmul.f32	q4, q4, q3			\n"
						"subs       %2, #1				\n"
						"vst1.f32   {d8-d9}, [%1 :128]!	\n"
						"bne        0b					\n"
						: "+r"(in_mit),			// %0
						  "+r"(out_mit),		// %1
						  "+r"(nn)				// %2
						: "r"(scale)			// %3
						: "memory", "q3", "q4"
						);
				}

#else
				int remain = size;
#endif
				for (; remain > 0; remain--)
				{
					*out_mit++ = *in_mit++ * scale;
				}
			}
		}
		else
		{
#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = 0; i < c; i++)
			{
				float* in_mit = in_mem + cstep * i;
				float* out_mit = out_mem + cstep * i;
				float* scit = sc + i * 4;
				// int nn = size;
#if __ARM_NEON
				asm volatile(
					"vld1.f32   {d6-d7}, [%3]		\n"
					"0:								\n"
					"pld        [%0, #128]			\n"
					"vld1.f32   {d8-d9}, [%0 :128]!	\n"
					"vmul.f32	q4, q4, q3			\n"
					"subs       %2, #1				\n"
					"vst1.f32   {d8-d9}, [%1 :128]!	\n"
					"bne        0b					\n"
					: "+r"(in_mit),			// %0
					  "+r"(out_mit),		// %1
					  "+r"(nn)				// %2
					: "r"(scit)				// %3
					: "memory", "q3", "q4"
					);
#else
				for (int s = 0; s < size; s++)
				{
					out_mit[s * 4 + 0] = in_mit[s * 4 + 0] * scit[0];
					out_mit[s * 4 + 1] = in_mit[s * 4 + 1] * scit[1];
					out_mit[s * 4 + 2] = in_mit[s * 4 + 2] * scit[2];
					out_mit[s * 4 + 3] = in_mit[s * 4 + 3] * scit[3];
				}
#endif
			}
		}
	}

	void multiply_spatial(enn_blob* in_blob0, enn_blob* in_blob1, enn_blob* out_blob)
	{
		int w = in_blob0->w;
		int h = in_blob0->h;
		int c = in_blob0->c;
		int cstep = in_blob0->cstep;
		int pack = in_blob0->pack;
		float* in_mem = (float*)in_blob0->mem;

		float* sc = (float*)in_blob1->mem;

		out_blob->w = w;
		out_blob->h = h;
		out_blob->c = c;
		out_blob->cstep = cstep;
		out_blob->pack = pack;
		float* out_mem = (float*)out_blob->mem;

		int size = w * h;

		if (pack == 1)
		{
#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = 0; i < c; i++)
			{
				float* in_mit = in_mem + cstep * i;
				float* out_mit = out_mem + cstep * i;
				float* scit = sc;

#if __ARM_NEON
				int nn = size >> 2;
				int remain = size & 3;
				if (nn > 0)
				{
					asm volatile(
						"0:								\n"
						"pld        [%0, #128]			\n"
						"vld1.f32   {d8-d9}, [%0 :128]!	\n"
						"vld1.f32   {d6-d7}, [%2 :128]!	\n"
						"vmul.f32	q4, q4, q3			\n"
						"subs       %3, #1				\n"
						"vst1.f32   {d8-d9}, [%1 :128]!	\n"
						"bne        0b					\n"
						: "+r"(in_mit),			// %0
						  "+r"(out_mit),		// %1
						  "+r"(scit),			// %2
						  "+r"(nn)				// %3
						:
						: "memory", "q3", "q4"
						);
				}
#else
				int remain = size;
#endif
				for (; remain > 0; remain--)
				{
					*out_mit++ = *in_mit++ * *scit++;
				}
			}
		}
		else
		{
#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = 0; i < c; i++)
			{
				float* in_mit = in_mem + cstep * i;
				float* out_mit = out_mem + cstep * i;
				float* scit = sc;

#if __ARM_NEON
				int nn = size >> 2;
				int remain = size & 3;
				asm volatile(
					"0:									\n"
					"vld1.f32	{d6-d7}, [%2 :128]!		\n"
					"pld        [%0, #128]				\n"
					"vld1.f32   {d8-d11}, [%0 :128]!	\n"
					"vld1.f32   {d12-d15}, [%0 :128]!	\n"
					"vmul.f32	q4, q4, d6[0]			\n"
					"vmul.f32	q5, q5, d6[1]			\n"
					"vmul.f32	q6, q6, d7[0]			\n"
					"vmul.f32	q7, q7, d7[1]			\n"

					"subs       %3, #1					\n"
					"vst1.f32   {d8-d11}, [%1 :128]!	\n"
					"vst1.f32   {d12-d15}, [%1 :128]!	\n"
					"bne        0b						\n"
					: "+r"(in_mit),			// %0
					  "+r"(out_mit),		// %1
					  "+r"(scit),			// %2
					  "+r"(nn)				// %3
					:
					: "memory", "q3", "q4", "q5", "q6", "q7"
					);
#else
				int remain = size;
#endif
				for (; remain > 0; remain--)
				{
					out_mit[0] = in_mit[0] * *scit;
					out_mit[1] = in_mit[1] * *scit;
					out_mit[2] = in_mit[2] * *scit;
					out_mit[3] = in_mit[3] * *scit;
					out_mit += 4;
					in_mit += 4;
					scit++;
				}
			}
		}
	}
}
