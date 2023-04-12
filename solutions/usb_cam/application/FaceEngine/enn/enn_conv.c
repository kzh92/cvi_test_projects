
#if __ARM_NEON
#include <arm_neon.h>
#endif

#include <memory.h>
#include "enn_conv.h"

extern int g_nThreadCount;

    void enn_conv1x1s1_sgemm_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias, float* tmp)// tmp /// 8 * inch ( = real_inch / 4) * 4 * (size / 8 + (size % 8) / 4 + size % 4)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
        int out_cstep = enn_get_blob_size(out_size, 4);
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

				int q;
				for (q = 0; q < inch; q++)
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

				int q;
				for (q = 0; q < inch; q++)
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

				int q;
				for (q = 0; q < inch; q++)
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

			const unsigned short zeros[4] = { 0, 0, 0, 0 };
			const unsigned short* biasptr = bias ? bias + p : zeros;

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				float* tmpptr = tmp + (i / 8) * 32 * inch; // .channel(i / 8);
				const unsigned short* kptr = kernel + (p / 4) * inch * 16; // (const float*)kernel.channel(p / 4);
				int nn = inch;// inch always > 0

				asm volatile(
					// "vld1.f32   {d30-d31}, [%14] \n"
					"vld1.u16   {d30}, [%14]	\n"
					"vcvt.f32.f16 q15, d30      \n"

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
					//"vldm       %6!, {d8-d15}   \n"
					"vldm       %6!, {d8-d11}   \n"

					"vcvt.f32.f16 q7, d11		\n"
					"vcvt.f32.f16 q6, d10		\n"
					"vcvt.f32.f16 q5, d9		\n"
					"vcvt.f32.f16 q4, d8		\n"

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
				const unsigned short* kptr = kernel + (p / 4) * 16 * inch; // (const float*)kernel.channel(p / 4);
				int nn = inch;// inch always > 0

				asm volatile(
					// "vld1.f32   {d22-d23}, [%14] \n"
					"vld1.u16   {d22}, [%14]    \n"
					"vcvt.f32.f16 q11, d22      \n"

					"vdup.f32   q8, d22[0]      \n"
					"vdup.f32   q9, d22[1]      \n"
					"vdup.f32   q10, d23[0]     \n"
					"vdup.f32   q11, d23[1]     \n"

					"0:                         \n"

					"pld        [%5, #512]      \n"
					"vldm       %5!, {d0-d7}    \n"

					"pld        [%6, #512]      \n"
					//"vldm       %6!, {d8-d15}   \n"
					"vldm       %6!, {d8-d11}   \n"
					"vcvt.f32.f16 q7, d11		\n"
					"vcvt.f32.f16 q6, d10		\n"
					"vcvt.f32.f16 q5, d9		\n"
					"vcvt.f32.f16 q4, d8		\n"

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
				const unsigned short* kptr = kernel + (p / 4) * 16 * inch; // (const float*)kernel.channel(p / 4);
				int nn = inch;// inch always > 0

				asm volatile(
					// "vld1.f32   {d16-d17}, [%14] \n"
					"vld1.u16   {d16}, [%14]	\n"
					"vcvt.f32.f16 q8, d16		\n"
					"veor       q9, q9          \n"
					"veor       q10, q10        \n"
					"veor       q11, q11        \n"

					"0:                         \n"

					"pld        [%5, #128]      \n"
					"vld1.f32   {d0-d1}, [%5]!  \n"

					"pld        [%6, #512]      \n"
					// "vldm       %6!, {d8-d15}   \n"
					"vldm       %6!, {d8-d11}   \n"
					"vcvt.f32.f16 q7, d11		\n"
					"vcvt.f32.f16 q6, d10		\n"
					"vcvt.f32.f16 q5, d9		\n"
					"vcvt.f32.f16 q4, d8		\n"

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

			const unsigned short bias0 = bias ? bias[p] : 0;

			int i = 0;
			for (; i + 7 < size; i += 8)
			{
				float* tmpptr = tmp + (i / 8) * 32 * inch; // .channel(i / 8);
				const unsigned short* kptr = kernel + (p / 4 + p % 4) * 16 * inch; // (const float*)kernel.channel(p / 4 + p % 4);
				int nn = inch;// inch always > 0

				asm volatile(
					// "vdup.f32   q8, %8          \n"
					// "vdup.f32   q9, %8          \n"
					"vdup.u16	d16, %8			\n"
					"vcvt.f32.f16 q8, d16		\n"
					"vmov.f32	q9, q8			\n"
					"veor       q10, q10        \n"
					"veor       q11, q11        \n"

					"0:                         \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"pld        [%3, #128]      \n"
					// "vld1.f32   {d8-d9}, [%3]!  \n"
					"vld1.u16   {d8}, [%3]!     \n"
					"vcvt.f32.f16 q4, d8		\n"

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
				const unsigned short* kptr = kernel + (p / 4 + p % 4) * 16 * inch; // (const float*)kernel.channel(p / 4 + p % 4);
				int nn = inch;// inch always > 0

				asm volatile(
					// "vdup.f32   q8, %8          \n"
					"vdup.u16   d16, %8         \n"
					"vcvt.f32.f16 q8, d16		\n"
					"veor       q9, q9          \n"
					"veor       q10, q10        \n"
					"veor       q11, q11        \n"

					"0:                         \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"pld        [%3, #128]      \n"
					// "vld1.f32   {d8-d9}, [%3]!  \n"
					"vld1.u16   {d8}, [%3]!		\n"
					"vcvt.f32.f16 q4, d8		\n"

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
				const unsigned short* kptr = kernel + (p / 4 + p % 4) * 16 * inch; // (const float*)kernel.channel(p / 4 + p % 4);
				float32x4_t _sum0 = vdupq_n_f32(0.f);

				int q;
				for (q = 0; q < inch; q++)
				{
					float32x4_t _r0 = vld1q_f32(tmpptr);

					// float32x4_t _k0 = vld1q_f32(kptr);
					float32x4_t _k0 = vcvt_f32_f16((float16x4_t)vld1_u16(kptr));
					_sum0 = vmlaq_f32(_sum0, _r0, _k0);

					kptr += 4;
					tmpptr += 4;
				}

				float32x2_t _ss = vadd_f32(vget_low_f32(_sum0), vget_high_f32(_sum0));
				float32x2_t _ss2 = vpadd_f32(_ss, _ss);
				float sum0 = vget_lane_f32(_ss2, 0);

				outptr0[0] = _gnu_h2f_internal_f(bias0) + sum0;

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
			float kptrf[16];

			float* outptr0 = out_mem + (pp * 4 + 0) * out_cstep;
			float* outptr1 = out_mem + (pp * 4 + 1) * out_cstep;
			float* outptr2 = out_mem + (pp * 4 + 2) * out_cstep;
			float* outptr3 = out_mem + (pp * 4 + 3) * out_cstep;

			for (int i = 0; i < size; i++)
			{
				float sum[4];

				sum[0] = (bias) ? _gnu_h2f_internal_f(bias[pp * 4 + 0]) : 0.0f;
				sum[1] = (bias) ? _gnu_h2f_internal_f(bias[pp * 4 + 1]) : 0.0f;
				sum[2] = (bias) ? _gnu_h2f_internal_f(bias[pp * 4 + 2]) : 0.0f;
				sum[3] = (bias) ? _gnu_h2f_internal_f(bias[pp * 4 + 3]) : 0.0f;

				const unsigned short* kptr = kernel + inch * pp * 16;
				for (int q = 0; q < inch; q++)
				{
					_gnu_h2f_internal_vector(kptr, kptrf, 16);
					const float* sptr = in_mem + q * cstep + i * 4;

					sum[0] += kptrf[0] * sptr[0];
					sum[1] += kptrf[1] * sptr[0];
					sum[2] += kptrf[2] * sptr[0];
					sum[3] += kptrf[3] * sptr[0];

					sum[0] += kptrf[4] * sptr[1];
					sum[1] += kptrf[5] * sptr[1];
					sum[2] += kptrf[6] * sptr[1];
					sum[3] += kptrf[7] * sptr[1];

					sum[0] += kptrf[8] * sptr[2];
					sum[1] += kptrf[9] * sptr[2];
					sum[2] += kptrf[10] * sptr[2];
					sum[3] += kptrf[11] * sptr[2];

					sum[0] += kptrf[12] * sptr[3];
					sum[1] += kptrf[13] * sptr[3];
					sum[2] += kptrf[14] * sptr[3];
					sum[3] += kptrf[15] * sptr[3];
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
			float kptrf[4];
			float* outptr = out_mem + p * out_cstep;

			int i = 0;
			int nn = size & 0xFFFFFFF8;
			for (; i < nn; i++)
			{
				float sum0 = (bias) ? _gnu_h2f_internal_f(bias[p]) : 0.0f;
				float sum1 = 0.0f;
				const unsigned short* kptr = kernel + (p / 4 + p % 4) * 16 * inch;
				for (int q = 0; q < inch; q++)
				{
					_gnu_h2f_internal_vector(kptr, kptrf, 4);
					const float* sptr = in_mem + q * cstep + i * 4;

					sum0 += kptrf[0] * sptr[0];
					sum1 += kptrf[1] * sptr[1];
					sum0 += kptrf[2] * sptr[2];
					sum1 += kptrf[3] * sptr[3];
					kptr += 4;
				}
				outptr[i] = sum0 + sum1;
			}

			nn = size & 0xFFFFFFFC;
			for (; i < nn; i++)
			{
				float sum0 = (bias) ? _gnu_h2f_internal_f(bias[p]) : 0.0f;
				float sum1 = 0.0f;
				float sum2 = 0.0f;
				float sum3 = 0.0f;
				const unsigned short* kptr = kernel + (p / 4 + p % 4) * 16 * inch;
				for (int q = 0; q < inch; q++)
				{
					_gnu_h2f_internal_vector(kptr, kptrf, 4);
					const float* sptr = in_mem + q * cstep + i * 4;

					sum0 += kptrf[0] * sptr[0];
					sum1 += kptrf[1] * sptr[1];
					sum2 += kptrf[2] * sptr[2];
					sum3 += kptrf[3] * sptr[3];
					kptr += 4;
				}
				outptr[i] = ((sum0 + sum1) + (sum2 + sum3));
			}

			for (; i < size; i++)
			{
				float sum0 = (bias) ? _gnu_h2f_internal_f(bias[p]) : 0.0f;
				float sum1 = 0.0f;
				float sum2 = 0.0f;
				float sum3 = 0.0f;
				const unsigned short* kptr = kernel + (p / 4 + p % 4) * 16 * inch;
				for (int q = 0; q < inch; q++)
				{
					_gnu_h2f_internal_vector(kptr, kptrf, 4);
					const float* sptr = in_mem + q * cstep + i * 4;

					sum0 += kptrf[0] * sptr[0];
					sum1 += kptrf[1] * sptr[1];
					sum2 += kptrf[2] * sptr[2];
					sum3 += kptrf[3] * sptr[3];
					kptr += 4;
				}
				outptr[i] = ((sum0 + sum2) + (sum1 + sum3));
			}
		}
#endif
	}

    void enn_conv1x1s1_sgemm_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias, float* tmp)
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

				int q;
				for (q = 0; q < inch; q++)
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

				int q;
				for (q = 0; q < inch; q++)
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

				int q;
				for (q = 0; q < inch; q++)
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

				int q;
				for (q = 0; q < inch; q++)
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

			const unsigned short zeros[4] = { 0, 0, 0, 0 };
			const unsigned short* biasptr = bias ? bias + p * 4 : zeros;

			int i = 0;
			for (; i + 7 < size; i += 8)
			{
				float* tmpptr = tmp + (i / 8) * inch * 32; // .channel(i / 8);
				const unsigned short* kptr0 = kernel + p * 16 * inch; // (const float*)kernel.channel(p);
				int nn = inch;// inch always > 0
				asm volatile(
					// "vld1.f32   {d0-d1}, [%8]   \n"
					"vld1.u16   {d0}, [%8]		\n"
					"vcvt.f32.f16 q0, d0		\n"
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
					// "vldm       %3!, {d8-d15}   \n"
					"vldm       %3!, {d8-d11}   \n"
					"vcvt.f32.f16 q7, d11		\n"
					"vcvt.f32.f16 q6, d10		\n"
					"vcvt.f32.f16 q5, d9		\n"
					"vcvt.f32.f16 q4, d8		\n"

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
				const unsigned short* kptr0 = kernel + p * inch * 16; // (const float*)kernel.channel(p);
				int nn = inch;// inch always > 0

				asm volatile(
					// "vld1.f32   {d0-d1}, [%8]   \n"
					"vld1.u16   {d0}, [%8]		\n"
					"vcvt.f32.f16 q0, d0		\n"
					"vmov       q8, q0          \n"
					"vmov       q9, q0          \n"
					"vmov       q10, q0         \n"
					"vmov       q11, q0         \n"

					"0:                         \n"

					"pld        [%2, #512]      \n"
					"vldm       %2!, {d0-d7}    \n"

					"pld        [%3, #512]      \n"
					// "vldm       %3!, {d8-d15}   \n"
					"vldm       %3!, {d8-d11}   \n"
					"vcvt.f32.f16 q7, d11		\n"
					"vcvt.f32.f16 q6, d10		\n"
					"vcvt.f32.f16 q5, d9		\n"
					"vcvt.f32.f16 q4, d8		\n"

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
				const unsigned short* kptr0 = kernel + p * inch * 16; // (const float*)kernel.channel(p);

				int nn = inch;// inch always > 0

				asm volatile(
					// "vld1.f32   {d0-d1}, [%8]   \n"
					"vld1.u16   {d0}, [%8]		\n"
					"vcvt.f32.f16 q0, d0		\n"
					"vmov       q8, q0          \n"
					"vmov       q9, q0          \n"

					"0:                         \n"

					"pld        [%2, #256]      \n"
					"vld1.f32   {d0-d3}, [%2 :128]! \n"

					"pld        [%3, #512]      \n"
					// "vldm       %3!, {d8-d15}   \n"
					"vldm       %3!, {d8-d11}   \n"
					"vcvt.f32.f16 q7, d11		\n"
					"vcvt.f32.f16 q6, d10		\n"
					"vcvt.f32.f16 q5, d9		\n"
					"vcvt.f32.f16 q4, d8		\n"

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
				const unsigned short* kptr0 = kernel + p * inch * 16; // (const float*)kernel.channel(p);
				int nn = inch;// inch always > 0

				asm volatile(
					// "vld1.f32   {d16-d17}, [%8] \n"
					"vld1.u16   {d16}, [%8]		\n"
					"vcvt.f32.f16 q8, d16		\n"

					"0:                         \n"

					"pld        [%2, #128]      \n"
					"vld1.f32   {d0-d1}, [%2 :128]! \n"

					"pld        [%3, #512]      \n"
					// "vldm       %3!, {d8-d15}   \n"
					"vldm       %3!, {d8-d11}   \n"
					"vcvt.f32.f16 q7, d11		\n"
					"vcvt.f32.f16 q6, d10		\n"
					"vcvt.f32.f16 q5, d9		\n"
					"vcvt.f32.f16 q4, d8		\n"

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
			float kptrf[16];

			for (int i = 0; i < size; i++)
			{
				float sum[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

				if (bias)
				{
					sum[0] = _gnu_h2f_internal_f(bias[p * 4]);
					sum[1] = _gnu_h2f_internal_f(bias[p * 4 + 1]);
					sum[2] = _gnu_h2f_internal_f(bias[p * 4 + 2]);
					sum[3] = _gnu_h2f_internal_f(bias[p * 4 + 3]);
				}

				const unsigned short* kptr = kernel + inch * p * 16;

				for (int q = 0; q < inch; q++)
				{
					_gnu_h2f_internal_vector(kptr, kptrf, 16);
					const float* sptr = in_mem + q * cstep + i * 4;

					sum[0] += kptrf[0] * sptr[0];
					sum[1] += kptrf[1] * sptr[0];
					sum[2] += kptrf[2] * sptr[0];
					sum[3] += kptrf[3] * sptr[0];

					sum[0] += kptrf[4] * sptr[1];
					sum[1] += kptrf[5] * sptr[1];
					sum[2] += kptrf[6] * sptr[1];
					sum[3] += kptrf[7] * sptr[1];

					sum[0] += kptrf[8] * sptr[2];
					sum[1] += kptrf[9] * sptr[2];
					sum[2] += kptrf[10] * sptr[2];
					sum[3] += kptrf[11] * sptr[2];

					sum[0] += kptrf[12] * sptr[3];
					sum[1] += kptrf[13] * sptr[3];
					sum[2] += kptrf[14] * sptr[3];
					sum[3] += kptrf[15] * sptr[3];

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

    void enn_conv3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias)
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

			// float32x4_t _bias0 = bias ? vld1q_f32((const float*)bias + p * 4) : vdupq_n_f32(0.f);
			float32x4_t _bias0 = bias ? vcvt_f32_f16((float16x4_t)vld1_u16((const unsigned short*)bias + p * 4)) : vdupq_n_f32(0.f);
			// out0.fill(_bias0);
			{
				float* ptr = out0;
				int t;
				for (t = 0; t < out_size; t++)
				{
					vst1q_f32(ptr, _bias0);
					ptr += 4;
				}
			}

			int q;
			for (q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0 + 0 * w; //img0.row(0);
				const float* r1 = img0 + 4 * w; //img0.row(1);
				const float* r2 = img0 + 8 * w; //img0.row(2);
												// 				const float* r3 = img0 + 12 * w; //img0.row(3);
												// 				const float* r4 = img0 + 16 * w; //img0.row(4);

				const unsigned short* kptr = kernel + p * kernel_cstep + q * 144; // (const float*)kernel.channel(p).row(q);

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4, {d16-d23}       \n"
							"vldm       %4, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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

							// "sub        %4, %4, #512       \n"// kptr -= 8 * 16;
							"sub        %4, %4, #256       \n"// kptr -= 8 * 16;

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmul.f32   q14, q8, d0[0]      \n"
							"vmul.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmla.f32   q14, q8, d0[0]      \n"
							"vmla.f32   q15, q8, d2[0]      \n"
							"vmla.f32   q12, q9, d0[1]      \n"
							"vmla.f32   q13, q9, d2[1]      \n"
							"vmla.f32   q14, q10, d1[0]     \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"
							"vmla.f32   q13, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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
							// "vldm       %4, {d16-d23}       \n"
							"vldm       %4, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

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

							// "sub        %4, %4, #512        \n"// kptr -= 8 * 16;
							"sub        %4, %4, #256        \n"// kptr -= 8 * 16;

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
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmul.f32   q13, q8, d0[0]      \n"
							"vmul.f32   q14, q9, d0[1]      \n"
							"vmul.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"pld        [%1, #512]          \n"
							"vldm       %1, {d2-d5}         \n"// r01 r02

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%2, #128]          \n"
							"vld1.f32   {d0-d1}, [%2 :128]! \n"// r10

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"pld        [%2, #512]          \n"
							"vldm       %2, {d2-d5}         \n"// r11 r12

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"pld        [%3, #128]          \n"
							"vld1.f32   {d0-d1}, [%3 :128]! \n"// r20

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmla.f32   q13, q8, d0[0]      \n"
							"vmla.f32   q14, q9, d0[1]      \n"
							"vmla.f32   q15, q10, d1[0]     \n"
							"vmla.f32   q12, q11, d1[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4!, {d16-d23}      \n"
							"vldm       %4!, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"pld        [%3, #512]          \n"
							"vldm       %3, {d2-d5}         \n"// r21 r22

							"vmla.f32   q13, q8, d2[0]      \n"
							"vmla.f32   q14, q9, d2[1]      \n"
							"vmla.f32   q15, q10, d3[0]     \n"
							"vmla.f32   q12, q11, d3[1]     \n"

							"pld        [%4, #512]          \n"
							// "vldm       %4, {d16-d23}      \n"
							"vldm       %4, {d16-d19}      \n"
							"vcvt.f32.f16 q11, d19			\n"
							"vcvt.f32.f16 q10, d18			\n"
							"vcvt.f32.f16 q9, d17			\n"
							"vcvt.f32.f16 q8, d16			\n"

							"vmla.f32   q13, q8, d4[0]      \n"
							"vmla.f32   q14, q9, d4[1]      \n"
							"vmla.f32   q15, q10, d5[0]     \n"
							"vmla.f32   q12, q11, d5[1]     \n"

							"vadd.f32   q13, q13, q14       \n"
							"vadd.f32   q12, q12, q15       \n"
							"vadd.f32   q12, q12, q13       \n"

							// "sub        %4, %4, #512       \n"// kptr -= 8 * 16;
							"sub        %4, %4, #256       \n"// kptr -= 8 * 16;

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
			float* out0 = out_mem + p * out_cstep; // top_blob.channel(p);
			
			if (bias)
			{
				float _bias[4];
				_gnu_h2f_internal_vector(bias + p * 4, _bias, 4);
				float* ptr = out0;
				int t;
				for (t = 0; t < out_size; t++)
				{
					memcpy(ptr, _bias, 16);
					ptr += 4;
				}
			}
			else
			{
				memset(out0, 0, out_size * 16);
			}

			int q;
			for (q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0 + 0 * w; //img0.row(0);
				const float* r1 = img0 + 4 * w; //img0.row(1);
				const float* r2 = img0 + 8 * w; //img0.row(2);
												// 				const float* r3 = img0 + 12 * w; //img0.row(3);
												// 				const float* r4 = img0 + 16 * w; //img0.row(4);

				const unsigned short* kptr = kernel + p * kernel_cstep + q * 144; // (const float*)kernel.channel(p).row(q);
				float _kptr[144];
				float* _k0 = _kptr;
				_gnu_h2f_internal_vector(kptr, _k0, 144);

				int i = 0;
				for (; i < outh; i++)
				{
					int j = 0;
					for (; j + 3 < outw; j += 4)
					{
						float sum00, sum01, sum02, sum03, sum10, sum11, sum12, sum13, sum20, sum21, sum22, sum23, sum30, sum31, sum32, sum33;

						sum00 = outptr0[0] + r0[0] * _k0[0] + r0[1] * _k0[4] + r0[2] * _k0[8 ] + r0[3] * _k0[12];
						sum01 = outptr0[1] + r0[0] * _k0[1] + r0[1] * _k0[5] + r0[2] * _k0[9 ] + r0[3] * _k0[13];
						sum02 = outptr0[2] + r0[0] * _k0[2] + r0[1] * _k0[6] + r0[2] * _k0[10] + r0[3] * _k0[14];
						sum03 = outptr0[3] + r0[0] * _k0[3] + r0[1] * _k0[7] + r0[2] * _k0[11] + r0[3] * _k0[15];

						sum10 = outptr0[4] + r0[4] * _k0[0] + r0[5] * _k0[4] + r0[6] * _k0[8 ] + r0[7] * _k0[12];
						sum11 = outptr0[5] + r0[4] * _k0[1] + r0[5] * _k0[5] + r0[6] * _k0[9 ] + r0[7] * _k0[13];
						sum12 = outptr0[6] + r0[4] * _k0[2] + r0[5] * _k0[6] + r0[6] * _k0[10] + r0[7] * _k0[14];
						sum13 = outptr0[7] + r0[4] * _k0[3] + r0[5] * _k0[7] + r0[6] * _k0[11] + r0[7] * _k0[15];

						sum20 = outptr0[8 ] + r0[8] * _k0[0] + r0[9] * _k0[4] + r0[10] * _k0[8 ] + r0[11] * _k0[12];
						sum21 = outptr0[9 ] + r0[8] * _k0[1] + r0[9] * _k0[5] + r0[10] * _k0[9 ] + r0[11] * _k0[13];
						sum22 = outptr0[10] + r0[8] * _k0[2] + r0[9] * _k0[6] + r0[10] * _k0[10] + r0[11] * _k0[14];
						sum23 = outptr0[11] + r0[8] * _k0[3] + r0[9] * _k0[7] + r0[10] * _k0[11] + r0[11] * _k0[15];

						sum30 = outptr0[12] + r0[12] * _k0[0] + r0[13] * _k0[4] + r0[14] * _k0[8 ] + r0[15] * _k0[12];
						sum31 = outptr0[13] + r0[12] * _k0[1] + r0[13] * _k0[5] + r0[14] * _k0[9 ] + r0[15] * _k0[13];
						sum32 = outptr0[14] + r0[12] * _k0[2] + r0[13] * _k0[6] + r0[14] * _k0[10] + r0[15] * _k0[14];
						sum33 = outptr0[15] + r0[12] * _k0[3] + r0[13] * _k0[7] + r0[14] * _k0[11] + r0[15] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r0[4] * _k0[0] + r0[5] * _k0[4] + r0[6] * _k0[ 8] + r0[7] * _k0[12];
						sum01 = sum01 + r0[4] * _k0[1] + r0[5] * _k0[5] + r0[6] * _k0[ 9] + r0[7] * _k0[13];
						sum02 = sum02 + r0[4] * _k0[2] + r0[5] * _k0[6] + r0[6] * _k0[10] + r0[7] * _k0[14];
						sum03 = sum03 + r0[4] * _k0[3] + r0[5] * _k0[7] + r0[6] * _k0[11] + r0[7] * _k0[15];

						sum10 = sum10 + r0[8] * _k0[0] + r0[9] * _k0[4] + r0[10] * _k0[ 8] + r0[11] * _k0[12];
						sum11 = sum11 + r0[8] * _k0[1] + r0[9] * _k0[5] + r0[10] * _k0[ 9] + r0[11] * _k0[13];
						sum12 = sum12 + r0[8] * _k0[2] + r0[9] * _k0[6] + r0[10] * _k0[10] + r0[11] * _k0[14];
						sum13 = sum13 + r0[8] * _k0[3] + r0[9] * _k0[7] + r0[10] * _k0[11] + r0[11] * _k0[15];

						sum20 = sum20 + r0[12] * _k0[0] + r0[13] * _k0[4] + r0[14] * _k0[ 8] + r0[15] * _k0[12];
						sum21 = sum21 + r0[12] * _k0[1] + r0[13] * _k0[5] + r0[14] * _k0[ 9] + r0[15] * _k0[13];
						sum22 = sum22 + r0[12] * _k0[2] + r0[13] * _k0[6] + r0[14] * _k0[10] + r0[15] * _k0[14];
						sum23 = sum23 + r0[12] * _k0[3] + r0[13] * _k0[7] + r0[14] * _k0[11] + r0[15] * _k0[15];

						sum30 = sum30 + r0[16] * _k0[0] + r0[17] * _k0[4] + r0[18] * _k0[ 8] + r0[19] * _k0[12];
						sum31 = sum31 + r0[16] * _k0[1] + r0[17] * _k0[5] + r0[18] * _k0[ 9] + r0[19] * _k0[13];
						sum32 = sum32 + r0[16] * _k0[2] + r0[17] * _k0[6] + r0[18] * _k0[10] + r0[19] * _k0[14];
						sum33 = sum33 + r0[16] * _k0[3] + r0[17] * _k0[7] + r0[18] * _k0[11] + r0[19] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r0[8] * _k0[0] + r0[9] * _k0[4] + r0[10] * _k0[8 ] + r0[11] * _k0[12];
						sum01 = sum01 + r0[8] * _k0[1] + r0[9] * _k0[5] + r0[10] * _k0[9 ] + r0[11] * _k0[13];
						sum02 = sum02 + r0[8] * _k0[2] + r0[9] * _k0[6] + r0[10] * _k0[10] + r0[11] * _k0[14];
						sum03 = sum03 + r0[8] * _k0[3] + r0[9] * _k0[7] + r0[10] * _k0[11] + r0[11] * _k0[15];

						sum10 = sum10 + r0[12] * _k0[0] + r0[13] * _k0[4] + r0[14] * _k0[8 ] + r0[15] * _k0[12];
						sum11 = sum11 + r0[12] * _k0[1] + r0[13] * _k0[5] + r0[14] * _k0[9 ] + r0[15] * _k0[13];
						sum12 = sum12 + r0[12] * _k0[2] + r0[13] * _k0[6] + r0[14] * _k0[10] + r0[15] * _k0[14];
						sum13 = sum13 + r0[12] * _k0[3] + r0[13] * _k0[7] + r0[14] * _k0[11] + r0[15] * _k0[15];

						sum20 = sum20 + r0[16] * _k0[0] + r0[17] * _k0[4] + r0[18] * _k0[8 ] + r0[19] * _k0[12];
						sum21 = sum21 + r0[16] * _k0[1] + r0[17] * _k0[5] + r0[18] * _k0[9 ] + r0[19] * _k0[13];
						sum22 = sum22 + r0[16] * _k0[2] + r0[17] * _k0[6] + r0[18] * _k0[10] + r0[19] * _k0[14];
						sum23 = sum23 + r0[16] * _k0[3] + r0[17] * _k0[7] + r0[18] * _k0[11] + r0[19] * _k0[15];

						sum30 = sum30 + r0[20] * _k0[0] + r0[21] * _k0[4] + r0[22] * _k0[8 ] + r0[23] * _k0[12];
						sum31 = sum31 + r0[20] * _k0[1] + r0[21] * _k0[5] + r0[22] * _k0[9 ] + r0[23] * _k0[13];
						sum32 = sum32 + r0[20] * _k0[2] + r0[21] * _k0[6] + r0[22] * _k0[10] + r0[23] * _k0[14];
						sum33 = sum33 + r0[20] * _k0[3] + r0[21] * _k0[7] + r0[22] * _k0[11] + r0[23] * _k0[15];
						_k0 += 16;


						sum00 = sum00 + r1[0] * _k0[0] + r1[1] * _k0[4] + r1[2] * _k0[8 ] + r1[3] * _k0[12];
						sum01 = sum01 + r1[0] * _k0[1] + r1[1] * _k0[5] + r1[2] * _k0[9 ] + r1[3] * _k0[13];
						sum02 = sum02 + r1[0] * _k0[2] + r1[1] * _k0[6] + r1[2] * _k0[10] + r1[3] * _k0[14];
						sum03 = sum03 + r1[0] * _k0[3] + r1[1] * _k0[7] + r1[2] * _k0[11] + r1[3] * _k0[15];

						sum10 = sum10 + r1[4] * _k0[0] + r1[5] * _k0[4] + r1[6] * _k0[8 ] + r1[7] * _k0[12];
						sum11 = sum11 + r1[4] * _k0[1] + r1[5] * _k0[5] + r1[6] * _k0[9 ] + r1[7] * _k0[13];
						sum12 = sum12 + r1[4] * _k0[2] + r1[5] * _k0[6] + r1[6] * _k0[10] + r1[7] * _k0[14];
						sum13 = sum13 + r1[4] * _k0[3] + r1[5] * _k0[7] + r1[6] * _k0[11] + r1[7] * _k0[15];

						sum20 = sum20 + r1[8] * _k0[0] + r1[9] * _k0[4] + r1[10] * _k0[8 ] + r1[11] * _k0[12];
						sum21 = sum21 + r1[8] * _k0[1] + r1[9] * _k0[5] + r1[10] * _k0[9 ] + r1[11] * _k0[13];
						sum22 = sum22 + r1[8] * _k0[2] + r1[9] * _k0[6] + r1[10] * _k0[10] + r1[11] * _k0[14];
						sum23 = sum23 + r1[8] * _k0[3] + r1[9] * _k0[7] + r1[10] * _k0[11] + r1[11] * _k0[15];

						sum30 = sum30 + r1[12] * _k0[0] + r1[13] * _k0[4] + r1[14] * _k0[8 ] + r1[15] * _k0[12];
						sum31 = sum31 + r1[12] * _k0[1] + r1[13] * _k0[5] + r1[14] * _k0[9 ] + r1[15] * _k0[13];
						sum32 = sum32 + r1[12] * _k0[2] + r1[13] * _k0[6] + r1[14] * _k0[10] + r1[15] * _k0[14];
						sum33 = sum33 + r1[12] * _k0[3] + r1[13] * _k0[7] + r1[14] * _k0[11] + r1[15] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r1[4] * _k0[0] + r1[5] * _k0[4] + r1[6] * _k0[8 ] + r1[7] * _k0[12];
						sum01 = sum01 + r1[4] * _k0[1] + r1[5] * _k0[5] + r1[6] * _k0[9 ] + r1[7] * _k0[13];
						sum02 = sum02 + r1[4] * _k0[2] + r1[5] * _k0[6] + r1[6] * _k0[10] + r1[7] * _k0[14];
						sum03 = sum03 + r1[4] * _k0[3] + r1[5] * _k0[7] + r1[6] * _k0[11] + r1[7] * _k0[15];

						sum10 = sum10 + r1[8] * _k0[0] + r1[9] * _k0[4] + r1[10] * _k0[8 ] + r1[11] * _k0[12];
						sum11 = sum11 + r1[8] * _k0[1] + r1[9] * _k0[5] + r1[10] * _k0[9 ] + r1[11] * _k0[13];
						sum12 = sum12 + r1[8] * _k0[2] + r1[9] * _k0[6] + r1[10] * _k0[10] + r1[11] * _k0[14];
						sum13 = sum13 + r1[8] * _k0[3] + r1[9] * _k0[7] + r1[10] * _k0[11] + r1[11] * _k0[15];

						sum20 = sum20 + r1[12] * _k0[0] + r1[13] * _k0[4] + r1[14] * _k0[8 ] + r1[15] * _k0[12];
						sum21 = sum21 + r1[12] * _k0[1] + r1[13] * _k0[5] + r1[14] * _k0[9 ] + r1[15] * _k0[13];
						sum22 = sum22 + r1[12] * _k0[2] + r1[13] * _k0[6] + r1[14] * _k0[10] + r1[15] * _k0[14];
						sum23 = sum23 + r1[12] * _k0[3] + r1[13] * _k0[7] + r1[14] * _k0[11] + r1[15] * _k0[15];

						sum30 = sum30 + r1[16] * _k0[0] + r1[17] * _k0[4] + r1[18] * _k0[8 ] + r1[19] * _k0[12];
						sum31 = sum31 + r1[16] * _k0[1] + r1[17] * _k0[5] + r1[18] * _k0[9 ] + r1[19] * _k0[13];
						sum32 = sum32 + r1[16] * _k0[2] + r1[17] * _k0[6] + r1[18] * _k0[10] + r1[19] * _k0[14];
						sum33 = sum33 + r1[16] * _k0[3] + r1[17] * _k0[7] + r1[18] * _k0[11] + r1[19] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r1[8] * _k0[0] + r1[9] * _k0[4] + r1[10] * _k0[8 ] + r1[11] * _k0[12];
						sum01 = sum01 + r1[8] * _k0[1] + r1[9] * _k0[5] + r1[10] * _k0[9 ] + r1[11] * _k0[13];
						sum02 = sum02 + r1[8] * _k0[2] + r1[9] * _k0[6] + r1[10] * _k0[10] + r1[11] * _k0[14];
						sum03 = sum03 + r1[8] * _k0[3] + r1[9] * _k0[7] + r1[10] * _k0[11] + r1[11] * _k0[15];

						sum10 = sum10 + r1[12] * _k0[0] + r1[13] * _k0[4] + r1[14] * _k0[8 ] + r1[15] * _k0[12];
						sum11 = sum11 + r1[12] * _k0[1] + r1[13] * _k0[5] + r1[14] * _k0[9 ] + r1[15] * _k0[13];
						sum12 = sum12 + r1[12] * _k0[2] + r1[13] * _k0[6] + r1[14] * _k0[10] + r1[15] * _k0[14];
						sum13 = sum13 + r1[12] * _k0[3] + r1[13] * _k0[7] + r1[14] * _k0[11] + r1[15] * _k0[15];

						sum20 = sum20 + r1[16] * _k0[0] + r1[17] * _k0[4] + r1[18] * _k0[8 ] + r1[19] * _k0[12];
						sum21 = sum21 + r1[16] * _k0[1] + r1[17] * _k0[5] + r1[18] * _k0[9 ] + r1[19] * _k0[13];
						sum22 = sum22 + r1[16] * _k0[2] + r1[17] * _k0[6] + r1[18] * _k0[10] + r1[19] * _k0[14];
						sum23 = sum23 + r1[16] * _k0[3] + r1[17] * _k0[7] + r1[18] * _k0[11] + r1[19] * _k0[15];

						sum30 = sum30 + r1[20] * _k0[0] + r1[21] * _k0[4] + r1[22] * _k0[8 ] + r1[23] * _k0[12];
						sum31 = sum31 + r1[20] * _k0[1] + r1[21] * _k0[5] + r1[22] * _k0[9 ] + r1[23] * _k0[13];
						sum32 = sum32 + r1[20] * _k0[2] + r1[21] * _k0[6] + r1[22] * _k0[10] + r1[23] * _k0[14];
						sum33 = sum33 + r1[20] * _k0[3] + r1[21] * _k0[7] + r1[22] * _k0[11] + r1[23] * _k0[15];
						_k0 += 16;


						sum00 = sum00 + r2[0] * _k0[0] + r2[1] * _k0[4] + r2[2] * _k0[8 ] + r2[3] * _k0[12];
						sum01 = sum01 + r2[0] * _k0[1] + r2[1] * _k0[5] + r2[2] * _k0[9 ] + r2[3] * _k0[13];
						sum02 = sum02 + r2[0] * _k0[2] + r2[1] * _k0[6] + r2[2] * _k0[10] + r2[3] * _k0[14];
						sum03 = sum03 + r2[0] * _k0[3] + r2[1] * _k0[7] + r2[2] * _k0[11] + r2[3] * _k0[15];

						sum10 = sum10 + r2[4] * _k0[0] + r2[5] * _k0[4] + r2[6] * _k0[8 ] + r2[7] * _k0[12];
						sum11 = sum11 + r2[4] * _k0[1] + r2[5] * _k0[5] + r2[6] * _k0[9 ] + r2[7] * _k0[13];
						sum12 = sum12 + r2[4] * _k0[2] + r2[5] * _k0[6] + r2[6] * _k0[10] + r2[7] * _k0[14];
						sum13 = sum13 + r2[4] * _k0[3] + r2[5] * _k0[7] + r2[6] * _k0[11] + r2[7] * _k0[15];

						sum20 = sum20 + r2[8] * _k0[0] + r2[9] * _k0[4] + r2[10] * _k0[8 ] + r2[11] * _k0[12];
						sum21 = sum21 + r2[8] * _k0[1] + r2[9] * _k0[5] + r2[10] * _k0[9 ] + r2[11] * _k0[13];
						sum22 = sum22 + r2[8] * _k0[2] + r2[9] * _k0[6] + r2[10] * _k0[10] + r2[11] * _k0[14];
						sum23 = sum23 + r2[8] * _k0[3] + r2[9] * _k0[7] + r2[10] * _k0[11] + r2[11] * _k0[15];

						sum30 = sum30 + r2[12] * _k0[0] + r2[13] * _k0[4] + r2[14] * _k0[8 ] + r2[15] * _k0[12];
						sum31 = sum31 + r2[12] * _k0[1] + r2[13] * _k0[5] + r2[14] * _k0[9 ] + r2[15] * _k0[13];
						sum32 = sum32 + r2[12] * _k0[2] + r2[13] * _k0[6] + r2[14] * _k0[10] + r2[15] * _k0[14];
						sum33 = sum33 + r2[12] * _k0[3] + r2[13] * _k0[7] + r2[14] * _k0[11] + r2[15] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r2[4] * _k0[0] + r2[5] * _k0[4] + r2[6] * _k0[8 ] + r2[7] * _k0[12];
						sum01 = sum01 + r2[4] * _k0[1] + r2[5] * _k0[5] + r2[6] * _k0[9 ] + r2[7] * _k0[13];
						sum02 = sum02 + r2[4] * _k0[2] + r2[5] * _k0[6] + r2[6] * _k0[10] + r2[7] * _k0[14];
						sum03 = sum03 + r2[4] * _k0[3] + r2[5] * _k0[7] + r2[6] * _k0[11] + r2[7] * _k0[15];

						sum10 = sum10 + r2[8] * _k0[0] + r2[9] * _k0[4] + r2[10] * _k0[8 ] + r2[11] * _k0[12];
						sum11 = sum11 + r2[8] * _k0[1] + r2[9] * _k0[5] + r2[10] * _k0[9 ] + r2[11] * _k0[13];
						sum12 = sum12 + r2[8] * _k0[2] + r2[9] * _k0[6] + r2[10] * _k0[10] + r2[11] * _k0[14];
						sum13 = sum13 + r2[8] * _k0[3] + r2[9] * _k0[7] + r2[10] * _k0[11] + r2[11] * _k0[15];

						sum20 = sum20 + r2[12] * _k0[0] + r2[13] * _k0[4] + r2[14] * _k0[8 ] + r2[15] * _k0[12];
						sum21 = sum21 + r2[12] * _k0[1] + r2[13] * _k0[5] + r2[14] * _k0[9 ] + r2[15] * _k0[13];
						sum22 = sum22 + r2[12] * _k0[2] + r2[13] * _k0[6] + r2[14] * _k0[10] + r2[15] * _k0[14];
						sum23 = sum23 + r2[12] * _k0[3] + r2[13] * _k0[7] + r2[14] * _k0[11] + r2[15] * _k0[15];

						sum30 = sum30 + r2[16] * _k0[0] + r2[17] * _k0[4] + r2[18] * _k0[8 ] + r2[19] * _k0[12];
						sum31 = sum31 + r2[16] * _k0[1] + r2[17] * _k0[5] + r2[18] * _k0[9 ] + r2[19] * _k0[13];
						sum32 = sum32 + r2[16] * _k0[2] + r2[17] * _k0[6] + r2[18] * _k0[10] + r2[19] * _k0[14];
						sum33 = sum33 + r2[16] * _k0[3] + r2[17] * _k0[7] + r2[18] * _k0[11] + r2[19] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r2[8] * _k0[0] + r2[9] * _k0[4] + r2[10] * _k0[8 ] + r2[11] * _k0[12];
						sum01 = sum01 + r2[8] * _k0[1] + r2[9] * _k0[5] + r2[10] * _k0[9 ] + r2[11] * _k0[13];
						sum02 = sum02 + r2[8] * _k0[2] + r2[9] * _k0[6] + r2[10] * _k0[10] + r2[11] * _k0[14];
						sum03 = sum03 + r2[8] * _k0[3] + r2[9] * _k0[7] + r2[10] * _k0[11] + r2[11] * _k0[15];

						sum10 = sum10 + r2[12] * _k0[0] + r2[13] * _k0[4] + r2[14] * _k0[8 ] + r2[15] * _k0[12];
						sum11 = sum11 + r2[12] * _k0[1] + r2[13] * _k0[5] + r2[14] * _k0[9 ] + r2[15] * _k0[13];
						sum12 = sum12 + r2[12] * _k0[2] + r2[13] * _k0[6] + r2[14] * _k0[10] + r2[15] * _k0[14];
						sum13 = sum13 + r2[12] * _k0[3] + r2[13] * _k0[7] + r2[14] * _k0[11] + r2[15] * _k0[15];

						sum20 = sum20 + r2[16] * _k0[0] + r2[17] * _k0[4] + r2[18] * _k0[8 ] + r2[19] * _k0[12];
						sum21 = sum21 + r2[16] * _k0[1] + r2[17] * _k0[5] + r2[18] * _k0[9 ] + r2[19] * _k0[13];
						sum22 = sum22 + r2[16] * _k0[2] + r2[17] * _k0[6] + r2[18] * _k0[10] + r2[19] * _k0[14];
						sum23 = sum23 + r2[16] * _k0[3] + r2[17] * _k0[7] + r2[18] * _k0[11] + r2[19] * _k0[15];

						sum30 = sum30 + r2[20] * _k0[0] + r2[21] * _k0[4] + r2[22] * _k0[8 ] + r2[23] * _k0[12];
						sum31 = sum31 + r2[20] * _k0[1] + r2[21] * _k0[5] + r2[22] * _k0[9 ] + r2[23] * _k0[13];
						sum32 = sum32 + r2[20] * _k0[2] + r2[21] * _k0[6] + r2[22] * _k0[10] + r2[23] * _k0[14];
						sum33 = sum33 + r2[20] * _k0[3] + r2[21] * _k0[7] + r2[22] * _k0[11] + r2[23] * _k0[15];
						_k0 -= 128;

						r0 += 16;
						r1 += 16;
						r2 += 16;

						outptr0[0] = sum00;
						outptr0[1] = sum01;
						outptr0[2] = sum02;
						outptr0[3] = sum03;

						outptr0[4] = sum10;
						outptr0[5] = sum11;
						outptr0[6] = sum12;
						outptr0[7] = sum13;

						outptr0[8 ] = sum20;
						outptr0[9 ] = sum21;
						outptr0[10] = sum22;
						outptr0[11] = sum23;

						outptr0[12] = sum30;
						outptr0[13] = sum31;
						outptr0[14] = sum32;
						outptr0[15] = sum33;

						outptr0 += 16;
					}
					for (; j + 1 < outw; j += 2)
					{
						float sum00, sum01, sum02, sum03, sum10, sum11, sum12, sum13, sum20, sum21, sum22, sum23, sum30, sum31, sum32, sum33;

						sum00 = r0[0] * _k0[0] + r0[2] * _k0[8 ];
						sum01 = r0[0] * _k0[1] + r0[2] * _k0[9 ];
						sum02 = r0[0] * _k0[2] + r0[2] * _k0[10];
						sum03 = r0[0] * _k0[3] + r0[2] * _k0[11];

						sum10 = r0[4] * _k0[0] + r0[6] * _k0[8 ];
						sum11 = r0[4] * _k0[1] + r0[6] * _k0[9 ];
						sum12 = r0[4] * _k0[2] + r0[6] * _k0[10];
						sum13 = r0[4] * _k0[3] + r0[6] * _k0[11];

						sum20 = outptr0[0] + r0[1] * _k0[4] + r0[3] * _k0[12];
						sum21 = outptr0[1] + r0[1] * _k0[5] + r0[3] * _k0[13];
						sum22 = outptr0[2] + r0[1] * _k0[6] + r0[3] * _k0[14];
						sum23 = outptr0[3] + r0[1] * _k0[7] + r0[3] * _k0[15];

						sum30 = outptr0[4] + r0[5] * _k0[4] + r0[7] * _k0[12];
						sum31 = outptr0[5] + r0[5] * _k0[5] + r0[7] * _k0[13];
						sum32 = outptr0[6] + r0[5] * _k0[6] + r0[7] * _k0[14];
						sum33 = outptr0[7] + r0[5] * _k0[7] + r0[7] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r0[4] * _k0[0] + r0[6] * _k0[8];
						sum01 = sum01 + r0[4] * _k0[1] + r0[6] * _k0[9];
						sum02 = sum02 + r0[4] * _k0[2] + r0[6] * _k0[10];
						sum03 = sum03 + r0[4] * _k0[3] + r0[6] * _k0[11];

						sum10 = sum10 + r0[8] * _k0[0] + r0[10] * _k0[8];
						sum11 = sum11 + r0[8] * _k0[1] + r0[10] * _k0[9];
						sum12 = sum12 + r0[8] * _k0[2] + r0[10] * _k0[10];
						sum13 = sum13 + r0[8] * _k0[3] + r0[10] * _k0[11];

						sum20 = sum20 + r0[5] * _k0[4] + r0[7] * _k0[12];
						sum21 = sum21 + r0[5] * _k0[5] + r0[7] * _k0[13];
						sum22 = sum22 + r0[5] * _k0[6] + r0[7] * _k0[14];
						sum23 = sum23 + r0[5] * _k0[7] + r0[7] * _k0[15];

						sum30 = sum30 + r0[9] * _k0[4] + r0[11] * _k0[12];
						sum31 = sum31 + r0[9] * _k0[5] + r0[11] * _k0[13];
						sum32 = sum32 + r0[9] * _k0[6] + r0[11] * _k0[14];
						sum33 = sum33 + r0[9] * _k0[7] + r0[11] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r0[8] * _k0[0] + r0[10] * _k0[8];
						sum01 = sum01 + r0[8] * _k0[1] + r0[10] * _k0[9];
						sum02 = sum02 + r0[8] * _k0[2] + r0[10] * _k0[10];
						sum03 = sum03 + r0[8] * _k0[3] + r0[10] * _k0[11];

						sum10 = sum10 + r0[12] * _k0[0] + r0[14] * _k0[8];
						sum11 = sum11 + r0[12] * _k0[1] + r0[14] * _k0[9];
						sum12 = sum12 + r0[12] * _k0[2] + r0[14] * _k0[10];
						sum13 = sum13 + r0[12] * _k0[3] + r0[14] * _k0[11];

						sum20 = sum20 + r0[9] * _k0[4] + r0[11] * _k0[12];
						sum21 = sum21 + r0[9] * _k0[5] + r0[11] * _k0[13];
						sum22 = sum22 + r0[9] * _k0[6] + r0[11] * _k0[14];
						sum23 = sum23 + r0[9] * _k0[7] + r0[11] * _k0[15];

						sum30 = sum30 + r0[13] * _k0[4] + r0[15] * _k0[12];
						sum31 = sum31 + r0[13] * _k0[5] + r0[15] * _k0[13];
						sum32 = sum32 + r0[13] * _k0[6] + r0[15] * _k0[14];
						sum33 = sum33 + r0[13] * _k0[7] + r0[15] * _k0[15];
						_k0 += 16;


						sum00 = sum00 + r1[0] * _k0[0] + r1[2] * _k0[8];
						sum01 = sum01 + r1[0] * _k0[1] + r1[2] * _k0[9];
						sum02 = sum02 + r1[0] * _k0[2] + r1[2] * _k0[10];
						sum03 = sum03 + r1[0] * _k0[3] + r1[2] * _k0[11];

						sum10 = sum10 + r1[4] * _k0[0] + r1[6] * _k0[8];
						sum11 = sum11 + r1[4] * _k0[1] + r1[6] * _k0[9];
						sum12 = sum12 + r1[4] * _k0[2] + r1[6] * _k0[10];
						sum13 = sum13 + r1[4] * _k0[3] + r1[6] * _k0[11];

						sum20 = sum20 + r1[1] * _k0[4] + r1[3] * _k0[12];
						sum21 = sum21 + r1[1] * _k0[5] + r1[3] * _k0[13];
						sum22 = sum22 + r1[1] * _k0[6] + r1[3] * _k0[14];
						sum23 = sum23 + r1[1] * _k0[7] + r1[3] * _k0[15];

						sum30 = sum30 + r1[5] * _k0[4] + r1[7] * _k0[12];
						sum31 = sum31 + r1[5] * _k0[5] + r1[7] * _k0[13];
						sum32 = sum32 + r1[5] * _k0[6] + r1[7] * _k0[14];
						sum33 = sum33 + r1[5] * _k0[7] + r1[7] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r1[4] * _k0[0] + r1[6] * _k0[8];
						sum01 = sum01 + r1[4] * _k0[1] + r1[6] * _k0[9];
						sum02 = sum02 + r1[4] * _k0[2] + r1[6] * _k0[10];
						sum03 = sum03 + r1[4] * _k0[3] + r1[6] * _k0[11];

						sum10 = sum10 + r1[8] * _k0[0] + r1[10] * _k0[8];
						sum11 = sum11 + r1[8] * _k0[1] + r1[10] * _k0[9];
						sum12 = sum12 + r1[8] * _k0[2] + r1[10] * _k0[10];
						sum13 = sum13 + r1[8] * _k0[3] + r1[10] * _k0[11];

						sum20 = sum20 + r1[5] * _k0[4] + r1[7] * _k0[12];
						sum21 = sum21 + r1[5] * _k0[5] + r1[7] * _k0[13];
						sum22 = sum22 + r1[5] * _k0[6] + r1[7] * _k0[14];
						sum23 = sum23 + r1[5] * _k0[7] + r1[7] * _k0[15];

						sum30 = sum30 + r1[9] * _k0[4] + r1[11] * _k0[12];
						sum31 = sum31 + r1[9] * _k0[5] + r1[11] * _k0[13];
						sum32 = sum32 + r1[9] * _k0[6] + r1[11] * _k0[14];
						sum33 = sum33 + r1[9] * _k0[7] + r1[11] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r1[8] * _k0[0] + r1[10] * _k0[8];
						sum01 = sum01 + r1[8] * _k0[1] + r1[10] * _k0[9];
						sum02 = sum02 + r1[8] * _k0[2] + r1[10] * _k0[10];
						sum03 = sum03 + r1[8] * _k0[3] + r1[10] * _k0[11];

						sum10 = sum10 + r1[12] * _k0[0] + r1[14] * _k0[8];
						sum11 = sum11 + r1[12] * _k0[1] + r1[14] * _k0[9];
						sum12 = sum12 + r1[12] * _k0[2] + r1[14] * _k0[10];
						sum13 = sum13 + r1[12] * _k0[3] + r1[14] * _k0[11];

						sum20 = sum20 + r1[9] * _k0[4] + r1[11] * _k0[12];
						sum21 = sum21 + r1[9] * _k0[5] + r1[11] * _k0[13];
						sum22 = sum22 + r1[9] * _k0[6] + r1[11] * _k0[14];
						sum23 = sum23 + r1[9] * _k0[7] + r1[11] * _k0[15];

						sum30 = sum30 + r1[13] * _k0[4] + r1[15] * _k0[12];
						sum31 = sum31 + r1[13] * _k0[5] + r1[15] * _k0[13];
						sum32 = sum32 + r1[13] * _k0[6] + r1[15] * _k0[14];
						sum33 = sum33 + r1[13] * _k0[7] + r1[15] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r2[0] * _k0[0] + r2[2] * _k0[8];
						sum01 = sum01 + r2[0] * _k0[1] + r2[2] * _k0[9];
						sum02 = sum02 + r2[0] * _k0[2] + r2[2] * _k0[10];
						sum03 = sum03 + r2[0] * _k0[3] + r2[2] * _k0[11];

						sum10 = sum10 + r2[4] * _k0[0] + r2[6] * _k0[8];
						sum11 = sum11 + r2[4] * _k0[1] + r2[6] * _k0[9];
						sum12 = sum12 + r2[4] * _k0[2] + r2[6] * _k0[10];
						sum13 = sum13 + r2[4] * _k0[3] + r2[6] * _k0[11];

						sum20 = sum20 + r2[1] * _k0[4] + r2[3] * _k0[12];
						sum21 = sum21 + r2[1] * _k0[5] + r2[3] * _k0[13];
						sum22 = sum22 + r2[1] * _k0[6] + r2[3] * _k0[14];
						sum23 = sum23 + r2[1] * _k0[7] + r2[3] * _k0[15];

						sum30 = sum30 + r2[5] * _k0[4] + r2[7] * _k0[12];
						sum31 = sum31 + r2[5] * _k0[5] + r2[7] * _k0[13];
						sum32 = sum32 + r2[5] * _k0[6] + r2[7] * _k0[14];
						sum33 = sum33 + r2[5] * _k0[7] + r2[7] * _k0[15];
						_k0 += 16;


						sum00 = sum00 + r2[4] * _k0[0] + r2[6] * _k0[8];
						sum01 = sum01 + r2[4] * _k0[1] + r2[6] * _k0[9];
						sum02 = sum02 + r2[4] * _k0[2] + r2[6] * _k0[10];
						sum03 = sum03 + r2[4] * _k0[3] + r2[6] * _k0[11];

						sum10 = sum10 + r2[8] * _k0[0] + r2[10] * _k0[8];
						sum11 = sum11 + r2[8] * _k0[1] + r2[10] * _k0[9];
						sum12 = sum12 + r2[8] * _k0[2] + r2[10] * _k0[10];
						sum13 = sum13 + r2[8] * _k0[3] + r2[10] * _k0[11];

						sum20 = sum20 + r2[5] * _k0[4] + r2[7] * _k0[12];
						sum21 = sum21 + r2[5] * _k0[5] + r2[7] * _k0[13];
						sum22 = sum22 + r2[5] * _k0[6] + r2[7] * _k0[14];
						sum23 = sum23 + r2[5] * _k0[7] + r2[7] * _k0[15];

						sum30 = sum30 + r2[9] * _k0[4] + r2[11] * _k0[12];
						sum31 = sum31 + r2[9] * _k0[5] + r2[11] * _k0[13];
						sum32 = sum32 + r2[9] * _k0[6] + r2[11] * _k0[14];
						sum33 = sum33 + r2[9] * _k0[7] + r2[11] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r2[8] * _k0[0] + r2[10] * _k0[8];
						sum01 = sum01 + r2[8] * _k0[1] + r2[10] * _k0[9];
						sum02 = sum02 + r2[8] * _k0[2] + r2[10] * _k0[10];
						sum03 = sum03 + r2[8] * _k0[3] + r2[10] * _k0[11];

						sum10 = sum10 + r2[12] * _k0[0] + r2[14] * _k0[8];
						sum11 = sum11 + r2[12] * _k0[1] + r2[14] * _k0[9];
						sum12 = sum12 + r2[12] * _k0[2] + r2[14] * _k0[10];
						sum13 = sum13 + r2[12] * _k0[3] + r2[14] * _k0[11];

						sum20 = sum20 + r2[9] * _k0[4] + r2[11] * _k0[12];
						sum21 = sum21 + r2[9] * _k0[5] + r2[11] * _k0[13];
						sum22 = sum22 + r2[9] * _k0[6] + r2[11] * _k0[14];
						sum23 = sum23 + r2[9] * _k0[7] + r2[11] * _k0[15];

						sum30 = sum30 + r2[13] * _k0[4] + r2[15] * _k0[12];
						sum31 = sum31 + r2[13] * _k0[5] + r2[15] * _k0[13];
						sum32 = sum32 + r2[13] * _k0[6] + r2[15] * _k0[14];
						sum33 = sum33 + r2[13] * _k0[7] + r2[15] * _k0[15];
						_k0 -= 128;

						r0 += 8;
						r1 += 8;
						r2 += 8;

						outptr0[0] = sum20 + sum00;
						outptr0[1] = sum21 + sum01;
						outptr0[2] = sum22 + sum02;
						outptr0[3] = sum23 + sum03;

						outptr0[4] = sum30 + sum10;
						outptr0[5] = sum31 + sum11;
						outptr0[6] = sum32 + sum12;
						outptr0[7] = sum33 + sum13;

						outptr0 += 8;
					}
					for (; j < outw; j++)
					{
						float sum00, sum01, sum02, sum03, sum10, sum11, sum12, sum13, sum20, sum21, sum22, sum23, sum30, sum31, sum32, sum33;

						sum00 = r0[0] * _k0[0];
						sum01 = r0[0] * _k0[1];
						sum02 = r0[0] * _k0[2];
						sum03 = r0[0] * _k0[3];

						sum10 = r0[1] * _k0[4];
						sum11 = r0[1] * _k0[5];
						sum12 = r0[1] * _k0[6];
						sum13 = r0[1] * _k0[7];

						sum20 = r0[2] * _k0[8];
						sum21 = r0[2] * _k0[9];
						sum22 = r0[2] * _k0[10];
						sum23 = r0[2] * _k0[11];

						sum30 = outptr0[0] + r0[3] * _k0[12];
						sum31 = outptr0[1] + r0[3] * _k0[13];
						sum32 = outptr0[2] + r0[3] * _k0[14];
						sum33 = outptr0[3] + r0[3] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r0[4] * _k0[0];
						sum01 = sum01 + r0[4] * _k0[1];
						sum02 = sum02 + r0[4] * _k0[2];
						sum03 = sum03 + r0[4] * _k0[3];

						sum10 = sum10 + r0[5] * _k0[4];
						sum11 = sum11 + r0[5] * _k0[5];
						sum12 = sum12 + r0[5] * _k0[6];
						sum13 = sum13 + r0[5] * _k0[7];

						sum20 = sum20 + r0[6] * _k0[8];
						sum21 = sum21 + r0[6] * _k0[9];
						sum22 = sum22 + r0[6] * _k0[10];
						sum23 = sum23 + r0[6] * _k0[11];

						sum30 = sum30 + r0[7] * _k0[12];
						sum31 = sum31 + r0[7] * _k0[13];
						sum32 = sum32 + r0[7] * _k0[14];
						sum33 = sum33 + r0[7] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r0[8] * _k0[0];
						sum01 = sum01 + r0[8] * _k0[1];
						sum02 = sum02 + r0[8] * _k0[2];
						sum03 = sum03 + r0[8] * _k0[3];

						sum10 = sum10 + r0[9] * _k0[4];
						sum11 = sum11 + r0[9] * _k0[5];
						sum12 = sum12 + r0[9] * _k0[6];
						sum13 = sum13 + r0[9] * _k0[7];

						sum20 = sum20 + r0[10] * _k0[8];
						sum21 = sum21 + r0[10] * _k0[9];
						sum22 = sum22 + r0[10] * _k0[10];
						sum23 = sum23 + r0[10] * _k0[11];

						sum30 = sum30 + r0[11] * _k0[12];
						sum31 = sum31 + r0[11] * _k0[13];
						sum32 = sum32 + r0[11] * _k0[14];
						sum33 = sum33 + r0[11] * _k0[15];
						_k0 += 16;


						sum00 = sum00 + r1[0] * _k0[0];
						sum01 = sum01 + r1[0] * _k0[1];
						sum02 = sum02 + r1[0] * _k0[2];
						sum03 = sum03 + r1[0] * _k0[3];

						sum10 = sum10 + r1[1] * _k0[4];
						sum11 = sum11 + r1[1] * _k0[5];
						sum12 = sum12 + r1[1] * _k0[6];
						sum13 = sum13 + r1[1] * _k0[7];

						sum20 = sum20 + r1[2] * _k0[8];
						sum21 = sum21 + r1[2] * _k0[9];
						sum22 = sum22 + r1[2] * _k0[10];
						sum23 = sum23 + r1[2] * _k0[11];

						sum30 = sum30 + r1[3] * _k0[12];
						sum31 = sum31 + r1[3] * _k0[13];
						sum32 = sum32 + r1[3] * _k0[14];
						sum33 = sum33 + r1[3] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r1[4] * _k0[0];
						sum01 = sum01 + r1[4] * _k0[1];
						sum02 = sum02 + r1[4] * _k0[2];
						sum03 = sum03 + r1[4] * _k0[3];

						sum10 = sum10 + r1[5] * _k0[4];
						sum11 = sum11 + r1[5] * _k0[5];
						sum12 = sum12 + r1[5] * _k0[6];
						sum13 = sum13 + r1[5] * _k0[7];

						sum20 = sum20 + r1[6] * _k0[8];
						sum21 = sum21 + r1[6] * _k0[9];
						sum22 = sum22 + r1[6] * _k0[10];
						sum23 = sum23 + r1[6] * _k0[11];

						sum30 = sum30 + r1[7] * _k0[12];
						sum31 = sum31 + r1[7] * _k0[13];
						sum32 = sum32 + r1[7] * _k0[14];
						sum33 = sum33 + r1[7] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r1[8] * _k0[0];
						sum01 = sum01 + r1[8] * _k0[1];
						sum02 = sum02 + r1[8] * _k0[2];
						sum03 = sum03 + r1[8] * _k0[3];

						sum10 = sum10 + r1[9] * _k0[4];
						sum11 = sum11 + r1[9] * _k0[5];
						sum12 = sum12 + r1[9] * _k0[6];
						sum13 = sum13 + r1[9] * _k0[7];

						sum20 = sum20 + r1[10] * _k0[8];
						sum21 = sum21 + r1[10] * _k0[9];
						sum22 = sum22 + r1[10] * _k0[10];
						sum23 = sum23 + r1[10] * _k0[11];

						sum30 = sum30 + r1[11] * _k0[12];
						sum31 = sum31 + r1[11] * _k0[13];
						sum32 = sum32 + r1[11] * _k0[14];
						sum33 = sum33 + r1[11] * _k0[15];
						_k0 += 16;


						sum00 = sum00 + r2[0] * _k0[0];
						sum01 = sum01 + r2[0] * _k0[1];
						sum02 = sum02 + r2[0] * _k0[2];
						sum03 = sum03 + r2[0] * _k0[3];

						sum10 = sum10 + r2[1] * _k0[4];
						sum11 = sum11 + r2[1] * _k0[5];
						sum12 = sum12 + r2[1] * _k0[6];
						sum13 = sum13 + r2[1] * _k0[7];

						sum20 = sum20 + r2[2] * _k0[8];
						sum21 = sum21 + r2[2] * _k0[9];
						sum22 = sum22 + r2[2] * _k0[10];
						sum23 = sum23 + r2[2] * _k0[11];

						sum30 = sum30 + r2[3] * _k0[12];
						sum31 = sum31 + r2[3] * _k0[13];
						sum32 = sum32 + r2[3] * _k0[14];
						sum33 = sum33 + r2[3] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r2[4] * _k0[0];
						sum01 = sum01 + r2[4] * _k0[1];
						sum02 = sum02 + r2[4] * _k0[2];
						sum03 = sum03 + r2[4] * _k0[3];

						sum10 = sum10 + r2[5] * _k0[4];
						sum11 = sum11 + r2[5] * _k0[5];
						sum12 = sum12 + r2[5] * _k0[6];
						sum13 = sum13 + r2[5] * _k0[7];

						sum20 = sum20 + r2[6] * _k0[8];
						sum21 = sum21 + r2[6] * _k0[9];
						sum22 = sum22 + r2[6] * _k0[10];
						sum23 = sum23 + r2[6] * _k0[11];

						sum30 = sum30 + r2[7] * _k0[12];
						sum31 = sum31 + r2[7] * _k0[13];
						sum32 = sum32 + r2[7] * _k0[14];
						sum33 = sum33 + r2[7] * _k0[15];
						_k0 += 16;

						sum00 = sum00 + r2[8] * _k0[0];
						sum01 = sum01 + r2[8] * _k0[1];
						sum02 = sum02 + r2[8] * _k0[2];
						sum03 = sum03 + r2[8] * _k0[3];

						sum10 = sum10 + r2[9] * _k0[4];
						sum11 = sum11 + r2[9] * _k0[5];
						sum12 = sum12 + r2[9] * _k0[6];
						sum13 = sum13 + r2[9] * _k0[7];

						sum20 = sum20 + r2[10] * _k0[8];
						sum21 = sum21 + r2[10] * _k0[9];
						sum22 = sum22 + r2[10] * _k0[10];
						sum23 = sum23 + r2[10] * _k0[11];

						sum30 = sum30 + r2[11] * _k0[12];
						sum31 = sum31 + r2[11] * _k0[13];
						sum32 = sum32 + r2[11] * _k0[14];
						sum33 = sum33 + r2[11] * _k0[15];
						_k0 -= 128;

						r0 += 4;
						r1 += 4;
						r2 += 4;

						outptr0[0] = (sum10 + sum00) + (sum20 + sum30);
						outptr0[1] = (sum11 + sum01) + (sum21 + sum31);
						outptr0[2] = (sum12 + sum02) + (sum22 + sum32);
						outptr0[3] = (sum13 + sum03) + (sum23 + sum33);

						outptr0 += 4;
					}
					r0 += 2 * 4;
					r1 += 2 * 4;
					r2 += 2 * 4;
				}
			}
		}
#endif
	}

    void enn_conv3x3s1_pack4to1_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int out_size = outw * outh;
        int out_cstep = enn_get_blob_size(out_size, 4);
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

			const float bias0 = bias ? _gnu_h2f_internal_f(bias[p]) : 0.f;
			{ float* ptr = out0; int t; for (t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out0.fill(bias0);

			// const float* k0 = kernel.channel(p);
			const unsigned short* k0 = kernel + p * (36 * inch);

			int q;
			for (q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				const float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0; // img0.row(0);
				const float* r1 = r0 + w * 4; // img0.row(1);
				const float* r2 = r1 + w * 4; // img0.row(2);

				float32x4_t _k00 = vcvt_f32_f16((float16x4_t)vld1_u16(k0));
				float32x4_t _k01 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 4));
				float32x4_t _k02 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 8));
				float32x4_t _k10 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 12));
				float32x4_t _k11 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 16));
				float32x4_t _k12 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 20));
				float32x4_t _k20 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 24));
				float32x4_t _k21 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 28));
				float32x4_t _k22 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 32));

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
			float* out0 = out_mem + p * out_cstep; // top_blob.channel(p);

			const float bias0 = bias ? _gnu_h2f_internal_f(bias[p]) : 0.f;
			{ float* ptr = out0; int t; for (t = 0; t < out_size; t++) { *ptr = bias0; ptr++; } } // out0.fill(bias0);

			const unsigned short* k0 = kernel + p * (36 * inch);

			int q;
			for (q = 0; q < inch; q++)
			{
				float* outptr0 = out0; // out0.row(0);

				const float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);

				const float* r0 = img0; // img0.row(0);
				const float* r1 = r0 + w * 4; // img0.row(1);
				const float* r2 = r1 + w * 4; // img0.row(2);

				float _k0[36];
				_gnu_h2f_internal_vector(k0, _k0, 36);

				int i = 0;
				for (; i < outh; i++)
				{
					int j = 0;
					for (; j + 3 < outw; j += 4)
					{
						float sum0, sum1, sum2, sum3;

						sum0 = r0[0] * _k0[0] + r0[4] * _k0[4] + r0[8 ] * _k0[8 ] + r1[0] * _k0[12] + r1[4] * _k0[16] + r1[8 ] * _k0[20] + r2[0] * _k0[24] + r2[4] * _k0[28] + r2[8 ] * _k0[32];
						sum1 = r0[1] * _k0[1] + r0[5] * _k0[5] + r0[9 ] * _k0[9 ] + r1[1] * _k0[13] + r1[5] * _k0[17] + r1[9 ] * _k0[21] + r2[1] * _k0[25] + r2[5] * _k0[29] + r2[9 ] * _k0[33];
						sum2 = r0[2] * _k0[2] + r0[6] * _k0[6] + r0[10] * _k0[10] + r1[2] * _k0[14] + r1[6] * _k0[18] + r1[10] * _k0[22] + r2[2] * _k0[26] + r2[6] * _k0[30] + r2[10] * _k0[34];
						sum3 = r0[3] * _k0[3] + r0[7] * _k0[7] + r0[11] * _k0[11] + r1[3] * _k0[15] + r1[7] * _k0[19] + r1[11] * _k0[23] + r2[3] * _k0[27] + r2[7] * _k0[31] + r2[11] * _k0[35];

						outptr0[0] += ((sum0 + sum2) + (sum1 + sum3));
						r0 += 4; r1 += 4; r2 += 4;

						sum0 = r0[0] * _k0[0] + r0[4] * _k0[4] + r0[8 ] * _k0[8 ] + r1[0] * _k0[12] + r1[4] * _k0[16] + r1[8 ] * _k0[20] + r2[0] * _k0[24] + r2[4] * _k0[28] + r2[8 ] * _k0[32];
						sum1 = r0[1] * _k0[1] + r0[5] * _k0[5] + r0[9 ] * _k0[9 ] + r1[1] * _k0[13] + r1[5] * _k0[17] + r1[9 ] * _k0[21] + r2[1] * _k0[25] + r2[5] * _k0[29] + r2[9 ] * _k0[33];
						sum2 = r0[2] * _k0[2] + r0[6] * _k0[6] + r0[10] * _k0[10] + r1[2] * _k0[14] + r1[6] * _k0[18] + r1[10] * _k0[22] + r2[2] * _k0[26] + r2[6] * _k0[30] + r2[10] * _k0[34];
						sum3 = r0[3] * _k0[3] + r0[7] * _k0[7] + r0[11] * _k0[11] + r1[3] * _k0[15] + r1[7] * _k0[19] + r1[11] * _k0[23] + r2[3] * _k0[27] + r2[7] * _k0[31] + r2[11] * _k0[35];

						outptr0[1] += ((sum0 + sum2) + (sum1 + sum3));
						r0 += 4; r1 += 4; r2 += 4;

						sum0 = r0[0] * _k0[0] + r0[4] * _k0[4] + r0[8 ] * _k0[8 ] + r1[0] * _k0[12] + r1[4] * _k0[16] + r1[8 ] * _k0[20] + r2[0] * _k0[24] + r2[4] * _k0[28] + r2[8 ] * _k0[32];
						sum1 = r0[1] * _k0[1] + r0[5] * _k0[5] + r0[9 ] * _k0[9 ] + r1[1] * _k0[13] + r1[5] * _k0[17] + r1[9 ] * _k0[21] + r2[1] * _k0[25] + r2[5] * _k0[29] + r2[9 ] * _k0[33];
						sum2 = r0[2] * _k0[2] + r0[6] * _k0[6] + r0[10] * _k0[10] + r1[2] * _k0[14] + r1[6] * _k0[18] + r1[10] * _k0[22] + r2[2] * _k0[26] + r2[6] * _k0[30] + r2[10] * _k0[34];
						sum3 = r0[3] * _k0[3] + r0[7] * _k0[7] + r0[11] * _k0[11] + r1[3] * _k0[15] + r1[7] * _k0[19] + r1[11] * _k0[23] + r2[3] * _k0[27] + r2[7] * _k0[31] + r2[11] * _k0[35];

						outptr0[2] += ((sum0 + sum2) + (sum1 + sum3));
						r0 += 4; r1 += 4; r2 += 4;

						sum0 = r0[0] * _k0[0] + r0[4] * _k0[4] + r0[8 ] * _k0[8 ] + r1[0] * _k0[12] + r1[4] * _k0[16] + r1[8 ] * _k0[20] + r2[0] * _k0[24] + r2[4] * _k0[28] + r2[8 ] * _k0[32];
						sum1 = r0[1] * _k0[1] + r0[5] * _k0[5] + r0[9 ] * _k0[9 ] + r1[1] * _k0[13] + r1[5] * _k0[17] + r1[9 ] * _k0[21] + r2[1] * _k0[25] + r2[5] * _k0[29] + r2[9 ] * _k0[33];
						sum2 = r0[2] * _k0[2] + r0[6] * _k0[6] + r0[10] * _k0[10] + r1[2] * _k0[14] + r1[6] * _k0[18] + r1[10] * _k0[22] + r2[2] * _k0[26] + r2[6] * _k0[30] + r2[10] * _k0[34];
						sum3 = r0[3] * _k0[3] + r0[7] * _k0[7] + r0[11] * _k0[11] + r1[3] * _k0[15] + r1[7] * _k0[19] + r1[11] * _k0[23] + r2[3] * _k0[27] + r2[7] * _k0[31] + r2[11] * _k0[35];

						outptr0[3] += ((sum0 + sum2) + (sum1 + sum3));
						r0 += 4; r1 += 4; r2 += 4;

						outptr0 += 4;
					}
					for (; j + 1 < outw; j += 2)
					{
						float sum0, sum1, sum2, sum3;

						sum0 = r0[0] * _k0[0] + r0[ 8] * _k0[ 8] + r1[4] * _k0[16] + r2[0] * _k0[24] + r2[ 8] * _k0[32];
						sum1 = r0[1] * _k0[1] + r0[ 9] * _k0[ 9] + r1[5] * _k0[17] + r2[1] * _k0[25] + r2[ 9] * _k0[33];
						sum2 = r0[2] * _k0[2] + r0[10] * _k0[10] + r1[6] * _k0[18] + r2[2] * _k0[26] + r2[10] * _k0[34];
						sum3 = r0[3] * _k0[3] + r0[11] * _k0[11] + r1[7] * _k0[19] + r2[3] * _k0[27] + r2[11] * _k0[35];

						sum0 += (r0[4] * _k0[4] + r1[0] * _k0[12] + r1[ 8] * _k0[20] + r2[4] * _k0[28]); 
						sum1 += (r0[5] * _k0[5] + r1[1] * _k0[13] + r1[ 9] * _k0[21] + r2[5] * _k0[29]); 
						sum2 += (r0[6] * _k0[6] + r1[2] * _k0[14] + r1[10] * _k0[22] + r2[6] * _k0[30]); 
						sum3 += (r0[7] * _k0[7] + r1[3] * _k0[15] + r1[11] * _k0[23] + r2[7] * _k0[31]); 

						outptr0[0] += ((sum0 + sum2) + (sum1 + sum3));
						r0 += 4; r1 += 4; r2 += 4;

						sum0 = r0[0] * _k0[0] + r0[ 8] * _k0[ 8] + r1[4] * _k0[16] + r2[0] * _k0[24] + r2[ 8] * _k0[32];
						sum1 = r0[1] * _k0[1] + r0[ 9] * _k0[ 9] + r1[5] * _k0[17] + r2[1] * _k0[25] + r2[ 9] * _k0[33];
						sum2 = r0[2] * _k0[2] + r0[10] * _k0[10] + r1[6] * _k0[18] + r2[2] * _k0[26] + r2[10] * _k0[34];
						sum3 = r0[3] * _k0[3] + r0[11] * _k0[11] + r1[7] * _k0[19] + r2[3] * _k0[27] + r2[11] * _k0[35];

						sum0 += (r0[4] * _k0[4] + r1[0] * _k0[12] + r1[ 8] * _k0[20] + r2[4] * _k0[28]);
						sum1 += (r0[5] * _k0[5] + r1[1] * _k0[13] + r1[ 9] * _k0[21] + r2[5] * _k0[29]);
						sum2 += (r0[6] * _k0[6] + r1[2] * _k0[14] + r1[10] * _k0[22] + r2[6] * _k0[30]);
						sum3 += (r0[7] * _k0[7] + r1[3] * _k0[15] + r1[11] * _k0[23] + r2[7] * _k0[31]);

						outptr0[1] += ((sum0 + sum2) + (sum1 + sum3));
						r0 += 4; r1 += 4; r2 += 4;

						outptr0 += 2;
					}
					for (; j < outw; j++)
					{
						float /*sum = outptr0[0],*/ sum0, sum1, sum2, sum3;

						sum0 = (outptr0[0] + r0[ 8] * _k0[8 ] + r1[8 ] * _k0[20] + r2[8 ] * _k0[32]) + ((r0[0] * _k0[0] + r1[0] * _k0[12] + r2[0] * _k0[24]) + (r0[4] * _k0[4] + r1[4] * _k0[16] + r2[4] * _k0[28]));
						sum1 = (             r0[ 9] * _k0[9 ] + r1[9 ] * _k0[21] + r2[9 ] * _k0[33]) + ((r0[1] * _k0[1] + r1[1] * _k0[13] + r2[1] * _k0[25]) + (r0[5] * _k0[5] + r1[5] * _k0[17] + r2[5] * _k0[29]));
						sum2 = (             r0[10] * _k0[10] + r1[10] * _k0[22] + r2[10] * _k0[34]) + ((r0[2] * _k0[2] + r1[2] * _k0[14] + r2[2] * _k0[26]) + (r0[6] * _k0[6] + r1[6] * _k0[18] + r2[6] * _k0[30]));
						sum3 = (             r0[11] * _k0[11] + r1[11] * _k0[23] + r2[11] * _k0[35]) + ((r0[3] * _k0[3] + r1[3] * _k0[15] + r2[3] * _k0[27]) + (r0[7] * _k0[7] + r1[7] * _k0[19] + r2[7] * _k0[31]));

						outptr0[0] = ((sum0 + sum2) + (sum1 + sum3));
						r0 += 4; r1 += 4; r2 += 4;
						outptr0++;
					}
					r0 += 2 * 4;
					r1 += 2 * 4;
					r2 += 2 * 4;
				}
				k0 += 9 * 4;
			}
		}
#endif
	}

    void enn_conv3x3s2_pack1to4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias)
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
			float32x4_t _bias0 = bias ? vcvt_f32_f16((float16x4_t)vld1_u16(bias + p * 4)) : vdupq_n_f32(0.f);
			{
				float* ptr = out0;
				int t;
				for (t = 0; t < out_size; t++)
				{
					vst1q_f32(ptr, _bias0);
					ptr += 4;
				}
			}
#else
			{
				if (bias)
				{
					float biasf[4];
					_gnu_h2f_internal_vector(bias + p * 4, biasf, 4);
					for (int t = 0; t < out_size; t++)
						memcpy(out0 + t * 4, biasf, 16);
				}
				else
				{
					memset(out0, 0, out_size * 16);
				}
			}
#endif
			const unsigned short* k0 = kernel + p * kernel_cstep; // .channel(p);

			int q;
			for (q = 0; q < inch; q++)
			{
				float* outptr0 = out0;

				float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);

				const float* r0 = img0 + 0 * w; // .row(0);
				const float* r1 = img0 + 1 * w; // .row(1);
				const float* r2 = img0 + 2 * w; // .row(2);
#if __ARM_NEON
				float32x4_t _k00 = vcvt_f32_f16((float16x4_t)vld1_u16(k0));
				float32x4_t _k01 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 4));
				float32x4_t _k02 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 8));
				float32x4_t _k10 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 12));
				float32x4_t _k11 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 16));
				float32x4_t _k12 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 20));
				float32x4_t _k20 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 24));
				float32x4_t _k21 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 28));
				float32x4_t _k22 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 32));
#else
				float k0f[36];
				_gnu_h2f_internal_vector(k0, k0f, 36);
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
						outptr0[0] += k0f[0] * r0[0];
						outptr0[1] += k0f[1] * r0[0];
						outptr0[2] += k0f[2] * r0[0];
						outptr0[3] += k0f[3] * r0[0];

						outptr0[0] += k0f[4] * r0[1];
						outptr0[1] += k0f[5] * r0[1];
						outptr0[2] += k0f[6] * r0[1];
						outptr0[3] += k0f[7] * r0[1];

						outptr0[0] += k0f[8] * r0[2];
						outptr0[1] += k0f[9] * r0[2];
						outptr0[2] += k0f[10] * r0[2];
						outptr0[3] += k0f[11] * r0[2];

						outptr0[0] += k0f[12] * r1[0];
						outptr0[1] += k0f[13] * r1[0];
						outptr0[2] += k0f[14] * r1[0];
						outptr0[3] += k0f[15] * r1[0];

						outptr0[0] += k0f[16] * r1[1];
						outptr0[1] += k0f[17] * r1[1];
						outptr0[2] += k0f[18] * r1[1];
						outptr0[3] += k0f[19] * r1[1];

						outptr0[0] += k0f[20] * r1[2];
						outptr0[1] += k0f[21] * r1[2];
						outptr0[2] += k0f[22] * r1[2];
						outptr0[3] += k0f[23] * r1[2];

						outptr0[0] += k0f[24] * r2[0];
						outptr0[1] += k0f[25] * r2[0];
						outptr0[2] += k0f[26] * r2[0];
						outptr0[3] += k0f[27] * r2[0];

						outptr0[0] += k0f[28] * r2[1];
						outptr0[1] += k0f[29] * r2[1];
						outptr0[2] += k0f[30] * r2[1];
						outptr0[3] += k0f[31] * r2[1];

						outptr0[0] += k0f[32] * r2[2];
						outptr0[1] += k0f[33] * r2[2];
						outptr0[2] += k0f[34] * r2[2];
						outptr0[3] += k0f[35] * r2[2];
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

    void enn_convdw3x3s1_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias)
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
			float32x4_t _bias0 = bias ? vcvt_f32_f16((float16x4_t)vld1_u16(bias + g * 4)) : vdupq_n_f32(0.f);
#endif
			const unsigned short* k0 = kernel + g * kernel_cstep; // kernel.row(g);

			float* outptr0 = out;				// out.row(0);
			//float* outptr1 = out + outw * 4;	// out.row(1);

			float* img0 = in_mem + g * cstep; // const Mat img0 = bottom_blob.channel(g);

			const float* r0 = img0;					// .row(0);
			const float* r1 = img0 + 4 * w;			// .row(1);
			const float* r2 = img0 + 8 * w;			// .row(2);
			//const float* r3 = img0 + 12 * w;		// .row(3);
#if __ARM_NEON
			float32x4_t _k00 = vcvt_f32_f16((float16x4_t)vld1_u16(k0));
			float32x4_t _k01 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 4));
			float32x4_t _k02 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 8));
			float32x4_t _k10 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 12));
			float32x4_t _k11 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 16));
			float32x4_t _k12 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 20));
			float32x4_t _k20 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 24));
			float32x4_t _k21 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 28));
			float32x4_t _k22 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 32));
#else
			float k0f[36];
			_gnu_h2f_internal_vector(k0, k0f, 36);
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
						outptr0[0] = _gnu_h2f_internal_f(*(bias + g * 4 + 0));
						outptr0[1] = _gnu_h2f_internal_f(*(bias + g * 4 + 1));
						outptr0[2] = _gnu_h2f_internal_f(*(bias + g * 4 + 2));
						outptr0[3] = _gnu_h2f_internal_f(*(bias + g * 4 + 3));
					}
					else
					{
						outptr0[0] = 0.0f;
						outptr0[1] = 0.0f;
						outptr0[2] = 0.0f;
						outptr0[3] = 0.0f;
					}

					outptr0[0] += k0f[0] * r0[0];
					outptr0[1] += k0f[1] * r0[1];
					outptr0[2] += k0f[2] * r0[2];
					outptr0[3] += k0f[3] * r0[3];

					outptr0[0] += k0f[4] * r0[4];
					outptr0[1] += k0f[5] * r0[5];
					outptr0[2] += k0f[6] * r0[6];
					outptr0[3] += k0f[7] * r0[7];

					outptr0[0] += k0f[8] * r0[8];
					outptr0[1] += k0f[9] * r0[9];
					outptr0[2] += k0f[10] * r0[10];
					outptr0[3] += k0f[11] * r0[11];

					outptr0[0] += k0f[12] * r1[0];
					outptr0[1] += k0f[13] * r1[1];
					outptr0[2] += k0f[14] * r1[2];
					outptr0[3] += k0f[15] * r1[3];

					outptr0[0] += k0f[16] * r1[4];
					outptr0[1] += k0f[17] * r1[5];
					outptr0[2] += k0f[18] * r1[6];
					outptr0[3] += k0f[19] * r1[7];

					outptr0[0] += k0f[20] * r1[8];
					outptr0[1] += k0f[21] * r1[9];
					outptr0[2] += k0f[22] * r1[10];
					outptr0[3] += k0f[23] * r1[11];

					outptr0[0] += k0f[24] * r2[0];
					outptr0[1] += k0f[25] * r2[1];
					outptr0[2] += k0f[26] * r2[2];
					outptr0[3] += k0f[27] * r2[3];

					outptr0[0] += k0f[28] * r2[4];
					outptr0[1] += k0f[29] * r2[5];
					outptr0[2] += k0f[30] * r2[6];
					outptr0[3] += k0f[31] * r2[7];

					outptr0[0] += k0f[32] * r2[8];
					outptr0[1] += k0f[33] * r2[9];
					outptr0[2] += k0f[34] * r2[10];
					outptr0[3] += k0f[35] * r2[11];
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

    void enn_convdw3x3s2_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias)
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
			float32x4_t _bias0 = bias ? vcvt_f32_f16((float16x4_t)vld1_u16(bias + g * 4)) : vdupq_n_f32(0.f);
#endif
			const unsigned short* k0 = kernel + g * 36; // .row(g);

			float* outptr0 = out;

			float* img0 = in_mem + g * cstep; // const Mat img0 = bottom_blob.channel(g);

			const float* r0 = img0; //.row(0);
			const float* r1 = img0 + 4 * w; //.row(1);
			const float* r2 = img0 + 8 * w; //.row(2);

#if __ARM_NEON
			float32x4_t _k00 = vcvt_f32_f16((float16x4_t)vld1_u16(k0));
			float32x4_t _k01 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 4));
			float32x4_t _k02 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 8));
			float32x4_t _k10 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 12));
			float32x4_t _k11 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 16));
			float32x4_t _k12 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 20));
			float32x4_t _k20 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 24));
			float32x4_t _k21 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 28));
			float32x4_t _k22 = vcvt_f32_f16((float16x4_t)vld1_u16(k0 + 32));
#else
			float k0f[36];
			_gnu_h2f_internal_vector(k0, k0f, 36);
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
					sum[0] = (bias) ? _gnu_h2f_internal_f(bias[g * 4]) : 0.0f;
					sum[1] = (bias) ? _gnu_h2f_internal_f(bias[g * 4 + 1]) : 0.0f;
					sum[2] = (bias) ? _gnu_h2f_internal_f(bias[g * 4 + 2]) : 0.0f;
					sum[3] = (bias) ? _gnu_h2f_internal_f(bias[g * 4 + 3]) : 0.0f;

					sum[0] += k0f[0] * r0[0];
					sum[1] += k0f[1] * r0[1];
					sum[2] += k0f[2] * r0[2];
					sum[3] += k0f[3] * r0[3];

					sum[0] += k0f[4] * r0[4];
					sum[1] += k0f[5] * r0[5];
					sum[2] += k0f[6] * r0[6];
					sum[3] += k0f[7] * r0[7];

					sum[0] += k0f[8] * r0[8];
					sum[1] += k0f[9] * r0[9];
					sum[2] += k0f[10] * r0[10];
					sum[3] += k0f[11] * r0[11];

					sum[0] += k0f[12] * r1[0];
					sum[1] += k0f[13] * r1[1];
					sum[2] += k0f[14] * r1[2];
					sum[3] += k0f[15] * r1[3];

					sum[0] += k0f[16] * r1[4];
					sum[1] += k0f[17] * r1[5];
					sum[2] += k0f[18] * r1[6];
					sum[3] += k0f[19] * r1[7];

					sum[0] += k0f[20] * r1[8];
					sum[1] += k0f[21] * r1[9];
					sum[2] += k0f[22] * r1[10];
					sum[3] += k0f[23] * r1[11];

					sum[0] += k0f[24] * r2[0];
					sum[1] += k0f[25] * r2[1];
					sum[2] += k0f[26] * r2[2];
					sum[3] += k0f[27] * r2[3];

					sum[0] += k0f[28] * r2[4];
					sum[1] += k0f[29] * r2[5];
					sum[2] += k0f[30] * r2[6];
					sum[3] += k0f[31] * r2[7];

					sum[0] += k0f[32] * r2[8];
					sum[1] += k0f[33] * r2[9];
					sum[2] += k0f[34] * r2[10];
					sum[3] += k0f[35] * r2[11];

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

    void enn_convdw_gdc_pack4_neon(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel, int stride, unsigned short* kernel, unsigned short* bias)
	{
        enn_convdw_gdc_pack4_neon_(in_blob, out_blob, dim_ch_out, dim_kernel, dim_kernel, stride, kernel, bias);
	}

    void enn_convdw_gdc_pack4_neon_(enn_blob* in_blob, enn_blob* out_blob, int dim_ch_out, int dim_kernel_w, int dim_kernel_h, int stride, unsigned short* kernel, unsigned short* bias)
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
			const unsigned short* kptr = kernel + maxk * g * 4; // (const float*)weight_data_pack4 + maxk * g * 4;
			const float* m = in_mem + g * cstep; // const Mat m = bottom_blob_bordered.channel(g);

#if __ARM_NEON
			float32x4_t _sum = vdupq_n_f32(0.f);

			if (bias)
			{
				_sum = vcvt_f32_f16((float16x4_t)vld1_u16(bias + g * 4));
			}

			//const float* sptr = m; // m.row(i*stride_h) + j*stride_w * 4;

			int k;
			for (k = 0; k < maxk; k++)
			{
				float32x4_t _val = vld1q_f32(m + k * 4);
				float32x4_t _w = vcvt_f32_f16((float16x4_t)vld1_u16(kptr + k * 4));
				_sum = vmlaq_f32(_sum, _val, _w);
			}

			// _sum = activation_ps(_sum, activation_type, activation_params);
			vst1q_f32(outptr, _sum);
#else
			float sum0 = 0.0f, sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;

			if (bias)
			{
				sum0 = _gnu_h2f_internal_f(bias[g * 4]);
				sum1 = _gnu_h2f_internal_f(bias[g * 4 + 1]);
				sum2 = _gnu_h2f_internal_f(bias[g * 4 + 2]);
				sum3 = _gnu_h2f_internal_f(bias[g * 4 + 3]);
			}

			// const float* sptr = m; // m.row(i*stride_h) + j*stride_w * 4;

			for (int k = 0; k < maxk; k++)
			{
				sum0 += m[k * 4 + 0] * _gnu_h2f_internal_f(kptr[k * 4 + 0]);
				sum1 += m[k * 4 + 1] * _gnu_h2f_internal_f(kptr[k * 4 + 1]);
				sum2 += m[k * 4 + 2] * _gnu_h2f_internal_f(kptr[k * 4 + 2]);
				sum3 += m[k * 4 + 3] * _gnu_h2f_internal_f(kptr[k * 4 + 3]);
			}

			outptr[0] = sum0;
			outptr[1] = sum1;
			outptr[2] = sum2;
			outptr[3] = sum3;
#endif
		}
	}
