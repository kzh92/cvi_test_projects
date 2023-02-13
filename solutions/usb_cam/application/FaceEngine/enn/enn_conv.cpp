
#if __ARM_NEON
#include <arm_neon.h>
#endif

#include <memory.h>
#include "enn_conv.h"

namespace ENN
{
	void conv1x1s1_sgemm_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp)// tmp /// 8 * inch ( = real_inch / 4) * 4 * (size / 8 + (size % 8) / 4 + size % 4)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = get_blob_size(out_size);
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 1;

		// 		size_t elemsize = bottom_blob.elemsize;
		// 		int elempack = bottom_blob.elempack;

		const int size = w * h;

		// 		elemsize = 16;
		// 		elempack = 4;
		// interleave
		// Mat tmp(8, inch, size / 8 + (size % 8) / 4 + size % 4, elemsize, elempack, opt.workspace_allocator);
		// 8 * inch ( = real_inch / 4) * 4 * (size / 8 + (size % 8) / 4 + size % 4)
#if __ARM_NEON
		{
			int nn_size = 0;
			int remain_size_start = 0;
			nn_size = size >> 3;

#if ((ENGINE_THREAD_COUNT) != 1)
            int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = remain_size_start + ii * 8;

				const float* img0 = in_mem + i * 4; // bottom_blob.channel(0);
				float* tmpptr = tmp + (i / 8) * 32 * inch; // .channel(i / 8);

				for (int q = 0; q < inch; q++)
				{
					// transpose 4x8
					asm volatile(
						"pld        [%0, #256]          \n"
						"vld4.f32   {d0-d3}, [%0 :128]! \n"
						"pld        [%0, #256]          \n"
						"vld4.f32   {d4-d7}, [%0 :128]! \n"
						"pld        [%0, #256]          \n"
						"vld4.f32   {d16-d19}, [%0 :128]! \n"
						"pld        [%0, #256]          \n"
						"vld4.f32   {d20-d23}, [%0 :128] \n"
						"sub        %0, %0, #96         \n"
						"vswp       d1, d4              \n"
						"vswp       d3, d6              \n"
						"vswp       d17, d20            \n"
						"vswp       d19, d22            \n"
						"vst1.f32   {d0-d1}, [%1 :128]! \n"
						"vst1.f32   {d16-d17}, [%1 :128]! \n"
						"vst1.f32   {d4-d5}, [%1 :128]! \n"
						"vst1.f32   {d20-d21}, [%1 :128]! \n"
						"vst1.f32   {d2-d3}, [%1 :128]! \n"
						"vst1.f32   {d18-d19}, [%1 :128]! \n"
						"vst1.f32   {d6-d7}, [%1 :128]! \n"
						"vst1.f32   {d22-d23}, [%1 :128]! \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11"
						);
					img0 += cstep;
				}
			}

			remain_size_start += nn_size << 3;
			nn_size = (size - remain_size_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
            nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = remain_size_start + ii * 4;

				const float* img0 = in_mem + i * 4; // bottom_blob.channel(0);
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4) * 32 * inch; // .channel(i / 8 + (i % 8) / 4);

				for (int q = 0; q < inch; q++)
				{
					// transpose 4x4
					asm volatile(
						"pld        [%0, #256]          \n"
						"vld4.f32   {d0-d3}, [%0 :128]! \n"
						"pld        [%0, #256]          \n"
						"vld4.f32   {d4-d7}, [%0 :128]  \n"
						"sub        %0, %0, #32         \n"
						"vswp       d1, d4              \n"
						"vswp       d3, d6              \n"
						"vst1.f32   {d0-d1}, [%1 :128]! \n"
						"vst1.f32   {d4-d5}, [%1 :128]! \n"
						"vst1.f32   {d2-d3}, [%1 :128]! \n"
						"vst1.f32   {d6-d7}, [%1 :128]! \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "q0", "q1", "q2", "q3"
						);
					img0 += cstep;
				}
			}

			remain_size_start += nn_size << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
            nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = remain_size_start; i < size; i++)
			{
				const float* img0 = in_mem + i * 4; // bottom_blob.channel(0);
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4 + i % 4) * 32 * inch; // .channel(i / 8 + (i % 8) / 4 + i % 4);

				for (int q = 0; q < inch; q++)
				{
					asm volatile(
						"pld        [%0, #128]          \n"
						"vld1.f32   {d0-d1}, [%0 :128]  \n"
						"vst1.f32   {d0-d1}, [%1 :128]! \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "q0"
						);
					img0 += cstep;
				}
			}
		}

		int nn_outch = 0;
		int remain_outch_start = 0;
		nn_outch = outch >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = remain_outch_start + pp * 4;

			float* outptr0 = out_mem + (p + 0) * out_cstep; //top_blob.channel(p);
			float* outptr1 = out_mem + (p + 1) * out_cstep; //top_blob.channel(p + 1);
			float* outptr2 = out_mem + (p + 2) * out_cstep; //top_blob.channel(p + 2);
			float* outptr3 = out_mem + (p + 3) * out_cstep; //top_blob.channel(p + 3);

			const float zeros[4] = { 0.f, 0.f, 0.f, 0.f };
			const float* biasptr = bias ? bias + p : zeros;

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				float* tmpptr = tmp + (i / 8) * 32 * inch; // .channel(i / 8);
				const float* kptr = kernel + (p / 4) * inch * 16; // (const float*)kernel.channel(p / 4);
				int nn = inch;// inch always > 0

				asm volatile(
					"vld1.f32   {d30-d31}, [%14] \n"
					"vdup.f32   q8, d30[0]      \n"
					"vdup.f32   q9, d30[0]      \n"
					"vdup.f32   q10, d30[1]     \n"
					"vdup.f32   q11, d30[1]     \n"
					"vdup.f32   q12, d31[0]     \n"
					"vdup.f32   q13, d31[0]     \n"
					"vdup.f32   q14, d31[1]     \n"
					"vdup.f32   q15, d31[1]     \n"

					"0:                         \n"

					"pld        [%5, #512]      \n"
					"vldm       %5!, {d0-d7}    \n"

					"pld        [%6, #512]      \n"
					"vldm       %6!, {d8-d15}   \n"

					"vmla.f32   q8, q0, d8[0]   \n"
					"vmla.f32   q10, q0, d8[1]  \n"
					"vmla.f32   q12, q0, d9[0]  \n"
					"vmla.f32   q14, q0, d9[1]  \n"
					"vmla.f32   q9, q1, d8[0]   \n"
					"vmla.f32   q11, q1, d8[1]  \n"
					"vmla.f32   q13, q1, d9[0]  \n"
					"vmla.f32   q15, q1, d9[1]  \n"

					"vmla.f32   q8, q2, d10[0]  \n"
					"vmla.f32   q10, q2, d10[1] \n"
					"vmla.f32   q12, q2, d11[0] \n"
					"vmla.f32   q14, q2, d11[1] \n"
					"vmla.f32   q9, q3, d10[0]  \n"
					"vmla.f32   q11, q3, d10[1] \n"
					"vmla.f32   q13, q3, d11[0] \n"
					"vmla.f32   q15, q3, d11[1] \n"

					"pld        [%5, #512]      \n"
					"vldm       %5!, {d0-d7}    \n"

					"vmla.f32   q8, q0, d12[0]  \n"
					"vmla.f32   q10, q0, d12[1] \n"
					"vmla.f32   q12, q0, d13[0] \n"
					"vmla.f32   q14, q0, d13[1] \n"
					"vmla.f32   q9, q1, d12[0]  \n"
					"vmla.f32   q11, q1, d12[1] \n"
					"vmla.f32   q13, q1, d13[0] \n"
					"vmla.f32   q15, q1, d13[1] \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q2, d14[0]  \n"
					"vmla.f32   q10, q2, d14[1] \n"
					"vmla.f32   q12, q2, d15[0] \n"
					"vmla.f32   q14, q2, d15[1] \n"
					"vmla.f32   q9, q3, d14[0]  \n"
					"vmla.f32   q11, q3, d14[1] \n"
					"vmla.f32   q13, q3, d15[0] \n"
					"vmla.f32   q15, q3, d15[1] \n"

					"bne        0b              \n"

					"vst1.f32   {d16-d19}, [%1 :128]! \n"
					"vst1.f32   {d20-d23}, [%2 :128]! \n"
					"vst1.f32   {d24-d27}, [%3 :128]! \n"
					"vst1.f32   {d28-d31}, [%4 :128]! \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(outptr1),    // %2
					"=r"(outptr2),    // %3
					"=r"(outptr3),    // %4
					"=r"(tmpptr),     // %5
					"=r"(kptr)        // %6
					: "0"(nn),
					"1"(outptr0),
					"2"(outptr1),
					"3"(outptr2),
					"4"(outptr3),
					"5"(tmpptr),
					"6"(kptr),
					"r"(biasptr)      // %14
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
			}
			for (; i + 3 < size; i += 4)
			{
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4) * 32 * inch; // .channel(i / 8 + (i % 8) / 4);
				const float* kptr = kernel + (p / 4) * 16 * inch; // (const float*)kernel.channel(p / 4);
				int nn = inch;// inch always > 0

				asm volatile(
					"vld1.f32   {d22-d23}, [%14] \n"
					"vdup.f32   q8, d22[0]      \n"
					"vdup.f32   q9, d22[1]      \n"
					"vdup.f32   q10, d23[0]     \n"
					"vdup.f32   q11, d23[1]     \n"

					"0:                         \n"

					"pld        [%5, #512]      \n"
					"vldm       %5!, {d0-d7}    \n"

					"pld        [%6, #512]      \n"
					"vldm       %6!, {d8-d15}   \n"

					"vmla.f32   q8, q0, d8[0]   \n"
					"vmla.f32   q9, q0, d8[1]   \n"
					"vmla.f32   q10, q0, d9[0]  \n"
					"vmla.f32   q11, q0, d9[1]  \n"

					"vmla.f32   q8, q1, d10[0]  \n"
					"vmla.f32   q9, q1, d10[1]  \n"
					"vmla.f32   q10, q1, d11[0] \n"
					"vmla.f32   q11, q1, d11[1] \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q2, d12[0]  \n"
					"vmla.f32   q9, q2, d12[1]  \n"
					"vmla.f32   q10, q2, d13[0] \n"
					"vmla.f32   q11, q2, d13[1] \n"

					"vmla.f32   q8, q3, d14[0]  \n"
					"vmla.f32   q9, q3, d14[1]  \n"
					"vmla.f32   q10, q3, d15[0] \n"
					"vmla.f32   q11, q3, d15[1] \n"

					"bne        0b              \n"

					"vst1.f32   {d16-d17}, [%1 :128]! \n"
					"vst1.f32   {d18-d19}, [%2 :128]! \n"
					"vst1.f32   {d20-d21}, [%3 :128]! \n"
					"vst1.f32   {d22-d23}, [%4 :128]! \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(outptr1),    // %2
					"=r"(outptr2),    // %3
					"=r"(outptr3),    // %4
					"=r"(tmpptr),     // %5
					"=r"(kptr)        // %6
					: "0"(nn),
					"1"(outptr0),
					"2"(outptr1),
					"3"(outptr2),
					"4"(outptr3),
					"5"(tmpptr),
					"6"(kptr),
					"r"(biasptr)      // %14
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11"
					);
			}
			for (; i < size; i++)
			{
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4 + i % 4) * 32 * inch; // .channel(i / 8 + (i % 8) / 4 + i % 4);
				const float* kptr = kernel + (p / 4) * 16 * inch; // (const float*)kernel.channel(p / 4);
				int nn = inch;// inch always > 0

				asm volatile(
					"vld1.f32   {d16-d17}, [%14] \n"
					"veor       q9, q9          \n"
					"veor       q10, q10        \n"
					"veor       q11, q11        \n"

					"0:                         \n"

					"pld        [%5, #128]      \n"
					"vld1.f32   {d0-d1}, [%5]!  \n"

					"pld        [%6, #512]      \n"
					"vldm       %6!, {d8-d15}   \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q4, d0[0]   \n"
					"vmla.f32   q9, q5, d0[1]   \n"
					"vmla.f32   q10, q6, d1[0]  \n"
					"vmla.f32   q11, q7, d1[1]  \n"

					"bne        0b              \n"

					"vadd.f32   q8, q8, q9      \n"
					"vadd.f32   q10, q10, q11   \n"
					"vadd.f32   q8, q8, q10     \n"

					"vst1.f32   {d16[0]}, [%1]! \n"
					"vst1.f32   {d16[1]}, [%2]! \n"
					"vst1.f32   {d17[0]}, [%3]! \n"
					"vst1.f32   {d17[1]}, [%4]! \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(outptr1),    // %2
					"=r"(outptr2),    // %3
					"=r"(outptr3),    // %4
					"=r"(tmpptr),     // %5
					"=r"(kptr)        // %6
					: "0"(nn),
					"1"(outptr0),
					"2"(outptr1),
					"3"(outptr2),
					"4"(outptr3),
					"5"(tmpptr),
					"6"(kptr),
					"r"(biasptr)      // %14
					: "cc", "memory", "q0", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11"
					);
			}
		}

		remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
        nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* outptr0 = out_mem + p * out_cstep; // top_blob.channel(p);

			const float bias0 = bias ? bias[p] : 0.f;

			int i = 0;
			for (; i + 7 < size; i += 8)
			{
				float* tmpptr = tmp + (i / 8) * 32 * inch; // .channel(i / 8);
				const float* kptr = kernel + (p / 4 + p % 4) * 16 * inch; // (const float*)kernel.channel(p / 4 + p % 4);
				int nn = inch;// inch always > 0

				asm volatile(
					"vdup.f32   q8, %8          \n"
					"vdup.f32   q9, %8          \n"
					"veor       q10, q10        \n"
					"veor       q11, q11        \n"

					"0:                         \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"pld        [%3, #128]      \n"
					"vld1.f32   {d8-d9}, [%3]!  \n"

					"vmla.f32   q8, q0, d8[0]   \n"
					"vmla.f32   q9, q1, d8[0]   \n"
					"vmla.f32   q10, q2, d8[1]  \n"
					"vmla.f32   q11, q3, d8[1]  \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d24-d31}  \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q12, d9[0]  \n"
					"vmla.f32   q9, q13, d9[0]  \n"
					"vmla.f32   q10, q14, d9[1] \n"
					"vmla.f32   q11, q15, d9[1] \n"

					"bne        0b              \n"

					"vadd.f32   q8, q8, q10     \n"
					"vadd.f32   q9, q9, q11     \n"

					"vst1.f32   {d16-d19}, [%1 :128]! \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(tmpptr),     // %2
					"=r"(kptr)        // %3
					: "0"(nn),
					"1"(outptr0),
					"2"(tmpptr),
					"3"(kptr),
					"r"(bias0)        // %8
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
			}
			for (; i + 3 < size; i += 4)
			{
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4) * 32 * inch; // .channel(i / 8 + (i % 8) / 4);
				const float* kptr = kernel + (p / 4 + p % 4) * 16 * inch; // (const float*)kernel.channel(p / 4 + p % 4);
				int nn = inch;// inch always > 0

				asm volatile(
					"vdup.f32   q8, %8          \n"
					"veor       q9, q9          \n"
					"veor       q10, q10        \n"
					"veor       q11, q11        \n"

					"0:                         \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"pld        [%3, #128]      \n"
					"vld1.f32   {d8-d9}, [%3]!  \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q0, d8[0]   \n"
					"vmla.f32   q9, q1, d8[1]   \n"
					"vmla.f32   q10, q2, d9[0]  \n"
					"vmla.f32   q11, q3, d9[1]  \n"

					"bne        0b              \n"

					"vadd.f32   q8, q8, q9      \n"
					"vadd.f32   q10, q10, q11   \n"
					"vadd.f32   q8, q8, q10     \n"

					"vst1.f32   {d16-d17}, [%1]! \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(tmpptr),     // %2
					"=r"(kptr)        // %3
					: "0"(nn),
					"1"(outptr0),
					"2"(tmpptr),
					"3"(kptr),
					"r"(bias0)        // %8
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q8", "q9", "q10", "q11"
					);
			}
			for (; i < size; i++)
			{
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4 + i % 4) * 32 * inch; // .channel(i / 8 + (i % 8) / 4 + i % 4);
				const float* kptr = kernel + (p / 4 + p % 4) * 16 * inch; // (const float*)kernel.channel(p / 4 + p % 4);
				float32x4_t _sum0 = vdupq_n_f32(0.f);

				for (int q = 0; q < inch; q++)
				{
					float32x4_t _r0 = vld1q_f32(tmpptr);

					float32x4_t _k0 = vld1q_f32(kptr);

					_sum0 = vmlaq_f32(_sum0, _r0, _k0);

					kptr += 4;
					tmpptr += 4;
				}

				float32x2_t _ss = vadd_f32(vget_low_f32(_sum0), vget_high_f32(_sum0));
				float32x2_t _ss2 = vpadd_f32(_ss, _ss);
				float sum0 = vget_lane_f32(_ss2, 0);

				outptr0[0] = bias0 + sum0;

				outptr0++;
			}
		}
#else
		int nn_outch = 0;
		nn_outch = outch >> 2;
		int remain_outch_start = nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			float* outptr0 = out_mem + (pp * 4 + 0) * out_cstep;
			float* outptr1 = out_mem + (pp * 4 + 1) * out_cstep;
			float* outptr2 = out_mem + (pp * 4 + 2) * out_cstep;
			float* outptr3 = out_mem + (pp * 4 + 3) * out_cstep;

			for (int i = 0; i < size; i++)
			{
				float sum[4];

				sum[0] = (bias) ? bias[pp * 4 + 0] : 0.0f;
				sum[1] = (bias) ? bias[pp * 4 + 1] : 0.0f;
				sum[2] = (bias) ? bias[pp * 4 + 2] : 0.0f;
				sum[3] = (bias) ? bias[pp * 4 + 3] : 0.0f;

				const float* kptr = kernel + inch * pp * 16;
				for (int q = 0; q < inch; q++)
				{
					const float* sptr = in_mem + q * cstep + i * 4;

					sum[0] += kptr[0] * sptr[0];
					sum[1] += kptr[1] * sptr[0];
					sum[2] += kptr[2] * sptr[0];
					sum[3] += kptr[3] * sptr[0];

					sum[0] += kptr[4] * sptr[1];
					sum[1] += kptr[5] * sptr[1];
					sum[2] += kptr[6] * sptr[1];
					sum[3] += kptr[7] * sptr[1];

					sum[0] += kptr[8] * sptr[2];
					sum[1] += kptr[9] * sptr[2];
					sum[2] += kptr[10] * sptr[2];
					sum[3] += kptr[11] * sptr[2];

					sum[0] += kptr[12] * sptr[3];
					sum[1] += kptr[13] * sptr[3];
					sum[2] += kptr[14] * sptr[3];
					sum[3] += kptr[15] * sptr[3];
					kptr += 16;
				}

				outptr0[i] = sum[0];
				outptr1[i] = sum[1];
				outptr2[i] = sum[2];
				outptr3[i] = sum[3];
			}
		}

#if ((ENGINE_THREAD_COUNT) != 1)
        nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* outptr = out_mem + p * out_cstep;

			for (int i = 0; i < size; i++)
			{
				float sum = (bias) ? bias[p] : 0.0f;
				const float* kptr = kernel + (p / 4 + p % 4) * 16 * inch;
				for (int q = 0; q < inch; q++)
				{
					const float* sptr = in_mem + q * cstep + i * 4;

					sum += kptr[0] * sptr[0];
					sum += kptr[1] * sptr[1];
					sum += kptr[2] * sptr[2];
					sum += kptr[3] * sptr[3];
					kptr += 4;
				}
				outptr[i] = sum;
			}
		}
#endif
	}

	void conv1x1s1_sgemm_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out / 4;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		const int size = w * h;

#if __ARM_NEON
		// Mat tmp(8, inch, size / 8 + (size % 8) / 4 + (size % 4) / 2 + size % 2, elemsize, elempack, opt.workspace_allocator);
		{
			int remain_size_start = 0;
			int nn_size = size >> 3;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = ii * 8;

				const float* img0 = in_mem + i * 4; // bottom_blob.channel(0);
				float* tmpptr = tmp + (i / 8) * inch * 32; // .channel(i / 8);

				for (int q = 0; q < inch; q++)
				{
					asm volatile(
						"pld        [%0, #512]          \n"
						"vldm       %0!, {d0-d7}        \n"
						"pld        [%0, #512]          \n"
						"vldm       %0, {d16-d23}       \n"

						// transpose 8x4
						"vtrn.32    q0, q1              \n"
						"vtrn.32    q2, q3              \n"
						"vtrn.32    q8, q9              \n"
						"vtrn.32    q10, q11            \n"
						"vswp       d1, d4              \n"
						"vswp       d3, d6              \n"
						"vswp       d17, d20            \n"
						"vswp       d19, d22            \n"
						"vswp       q1, q8              \n"
						"vswp       q3, q10             \n"

						"vst1.f32   {d0-d3}, [%1 :128]! \n"
						"vst1.f32   {d16-d19}, [%1 :128]! \n"
						"sub        %0, %0, #64         \n"
						"vst1.f32   {d4-d7}, [%1 :128]! \n"
						"vst1.f32   {d20-d23}, [%1 :128]! \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11"
						);
					img0 += cstep;
				}
			}

			remain_size_start = nn_size << 3;
			nn_size = (size - remain_size_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
            nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = remain_size_start + ii * 4;

				const float* img0 = in_mem + i * 4; // bottom_blob.channel(0);
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4) * inch * 32; // tmp.channel(i / 8 + (i % 8) / 4);

				for (int q = 0; q < inch; q++)
				{
					asm volatile(
						"pld        [%0, #512]          \n"
						"vldm       %0, {d0-d7}         \n"
						"vstm       %1!, {d0-d7}        \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "q0", "q1", "q2", "q3"
						);
					img0 += cstep;
				}
			}

			remain_size_start += nn_size << 2;
			nn_size = (size - remain_size_start) >> 1;

#if ((ENGINE_THREAD_COUNT) != 1)
            nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = remain_size_start + ii * 2;

				const float* img0 = in_mem + i * 4; // bottom_blob.channel(0);
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4 + (i % 4) / 2) * inch * 32; // tmp.channel(i / 8 + (i % 8) / 4 + (i % 4) / 2);

				for (int q = 0; q < inch; q++)
				{
					asm volatile(
						"pld        [%0, #256]          \n"
						"vld1.f32   {d0-d3}, [%0 :128]  \n"
						"vst1.f32   {d0-d3}, [%1 :128]! \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "q0", "q1"
						);
					img0 += cstep;
				}
			}

			remain_size_start += nn_size << 1;

#if ((ENGINE_THREAD_COUNT) != 1)
            nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = remain_size_start; i < size; i++)
			{
				const float* img0 = in_mem + i * 4; // bottom_blob.channel(0);
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4 + (i % 4) / 2 + i % 2) * inch * 32; // tmp.channel(i / 8 + (i % 8) / 4 + (i % 4) / 2 + i % 2);

				for (int q = 0; q < inch; q++)
				{
					asm volatile(
						"pld        [%0, #128]          \n"
						"vld1.f32   {d0-d1}, [%0 :128]  \n"
						"vst1.f32   {d0-d1}, [%1 :128]! \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "q0"
						);
					img0 += cstep;
				}
			}
		}

		//int nn_outch = 0;
		int remain_outch_start = 0;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* outptr0 = out_mem + p * out_cstep; // top_blob.channel(p);

			const float zeros[4] = { 0.f, 0.f, 0.f, 0.f };
			const float* biasptr = bias ? bias + p * 4 : zeros;

			int i = 0;
			for (; i + 7 < size; i += 8)
			{
				float* tmpptr = tmp + (i / 8) * inch * 32; // .channel(i / 8);
				const float* kptr0 = kernel + p * 16 * inch; // (const float*)kernel.channel(p);
				int nn = inch;// inch always > 0
				asm volatile(
					"vld1.f32   {d0-d1}, [%8]   \n"
					"vmov       q8, q0          \n"
					"vmov       q9, q0          \n"
					"vmov       q10, q0         \n"
					"vmov       q11, q0         \n"
					"vmov       q12, q0         \n"
					"vmov       q13, q0         \n"
					"vmov       q14, q0         \n"
					"vmov       q15, q0         \n"

					"0:                         \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"pld        [%3, #512]      \n"
					"vldm       %3!, {d8-d15}   \n"

					"vmla.f32   q8, q4, d0[0]   \n"
					"vmla.f32   q9, q4, d0[1]   \n"
					"vmla.f32   q10, q4, d1[0]  \n"
					"vmla.f32   q11, q4, d1[1]  \n"
					"vmla.f32   q12, q4, d2[0]  \n"
					"vmla.f32   q13, q4, d2[1]  \n"
					"vmla.f32   q14, q4, d3[0]  \n"
					"vmla.f32   q15, q4, d3[1]  \n"

					"vmla.f32   q8, q5, d4[0]   \n"
					"vmla.f32   q9, q5, d4[1]   \n"
					"vmla.f32   q10, q5, d5[0]  \n"
					"vmla.f32   q11, q5, d5[1]  \n"
					"vmla.f32   q12, q5, d6[0]  \n"
					"vmla.f32   q13, q5, d6[1]  \n"
					"vmla.f32   q14, q5, d7[0]  \n"
					"vmla.f32   q15, q5, d7[1]  \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"vmla.f32   q8, q6, d0[0]   \n"
					"vmla.f32   q9, q6, d0[1]   \n"
					"vmla.f32   q10, q6, d1[0]  \n"
					"vmla.f32   q11, q6, d1[1]  \n"
					"vmla.f32   q12, q6, d2[0]  \n"
					"vmla.f32   q13, q6, d2[1]  \n"
					"vmla.f32   q14, q6, d3[0]  \n"
					"vmla.f32   q15, q6, d3[1]  \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q7, d4[0]   \n"
					"vmla.f32   q9, q7, d4[1]   \n"
					"vmla.f32   q10, q7, d5[0]  \n"
					"vmla.f32   q11, q7, d5[1]  \n"
					"vmla.f32   q12, q7, d6[0]  \n"
					"vmla.f32   q13, q7, d6[1]  \n"
					"vmla.f32   q14, q7, d7[0]  \n"
					"vmla.f32   q15, q7, d7[1]  \n"

					"bne        0b              \n"

					"vstm       %1!, {d16-d23}  \n"
					"vstm       %1!, {d24-d31}  \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(tmpptr),     // %2
					"=r"(kptr0)       // %3
					: "0"(nn),
					"1"(outptr0),
					"2"(tmpptr),
					"3"(kptr0),
					"r"(biasptr)      // %8
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
			}
			for (; i + 3 < size; i += 4)
			{
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4) * inch * 32; // .channel(i / 8 + (i % 8) / 4);
				const float* kptr0 = kernel + p * inch * 16; // (const float*)kernel.channel(p);
				int nn = inch;// inch always > 0

				asm volatile(
					"vld1.f32   {d0-d1}, [%8]   \n"
					"vmov       q8, q0          \n"
					"vmov       q9, q0          \n"
					"vmov       q10, q0         \n"
					"vmov       q11, q0         \n"

					"0:                         \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"pld        [%3, #512]      \n"
					"vldm       %3!, {d8-d15}   \n"

					"vmla.f32   q8, q4, d0[0]   \n"
					"vmla.f32   q9, q4, d2[0]   \n"
					"vmla.f32   q10, q4, d4[0]  \n"
					"vmla.f32   q11, q4, d6[0]  \n"

					"vmla.f32   q8, q5, d0[1]   \n"
					"vmla.f32   q9, q5, d2[1]   \n"
					"vmla.f32   q10, q5, d4[1]  \n"
					"vmla.f32   q11, q5, d6[1]  \n"

					"vmla.f32   q8, q6, d1[0]   \n"
					"vmla.f32   q9, q6, d3[0]   \n"
					"vmla.f32   q10, q6, d5[0]  \n"
					"vmla.f32   q11, q6, d7[0]  \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q7, d1[1]   \n"
					"vmla.f32   q9, q7, d3[1]   \n"
					"vmla.f32   q10, q7, d5[1]  \n"
					"vmla.f32   q11, q7, d7[1]  \n"

					"bne        0b              \n"

					"vstm       %1!, {d16-d23}  \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(tmpptr),     // %2
					"=r"(kptr0)       // %3
					: "0"(nn),
					"1"(outptr0),
					"2"(tmpptr),
					"3"(kptr0),
					"r"(biasptr)      // %8
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11"
					);
			}
			for (; i + 1 < size; i += 2)
			{
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4 + (i % 4) / 2) * inch * 32; // .channel(i / 8 + (i % 8) / 4 + (i % 4) / 2);
				const float* kptr0 = kernel + p * inch * 16; // (const float*)kernel.channel(p);

				int nn = inch;// inch always > 0

				asm volatile(
					"vld1.f32   {d0-d1}, [%8]   \n"
					"vmov       q8, q0          \n"
					"vmov       q9, q0          \n"

					"0:                         \n"

					"pld        [%2, #256]      \n"
					"vld1.f32   {d0-d3}, [%2 :128]! \n"

					"pld        [%3, #512]      \n"
					"vldm       %3!, {d8-d15}   \n"

					"vmla.f32   q8, q4, d0[0]   \n"
					"vmla.f32   q9, q4, d2[0]   \n"

					"vmla.f32   q8, q5, d0[1]   \n"
					"vmla.f32   q9, q5, d2[1]   \n"

					"vmla.f32   q8, q6, d1[0]   \n"
					"vmla.f32   q9, q6, d3[0]   \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q7, d1[1]   \n"
					"vmla.f32   q9, q7, d3[1]   \n"

					"bne        0b              \n"

					"vst1.f32   {d16-d19}, [%1 :128]! \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(tmpptr),     // %2
					"=r"(kptr0)       // %3
					: "0"(nn),
					"1"(outptr0),
					"2"(tmpptr),
					"3"(kptr0),
					"r"(biasptr)      // %8
					: "cc", "memory", "q0", "q1", "q4", "q5", "q6", "q7", "q8", "q9"
					);
			}
			for (; i < size; i++)
			{
				float* tmpptr = tmp + (i / 8 + (i % 8) / 4 + (i % 4) / 2 + i % 2) * inch * 32; // .channel(i / 8 + (i % 8) / 4 + (i % 4) / 2 + i % 2);
				const float* kptr0 = kernel + p * inch * 16; // (const float*)kernel.channel(p);
				int nn = inch;// inch always > 0

				asm volatile(
					"vld1.f32   {d16-d17}, [%8] \n"

					"0:                         \n"

					"pld        [%2, #128]      \n"
					"vld1.f32   {d0-d1}, [%2 :128]! \n"

					"pld        [%3, #512]      \n"
					"vldm       %3!, {d8-d15}   \n"

					"vmla.f32   q8, q4, d0[0]   \n"
					"vmla.f32   q8, q5, d0[1]   \n"

					"subs       %0, %0, #1      \n"

					"vmla.f32   q8, q6, d1[0]   \n"
					"vmla.f32   q8, q7, d1[1]   \n"

					"bne        0b              \n"

					"vst1.f32   {d16-d17}, [%1 :128]! \n"

					: "=r"(nn),         // %0
					"=r"(outptr0),    // %1
					"=r"(tmpptr),     // %2
					"=r"(kptr0)       // %3
					: "0"(nn),
					"1"(outptr0),
					"2"(tmpptr),
					"3"(kptr0),
					"r"(biasptr)      // %8
					: "cc", "memory", "q0", "q4", "q5", "q6", "q7", "q8"
					);
			}
		}
#else
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* outptr = out_mem + p * out_cstep; // top_blob.channel(p);

			for (int i = 0; i < size; i++)
			{
				float sum[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

				if (bias)
				{
					sum[0] = bias[p * 4];
					sum[1] = bias[p * 4 + 1];
					sum[2] = bias[p * 4 + 2];
					sum[3] = bias[p * 4 + 3];
				}

				const float* kptr = kernel + inch * p * 16;

				for (int q = 0; q < inch; q++)
				{
					const float* sptr = in_mem + q * cstep + i * 4;

					sum[0] += kptr[0] * sptr[0];
					sum[1] += kptr[1] * sptr[0];
					sum[2] += kptr[2] * sptr[0];
					sum[3] += kptr[3] * sptr[0];

					sum[0] += kptr[4] * sptr[1];
					sum[1] += kptr[5] * sptr[1];
					sum[2] += kptr[6] * sptr[1];
					sum[3] += kptr[7] * sptr[1];

					sum[0] += kptr[8] * sptr[2];
					sum[1] += kptr[9] * sptr[2];
					sum[2] += kptr[10] * sptr[2];
					sum[3] += kptr[11] * sptr[2];

					sum[0] += kptr[12] * sptr[3];
					sum[1] += kptr[13] * sptr[3];
					sum[2] += kptr[14] * sptr[3];
					sum[3] += kptr[15] * sptr[3];

					kptr += 16;
				}

				outptr[0] = sum[0];
				outptr[1] = sum[1];
				outptr[2] = sum[2];
				outptr[3] = sum[3];
				outptr += 4;
			}
		}
#endif
	}

	void conv3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out / 4;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		int kernel_cstep = 144 * inch;

		// 		size_t elemsize = bottom_blob.elemsize;
		// 		int elempack = bottom_blob.elempack;

		//const int size = w * h;

#if __ARM_NEON
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* out0 = out_mem + p * out_cstep; // top_blob.channel(p);

			float32x4_t _bias0 = bias ? vld1q_f32((const float*)bias + p * 4) : vdupq_n_f32(0.f);
			// out0.fill(_bias0);
			{
				float* ptr = out0;
				for (int t = 0; t < out_size; t++)
				{
					vst1q_f32(ptr, _bias0);
					ptr += 4;
				}
			}

			for (int q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0 + 0 * w; //img0.row(0);
				const float* r1 = img0 + 4 * w; //img0.row(1);
				const float* r2 = img0 + 8 * w; //img0.row(2);
												// 				const float* r3 = img0 + 12 * w; //img0.row(3);
												// 				const float* r4 = img0 + 16 * w; //img0.row(4);

				const float* kptr = kernel + p * kernel_cstep + q * 144; // (const float*)kernel.channel(p).row(q);

				int i = 0;
				for (; i < outh; i++)
				{
					int j = 0;
					for (; j + 3 < outw; j += 4)
					{
						asm volatile(
							"pld        [%0, #512]          \n"
							"vldm       %0, {d24-d31}       \n"// sum0 sum1 sum2 sum3

							"pld        [%1, #512]          \n"
							"vldm       %1!, {d0-d7}        \n"// r00 r01 r02 r03

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d8-d11}        \n"// r04 r05

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"pld        [%2, #512]          \n"
							"vldm       %2!, {d0-d7}        \n"// r10 r11 r12 r13

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d8-d11}        \n"// r14 r15

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"pld        [%3, #512]          \n"
							"vldm       %3!, {d0-d7}        \n"// r20 r21 r22 r23

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d8-d11}        \n"// r24 r25

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4, {d16-d23}       \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"sub        %4, %4, #512       \n"// kptr -= 8 * 16;

							"vstm       %0!, {d24-d31}      \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(kptr)        // %4
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(kptr)
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
					for (; j + 1 < outw; j += 2)
					{
						asm volatile(
							"pld        [%0, #256]          \n"
							"vld1.f32   {d24-d27}, [%0 :128] \n"// sum0 sum1

							"pld        [%1, #256]          \n"
							"vld1.f32   {d0-d3}, [%1 :128]! \n"// r00 r01

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmul.f32   q14, q8, d0[0]      \n"
							"vmul.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d4-d7}         \n"// r02 r03

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"pld        [%2, #256]          \n"
							"vld1.f32   {d0-d3}, [%2 :128]! \n"// r10 r11

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d4-d7}         \n"// r12 r13

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"pld        [%3, #256]          \n"
							"vld1.f32   {d0-d3}, [%3 :128]! \n"// r20 r21

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d4-d7}         \n"// r22 r23

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4, {d16-d23}       \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"vadd.f32   q12, q12, q14       \n"
							"vadd.f32   q13, q13, q15       \n"

							"sub        %4, %4, #512        \n"// kptr -= 8 * 16;

							"vst1.f32   {d24-d27}, [%0 :128]! \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(kptr)        // %4
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(kptr)
							: "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
					for (; j < outw; j++)
					{
						asm volatile(
							"pld        [%0, #128]          \n"
							"vld1.f32   {d24-d25}, [%0 :128] \n"// sum0

							"pld        [%1, #128]          \n"
							"vld1.f32   {d0-d1}, [%1 :128]! \n"// r00

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmul.f32   q13, q8, d0[0]      \n"
							"vmul.f32   q14, q9, d0[1]      \n"
							"vmul.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d2-d5}         \n"// r01 r02

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%2, #128]          \n"
							"vld1.f32   {d0-d1}, [%2 :128]! \n"// r10

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d2-d5}         \n"// r11 r12

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%3, #128]          \n"
							"vld1.f32   {d0-d1}, [%3 :128]! \n"// r20

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d16-d23}      \n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d2-d5}         \n"// r21 r22

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							"vldm       %4, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"vadd.f32   q13, q13, q14       \n"
							"vadd.f32   q12, q12, q15       \n"
							"vadd.f32   q12, q12, q13       \n"

							"sub        %4, %4, #512       \n"// kptr -= 8 * 16;

							"vst1.f32   {d24-d25}, [%0 :128]! \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(kptr)        // %4
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(kptr)
							: "memory", "q0", "q1", "q2", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
					r0 += 2 * 4;
					r1 += 2 * 4;
					r2 += 2 * 4;
				}
			}
		}
#else
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			for (int i = 0; i < outh; i++)
			{
				for (int j = 0; j < outw; j++)
				{
					float sum0, sum1, sum2, sum3;
					if (bias)
					{
						sum0 = bias[p * 4];
						sum1 = bias[p * 4 + 1];
						sum2 = bias[p * 4 + 2];
						sum3 = bias[p * 4 + 3];
					}
					else
					{
						sum0 = 0.0f;
						sum1 = 0.0f;
						sum2 = 0.0f;
						sum3 = 0.0f;
					}
					for (int q = 0; q < inch; q++)
					{
						for (int ki = 0; ki < dim_kernel; ki++)
						{
							for (int kj = 0; kj < dim_kernel; kj++)
							{
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 0];
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 4];
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 8];
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 12];

								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 1];
								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 5];
								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 9];
								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 13];

								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 2];
								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 6];
								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 10];
								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 14];

								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 3];
								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 7];
								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 11];
								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 144 * inch + q * 144 + ki * 48 + kj * 16 + 15];
							}
						}
					}
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 0] = sum0;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 1] = sum1;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 2] = sum2;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 3] = sum3;
				}
			}
		}
#endif
	}

	void conv3x3s1_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = get_blob_size(out_size);
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 1;

// 		int inch = bottom_blob.c;
// 		int outw = top_blob.w;
// 		int outh = top_blob.h;
// 		int outch = top_blob.c;

#if __ARM_NEON
		//int nn_outch = 0;
		int remain_outch_start = 0;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* out0 = out_mem + p * out_cstep; // Mat out0 = top_blob.channel(p);

			const float bias0 = bias ? bias[p] : 0.f;
			{ float* ptr = out0; for (int t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out0.fill(bias0);

			// const float* k0 = kernel.channel(p);
			const float* k0 = kernel + p * (36 * inch); // ????

			for (int q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				const float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0; // img0.row(0);
				const float* r1 = r0 + w * 4; // img0.row(1);
				const float* r2 = r1 + w * 4; // img0.row(2);

				float32x4_t _k00 = vld1q_f32(k0);
				float32x4_t _k01 = vld1q_f32(k0 + 4);
				float32x4_t _k02 = vld1q_f32(k0 + 8);
				float32x4_t _k10 = vld1q_f32(k0 + 12);
				float32x4_t _k11 = vld1q_f32(k0 + 16);
				float32x4_t _k12 = vld1q_f32(k0 + 20);
				float32x4_t _k20 = vld1q_f32(k0 + 24);
				float32x4_t _k21 = vld1q_f32(k0 + 28);
				float32x4_t _k22 = vld1q_f32(k0 + 32);

				int i = 0;

				for (; i < outh; i++)
				{
					int j = 0;
					for (; j + 3 < outw; j += 4)
					{
						asm volatile(
							"pld        [%1, #256]      \n"
							"vld1.f32   {d0-d3}, [%1 :128]! \n"// r00 r01

							"vmul.f32   q3, %q8, q0     \n"

							"pld        [%1, #128]      \n"
							"vld1.f32   {d4-d5}, [%1 :128]! \n"// r02

							"vmul.f32   q4, %q8, q1     \n"
							"vmla.f32   q3, %q9, q1     \n"

							"pld        [%1, #256]      \n"
							"vld1.f32   {d0-d3}, [%1 :128]! \n"// r03 r04

							"vmul.f32   q5, %q8, q2     \n"
							"vmla.f32   q4, %q9, q2     \n"
							"vmla.f32   q3, %q10, q2    \n"

							"vmul.f32   q6, %q8, q0     \n"
							"vmla.f32   q5, %q9, q0     \n"
							"vmla.f32   q4, %q10, q0    \n"

							"pld        [%1, #128]      \n"
							"vld1.f32   {d4-d5}, [%1 :128] \n"// r05

							"vmla.f32   q6, %q9, q1     \n"
							"vmla.f32   q5, %q10, q1    \n"

							"pld        [%2, #256]      \n"
							"vld1.f32   {d0-d3}, [%2 :128]! \n"// r10 r11

							"vmla.f32   q6, %q10, q2    \n"

							"vmla.f32   q3, %q11, q0    \n"

							"pld        [%2, #128]      \n"
							"vld1.f32   {d4-d5}, [%2 :128]! \n"// r12

							"vmla.f32   q4, %q11, q1    \n"
							"vmla.f32   q3, %q12, q1    \n"

							"pld        [%2, #256]      \n"
							"vld1.f32   {d0-d3}, [%2 :128]! \n"// r13 r14

							"vmla.f32   q5, %q11, q2    \n"
							"vmla.f32   q4, %q12, q2    \n"
							"vmla.f32   q3, %q13, q2    \n"

							"vmla.f32   q6, %q11, q0    \n"
							"vmla.f32   q5, %q12, q0    \n"
							"vmla.f32   q4, %q13, q0    \n"

							"pld        [%2, #128]      \n"
							"vld1.f32   {d4-d5}, [%2 :128] \n"// r15

							"vmla.f32   q6, %q12, q1    \n"
							"vmla.f32   q5, %q13, q1    \n"

							"pld        [%3, #256]      \n"
							"vld1.f32   {d0-d3}, [%3 :128]! \n"// r20 r21

							"vmla.f32   q6, %q13, q2    \n"

							"vmla.f32   q3, %q14, q0    \n"

							"pld        [%3, #128]      \n"
							"vld1.f32   {d4-d5}, [%3 :128]! \n"// r22

							"vmla.f32   q4, %q14, q1    \n"
							"vmla.f32   q3, %q15, q1    \n"

							"pld        [%3, #256]      \n"
							"vld1.f32   {d0-d3}, [%3 :128]! \n"// r23 r24

							"vmla.f32   q5, %q14, q2    \n"
							"vmla.f32   q4, %q15, q2    \n"
							"vmla.f32   q3, %q16, q2    \n"

							"vmla.f32   q6, %q14, q0    \n"
							"vmla.f32   q5, %q15, q0    \n"
							"vmla.f32   q4, %q16, q0    \n"

							"pld        [%3, #128]      \n"
							"vld1.f32   {d4-d5}, [%3 :128] \n"// r25

							"vmla.f32   q6, %q15, q1    \n"
							"vmla.f32   q5, %q16, q1    \n"

							"vld1.f32   {d0-d1}, [%0]   \n"// sum0 sum1 sum2 sum3

							"vmla.f32   q6, %q16, q2    \n"

							"vadd.f32   d6, d6, d7      \n"
							"vadd.f32   d8, d8, d9      \n"
							"vadd.f32   d10, d10, d11   \n"
							"vadd.f32   d12, d12, d13   \n"

							"sub        %1, %1, #16     \n"

							"vpadd.f32  d6, d6, d8      \n"
							"vpadd.f32  d7, d10, d12    \n"

							"sub        %2, %2, #16     \n"

							"vadd.f32   q0, q0, q3      \n"

							"sub        %3, %3, #16     \n"

							"vst1.f32   {d0-d1}, [%0]!  \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2)          // %3
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"w"(_k00),        // %8
							"w"(_k01),        // %9
							"w"(_k02),        // %10
							"w"(_k10),        // %11
							"w"(_k11),        // %12
							"w"(_k12),        // %13
							"w"(_k20),        // %14
							"w"(_k21),        // %15
							"w"(_k22)         // %16
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6"
							);
					}
					for (; j + 1 < outw; j += 2)
					{
						asm volatile(
							"pld        [%1, #256]      \n"
							"vld1.f32   {d0-d3}, [%1 :128]! \n"// r00 r01

							"vmul.f32   q5, %q8, q0     \n"
							"vmul.f32   q6, %q8, q1     \n"
							"vmul.f32   q2, %q9, q1     \n"

							"pld        [%1, #256]      \n"
							"vld1.f32   {d0-d3}, [%1 :128] \n"// r02 r03

							"vmul.f32   q3, %q9, q0     \n"
							"vmla.f32   q5, %q10, q0    \n"
							"vmla.f32   q6, %q10, q1    \n"

							"pld        [%2, #256]      \n"
							"vld1.f32   {d0-d3}, [%2 :128]! \n"// r10 r11

							"vmla.f32   q2, %q11, q0    \n"
							"vmla.f32   q3, %q11, q1    \n"
							"vmla.f32   q5, %q12, q1    \n"

							"pld        [%2, #256]      \n"
							"vld1.f32   {d0-d3}, [%2 :128] \n"// r12 r13

							"vmla.f32   q6, %q12, q0    \n"
							"vmla.f32   q2, %q13, q0    \n"
							"vmla.f32   q3, %q13, q1    \n"

							"pld        [%3, #256]      \n"
							"vld1.f32   {d0-d3}, [%3 :128]! \n"// r20 r21

							"vmla.f32   q5, %q14, q0    \n"
							"vmla.f32   q6, %q14, q1    \n"
							"vmla.f32   q2, %q15, q1    \n"

							"pld        [%3, #256]      \n"
							"vld1.f32   {d0-d3}, [%3 :128] \n"// r22 r23

							"vmla.f32   q3, %q15, q0    \n"
							"vmla.f32   q5, %q16, q0    \n"
							"vmla.f32   q6, %q16, q1    \n"

							"vld1.f32   {d8}, [%0]      \n"// sum0 sum1

							"vadd.f32   q5, q5, q2      \n"
							"vadd.f32   q6, q6, q3      \n"

							"vadd.f32   d10, d10, d11   \n"
							"vadd.f32   d12, d12, d13   \n"

							"vpadd.f32  d10, d10, d12   \n"

							"vadd.f32   d8, d8, d10     \n"

							"vst1.f32   {d8}, [%0]!     \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2)          // %3
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"w"(_k00),        // %8
							"w"(_k01),        // %9
							"w"(_k02),        // %10
							"w"(_k10),        // %11
							"w"(_k11),        // %12
							"w"(_k12),        // %13
							"w"(_k20),        // %14
							"w"(_k21),        // %15
							"w"(_k22)         // %16
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6"
							);
					}
					for (; j < outw; j++)
					{
						asm volatile(
							"pld        [%1, #384]      \n"
							"vldm       %1, {d0-d5}     \n"// r00 r01 r02

							"veor       q3, q3          \n"
							"vld1.f32   {d6[0]}, [%0]   \n"// sum0

							"vmul.f32   q4, %q8, q0     \n"
							"vmul.f32   q5, %q9, q1     \n"
							"vmla.f32   q3, %q10, q2    \n"

							"pld        [%2, #384]      \n"
							"vldm       %2, {d0-d5}     \n"// r10 r11 r12

							"vmla.f32   q4, %q11, q0    \n"
							"vmla.f32   q5, %q12, q1    \n"
							"vmla.f32   q3, %q13, q2    \n"

							"pld        [%3, #384]      \n"
							"vldm       %3, {d0-d5}     \n"// r20 r21 r22

							"vmla.f32   q4, %q14, q0    \n"
							"vmla.f32   q5, %q15, q1    \n"
							"vmla.f32   q3, %q16, q2    \n"

							"vadd.f32   q4, q4, q5      \n"
							"vadd.f32   q3, q3, q4      \n"

							"add        %1, %1, #16     \n"

							"vadd.f32   d6, d6, d7      \n"

							"add        %2, %2, #16     \n"

							"vpadd.f32  d6, d6, d6      \n"

							"add        %3, %3, #16     \n"

							"vst1.f32   {d6[0]}, [%0]!  \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2)          // %3
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"w"(_k00),        // %8
							"w"(_k01),        // %9
							"w"(_k02),        // %10
							"w"(_k10),        // %11
							"w"(_k11),        // %12
							"w"(_k12),        // %13
							"w"(_k20),        // %14
							"w"(_k21),        // %15
							"w"(_k22)         // %16
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5"
							);
					}

					r0 += 2 * 4;
					r1 += 2 * 4;
					r2 += 2 * 4;
				}

				k0 += 9 * 4;
			}
		}
#else
#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
		#pragma omp parallel for num_threads(nThreadCount)
		#endif
		for (int p = 0; p < outch; p++)
		{
			float* outptr = out_mem + p * out_cstep; // top_blob.channel(p);

			for (int i = 0; i < outh; i++)
			{
				for (int j = 0; j < outw; j++)
				{
					float sum = 0.f;
					if (bias)
					{
						sum = bias[p];
					}

					const float* kptr = (const float*)kernel + 4 * 9 * inch * p;

					for (int q = 0; q < inch; q++)
					{
						float* m = in_mem + q * cstep;
						const float* sptr = m + i * w * 4 + j * 4;

						for (int ky = 0; ky < dim_kernel; ky++)
						{
							for (int kx = 0; kx < dim_kernel; kx++)
							{
								sum += sptr[(ky * w + kx) * 4] * kptr[0]
									+ sptr[(ky * w + kx) * 4 + 1] * kptr[1]
									+ sptr[(ky * w + kx) * 4 + 2] * kptr[2]
									+ sptr[(ky * w + kx) * 4 + 3] * kptr[3];
								kptr += 4;
							}
						}
					}
					outptr[j] = sum;	
				}
				outptr += outw;
			}
		}
#endif
	}

	void conv3x3s2_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out / 4;
		int kernel_cstep = 36 * inch;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		const int tailstep = w - 2 * outw + w;

		//int nn_outch = 0;
		int remain_outch_start = 0;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* out0 = out_mem + p * out_cstep; // Mat out0 = top_blob.channel(p);

#if __ARM_NEON
			float32x4_t _bias0 = bias ? vld1q_f32((const float*)bias + p * 4) : vdupq_n_f32(0.f);
			{
				float* ptr = out0;
				for (int t = 0; t < out_size; t++)
				{
					vst1q_f32(ptr, _bias0);
					ptr += 4;
				}
			}
#else
			{
				if (bias)
				{
					for (int t = 0; t < out_size; t++)
						memcpy(out0 + t * 4, bias + p * 4, 16);
				}
				else
				{
					memset(out0, 0, out_size * 16);
				}
			}
#endif
			const float* k0 = kernel + p * kernel_cstep; // .channel(p);

			for (int q = 0; q < inch; q++)
			{
				float* outptr0 = out0;

				float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* r0 = img0 + 0 * w; // .row(0);
				const float* r1 = img0 + 1 * w; // .row(1);
				const float* r2 = img0 + 2 * w; // .row(2);
#if __ARM_NEON
				float32x4_t _k00 = vld1q_f32(k0);
				float32x4_t _k01 = vld1q_f32(k0 + 4);
				float32x4_t _k02 = vld1q_f32(k0 + 8);
				float32x4_t _k10 = vld1q_f32(k0 + 12);
				float32x4_t _k11 = vld1q_f32(k0 + 16);
				float32x4_t _k12 = vld1q_f32(k0 + 20);
				float32x4_t _k20 = vld1q_f32(k0 + 24);
				float32x4_t _k21 = vld1q_f32(k0 + 28);
				float32x4_t _k22 = vld1q_f32(k0 + 32);
#endif
				int i = 0;

				for (; i < outh; i++)
				{
#if __ARM_NEON
					int nn = outw >> 2;
					int remain = outw & 3;
					if (nn > 0)
					{
						asm volatile(
							"0:                             \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d0-d7}         \n"// sum0

															   // r0
							"pld        [%2, #256]          \n"
							"vld1.f32   {d8-d11}, [%2]!     \n"
							"vld1.f32   {d12[]}, [%2]       \n"

							"vmla.f32   q0, %q10, d8[0]     \n"
							"vmla.f32   q1, %q10, d9[0]     \n"
							"vmla.f32   q2, %q10, d10[0]    \n"
							"vmla.f32   q3, %q10, d11[0]    \n"


							"vmla.f32   q0, %q11, d8[1]     \n"
							"vmla.f32   q1, %q11, d9[1]     \n"
							"vmla.f32   q2, %q11, d10[1]    \n"
							"vmla.f32   q3, %q11, d11[1]    \n"

							"vmla.f32   q0, %q12, d9[0]     \n"
							"vmla.f32   q1, %q12, d10[0]    \n"
							"vmla.f32   q2, %q12, d11[0]    \n"

							// r1
							"pld        [%3, #256]          \n"
							"vld1.f32   {d8-d11}, [%3]!     \n"
							"vld1.f32   {d13[]}, [%3]       \n"

							"vmla.f32   q3, %q12, d12[0]    \n"

							"vmla.f32   q0, %q13, d8[0]     \n"
							"vmla.f32   q1, %q13, d9[0]     \n"
							"vmla.f32   q2, %q13, d10[0]    \n"
							"vmla.f32   q3, %q13, d11[0]    \n"


							"vmla.f32   q0, %q14, d8[1]     \n"
							"vmla.f32   q1, %q14, d9[1]     \n"
							"vmla.f32   q2, %q14, d10[1]    \n"
							"vmla.f32   q3, %q14, d11[1]    \n"

							"vmla.f32   q0, %q15, d9[0]     \n"
							"vmla.f32   q1, %q15, d10[0]    \n"
							"vmla.f32   q2, %q15, d11[0]    \n"

							// r2
							"pld        [%4, #256]          \n"
							"vld1.f32   {d8-d11}, [%4]!     \n"
							"vld1.f32   {d12[]}, [%4]       \n"

							"vmla.f32   q3, %q15, d13[0]    \n"

							"vmla.f32   q0, %q16, d8[0]     \n"
							"vmla.f32   q1, %q16, d9[0]     \n"
							"vmla.f32   q2, %q16, d10[0]    \n"
							"vmla.f32   q3, %q16, d11[0]    \n"


							"vmla.f32   q0, %q17, d8[1]     \n"
							"vmla.f32   q1, %q17, d9[1]     \n"
							"vmla.f32   q2, %q17, d10[1]    \n"
							"vmla.f32   q3, %q17, d11[1]    \n"

							"vmla.f32   q0, %q18, d9[0]     \n"
							"vmla.f32   q1, %q18, d10[0]    \n"
							"vmla.f32   q2, %q18, d11[0]    \n"
							"vmla.f32   q3, %q18, d12[0]    \n"

							"subs       %0, %0, #1          \n"

							"vstm       %1!, {d0-d7}        \n"

							"bne        0b                  \n"

							: "=r"(nn),         // %0
							"=r"(outptr0),    // %1
							"=r"(r0),         // %2
							"=r"(r1),         // %3
							"=r"(r2)          // %4
							: "0"(nn),
							"1"(outptr0),
							"2"(r0),
							"3"(r1),
							"4"(r2),
							"w"(_k00),        // %10
							"w"(_k01),        // %11
							"w"(_k02),        // %12
							"w"(_k10),        // %13
							"w"(_k11),        // %14
							"w"(_k12),        // %15
							"w"(_k20),        // %16
							"w"(_k21),        // %17
							"w"(_k22)         // %18
							: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6"
							);
					}
#else
					int remain = outw;
#endif
					for (; remain > 0; remain--)
					{
#if __ARM_NEON
						float32x4_t _sum0 = vld1q_f32(outptr0);

						float32x4_t _r0 = vld1q_f32(r0);
						float32x4_t _r1 = vld1q_f32(r1);
						float32x4_t _r2 = vld1q_f32(r2);

						_sum0 = vmlaq_lane_f32(_sum0, _k00, vget_low_f32(_r0), 0);
						_sum0 = vmlaq_lane_f32(_sum0, _k01, vget_low_f32(_r0), 1);
						_sum0 = vmlaq_lane_f32(_sum0, _k02, vget_high_f32(_r0), 0);
						_sum0 = vmlaq_lane_f32(_sum0, _k10, vget_low_f32(_r1), 0);
						_sum0 = vmlaq_lane_f32(_sum0, _k11, vget_low_f32(_r1), 1);
						_sum0 = vmlaq_lane_f32(_sum0, _k12, vget_high_f32(_r1), 0);
						_sum0 = vmlaq_lane_f32(_sum0, _k20, vget_low_f32(_r2), 0);
						_sum0 = vmlaq_lane_f32(_sum0, _k21, vget_low_f32(_r2), 1);
						_sum0 = vmlaq_lane_f32(_sum0, _k22, vget_high_f32(_r2), 0);

						vst1q_f32(outptr0, _sum0);
#else
						outptr0[0] += k0[0] * r0[0];
						outptr0[1] += k0[1] * r0[0];
						outptr0[2] += k0[2] * r0[0];
						outptr0[3] += k0[3] * r0[0];

						outptr0[0] += k0[4] * r0[1];
						outptr0[1] += k0[5] * r0[1];
						outptr0[2] += k0[6] * r0[1];
						outptr0[3] += k0[7] * r0[1];

						outptr0[0] += k0[8] * r0[2];
						outptr0[1] += k0[9] * r0[2];
						outptr0[2] += k0[10] * r0[2];
						outptr0[3] += k0[11] * r0[2];

						outptr0[0] += k0[12] * r1[0];
						outptr0[1] += k0[13] * r1[0];
						outptr0[2] += k0[14] * r1[0];
						outptr0[3] += k0[15] * r1[0];

						outptr0[0] += k0[16] * r1[1];
						outptr0[1] += k0[17] * r1[1];
						outptr0[2] += k0[18] * r1[1];
						outptr0[3] += k0[19] * r1[1];

						outptr0[0] += k0[20] * r1[2];
						outptr0[1] += k0[21] * r1[2];
						outptr0[2] += k0[22] * r1[2];
						outptr0[3] += k0[23] * r1[2];

						outptr0[0] += k0[24] * r2[0];
						outptr0[1] += k0[25] * r2[0];
						outptr0[2] += k0[26] * r2[0];
						outptr0[3] += k0[27] * r2[0];

						outptr0[0] += k0[28] * r2[1];
						outptr0[1] += k0[29] * r2[1];
						outptr0[2] += k0[30] * r2[1];
						outptr0[3] += k0[31] * r2[1];

						outptr0[0] += k0[32] * r2[2];
						outptr0[1] += k0[33] * r2[2];
						outptr0[2] += k0[34] * r2[2];
						outptr0[3] += k0[35] * r2[2];
#endif
						r0 += 2;
						r1 += 2;
						r2 += 2;
						outptr0 += 4;
					}

					r0 += tailstep;
					r1 += tailstep;
					r2 += tailstep;
				}

				k0 += 9 * 4;
			}
		}
	}

	void conv5x5s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out / 4;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		int kernel_cstep = 400 * inch;

		// 		size_t elemsize = bottom_blob.elemsize;
		// 		int elempack = bottom_blob.elempack;

		//const int size = w * h;

#if __ARM_NEON
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* out0 = out_mem + p * out_cstep; // top_blob.channel(p);

			float32x4_t _bias0 = bias ? vld1q_f32((const float*)bias + p * 4) : vdupq_n_f32(0.f);
			// out0.fill(_bias0);
			{
				float* ptr = out0;
				for (int t = 0; t < out_size; t++)
				{
					vst1q_f32(ptr, _bias0);
					ptr += 4;
				}
			}

			for (int q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0 + 0 * w; //img0.row(0);
				const float* r1 = img0 + 4 * w; //img0.row(1);
				const float* r2 = img0 + 8 * w; //img0.row(2);
				const float* r3 = img0 + 12 * w; //img0.row(3);
				const float* r4 = img0 + 16 * w; //img0.row(4);

				const float* kptr = kernel + p * kernel_cstep + q * 400; // (const float*)kernel.channel(p).row(q);

				int i = 0;
				for (; i < outh; i++)
				{
					int j = 0;
					for (; j + 3 < outw; j += 4)
					{
						asm volatile(
							"pld        [%0, #512]          \n"
							"vldm       %0, {d24-d31}       \n"// sum0 sum1 sum2 sum3

							"pld        [%1, #512]          \n"
							"vldm       %1!, {d0-d7}        \n"// r00 r01 r02 r03

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d8-d15}        \n"// r04 r05 r06 r07

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d6[0]      \n"
							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q8, d10[0]     \n"
							"vmla.f32   q15, q8, d12[0]     \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q9, d10[1]     \n"
							"vmla.f32   q15, q9, d12[1]     \n"
							"vmla.f32   q12, q10, d7[0]     \n"
							"vmla.f32   q13, q10, d9[0]     \n"
							"vmla.f32   q14, q10, d11[0]    \n"
							"vmla.f32   q15, q10, d13[0]    \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"
							"vmla.f32   q14, q11, d11[1]    \n"
							"vmla.f32   q15, q11, d13[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%2, #512]          \n"
							"vldm       %2!, {d0-d7}        \n"// r10 r11 r12 r13

							"vmla.f32   q12, q8, d8[0]      \n"
							"vmla.f32   q13, q8, d10[0]     \n"
							"vmla.f32   q14, q8, d12[0]     \n"
							"vmla.f32   q15, q8, d14[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q9, d12[1]     \n"
							"vmla.f32   q15, q9, d14[1]     \n"
							"vmla.f32   q12, q10, d9[0]     \n"
							"vmla.f32   q13, q10, d11[0]    \n"
							"vmla.f32   q14, q10, d13[0]    \n"
							"vmla.f32   q15, q10, d15[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"
							"vmla.f32   q14, q11, d13[1]    \n"
							"vmla.f32   q15, q11, d15[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d8-d15}        \n"// r14 r15 r16 r17

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d6[0]      \n"
							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q8, d10[0]     \n"
							"vmla.f32   q15, q8, d12[0]     \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q9, d10[1]     \n"
							"vmla.f32   q15, q9, d12[1]     \n"
							"vmla.f32   q12, q10, d7[0]     \n"
							"vmla.f32   q13, q10, d9[0]     \n"
							"vmla.f32   q14, q10, d11[0]    \n"
							"vmla.f32   q15, q10, d13[0]    \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"
							"vmla.f32   q14, q11, d11[1]    \n"
							"vmla.f32   q15, q11, d13[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%3, #512]          \n"
							"vldm       %3!, {d0-d7}        \n"// r20 r21 r22 r23

							"vmla.f32   q12, q8, d8[0]      \n"
							"vmla.f32   q13, q8, d10[0]     \n"
							"vmla.f32   q14, q8, d12[0]     \n"
							"vmla.f32   q15, q8, d14[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q9, d12[1]     \n"
							"vmla.f32   q15, q9, d14[1]     \n"
							"vmla.f32   q12, q10, d9[0]     \n"
							"vmla.f32   q13, q10, d11[0]    \n"
							"vmla.f32   q14, q10, d13[0]    \n"
							"vmla.f32   q15, q10, d15[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"
							"vmla.f32   q14, q11, d13[1]    \n"
							"vmla.f32   q15, q11, d15[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d8-d15}        \n"// r24 r25 r26 r27

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d6[0]      \n"
							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q8, d10[0]     \n"
							"vmla.f32   q15, q8, d12[0]     \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q9, d10[1]     \n"
							"vmla.f32   q15, q9, d12[1]     \n"
							"vmla.f32   q12, q10, d7[0]     \n"
							"vmla.f32   q13, q10, d9[0]     \n"
							"vmla.f32   q14, q10, d11[0]    \n"
							"vmla.f32   q15, q10, d13[0]    \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"
							"vmla.f32   q14, q11, d11[1]    \n"
							"vmla.f32   q15, q11, d13[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d0-d7}        \n"// r30 r31 r32 r33

							"vmla.f32   q12, q8, d8[0]      \n"
							"vmla.f32   q13, q8, d10[0]     \n"
							"vmla.f32   q14, q8, d12[0]     \n"
							"vmla.f32   q15, q8, d14[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q9, d12[1]     \n"
							"vmla.f32   q15, q9, d14[1]     \n"
							"vmla.f32   q12, q10, d9[0]     \n"
							"vmla.f32   q13, q10, d11[0]    \n"
							"vmla.f32   q14, q10, d13[0]    \n"
							"vmla.f32   q15, q10, d15[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"
							"vmla.f32   q14, q11, d13[1]    \n"
							"vmla.f32   q15, q11, d15[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%4, #512]          \n"
							"vldm       %4, {d8-d15}        \n"// r34 r35 r36 r37

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d6[0]      \n"
							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q8, d10[0]     \n"
							"vmla.f32   q15, q8, d12[0]     \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q9, d10[1]     \n"
							"vmla.f32   q15, q9, d12[1]     \n"
							"vmla.f32   q12, q10, d7[0]     \n"
							"vmla.f32   q13, q10, d9[0]     \n"
							"vmla.f32   q14, q10, d11[0]    \n"
							"vmla.f32   q15, q10, d13[0]    \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"
							"vmla.f32   q14, q11, d11[1]    \n"
							"vmla.f32   q15, q11, d13[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%5, #512]          \n"
							"vldm       %5!, {d0-d7}        \n"// r40 r41 r42 r43

							"vmla.f32   q12, q8, d8[0]      \n"
							"vmla.f32   q13, q8, d10[0]     \n"
							"vmla.f32   q14, q8, d12[0]     \n"
							"vmla.f32   q15, q8, d14[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q9, d12[1]     \n"
							"vmla.f32   q15, q9, d14[1]     \n"
							"vmla.f32   q12, q10, d9[0]     \n"
							"vmla.f32   q13, q10, d11[0]    \n"
							"vmla.f32   q14, q10, d13[0]    \n"
							"vmla.f32   q15, q10, d15[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"
							"vmla.f32   q14, q11, d13[1]    \n"
							"vmla.f32   q15, q11, d15[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q9, d6[1]      \n"
							"vmla.f32   q12, q10, d1[0]     \n"
							"vmla.f32   q13, q10, d3[0]     \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"
							"vmla.f32   q14, q11, d5[1]     \n"
							"vmla.f32   q15, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%5, #512]          \n"
							"vldm       %5, {d8-d15}        \n"// r44 r45 r46 r47

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q9, d8[1]      \n"
							"vmla.f32   q12, q10, d3[0]     \n"
							"vmla.f32   q13, q10, d5[0]     \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"
							"vmla.f32   q14, q11, d7[1]     \n"
							"vmla.f32   q15, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d4[0]      \n"
							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q9, d10[1]     \n"
							"vmla.f32   q12, q10, d5[0]     \n"
							"vmla.f32   q13, q10, d7[0]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"
							"vmla.f32   q14, q11, d9[1]     \n"
							"vmla.f32   q15, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q12, q8, d6[0]      \n"
							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q8, d10[0]     \n"
							"vmla.f32   q15, q8, d12[0]     \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q9, d10[1]     \n"
							"vmla.f32   q15, q9, d12[1]     \n"
							"vmla.f32   q12, q10, d7[0]     \n"
							"vmla.f32   q13, q10, d9[0]     \n"
							"vmla.f32   q14, q10, d11[0]    \n"
							"vmla.f32   q15, q10, d13[0]    \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"
							"vmla.f32   q14, q11, d11[1]    \n"
							"vmla.f32   q15, q11, d13[1]    \n"

							//                         "pld        [%6, #512]          \n"
							"vldm       %6, {d16-d23}       \n"

							"vmla.f32   q12, q8, d8[0]      \n"
							"vmla.f32   q13, q8, d10[0]     \n"
							"vmla.f32   q14, q8, d12[0]     \n"
							"vmla.f32   q15, q8, d14[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q9, d12[1]     \n"
							"vmla.f32   q15, q9, d14[1]     \n"
							"vmla.f32   q12, q10, d9[0]     \n"
							"vmla.f32   q13, q10, d11[0]    \n"
							"vmla.f32   q14, q10, d13[0]    \n"
							"vmla.f32   q15, q10, d15[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"
							"vmla.f32   q14, q11, d13[1]    \n"
							"vmla.f32   q15, q11, d15[1]    \n"

							"sub        %6, %6, #1536       \n"// kptr -= 24 * 16;

							"vstm       %0!, {d24-d31}      \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(r3),         // %4
							"=r"(r4),         // %5
							"=r"(kptr)        // %6
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(r3),
							"5"(r4),
							"6"(kptr)
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
					for (; j + 1 < outw; j += 2)
					{
						asm volatile(
							"pld        [%0, #256]          \n"
							"vld1.f32   {d24-d27}, [%0 :128] \n"// sum0 sum1

							"pld        [%1, #256]          \n"
							"vld1.f32   {d0-d3}, [%1 :128]! \n"// r00 r01

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmul.f32   q14, q8, d0[0]      \n"
							"vmul.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d4-d11}        \n"// r02 r03 r04 r05

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%2, #256]          \n"
							"vld1.f32   {d0-d3}, [%2 :128]! \n"// r10 r11

							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d4-d11}        \n"// r12 r13 r14 r15

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%3, #256]          \n"
							"vld1.f32   {d0-d3}, [%3 :128]! \n"// r20 r21

							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d4-d11}        \n"// r22 r23 r24 r25

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%4, #256]          \n"
							"vld1.f32   {d0-d3}, [%4 :128]! \n"// r30 r31

							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%4, #512]          \n"
							"vldm       %4, {d4-d11}        \n"// r32 r33 r34 r35

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%5, #256]          \n"
							"vld1.f32   {d0-d3}, [%5 :128]! \n"// r40 r41

							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%5, #512]          \n"
							"vldm       %5, {d4-d11}        \n"// r42 r43 r44 r45

							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d4[0]      \n"
							"vmla.f32   q12, q9, d2[1]      \n"
							"vmla.f32   q13, q9, d4[1]      \n"
							"vmla.f32   q14, q10, d3[0]     \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"
							"vmla.f32   q13, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d4[0]      \n"
							"vmla.f32   q15, q8, d6[0]      \n"
							"vmla.f32   q12, q9, d4[1]      \n"
							"vmla.f32   q13, q9, d6[1]      \n"
							"vmla.f32   q14, q10, d5[0]     \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"
							"vmla.f32   q13, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q14, q8, d6[0]      \n"
							"vmla.f32   q15, q8, d8[0]      \n"
							"vmla.f32   q12, q9, d6[1]      \n"
							"vmla.f32   q13, q9, d8[1]      \n"
							"vmla.f32   q14, q10, d7[0]     \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"
							"vmla.f32   q13, q11, d9[1]     \n"

							//                         "pld        [%6, #512]          \n"
							"vldm       %6, {d16-d23}       \n"

							"vmla.f32   q14, q8, d8[0]      \n"
							"vmla.f32   q15, q8, d10[0]     \n"
							"vmla.f32   q12, q9, d8[1]      \n"
							"vmla.f32   q13, q9, d10[1]     \n"
							"vmla.f32   q14, q10, d9[0]     \n"
							"vmla.f32   q15, q10, d11[0]    \n"
							"vmla.f32   q12, q11, d9[1]     \n"
							"vmla.f32   q13, q11, d11[1]    \n"

							"vadd.f32   q12, q12, q14       \n"
							"vadd.f32   q13, q13, q15       \n"

							"sub        %6, %6, #1536       \n"// kptr -= 24 * 16;

							"vst1.f32   {d24-d27}, [%0 :128]! \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(r3),         // %4
							"=r"(r4),         // %5
							"=r"(kptr)        // %6
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(r3),
							"5"(r4),
							"6"(kptr)
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
					for (; j < outw; j++)
					{
						asm volatile(
							"pld        [%0, #128]          \n"
							"vld1.f32   {d24-d25}, [%0 :128] \n"// sum0

							"pld        [%1, #128]          \n"
							"vld1.f32   {d0-d1}, [%1 :128]! \n"// r00

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmul.f32   q13, q8, d0[0]      \n"
							"vmul.f32   q14, q9, d0[1]      \n"
							"vmul.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d2-d9}         \n"// r01 r02 r03 r04

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%2, #128]          \n"
							"vld1.f32   {d0-d1}, [%2 :128]! \n"// r10

							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d2-d9}         \n"// r11 r12 r13 r14

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%3, #128]          \n"
							"vld1.f32   {d0-d1}, [%3 :128]! \n"// r20

							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d2-d9}         \n"// r21 r22 r23 r24

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%4, #128]          \n"
							"vld1.f32   {d0-d1}, [%4 :128]! \n"// r30

							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%4, #512]          \n"
							"vldm       %4, {d2-d9}         \n"// r31 r32 r33 r34

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%5, #128]          \n"
							"vld1.f32   {d0-d1}, [%5 :128]! \n"// r40

							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d9[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"pld        [%5, #512]          \n"
							"vldm       %5, {d2-d9}         \n"// r41 r42 r43 r44

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%6, #512]          \n"
							"vldm       %6!, {d16-d23}      \n"

							"vmla.f32   q13, q8, d6[0]      \n"
							"vmla.f32   q14, q9, d6[1]      \n"
							"vmla.f32   q15, q10, d7[0]     \n"
							"vmla.f32   q12, q11, d7[1]     \n"

							//                         "pld        [%6, #512]          \n"
							"vldm       %6, {d16-d23}       \n"

							"vmla.f32   q13, q8, d8[0]      \n"
							"vmla.f32   q14, q9, d8[1]      \n"
							"vmla.f32   q15, q10, d9[0]     \n"
							"vmla.f32   q12, q11, d9[1]     \n"

							"vadd.f32   q13, q13, q14       \n"
							"vadd.f32   q12, q12, q15       \n"
							"vadd.f32   q12, q12, q13       \n"

							"sub        %6, %6, #1536       \n"// kptr -= 24 * 16;

							"vst1.f32   {d24-d25}, [%0 :128]! \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(r3),         // %4
							"=r"(r4),         // %5
							"=r"(kptr)        // %6
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(r3),
							"5"(r4),
							"6"(kptr)
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
					r0 += 4 * 4;
					r1 += 4 * 4;
					r2 += 4 * 4;
					r3 += 4 * 4;
					r4 += 4 * 4;
				}
			}
		}
#else
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			for (int i = 0; i < outh; i++)
			{
				for (int j = 0; j < outw; j++)
				{
					float sum0, sum1, sum2, sum3;
					if (bias)
					{
						sum0 = bias[p * 4];
						sum1 = bias[p * 4 + 1];
						sum2 = bias[p * 4 + 2];
						sum3 = bias[p * 4 + 3];
					}
					else
					{
						sum0 = 0.0f;
						sum1 = 0.0f;
						sum2 = 0.0f;
						sum3 = 0.0f;
					}
					for (int q = 0; q < inch; q++)
					{
						for (int ki = 0; ki < dim_kernel; ki++)
						{
							for (int kj = 0; kj < dim_kernel; kj++)
							{
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 0];
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 4];
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 8];
								sum0 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 12];

								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 1];
								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 5];
								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 9];
								sum1 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 13];

								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 2];
								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 6];
								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 10];
								sum2 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 14];

								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 0] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 3];
								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 1] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 7];
								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 2] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 11];
								sum3 += in_mem[q * cstep + (i + ki) * w * 4 + (j + kj) * 4 + 3] * kernel[p * 400 * inch + q * 400 + ki * 80 + kj * 16 + 15];
							}
						}
					}

					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 0] = sum0;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 1] = sum1;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 2] = sum2;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 3] = sum3;
				}
			}
		}
#endif
	}

	void conv5x5s1_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out / 4;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		int kernel_cstep = 100 * inch;

		// 		size_t elemsize = bottom_blob.elemsize;
		// 		int elempack = bottom_blob.elempack;

		//const int size = w * h;

#if __ARM_NEON
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* out0 = out_mem + p * out_cstep; // top_blob.channel(p);

			float32x4_t _bias0 = bias ? vld1q_f32((const float*)bias + p * 4) : vdupq_n_f32(0.f);
			// out0.fill(_bias0);
			{
				float* ptr = out0;
				for (int t = 0; t < out_size; t++)
				{
					vst1q_f32(ptr, _bias0);
					ptr += 4;
				}
			}

			for (int q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0; //img0.row(0);
				const float* r1 = img0 + 1 * w; //img0.row(1);
				const float* r2 = img0 + 2 * w; //img0.row(2);
				const float* r3 = img0 + 3 * w; //img0.row(3);
				const float* r4 = img0 + 4 * w; //img0.row(4);

				const float* kptr = kernel + p * kernel_cstep + q * 100; // (const float*)kernel.channel(p).row(q);

				int i = 0;
				for (; i < outh; i++)
				{
					int j = 0;
					for (; j + 3 < outw; j += 4)
					{
						// vld1.f32   {d16-d17}, [%6 :128]!
						asm volatile(
							"pld        [%0, #512]          \n"
							"vldm       %0, {d24-d31}       \n"// sum0 sum1 sum2 sum3

							"pld        [%1, #512]          \n"
							"vldm       %1!, {d0-d1}        \n"// r00 r01 r02 r03

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"
							"vmla.f32   q14, q8, d1[0]      \n"
							"vmla.f32   q15, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d2-d3}        \n"// r04 r05 r06 r07

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"
							"vmla.f32   q14, q8, d1[1]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"
							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d2[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d2[1]      \n"
							"vmla.f32   q15, q8, d3[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%2, #512]          \n"
							"vldm       %2!, {d0-d1}        \n"// r10 r11 r12 r13

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"
							"vmla.f32   q14, q8, d3[0]      \n"
							"vmla.f32   q15, q8, d3[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"
							"vmla.f32   q14, q8, d1[0]      \n"
							"vmla.f32   q15, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d2-d3}         \n"// r14 r15 r16 r17

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"
							"vmla.f32   q14, q8, d1[1]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"
							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d2[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d2[1]      \n"
							"vmla.f32   q15, q8, d3[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%3, #512]          \n"
							"vldm       %3!, {d0-d1}        \n"// r20 r21 r22 r23

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"
							"vmla.f32   q14, q8, d3[0]      \n"
							"vmla.f32   q15, q8, d3[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"
							"vmla.f32   q14, q8, d1[0]      \n"
							"vmla.f32   q15, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d2-d3}         \n"// r24 r25 r26 r27

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"
							"vmla.f32   q14, q8, d1[1]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"
							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d2[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d2[1]      \n"
							"vmla.f32   q15, q8, d3[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%4, #512]          \n"
							"vldm       %4!, {d0-d1}        \n"// r30 r31 r32 r33

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"
							"vmla.f32   q14, q8, d3[0]      \n"
							"vmla.f32   q15, q8, d3[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"
							"vmla.f32   q14, q8, d1[0]      \n"
							"vmla.f32   q15, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%4, #512]          \n"
							"vldm       %4, {d2-d3}         \n"// r34 r35 r36 r37

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"
							"vmla.f32   q14, q8, d1[1]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"
							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d2[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d2[1]      \n"
							"vmla.f32   q15, q8, d3[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%5, #512]          \n"
							"vldm       %5!, {d0-d1}        \n"// r40 r41 r42 r43

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"
							"vmla.f32   q14, q8, d3[0]      \n"
							"vmla.f32   q15, q8, d3[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"
							"vmla.f32   q14, q8, d1[0]      \n"
							"vmla.f32   q15, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"pld        [%5, #512]          \n"
							"vldm       %5, {d2-d3}        \n"// r44 r45 r46 r47

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"
							"vmla.f32   q14, q8, d1[1]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"
							"vmla.f32   q14, q8, d2[0]      \n"
							"vmla.f32   q15, q8, d2[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q8, d2[1]      \n"
							"vmla.f32   q15, q8, d3[0]      \n"

							//                         "pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]  \n"

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"
							"vmla.f32   q14, q8, d3[0]      \n"
							"vmla.f32   q15, q8, d3[1]      \n"

							"sub        %6, %6, #384        \n"// kptr -= 24 * 4;

							"vstm       %0!, {d24-d31}      \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(r3),         // %4
							"=r"(r4),         // %5
							"=r"(kptr)        // %6
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(r3),
							"5"(r4),
							"6"(kptr)
							: "memory", "q0", "q1", "q8", "q12", "q13", "q14", "q15"
							);
					}
					for (; j + 1 < outw; j += 2)
					{
						asm volatile(
							"pld        [%0, #256]          \n"
							"vld1.f32   {d24-d27}, [%0 :128] \n"// sum0 sum1

							"pld        [%1, #256]          \n"
							"vld1.f32   {d0-d2}, [%1]!      \n"// r00 r01 r02 r03 r04 r05

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"

							"pld        [%2, #256]          \n"
							"vld1.f32   {d0-d2}, [%2]!      \n"// r10 r11 r12 r13 r14 r15

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"

							"pld        [%3, #256]          \n"
							"vld1.f32   {d0-d2}, [%3]!      \n"// r20 r21 r22 r23 r24 r25

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"

							"pld        [%4, #256]          \n"
							"vld1.f32   {d0-d3}, [%4]!      \n"// r30 r31 r32 r33 r34 r35

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"

							"pld        [%5, #256]          \n"
							"vld1.f32   {d0-d3}, [%5]!      \n"// r40 r41 r42 r43 r44 r45

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"
							"vmla.f32   q13, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"
							"vmla.f32   q13, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"
							"vmla.f32   q13, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"
							"vmla.f32   q13, q8, d2[0]      \n"

							//                         "pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128] \n"

							"vmla.f32   q12, q8, d2[0]      \n"
							"vmla.f32   q13, q8, d2[1]      \n"

							"sub        %6, %6, #1536       \n"// kptr -= 24 * 16;

							"vst1.f32   {d24-d27}, [%0 :128]! \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(r3),         // %4
							"=r"(r4),         // %5
							"=r"(kptr)        // %6
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(r3),
							"5"(r4),
							"6"(kptr)
							: "memory", "q0", "q1", "q8", "q12", "q13"
							);
					}
					for (; j < outw; j++)
					{
						asm volatile(
							"pld        [%0, #128]          \n"
							"vld1.f32   {d24-d25}, [%0 :128]! \n"// sum0

							"pld        [%1, #128]          \n"
							"vld1.f32   {d0-d2}, [%1]!      \n"// r00 r01 r02 r03 r04

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmul.f32   q12, q8, d0[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"

							"pld        [%2, #128]          \n"
							"vld1.f32   {d0-d2}, [%2]       \n"// r10 r11 r12 r13 r14

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"

							"pld        [%3, #128]          \n"
							"vld1.f32   {d0-d1}, [%3]!      \n"// r20 r21 r22 r23 r24

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"

							"pld        [%4, #128]          \n"
							"vld1.f32   {d0-d1}, [%4]!      \n"// r30 r31 r32 r33 r34

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d2[0]      \n"

							"pld        [%5, #128]          \n"
							"vld1.f32   {d0-d2}, [%5]!      \n"// r40 r41 r42 r43 r44

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d0[1]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[0]      \n"

							"pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128]! \n"

							"vmla.f32   q12, q8, d1[1]      \n"

							//                         "pld        [%6, #512]          \n"
							"vld1.f32   {d16-d17}, [%6 :128] \n"

							"vmla.f32   q12, q8, d2[0]      \n"

							"sub        %6, %6, #384        \n"// kptr -= 24 * 16;

							"vst1.f32   {d24-d25}, [%0 :128]! \n"

							: "=r"(outptr0),    // %0
							"=r"(r0),         // %1
							"=r"(r1),         // %2
							"=r"(r2),         // %3
							"=r"(r3),         // %4
							"=r"(r4),         // %5
							"=r"(kptr)        // %6
							: "0"(outptr0),
							"1"(r0),
							"2"(r1),
							"3"(r2),
							"4"(r3),
							"5"(r4),
							"6"(kptr)
							: "memory", "q0", "q1", "q8", "q12"
							);
					}
					r0 += 4;
					r1 += 4;
					r2 += 4;
					r3 += 4;
					r4 += 4;
				}
			}
		}
#else
#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			for (int i = 0; i < outh; i++)
			{
				for (int j = 0; j < outw; j++)
				{
					float sum0, sum1, sum2, sum3;
					if (bias)
					{
						sum0 = bias[p * 4];
						sum1 = bias[p * 4 + 1];
						sum2 = bias[p * 4 + 2];
						sum3 = bias[p * 4 + 3];
					}
					else
					{
						sum0 = 0.0f;
						sum1 = 0.0f;
						sum2 = 0.0f;
						sum3 = 0.0f;
					}
					for (int q = 0; q < inch; q++)
					{
						for (int ki = 0; ki < dim_kernel; ki++)
						{
							for (int kj = 0; kj < dim_kernel; kj++)
							{
								sum0 += in_mem[q * cstep + (i + ki) * w + (j + kj)] * kernel[p * 100 * inch + q * 100 + ki * 20 + kj * 4 + 0];
								sum1 += in_mem[q * cstep + (i + ki) * w + (j + kj)] * kernel[p * 100 * inch + q * 100 + ki * 20 + kj * 4 + 1];
								sum2 += in_mem[q * cstep + (i + ki) * w + (j + kj)] * kernel[p * 100 * inch + q * 100 + ki * 20 + kj * 4 + 2];
								sum3 += in_mem[q * cstep + (i + ki) * w + (j + kj)] * kernel[p * 100 * inch + q * 100 + ki * 20 + kj * 4 + 3];
							}
						}
					}
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 0] = sum0;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 1] = sum1;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 2] = sum2;
					out_mem[p * out_cstep + i * outw * 4 + j * 4 + 3] = sum3;
				}
			}
		}
#endif
	}

	void convdw3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = inch;
		int kernel_cstep = 36;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		const int group = inch;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < group; g++)
		{
			float* out = out_mem + g * out_cstep; // Mat out = top_blob.channel(g);

#if __ARM_NEON
			float32x4_t _bias0 = bias ? vld1q_f32((const float*)bias + g * 4) : vdupq_n_f32(0.f);
#endif
			const float* k0 = kernel + g * kernel_cstep; // kernel.row(g);

			float* outptr0 = out;				// out.row(0);
			//float* outptr1 = out + outw * 4;	// out.row(1);

			float* img0 = in_mem + g * cstep; // const Mat img0 = bottom_blob.channel(g);

			const float* r0 = img0;					// .row(0);
			const float* r1 = img0 + 4 * w;			// .row(1);
			const float* r2 = img0 + 8 * w;			// .row(2);
			//const float* r3 = img0 + 12 * w;		// .row(3);
#if __ARM_NEON
			float32x4_t _k00 = vld1q_f32(k0);
			float32x4_t _k01 = vld1q_f32(k0 + 4);
			float32x4_t _k02 = vld1q_f32(k0 + 8);
			float32x4_t _k10 = vld1q_f32(k0 + 12);
			float32x4_t _k11 = vld1q_f32(k0 + 16);
			float32x4_t _k12 = vld1q_f32(k0 + 20);
			float32x4_t _k20 = vld1q_f32(k0 + 24);
			float32x4_t _k21 = vld1q_f32(k0 + 28);
			float32x4_t _k22 = vld1q_f32(k0 + 32);
#endif
			int i = 0;
			for (; i < outh; i++)
			{
				int j = 0;
#if __ARM_NEON
				for (; j + 3 < outw; j += 4)
				{
					asm volatile(
						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128]! \n"// r00 r01

						"vmov       q10, %q17       \n"// sum00
						"vmov       q11, %q17       \n"// sum01

						"vmla.f32   q10, %q8, q14   \n"
						"vmla.f32   q11, %q8, q15   \n"
						"vmla.f32   q10, %q9, q15   \n"

						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128]! \n"// r02 r03

						"vmov       q12, %q17       \n"// sum02
						"vmov       q13, %q17       \n"// sum03

						"vmla.f32   q12, %q8, q14   \n"
						"vmla.f32   q11, %q9, q14   \n"
						"vmla.f32   q13, %q8, q15   \n"
						"vmla.f32   q10, %q10, q14  \n"
						"vmla.f32   q12, %q9, q15   \n"
						"vmla.f32   q11, %q10, q15  \n"

						//                     "pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128] \n"// r04 r05

						"vmla.f32   q13, %q9, q14   \n"
						"vmla.f32   q12, %q10, q14  \n"
						"vmla.f32   q13, %q10, q15  \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128]! \n"// r10 r11

						"vmla.f32   q10, %q11, q14  \n"
						"vmla.f32   q11, %q11, q15  \n"
						"vmla.f32   q10, %q12, q15  \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128]! \n"// r12 r13

						"vmla.f32   q12, %q11, q14  \n"
						"vmla.f32   q11, %q12, q14  \n"
						"vmla.f32   q13, %q11, q15  \n"
						"vmla.f32   q10, %q13, q14  \n"
						"vmla.f32   q12, %q12, q15  \n"
						"vmla.f32   q11, %q13, q15  \n"

						//                     "pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128] \n"// r14 r15

						"vmla.f32   q13, %q12, q14  \n"
						"vmla.f32   q12, %q13, q14  \n"
						"vmla.f32   q13, %q13, q15  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128]! \n"// r20 r21

						"vmla.f32   q10, %q14, q14  \n"
						"vmla.f32   q11, %q14, q15  \n"
						"vmla.f32   q10, %q15, q15  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128]! \n"// r22 r23

						"vmla.f32   q12, %q14, q14  \n"
						"vmla.f32   q11, %q15, q14  \n"
						"vmla.f32   q13, %q14, q15  \n"
						"vmla.f32   q10, %q16, q14  \n"
						"vmla.f32   q12, %q15, q15  \n"
						"vmla.f32   q11, %q16, q15  \n"

						//                     "pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128] \n"// r24 r25

						"vmla.f32   q13, %q15, q14  \n"
						"vmla.f32   q12, %q16, q14  \n"
						"vmla.f32   q13, %q16, q15  \n"

						"vstm       %0!, {d20-d27}  \n"

						: "=r"(outptr0),    // %0
						"=r"(r0),         // %1
						"=r"(r1),         // %2
						"=r"(r2)          // %3
						: "0"(outptr0),
						"1"(r0),
						"2"(r1),
						"3"(r2),
						"w"(_k00),        // %8
						"w"(_k01),        // %9
						"w"(_k02),        // %10
						"w"(_k10),        // %11
						"w"(_k11),        // %12
						"w"(_k12),        // %13
						"w"(_k20),        // %14
						"w"(_k21),        // %15
						"w"(_k22),        // %16
						"w"(_bias0)       // %17
						: "memory", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
				for (; j + 1 < outw; j += 2)
				{
					asm volatile(
						"pld        [%1, #256]      \n"
						"vld1.f32   {d24-d27}, [%1 :128]! \n"// r00 r01

						"vmov       q10, %q17       \n"// sum00
						"vmov       q11, %q17       \n"// sum01

						"vmla.f32   q10, %q8, q12   \n"
						"vmla.f32   q11, %q8, q13   \n"

						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128] \n"// r02 r03

						"vmla.f32   q10, %q9, q13   \n"

						"vmla.f32   q11, %q9, q14   \n"
						"vmla.f32   q10, %q10, q14  \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d24-d27}, [%2 :128]! \n"// r10 r11

						"vmla.f32   q11, %q10, q15  \n"

						"vmla.f32   q10, %q11, q12  \n"
						"vmla.f32   q11, %q11, q13  \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128] \n"// r12 r13

						"vmla.f32   q10, %q12, q13  \n"

						"vmla.f32   q11, %q12, q14  \n"
						"vmla.f32   q10, %q13, q14  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d24-d27}, [%3 :128]! \n"// r20 r21

						"vmla.f32   q11, %q13, q15  \n"

						"vmla.f32   q10, %q14, q12  \n"
						"vmla.f32   q11, %q14, q13  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128] \n"// r22 r23

						"vmla.f32   q10, %q15, q13  \n"

						"vmla.f32   q11, %q15, q14  \n"
						"vmla.f32   q10, %q16, q14  \n"
						"vmla.f32   q11, %q16, q15  \n"

						"vst1.f32   {d20-d23}, [%0 :128]! \n"

						: "=r"(outptr0),    // %0
						"=r"(r0),         // %1
						"=r"(r1),         // %2
						"=r"(r2)          // %3
						: "0"(outptr0),
						"1"(r0),
						"2"(r1),
						"3"(r2),
						"w"(_k00),        // %8
						"w"(_k01),        // %9
						"w"(_k02),        // %10
						"w"(_k10),        // %11
						"w"(_k11),        // %12
						"w"(_k12),        // %13
						"w"(_k20),        // %14
						"w"(_k21),        // %15
						"w"(_k22),        // %16
						"w"(_bias0)       // %17
						: "memory", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#endif
				for (; j < outw; j++)
				{
#if __ARM_NEON
					float32x4_t _sum0 = _bias0;

					float32x4_t _r00 = vld1q_f32(r0);
					float32x4_t _r01 = vld1q_f32(r0 + 4);
					float32x4_t _r02 = vld1q_f32(r0 + 8);
					float32x4_t _r10 = vld1q_f32(r1);
					float32x4_t _r11 = vld1q_f32(r1 + 4);
					float32x4_t _r12 = vld1q_f32(r1 + 8);
					float32x4_t _r20 = vld1q_f32(r2);
					float32x4_t _r21 = vld1q_f32(r2 + 4);
					float32x4_t _r22 = vld1q_f32(r2 + 8);

					_sum0 = vmlaq_f32(_sum0, _k00, _r00);
					_sum0 = vmlaq_f32(_sum0, _k01, _r01);
					_sum0 = vmlaq_f32(_sum0, _k02, _r02);
					_sum0 = vmlaq_f32(_sum0, _k10, _r10);
					_sum0 = vmlaq_f32(_sum0, _k11, _r11);
					_sum0 = vmlaq_f32(_sum0, _k12, _r12);
					_sum0 = vmlaq_f32(_sum0, _k20, _r20);
					_sum0 = vmlaq_f32(_sum0, _k21, _r21);
					_sum0 = vmlaq_f32(_sum0, _k22, _r22);

					vst1q_f32(outptr0, _sum0);
#else
					if (bias)
					{
						outptr0[0] = *(bias + g * 4 + 0);
						outptr0[1] = *(bias + g * 4 + 1);
						outptr0[2] = *(bias + g * 4 + 2);
						outptr0[3] = *(bias + g * 4 + 3);
					}
					else
					{
						outptr0[0] = 0.0f;
						outptr0[1] = 0.0f;
						outptr0[2] = 0.0f;
						outptr0[3] = 0.0f;
					}

					outptr0[0] += k0[0] * r0[0];
					outptr0[1] += k0[1] * r0[1];
					outptr0[2] += k0[2] * r0[2];
					outptr0[3] += k0[3] * r0[3];

					outptr0[0] += k0[4] * r0[4];
					outptr0[1] += k0[5] * r0[5];
					outptr0[2] += k0[6] * r0[6];
					outptr0[3] += k0[7] * r0[7];

					outptr0[0] += k0[8] * r0[8];
					outptr0[1] += k0[9] * r0[9];
					outptr0[2] += k0[10] * r0[10];
					outptr0[3] += k0[11] * r0[11];

					outptr0[0] += k0[12] * r1[0];
					outptr0[1] += k0[13] * r1[1];
					outptr0[2] += k0[14] * r1[2];
					outptr0[3] += k0[15] * r1[3];

					outptr0[0] += k0[16] * r1[4];
					outptr0[1] += k0[17] * r1[5];
					outptr0[2] += k0[18] * r1[6];
					outptr0[3] += k0[19] * r1[7];

					outptr0[0] += k0[20] * r1[8];
					outptr0[1] += k0[21] * r1[9];
					outptr0[2] += k0[22] * r1[10];
					outptr0[3] += k0[23] * r1[11];

					outptr0[0] += k0[24] * r2[0];
					outptr0[1] += k0[25] * r2[1];
					outptr0[2] += k0[26] * r2[2];
					outptr0[3] += k0[27] * r2[3];

					outptr0[0] += k0[28] * r2[4];
					outptr0[1] += k0[29] * r2[5];
					outptr0[2] += k0[30] * r2[6];
					outptr0[3] += k0[31] * r2[7];

					outptr0[0] += k0[32] * r2[8];
					outptr0[1] += k0[33] * r2[9];
					outptr0[2] += k0[34] * r2[10];
					outptr0[3] += k0[35] * r2[11];
#endif
					r0 += 4;
					r1 += 4;
					r2 += 4;
					outptr0 += 4;
				}

				r0 += 2 * 4;
				r1 += 2 * 4;
				r2 += 2 * 4;
			}
		}
	}

	void convdw3x3s2_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = inch;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		//int elemsize = 16;
		//int elempack = 4;

		const int group = inch;

		const int tailstep = (w - 2 * outw + w) * 4;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < group; g++)
		{
			float* out = out_mem + g * out_cstep; // Mat out = top_blob.channel(g);

#if __ARM_NEON
			float32x4_t _bias0 = bias ? vld1q_f32((const float*)bias + g * 4) : vdupq_n_f32(0.f);
#endif
			const float* k0 = kernel + g * 36; // .row(g);

			float* outptr0 = out;

			float* img0 = in_mem + g * cstep; // const Mat img0 = bottom_blob.channel(g);

			const float* r0 = img0; //.row(0);
			const float* r1 = img0 + 4 * w; //.row(1);
			const float* r2 = img0 + 8 * w; //.row(2);

#if __ARM_NEON
			float32x4_t _k00 = vld1q_f32(k0);
			float32x4_t _k01 = vld1q_f32(k0 + 4);
			float32x4_t _k02 = vld1q_f32(k0 + 8);
			float32x4_t _k10 = vld1q_f32(k0 + 12);
			float32x4_t _k11 = vld1q_f32(k0 + 16);
			float32x4_t _k12 = vld1q_f32(k0 + 20);
			float32x4_t _k20 = vld1q_f32(k0 + 24);
			float32x4_t _k21 = vld1q_f32(k0 + 28);
			float32x4_t _k22 = vld1q_f32(k0 + 32);
#endif
			int i = 0;

			for (; i < outh; i++)
			{
				int j = 0;
#if __ARM_NEON
				for (; j + 3 < outw; j += 4)
				{
					asm volatile(
						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128]! \n"// r00 r01

						"vmov       q10, %q17       \n"// sum00

						"vmla.f32   q10, %q8, q14   \n"

						"vmov       q11, %q17       \n"// sum01

						"vmla.f32   q10, %q9, q15   \n"

						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128]! \n"// r02 r03

						"vmla.f32   q11, %q8, q14   \n"
						"vmla.f32   q10, %q10, q14  \n"

						"vmov       q12, %q17       \n"// sum02

						"vmla.f32   q11, %q9, q15   \n"

						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128]! \n"// r04 r05

						"vmla.f32   q12, %q8, q14   \n"
						"vmla.f32   q11, %q10, q14  \n"

						"vmla.f32   q12, %q9, q15   \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128]! \n"// r10 r11

						"vmla.f32   q10, %q11, q14  \n"

						"vmov       q13, %q17       \n"// sum03

						"vmla.f32   q10, %q12, q15  \n"

						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128]! \n"// r06 r07

						"vmla.f32   q13, %q8, q14   \n"
						"vmla.f32   q12, %q10, q14  \n"

						"vmla.f32   q13, %q9, q15   \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128]! \n"// r12 r13

						"vmla.f32   q11, %q11, q14  \n"
						"vmla.f32   q10, %q13, q14  \n"

						"vmla.f32   q11, %q12, q15  \n"

						"vld1.f32   {d28-d29}, [%1 :128] \n"// r08

						"vmla.f32   q13, %q10, q14  \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128]! \n"// r14 r15

						"vmla.f32   q12, %q11, q14  \n"
						"vmla.f32   q11, %q13, q14  \n"

						"vmla.f32   q12, %q12, q15  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128]! \n"// r20 r21

						"vmla.f32   q10, %q14, q14  \n"
						"vmla.f32   q10, %q15, q15  \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128]! \n"// r16 r17

						"vmla.f32   q13, %q11, q14  \n"
						"vmla.f32   q12, %q13, q14  \n"

						"vmla.f32   q13, %q12, q15  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128]! \n"// r22 r23

						"vmla.f32   q11, %q14, q14  \n"
						"vmla.f32   q10, %q16, q14  \n"

						"vmla.f32   q11, %q15, q15  \n"

						"vld1.f32   {d28-d29}, [%2 :128] \n"// r18

						"vmla.f32   q13, %q13, q14  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128]! \n"// r24 r25

						"vmla.f32   q12, %q14, q14  \n"
						"vmla.f32   q11, %q16, q14  \n"

						"vmla.f32   q12, %q15, q15  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128]! \n"// r26 r27

						"vmla.f32   q13, %q14, q14  \n"
						"vmla.f32   q12, %q16, q14  \n"

						"vmla.f32   q13, %q15, q15  \n"

						"vld1.f32   {d28-d29}, [%3 :128] \n"// r28

						"vmla.f32   q13, %q16, q14  \n"

						"vstm       %0!, {d20-d27}  \n"

						: "=r"(outptr0),    // %0
						"=r"(r0),         // %1
						"=r"(r1),         // %2
						"=r"(r2)          // %3
						: "0"(outptr0),
						"1"(r0),
						"2"(r1),
						"3"(r2),
						"w"(_k00),        // %8
						"w"(_k01),        // %9
						"w"(_k02),        // %10
						"w"(_k10),        // %11
						"w"(_k11),        // %12
						"w"(_k12),        // %13
						"w"(_k20),        // %14
						"w"(_k21),        // %15
						"w"(_k22),        // %16
						"w"(_bias0)       // %17
						: "memory", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
				for (; j + 1 < outw; j += 2)
				{
					asm volatile(
						"pld        [%1, #256]      \n"
						"vld1.f32   {d24-d27}, [%1 :128]! \n"// r00 r01

						"vmov       q10, %q17       \n"// sum00
						"vmov       q11, %q17       \n"// sum01

						"vmla.f32   q10, %q8, q12   \n"

						"pld        [%1, #256]      \n"
						"vld1.f32   {d28-d31}, [%1 :128]! \n"// r02 r03

						"vmla.f32   q10, %q9, q13   \n"

						"vmla.f32   q11, %q8, q14   \n"
						"vmla.f32   q10, %q10, q14  \n"

						"vld1.f32   {d24-d25}, [%1 :128] \n"// r04

						"vmla.f32   q11, %q9, q15   \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d28-d31}, [%2 :128]! \n"// r10 r11

						"vmla.f32   q11, %q10, q12  \n"

						"vmla.f32   q10, %q11, q14  \n"

						"pld        [%2, #256]      \n"
						"vld1.f32   {d24-d27}, [%2 :128]! \n"// r12 r13

						"vmla.f32   q10, %q12, q15  \n"

						"vmla.f32   q11, %q11, q12  \n"
						"vmla.f32   q10, %q13, q12  \n"

						"vld1.f32   {d28-d29}, [%2 :128] \n"// r14

						"vmla.f32   q11, %q12, q13  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d24-d27}, [%3 :128]! \n"// r20 r21

						"vmla.f32   q11, %q13, q14  \n"

						"vmla.f32   q10, %q14, q12  \n"

						"pld        [%3, #256]      \n"
						"vld1.f32   {d28-d31}, [%3 :128]! \n"// r22 r23

						"vmla.f32   q10, %q15, q13  \n"

						"vmla.f32   q11, %q14, q14  \n"
						"vmla.f32   q10, %q16, q14  \n"

						"vld1.f32   {d24-d25}, [%3 :128] \n"// r24

						"vmla.f32   q11, %q15, q15  \n"

						"vmla.f32   q11, %q16, q12  \n"

						"vst1.f32   {d20-d23}, [%0 :128]! \n"

						: "=r"(outptr0),    // %0
						"=r"(r0),         // %1
						"=r"(r1),         // %2
						"=r"(r2)          // %3
						: "0"(outptr0),
						"1"(r0),
						"2"(r1),
						"3"(r2),
						"w"(_k00),        // %8
						"w"(_k01),        // %9
						"w"(_k02),        // %10
						"w"(_k10),        // %11
						"w"(_k11),        // %12
						"w"(_k12),        // %13
						"w"(_k20),        // %14
						"w"(_k21),        // %15
						"w"(_k22),        // %16
						"w"(_bias0)       // %17
						: "memory", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#endif
				for (; j < outw; j++)
				{
#if __ARM_NEON
					float32x4_t _sum0 = _bias0;

					float32x4_t _r00 = vld1q_f32(r0);
					float32x4_t _r01 = vld1q_f32(r0 + 4);
					float32x4_t _r02 = vld1q_f32(r0 + 8);
					float32x4_t _r10 = vld1q_f32(r1);
					float32x4_t _r11 = vld1q_f32(r1 + 4);
					float32x4_t _r12 = vld1q_f32(r1 + 8);
					float32x4_t _r20 = vld1q_f32(r2);
					float32x4_t _r21 = vld1q_f32(r2 + 4);
					float32x4_t _r22 = vld1q_f32(r2 + 8);

					_sum0 = vmlaq_f32(_sum0, _k00, _r00);
					_sum0 = vmlaq_f32(_sum0, _k01, _r01);
					_sum0 = vmlaq_f32(_sum0, _k02, _r02);
					_sum0 = vmlaq_f32(_sum0, _k10, _r10);
					_sum0 = vmlaq_f32(_sum0, _k11, _r11);
					_sum0 = vmlaq_f32(_sum0, _k12, _r12);
					_sum0 = vmlaq_f32(_sum0, _k20, _r20);
					_sum0 = vmlaq_f32(_sum0, _k21, _r21);
					_sum0 = vmlaq_f32(_sum0, _k22, _r22);

					vst1q_f32(outptr0, _sum0);
#else
					float sum[4];
					sum[0] = (bias) ? bias[g * 4] : 0.0f;
					sum[1] = (bias) ? bias[g * 4 + 1] : 0.0f;
					sum[2] = (bias) ? bias[g * 4 + 2] : 0.0f;
					sum[3] = (bias) ? bias[g * 4 + 3] : 0.0f;

					sum[0] += k0[0] * r0[0];
					sum[1] += k0[1] * r0[1];
					sum[2] += k0[2] * r0[2];
					sum[3] += k0[3] * r0[3];

					sum[0] += k0[4] * r0[4];
					sum[1] += k0[5] * r0[5];
					sum[2] += k0[6] * r0[6];
					sum[3] += k0[7] * r0[7];

					sum[0] += k0[8] * r0[8];
					sum[1] += k0[9] * r0[9];
					sum[2] += k0[10] * r0[10];
					sum[3] += k0[11] * r0[11];

					sum[0] += k0[12] * r1[0];
					sum[1] += k0[13] * r1[1];
					sum[2] += k0[14] * r1[2];
					sum[3] += k0[15] * r1[3];

					sum[0] += k0[16] * r1[4];
					sum[1] += k0[17] * r1[5];
					sum[2] += k0[18] * r1[6];
					sum[3] += k0[19] * r1[7];

					sum[0] += k0[20] * r1[8];
					sum[1] += k0[21] * r1[9];
					sum[2] += k0[22] * r1[10];
					sum[3] += k0[23] * r1[11];

					sum[0] += k0[24] * r2[0];
					sum[1] += k0[25] * r2[1];
					sum[2] += k0[26] * r2[2];
					sum[3] += k0[27] * r2[3];

					sum[0] += k0[28] * r2[4];
					sum[1] += k0[29] * r2[5];
					sum[2] += k0[30] * r2[6];
					sum[3] += k0[31] * r2[7];

					sum[0] += k0[32] * r2[8];
					sum[1] += k0[33] * r2[9];
					sum[2] += k0[34] * r2[10];
					sum[3] += k0[35] * r2[11];

					outptr0[0] = sum[0];
					outptr0[1] = sum[1];
					outptr0[2] = sum[2];
					outptr0[3] = sum[3];

#endif
					r0 += 2 * 4;
					r1 += 2 * 4;
					r2 += 2 * 4;
					outptr0 += 4;
				}

				r0 += tailstep;
				r1 += tailstep;
				r2 += tailstep;
			}
		}
	}

	// void conv5x5s1_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& _kernel, const Mat& _bias, const Option& opt)
	void conv5x5s1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 1;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* out = out_mem + p * out_cstep; // Mat out = top_blob.channel(p);

			const float bias0 = bias ? bias[p] : 0.f;

			// out.fill(bias0);
			{
				float* ptr = out;
				for (int t = 0; t < out_size; t++)
				{
					*ptr = bias0;
					ptr++;
				}
			}


			for (int q = 0; q < inch; q++)
			{
				float* outptr = out;
				float* outptr2 = outptr + outw;

				const float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* kernel0 = kernel + p*inch * 25 + q * 25;

				const float* r0 = img0;
				const float* r1 = img0 + w;
				const float* r2 = img0 + w * 2;
				const float* r3 = img0 + w * 3;
				const float* r4 = img0 + w * 4;
				const float* r5 = img0 + w * 5;

				const float* k0 = kernel0;
				const float* k1 = kernel0 + 5;
				const float* k2 = kernel0 + 10;
				const float* k3 = kernel0 + 15;
				const float* k4 = kernel0 + 20;

#if __ARM_NEON
				float32x4_t _k0123 = vld1q_f32(kernel0);
				float32x4_t _k4567 = vld1q_f32(kernel0 + 4);
				float32x4_t _k891011 = vld1q_f32(kernel0 + 8);
				float32x4_t _k12131415 = vld1q_f32(kernel0 + 12);
				float32x4_t _k16171819 = vld1q_f32(kernel0 + 16);
				float32x4_t _k20212223 = vld1q_f32(kernel0 + 20);
				float32x4_t _k24242424 = vdupq_n_f32(kernel0[24]);
#endif // __ARM_NEON

				int i = 0;

				for (; i + 1 < outh; i += 2)
				{

#if __ARM_NEON
					int nn = outw >> 2;
					int remain = outw - (nn << 2);
#else
					int remain = outw;
#endif // __ARM_NEON

#if __ARM_NEON
					if (nn > 0)
					{
						asm volatile(
							//                     "veor       q13, q13            \n"
							//                     "veor       q14, q14            \n"

							"pld        [%1, #128]          \n"

							"vld1.f32   {d14-d15}, [%1]     \n"// q7 = out

							"0:                             \n"

							// q11 = rx1 / rx3
							// q12 = rx2

							// q13 q14 = intermediate sum register

							"pld        [%2, #128]          \n"

							"vld1.f32   {d16-d17}, [%2]     \n"// q8 = out2


							"pld        [%4, #256]          \n"

							// r1
							"vld1.f32   {d18-d21}, [%4]     \n"// q9 q10 = r10 r14
							"add        %4, #16             \n"

							"vext.32    q11, q9, q10, #1    \n"// r11
							"vmul.f32   q13, q9, %e19[1]    \n"
							"vmla.f32   q8, q9, %e18[0]     \n"

							"vext.32    q12, q9, q10, #2    \n"// r12
							"vmla.f32   q7, q11, %f19[0]    \n"
							"vmul.f32   q14, q11, %e18[1]   \n"

							"vext.32    q11, q9, q10, #3    \n"// r13
							"vmla.f32   q13, q12, %f19[1]   \n"
							"vmla.f32   q8, q12, %f18[0]    \n"

							"vmla.f32   q7, q11, %e20[0]    \n"
							"vmla.f32   q14, q11, %f18[1]   \n"

							"pld        [%5, #256]          \n"

							"vmla.f32   q13, q10, %e20[1]   \n"
							"vmla.f32   q8, q10, %e19[0]    \n"

							// r2
							"vld1.f32   {d18-d21}, [%5]     \n"// q9 q10 = r20 r24
							"add        %5, #16             \n"

							"vext.32    q11, q9, q10, #1    \n"// r21
							"vmla.f32   q7, q9, %f20[0]     \n"
							"vmla.f32   q14, q9, %e19[1]    \n"

							"vext.32    q12, q9, q10, #2    \n"// r22
							"vmla.f32   q13, q11, %f20[1]   \n"
							"vmla.f32   q8, q11, %f19[0]    \n"

							"vext.32    q11, q9, q10, #3    \n"// r23
							"vmla.f32   q7, q12, %e21[0]    \n"
							"vmla.f32   q14, q12, %f19[1]   \n"

							"vmla.f32   q13, q11, %e21[1]   \n"
							"vmla.f32   q8, q11, %e20[0]    \n"

							"pld        [%6, #256]          \n"

							"vmla.f32   q7, q10, %f21[0]    \n"
							"vmla.f32   q14, q10, %e20[1]   \n"

							// r3
							"vld1.f32   {d18-d21}, [%6]     \n"// q9 q10 = r30 r34
							"add        %6, #16             \n"

							"vext.32    q11, q9, q10, #1    \n"// r31
							"vmla.f32   q13, q9, %f21[1]    \n"
							"vmla.f32   q8, q9, %f20[0]     \n"

							"vext.32    q12, q9, q10, #2    \n"// r32
							"vmla.f32   q7, q11, %e22[0]    \n"
							"vmla.f32   q14, q11, %f20[1]   \n"

							"vext.32    q11, q9, q10, #3    \n"// r33
							"vmla.f32   q13, q12, %e22[1]   \n"
							"vmla.f32   q8, q12, %e21[0]    \n"

							"vmla.f32   q7, q11, %f22[0]    \n"
							"vmla.f32   q14, q11, %e21[1]   \n"

							"pld        [%7, #256]          \n"

							"vmla.f32   q13, q10, %f22[1]   \n"
							"vmla.f32   q8, q10, %f21[0]    \n"

							// r4
							"vld1.f32   {d18-d21}, [%7]     \n"// q9 q10 = r40 r44
							"add        %7, #16             \n"

							"vext.32    q11, q9, q10, #1    \n"// r41
							"vmla.f32   q7, q9, %e23[0]     \n"
							"vmla.f32   q14, q9, %f21[1]    \n"

							"vext.32    q12, q9, q10, #2    \n"// r42
							"vmla.f32   q13, q11, %e23[1]   \n"
							"vmla.f32   q8, q11, %e22[0]    \n"

							"vext.32    q11, q9, q10, #3    \n"// r43
							"vmla.f32   q7, q12, %f23[0]    \n"
							"vmla.f32   q14, q12, %e22[1]   \n"

							"vmla.f32   q13, q11, %f23[1]   \n"
							"vmla.f32   q8, q11, %f22[0]    \n"

							"pld        [%3, #256]          \n"

							"vmla.f32   q7, q10, %e24[0]    \n"
							"vmla.f32   q14, q10, %f22[1]   \n"

							// r0 and r5
							"vld1.f32   {d18-d21}, [%3]     \n"// q9 q10 = r00 r04
							"add        %3, #16             \n"

							"vext.32    q11, q9, q10, #1    \n"// r01
							"vmla.f32   q13, q11, %e18[1]   \n"

							"vext.32    q12, q9, q10, #2    \n"// r02
							"vmla.f32   q7, q12, %f18[0]    \n"

							"vext.32    q11, q9, q10, #3    \n"// r03

							"pld        [%8, #256]          \n"

							"vmla.f32   q13, q11, %f18[1]   \n"

							// r5
							"vld1.f32   {d22-d25}, [%8]     \n"// q11 q12 = r50 r54
							"add        %8, #16             \n"

							"vmla.f32   q8, q11, %e23[0]    \n"
							"vmla.f32   q14, q12, %e24[0]   \n"

							"vmla.f32   q7, q9, %e18[0]     \n"
							"vmla.f32   q13, q10, %e19[0]   \n"

							"vext.32    q9, q11, q12, #1    \n"// r51
							"vext.32    q10, q11, q12, #2   \n"// r52

							"vmla.f32   q14, q9, %e23[1]    \n"

							"vext.32    q9, q11, q12, #3    \n"// r53
							"vmla.f32   q8, q10, %f23[0]    \n"

							"vmla.f32   q14, q9, %f23[1]    \n"

							"vadd.f32   q7, q7, q13         \n"

							//                     "veor       q13, q13            \n"

							"vst1.f32   {d14-d15}, [%1]!    \n"

							"vadd.f32   q8, q8, q14         \n"

							"pld        [%1, #128]          \n"

							"vld1.f32   {d14-d15}, [%1]     \n"// q7 = out

															   //                     "veor       q14, q14            \n"

							"vst1.f32   {d16-d17}, [%2]!    \n"

							"subs       %0, #1              \n"
							"bne        0b                  \n"
							: "=r"(nn),         // %0
							"=r"(outptr),     // %1
							"=r"(outptr2),    // %2
							"=r"(r0),         // %3
							"=r"(r1),         // %4
							"=r"(r2),         // %5
							"=r"(r3),         // %6
							"=r"(r4),         // %7
							"=r"(r5)          // %8
							: "0"(nn),
							"1"(outptr),
							"2"(outptr2),
							"3"(r0),
							"4"(r1),
							"5"(r2),
							"6"(r3),
							"7"(r4),
							"8"(r5),
							"w"(_k0123),      // %18
							"w"(_k4567),      // %19
							"w"(_k891011),    // %20
							"w"(_k12131415),  // %21
							"w"(_k16171819),  // %22
							"w"(_k20212223),  // %23
							"w"(_k24242424)   // %24
							: "cc", "memory", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
						float sum = 0;
						float sum2 = 0;
#if __ARM_NEON
						float32x4_t _r1 = vld1q_f32(r1);
						float32x4_t _k1 = vld1q_f32(k1);
						float32x4_t _sum = vmulq_f32(_r1, _k1);
						float32x4_t _sum2 = vmulq_f32(_r1, _k0123);

						float32x4_t _r2 = vld1q_f32(r2);
						float32x4_t _k2 = vld1q_f32(k2);
						_sum = vmlaq_f32(_sum, _r2, _k2);
						_sum2 = vmlaq_f32(_sum2, _r2, _k1);

						float32x4_t _r3 = vld1q_f32(r3);
						float32x4_t _k3 = vld1q_f32(k3);
						_sum = vmlaq_f32(_sum, _r3, _k3);
						_sum2 = vmlaq_f32(_sum2, _r3, _k2);

						float32x4_t _r4 = vld1q_f32(r4);
						_sum = vmlaq_f32(_sum, _r4, _k20212223);
						_sum2 = vmlaq_f32(_sum2, _r4, _k3);

						float32x4_t _r0 = vld1q_f32(r0);
						_sum = vmlaq_f32(_sum, _r0, _k0123);
						float32x4_t _r5 = vld1q_f32(r5);
						_sum2 = vmlaq_f32(_sum2, _r5, _k20212223);

						float32x4_t _k_t4;
						_k_t4 = vsetq_lane_f32(k0[4], _k_t4, 0);
						_k_t4 = vsetq_lane_f32(k1[4], _k_t4, 1);
						_k_t4 = vsetq_lane_f32(k2[4], _k_t4, 2);
						_k_t4 = vsetq_lane_f32(k3[4], _k_t4, 3);

						float32x4_t _r_t4;

						_r_t4 = vsetq_lane_f32(r0[4], _r_t4, 0);
						_r_t4 = vsetq_lane_f32(r1[4], _r_t4, 1);
						_r_t4 = vsetq_lane_f32(r2[4], _r_t4, 2);
						_r_t4 = vsetq_lane_f32(r3[4], _r_t4, 3);
						_sum = vmlaq_f32(_sum, _r_t4, _k_t4);

						sum = r4[4] * k4[4];

						_r_t4 = vextq_f32(_r_t4, _r_t4, 1);
						_r_t4 = vsetq_lane_f32(r4[4], _r_t4, 3);
						_sum2 = vmlaq_f32(_sum2, _r_t4, _k_t4);

						sum2 = r5[4] * k4[4];

						float32x2_t _ss = vadd_f32(vget_low_f32(_sum), vget_high_f32(_sum));
						float32x2_t _ss2 = vadd_f32(vget_low_f32(_sum2), vget_high_f32(_sum2));
						float32x2_t _ss_ss2 = vpadd_f32(_ss, _ss2);

						sum += vget_lane_f32(_ss_ss2, 0);
						sum2 += vget_lane_f32(_ss_ss2, 1);
#else
						sum += r0[0] * k0[0];
						sum += r0[1] * k0[1];
						sum += r0[2] * k0[2];
						sum += r0[3] * k0[3];
						sum += r0[4] * k0[4];

						sum += r1[0] * k1[0];
						sum += r1[1] * k1[1];
						sum += r1[2] * k1[2];
						sum += r1[3] * k1[3];
						sum += r1[4] * k1[4];

						sum += r2[0] * k2[0];
						sum += r2[1] * k2[1];
						sum += r2[2] * k2[2];
						sum += r2[3] * k2[3];
						sum += r2[4] * k2[4];

						sum += r3[0] * k3[0];
						sum += r3[1] * k3[1];
						sum += r3[2] * k3[2];
						sum += r3[3] * k3[3];
						sum += r3[4] * k3[4];

						sum += r4[0] * k4[0];
						sum += r4[1] * k4[1];
						sum += r4[2] * k4[2];
						sum += r4[3] * k4[3];
						sum += r4[4] * k4[4];

						sum2 += r1[0] * k0[0];
						sum2 += r1[1] * k0[1];
						sum2 += r1[2] * k0[2];
						sum2 += r1[3] * k0[3];
						sum2 += r1[4] * k0[4];

						sum2 += r2[0] * k1[0];
						sum2 += r2[1] * k1[1];
						sum2 += r2[2] * k1[2];
						sum2 += r2[3] * k1[3];
						sum2 += r2[4] * k1[4];

						sum2 += r3[0] * k2[0];
						sum2 += r3[1] * k2[1];
						sum2 += r3[2] * k2[2];
						sum2 += r3[3] * k2[3];
						sum2 += r3[4] * k2[4];

						sum2 += r4[0] * k3[0];
						sum2 += r4[1] * k3[1];
						sum2 += r4[2] * k3[2];
						sum2 += r4[3] * k3[3];
						sum2 += r4[4] * k3[4];

						sum2 += r5[0] * k4[0];
						sum2 += r5[1] * k4[1];
						sum2 += r5[2] * k4[2];
						sum2 += r5[3] * k4[3];
						sum2 += r5[4] * k4[4];
#endif // __ARM_NEON
						*outptr += sum;
						*outptr2 += sum2;

						r0++;
						r1++;
						r2++;
						r3++;
						r4++;
						r5++;
						outptr++;
						outptr2++;
					}

					r0 += 4 + w;
					r1 += 4 + w;
					r2 += 4 + w;
					r3 += 4 + w;
					r4 += 4 + w;
					r5 += 4 + w;

					outptr += outw;
					outptr2 += outw;
				}

				for (; i < outh; i++)
				{

#if __ARM_NEON
					int nn = outw >> 2;
					int remain = outw - (nn << 2);
#else
					int remain = outw;
#endif // __ARM_NEON

#if __ARM_NEON
					if (nn > 0)
					{
						asm volatile(
							//                     "veor       q15, q15            \n"// _sum3 = 0;

							"pld        [%1, #128]          \n"

							"pld        [%2, #256]          \n"

							"vld1.f32   {d16-d19}, [%2]     \n"// _r00 = vld1q_f32(r0+j);
							"add        %2, #16             \n"

							"0:                             \n"

							"vld1.f32   {d14-d15}, [%1]     \n"// _sum = vld1q_f32(outptr+j);
															   //                     "veor       q13, q13            \n"// _sum2 = 0;
															   //                     "veor       q14, q14            \n"// _sum3 = 0;

							"vext.32    q10, q8, q9, #1     \n"// _r01
							"vext.32    q11, q8, q9, #2     \n"// _r02
							"vext.32    q12, q8, q9, #3     \n"// _r03

							"vmla.f32   q7, q8, %e14[0]     \n"
							"vmul.f32   q13, q10, %e14[1]   \n"

							"pld        [%3, #256]          \n"

							"vmul.f32   q14, q11, %f14[0]   \n"
							"vmul.f32   q15, q12, %f14[1]   \n"
							"vmla.f32   q7, q9, %e15[0]     \n"

							"vld1.f32   {d16-d19}, [%3]     \n"
							"add        %3, #16             \n"
							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"
							"vext.32    q12, q8, q9, #3     \n"

							"vmla.f32   q7, q8, %e15[1]     \n"
							"vmla.f32   q13, q10, %f15[0]   \n"

							"pld        [%4, #256]          \n"

							"vmla.f32   q14, q11, %f15[1]   \n"
							"vmla.f32   q15, q12, %e16[0]   \n"
							"vmla.f32   q7, q9, %e16[1]     \n"

							"vld1.f32   {d16-d19}, [%4]     \n"
							"add        %4, #16             \n"
							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"
							"vext.32    q12, q8, q9, #3     \n"

							"vmla.f32   q7, q8, %f16[0]     \n"
							"vmla.f32   q13, q10, %f16[1]   \n"

							"pld        [%5, #256]          \n"

							"vmla.f32   q14, q11, %e17[0]   \n"
							"vmla.f32   q15, q12, %e17[1]   \n"
							"vmla.f32   q7, q9, %f17[0]     \n"

							"vld1.f32   {d16-d19}, [%5]     \n"
							"add        %5, #16             \n"
							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"
							"vext.32    q12, q8, q9, #3     \n"

							"vmla.f32   q7, q8, %f17[1]     \n"
							"vmla.f32   q13, q10, %e18[0]   \n"

							"pld        [%6, #256]          \n"

							"vmla.f32   q14, q11, %e18[1]   \n"
							"vmla.f32   q15, q12, %f18[0]   \n"
							"vmla.f32   q7, q9, %f18[1]     \n"

							"vld1.f32   {d16-d19}, [%6]     \n"
							"add        %6, #16             \n"
							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"
							"vext.32    q12, q8, q9, #3     \n"

							"vmla.f32   q7, q8, %e19[0]     \n"
							"vmla.f32   q13, q10, %e19[1]   \n"
							"vmla.f32   q14, q11, %f19[0]   \n"
							"vmla.f32   q15, q12, %f19[1]   \n"
							"vmla.f32   q7, q9, %e20[0]     \n"

							"vadd.f32   q14, q14, q15       \n"
							"vadd.f32   q7, q7, q13         \n"
							//                     "veor       q15, q15            \n"// _sum3 = 0;

							"pld        [%2, #256]          \n"

							"vadd.f32   q7, q7, q14         \n"

							"vld1.f32   {d16-d19}, [%2]     \n"// _r00 = vld1q_f32(r0+j);
							"add        %2, #16             \n"

							"vst1.f32   {d14-d15}, [%1]!    \n"

							"pld        [%1, #128]          \n"

							"subs       %0, #1              \n"
							"bne        0b                  \n"

							"sub        %2, #16             \n"
							: "=r"(nn),         // %0
							"=r"(outptr),     // %1
							"=r"(r0),         // %2
							"=r"(r1),         // %3
							"=r"(r2),         // %4
							"=r"(r3),         // %5
							"=r"(r4)          // %6
							: "0"(nn),
							"1"(outptr),
							"2"(r0),
							"3"(r1),
							"4"(r2),
							"5"(r3),
							"6"(r4),
							"w"(_k0123),      // %14
							"w"(_k4567),      // %15
							"w"(_k891011),    // %16
							"w"(_k12131415),  // %17
							"w"(_k16171819),  // %18
							"w"(_k20212223),  // %19
							"w"(_k24242424)   // %20
							: "cc", "memory", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
						float sum = 0;
#if __ARM_NEON
						float32x4_t _r0 = vld1q_f32(r0);
						float32x4_t _sum = vmulq_f32(_r0, _k0123);

						float32x4_t _r1 = vld1q_f32(r1);
						_sum = vmlaq_f32(_sum, _r1, vld1q_f32(k1));

						float32x4_t _r2 = vld1q_f32(r2);
						_sum = vmlaq_f32(_sum, _r2, vld1q_f32(k2));

						float32x4_t _r3 = vld1q_f32(r3);
						_sum = vmlaq_f32(_sum, _r3, vld1q_f32(k3));

						float32x4_t _r4 = vld1q_f32(r4);
						_sum = vmlaq_f32(_sum, _r4, _k20212223);

						float32x4_t _k_t4;
						_k_t4 = vsetq_lane_f32(k0[4], _k_t4, 0);
						_k_t4 = vsetq_lane_f32(k1[4], _k_t4, 1);
						_k_t4 = vsetq_lane_f32(k2[4], _k_t4, 2);
						_k_t4 = vsetq_lane_f32(k3[4], _k_t4, 3);

						float32x4_t _r_t4;

						_r_t4 = vsetq_lane_f32(r0[4], _r_t4, 0);
						_r_t4 = vsetq_lane_f32(r1[4], _r_t4, 1);
						_r_t4 = vsetq_lane_f32(r2[4], _r_t4, 2);
						_r_t4 = vsetq_lane_f32(r3[4], _r_t4, 3);
						_sum = vmlaq_f32(_sum, _r_t4, _k_t4);

						sum = r4[4] * k4[4];

						float32x2_t _ss = vadd_f32(vget_low_f32(_sum), vget_high_f32(_sum));
						_ss = vpadd_f32(_ss, _ss);

						sum += vget_lane_f32(_ss, 0);
#else
						sum += r0[0] * k0[0];
						sum += r0[1] * k0[1];
						sum += r0[2] * k0[2];
						sum += r0[3] * k0[3];
						sum += r0[4] * k0[4];

						sum += r1[0] * k1[0];
						sum += r1[1] * k1[1];
						sum += r1[2] * k1[2];
						sum += r1[3] * k1[3];
						sum += r1[4] * k1[4];

						sum += r2[0] * k2[0];
						sum += r2[1] * k2[1];
						sum += r2[2] * k2[2];
						sum += r2[3] * k2[3];
						sum += r2[4] * k2[4];

						sum += r3[0] * k3[0];
						sum += r3[1] * k3[1];
						sum += r3[2] * k3[2];
						sum += r3[3] * k3[3];
						sum += r3[4] * k3[4];

						sum += r4[0] * k4[0];
						sum += r4[1] * k4[1];
						sum += r4[2] * k4[2];
						sum += r4[3] * k4[3];
						sum += r4[4] * k4[4];
#endif
						*outptr += sum;

						r0++;
						r1++;
						r2++;
						r3++;
						r4++;
						outptr++;
					}

					r0 += 4;
					r1 += 4;
					r2 += 4;
					r3 += 4;
					r4 += 4;

				}

			}
		}
	}

	// void conv1x1s1_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& _kernel, const Mat& _bias, const Option& opt)
	void conv1x1s1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 1;

		int nn_outch = 0;
		int remain_outch_start = 0;

		nn_outch = outch / 6;
		remain_outch_start = nn_outch * 6;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = pp * 6;

			float* out0 = out_mem + (p + 0) * out_cstep; // Mat out0 = top_blob.channel(p);
			float* out1 = out_mem + (p + 1) * out_cstep; // Mat out1 = top_blob.channel(p + 1);
			float* out2 = out_mem + (p + 2) * out_cstep; // Mat out2 = top_blob.channel(p + 2);
			float* out3 = out_mem + (p + 3) * out_cstep; // Mat out3 = top_blob.channel(p + 3);
			float* out4 = out_mem + (p + 4) * out_cstep; // Mat out4 = top_blob.channel(p + 4);
			float* out5 = out_mem + (p + 5) * out_cstep; // Mat out5 = top_blob.channel(p + 5);

			const float bias0 = bias ? bias[p] : 0.f;
			const float bias1 = bias ? bias[p + 1] : 0.f;
			const float bias2 = bias ? bias[p + 2] : 0.f;
			const float bias3 = bias ? bias[p + 3] : 0.f;
			const float bias4 = bias ? bias[p + 4] : 0.f;
			const float bias5 = bias ? bias[p + 5] : 0.f;

			{ float* ptr = out0; for (int t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out0.fill(bias0);
			{ float* ptr = out1; for (int t = 0; t < out_size; t++) { *ptr = bias1; ptr++; } } // out1.fill(bias1);
			{ float* ptr = out2; for (int t = 0; t < out_size; t++) { *ptr = bias2; ptr++; } } // out2.fill(bias2);
			{ float* ptr = out3; for (int t = 0; t < out_size; t++) { *ptr = bias3; ptr++; } } // out3.fill(bias3);
			{ float* ptr = out4; for (int t = 0; t < out_size; t++) { *ptr = bias4; ptr++; } } // out4.fill(bias4);
			{ float* ptr = out5; for (int t = 0; t < out_size; t++) { *ptr = bias5; ptr++; } } // out5.fill(bias5);

			int q = 0;

			for (; q + 3 < inch; q += 4)
			{
				float* outptr0 = out0;
				float* outptr1 = out1;
				float* outptr2 = out2;
				float* outptr3 = out3;
				float* outptr4 = out4;
				float* outptr5 = out5;

				const float* img0 = in_mem + (q + 0) * cstep; // bottom_blob.channel(q);
				const float* img1 = in_mem + (q + 1) * cstep; // bottom_blob.channel(q + 1);
				const float* img2 = in_mem + (q + 2) * cstep; // bottom_blob.channel(q + 2);
				const float* img3 = in_mem + (q + 3) * cstep; // bottom_blob.channel(q + 3);

				const float* kernel0 = kernel + p*inch + q;
				const float* kernel1 = kernel + (p + 1)*inch + q;
				const float* kernel2 = kernel + (p + 2)*inch + q;
				const float* kernel3 = kernel + (p + 3)*inch + q;
				const float* kernel4 = kernel + (p + 4)*inch + q;
				const float* kernel5 = kernel + (p + 5)*inch + q;

				const float* r0 = img0;
				const float* r1 = img1;
				const float* r2 = img2;
				const float* r3 = img3;

				int size = outw * outh;

#if __ARM_NEON
				int nn = size >> 2;
				int remain = size & 3;
#else
				int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
				float32x4_t _k0 = vld1q_f32(kernel0);
				float32x4_t _k1 = vld1q_f32(kernel1);
				float32x4_t _k2 = vld1q_f32(kernel2);
				float32x4_t _k3 = vld1q_f32(kernel3);
				float32x4_t _k4 = vld1q_f32(kernel4);
				float32x4_t _k5 = vld1q_f32(kernel5);

				if (nn > 0)
				{
					asm volatile(
						"pld        [%7, #128]              \n"
						"vld1.f32   {d24-d25}, [%7 :128]!   \n"// q12 = r0

						"pld        [%1, #128]              \n"
						"vld1.f32   {d12-d13}, [%1 :128]    \n"// q6 = outptr0

						"pld        [%2, #128]              \n"
						"vld1.f32   {d14-d15}, [%2 :128]    \n"// q7 = outptr1

						"vmla.f32   q6, q12, %e22[0]        \n"

						"0:                                 \n"

						"pld        [%3, #128]              \n"
						"vld1.f32   {d16-d17}, [%3 :128]    \n"// q8 = outptr2

						"vmla.f32   q7, q12, %e23[0]        \n"

						"pld        [%4, #128]              \n"
						"vld1.f32   {d18-d19}, [%4 :128]    \n"// q9 = outptr3

						"vmla.f32   q8, q12, %e24[0]        \n"

						"pld        [%8, #128]              \n"
						"vld1.f32   {d26-d27}, [%8 :128]!   \n"// q13 = r1

						"vmla.f32   q9, q12, %e25[0]        \n"

						"pld        [%5, #128]              \n"
						"vld1.f32   {d20-d21}, [%5 :128]    \n"// q10 = outptr4

						"vmla.f32   q6, q13, %e22[1]        \n"
						"vmla.f32   q7, q13, %e23[1]        \n"

						"pld        [%6, #128]              \n"
						"vld1.f32   {d22-d23}, [%6 :128]    \n"// q11 = outptr5

						"vmla.f32   q10, q12, %e26[0]       \n"
						"vmla.f32   q11, q12, %e27[0]       \n"

						"vmla.f32   q8, q13, %e24[1]        \n"
						"vmla.f32   q9, q13, %e25[1]        \n"

						"pld        [%9, #128]              \n"
						"vld1.f32   {d28-d29}, [%9 :128]!   \n"// q14 = r2

						"vmla.f32   q10, q13, %e26[1]       \n"
						"vmla.f32   q11, q13, %e27[1]       \n"

						"vmla.f32   q6, q14, %f22[0]        \n"
						"vmla.f32   q7, q14, %f23[0]        \n"
						"vmla.f32   q8, q14, %f24[0]        \n"
						"vmla.f32   q9, q14, %f25[0]        \n"

						"pld        [%10, #128]             \n"
						"vld1.f32   {d30-d31}, [%10 :128]!  \n"// q15 = r3

						"vmla.f32   q10, q14, %f26[0]       \n"
						"vmla.f32   q11, q14, %f27[0]       \n"

						"vmla.f32   q6, q15, %f22[1]        \n"
						"vmla.f32   q7, q15, %f23[1]        \n"
						"vmla.f32   q8, q15, %f24[1]        \n"
						"vmla.f32   q9, q15, %f25[1]        \n"

						"pld        [%7, #128]              \n"
						"vld1.f32   {d24-d25}, [%7 :128]!   \n"// q12 = r0

						"vmla.f32   q10, q15, %f26[1]       \n"
						"vmla.f32   q11, q15, %f27[1]       \n"

						"vst1.f32   {d12-d13}, [%1 :128]!   \n"
						"vst1.f32   {d14-d15}, [%2 :128]!   \n"

						"pld        [%1, #128]              \n"
						"vld1.f32   {d12-d13}, [%1 :128]    \n"// q6 = outptr0

						"vst1.f32   {d16-d17}, [%3 :128]!   \n"
						"vst1.f32   {d18-d19}, [%4 :128]!   \n"

						"vmla.f32   q6, q12, %e22[0]        \n"

						"pld        [%2, #128]              \n"
						"vld1.f32   {d14-d15}, [%2 :128]    \n"// q7 = outptr1

						"subs       %0, #1                  \n"

						"vst1.f32   {d20-d21}, [%5 :128]!   \n"
						"vst1.f32   {d22-d23}, [%6 :128]!   \n"

						"bne        0b                      \n"

						"sub        %7, #16                 \n"

						: "=r"(nn),     // %0
						"=r"(outptr0),// %1
						"=r"(outptr1),// %2
						"=r"(outptr2),// %3
						"=r"(outptr3),// %4
						"=r"(outptr4),// %5
						"=r"(outptr5),// %6
						"=r"(r0),     // %7
						"=r"(r1),     // %8
						"=r"(r2),     // %9
						"=r"(r3)      // %10
						: "0"(nn),
						"1"(outptr0),
						"2"(outptr1),
						"3"(outptr2),
						"4"(outptr3),
						"5"(outptr4),
						"6"(outptr5),
						"7"(r0),
						"8"(r1),
						"9"(r2),
						"10"(r3),
						"w"(_k0),     // %22
						"w"(_k1),     // %23
						"w"(_k2),     // %24
						"w"(_k3),     // %25
						"w"(_k4),     // %26
						"w"(_k5)      // %27
						: "cc", "memory", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#endif // __ARM_NEON

				for (; remain > 0; remain--)
				{
					// TODO neon optimize
					float sum0 = *r0 * kernel0[0] + *r1 * kernel0[1] + *r2 * kernel0[2] + *r3 * kernel0[3];
					float sum1 = *r0 * kernel1[0] + *r1 * kernel1[1] + *r2 * kernel1[2] + *r3 * kernel1[3];
					float sum2 = *r0 * kernel2[0] + *r1 * kernel2[1] + *r2 * kernel2[2] + *r3 * kernel2[3];
					float sum3 = *r0 * kernel3[0] + *r1 * kernel3[1] + *r2 * kernel3[2] + *r3 * kernel3[3];
					float sum4 = *r0 * kernel4[0] + *r1 * kernel4[1] + *r2 * kernel4[2] + *r3 * kernel4[3];
					float sum5 = *r0 * kernel5[0] + *r1 * kernel5[1] + *r2 * kernel5[2] + *r3 * kernel5[3];

					*outptr0 += sum0;
					*outptr1 += sum1;
					*outptr2 += sum2;
					*outptr3 += sum3;
					*outptr4 += sum4;
					*outptr5 += sum5;

					r0++;
					r1++;
					r2++;
					r3++;
					outptr0++;
					outptr1++;
					outptr2++;
					outptr3++;
					outptr4++;
					outptr5++;
				}
			}

			for (; q < inch; q++)
			{
				float* outptr0 = out0;
				float* outptr1 = out1;
				float* outptr2 = out2;
				float* outptr3 = out3;
				float* outptr4 = out4;
				float* outptr5 = out5;

				const float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* kernel0 = kernel + p*inch + q;
				const float* kernel1 = kernel + (p + 1)*inch + q;
				const float* kernel2 = kernel + (p + 2)*inch + q;
				const float* kernel3 = kernel + (p + 3)*inch + q;
				const float* kernel4 = kernel + (p + 4)*inch + q;
				const float* kernel5 = kernel + (p + 5)*inch + q;

				const float k0 = kernel0[0];
				const float k1 = kernel1[0];
				const float k2 = kernel2[0];
				const float k3 = kernel3[0];
				const float k4 = kernel4[0];
				const float k5 = kernel5[0];

				const float* r0 = img0;

				int size = outw * outh;

#if __ARM_NEON
				int nn = size >> 2;
				int remain = size & 3;
#else
				int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
				float32x4_t _k0 = vdupq_n_f32(k0);
				float32x4_t _k1 = vdupq_n_f32(k1);
				float32x4_t _k2 = vdupq_n_f32(k2);
				float32x4_t _k3 = vdupq_n_f32(k3);
				float32x4_t _k4 = vdupq_n_f32(k4);
				float32x4_t _k5 = vdupq_n_f32(k5);

				if (nn > 0)
				{
					asm volatile(
						"pld        [%7, #128]              \n"
						"vld1.f32   {d24-d25}, [%7 :128]!   \n"// q12 = r0

						"pld        [%1, #128]              \n"
						"vld1.f32   {d12-d13}, [%1 :128]    \n"// q6 = outptr0

						"0:                                 \n"

						"pld        [%2, #128]              \n"
						"vld1.f32   {d14-d15}, [%2 :128]    \n"// q7 = outptr1

						"vmla.f32   q6, q12, %q16           \n"

						"pld        [%3, #128]              \n"
						"vld1.f32   {d16-d17}, [%3 :128]    \n"// q8 = outptr2

						"vmla.f32   q7, q12, %q17           \n"

						"pld        [%4, #128]              \n"
						"vld1.f32   {d18-d19}, [%4 :128]    \n"// q9 = outptr3

						"vmla.f32   q8, q12, %q18           \n"

						"pld        [%5, #128]              \n"
						"vld1.f32   {d20-d21}, [%5 :128]    \n"// q10 = outptr4

						"vmla.f32   q9, q12, %q19           \n"

						"pld        [%6, #128]              \n"
						"vld1.f32   {d22-d23}, [%6 :128]    \n"// q11 = outptr5

						"vmla.f32   q10, q12, %q20          \n"
						"vmla.f32   q11, q12, %q21          \n"

						"pld        [%7, #128]              \n"
						"vld1.f32   {d24-d25}, [%7 :128]!   \n"// q12 = r0

						"vst1.f32   {d12-d13}, [%1 :128]!   \n"
						"vst1.f32   {d14-d15}, [%2 :128]!   \n"

						"pld        [%1, #128]              \n"
						"vld1.f32   {d12-d13}, [%1 :128]    \n"// q6 = outptr0

						"vst1.f32   {d16-d17}, [%3 :128]!   \n"
						"vst1.f32   {d18-d19}, [%4 :128]!   \n"

						"subs       %0, #1                  \n"

						"vst1.f32   {d20-d21}, [%5 :128]!   \n"
						"vst1.f32   {d22-d23}, [%6 :128]!   \n"

						"bne        0b                      \n"

						"sub        %7, #16                 \n"

						: "=r"(nn),     // %0
						"=r"(outptr0),// %1
						"=r"(outptr1),// %2
						"=r"(outptr2),// %3
						"=r"(outptr3),// %4
						"=r"(outptr4),// %5
						"=r"(outptr5),// %6
						"=r"(r0)      // %7
						: "0"(nn),
						"1"(outptr0),
						"2"(outptr1),
						"3"(outptr2),
						"4"(outptr3),
						"5"(outptr4),
						"6"(outptr5),
						"7"(r0),
						"w"(_k0),     // %16
						"w"(_k1),     // %17
						"w"(_k2),     // %18
						"w"(_k3),     // %19
						"w"(_k4),     // %20
						"w"(_k5)      // %21
						: "cc", "memory", "q6", "q7", "q8", "q9", "q10", "q11", "q12"
						);
				}
#endif // __ARM_NEON

				for (; remain > 0; remain--)
				{
					// TODO neon optimize
					float sum0 = *r0 * k0;
					float sum1 = *r0 * k1;
					float sum2 = *r0 * k2;
					float sum3 = *r0 * k3;
					float sum4 = *r0 * k4;
					float sum5 = *r0 * k5;

					*outptr0 += sum0;
					*outptr1 += sum1;
					*outptr2 += sum2;
					*outptr3 += sum3;
					*outptr4 += sum4;
					*outptr5 += sum5;

					r0++;
					outptr0++;
					outptr1++;
					outptr2++;
					outptr3++;
					outptr4++;
					outptr5++;
				}
			}
		}

		nn_outch = (outch - remain_outch_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
        nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = remain_outch_start + pp * 4;

			float* out0 = out_mem + (p + 0) * out_cstep; // Mat out0 = top_blob.channel(p);
			float* out1 = out_mem + (p + 1) * out_cstep; // Mat out1 = top_blob.channel(p + 1);
			float* out2 = out_mem + (p + 2) * out_cstep; // Mat out2 = top_blob.channel(p + 2);
			float* out3 = out_mem + (p + 3) * out_cstep; // Mat out3 = top_blob.channel(p + 3);

			const float bias0 = bias ? bias[p] : 0.f;
			const float bias1 = bias ? bias[p + 1] : 0.f;
			const float bias2 = bias ? bias[p + 2] : 0.f;
			const float bias3 = bias ? bias[p + 3] : 0.f;

			{ float* ptr = out0; for (int t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out0.fill(bias0);
			{ float* ptr = out1; for (int t = 0; t < out_size; t++) { *ptr = bias1; ptr++; } } // out1.fill(bias1);
			{ float* ptr = out2; for (int t = 0; t < out_size; t++) { *ptr = bias2; ptr++; } } // out2.fill(bias2);
			{ float* ptr = out3; for (int t = 0; t < out_size; t++) { *ptr = bias3; ptr++; } } // out3.fill(bias3);

			int q = 0;

			for (; q + 3 < inch; q += 4)
			{
				float* outptr0 = out0;
				float* outptr1 = out1;
				float* outptr2 = out2;
				float* outptr3 = out3;

				const float* img0 = in_mem + (q + 0) * cstep; //bottom_blob.channel(q);
				const float* img1 = in_mem + (q + 1) * cstep; //bottom_blob.channel(q + 1);
				const float* img2 = in_mem + (q + 2) * cstep; //bottom_blob.channel(q + 2);
				const float* img3 = in_mem + (q + 3) * cstep; //bottom_blob.channel(q + 3);

				const float* kernel0 = kernel + p*inch + q;
				const float* kernel1 = kernel + (p + 1)*inch + q;
				const float* kernel2 = kernel + (p + 2)*inch + q;
				const float* kernel3 = kernel + (p + 3)*inch + q;

				const float* r0 = img0;
				const float* r1 = img1;
				const float* r2 = img2;
				const float* r3 = img3;

				int size = outw * outh;

#if __ARM_NEON
				int nn = size >> 3;
				int remain = size & 7;
#else
				int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
				float32x4_t _k0 = vld1q_f32(kernel0);
				float32x4_t _k1 = vld1q_f32(kernel1);
				float32x4_t _k2 = vld1q_f32(kernel2);
				float32x4_t _k3 = vld1q_f32(kernel3);

				if (nn > 0)
				{
					asm volatile(
						"pld        [%5, #256]              \n"
						"vld1.f32   {d12-d15}, [%5 :128]!   \n"
						"pld        [%1, #256]              \n"
						"vld1.f32   {d16-d19}, [%1 :128]    \n"
						"0:                                 \n"

						"vmla.f32   q8, q6, %e18[0]         \n"

						"pld        [%2, #256]              \n"
						"vld1.f32   {d20-d23}, [%2 :128]    \n"
						"vmla.f32   q9, q7, %e18[0]         \n"

						"vmla.f32   q10, q6, %e19[0]        \n"

						"pld        [%3, #256]              \n"
						"vld1.f32   {d24-d27}, [%3 :128]    \n"
						"vmla.f32   q11, q7, %e19[0]        \n"

						"vmla.f32   q12, q6, %e20[0]        \n"

						"pld        [%4, #256]              \n"
						"vld1.f32   {d28-d31}, [%4 :128]    \n"
						"vmla.f32   q13, q7, %e20[0]        \n"

						"pld        [%6, #256]              \n"
						"vld1.f32   {d8-d11}, [%6 :128]!    \n"

						"vmla.f32   q14, q6, %e21[0]        \n"
						"vmla.f32   q15, q7, %e21[0]        \n"

						"vmla.f32   q8, q4, %e18[1]         \n"
						"vmla.f32   q9, q5, %e18[1]         \n"

						"vmla.f32   q10, q4, %e19[1]        \n"
						"vmla.f32   q11, q5, %e19[1]        \n"

						"vmla.f32   q12, q4, %e20[1]        \n"
						"vmla.f32   q13, q5, %e20[1]        \n"

						"pld        [%7, #256]              \n"
						"vld1.f32   {d12-d15}, [%7 :128]!   \n"

						"vmla.f32   q14, q4, %e21[1]        \n"
						"vmla.f32   q15, q5, %e21[1]        \n"

						"vmla.f32   q8, q6, %f18[0]         \n"
						"vmla.f32   q9, q7, %f18[0]         \n"

						"vmla.f32   q10, q6, %f19[0]        \n"
						"vmla.f32   q11, q7, %f19[0]        \n"

						"vmla.f32   q12, q6, %f20[0]        \n"
						"vmla.f32   q13, q7, %f20[0]        \n"

						"pld        [%8, #256]              \n"
						"vld1.f32   {d8-d11}, [%8 :128]!    \n"

						"vmla.f32   q14, q6, %f21[0]        \n"
						"vmla.f32   q15, q7, %f21[0]        \n"

						"vmla.f32   q8, q4, %f18[1]         \n"
						"vmla.f32   q9, q5, %f18[1]         \n"

						"vmla.f32   q10, q4, %f19[1]        \n"
						"vmla.f32   q11, q5, %f19[1]        \n"

						"vmla.f32   q12, q4, %f20[1]        \n"
						"vst1.f32   {d16-d19}, [%1 :128]!   \n"

						"vmla.f32   q13, q5, %f20[1]        \n"

						"vst1.f32   {d20-d23}, [%2 :128]!   \n"

						"vmla.f32   q14, q4, %f21[1]        \n"
						"pld        [%5, #256]              \n"
						"vld1.f32   {d12-d15}, [%5 :128]!   \n"

						"vmla.f32   q15, q5, %f21[1]        \n"

						"vst1.f32   {d24-d27}, [%3 :128]!   \n"

						"pld        [%1, #256]              \n"
						"vld1.f32   {d16-d19}, [%1 :128]    \n"

						"subs       %0, #1                  \n"
						"vst1.f32   {d28-d31}, [%4 :128]!   \n"

						"bne        0b                      \n"
						"sub        %5, #32                 \n"
						: "=r"(nn),     // %0
						"=r"(outptr0),// %1
						"=r"(outptr1),// %2
						"=r"(outptr2),// %3
						"=r"(outptr3),// %4
						"=r"(r0),     // %5
						"=r"(r1),     // %6
						"=r"(r2),     // %7
						"=r"(r3)      // %8
						: "0"(nn),
						"1"(outptr0),
						"2"(outptr1),
						"3"(outptr2),
						"4"(outptr3),
						"5"(r0),
						"6"(r1),
						"7"(r2),
						"8"(r3),
						"w"(_k0),     // %18
						"w"(_k1),     // %19
						"w"(_k2),     // %20
						"w"(_k3)      // %21
						: "cc", "memory", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					// TODO neon optimize
					float sum0 = *r0 * kernel0[0] + *r1 * kernel0[1] + *r2 * kernel0[2] + *r3 * kernel0[3];
					float sum1 = *r0 * kernel1[0] + *r1 * kernel1[1] + *r2 * kernel1[2] + *r3 * kernel1[3];
					float sum2 = *r0 * kernel2[0] + *r1 * kernel2[1] + *r2 * kernel2[2] + *r3 * kernel2[3];
					float sum3 = *r0 * kernel3[0] + *r1 * kernel3[1] + *r2 * kernel3[2] + *r3 * kernel3[3];

					*outptr0 += sum0;
					*outptr1 += sum1;
					*outptr2 += sum2;
					*outptr3 += sum3;

					r0++;
					r1++;
					r2++;
					r3++;
					outptr0++;
					outptr1++;
					outptr2++;
					outptr3++;
				}
			}

			for (; q < inch; q++)
			{
				float* outptr0 = out0;
				float* outptr1 = out1;
				float* outptr2 = out2;
				float* outptr3 = out3;

				const float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* kernel0 = kernel + p*inch + q;
				const float* kernel1 = kernel + (p + 1)*inch + q;
				const float* kernel2 = kernel + (p + 2)*inch + q;
				const float* kernel3 = kernel + (p + 3)*inch + q;

				const float k0 = kernel0[0];
				const float k1 = kernel1[0];
				const float k2 = kernel2[0];
				const float k3 = kernel3[0];

				const float* r0 = img0;

				int size = outw * outh;

#if __ARM_NEON
				int nn = size >> 3;
				int remain = size & 7;
#else
				int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
				float32x4_t _k0 = vdupq_n_f32(k0);
				float32x4_t _k1 = vdupq_n_f32(k1);
				float32x4_t _k2 = vdupq_n_f32(k2);
				float32x4_t _k3 = vdupq_n_f32(k3);
				if (nn > 0)
				{
					asm volatile(
						"pld        [%5, #256]              \n"
						"vld1.f32   {d12-d15}, [%5 :128]!   \n"
						"0:                                 \n"
						"pld        [%1, #256]              \n"
						"vld1.f32   {d16-d19}, [%1 :128]    \n"
						"vmla.f32   q8, q6, %q12            \n"
						"vmla.f32   q9, q7, %q12            \n"

						"pld        [%2, #256]              \n"
						"vld1.f32   {d20-d23}, [%2 :128]    \n"
						"vmla.f32   q10, q6, %q13           \n"
						"vmla.f32   q11, q7, %q13           \n"

						"vst1.f32   {d16-d19}, [%1 :128]!   \n"

						"pld        [%3, #256]              \n"
						"vld1.f32   {d24-d27}, [%3 :128]    \n"
						"vmla.f32   q12, q6, %q14           \n"
						"vmla.f32   q13, q7, %q14           \n"

						"vst1.f32   {d20-d23}, [%2 :128]!   \n"

						"pld        [%4, #256]              \n"
						"vld1.f32   {d28-d31}, [%4 :128]    \n"
						"vmla.f32   q14, q6, %q15           \n"
						"vmla.f32   q15, q7, %q15           \n"

						"vst1.f32   {d24-d27}, [%3 :128]!   \n"

						"pld        [%5, #256]              \n"
						"vld1.f32   {d12-d15}, [%5 :128]!   \n"
						"subs       %0, #1                  \n"
						"vst1.f32   {d28-d31}, [%4 :128]!   \n"
						"bne        0b                      \n"
						"sub        %5, #32                 \n"
						: "=r"(nn),     // %0
						"=r"(outptr0),// %1
						"=r"(outptr1),// %2
						"=r"(outptr2),// %3
						"=r"(outptr3),// %4
						"=r"(r0)      // %5
						: "0"(nn),
						"1"(outptr0),
						"2"(outptr1),
						"3"(outptr2),
						"4"(outptr3),
						"5"(r0),
						"w"(_k0),     // %12
						"w"(_k1),     // %13
						"w"(_k2),     // %14
						"w"(_k3)      // %15
						: "cc", "memory", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}

#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					// TODO neon optimize
					float sum0 = *r0 * k0;
					float sum1 = *r0 * k1;
					float sum2 = *r0 * k2;
					float sum3 = *r0 * k3;

					*outptr0 += sum0;
					*outptr1 += sum1;
					*outptr2 += sum2;
					*outptr3 += sum3;

					r0++;
					outptr0++;
					outptr1++;
					outptr2++;
					outptr3++;
				}
			}
		}

		remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
        nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* out = out_mem + p * out_cstep; // Mat out = top_blob.channel(p);

			const float bias0 = bias ? bias[p] : 0.f;

			{ float* ptr = out; for (int t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out.fill(bias0);

			int q = 0;

			for (; q + 3 < inch; q += 4)
			{
				float* outptr = out;

				const float* img0 = in_mem + (q + 0) * cstep; // bottom_blob.channel(q);
				const float* img1 = in_mem + (q + 1) * cstep; // bottom_blob.channel(q + 1);
				const float* img2 = in_mem + (q + 2) * cstep; // bottom_blob.channel(q + 2);
				const float* img3 = in_mem + (q + 3) * cstep; // bottom_blob.channel(q + 3);

				const float* kernel0 = kernel + p*inch + q;
				const float k0 = kernel0[0];
				const float k1 = kernel0[1];
				const float k2 = kernel0[2];
				const float k3 = kernel0[3];

				const float* r0 = img0;
				const float* r1 = img1;
				const float* r2 = img2;
				const float* r3 = img3;

				int size = outw * outh;

#if __ARM_NEON
				int nn = size >> 3;
				int remain = size & 7;
#else
				int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
				float32x4_t _k0 = vdupq_n_f32(k0);
				float32x4_t _k1 = vdupq_n_f32(k1);
				float32x4_t _k2 = vdupq_n_f32(k2);
				float32x4_t _k3 = vdupq_n_f32(k3);

				if (nn > 0)
				{
					asm volatile(
						"pld        [%2, #256]          \n"
						"vld1.f32   {d4-d7}, [%2 :128]! \n"
						"0:                             \n"
						"pld        [%1, #256]          \n"
						"vld1.f32   {d0-d3}, [%1 :128]  \n"
						"vmla.f32   q0, q2, %q12        \n"
						"vmla.f32   q1, q3, %q12        \n"
						"pld        [%3, #256]          \n"
						"vld1.f32   {d4-d7}, [%3 :128]! \n"
						"vmla.f32   q0, q2, %q13        \n"
						"vmla.f32   q1, q3, %q13        \n"
						"pld        [%4, #256]          \n"
						"vld1.f32   {d4-d7}, [%4 :128]! \n"
						"vmla.f32   q0, q2, %q14        \n"
						"vmla.f32   q1, q3, %q14        \n"
						"pld        [%5, #256]          \n"
						"vld1.f32   {d4-d7}, [%5 :128]! \n"
						"vmla.f32   q0, q2, %q15        \n"
						"vmla.f32   q1, q3, %q15        \n"
						"pld        [%2, #256]          \n"
						"vld1.f32   {d4-d7}, [%2 :128]! \n"
						"subs       %0, #1              \n"
						"vst1.f32   {d0-d3}, [%1 :128]! \n"
						"bne        0b                  \n"
						"sub        %2, #32             \n"
						: "=r"(nn),     // %0
						"=r"(outptr), // %1
						"=r"(r0),     // %2
						"=r"(r1),     // %3
						"=r"(r2),     // %4
						"=r"(r3)      // %5
						: "0"(nn),
						"1"(outptr),
						"2"(r0),
						"3"(r1),
						"4"(r2),
						"5"(r3),
						"w"(_k0),     // %12
						"w"(_k1),     // %13
						"w"(_k2),     // %14
						"w"(_k3)      // %15
						: "cc", "memory", "q0", "q1", "q2", "q3"
						);
				}
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					float sum = *r0 * k0;
					float sum1 = *r1 * k1;
					float sum2 = *r2 * k2;
					float sum3 = *r3 * k3;

					*outptr += sum + sum1 + sum2 + sum3;

					r0++;
					r1++;
					r2++;
					r3++;
					outptr++;
				}

			}

			for (; q < inch; q++)
			{
				float* outptr = out;

				const float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* kernel0 = kernel + p*inch + q;
				const float k0 = kernel0[0];

				const float* r0 = img0;

				int size = outw * outh;

#if __ARM_NEON
				int nn = size >> 3;
				int remain = size & 7;
#else
				int remain = size;
#endif // __ARM_NEON

#if __ARM_NEON
				float32x4_t _k0 = vdupq_n_f32(k0);
				if (nn > 0)
				{
					asm volatile(
						"pld        [%2, #256]          \n"
						"vld1.f32   {d4-d7}, [%2 :128]! \n"
						"0:                             \n"
						"pld        [%1, #256]          \n"
						"vld1.f32   {d0-d3}, [%1 :128]  \n"
						"vmla.f32   q0, q2, %q6         \n"
						"vmla.f32   q1, q3, %q6         \n"
						"pld        [%2, #256]          \n"
						"vld1.f32   {d4-d7}, [%2 :128]! \n"
						"subs       %0, #1              \n"
						"vst1.f32   {d0-d3}, [%1 :128]! \n"
						"bne        0b                  \n"
						"sub        %2, #32             \n"
						: "=r"(nn),     // %0
						"=r"(outptr), // %1
						"=r"(r0)      // %2
						: "0"(nn),
						"1"(outptr),
						"2"(r0),
						"w"(_k0)      // %6
						: "cc", "memory", "q0", "q1", "q2", "q3"
						);
				}
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					float sum = *r0 * k0;

					*outptr += sum;

					r0++;
					outptr++;
				}
			}
		}
	}

	// void conv3x3s1_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& _kernel, const Mat& _bias, const Option& opt)
	void conv3x3s1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias, float* tmp)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
		int out_cstep = out_size;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 1;

		int nn_outch = outch >> 1;
		int remain_outch_start = nn_outch << 1;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = pp * 2;

			float* out0 = out_mem + (p + 0) * out_cstep; // Mat out0 = top_blob.channel(p);
			float* out1 = out_mem + (p + 1) * out_cstep; // Mat out1 = top_blob.channel(p + 1);

			const float bias0 = bias ? bias[p] : 0.f;
			const float bias1 = bias ? bias[p + 1] : 0.f;

			{ float* ptr = out0; for (int t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out0.fill(bias0);
			{ float* ptr = out1; for (int t = 0; t < out_size; t++) { *ptr = bias1; ptr++; } } // out1.fill(bias1);

			const float* k0 = kernel + p*inch * 9;
			const float* k1 = kernel + (p + 1)*inch * 9;

			for (int q = 0; q < inch; q++)
			{
				float* outptr0 = out0;
				float* outptr1 = out1;
				float* outptr0n = outptr0 + outw;
				float* outptr1n = outptr1 + outw;

				const float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* r0 = img0;
				const float* r1 = img0 + w;
				const float* r2 = img0 + w * 2;
				const float* r3 = img0 + w * 3;

#if __ARM_NEON
				float32x4_t _k00 = vld1q_f32(k0);
				float32x4_t _k03 = vld1q_f32(k0 + 3);
				float32x4_t _k06 = vld1q_f32(k0 + 6);

				float32x4_t _k10 = vld1q_f32(k1);
				float32x4_t _k13 = vld1q_f32(k1 + 3);
				float32x4_t _k16 = vld1q_f32(k1 + 6);
#endif // __ARM_NEON

				int i = 0;

				for (; i + 1 < outh; i += 2)
				{
#if __ARM_NEON
					int nn = outw >> 2;
					int remain = outw & 3;
#else
					int remain = outw;
#endif // __ARM_NEON

#if __ARM_NEON
					if (nn > 0)
					{
						asm volatile(

							"pld        [%5, #192]          \n"
							"vld1.f32   {d16-d18}, [%5 :64] \n"// r0
							"add        %5, #16             \n"

							"pld        [%8, #192]          \n"
							"vld1.f32   {d28-d30}, [%8]     \n"// r3
							"add        %8, #16             \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q14, q15, #2   \n"

							"0:                             \n"

							"pld        [%1, #128]          \n"
							"vld1.f32   {d12-d13}, [%1 :64] \n"// _sum0

							"pld        [%2, #128]          \n"
							"vld1.f32   {d14-d15}, [%2 :64] \n"// _sum1

							"vmla.f32   q6, q8, %e18[0]     \n"
							"vmla.f32   q7, q8, %e21[0]     \n"

							"pld        [%3, #128]          \n"
							"vld1.f32   {d24-d25}, [%3]     \n"// _sum0n

							"pld        [%4, #128]          \n"
							"vld1.f32   {d26-d27}, [%4]     \n"// _sum1n

							"vmla.f32   q12, q14, %e20[0]   \n"
							"vmla.f32   q13, q14, %e23[0]   \n"

							"vext.32    q8, q8, q9, #2      \n"
							"vext.32    q9, q14, q15, #1    \n"

							"vmla.f32   q6, q10, %e18[1]    \n"
							"vmla.f32   q7, q10, %e21[1]    \n"
							"vmla.f32   q12, q11, %f20[0]   \n"
							"vmla.f32   q13, q11, %f23[0]   \n"

							"pld        [%6, #192]          \n"
							"vld1.f32   {d28-d30}, [%6]     \n"// r1
							"add        %6, #16             \n"

							"vmla.f32   q6, q8, %f18[0]     \n"
							"vmla.f32   q7, q8, %f21[0]     \n"
							"vmla.f32   q12, q9, %e20[1]    \n"
							"vmla.f32   q13, q9, %e23[1]    \n"

							"vext.32    q10, q14, q15, #1   \n"

							"vmla.f32   q6, q14, %e19[0]    \n"
							"vmla.f32   q7, q14, %e22[0]    \n"
							"vmla.f32   q12, q14, %e18[0]   \n"
							"vmla.f32   q13, q14, %e21[0]   \n"

							"vext.32    q11, q14, q15, #2   \n"

							"vmla.f32   q6, q10, %e19[1]    \n"
							"vmla.f32   q7, q10, %e22[1]    \n"
							"vmla.f32   q12, q10, %e18[1]   \n"
							"vmla.f32   q13, q10, %e21[1]   \n"

							"pld        [%7, #192]          \n"
							"vld1.f32   {d16-d18}, [%7 :64] \n"// r2
							"add        %7, #16             \n"

							"vmla.f32   q6, q11, %f19[0]    \n"
							"vmla.f32   q7, q11, %f22[0]    \n"
							"vmla.f32   q12, q11, %f18[0]   \n"
							"vmla.f32   q13, q11, %f21[0]   \n"

							"vext.32    q10, q8, q9, #1     \n"

							"vmla.f32   q6, q8, %e20[0]     \n"
							"vmla.f32   q7, q8, %e23[0]     \n"
							"vmla.f32   q12, q8, %e19[0]    \n"
							"vmla.f32   q13, q8, %e22[0]    \n"

							"vext.32    q11, q8, q9, #2     \n"

							"vmla.f32   q6, q10, %e20[1]    \n"
							"vmla.f32   q7, q10, %e23[1]    \n"
							"vmla.f32   q12, q10, %e19[1]   \n"
							"vmla.f32   q13, q10, %e22[1]   \n"

							"pld        [%5, #192]          \n"
							"vld1.f32   {d16-d18}, [%5 :64] \n"// r0
							"add        %5, #16             \n"

							"vmla.f32   q6, q11, %f20[0]    \n"
							"vmla.f32   q7, q11, %f23[0]    \n"
							"vmla.f32   q12, q11, %f19[0]   \n"
							"vmla.f32   q13, q11, %f22[0]   \n"

							"pld        [%8, #192]          \n"
							"vld1.f32   {d28-d30}, [%8]     \n"// r3
							"add        %8, #16             \n"

							"vext.32    q10, q8, q9, #1     \n"

							"vst1.f32   {d12-d13}, [%1 : 64]!\n"
							"vst1.f32   {d14-d15}, [%2 : 64]!\n"

							"vext.32    q11, q14, q15, #2   \n"

							"vst1.f32   {d24-d25}, [%3]!    \n"
							"vst1.f32   {d26-d27}, [%4]!    \n"

							"subs       %0, #1              \n"
							"bne        0b                  \n"

							"sub        %5, #16             \n"
							"sub        %8, #16             \n"
							: "=r"(nn),         // %0
							"=r"(outptr0),    // %1
							"=r"(outptr1),    // %2
							"=r"(outptr0n),   // %3
							"=r"(outptr1n),   // %4
							"=r"(r0),         // %5
							"=r"(r1),         // %6
							"=r"(r2),         // %7
							"=r"(r3)          // %8
							: "0"(nn),
							"1"(outptr0),
							"2"(outptr1),
							"3"(outptr0n),
							"4"(outptr1n),
							"5"(r0),
							"6"(r1),
							"7"(r2),
							"8"(r3),
							"w"(_k00),      // %18
							"w"(_k03),      // %19
							"w"(_k06),      // %20
							"w"(_k10),      // %21
							"w"(_k13),      // %22
							"w"(_k16)       // %23
							: "cc", "memory", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
#if __ARM_NEON
						float32x4_t _r00 = vld1q_f32(r0);
						float32x4_t _r10 = vld1q_f32(r1);
						float32x4_t _r20 = vld1q_f32(r2);
						float32x4_t _r30 = vld1q_f32(r3);

						float32x4_t _sum0 = vmulq_f32(_r00, _k00);
						float32x4_t _sum1 = vmulq_f32(_r00, _k10);
						_sum0 = vmlaq_f32(_sum0, _r10, _k03);
						_sum1 = vmlaq_f32(_sum1, _r10, _k13);
						_sum0 = vmlaq_f32(_sum0, _r20, _k06);
						_sum1 = vmlaq_f32(_sum1, _r20, _k16);

						float32x4_t _sum0n = vmulq_f32(_r10, _k00);
						float32x4_t _sum1n = vmulq_f32(_r10, _k10);
						_sum0n = vmlaq_f32(_sum0n, _r20, _k03);
						_sum1n = vmlaq_f32(_sum1n, _r20, _k13);
						_sum0n = vmlaq_f32(_sum0n, _r30, _k06);
						_sum1n = vmlaq_f32(_sum1n, _r30, _k16);

						_sum0 = vsetq_lane_f32(*outptr0, _sum0, 3);
						_sum1 = vsetq_lane_f32(*outptr1, _sum1, 3);
						_sum0n = vsetq_lane_f32(*outptr0n, _sum0n, 3);
						_sum1n = vsetq_lane_f32(*outptr1n, _sum1n, 3);

						float32x2_t _ss0 = vadd_f32(vget_low_f32(_sum0), vget_high_f32(_sum0));
						float32x2_t _ss1 = vadd_f32(vget_low_f32(_sum1), vget_high_f32(_sum1));
						float32x2_t _ss0n = vadd_f32(vget_low_f32(_sum0n), vget_high_f32(_sum0n));
						float32x2_t _ss1n = vadd_f32(vget_low_f32(_sum1n), vget_high_f32(_sum1n));

						float32x2_t _ss01 = vpadd_f32(_ss0, _ss1);
						float32x2_t _ss01n = vpadd_f32(_ss0n, _ss1n);

						*outptr0 = vget_lane_f32(_ss01, 0);
						*outptr1 = vget_lane_f32(_ss01, 1);
						*outptr0n = vget_lane_f32(_ss01n, 0);
						*outptr1n = vget_lane_f32(_ss01n, 1);
#else
						float sum0 = 0.f;
						float sum0n = 0.f;
						float sum1 = 0.f;
						float sum1n = 0.f;

						sum0 += r0[0] * k0[0];
						sum0 += r0[1] * k0[1];
						sum0 += r0[2] * k0[2];
						sum0 += r1[0] * k0[3];
						sum0 += r1[1] * k0[4];
						sum0 += r1[2] * k0[5];
						sum0 += r2[0] * k0[6];
						sum0 += r2[1] * k0[7];
						sum0 += r2[2] * k0[8];

						sum1 += r0[0] * k1[0];
						sum1 += r0[1] * k1[1];
						sum1 += r0[2] * k1[2];
						sum1 += r1[0] * k1[3];
						sum1 += r1[1] * k1[4];
						sum1 += r1[2] * k1[5];
						sum1 += r2[0] * k1[6];
						sum1 += r2[1] * k1[7];
						sum1 += r2[2] * k1[8];

						sum0n += r1[0] * k0[0];
						sum0n += r1[1] * k0[1];
						sum0n += r1[2] * k0[2];
						sum0n += r2[0] * k0[3];
						sum0n += r2[1] * k0[4];
						sum0n += r2[2] * k0[5];
						sum0n += r3[0] * k0[6];
						sum0n += r3[1] * k0[7];
						sum0n += r3[2] * k0[8];

						sum1n += r1[0] * k1[0];
						sum1n += r1[1] * k1[1];
						sum1n += r1[2] * k1[2];
						sum1n += r2[0] * k1[3];
						sum1n += r2[1] * k1[4];
						sum1n += r2[2] * k1[5];
						sum1n += r3[0] * k1[6];
						sum1n += r3[1] * k1[7];
						sum1n += r3[2] * k1[8];

						*outptr0 += sum0;
						*outptr1 += sum1;
						*outptr0n += sum0n;
						*outptr1n += sum1n;
#endif // __ARM_NEON
						r0++;
						r1++;
						r2++;
						r3++;
						outptr0++;
						outptr1++;
						outptr0n++;
						outptr1n++;
					}

					r0 += 2 + w;
					r1 += 2 + w;
					r2 += 2 + w;
					r3 += 2 + w;

					outptr0 += outw;
					outptr1 += outw;
					outptr0n += outw;
					outptr1n += outw;
				}

				for (; i < outh; i++)
				{
#if __ARM_NEON
					int nn = outw >> 2;
					int remain = outw & 3;
#else
					int remain = outw;
#endif // __ARM_NEON

#if __ARM_NEON
					if (nn > 0)
					{
						asm volatile(
							"0:                             \n"

							"pld        [%3, #192]          \n"
							"vld1.f32   {d16-d18}, [%3]     \n"// r0
							"add        %3, #16             \n"

							"pld        [%1, #128]          \n"
							"vld1.f32   {d12-d13}, [%1]     \n"// _sum0

							"pld        [%2, #128]          \n"
							"vld1.f32   {d14-d15}, [%2]     \n"// _sum1

							"vmul.f32   q14, q8, %e12[0]    \n"
							"vmul.f32   q15, q8, %e15[0]    \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"

							"vmla.f32   q6, q10, %e12[1]    \n"
							"vmla.f32   q7, q10, %e15[1]    \n"

							"pld        [%4, #192]          \n"
							"vld1.f32   {d16-d18}, [%4]     \n"// r1
							"add        %4, #16             \n"

							"vmla.f32   q14, q11, %f12[0]   \n"
							"vmla.f32   q15, q11, %f15[0]   \n"

							"vmla.f32   q6, q8, %e13[0]     \n"
							"vmla.f32   q7, q8, %e16[0]     \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"

							"vmla.f32   q14, q10, %e13[1]   \n"
							"vmla.f32   q15, q10, %e16[1]   \n"

							"pld        [%5, #192]          \n"
							"vld1.f32   {d16-d18}, [%5]     \n"// r2
							"add        %5, #16             \n"

							"vmla.f32   q6, q11, %f13[0]    \n"
							"vmla.f32   q7, q11, %f16[0]    \n"

							"vmla.f32   q14, q8, %e14[0]    \n"
							"vmla.f32   q15, q8, %e17[0]    \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"

							"vmla.f32   q6, q10, %e14[1]    \n"
							"vmla.f32   q7, q10, %e17[1]    \n"

							"vmla.f32   q14, q11, %f14[0]   \n"
							"vmla.f32   q15, q11, %f17[0]   \n"

							"vadd.f32   q6, q6, q14         \n"
							"vadd.f32   q7, q7, q15         \n"

							"vst1.f32   {d12-d13}, [%1]!    \n"

							"vst1.f32   {d14-d15}, [%2]!    \n"

							"subs       %0, #1              \n"
							"bne        0b                  \n"

							: "=r"(nn),         // %0
							"=r"(outptr0),    // %1
							"=r"(outptr1),    // %2
							"=r"(r0),         // %3
							"=r"(r1),         // %4
							"=r"(r2)          // %5
							: "0"(nn),
							"1"(outptr0),
							"2"(outptr1),
							"3"(r0),
							"4"(r1),
							"5"(r2),
							"w"(_k00),      // %12
							"w"(_k03),      // %13
							"w"(_k06),      // %14
							"w"(_k10),      // %15
							"w"(_k13),      // %16
							"w"(_k16)       // %17
							: "cc", "memory", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
#if __ARM_NEON
						float32x4_t _r00 = vld1q_f32(r0);
						float32x4_t _r10 = vld1q_f32(r1);
						float32x4_t _r20 = vld1q_f32(r2);

						float32x4_t _sum0 = vmulq_f32(_r00, _k00);
						float32x4_t _sum1 = vmulq_f32(_r00, _k10);
						_sum0 = vmlaq_f32(_sum0, _r10, _k03);
						_sum1 = vmlaq_f32(_sum1, _r10, _k13);
						_sum0 = vmlaq_f32(_sum0, _r20, _k06);
						_sum1 = vmlaq_f32(_sum1, _r20, _k16);

						_sum0 = vsetq_lane_f32(*outptr0, _sum0, 3);
						_sum1 = vsetq_lane_f32(*outptr1, _sum1, 3);

						float32x2_t _ss0 = vadd_f32(vget_low_f32(_sum0), vget_high_f32(_sum0));
						float32x2_t _ss1 = vadd_f32(vget_low_f32(_sum1), vget_high_f32(_sum1));
						float32x2_t _ss01 = vpadd_f32(_ss0, _ss1);

						*outptr0 = vget_lane_f32(_ss01, 0);
						*outptr1 = vget_lane_f32(_ss01, 1);
#else
						float sum0 = 0.f;
						float sum1 = 0.f;

						sum0 += r0[0] * k0[0];
						sum0 += r0[1] * k0[1];
						sum0 += r0[2] * k0[2];
						sum0 += r1[0] * k0[3];
						sum0 += r1[1] * k0[4];
						sum0 += r1[2] * k0[5];
						sum0 += r2[0] * k0[6];
						sum0 += r2[1] * k0[7];
						sum0 += r2[2] * k0[8];

						sum1 += r0[0] * k1[0];
						sum1 += r0[1] * k1[1];
						sum1 += r0[2] * k1[2];
						sum1 += r1[0] * k1[3];
						sum1 += r1[1] * k1[4];
						sum1 += r1[2] * k1[5];
						sum1 += r2[0] * k1[6];
						sum1 += r2[1] * k1[7];
						sum1 += r2[2] * k1[8];

						*outptr0 += sum0;
						*outptr1 += sum1;
#endif // __ARM_NEON
						r0++;
						r1++;
						r2++;
						outptr0++;
						outptr1++;
					}

					r0 += 2;
					r1 += 2;
					r2 += 2;
				}

				k0 += 9;
				k1 += 9;
			}
		}

#if ((ENGINE_THREAD_COUNT) != 1)
        nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* out = out_mem + p * out_cstep; // Mat out = top_blob.channel(p);

			const float bias0 = bias ? bias[p] : 0.f;

			{ float* ptr = out; for (int t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out.fill(bias0);

			const float* kernel0 = kernel + p*inch * 9;

			for (int q = 0; q < inch; q++)
			{
				float* outptr = out;
				float* outptr2 = outptr + outw;

				const float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* r0 = img0;
				const float* r1 = img0 + w;
				const float* r2 = img0 + w * 2;
				const float* r3 = img0 + w * 3;

#if __ARM_NEON
				float32x4_t _k0123 = vld1q_f32(kernel0);
				float32x4_t _k3456 = vld1q_f32(kernel0 + 3);
				float32x4_t _k6789 = vld1q_f32(kernel0 + 6);
#else
				const float* k0 = kernel0;
				const float* k1 = kernel0 + 3;
				const float* k2 = kernel0 + 6;
#endif // __ARM_NEON

				int i = 0;

				for (; i + 1 < outh; i += 2)
				{

#if __ARM_NEON
					int nn = outw >> 2;
					int remain = outw & 3;
#else
					int remain = outw;
#endif // __ARM_NEON

#if __ARM_NEON
					if (nn > 0)
					{
						asm volatile(
							"pld        [%3, #192]          \n"
							"vld1.f32   {d18-d20}, [%3 :64] \n"// r0
							"add        %3, #16             \n"

							"vext.32    q11, q9, q10, #1    \n"
							"vext.32    q12, q9, q10, #2    \n"

							"0:                             \n"

							"pld        [%1, #128]          \n"
							"vld1.f32   {d14-d15}, [%1 :64] \n"// _sum

							"vmla.f32   q7, q9, %e14[0]     \n"
							"vmul.f32   q6, q11, %e14[1]    \n"
							"vmul.f32   q13, q12, %f14[0]   \n"

							"pld        [%4, #192]          \n"
							"vld1.f32   {d18-d20}, [%4]     \n"// r1
							"add        %4, #16             \n"

							"vmla.f32   q7, q9, %e15[0]     \n"

							"vext.32    q11, q9, q10, #1    \n"
							"vext.32    q12, q9, q10, #2    \n"

							"vmla.f32   q6, q11, %e15[1]    \n"
							"vmla.f32   q13, q12, %f15[0]   \n"

							"pld        [%2, #128]          \n"
							"vld1.f32   {d16-d17}, [%2]     \n"// _sum2

							"vmla.f32   q8, q9, %e14[0]     \n"
							"vmul.f32   q14, q11, %e14[1]   \n"
							"vmul.f32   q15, q12, %f14[0]   \n"

							"pld        [%5, #192]          \n"
							"vld1.f32   {d18-d20}, [%5 :64] \n"// r2
							"add        %5, #16             \n"

							"vmla.f32   q7, q9, %e16[0]     \n"

							"vext.32    q11, q9, q10, #1    \n"
							"vext.32    q12, q9, q10, #2    \n"

							"vmla.f32   q6, q11, %e16[1]    \n"
							"vmla.f32   q13, q12, %f16[0]   \n"

							"vmla.f32   q8, q9, %e15[0]     \n"
							"vmla.f32   q14, q11, %e15[1]   \n"
							"vmla.f32   q15, q12, %f15[0]   \n"

							"pld        [%6, #192]          \n"
							"vld1.f32   {d18-d20}, [%6]     \n"// r3
							"add        %6, #16             \n"

							"vmla.f32   q8, q9, %e16[0]     \n"

							"vext.32    q11, q9, q10, #1    \n"
							"vext.32    q12, q9, q10, #2    \n"

							"vmla.f32   q14, q11, %e16[1]   \n"
							"vmla.f32   q15, q12, %f16[0]   \n"

							"vadd.f32   q7, q7, q6          \n"

							"pld        [%3, #192]          \n"
							"vld1.f32   {d18-d20}, [%3 :64] \n"// r0

							"vadd.f32   q8, q8, q14         \n"
							"vadd.f32   q7, q7, q13         \n"
							"vadd.f32   q8, q8, q15         \n"

							"vext.32    q11, q9, q10, #1    \n"
							"vext.32    q12, q9, q10, #2    \n"

							"add        %3, #16             \n"

							"vst1.f32   {d14-d15}, [%1]!    \n"
							"vst1.f32   {d16-d17}, [%2]!    \n"

							"subs       %0, #1              \n"
							"bne        0b                  \n"

							"sub        %3, #16             \n"
							: "=r"(nn),         // %0
							"=r"(outptr),     // %1
							"=r"(outptr2),    // %2
							"=r"(r0),         // %3
							"=r"(r1),         // %4
							"=r"(r2),         // %5
							"=r"(r3)          // %6
							: "0"(nn),
							"1"(outptr),
							"2"(outptr2),
							"3"(r0),
							"4"(r1),
							"5"(r2),
							"6"(r3),
							"w"(_k0123),      // %14
							"w"(_k3456),      // %15
							"w"(_k6789)       // %16
							: "cc", "memory", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
#if __ARM_NEON
						float32x4_t _r00 = vld1q_f32(r0);
						float32x4_t _r10 = vld1q_f32(r1);
						float32x4_t _r20 = vld1q_f32(r2);
						float32x4_t _r30 = vld1q_f32(r3);

						float32x4_t _sum = vmulq_f32(_r00, _k0123);
						_sum = vmlaq_f32(_sum, _r10, _k3456);
						_sum = vmlaq_f32(_sum, _r20, _k6789);

						float32x4_t _sum2 = vmulq_f32(_r10, _k0123);
						_sum2 = vmlaq_f32(_sum2, _r20, _k3456);
						_sum2 = vmlaq_f32(_sum2, _r30, _k6789);

						_sum = vsetq_lane_f32(*outptr, _sum, 3);
						_sum2 = vsetq_lane_f32(*outptr2, _sum2, 3);

						float32x2_t _ss = vadd_f32(vget_low_f32(_sum), vget_high_f32(_sum));
						float32x2_t _ss2 = vadd_f32(vget_low_f32(_sum2), vget_high_f32(_sum2));

						float32x2_t _sss2 = vpadd_f32(_ss, _ss2);

						*outptr = vget_lane_f32(_sss2, 0);
						*outptr2 = vget_lane_f32(_sss2, 1);
#else
						float sum = 0;
						float sum2 = 0;

						sum += r0[0] * k0[0];
						sum += r0[1] * k0[1];
						sum += r0[2] * k0[2];
						sum += r1[0] * k1[0];
						sum += r1[1] * k1[1];
						sum += r1[2] * k1[2];
						sum += r2[0] * k2[0];
						sum += r2[1] * k2[1];
						sum += r2[2] * k2[2];

						sum2 += r1[0] * k0[0];
						sum2 += r1[1] * k0[1];
						sum2 += r1[2] * k0[2];
						sum2 += r2[0] * k1[0];
						sum2 += r2[1] * k1[1];
						sum2 += r2[2] * k1[2];
						sum2 += r3[0] * k2[0];
						sum2 += r3[1] * k2[1];
						sum2 += r3[2] * k2[2];

						*outptr += sum;
						*outptr2 += sum2;
#endif
						r0++;
						r1++;
						r2++;
						r3++;
						outptr++;
						outptr2++;
					}

					r0 += 2 + w;
					r1 += 2 + w;
					r2 += 2 + w;
					r3 += 2 + w;

					outptr += outw;
					outptr2 += outw;
				}

				for (; i < outh; i++)
				{

#if __ARM_NEON
					int nn = outw >> 2;
					int remain = outw & 3;
#else
					int remain = outw;
#endif // __ARM_NEON

#if __ARM_NEON
					if (nn > 0)
					{
						asm volatile(
							"pld        [%2, #192]          \n"
							"vld1.f32   {d16-d18}, [%2]     \n"// r0
							"add        %2, #16             \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"

							"0:                             \n"

							"pld        [%1, #128]          \n"
							"vld1.f32   {d14-d15}, [%1]     \n"// _sum

							"vmla.f32   q7, q8, %e10[0]     \n"
							"vmul.f32   q13, q10, %e10[1]   \n"
							"vmul.f32   q14, q11, %f10[0]   \n"

							"pld        [%3, #192]          \n"
							"vld1.f32   {d16-d18}, [%3]     \n"// r1
							"add        %3, #16             \n"

							"vmla.f32   q7, q8, %e11[0]     \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"

							"vmla.f32   q13, q10, %e11[1]   \n"
							"vmla.f32   q14, q11, %f11[0]   \n"

							"pld        [%4, #192]          \n"
							"vld1.f32   {d16-d18}, [%4]     \n"// r2
							"add        %4, #16             \n"

							"vmla.f32   q7, q8, %e12[0]     \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"

							"vmla.f32   q13, q10, %e12[1]   \n"
							"vmla.f32   q14, q11, %f12[0]   \n"

							"pld        [%2, #192]          \n"
							"vld1.f32   {d16-d18}, [%2]     \n"// r0
							"add        %2, #16             \n"

							"vadd.f32   q7, q7, q13         \n"
							"vadd.f32   q7, q7, q14         \n"

							"vext.32    q10, q8, q9, #1     \n"
							"vext.32    q11, q8, q9, #2     \n"

							"vst1.f32   {d14-d15}, [%1]!    \n"

							"subs       %0, #1              \n"
							"bne        0b                  \n"

							"sub        %2, #16             \n"
							: "=r"(nn),         // %0
							"=r"(outptr),     // %1
							"=r"(r0),         // %2
							"=r"(r1),         // %3
							"=r"(r2)          // %4
							: "0"(nn),
							"1"(outptr),
							"2"(r0),
							"3"(r1),
							"4"(r2),
							"w"(_k0123),      // %10
							"w"(_k3456),      // %11
							"w"(_k6789)       // %12
							: "cc", "memory", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
#if __ARM_NEON
						float32x4_t _r00 = vld1q_f32(r0);
						float32x4_t _r10 = vld1q_f32(r1);
						float32x4_t _r20 = vld1q_f32(r2);

						float32x4_t _sum = vmulq_f32(_r00, _k0123);
						_sum = vmlaq_f32(_sum, _r10, _k3456);
						_sum = vmlaq_f32(_sum, _r20, _k6789);

						_sum = vsetq_lane_f32(*outptr, _sum, 3);

						float32x2_t _ss = vadd_f32(vget_low_f32(_sum), vget_high_f32(_sum));
						_ss = vpadd_f32(_ss, _ss);

						*outptr = vget_lane_f32(_ss, 0);
#else
						float sum = 0;

						sum += r0[0] * k0[0];
						sum += r0[1] * k0[1];
						sum += r0[2] * k0[2];
						sum += r1[0] * k1[0];
						sum += r1[1] * k1[1];
						sum += r1[2] * k1[2];
						sum += r2[0] * k2[0];
						sum += r2[1] * k2[1];
						sum += r2[2] * k2[2];

						*outptr += sum;
#endif
						r0++;
						r1++;
						r2++;
						outptr++;
					}

					r0 += 2;
					r1 += 2;
					r2 += 2;
				}

				kernel0 += 9;
			}
		}
	}

	void conv1x1s1_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = w;
		int outh = h;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out / 4;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* outptr = out_mem + p * out_cstep; // top_blob.channel(p);

#if __ARM_NEON
			float32x4_t sum0 = vdupq_n_f32(0.f);
			if (bias) sum0 = vld1q_f32(bias + p * 4);
#endif
			for (int i = 0; i < out_size; i++)
			{
#if __ARM_NEON
				float32x4_t _sum = sum0;
#else
				float _sum[4] = { 0.0f };
				if (bias)
				{
					_sum[0] = bias[p * 4 + 0];
					_sum[1] = bias[p * 4 + 1];
					_sum[2] = bias[p * 4 + 2];
					_sum[3] = bias[p * 4 + 3];
				}
#endif
				const float* kptr = kernel + inch * p * 4;

				// channels
				for (int q = 0; q < inch; q++)
				{
					float* m = in_mem + q * cstep + i; // bottom_blob_bordered.channel(q);
#if __ARM_NEON
					float32x4_t _val = vdupq_n_f32(m[0]);
					float32x4_t _w = vld1q_f32(kptr);
					_sum = vmlaq_f32(_sum, _val, _w);
#else
					_sum[0] += m[0] * kptr[0];
					_sum[1] += m[0] * kptr[1];
					_sum[2] += m[0] * kptr[2];
					_sum[3] += m[0] * kptr[3];
#endif
					kptr += 4;
				}

#if __ARM_NEON
				vst1q_f32(outptr, _sum);
#else
				outptr[0] = _sum[0];
				outptr[1] = _sum[1];
				outptr[2] = _sum[2];
				outptr[3] = _sum[3];
#endif
				outptr += 4;
			}
		}
	}

	void convdw_gdc_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, float* kernel, float* bias)
	{
		convdw_gdc_pack4_neon(in_blob, out_blob, dim_ch_out, dim_kernel, dim_kernel, stride, kernel, bias);
	}

	void convdw_gdc_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel_w, int dim_kernel_h, int stride, float* kernel, float* bias)
	{
		const int maxk = dim_kernel_w * dim_kernel_h;

		//int w = in_blob->w;
		//int h = in_blob->h;
		//int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = 1;
		int outh = 1;
		//int out_size = 1;
		int out_cstep = 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = dim_ch_out / 4;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < outch; g++)
		{
			float* outptr = out_mem + g * out_cstep; // top_blob.channel(g);
			const float* kptr = kernel + maxk * g * 4; // (const float*)weight_data_pack4 + maxk * g * 4;
			const float* m = in_mem + g * cstep; // const Mat m = bottom_blob_bordered.channel(g);

#if __ARM_NEON
			float32x4_t _sum = vdupq_n_f32(0.f);

			if (bias)
			{
				_sum = vld1q_f32(((const float*)bias) + g * 4);
			}

			//const float* sptr = m; // m.row(i*stride_h) + j*stride_w * 4;

			for (int k = 0; k < maxk; k++)
			{
				float32x4_t _val = vld1q_f32(m + k * 4);
				float32x4_t _w = vld1q_f32(kptr + k * 4);
				_sum = vmlaq_f32(_sum, _val, _w);
			}

			// _sum = activation_ps(_sum, activation_type, activation_params);
			vst1q_f32(outptr, _sum);
#else
			float sum0 = 0.0f, sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;

			if (bias)
			{
				sum0 = bias[g * 4];
				sum1 = bias[g * 4 + 1];
				sum2 = bias[g * 4 + 2];
				sum3 = bias[g * 4 + 3];
			}

			// const float* sptr = m; // m.row(i*stride_h) + j*stride_w * 4;

			for (int k = 0; k < maxk; k++)
			{
				sum0 += m[k * 4 + 0] * kptr[k * 4 + 0];
				sum1 += m[k * 4 + 1] * kptr[k * 4 + 1];
				sum2 += m[k * 4 + 2] * kptr[k * 4 + 2];
				sum3 += m[k * 4 + 3] * kptr[k * 4 + 3];
			}

			outptr[0] = sum0;
			outptr[1] = sum1;
			outptr[2] = sum2;
			outptr[3] = sum3;
#endif
		}
	}
}
