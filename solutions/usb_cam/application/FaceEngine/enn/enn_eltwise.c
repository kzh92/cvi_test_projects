
#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <string.h>
#include <math.h>

#include "enn_eltwise.h"

extern int g_nThreadCount;

    void enn_slice_eltwise(enn_blob* in_blob, enn_blob* out_blob)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int c = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;
		int pack = in_blob->pack;
		// int in_size = w * h;

		int outw = w;
		int outh = h;
		float* out_mem = (float*)out_blob->mem;
		int outc;
		int out_cstep;
		int out_pack;
		int out_size;

		if (pack == 4)
		{
			if (c % 2 == 0) // 4pack => 4pack
			{
				outc = c / 2;
				out_cstep = cstep;
				out_pack = 4;

				out_size = outw * outh;

				out_blob->w = outw;
				out_blob->h = outh;
				out_blob->c = outc;
				out_blob->cstep = out_cstep;
				out_blob->pack = out_pack;

				float* ptr = in_mem;
				float* ptr1 = in_mem + outc * cstep;
				float* outptr = out_mem;
#if __ARM_NEON
				int nn = outc * out_size;
				asm volatile(
					"0:                             \n"
					"pld        [%1, #128]          \n"
					"pld        [%2, #128]          \n"
					"vld1.f32   {d0-d1}, [%1 :128]! \n"
					"vld1.f32   {d2-d3}, [%2 :128]! \n"
					"vmax.f32   q0, q0, q1          \n"
					"subs       %0, #1              \n"
					"vst1.f32   {d0-d1}, [%3 :128]! \n"
					"bne        0b                  \n"
					: "+r"(nn),     // %0
					"+r"(ptr),    // %1
					"+r"(ptr1),   // %2
					"+r"(outptr)  // %3
					:
					: "cc", "memory", "q0", "q1"
					);
#else
				int remain = outc * out_size;
				float val, val1;
				for (; remain > 0; remain--)
				{
					val = *ptr++;
					val1 = *ptr1++;
					if (val < val1) val = val1;
					*outptr++ = val;

					val = *ptr++;
					val1 = *ptr1++;
					if (val < val1) val = val1;
					*outptr++ = val;

					val = *ptr++;
					val1 = *ptr1++;
					if (val < val1) val = val1;
					*outptr++ = val;

					val = *ptr++;
					val1 = *ptr1++;
					if (val < val1) val = val1;
					*outptr++ = val;
				}
#endif
			}
			else // 4pack => 1pack
			{
				/// c == 3
				outc = c * 2;
				out_size = outw * outh;
                out_cstep = enn_get_blob_size(out_size, 4);
				out_pack = 1;

				out_blob->w = outw;
				out_blob->h = outh;
				out_blob->c = outc;
				out_blob->cstep = out_cstep;
				out_blob->pack = out_pack;

				float* ptr0 = in_mem;
				float* ptr1 = ptr0 + cstep;
				float* ptr2 = ptr1 + cstep;

				float* outptr0 = out_mem;
				float* outptr1 = outptr0 + out_cstep;
				float* outptr2 = outptr1 + out_cstep;
				float* outptr3 = outptr2 + out_cstep;
				float* outptr4 = outptr3 + out_cstep;
				float* outptr5 = outptr4 + out_cstep;
#if __ARM_NEON
				int nn = out_size >> 2;
				asm volatile(
					"0:                             \n"
					"pld        [%1, #128]          \n"
					"vld1.f32   {d0-d3}, [%1 :128]!   \n"
					"vld1.f32   {d4-d7}, [%1 :128]!   \n"
					"pld        [%2, #128]          \n"
					"vld1.f32   {d8-d11}, [%2 :128]!  \n"
					"vld1.f32   {d12-d15}, [%2 :128]! \n"
					"pld        [%3, #128]          \n"
					"vld1.f32   {d16-d19}, [%3 :128]! \n"
					"vld1.f32   {d20-d23}, [%3 :128]! \n"

					"vmax.f32   d0, d0, d9          \n"
					"vmax.f32   d2, d2, d11         \n"
					"vmax.f32   d4, d4, d13         \n"
					"vmax.f32   d6, d6, d15         \n"

					"vmax.f32   d1, d1, d16         \n"
					"vmax.f32   d3, d3, d18         \n"
					"vmax.f32   d5, d5, d20         \n"
					"vmax.f32   d7, d7, d22         \n"

					"vmax.f32   d8,  d8,  d17       \n"
					"vmax.f32   d10, d10, d19       \n"
					"vmax.f32   d12, d12, d21       \n"
					"vmax.f32   d14, d14, d23       \n"

					"vzip.32    q0, q1              \n" // q0, q2, q1, q3
					"vzip.32    q2, q3              \n"
					"vswp       d1, d4				\n"
					"vswp       d3, d6				\n"
					"vzip.32    d8, d10				\n"
					"vzip.32    d12, d14			\n"
					"vmov.32    d9, d12				\n" // q4, q5
					"vmov.32    d11, d14			\n"

					"vst1.f32  {d0-d1}, [%4]!		\n"
					"vst1.f32  {d2-d3}, [%6]!		\n"
					"vst1.f32  {d4-d5}, [%5]!		\n"
					"vst1.f32  {d6-d7}, [%7]!		\n"
					"vst1.f32  {d8-d9}, [%8]!		\n"
					"vst1.f32  {d10-d11}, [%9]!		\n"
					"subs       %0, #1              \n"
					"bne        0b                  \n"

					: "+r"(nn),     // %0
					"+r"(ptr0),     // %1
					"+r"(ptr1),     // %2
					"+r"(ptr2),		// %3
					"+r"(outptr0),  // %4
					"+r"(outptr1),  // %5
					"+r"(outptr2),  // %6
					"+r"(outptr3),  // %7
					"+r"(outptr4),  // %8
					"+r"(outptr5)   // %9
					:
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12"
					);
#else
				int remain = out_size;
				for (; remain > 0; remain--)
				{
					float v0 = *ptr0++;
					float v1 = *ptr0++;
					float v2 = *ptr0++;
					float v3 = *ptr0++;

					float v4 = *ptr1++;
					float v5 = *ptr1++;
					float v6 = *ptr1++;
					float v7 = *ptr1++;

					float v8 = *ptr2++;
					float v9 = *ptr2++;
					float va = *ptr2++;
					float vb = *ptr2++;

					if (v0 < v6) v0 = v6; *outptr0++ = v0;
					if (v1 < v7) v1 = v7; *outptr1++ = v1;
					if (v2 < v8) v2 = v8; *outptr2++ = v2;
					if (v3 < v9) v3 = v9; *outptr3++ = v3;
					if (v4 < va) v4 = va; *outptr4++ = v4;
					if (v5 < vb) v5 = vb; *outptr5++ = v5;
				}
#endif
			}
		}
		else // 1pack => 1pack
		{
			if (c == 1) // flatten
			{
				int sz = h / 2;
				out_blob->c = 1;
				out_blob->w = 1;
				out_blob->h = sz;
				out_blob->pack = 1;
				out_blob->cstep = sz;

				const float* ptr = (const float*)in_blob->mem;
				const float* ptr1 = ptr + sz;
				float* outptr = (float*)out_blob->mem;

#if __ARM_NEON
				int nn = sz >> 2;
				int remain = sz - (nn << 2);
				if (nn > 0)
				{
					asm volatile(
						"0:                             \n"
						"pld        [%1, #128]          \n"
						"pld        [%2, #128]          \n"
						"vld1.f32   {d0-d1}, [%1]!      \n"
						"vld1.f32   {d2-d3}, [%2]!      \n"
						"vmax.f32   q0, q0, q1          \n"
						"subs       %0, #1              \n"
						"vst1.f32   {d0-d1}, [%3]!      \n"
						"bne        0b                  \n"
						: "+r"(nn),     // %0
						"+r"(ptr),    // %1
						"+r"(ptr1),   // %2
						"+r"(outptr)  // %3
						:
						: "cc", "memory", "q0", "q1"
						);
				}
#else
				int remain = sz;
#endif // __ARM_NEON

				for (; remain > 0; remain--)
				{
					float val = *ptr;
					if (val < *ptr1) val = *ptr1;
					*outptr = val;
					ptr++;
					ptr1++;
					outptr++;
				}
			}
			else
			{
				outc = c / 2;
				out_cstep = cstep;
				out_pack = 1;

				out_size = outw * outh;

				out_blob->w = outw;
				out_blob->h = outh;
				out_blob->c = outc;
				out_blob->cstep = out_cstep;
				out_blob->pack = out_pack;

				float* bottom_blob1 = in_mem + outc * cstep;

#if ((ENGINE_THREAD_COUNT) != 1)
				int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
				for (int q = 0; q < outc; q++)
				{
					const float* ptr = in_mem + q * cstep; // bottom_blob.channel(q);
					const float* ptr1 = bottom_blob1 + q * cstep; // bottom_blob1.channel(q);
					float* outptr = out_mem + q * out_cstep; // top_blob.channel(q);

#if __ARM_NEON
					int nn = out_size >> 2;
					int remain = out_size - (nn << 2);
					if (nn > 0)
					{
						asm volatile(
							"0:                             \n"
							"pld        [%1, #128]          \n"
							"pld        [%2, #128]          \n"
							"vld1.f32   {d0-d1}, [%1 :128]! \n"
							"vld1.f32   {d2-d3}, [%2 :128]! \n"
							"vmax.f32   q0, q0, q1          \n"
							"subs       %0, #1              \n"
							"vst1.f32   {d0-d1}, [%3 :128]! \n"
							"bne        0b                  \n"
							: "=r"(nn),     // %0
							"=r"(ptr),    // %1
							"=r"(ptr1),   // %2
							"=r"(outptr)  // %3
							: "0"(nn),
							"1"(ptr),
							"2"(ptr1),
							"3"(outptr)
							: "cc", "memory", "q0", "q1"
							);
					}
#else
					int remain = out_size;
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
						float val = *ptr;
						if (val < *ptr1) val = *ptr1;
						*outptr = val;

						ptr++;
						ptr1++;
						outptr++;
					}
				}
			}
		}
	}

    void enn_eltwise_sum_pack4_neon(enn_blob* in_blob, enn_blob* in_blob1, enn_blob* out_blob)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int c = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem0 = (float*)in_blob->mem;
		float* in_mem1 = (float*)in_blob1->mem;

		out_blob->w = w;
		out_blob->h = h;
		out_blob->cstep = cstep;
		out_blob->c = c;
		out_blob->pack = 4;
		float* out_mem = (float*)out_blob->mem;

		int size = w * h * c;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
        for (int i = 0; i < size; i++)
		{
			const float* ptr = in_mem0 + i * 4;
			const float* ptr1 = in_mem1 + i * 4;
			float* outptr = out_mem + i * 4;
#if __ARM_NEON
			float32x4_t _p = vld1q_f32(ptr);
			float32x4_t _p1 = vld1q_f32(ptr1);
			_p = vaddq_f32(_p, _p1);
			vst1q_f32(outptr, _p);
#else
			outptr[0] = ptr[0] + ptr1[0];
			outptr[1] = ptr[1] + ptr1[1];
			outptr[2] = ptr[2] + ptr1[2];
			outptr[3] = ptr[3] + ptr1[3];
#endif
		}
	}

