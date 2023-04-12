
#include <string.h>
#include <math.h>
#include <stdio.h>

#if __ARM_NEON
#include <arm_neon.h>
#endif

#include "ennq_conv.h"

namespace ENNQ
{
	static inline signed char float2int8(float v)
	{
		int int32 = (int)round(v);
		if (int32 > 127) return 127;
		if (int32 < -128) return -128;
		return (signed char)int32;
	}

	// static void conv1x1s1_sgemm_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& kernel, const Option& opt)
	// conv1x1s1_sgemm_transform_kernel_int8_neon
	// (8 * 4) * (inch / 4 + inch % 4) * (size / 8 + (size % 8) / 4 + size % 4)
	void conv_k1s1_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;
		int inch = ch_in;
		int outch = ch_out;

		const int size = w * h;

		int tmp_cstep = 32 * (inch / 4 + inch % 4);
		int kernel_cstep = 16 * (inch / 4 + inch % 4);

		// interleave
		// Mat tmp(8 * 4, inch / 4 + inch % 4, size / 8 + (size % 8) / 4 + size % 4, 1u, opt.workspace_allocator);
		signed char* mem_tmp = (signed char*)mem_tm;
		{
			int nn_size = size >> 3;
			int remain_size_start = nn_size << 3;

			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = ii * 8;

				const signed char* img0 = data_in + i; // bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + ii * tmp_cstep; // tmp.channel(i / 8);

				for (int q = 0; q < inch; q++)
				{
#if __ARM_NEON
					asm volatile(
						"pld        [%0, #64]     \n"
						"vld1.s8   {d0}, [%0]     \n"
						"vst1.s8   {d0}, [%1]!    \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "d0"
						);
					img0 += size;
#else                
					tmpptr[0] = img0[0];
					tmpptr[1] = img0[1];
					tmpptr[2] = img0[2];
					tmpptr[3] = img0[3];
					tmpptr[4] = img0[4];
					tmpptr[5] = img0[5];
					tmpptr[6] = img0[6];
					tmpptr[7] = img0[7];

					tmpptr += 8;
					img0 += size; // bottom_blob.cstep;
#endif // __ARM_NEON
				}
			}

			nn_size = (size - remain_size_start) >> 2;

			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = remain_size_start + ii * 4;

				const signed char* img0 = data_in + i; // bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);

				for (int q = 0; q < inch; q++)
				{
					tmpptr[0] = img0[0];
					tmpptr[1] = img0[1];
					tmpptr[2] = img0[2];
					tmpptr[3] = img0[3];

					tmpptr += 4;
					img0 += size; // bottom_blob.cstep;
				}
			}

			remain_size_start += nn_size << 2;

			for (int i = remain_size_start; i < size; i++)
			{
				const signed char* img0 = data_in + i; //  bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; //  tmp.channel(i / 8 + (i % 8) / 4 + i % 4);

				for (int q = 0; q < inch; q++)
				{
					tmpptr[0] = img0[0];
					tmpptr++;
					img0 += size; // bottom_blob.cstep;
				}
			}
		}

		// sgemm process
		int nn_outch = 0;
		int remain_outch_start = 0;

		nn_outch = (outch - remain_outch_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = remain_outch_start + pp * 4;

			float* outptr0 = data_out + p * size; // top_blob.channel(p);
			float* outptr1 = outptr0 + size; // top_blob.channel(p + 1);
			float* outptr2 = outptr1 + size; // top_blob.channel(p + 2);
			float* outptr3 = outptr2 + size; // top_blob.channel(p + 3);

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				const signed char* tmpptr = mem_tmp + (i / 8) * tmp_cstep; //  tmp.channel(i / 8);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"
					"vmov.s32    q8, #0            \n"
					"vmov.s32    q9, #0            \n"
					"vmov.s32    q10, #0           \n"
					"vmov.s32    q11, #0           \n"
					"vmov.s32    q12, #0           \n"
					"vmov.s32    q13, #0           \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4-d7}, [%4]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
					"vmovl.s8    q5, d7            \n"// a30-a37
					"vmovl.s8    q4, d6            \n"// a20-a27
					"vmovl.s8    q3, d5            \n"// a10-a17
					"vmovl.s8    q2, d4            \n"// a00-a07

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d4, d0[0]     \n"// sum0 = (a00-a07) * k00
					"vmlal.s16   q7, d5, d0[0]     \n"
					"vmlal.s16   q8, d4, d0[1]     \n"// sum1 = (a00-a07) * k10
					"vmlal.s16   q9, d5, d0[1]     \n"
					"vmlal.s16   q10, d4, d0[2]    \n"// sum2 = (a00-a07) * k20
					"vmlal.s16   q11, d5, d0[2]    \n"
					"vmlal.s16   q12, d4, d0[3]    \n"// sum3 = (a00-a07) * k30
					"vmlal.s16   q13, d5, d0[3]    \n"

					"vmlal.s16   q6, d6, d1[0]     \n"// sum0 += (a10-a17) * k01
					"vmlal.s16   q7, d7, d1[0]     \n"
					"vmlal.s16   q8, d6, d1[1]     \n"// sum1 += (a10-a17) * k11
					"vmlal.s16   q9, d7, d1[1]     \n"
					"vmlal.s16   q10, d6, d1[2]    \n"// sum2 += (a10-a17) * k21
					"vmlal.s16   q11, d7, d1[2]    \n"
					"vmlal.s16   q12, d6, d1[3]    \n"// sum3 += (a10-a17) * k31
					"vmlal.s16   q13, d7, d1[3]    \n"

					"vmlal.s16   q6, d8, d2[0]     \n"// sum0 += (a20-a27) * k02
					"vmlal.s16   q7, d9, d2[0]     \n"
					"vmlal.s16   q8, d8, d2[1]     \n"// sum1 += (a20-a27) * k12
					"vmlal.s16   q9, d9, d2[1]     \n"
					"vmlal.s16   q10, d8, d2[2]    \n"// sum2 += (a20-a27) * k22
					"vmlal.s16   q11, d9, d2[2]    \n"
					"vmlal.s16   q12, d8, d2[3]    \n"// sum3 += (a20-a27) * k32
					"vmlal.s16   q13, d9, d2[3]    \n"

					"vmlal.s16   q6, d10, d3[0]    \n"// sum0 += (a30-a37) * k03
					"vmlal.s16   q7, d11, d3[0]    \n"
					"vmlal.s16   q8, d10, d3[1]    \n"// sum1 += (a30-a37) * k13
					"vmlal.s16   q9, d11, d3[1]    \n"
					"vmlal.s16   q10, d10, d3[2]   \n"// sum2 += (a30-a37) * k23
					"vmlal.s16   q11, d11, d3[2]   \n"
					"vmlal.s16   q12, d10, d3[3]   \n"// sum3 += (a30-a37) * k33
					"vmlal.s16   q13, d11, d3[3]   \n"

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]!       \n"// tmpr a00-a07    a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// sum0 += (a00-a07) * k00
					"vmlal.s16   q7, d3, d0[0]     \n"
					"vmlal.s16   q8, d2, d0[1]     \n"// sum1 += (a00-a07) * k10
					"vmlal.s16   q9, d3, d0[1]     \n"
					"vmlal.s16   q10, d2, d0[2]    \n"// sum2 += (a00-a07) * k20
					"vmlal.s16   q11, d3, d0[2]    \n"
					"vmlal.s16   q12, d2, d0[3]    \n"// sum3 += (a00-a07) * k30
					"vmlal.s16   q13, d3, d0[3]    \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"
					"vcvt.f32.s32 q8, q8           \n"
					"vcvt.f32.s32 q9, q9           \n"

					"vld1.f32    {d0, d1}, [%7]    \n"

					"vcvt.f32.s32 q10, q10         \n"
					"vcvt.f32.s32 q11, q11         \n"
					"vcvt.f32.s32 q12, q12         \n"
					"vcvt.f32.s32 q13, q13         \n"

					"vdup.f32    q1, %8            \n"
					"vdup.f32    q3, %9            \n"
					"vmov.32     q2, q1            \n"
					"vmov.32     q4, q3            \n"

					"vmla.f32    q1, q6, d0[0]     \n"
					"vmla.f32    q2, q7, d0[0]     \n"
					"vmla.f32    q3, q8, d0[1]     \n"
					"vmla.f32    q4, q9, d0[1]     \n"

					"vdup.f32    q5, %10           \n"
					"vdup.f32    q7, %11           \n"
					"vmov.32     q6, q5            \n"
					"vmov.32     q8, q7            \n"

					"vmla.f32    q5, q10, d1[0]    \n"
					"vmla.f32    q6, q11, d1[0]    \n"
					"vmla.f32    q7, q12, d1[1]    \n"
					"vmla.f32    q8, q13, d1[1]    \n"

					"vst1.f32    {d2-d5}, [%0]!    \n"
					"vst1.f32    {d6-d9}, [%1]!    \n"
					"vst1.f32    {d10-d13}, [%2]!  \n"
					"vst1.f32    {d14-d17}, [%3]!  \n"

					: "+r"(outptr0),		// %0
					"+r"(outptr1),		// %1
					"+r"(outptr2),		// %2
					"+r"(outptr3),		// %3
					"+r"(tmpptr),			// %4
					"+r"(kptr)			// %5
					: "r"(inch),			// %6
					"r"(mem_scale + p),	// %7
					"r"(mem_bias[p]),		// %8
					"r"(mem_bias[p + 1]),	// %9
					"r"(mem_bias[p + 2]),	// %10
					"r"(mem_bias[p + 3])	// %11
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0_0 = 0;
				int sum0_1 = 0;
				int sum0_2 = 0;
				int sum0_3 = 0;
				int sum0_4 = 0;
				int sum0_5 = 0;
				int sum0_6 = 0;
				int sum0_7 = 0;

				int sum1_0 = 0;
				int sum1_1 = 0;
				int sum1_2 = 0;
				int sum1_3 = 0;
				int sum1_4 = 0;
				int sum1_5 = 0;
				int sum1_6 = 0;
				int sum1_7 = 0;

				int sum2_0 = 0;
				int sum2_1 = 0;
				int sum2_2 = 0;
				int sum2_3 = 0;
				int sum2_4 = 0;
				int sum2_5 = 0;
				int sum2_6 = 0;
				int sum2_7 = 0;

				int sum3_0 = 0;
				int sum3_1 = 0;
				int sum3_2 = 0;
				int sum3_3 = 0;
				int sum3_4 = 0;
				int sum3_5 = 0;
				int sum3_6 = 0;
				int sum3_7 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0_0 += tmpptr[0] * kptr[0];
					sum0_1 += tmpptr[1] * kptr[0];
					sum0_2 += tmpptr[2] * kptr[0];
					sum0_3 += tmpptr[3] * kptr[0];
					sum0_4 += tmpptr[4] * kptr[0];
					sum0_5 += tmpptr[5] * kptr[0];
					sum0_6 += tmpptr[6] * kptr[0];
					sum0_7 += tmpptr[7] * kptr[0];

					sum1_0 += tmpptr[0] * kptr[1];
					sum1_1 += tmpptr[1] * kptr[1];
					sum1_2 += tmpptr[2] * kptr[1];
					sum1_3 += tmpptr[3] * kptr[1];
					sum1_4 += tmpptr[4] * kptr[1];
					sum1_5 += tmpptr[5] * kptr[1];
					sum1_6 += tmpptr[6] * kptr[1];
					sum1_7 += tmpptr[7] * kptr[1];

					sum2_0 += tmpptr[0] * kptr[2];
					sum2_1 += tmpptr[1] * kptr[2];
					sum2_2 += tmpptr[2] * kptr[2];
					sum2_3 += tmpptr[3] * kptr[2];
					sum2_4 += tmpptr[4] * kptr[2];
					sum2_5 += tmpptr[5] * kptr[2];
					sum2_6 += tmpptr[6] * kptr[2];
					sum2_7 += tmpptr[7] * kptr[2];

					sum3_0 += tmpptr[0] * kptr[3];
					sum3_1 += tmpptr[1] * kptr[3];
					sum3_2 += tmpptr[2] * kptr[3];
					sum3_3 += tmpptr[3] * kptr[3];
					sum3_4 += tmpptr[4] * kptr[3];
					sum3_5 += tmpptr[5] * kptr[3];
					sum3_6 += tmpptr[6] * kptr[3];
					sum3_7 += tmpptr[7] * kptr[3];

					tmpptr += 8;
					kptr += 4;
				}

				outptr0[0] = sum0_0 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[1] = sum0_1 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[2] = sum0_2 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[3] = sum0_3 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[4] = sum0_4 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[5] = sum0_5 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[6] = sum0_6 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[7] = sum0_7 * mem_scale[p + 0] + mem_bias[p + 0];

				outptr1[0] = sum1_0 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[1] = sum1_1 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[2] = sum1_2 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[3] = sum1_3 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[4] = sum1_4 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[5] = sum1_5 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[6] = sum1_6 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[7] = sum1_7 * mem_scale[p + 1] + mem_bias[p + 1];

				outptr2[0] = sum2_0 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[1] = sum2_1 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[2] = sum2_2 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[3] = sum2_3 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[4] = sum2_4 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[5] = sum2_5 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[6] = sum2_6 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[7] = sum2_7 * mem_scale[p + 2] + mem_bias[p + 2];

				outptr3[0] = sum3_0 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[1] = sum3_1 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[2] = sum3_2 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[3] = sum3_3 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[4] = sum3_4 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[5] = sum3_5 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[6] = sum3_6 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[7] = sum3_7 * mem_scale[p + 3] + mem_bias[p + 3];

				outptr0 += 8;
				outptr1 += 8;
				outptr2 += 8;
				outptr3 += 8;
#endif // __ARM_NEON            
			}

			for (; i + 3 < size; i += 4)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"
					"vmov.s32    q8, #0            \n"
					"vmov.s32    q9, #0            \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4-d5}, [%4]!    \n"// tmpr a00-a03,a10-a13,a20-a23,a30-a33    a(inch)(data)
					"vmovl.s8    q3, d5            \n"// a20-a23,a30-a33
					"vmovl.s8    q2, d4            \n"// a00-a04,a10-a14

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d4, d0[0]     \n"// sum0 = (a00-a03) * k00
					"vmlal.s16   q7, d4, d0[1]     \n"// sum1 = (a00-a03) * k10
					"vmlal.s16   q8, d4, d0[2]     \n"// sum2 = (a00-a03) * k20
					"vmlal.s16   q9, d4, d0[3]     \n"// sum3 = (a00-a03) * k30

					"vmlal.s16   q6, d5, d1[0]     \n"// sum0 += (a10-a13) * k01
					"vmlal.s16   q7, d5, d1[1]     \n"// sum1 += (a10-a13) * k11
					"vmlal.s16   q8, d5, d1[2]     \n"// sum2 += (a10-a13) * k21
					"vmlal.s16   q9, d5, d1[3]     \n"// sum3 += (a10-a13) * k31

					"vmlal.s16   q6, d6, d2[0]     \n"// sum0 += (a20-a23) * k02
					"vmlal.s16   q7, d6, d2[1]     \n"// sum1 += (a20-a23) * k12
					"vmlal.s16   q8, d6, d2[2]     \n"// sum2 += (a20-a23) * k22
					"vmlal.s16   q9, d6, d2[3]     \n"// sum3 += (a20-a23) * k32

					"vmlal.s16   q6, d7, d3[0]     \n"// sum0 += (a30-a33) * k03
					"vmlal.s16   q7, d7, d3[1]     \n"// sum1 += (a30-a33) * k13
					"vmlal.s16   q8, d7, d3[2]     \n"// sum2 += (a30-a33) * k23
					"vmlal.s16   q9, d7, d3[3]     \n"// sum3 += (a30-a33) * k33

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]        \n"// tmpr a00-a03    a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %4, #4            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// sum0 += (a00-a03) * k00
					"vmlal.s16   q7, d2, d0[1]     \n"// sum1 += (a00-a03) * k10
					"vmlal.s16   q8, d2, d0[2]     \n"// sum2 += (a00-a03) * k20
					"vmlal.s16   q9, d2, d0[3]     \n"// sum3 += (a00-a03) * k30

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"
					"vcvt.f32.s32 q8, q8           \n"
					"vcvt.f32.s32 q9, q9           \n"

					"vld1.f32    {d0, d1}, [%7]    \n"

					"vdup.f32    q1, %8            \n"
					"vdup.f32    q2, %9            \n"
					"vdup.f32    q3, %10           \n"
					"vdup.f32    q4, %11           \n"

					"vmla.f32    q1, q6, d0[0]     \n"
					"vmla.f32    q2, q7, d0[1]     \n"
					"vmla.f32    q3, q8, d1[0]     \n"
					"vmla.f32    q4, q9, d1[1]     \n"

					"vst1.f32    {d2-d3}, [%0]!    \n"
					"vst1.f32    {d4-d5}, [%1]!    \n"
					"vst1.f32    {d6-d7}, [%2]!    \n"
					"vst1.f32    {d8-d9}, [%3]!    \n"

					: "+r"(outptr0), // %0
					"+r"(outptr1), // %1
					"+r"(outptr2), // %2
					"+r"(outptr3), // %3
					"+r"(tmpptr),  // %4
					"+r"(kptr)     // %5
					: "r"(inch),     // %6  
					"r"(mem_scale + p),	// %7
					"r"(mem_bias[p]),		// %8
					"r"(mem_bias[p + 1]),	// %9
					"r"(mem_bias[p + 2]),	// %10
					"r"(mem_bias[p + 3])	// %11
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0_0 = 0;
				int sum0_1 = 0;
				int sum0_2 = 0;
				int sum0_3 = 0;

				int sum1_0 = 0;
				int sum1_1 = 0;
				int sum1_2 = 0;
				int sum1_3 = 0;

				int sum2_0 = 0;
				int sum2_1 = 0;
				int sum2_2 = 0;
				int sum2_3 = 0;

				int sum3_0 = 0;
				int sum3_1 = 0;
				int sum3_2 = 0;
				int sum3_3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0_0 += tmpptr[0] * kptr[0];
					sum0_1 += tmpptr[1] * kptr[0];
					sum0_2 += tmpptr[2] * kptr[0];
					sum0_3 += tmpptr[3] * kptr[0];

					sum1_0 += tmpptr[0] * kptr[1];
					sum1_1 += tmpptr[1] * kptr[1];
					sum1_2 += tmpptr[2] * kptr[1];
					sum1_3 += tmpptr[3] * kptr[1];

					sum2_0 += tmpptr[0] * kptr[2];
					sum2_1 += tmpptr[1] * kptr[2];
					sum2_2 += tmpptr[2] * kptr[2];
					sum2_3 += tmpptr[3] * kptr[2];

					sum3_0 += tmpptr[0] * kptr[3];
					sum3_1 += tmpptr[1] * kptr[3];
					sum3_2 += tmpptr[2] * kptr[3];
					sum3_3 += tmpptr[3] * kptr[3];

					tmpptr += 4;
					kptr += 4;
				}

				outptr0[0] = sum0_0 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[1] = sum0_1 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[2] = sum0_2 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr0[3] = sum0_3 * mem_scale[p + 0] + mem_bias[p + 0];

				outptr1[0] = sum1_0 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[1] = sum1_1 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[2] = sum1_2 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr1[3] = sum1_3 * mem_scale[p + 1] + mem_bias[p + 1];

				outptr2[0] = sum2_0 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[1] = sum2_1 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[2] = sum2_2 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr2[3] = sum2_3 * mem_scale[p + 2] + mem_bias[p + 2];

				outptr3[0] = sum3_0 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[1] = sum3_1 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[2] = sum3_2 * mem_scale[p + 3] + mem_bias[p + 3];
				outptr3[3] = sum3_3 * mem_scale[p + 3] + mem_bias[p + 3];

				outptr0 += 4;
				outptr1 += 4;
				outptr2 += 4;
				outptr3 += 4;
#endif // __ARM_NEON            
			}

			for (; i < size; i++)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4 + i % 4);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"veor        q6, q6, q6        \n"
					"veor        q7, q7, q7        \n"
					"veor        q8, q8, q8        \n"
					"veor        q9, q9, q9        \n"
					"vmov.s32    q10, #0           \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4}, [%4]        \n"// tmpr a00,a10,a20,a30    a(inch)(data)
					"add         %4, #4            \n"
					"vmovl.s8    q2, d4            \n"// a00,a10,a20,a30

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d0, d4[0]     \n"// (k00-k30) * a00
					"vmlal.s16   q7, d1, d4[1]     \n"// (k01-k31) * a10
					"vmlal.s16   q8, d2, d4[2]     \n"// (k02-k32) * a20
					"vmlal.s16   q9, d3, d4[3]     \n"// (k03-k33) * a30

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"vadd.s32    q6, q6, q7        \n"
					"vadd.s32    q9, q9, q8        \n"
					"vadd.s32    q10, q6, q9       \n"

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]        \n"// tmpr a00        a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %4, #1            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q10, d0, d2[0]    \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q10, q10         \n"

					"vld1.f32    {d0, d1}, [%7]    \n"
					"vld1.f32    {d2, d3}, [%8]    \n"

					"vmla.f32    q1, q10, q0       \n"

					"vst1.s32    {d2[0]}, [%0]!    \n"
					"vst1.s32    {d2[1]}, [%1]!    \n"
					"vst1.s32    {d3[0]}, [%2]!    \n"
					"vst1.s32    {d3[1]}, [%3]!    \n"

					: "+r"(outptr0), // %0
					"+r"(outptr1), // %1
					"+r"(outptr2), // %2
					"+r"(outptr3), // %3
					"+r"(tmpptr),  // %4
					"+r"(kptr)     // %5
					: "r"(inch),     // %6  
					"r"(mem_scale + p),	// %7
					"r"(mem_bias + p) 	// %8
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[0] * kptr[1];
					sum2 += tmpptr[0] * kptr[2];
					sum3 += tmpptr[0] * kptr[3];

					tmpptr++;
					kptr += 4;
				}

				outptr0[0] = sum0 * mem_scale[p + 0] + mem_bias[p + 0];
				outptr1[0] = sum1 * mem_scale[p + 1] + mem_bias[p + 1];
				outptr2[0] = sum2 * mem_scale[p + 2] + mem_bias[p + 2];
				outptr3[0] = sum3 * mem_scale[p + 3] + mem_bias[p + 3];

				outptr0++;
				outptr1++;
				outptr2++;
				outptr3++;
#endif // __ARM_NEON
			}
		}

		remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
		nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			float* outptr0 = data_out + p * size; // Mat out0 = top_blob.channel(p);

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				const signed char* tmpptr = mem_tmp + (i / 8) * tmp_cstep; // tmp.channel(i / 8);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"

					"lsr         r4, %3, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%1, #128]        \n"
					"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
					"vmovl.s8    q5, d7            \n"// a30-a37
					"vmovl.s8    q4, d6            \n"// a20-a27
					"vmovl.s8    q3, d5            \n"// a10-a17
					"vmovl.s8    q2, d4            \n"// a00-a07

					"vld1.s8     {d0}, [%2]        \n"// kptr k00,k01,k02,k03    k(outch)(inch)
					"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03
					"add         %2, #4            \n"

					"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a07) * k00
					"vmlal.s16   q7, d5, d0[0]     \n"
					"vmlal.s16   q6, d6, d0[1]     \n"// (a10-a17) * k01
					"vmlal.s16   q7, d7, d0[1]     \n"
					"vmlal.s16   q6, d8, d0[2]     \n"// (a20-a27) * k02
					"vmlal.s16   q7, d9, d0[2]     \n"
					"vmlal.s16   q6, d10, d0[3]    \n"// (a30-a37) * k03
					"vmlal.s16   q7, d11, d0[3]    \n"

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %3, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%1]!       \n"// tmpr a00-a07    a(inch)(data)
					"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %2, #1            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a07) * k00
					"vmlal.s16   q7, d3, d0[0]     \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vdup.f32    q0, %4            \n"

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"

					"vdup.f32    q1, %5            \n"
					"vmov.32     q2, q1            \n"

					"vmla.f32    q1, q6, q0        \n"
					"vmla.f32    q2, q7, q0        \n"

					"vst1.s32    {d2-d5}, [%0]!    \n"

					: "+r"(outptr0), // %0
					"+r"(tmpptr),  // %1
					"+r"(kptr)     // %2
					: "r"(inch),     // %3  
					"r"(mem_scale[p]), // %4
					"r"(mem_bias[p])   // %5
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;
				int sum4 = 0;
				int sum5 = 0;
				int sum6 = 0;
				int sum7 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[1] * kptr[0];
					sum2 += tmpptr[2] * kptr[0];
					sum3 += tmpptr[3] * kptr[0];
					sum4 += tmpptr[4] * kptr[0];
					sum5 += tmpptr[5] * kptr[0];
					sum6 += tmpptr[6] * kptr[0];
					sum7 += tmpptr[7] * kptr[0];

					tmpptr += 8;
					kptr++;
				}

				outptr0[0] = sum0 * mem_scale[p] + mem_bias[p];
				outptr0[1] = sum1 * mem_scale[p] + mem_bias[p];
				outptr0[2] = sum2 * mem_scale[p] + mem_bias[p];
				outptr0[3] = sum3 * mem_scale[p] + mem_bias[p];
				outptr0[4] = sum4 * mem_scale[p] + mem_bias[p];
				outptr0[5] = sum5 * mem_scale[p] + mem_bias[p];
				outptr0[6] = sum6 * mem_scale[p] + mem_bias[p];
				outptr0[7] = sum7 * mem_scale[p] + mem_bias[p];

				outptr0 += 8;
#endif // __ARM_NEON
			}

			for (; i + 3 < size; i += 4)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"

					"lsr         r4, %3, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%2, #128]        \n"
					"vld1.s8     {d4-d5}, [%1]!    \n"// tmpr a00-a03,a10-a13,a20-a23,a30-a33    a(inch)(data)
					"vmovl.s8    q3, d5            \n"// a20-a23,a30-a33
					"vmovl.s8    q2, d4            \n"// a00-a03,a10-a13

					"vld1.s8     {d0}, [%2]        \n"// kptr k00,k01,k02,k03    k(outch)(inch)
					"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03
					"add         %2, #4            \n"

					"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a03) * k00
					"vmlal.s16   q6, d5, d0[1]     \n"// (a10-a13) * k01
					"vmlal.s16   q6, d6, d0[2]     \n"// (a20-a23) * k02
					"vmlal.s16   q6, d7, d0[3]     \n"// (a30-a33) * k03

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %3, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%1]        \n"// tmpr a00-a03    a(inch)(data)
					"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %1, #4            \n"
					"add         %2, #1            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a03) * k00

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vdup.f32    q0, %4            \n"
					"vdup.f32    q1, %5            \n"

					"vcvt.f32.s32 q6, q6           \n"

					"vmla.f32    q1, q6, q0        \n"

					"vst1.s32    {d2-d3}, [%0]!    \n"

					: "+r"(outptr0), // %0
					"+r"(tmpptr),  // %1
					"+r"(kptr)     // %2
					: "r"(inch),     // %3  
					"r"(mem_scale[p]), // %4
					"r"(mem_bias[p])   // %5
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[1] * kptr[0];
					sum2 += tmpptr[2] * kptr[0];
					sum3 += tmpptr[3] * kptr[0];

					tmpptr += 4;
					kptr++;
				}

				outptr0[0] = sum0 * mem_scale[p] + mem_bias[p];
				outptr0[1] = sum1 * mem_scale[p] + mem_bias[p];
				outptr0[2] = sum2 * mem_scale[p] + mem_bias[p];
				outptr0[3] = sum3 * mem_scale[p] + mem_bias[p];

				outptr0 += 4;
#endif // __ARM_NEON
			}

			for (; i < size; i++)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4 + i % 4);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

				int q = 0;
				int sum0 = 0;

				for (; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					tmpptr++;
					kptr++;
				}

				outptr0[0] = sum0 * mem_scale[p] + mem_bias[p];

				outptr0++;
			}
		}
	}

	// static void conv1x1s1_sgemm_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& kernel, const Option& opt)
	// conv1x1s1_sgemm_transform_kernel_int8_neon
	// (8 * 4) * (inch / 4 + inch % 4) * (size / 8 + (size % 8) / 4 + size % 4)
	void conv_k1s1_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;
		int inch = ch_in;
		int outch = ch_out;

		const int size = w * h;

		int tmp_cstep = 32 * (inch / 4 + inch % 4);
		int kernel_cstep = 16 * (inch / 4 + inch % 4);

		// interleave
		// Mat tmp(8 * 4, inch / 4 + inch % 4, size / 8 + (size % 8) / 4 + size % 4, 1u, opt.workspace_allocator);
		signed char* mem_tmp = (signed char*)mem_tm;
		{
			int nn_size = size >> 3;
			int remain_size_start = nn_size << 3;

			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = ii * 8;

				const signed char* img0 = data_in + i; // bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + ii * tmp_cstep; // tmp.channel(i / 8);

				for (int q = 0; q < inch; q++)
				{
#if __ARM_NEON
					asm volatile(
						"pld        [%0, #64]     \n"
						"vld1.s8   {d0}, [%0]     \n"
						"vst1.s8   {d0}, [%1]!    \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "d0"
						);
					img0 += size;
#else                
					tmpptr[0] = img0[0];
					tmpptr[1] = img0[1];
					tmpptr[2] = img0[2];
					tmpptr[3] = img0[3];
					tmpptr[4] = img0[4];
					tmpptr[5] = img0[5];
					tmpptr[6] = img0[6];
					tmpptr[7] = img0[7];

					tmpptr += 8;
					img0 += size; // bottom_blob.cstep;
#endif // __ARM_NEON
				}
			}

			nn_size = (size - remain_size_start) >> 2;

			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = remain_size_start + ii * 4;

				const signed char* img0 = data_in + i; // bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);

				for (int q = 0; q < inch; q++)
				{
					tmpptr[0] = img0[0];
					tmpptr[1] = img0[1];
					tmpptr[2] = img0[2];
					tmpptr[3] = img0[3];

					tmpptr += 4;
					img0 += size; // bottom_blob.cstep;
				}
			}

			remain_size_start += nn_size << 2;

			for (int i = remain_size_start; i < size; i++)
			{
				const signed char* img0 = data_in + i; //  bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; //  tmp.channel(i / 8 + (i % 8) / 4 + i % 4);

				for (int q = 0; q < inch; q++)
				{
					tmpptr[0] = img0[0];
					tmpptr++;
					img0 += size; // bottom_blob.cstep;
				}
			}
		}

		// sgemm process
		int nn_outch = 0;
		int remain_outch_start = 0;

		nn_outch = (outch - remain_outch_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = remain_outch_start + pp * 4;

			signed char* outptr0 = data_out + p * size; // top_blob.channel(p);
			signed char* outptr1 = outptr0 + size; // top_blob.channel(p + 1);
			signed char* outptr2 = outptr1 + size; // top_blob.channel(p + 2);
			signed char* outptr3 = outptr2 + size; // top_blob.channel(p + 3);

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				const signed char* tmpptr = mem_tmp + (i / 8) * tmp_cstep; //  tmp.channel(i / 8);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"
					"vmov.s32    q8, #0            \n"
					"vmov.s32    q9, #0            \n"
					"vmov.s32    q10, #0           \n"
					"vmov.s32    q11, #0           \n"
					"vmov.s32    q12, #0           \n"
					"vmov.s32    q13, #0           \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4-d7}, [%4]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
					"vmovl.s8    q5, d7            \n"// a30-a37
					"vmovl.s8    q4, d6            \n"// a20-a27
					"vmovl.s8    q3, d5            \n"// a10-a17
					"vmovl.s8    q2, d4            \n"// a00-a07

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d4, d0[0]     \n"// sum0 = (a00-a07) * k00
					"vmlal.s16   q7, d5, d0[0]     \n"
					"vmlal.s16   q8, d4, d0[1]     \n"// sum1 = (a00-a07) * k10
					"vmlal.s16   q9, d5, d0[1]     \n"
					"vmlal.s16   q10, d4, d0[2]    \n"// sum2 = (a00-a07) * k20
					"vmlal.s16   q11, d5, d0[2]    \n"
					"vmlal.s16   q12, d4, d0[3]    \n"// sum3 = (a00-a07) * k30
					"vmlal.s16   q13, d5, d0[3]    \n"

					"vmlal.s16   q6, d6, d1[0]     \n"// sum0 += (a10-a17) * k01
					"vmlal.s16   q7, d7, d1[0]     \n"
					"vmlal.s16   q8, d6, d1[1]     \n"// sum1 += (a10-a17) * k11
					"vmlal.s16   q9, d7, d1[1]     \n"
					"vmlal.s16   q10, d6, d1[2]    \n"// sum2 += (a10-a17) * k21
					"vmlal.s16   q11, d7, d1[2]    \n"
					"vmlal.s16   q12, d6, d1[3]    \n"// sum3 += (a10-a17) * k31
					"vmlal.s16   q13, d7, d1[3]    \n"

					"vmlal.s16   q6, d8, d2[0]     \n"// sum0 += (a20-a27) * k02
					"vmlal.s16   q7, d9, d2[0]     \n"
					"vmlal.s16   q8, d8, d2[1]     \n"// sum1 += (a20-a27) * k12
					"vmlal.s16   q9, d9, d2[1]     \n"
					"vmlal.s16   q10, d8, d2[2]    \n"// sum2 += (a20-a27) * k22
					"vmlal.s16   q11, d9, d2[2]    \n"
					"vmlal.s16   q12, d8, d2[3]    \n"// sum3 += (a20-a27) * k32
					"vmlal.s16   q13, d9, d2[3]    \n"

					"vmlal.s16   q6, d10, d3[0]    \n"// sum0 += (a30-a37) * k03
					"vmlal.s16   q7, d11, d3[0]    \n"
					"vmlal.s16   q8, d10, d3[1]    \n"// sum1 += (a30-a37) * k13
					"vmlal.s16   q9, d11, d3[1]    \n"
					"vmlal.s16   q10, d10, d3[2]   \n"// sum2 += (a30-a37) * k23
					"vmlal.s16   q11, d11, d3[2]   \n"
					"vmlal.s16   q12, d10, d3[3]   \n"// sum3 += (a30-a37) * k33
					"vmlal.s16   q13, d11, d3[3]   \n"

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]!       \n"// tmpr a00-a07    a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// sum0 += (a00-a07) * k00
					"vmlal.s16   q7, d3, d0[0]     \n"
					"vmlal.s16   q8, d2, d0[1]     \n"// sum1 += (a00-a07) * k10
					"vmlal.s16   q9, d3, d0[1]     \n"
					"vmlal.s16   q10, d2, d0[2]    \n"// sum2 += (a00-a07) * k20
					"vmlal.s16   q11, d3, d0[2]    \n"
					"vmlal.s16   q12, d2, d0[3]    \n"// sum3 += (a00-a07) * k30
					"vmlal.s16   q13, d3, d0[3]    \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"
					"vcvt.f32.s32 q8, q8           \n"
					"vcvt.f32.s32 q9, q9           \n"

					"vld1.f32    {d0, d1}, [%7]    \n"

					"vcvt.f32.s32 q10, q10         \n"
					"vcvt.f32.s32 q11, q11         \n"
					"vcvt.f32.s32 q12, q12         \n"
					"vcvt.f32.s32 q13, q13         \n"

					"vdup.f32    q1, %8            \n"
					"vdup.f32    q3, %9            \n"
					"vmov.32     q2, q1            \n"
					"vmov.32     q4, q3            \n"

					"vmla.f32    q1, q6, d0[0]     \n"
					"vmla.f32    q2, q7, d0[0]     \n"
					"vmla.f32    q3, q8, d0[1]     \n"
					"vmla.f32    q4, q9, d0[1]     \n"

					"vdup.f32    q5, %10           \n"
					"vdup.f32    q7, %11           \n"
					"vmov.32     q6, q5            \n"
					"vmov.32     q8, q7            \n"

					"vmla.f32    q5, q10, d1[0]    \n"
					"vmla.f32    q6, q11, d1[0]    \n"
					"vmla.f32    q7, q12, d1[1]    \n"
					"vmla.f32    q8, q13, d1[1]    \n"

					"vcvt.s32.f32 q1, q1           \n"
					"vcvt.s32.f32 q2, q2           \n"
					"vcvt.s32.f32 q3, q3           \n"
					"vcvt.s32.f32 q4, q4           \n"

					"vqmovn.s32  d2, q1            \n"
					"vqmovn.s32  d3, q2            \n"
					"vqmovn.s32  d4, q3            \n"
					"vqmovn.s32  d5, q4            \n"

					"vcvt.s32.f32 q5, q5           \n"
					"vcvt.s32.f32 q6, q6           \n"
					"vcvt.s32.f32 q7, q7           \n"
					"vcvt.s32.f32 q8, q8           \n"

					"veor        q12, q12, q12     \n"

					"vqmovn.s32  d6, q5            \n"
					"vqmovn.s32  d7, q6            \n"
					"vqmovn.s32  d8, q7            \n"
					"vqmovn.s32  d9, q8            \n"

					"vqmovn.s16  d0, q1            \n"
					"vqmovn.s16  d1, q2            \n"
					"vqmovn.s16  d2, q3            \n"
					"vqmovn.s16  d3, q4            \n"

					"vmax.s8     q0, q0, q12       \n"
					"vmax.s8     q1, q1, q12       \n"

					"vst1.s8     {d0}, [%0]!       \n"
					"vst1.s8     {d1}, [%1]!       \n"
					"vst1.s8     {d2}, [%2]!       \n"
					"vst1.s8     {d3}, [%3]!       \n"

					: "+r"(outptr0),		// %0
					"+r"(outptr1),		// %1
					"+r"(outptr2),		// %2
					"+r"(outptr3),		// %3
					"+r"(tmpptr),			// %4
					"+r"(kptr)			// %5
					: "r"(inch),			// %6
					"r"(mem_scale + p),	// %7
					"r"(mem_bias[p]),		// %8
					"r"(mem_bias[p + 1]),	// %9
					"r"(mem_bias[p + 2]),	// %10
					"r"(mem_bias[p + 3])	// %11
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0_0 = 0;
				int sum0_1 = 0;
				int sum0_2 = 0;
				int sum0_3 = 0;
				int sum0_4 = 0;
				int sum0_5 = 0;
				int sum0_6 = 0;
				int sum0_7 = 0;

				int sum1_0 = 0;
				int sum1_1 = 0;
				int sum1_2 = 0;
				int sum1_3 = 0;
				int sum1_4 = 0;
				int sum1_5 = 0;
				int sum1_6 = 0;
				int sum1_7 = 0;

				int sum2_0 = 0;
				int sum2_1 = 0;
				int sum2_2 = 0;
				int sum2_3 = 0;
				int sum2_4 = 0;
				int sum2_5 = 0;
				int sum2_6 = 0;
				int sum2_7 = 0;

				int sum3_0 = 0;
				int sum3_1 = 0;
				int sum3_2 = 0;
				int sum3_3 = 0;
				int sum3_4 = 0;
				int sum3_5 = 0;
				int sum3_6 = 0;
				int sum3_7 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0_0 += tmpptr[0] * kptr[0];
					sum0_1 += tmpptr[1] * kptr[0];
					sum0_2 += tmpptr[2] * kptr[0];
					sum0_3 += tmpptr[3] * kptr[0];
					sum0_4 += tmpptr[4] * kptr[0];
					sum0_5 += tmpptr[5] * kptr[0];
					sum0_6 += tmpptr[6] * kptr[0];
					sum0_7 += tmpptr[7] * kptr[0];

					sum1_0 += tmpptr[0] * kptr[1];
					sum1_1 += tmpptr[1] * kptr[1];
					sum1_2 += tmpptr[2] * kptr[1];
					sum1_3 += tmpptr[3] * kptr[1];
					sum1_4 += tmpptr[4] * kptr[1];
					sum1_5 += tmpptr[5] * kptr[1];
					sum1_6 += tmpptr[6] * kptr[1];
					sum1_7 += tmpptr[7] * kptr[1];

					sum2_0 += tmpptr[0] * kptr[2];
					sum2_1 += tmpptr[1] * kptr[2];
					sum2_2 += tmpptr[2] * kptr[2];
					sum2_3 += tmpptr[3] * kptr[2];
					sum2_4 += tmpptr[4] * kptr[2];
					sum2_5 += tmpptr[5] * kptr[2];
					sum2_6 += tmpptr[6] * kptr[2];
					sum2_7 += tmpptr[7] * kptr[2];

					sum3_0 += tmpptr[0] * kptr[3];
					sum3_1 += tmpptr[1] * kptr[3];
					sum3_2 += tmpptr[2] * kptr[3];
					sum3_3 += tmpptr[3] * kptr[3];
					sum3_4 += tmpptr[4] * kptr[3];
					sum3_5 += tmpptr[5] * kptr[3];
					sum3_6 += tmpptr[6] * kptr[3];
					sum3_7 += tmpptr[7] * kptr[3];

					tmpptr += 8;
					kptr += 4;
				}

				int val;
				val = (int)(sum0_0 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[0] = (signed char)val;
				val = (int)(sum0_1 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[1] = (signed char)val;
				val = (int)(sum0_2 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[2] = (signed char)val;
				val = (int)(sum0_3 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[3] = (signed char)val;
				val = (int)(sum0_4 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[4] = (signed char)val;
				val = (int)(sum0_5 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[5] = (signed char)val;
				val = (int)(sum0_6 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[6] = (signed char)val;
				val = (int)(sum0_7 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[7] = (signed char)val;

				val = (int)(sum1_0 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[0] = (signed char)val;
				val = (int)(sum1_1 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[1] = (signed char)val;
				val = (int)(sum1_2 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[2] = (signed char)val;
				val = (int)(sum1_3 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[3] = (signed char)val;
				val = (int)(sum1_4 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[4] = (signed char)val;
				val = (int)(sum1_5 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[5] = (signed char)val;
				val = (int)(sum1_6 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[6] = (signed char)val;
				val = (int)(sum1_7 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[7] = (signed char)val;

				val = (int)(sum2_0 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[0] = (signed char)val;
				val = (int)(sum2_1 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[1] = (signed char)val;
				val = (int)(sum2_2 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[2] = (signed char)val;
				val = (int)(sum2_3 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[3] = (signed char)val;
				val = (int)(sum2_4 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[4] = (signed char)val;
				val = (int)(sum2_5 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[5] = (signed char)val;
				val = (int)(sum2_6 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[6] = (signed char)val;
				val = (int)(sum2_7 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[7] = (signed char)val;

				val = (int)(sum3_0 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[0] = (signed char)val;
				val = (int)(sum3_1 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[1] = (signed char)val;
				val = (int)(sum3_2 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[2] = (signed char)val;
				val = (int)(sum3_3 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[3] = (signed char)val;
				val = (int)(sum3_4 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[4] = (signed char)val;
				val = (int)(sum3_5 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[5] = (signed char)val;
				val = (int)(sum3_6 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[6] = (signed char)val;
				val = (int)(sum3_7 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[7] = (signed char)val;

				outptr0 += 8;
				outptr1 += 8;
				outptr2 += 8;
				outptr3 += 8;
#endif // __ARM_NEON            
			}

			for (; i + 3 < size; i += 4)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"
					"vmov.s32    q8, #0            \n"
					"vmov.s32    q9, #0            \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4-d5}, [%4]!    \n"// tmpr a00-a03,a10-a13,a20-a23,a30-a33    a(inch)(data)
					"vmovl.s8    q3, d5            \n"// a20-a23,a30-a33
					"vmovl.s8    q2, d4            \n"// a00-a04,a10-a14

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d4, d0[0]     \n"// sum0 = (a00-a03) * k00
					"vmlal.s16   q7, d4, d0[1]     \n"// sum1 = (a00-a03) * k10
					"vmlal.s16   q8, d4, d0[2]     \n"// sum2 = (a00-a03) * k20
					"vmlal.s16   q9, d4, d0[3]     \n"// sum3 = (a00-a03) * k30

					"vmlal.s16   q6, d5, d1[0]     \n"// sum0 += (a10-a13) * k01
					"vmlal.s16   q7, d5, d1[1]     \n"// sum1 += (a10-a13) * k11
					"vmlal.s16   q8, d5, d1[2]     \n"// sum2 += (a10-a13) * k21
					"vmlal.s16   q9, d5, d1[3]     \n"// sum3 += (a10-a13) * k31

					"vmlal.s16   q6, d6, d2[0]     \n"// sum0 += (a20-a23) * k02
					"vmlal.s16   q7, d6, d2[1]     \n"// sum1 += (a20-a23) * k12
					"vmlal.s16   q8, d6, d2[2]     \n"// sum2 += (a20-a23) * k22
					"vmlal.s16   q9, d6, d2[3]     \n"// sum3 += (a20-a23) * k32

					"vmlal.s16   q6, d7, d3[0]     \n"// sum0 += (a30-a33) * k03
					"vmlal.s16   q7, d7, d3[1]     \n"// sum1 += (a30-a33) * k13
					"vmlal.s16   q8, d7, d3[2]     \n"// sum2 += (a30-a33) * k23
					"vmlal.s16   q9, d7, d3[3]     \n"// sum3 += (a30-a33) * k33

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]        \n"// tmpr a00-a03    a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %4, #4            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// sum0 += (a00-a03) * k00
					"vmlal.s16   q7, d2, d0[1]     \n"// sum1 += (a00-a03) * k10
					"vmlal.s16   q8, d2, d0[2]     \n"// sum2 += (a00-a03) * k20
					"vmlal.s16   q9, d2, d0[3]     \n"// sum3 += (a00-a03) * k30

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"
					"vcvt.f32.s32 q8, q8           \n"
					"vcvt.f32.s32 q9, q9           \n"

					"vld1.f32    {d0, d1}, [%7]    \n"

					"vdup.f32    q1, %8            \n"
					"vdup.f32    q2, %9            \n"
					"vdup.f32    q3, %10           \n"
					"vdup.f32    q4, %11           \n"

					"vmla.f32    q1, q6, d0[0]     \n"
					"vmla.f32    q2, q7, d0[1]     \n"
					"vmla.f32    q3, q8, d1[0]     \n"
					"vmla.f32    q4, q9, d1[1]     \n"

					"vcvt.s32.f32 q1, q1           \n"
					"vcvt.s32.f32 q2, q2           \n"
					"vcvt.s32.f32 q3, q3           \n"
					"vcvt.s32.f32 q4, q4           \n"

					"vqmovn.s32  d2, q1            \n"
					"vqmovn.s32  d3, q2            \n"
					"vqmovn.s32  d4, q3            \n"
					"vqmovn.s32  d5, q4            \n"

					"veor        q12, q12, q12     \n"

					"vqmovn.s16  d0, q1            \n"
					"vqmovn.s16  d1, q2            \n"

					"vmax.s8     q0, q0, q12       \n"

					"vst1.s32   {d0[0]}, [%0]!    \n"
					"vst1.s32   {d0[1]}, [%1]!    \n"
					"vst1.s32   {d1[0]}, [%2]!    \n"
					"vst1.s32   {d1[1]}, [%3]!    \n"

					: "+r"(outptr0), // %0
					"+r"(outptr1), // %1
					"+r"(outptr2), // %2
					"+r"(outptr3), // %3
					"+r"(tmpptr),  // %4
					"+r"(kptr)     // %5
					: "r"(inch),     // %6  
					"r"(mem_scale + p),	// %7
					"r"(mem_bias[p]),		// %8
					"r"(mem_bias[p + 1]),	// %9
					"r"(mem_bias[p + 2]),	// %10
					"r"(mem_bias[p + 3])	// %11
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0_0 = 0;
				int sum0_1 = 0;
				int sum0_2 = 0;
				int sum0_3 = 0;

				int sum1_0 = 0;
				int sum1_1 = 0;
				int sum1_2 = 0;
				int sum1_3 = 0;

				int sum2_0 = 0;
				int sum2_1 = 0;
				int sum2_2 = 0;
				int sum2_3 = 0;

				int sum3_0 = 0;
				int sum3_1 = 0;
				int sum3_2 = 0;
				int sum3_3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0_0 += tmpptr[0] * kptr[0];
					sum0_1 += tmpptr[1] * kptr[0];
					sum0_2 += tmpptr[2] * kptr[0];
					sum0_3 += tmpptr[3] * kptr[0];

					sum1_0 += tmpptr[0] * kptr[1];
					sum1_1 += tmpptr[1] * kptr[1];
					sum1_2 += tmpptr[2] * kptr[1];
					sum1_3 += tmpptr[3] * kptr[1];

					sum2_0 += tmpptr[0] * kptr[2];
					sum2_1 += tmpptr[1] * kptr[2];
					sum2_2 += tmpptr[2] * kptr[2];
					sum2_3 += tmpptr[3] * kptr[2];

					sum3_0 += tmpptr[0] * kptr[3];
					sum3_1 += tmpptr[1] * kptr[3];
					sum3_2 += tmpptr[2] * kptr[3];
					sum3_3 += tmpptr[3] * kptr[3];

					tmpptr += 4;
					kptr += 4;
				}

				int val;
				val = (int)(sum0_0 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[0] = (signed char)val;
				val = (int)(sum0_1 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[1] = (signed char)val;
				val = (int)(sum0_2 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[2] = (signed char)val;
				val = (int)(sum0_3 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[3] = (signed char)val;

				val = (int)(sum1_0 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[0] = (signed char)val;
				val = (int)(sum1_1 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[1] = (signed char)val;
				val = (int)(sum1_2 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[2] = (signed char)val;
				val = (int)(sum1_3 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[3] = (signed char)val;

				val = (int)(sum2_0 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[0] = (signed char)val;
				val = (int)(sum2_1 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[1] = (signed char)val;
				val = (int)(sum2_2 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[2] = (signed char)val;
				val = (int)(sum2_3 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[3] = (signed char)val;

				val = (int)(sum3_0 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[0] = (signed char)val;
				val = (int)(sum3_1 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[1] = (signed char)val;
				val = (int)(sum3_2 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[2] = (signed char)val;
				val = (int)(sum3_3 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[3] = (signed char)val;

				outptr0 += 4;
				outptr1 += 4;
				outptr2 += 4;
				outptr3 += 4;
#endif // __ARM_NEON            
			}

			for (; i < size; i++)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4 + i % 4);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"veor        q6, q6, q6        \n"
					"veor        q7, q7, q7        \n"
					"veor        q8, q8, q8        \n"
					"veor        q9, q9, q9        \n"
					"vmov.s32    q10, #0           \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4}, [%4]        \n"// tmpr a00,a10,a20,a30    a(inch)(data)
					"add         %4, #4            \n"
					"vmovl.s8    q2, d4            \n"// a00,a10,a20,a30

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d0, d4[0]     \n"// (k00-k30) * a00
					"vmlal.s16   q7, d1, d4[1]     \n"// (k01-k31) * a10
					"vmlal.s16   q8, d2, d4[2]     \n"// (k02-k32) * a20
					"vmlal.s16   q9, d3, d4[3]     \n"// (k03-k33) * a30

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"vadd.s32    q6, q6, q7        \n"
					"vadd.s32    q9, q9, q8        \n"
					"vadd.s32    q10, q6, q9       \n"

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]        \n"// tmpr a00        a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %4, #1            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q10, d0, d2[0]    \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q10, q10         \n"

					"vld1.f32    {d0, d1}, [%7]    \n"
					"vld1.f32    {d2, d3}, [%8]    \n"

					"vmla.f32    q1, q10, q0       \n"

					"veor        d4, d4, d4        \n"

					"vcvt.s32.f32 q1, q1           \n"

					"vqmovn.s32   d0, q1           \n"
					"vqmovn.s16   d0, q0           \n"
					"vmax.s8     d0, d0, d4        \n"

					"vst1.s8    {d0[0]}, [%0]!     \n"
					"vst1.s8    {d0[1]}, [%1]!     \n"
					"vst1.s8    {d0[2]}, [%2]!     \n"
					"vst1.s8    {d0[3]}, [%3]!     \n"

					: "+r"(outptr0), // %0
					"+r"(outptr1), // %1
					"+r"(outptr2), // %2
					"+r"(outptr3), // %3
					"+r"(tmpptr),  // %4
					"+r"(kptr)     // %5
					: "r"(inch),     // %6  
					"r"(mem_scale + p),	// %7
					"r"(mem_bias + p) 	// %8
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[0] * kptr[1];
					sum2 += tmpptr[0] * kptr[2];
					sum3 += tmpptr[0] * kptr[3];

					tmpptr++;
					kptr += 4;
				}

				int val;
				val = (int)(sum0 * mem_scale[p + 0] + mem_bias[p + 0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[0] = (signed char)val;
				val = (int)(sum1 * mem_scale[p + 1] + mem_bias[p + 1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[0] = (signed char)val;
				val = (int)(sum2 * mem_scale[p + 2] + mem_bias[p + 2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr2[0] = (signed char)val;
				val = (int)(sum3 * mem_scale[p + 3] + mem_bias[p + 3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr3[0] = (signed char)val;

				outptr0++;
				outptr1++;
				outptr2++;
				outptr3++;
#endif // __ARM_NEON
			}
		}

		remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
		nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			signed char* outptr0 = data_out + p * size; // Mat out0 = top_blob.channel(p);

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				const signed char* tmpptr = mem_tmp + (i / 8) * tmp_cstep; // tmp.channel(i / 8);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"

					"lsr         r4, %3, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%1, #128]        \n"
					"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
					"vmovl.s8    q5, d7            \n"// a30-a37
					"vmovl.s8    q4, d6            \n"// a20-a27
					"vmovl.s8    q3, d5            \n"// a10-a17
					"vmovl.s8    q2, d4            \n"// a00-a07

					"vld1.s8     {d0}, [%2]        \n"// kptr k00,k01,k02,k03    k(outch)(inch)
					"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03
					"add         %2, #4            \n"

					"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a07) * k00
					"vmlal.s16   q7, d5, d0[0]     \n"
					"vmlal.s16   q6, d6, d0[1]     \n"// (a10-a17) * k01
					"vmlal.s16   q7, d7, d0[1]     \n"
					"vmlal.s16   q6, d8, d0[2]     \n"// (a20-a27) * k02
					"vmlal.s16   q7, d9, d0[2]     \n"
					"vmlal.s16   q6, d10, d0[3]    \n"// (a30-a37) * k03
					"vmlal.s16   q7, d11, d0[3]    \n"

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %3, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%1]!       \n"// tmpr a00-a07    a(inch)(data)
					"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %2, #1            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a07) * k00
					"vmlal.s16   q7, d3, d0[0]     \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vdup.f32    q0, %4            \n"

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"

					"vdup.f32    q1, %5            \n"
					"vmov.32     q2, q1            \n"

					"vmla.f32    q1, q6, q0        \n"
					"vmla.f32    q2, q7, q0        \n"

					"vcvt.s32.f32 q1, q1           \n"
					"vcvt.s32.f32 q2, q2           \n"

					"vqmovn.s32 d0, q1             \n"
					"vqmovn.s32 d1, q2             \n"

					"veor       d2, d2, d2         \n"
					"vqmovn.s16 d0, q0             \n"
					"vmax.s8    d0, d0, d2         \n"

					"vst1.s32    {d0}, [%0]!       \n"

					: "+r"(outptr0), // %0
					"+r"(tmpptr),  // %1
					"+r"(kptr)     // %2
					: "r"(inch),     // %3  
					"r"(mem_scale[p]), // %4
					"r"(mem_bias[p])   // %5
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;
				int sum4 = 0;
				int sum5 = 0;
				int sum6 = 0;
				int sum7 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[1] * kptr[0];
					sum2 += tmpptr[2] * kptr[0];
					sum3 += tmpptr[3] * kptr[0];
					sum4 += tmpptr[4] * kptr[0];
					sum5 += tmpptr[5] * kptr[0];
					sum6 += tmpptr[6] * kptr[0];
					sum7 += tmpptr[7] * kptr[0];

					tmpptr += 8;
					kptr++;
				}

				int val;
				val = (int)(sum0 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[0] = (signed char)val;
				val = (int)(sum1 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[1] = (signed char)val;
				val = (int)(sum2 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[2] = (signed char)val;
				val = (int)(sum3 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[3] = (signed char)val;
				val = (int)(sum4 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[4] = (signed char)val;
				val = (int)(sum5 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[5] = (signed char)val;
				val = (int)(sum6 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[6] = (signed char)val;
				val = (int)(sum7 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[7] = (signed char)val;

				outptr0 += 8;
#endif // __ARM_NEON
			}

			for (; i + 3 < size; i += 4)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"

					"lsr         r4, %3, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%2, #128]        \n"
					"vld1.s8     {d4-d5}, [%1]!    \n"// tmpr a00-a03,a10-a13,a20-a23,a30-a33    a(inch)(data)
					"vmovl.s8    q3, d5            \n"// a20-a23,a30-a33
					"vmovl.s8    q2, d4            \n"// a00-a03,a10-a13

					"vld1.s8     {d0}, [%2]        \n"// kptr k00,k01,k02,k03    k(outch)(inch)
					"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03
					"add         %2, #4            \n"

					"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a03) * k00
					"vmlal.s16   q6, d5, d0[1]     \n"// (a10-a13) * k01
					"vmlal.s16   q6, d6, d0[2]     \n"// (a20-a23) * k02
					"vmlal.s16   q6, d7, d0[3]     \n"// (a30-a33) * k03

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %3, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%1]        \n"// tmpr a00-a03    a(inch)(data)
					"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %1, #4            \n"
					"add         %2, #1            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a03) * k00

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vdup.f32    q0, %4            \n"
					"vdup.f32    q1, %5            \n"

					"vcvt.f32.s32 q6, q6           \n"

					"vmla.f32    q1, q6, q0        \n"

					"vcvt.s32.f32 q1, q1           \n"
					"vqmovn.s32  d0, q1            \n"
					"veor       d2, d2, d2         \n"
					"vqmovn.s16  d0, q0            \n"

					"vmax.s8    d0, d0, d2         \n"

					"vst1.s32    {d0[0]}, [%0]!    \n"

					: "+r"(outptr0), // %0
					"+r"(tmpptr),  // %1
					"+r"(kptr)     // %2
					: "r"(inch),     // %3  
					"r"(mem_scale[p]), // %4
					"r"(mem_bias[p])   // %5
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[1] * kptr[0];
					sum2 += tmpptr[2] * kptr[0];
					sum3 += tmpptr[3] * kptr[0];

					tmpptr += 4;
					kptr++;
				}

				int val;
				val = (int)(sum0 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[0] = (signed char)val;
				val = (int)(sum1 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[1] = (signed char)val;
				val = (int)(sum2 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[2] = (signed char)val;
				val = (int)(sum3 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[3] = (signed char)val;

				outptr0 += 4;
#endif // __ARM_NEON
			}

			for (; i < size; i++)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4 + i % 4);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

				int q = 0;
				int sum0 = 0;

				for (; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					tmpptr++;
					kptr++;
				}

				int val;
				val = (int)(sum0 * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[0] = (signed char)val;

				outptr0++;
			}
		}
	}

	// static void conv1x1s1_sgemm_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& kernel, const Option& opt)
	// conv1x1s1_sgemm_transform_kernel_int8_neon
	// (8 * 4) * (inch / 4 + inch % 4) * (size / 8 + (size % 8) / 4 + size % 4)
	void conv_k1s1_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;
		int inch = ch_in;
		int outch = ch_out;

		const int size = w * h;

		int tmp_cstep = 32 * (inch / 4 + inch % 4);
		int kernel_cstep = 16 * (inch / 4 + inch % 4);

		// interleave
		// Mat tmp(8 * 4, inch / 4 + inch % 4, size / 8 + (size % 8) / 4 + size % 4, 1u, opt.workspace_allocator);
		signed char* mem_tmp = (signed char*)mem_tm;
		{
			int nn_size = size >> 3;
			int remain_size_start = nn_size << 3;

			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = ii * 8;

				const signed char* img0 = data_in + i; // bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + ii * tmp_cstep; // tmp.channel(i / 8);

				for (int q = 0; q < inch; q++)
				{
#if __ARM_NEON
					asm volatile(
						"pld        [%0, #64]     \n"
						"vld1.s8   {d0}, [%0]     \n"
						"vst1.s8   {d0}, [%1]!    \n"
						: "=r"(img0),   // %0
						"=r"(tmpptr)  // %1
						: "0"(img0),
						"1"(tmpptr)
						: "memory", "d0"
						);
					img0 += size;
#else                
					tmpptr[0] = img0[0];
					tmpptr[1] = img0[1];
					tmpptr[2] = img0[2];
					tmpptr[3] = img0[3];
					tmpptr[4] = img0[4];
					tmpptr[5] = img0[5];
					tmpptr[6] = img0[6];
					tmpptr[7] = img0[7];

					tmpptr += 8;
					img0 += size; // bottom_blob.cstep;
#endif // __ARM_NEON
				}
			}

			nn_size = (size - remain_size_start) >> 2;

			for (int ii = 0; ii < nn_size; ii++)
			{
				int i = remain_size_start + ii * 4;

				const signed char* img0 = data_in + i; // bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);

				for (int q = 0; q < inch; q++)
				{
					tmpptr[0] = img0[0];
					tmpptr[1] = img0[1];
					tmpptr[2] = img0[2];
					tmpptr[3] = img0[3];

					tmpptr += 4;
					img0 += size; // bottom_blob.cstep;
				}
			}

			remain_size_start += nn_size << 2;

			for (int i = remain_size_start; i < size; i++)
			{
				const signed char* img0 = data_in + i; //  bottom_blob.channel(0); img0 += i;

				signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; //  tmp.channel(i / 8 + (i % 8) / 4 + i % 4);

				for (int q = 0; q < inch; q++)
				{
					tmpptr[0] = img0[0];
					tmpptr++;
					img0 += size; // bottom_blob.cstep;
				}
			}
		}

		// sgemm process
		int nn_outch = 0;
		int remain_outch_start = 0;

		nn_outch = (outch - remain_outch_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = remain_outch_start + pp * 4;

			signed char* outptr0 = data_out + p * size; // top_blob.channel(p);
			signed char* outptr1 = outptr0 + size; // top_blob.channel(p + 1);
			signed char* outptr2 = outptr1 + size; // top_blob.channel(p + 2);
			signed char* outptr3 = outptr2 + size; // top_blob.channel(p + 3);

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				const signed char* tmpptr = mem_tmp + (i / 8) * tmp_cstep; //  tmp.channel(i / 8);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"
					"vmov.s32    q8, #0            \n"
					"vmov.s32    q9, #0            \n"
					"vmov.s32    q10, #0           \n"
					"vmov.s32    q11, #0           \n"
					"vmov.s32    q12, #0           \n"
					"vmov.s32    q13, #0           \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4-d7}, [%4]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
					"vmovl.s8    q5, d7            \n"// a30-a37
					"vmovl.s8    q4, d6            \n"// a20-a27
					"vmovl.s8    q3, d5            \n"// a10-a17
					"vmovl.s8    q2, d4            \n"// a00-a07

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d4, d0[0]     \n"// sum0 = (a00-a07) * k00
					"vmlal.s16   q7, d5, d0[0]     \n"
					"vmlal.s16   q8, d4, d0[1]     \n"// sum1 = (a00-a07) * k10
					"vmlal.s16   q9, d5, d0[1]     \n"
					"vmlal.s16   q10, d4, d0[2]    \n"// sum2 = (a00-a07) * k20
					"vmlal.s16   q11, d5, d0[2]    \n"
					"vmlal.s16   q12, d4, d0[3]    \n"// sum3 = (a00-a07) * k30
					"vmlal.s16   q13, d5, d0[3]    \n"

					"vmlal.s16   q6, d6, d1[0]     \n"// sum0 += (a10-a17) * k01
					"vmlal.s16   q7, d7, d1[0]     \n"
					"vmlal.s16   q8, d6, d1[1]     \n"// sum1 += (a10-a17) * k11
					"vmlal.s16   q9, d7, d1[1]     \n"
					"vmlal.s16   q10, d6, d1[2]    \n"// sum2 += (a10-a17) * k21
					"vmlal.s16   q11, d7, d1[2]    \n"
					"vmlal.s16   q12, d6, d1[3]    \n"// sum3 += (a10-a17) * k31
					"vmlal.s16   q13, d7, d1[3]    \n"

					"vmlal.s16   q6, d8, d2[0]     \n"// sum0 += (a20-a27) * k02
					"vmlal.s16   q7, d9, d2[0]     \n"
					"vmlal.s16   q8, d8, d2[1]     \n"// sum1 += (a20-a27) * k12
					"vmlal.s16   q9, d9, d2[1]     \n"
					"vmlal.s16   q10, d8, d2[2]    \n"// sum2 += (a20-a27) * k22
					"vmlal.s16   q11, d9, d2[2]    \n"
					"vmlal.s16   q12, d8, d2[3]    \n"// sum3 += (a20-a27) * k32
					"vmlal.s16   q13, d9, d2[3]    \n"

					"vmlal.s16   q6, d10, d3[0]    \n"// sum0 += (a30-a37) * k03
					"vmlal.s16   q7, d11, d3[0]    \n"
					"vmlal.s16   q8, d10, d3[1]    \n"// sum1 += (a30-a37) * k13
					"vmlal.s16   q9, d11, d3[1]    \n"
					"vmlal.s16   q10, d10, d3[2]   \n"// sum2 += (a30-a37) * k23
					"vmlal.s16   q11, d11, d3[2]   \n"
					"vmlal.s16   q12, d10, d3[3]   \n"// sum3 += (a30-a37) * k33
					"vmlal.s16   q13, d11, d3[3]   \n"

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]!       \n"// tmpr a00-a07    a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// sum0 += (a00-a07) * k00
					"vmlal.s16   q7, d3, d0[0]     \n"
					"vmlal.s16   q8, d2, d0[1]     \n"// sum1 += (a00-a07) * k10
					"vmlal.s16   q9, d3, d0[1]     \n"
					"vmlal.s16   q10, d2, d0[2]    \n"// sum2 += (a00-a07) * k20
					"vmlal.s16   q11, d3, d0[2]    \n"
					"vmlal.s16   q12, d2, d0[3]    \n"// sum3 += (a00-a07) * k30
					"vmlal.s16   q13, d3, d0[3]    \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"
					"vcvt.f32.s32 q8, q8           \n"
					"vcvt.f32.s32 q9, q9           \n"

					"vld1.f32    {d0, d1}, [%7]    \n"

					"vcvt.f32.s32 q10, q10         \n"
					"vcvt.f32.s32 q11, q11         \n"
					"vcvt.f32.s32 q12, q12         \n"
					"vcvt.f32.s32 q13, q13         \n"

					"vdup.f32    q1, %8            \n"
					"vdup.f32    q3, %9            \n"
					"vmov.32     q2, q1            \n"
					"vmov.32     q4, q3            \n"

					"vmla.f32    q1, q6, d0[0]     \n"
					"vmla.f32    q2, q7, d0[0]     \n"
					"vmla.f32    q3, q8, d0[1]     \n"
					"vmla.f32    q4, q9, d0[1]     \n"

					"vdup.f32    q5, %10           \n"
					"vdup.f32    q7, %11           \n"
					"vmov.32     q6, q5            \n"
					"vmov.32     q8, q7            \n"

					"vmla.f32    q5, q10, d1[0]    \n"
					"vmla.f32    q6, q11, d1[0]    \n"
					"vmla.f32    q7, q12, d1[1]    \n"
					"vmla.f32    q8, q13, d1[1]    \n"

					"vcvtr.s32.f32 s4, s4          \n"
					"vcvtr.s32.f32 s5, s5          \n"
					"vcvtr.s32.f32 s6, s6          \n"
					"vcvtr.s32.f32 s7, s7          \n"

					"vcvtr.s32.f32 s8, s8          \n"
					"vcvtr.s32.f32 s9, s9          \n"
					"vcvtr.s32.f32 s10, s10        \n"
					"vcvtr.s32.f32 s11, s11        \n"

					"vcvtr.s32.f32 s12, s12        \n"
					"vcvtr.s32.f32 s13, s13        \n"
					"vcvtr.s32.f32 s14, s14        \n"
					"vcvtr.s32.f32 s15, s15        \n"

					"vcvtr.s32.f32 s16, s16        \n"
					"vcvtr.s32.f32 s17, s17        \n"
					"vcvtr.s32.f32 s18, s18        \n"
					"vcvtr.s32.f32 s19, s19        \n"

					"vqmovn.s32  d2, q1            \n"
					"vqmovn.s32  d3, q2            \n"
					"vqmovn.s32  d4, q3            \n"
					"vqmovn.s32  d5, q4            \n"

					"vcvtr.s32.f32 s20, s20        \n"
					"vcvtr.s32.f32 s21, s21        \n"
					"vcvtr.s32.f32 s22, s22        \n"
					"vcvtr.s32.f32 s23, s23        \n"

					"vcvtr.s32.f32 s24, s24        \n"
					"vcvtr.s32.f32 s25, s25        \n"
					"vcvtr.s32.f32 s26, s26        \n"
					"vcvtr.s32.f32 s27, s27        \n"

					"vcvtr.s32.f32 s28, s28        \n"
					"vcvtr.s32.f32 s29, s29        \n"
					"vcvtr.s32.f32 s30, s30        \n"
					"vcvtr.s32.f32 s31, s31        \n"

					"vmov.32  q3, q8               \n"

					"vcvtr.s32.f32 s12, s12        \n"
					"vcvtr.s32.f32 s13, s13        \n"
					"vcvtr.s32.f32 s14, s14        \n"
					"vcvtr.s32.f32 s15, s15        \n"

					"vmov.32 q8, q3                \n"

					"vqmovn.s32  d6, q5            \n"
					"vqmovn.s32  d7, q6            \n"
					"vqmovn.s32  d8, q7            \n"
					"vqmovn.s32  d9, q8            \n"

					"vqmovn.s16  d0, q1            \n"
					"vqmovn.s16  d1, q2            \n"
					"vqmovn.s16  d2, q3            \n"
					"vqmovn.s16  d3, q4            \n"

					"vst1.s8     {d0}, [%0]!       \n"
					"vst1.s8     {d1}, [%1]!       \n"
					"vst1.s8     {d2}, [%2]!       \n"
					"vst1.s8     {d3}, [%3]!       \n"

					: "+r"(outptr0),		// %0
					"+r"(outptr1),		// %1
					"+r"(outptr2),		// %2
					"+r"(outptr3),		// %3
					"+r"(tmpptr),			// %4
					"+r"(kptr)			// %5
					: "r"(inch),			// %6
					"r"(mem_scale + p),	// %7
					"r"(mem_bias[p]),		// %8
					"r"(mem_bias[p + 1]),	// %9
					"r"(mem_bias[p + 2]),	// %10
					"r"(mem_bias[p + 3])	// %11
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0_0 = 0;
				int sum0_1 = 0;
				int sum0_2 = 0;
				int sum0_3 = 0;
				int sum0_4 = 0;
				int sum0_5 = 0;
				int sum0_6 = 0;
				int sum0_7 = 0;

				int sum1_0 = 0;
				int sum1_1 = 0;
				int sum1_2 = 0;
				int sum1_3 = 0;
				int sum1_4 = 0;
				int sum1_5 = 0;
				int sum1_6 = 0;
				int sum1_7 = 0;

				int sum2_0 = 0;
				int sum2_1 = 0;
				int sum2_2 = 0;
				int sum2_3 = 0;
				int sum2_4 = 0;
				int sum2_5 = 0;
				int sum2_6 = 0;
				int sum2_7 = 0;

				int sum3_0 = 0;
				int sum3_1 = 0;
				int sum3_2 = 0;
				int sum3_3 = 0;
				int sum3_4 = 0;
				int sum3_5 = 0;
				int sum3_6 = 0;
				int sum3_7 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0_0 += tmpptr[0] * kptr[0];
					sum0_1 += tmpptr[1] * kptr[0];
					sum0_2 += tmpptr[2] * kptr[0];
					sum0_3 += tmpptr[3] * kptr[0];
					sum0_4 += tmpptr[4] * kptr[0];
					sum0_5 += tmpptr[5] * kptr[0];
					sum0_6 += tmpptr[6] * kptr[0];
					sum0_7 += tmpptr[7] * kptr[0];

					sum1_0 += tmpptr[0] * kptr[1];
					sum1_1 += tmpptr[1] * kptr[1];
					sum1_2 += tmpptr[2] * kptr[1];
					sum1_3 += tmpptr[3] * kptr[1];
					sum1_4 += tmpptr[4] * kptr[1];
					sum1_5 += tmpptr[5] * kptr[1];
					sum1_6 += tmpptr[6] * kptr[1];
					sum1_7 += tmpptr[7] * kptr[1];

					sum2_0 += tmpptr[0] * kptr[2];
					sum2_1 += tmpptr[1] * kptr[2];
					sum2_2 += tmpptr[2] * kptr[2];
					sum2_3 += tmpptr[3] * kptr[2];
					sum2_4 += tmpptr[4] * kptr[2];
					sum2_5 += tmpptr[5] * kptr[2];
					sum2_6 += tmpptr[6] * kptr[2];
					sum2_7 += tmpptr[7] * kptr[2];

					sum3_0 += tmpptr[0] * kptr[3];
					sum3_1 += tmpptr[1] * kptr[3];
					sum3_2 += tmpptr[2] * kptr[3];
					sum3_3 += tmpptr[3] * kptr[3];
					sum3_4 += tmpptr[4] * kptr[3];
					sum3_5 += tmpptr[5] * kptr[3];
					sum3_6 += tmpptr[6] * kptr[3];
					sum3_7 += tmpptr[7] * kptr[3];

					tmpptr += 8;
					kptr += 4;
				}

				outptr0[0] = float2int8(sum0_0 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[1] = float2int8(sum0_1 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[2] = float2int8(sum0_2 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[3] = float2int8(sum0_3 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[4] = float2int8(sum0_4 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[5] = float2int8(sum0_5 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[6] = float2int8(sum0_6 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[7] = float2int8(sum0_7 * mem_scale[p + 0] + mem_bias[p + 0]);

				outptr1[0] = float2int8(sum1_0 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[1] = float2int8(sum1_1 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[2] = float2int8(sum1_2 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[3] = float2int8(sum1_3 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[4] = float2int8(sum1_4 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[5] = float2int8(sum1_5 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[6] = float2int8(sum1_6 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[7] = float2int8(sum1_7 * mem_scale[p + 1] + mem_bias[p + 1]);

				outptr2[0] = float2int8(sum2_0 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[1] = float2int8(sum2_1 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[2] = float2int8(sum2_2 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[3] = float2int8(sum2_3 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[4] = float2int8(sum2_4 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[5] = float2int8(sum2_5 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[6] = float2int8(sum2_6 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[7] = float2int8(sum2_7 * mem_scale[p + 2] + mem_bias[p + 2]);

				outptr3[0] = float2int8(sum3_0 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[1] = float2int8(sum3_1 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[2] = float2int8(sum3_2 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[3] = float2int8(sum3_3 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[4] = float2int8(sum3_4 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[5] = float2int8(sum3_5 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[6] = float2int8(sum3_6 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[7] = float2int8(sum3_7 * mem_scale[p + 3] + mem_bias[p + 3]);

				outptr0 += 8;
				outptr1 += 8;
				outptr2 += 8;
				outptr3 += 8;
#endif // __ARM_NEON            
			}

			for (; i + 3 < size; i += 4)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"
					"vmov.s32    q8, #0            \n"
					"vmov.s32    q9, #0            \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4-d5}, [%4]!    \n"// tmpr a00-a03,a10-a13,a20-a23,a30-a33    a(inch)(data)
					"vmovl.s8    q3, d5            \n"// a20-a23,a30-a33
					"vmovl.s8    q2, d4            \n"// a00-a04,a10-a14

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d4, d0[0]     \n"// sum0 = (a00-a03) * k00
					"vmlal.s16   q7, d4, d0[1]     \n"// sum1 = (a00-a03) * k10
					"vmlal.s16   q8, d4, d0[2]     \n"// sum2 = (a00-a03) * k20
					"vmlal.s16   q9, d4, d0[3]     \n"// sum3 = (a00-a03) * k30

					"vmlal.s16   q6, d5, d1[0]     \n"// sum0 += (a10-a13) * k01
					"vmlal.s16   q7, d5, d1[1]     \n"// sum1 += (a10-a13) * k11
					"vmlal.s16   q8, d5, d1[2]     \n"// sum2 += (a10-a13) * k21
					"vmlal.s16   q9, d5, d1[3]     \n"// sum3 += (a10-a13) * k31

					"vmlal.s16   q6, d6, d2[0]     \n"// sum0 += (a20-a23) * k02
					"vmlal.s16   q7, d6, d2[1]     \n"// sum1 += (a20-a23) * k12
					"vmlal.s16   q8, d6, d2[2]     \n"// sum2 += (a20-a23) * k22
					"vmlal.s16   q9, d6, d2[3]     \n"// sum3 += (a20-a23) * k32

					"vmlal.s16   q6, d7, d3[0]     \n"// sum0 += (a30-a33) * k03
					"vmlal.s16   q7, d7, d3[1]     \n"// sum1 += (a30-a33) * k13
					"vmlal.s16   q8, d7, d3[2]     \n"// sum2 += (a30-a33) * k23
					"vmlal.s16   q9, d7, d3[3]     \n"// sum3 += (a30-a33) * k33

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]        \n"// tmpr a00-a03    a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %4, #4            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// sum0 += (a00-a03) * k00
					"vmlal.s16   q7, d2, d0[1]     \n"// sum1 += (a00-a03) * k10
					"vmlal.s16   q8, d2, d0[2]     \n"// sum2 += (a00-a03) * k20
					"vmlal.s16   q9, d2, d0[3]     \n"// sum3 += (a00-a03) * k30

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"
					"vcvt.f32.s32 q8, q8           \n"
					"vcvt.f32.s32 q9, q9           \n"

					"vld1.f32    {d0, d1}, [%7]    \n"

					"vdup.f32    q1, %8            \n"
					"vdup.f32    q2, %9            \n"
					"vdup.f32    q3, %10           \n"
					"vdup.f32    q4, %11           \n"

					"vmla.f32    q1, q6, d0[0]     \n"
					"vmla.f32    q2, q7, d0[1]     \n"
					"vmla.f32    q3, q8, d1[0]     \n"
					"vmla.f32    q4, q9, d1[1]     \n"

					"vcvtr.s32.f32 s4, s4          \n"
					"vcvtr.s32.f32 s5, s5          \n"
					"vcvtr.s32.f32 s6, s6          \n"
					"vcvtr.s32.f32 s7, s7          \n"

					"vcvtr.s32.f32 s8, s8          \n"
					"vcvtr.s32.f32 s9, s9          \n"
					"vcvtr.s32.f32 s10, s10        \n"
					"vcvtr.s32.f32 s11, s11        \n"

					"vcvtr.s32.f32 s12, s12        \n"
					"vcvtr.s32.f32 s13, s13        \n"
					"vcvtr.s32.f32 s14, s14        \n"
					"vcvtr.s32.f32 s15, s15        \n"

					"vcvtr.s32.f32 s16, s16        \n"
					"vcvtr.s32.f32 s17, s17        \n"
					"vcvtr.s32.f32 s18, s18        \n"
					"vcvtr.s32.f32 s19, s19        \n"

					"vqmovn.s32  d2, q1            \n"
					"vqmovn.s32  d3, q2            \n"
					"vqmovn.s32  d4, q3            \n"
					"vqmovn.s32  d5, q4            \n"

					"vqmovn.s16  d0, q1            \n"
					"vqmovn.s16  d1, q2            \n"

					"vst1.s32   {d0[0]}, [%0]!    \n"
					"vst1.s32   {d0[1]}, [%1]!    \n"
					"vst1.s32   {d1[0]}, [%2]!    \n"
					"vst1.s32   {d1[1]}, [%3]!    \n"

					: "+r"(outptr0), // %0
					"+r"(outptr1), // %1
					"+r"(outptr2), // %2
					"+r"(outptr3), // %3
					"+r"(tmpptr),  // %4
					"+r"(kptr)     // %5
					: "r"(inch),     // %6  
					"r"(mem_scale + p),	// %7
					"r"(mem_bias[p]),		// %8
					"r"(mem_bias[p + 1]),	// %9
					"r"(mem_bias[p + 2]),	// %10
					"r"(mem_bias[p + 3])	// %11
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0_0 = 0;
				int sum0_1 = 0;
				int sum0_2 = 0;
				int sum0_3 = 0;

				int sum1_0 = 0;
				int sum1_1 = 0;
				int sum1_2 = 0;
				int sum1_3 = 0;

				int sum2_0 = 0;
				int sum2_1 = 0;
				int sum2_2 = 0;
				int sum2_3 = 0;

				int sum3_0 = 0;
				int sum3_1 = 0;
				int sum3_2 = 0;
				int sum3_3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0_0 += tmpptr[0] * kptr[0];
					sum0_1 += tmpptr[1] * kptr[0];
					sum0_2 += tmpptr[2] * kptr[0];
					sum0_3 += tmpptr[3] * kptr[0];

					sum1_0 += tmpptr[0] * kptr[1];
					sum1_1 += tmpptr[1] * kptr[1];
					sum1_2 += tmpptr[2] * kptr[1];
					sum1_3 += tmpptr[3] * kptr[1];

					sum2_0 += tmpptr[0] * kptr[2];
					sum2_1 += tmpptr[1] * kptr[2];
					sum2_2 += tmpptr[2] * kptr[2];
					sum2_3 += tmpptr[3] * kptr[2];

					sum3_0 += tmpptr[0] * kptr[3];
					sum3_1 += tmpptr[1] * kptr[3];
					sum3_2 += tmpptr[2] * kptr[3];
					sum3_3 += tmpptr[3] * kptr[3];

					tmpptr += 4;
					kptr += 4;
				}

				outptr0[0] = float2int8(sum0_0 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[1] = float2int8(sum0_1 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[2] = float2int8(sum0_2 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr0[3] = float2int8(sum0_3 * mem_scale[p + 0] + mem_bias[p + 0]);

				outptr1[0] = float2int8(sum1_0 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[1] = float2int8(sum1_1 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[2] = float2int8(sum1_2 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr1[3] = float2int8(sum1_3 * mem_scale[p + 1] + mem_bias[p + 1]);

				outptr2[0] = float2int8(sum2_0 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[1] = float2int8(sum2_1 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[2] = float2int8(sum2_2 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr2[3] = float2int8(sum2_3 * mem_scale[p + 2] + mem_bias[p + 2]);

				outptr3[0] = float2int8(sum3_0 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[1] = float2int8(sum3_1 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[2] = float2int8(sum3_2 * mem_scale[p + 3] + mem_bias[p + 3]);
				outptr3[3] = float2int8(sum3_3 * mem_scale[p + 3] + mem_bias[p + 3]);

				outptr0 += 4;
				outptr1 += 4;
				outptr2 += 4;
				outptr3 += 4;
#endif // __ARM_NEON            
			}

			for (; i < size; i++)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4 + i % 4);
				const signed char* kptr = weight + (p / 4) * kernel_cstep; // kernel.channel(p / 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"veor        q6, q6, q6        \n"
					"veor        q7, q7, q7        \n"
					"veor        q8, q8, q8        \n"
					"veor        q9, q9, q9        \n"
					"vmov.s32    q10, #0           \n"

					"lsr         r4, %6, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%4, #128]        \n"
					"vld1.s8     {d4}, [%4]        \n"// tmpr a00,a10,a20,a30    a(inch)(data)
					"add         %4, #4            \n"
					"vmovl.s8    q2, d4            \n"// a00,a10,a20,a30

					"vld1.s8     {d0-d1}, [%5]!    \n"// kptr k00-k30,k01-k31,k02-k32,k03-k33    k(outch)(inch)
					"vmovl.s8    q1, d1            \n"// k02-k32,k03-k33
					"vmovl.s8    q0, d0            \n"// k00-k30,k01-k31

					"vmlal.s16   q6, d0, d4[0]     \n"// (k00-k30) * a00
					"vmlal.s16   q7, d1, d4[1]     \n"// (k01-k31) * a10
					"vmlal.s16   q8, d2, d4[2]     \n"// (k02-k32) * a20
					"vmlal.s16   q9, d3, d4[3]     \n"// (k03-k33) * a30

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"vadd.s32    q6, q6, q7        \n"
					"vadd.s32    q9, q9, q8        \n"
					"vadd.s32    q10, q6, q9       \n"

					"1:                            \n"
					// remain loop
					"and         r4, %6, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%4]        \n"// tmpr a00        a(inch)(data)
					"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %4, #1            \n"
					"add         %5, #4            \n"

					"vmlal.s16   q10, d0, d2[0]    \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vcvt.f32.s32 q10, q10         \n"

					"vld1.f32    {d0, d1}, [%7]    \n"
					"vld1.f32    {d2, d3}, [%8]    \n"

					"vmla.f32    q1, q10, q0       \n"

					"vcvtr.s32.f32 s4, s4          \n"
					"vcvtr.s32.f32 s5, s5          \n"
					"vcvtr.s32.f32 s6, s6          \n"
					"vcvtr.s32.f32 s7, s7          \n"

					"vqmovn.s32  d0, q1            \n"
					"vqmovn.s32  d0, q0            \n"

					"vst1.s8     {d0[0]}, [%0]!    \n"
					"vst1.s8     {d0[1]}, [%1]!    \n"
					"vst1.s8     {d0[2]}, [%2]!    \n"
					"vst1.s8     {d0[3]}, [%3]!    \n"

					: "+r"(outptr0), // %0
					"+r"(outptr1), // %1
					"+r"(outptr2), // %2
					"+r"(outptr3), // %3
					"+r"(tmpptr),  // %4
					"+r"(kptr)     // %5
					: "r"(inch),     // %6  
					"r"(mem_scale + p),	// %7
					"r"(mem_bias + p) 	// %8
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[0] * kptr[1];
					sum2 += tmpptr[0] * kptr[2];
					sum3 += tmpptr[0] * kptr[3];

					tmpptr++;
					kptr += 4;
				}

				outptr0[0] = float2int8(sum0 * mem_scale[p + 0] + mem_bias[p + 0]);
				outptr1[0] = float2int8(sum1 * mem_scale[p + 1] + mem_bias[p + 1]);
				outptr2[0] = float2int8(sum2 * mem_scale[p + 2] + mem_bias[p + 2]);
				outptr3[0] = float2int8(sum3 * mem_scale[p + 3] + mem_bias[p + 3]);

				outptr0++;
				outptr1++;
				outptr2++;
				outptr3++;
#endif // __ARM_NEON
			}
		}

		remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
		nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			signed char* outptr0 = data_out + p * size; // Mat out0 = top_blob.channel(p);

			int i = 0;

			for (; i + 7 < size; i += 8)
			{
				const signed char* tmpptr = mem_tmp + (i / 8) * tmp_cstep; // tmp.channel(i / 8);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"
					"vmov.s32    q7, #0            \n"

					"lsr         r4, %3, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%1, #128]        \n"
					"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
					"vmovl.s8    q5, d7            \n"// a30-a37
					"vmovl.s8    q4, d6            \n"// a20-a27
					"vmovl.s8    q3, d5            \n"// a10-a17
					"vmovl.s8    q2, d4            \n"// a00-a07

					"vld1.s8     {d0}, [%2]        \n"// kptr k00,k01,k02,k03    k(outch)(inch)
					"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03
					"add         %2, #4            \n"

					"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a07) * k00
					"vmlal.s16   q7, d5, d0[0]     \n"
					"vmlal.s16   q6, d6, d0[1]     \n"// (a10-a17) * k01
					"vmlal.s16   q7, d7, d0[1]     \n"
					"vmlal.s16   q6, d8, d0[2]     \n"// (a20-a27) * k02
					"vmlal.s16   q7, d9, d0[2]     \n"
					"vmlal.s16   q6, d10, d0[3]    \n"// (a30-a37) * k03
					"vmlal.s16   q7, d11, d0[3]    \n"

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %3, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%1]!       \n"// tmpr a00-a07    a(inch)(data)
					"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %2, #1            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a07) * k00
					"vmlal.s16   q7, d3, d0[0]     \n"

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vdup.f32    q0, %4            \n"

					"vcvt.f32.s32 q6, q6           \n"
					"vcvt.f32.s32 q7, q7           \n"

					"vdup.f32    q1, %5            \n"
					"vmov.32     q2, q1            \n"

					"vmla.f32    q1, q6, q0        \n"
					"vmla.f32    q2, q7, q0        \n"

					"vcvtr.s32.f32 s4, s4          \n"
					"vcvtr.s32.f32 s5, s5          \n"
					"vcvtr.s32.f32 s6, s6          \n"
					"vcvtr.s32.f32 s7, s7          \n"

					"vcvtr.s32.f32 s8, s8          \n"
					"vcvtr.s32.f32 s9, s9          \n"
					"vcvtr.s32.f32 s10, s10        \n"
					"vcvtr.s32.f32 s11, s11        \n"

					"vqmovn.s32 d0, q1             \n"
					"vqmovn.s32 d1, q2             \n"

					"vqmovn.s16 d0, q0             \n"

					"vst1.s32    {d0}, [%0]!       \n"

					: "+r"(outptr0), // %0
					"+r"(tmpptr),  // %1
					"+r"(kptr)     // %2
					: "r"(inch),     // %3  
					"r"(mem_scale[p]), // %4
					"r"(mem_bias[p])   // %5
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;
				int sum4 = 0;
				int sum5 = 0;
				int sum6 = 0;
				int sum7 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[1] * kptr[0];
					sum2 += tmpptr[2] * kptr[0];
					sum3 += tmpptr[3] * kptr[0];
					sum4 += tmpptr[4] * kptr[0];
					sum5 += tmpptr[5] * kptr[0];
					sum6 += tmpptr[6] * kptr[0];
					sum7 += tmpptr[7] * kptr[0];

					tmpptr += 8;
					kptr++;
				}

				outptr0[0] = float2int8(sum0 * mem_scale[p] + mem_bias[p]);
				outptr0[1] = float2int8(sum1 * mem_scale[p] + mem_bias[p]);
				outptr0[2] = float2int8(sum2 * mem_scale[p] + mem_bias[p]);
				outptr0[3] = float2int8(sum3 * mem_scale[p] + mem_bias[p]);
				outptr0[4] = float2int8(sum4 * mem_scale[p] + mem_bias[p]);
				outptr0[5] = float2int8(sum5 * mem_scale[p] + mem_bias[p]);
				outptr0[6] = float2int8(sum6 * mem_scale[p] + mem_bias[p]);
				outptr0[7] = float2int8(sum7 * mem_scale[p] + mem_bias[p]);

				outptr0 += 8;
#endif // __ARM_NEON
			}

			for (; i + 3 < size; i += 4)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

#if __ARM_NEON
				asm volatile(
					// inch loop
					"vmov.s32    q6, #0            \n"

					"lsr         r4, %3, #2        \n"// r4 = nn = inch >> 2
					"cmp         r4, #0            \n"
					"beq         1f                \n"

					"0:                            \n"// for(; nn != 0; nn--)
					"pld         [%2, #128]        \n"
					"vld1.s8     {d4-d5}, [%1]!    \n"// tmpr a00-a03,a10-a13,a20-a23,a30-a33    a(inch)(data)
					"vmovl.s8    q3, d5            \n"// a20-a23,a30-a33
					"vmovl.s8    q2, d4            \n"// a00-a03,a10-a13

					"vld1.s8     {d0}, [%2]        \n"// kptr k00,k01,k02,k03    k(outch)(inch)
					"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03
					"add         %2, #4            \n"

					"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a03) * k00
					"vmlal.s16   q6, d5, d0[1]     \n"// (a10-a13) * k01
					"vmlal.s16   q6, d6, d0[2]     \n"// (a20-a23) * k02
					"vmlal.s16   q6, d7, d0[3]     \n"// (a30-a33) * k03

					"subs        r4, r4, #1        \n"
					"bne         0b                \n"// end for

					"1:                            \n"
					// remain loop
					"and         r4, %3, #3        \n"// r4 = remain = inch & 3
					"cmp         r4, #0            \n"
					"beq         3f                \n"

					"2:                            \n"// for(; remain != 0; remain--)
					"vld1.s8     {d2}, [%1]        \n"// tmpr a00-a03    a(inch)(data)
					"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
					"vmovl.s8    q1, d2            \n"
					"vmovl.s8    q0, d0            \n"
					"add         %1, #4            \n"
					"add         %2, #1            \n"

					"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a03) * k00

					"subs        r4, r4, #1        \n"
					"bne         2b                \n"

					"3:                            \n"// store the result to memory

					"vdup.f32    q0, %4            \n"
					"vdup.f32    q1, %5            \n"

					"vcvt.f32.s32 q6, q6           \n"

					"vmla.f32    q1, q6, q0        \n"

					"vcvt.s32.f32 s4, s4           \n"
					"vcvt.s32.f32 s5, s5           \n"
					"vcvt.s32.f32 s6, s6           \n"
					"vcvt.s32.f32 s7, s7           \n"

					"vqmovn.s32  d0, q1            \n"
					"vqmovn.s16  d0, q0            \n"

					"vst1.s32    {d0[0]}, [%0]!    \n"

					: "+r"(outptr0), // %0
					"+r"(tmpptr),  // %1
					"+r"(kptr)     // %2
					: "r"(inch),     // %3  
					"r"(mem_scale[p]), // %4
					"r"(mem_bias[p])   // %5
					: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6"
					);
#else
				int sum0 = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;

				for (int q = 0; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					sum1 += tmpptr[1] * kptr[0];
					sum2 += tmpptr[2] * kptr[0];
					sum3 += tmpptr[3] * kptr[0];

					tmpptr += 4;
					kptr++;
				}

				outptr0[0] = float2int8(sum0 * mem_scale[p] + mem_bias[p]);
				outptr0[1] = float2int8(sum1 * mem_scale[p] + mem_bias[p]);
				outptr0[2] = float2int8(sum2 * mem_scale[p] + mem_bias[p]);
				outptr0[3] = float2int8(sum3 * mem_scale[p] + mem_bias[p]);

				outptr0 += 4;
#endif // __ARM_NEON
			}

			for (; i < size; i++)
			{
				const signed char* tmpptr = mem_tmp + (i / 8 + (i % 8) / 4 + i % 4) * tmp_cstep; // tmp.channel(i / 8 + (i % 8) / 4 + i % 4);
				const signed char* kptr = weight + (p / 4 + p % 4) * kernel_cstep; // kernel.channel(p / 4 + p % 4);

				int q = 0;
				int sum0 = 0;

				for (; q < inch; q++)
				{
					sum0 += tmpptr[0] * kptr[0];
					tmpptr++;
					kptr++;
				}

				outptr0[0] = float2int8(sum0 * mem_scale[p] + mem_bias[p]);
				outptr0++;
			}
		}
	}

	// static void convdw3x3s1_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat &_kernel, const Option& opt)
	// null
	// 0
	void convd_k3s1_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;

		int outw = dim_out;
		int outh = dim_out;

		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			signed char* out = data_out + p * size_out; // Mat out = top_blob.channel(p);

			const signed char* kernel = weight + p * 9; // (const signed char *)_kernel + p * 9;

			signed char* outptr0 = out;
			signed char* outptr0n = outptr0 + outw;

			const signed char* img0 = data_in + p * size_in; // bottom_blob.channel(p);

			const signed char* r0 = img0;
			const signed char* r1 = img0 + w;
			const signed char* r2 = img0 + w * 2;
			const signed char* r3 = img0 + w * 3;

			float scale = mem_scale[p];
			float bias = mem_bias[p];

			int i = 0;

#if __ARM_NEON
			int8x16_t _k0123456789x = vld1q_s8(kernel);
			int16x8_t _k_s16 = vmovl_s8(vget_low_s8(_k0123456789x));
			int16x8_t _kn_s16 = vmovl_s8(vget_high_s8(_k0123456789x));

			int16x4_t _k0123 = vget_low_s16(_k_s16);
			int16x4_t _k4567 = vget_high_s16(_k_s16);
			int16x4_t _k8xxx = vget_low_s16(_kn_s16);
#endif // __ARM_NEON 

			for (; i + 1 < outh; i += 2)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"veor      q2, q2, q2            \n"

						"vdup.32   q3, %10               \n"
						"vdup.32   q4, %11               \n"

						"0:                              \n"
						// r0
						"vld1.s8    {d30-d31}, [%3]      \n"// r0
						"add    %3, %3, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r00
						"vmovl.s8    q5, d10             \n"// r01
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P7[0]      \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P7[0]      \n"
						"vmull.s16  q9, d10, %P7[1]      \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P7[1]     \n"
						"vmlal.s16  q7, d12, %P7[2]      \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P7[2]      \n"

						// r1
						"vld1.s8    {d30-d31}, [%4]      \n"// r1
						"add    %4, %4, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r10
						"vmovl.s8    q5, d10             \n"// r11
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P7[3]      \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P7[3]      \n"
						"vmlal.s16  q9, d10, %P8[0]      \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P8[0]     \n"
						"vmlal.s16  q7, d12, %P8[1]      \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P8[1]      \n"
						// sum1
						"vmull.s16  q11, d30, %P7[0]     \n"// (r10 - r17) * k00
						"vmull.s16  q12, d31, %P7[0]     \n"
						"vmull.s16  q13, d10, %P7[1]     \n"// (r11 - r18) * k01
						"vmull.s16  q14, d11, %P7[1]     \n"
						"vmlal.s16  q11, d12, %P7[2]     \n"// (r12 - r19) * k02
						"vmlal.s16  q12, d13, %P7[2]     \n"

						// r2
						"vld1.s8    {d30-d31}, [%5]      \n"// r2
						"add    %5, %5, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r20
						"vmovl.s8    q5, d10             \n"// r21
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P8[2]      \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P8[2]      \n"
						"vmlal.s16  q9, d10, %P8[3]      \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P8[3]     \n"
						"vmlal.s16  q7, d12, %P9[0]      \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P9[0]      \n"
						// sum1
						"vmlal.s16  q11, d30, %P7[3]     \n"// (r20 - r27) * k03
						"vmlal.s16  q12, d31, %P7[3]     \n"
						"vmlal.s16  q13, d10, %P8[0]     \n"// (r21 - r28) * k04
						"vmlal.s16  q14, d11, %P8[0]     \n"
						"vmlal.s16  q11, d12, %P8[1]     \n"// (r22 - r29) * k05
						"vmlal.s16  q12, d13, %P8[1]     \n"

						// r3
						"vld1.s8    {d30-d31}, [%6]      \n"// r3
						"add    %6, %6, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r30
						"vmovl.s8    q5, d10             \n"// r31
						"vmovl.s8    q6, d12             \n"// r32

															// sum1
						"vmlal.s16  q11, d30, %P8[2]     \n"// (r30 - r37) * k06
						"vmlal.s16  q12, d31, %P8[2]     \n"
						"vmlal.s16  q13, d10, %P8[3]     \n"// (r31 - r38) * k07
						"vmlal.s16  q14, d11, %P8[3]     \n"
						"vmlal.s16  q11, d12, %P9[0]     \n"// (r32 - r39) * k08
						"vmlal.s16  q12, d13, %P9[0]     \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"
						"vadd.s32    q11, q11, q13       \n"
						"vadd.s32    q12, q12, q14       \n"

						"vcvt.f32.s32 q9, q7             \n"
						"vcvt.f32.s32 q10, q8            \n"
						"vcvt.f32.s32 q11, q11           \n"
						"vcvt.f32.s32 q12, q12           \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"
						"vmov.32     q7, q4              \n"
						"vmov.32     q8, q4              \n"

						"vmla.f32    q5, q9, q3          \n"
						"vmla.f32    q6, q10, q3         \n"
						"vmla.f32    q7, q11, q3         \n"
						"vmla.f32    q8, q12, q3         \n"

						"vcvt.s32.f32 q5, q5			 \n"
						"vcvt.s32.f32 q6, q6			 \n"
						"vcvt.s32.f32 q7, q7			 \n"
						"vcvt.s32.f32 q8, q8			 \n"

						"vqmovn.s32  d10, q5             \n"
						"vqmovn.s32  d11, q6             \n"
						"vqmovn.s32  d12, q7             \n"
						"vqmovn.s32  d13, q8             \n"

						"vqmovn.s16  d10, q5             \n"
						"vqmovn.s16  d11, q6             \n"

						"vmax.s8     q5, q5, q2          \n"

						"vst1.s8     {d10}, [%1]!        \n"
						"vst1.s8     {d11}, [%2]!        \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr0),  // %1
						"+r"(outptr0n), // %2
						"+r"(r0),       // %3
						"+r"(r1),       // %4
						"+r"(r2),       // %5
						"+r"(r3)        // %6
						: "w"(_k0123),    // %7
						"w"(_k4567),    // %8
						"w"(_k8xxx),    // %9
						"r"(scale),	  // %10
						"r"(bias)       // %11
						: "cc", "memory", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					// TODO NEON
					int sum0 = 0;
					int sum0n = 0;

					sum0 += (int)r0[0] * kernel[0];
					sum0 += (int)r0[1] * kernel[1];
					sum0 += (int)r0[2] * kernel[2];
					sum0 += (int)r1[0] * kernel[3];
					sum0 += (int)r1[1] * kernel[4];
					sum0 += (int)r1[2] * kernel[5];
					sum0 += (int)r2[0] * kernel[6];
					sum0 += (int)r2[1] * kernel[7];
					sum0 += (int)r2[2] * kernel[8];

					sum0n += (int)r1[0] * kernel[0];
					sum0n += (int)r1[1] * kernel[1];
					sum0n += (int)r1[2] * kernel[2];
					sum0n += (int)r2[0] * kernel[3];
					sum0n += (int)r2[1] * kernel[4];
					sum0n += (int)r2[2] * kernel[5];
					sum0n += (int)r3[0] * kernel[6];
					sum0n += (int)r3[1] * kernel[7];
					sum0n += (int)r3[2] * kernel[8];

					int val;
					val = (int)(sum0 * scale + bias); if (val < 0) val = 0; if (val > 127) val = 127; *outptr0 = (signed char)val;
					val = (int)(sum0n * scale + bias); if (val < 0) val = 0; if (val > 127) val = 127; *outptr0n = (signed char)val;

					r0++;
					r1++;
					r2++;
					r3++;
					outptr0++;
					outptr0n++;
				}

				r0 += 2 + w;
				r1 += 2 + w;
				r2 += 2 + w;
				r3 += 2 + w;

				outptr0 += outw;
				outptr0n += outw;
			}

			for (; i < outh; i++)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"veor      q2, q2, q2            \n"

						"vdup.32   q3, %8                \n"
						"vdup.32   q4, %9                \n"

						"0:                              \n"
						// r0
						"vld1.s8    {d30-d31}, [%2]        \n"// r0
						"add    %2, %2, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r00
						"vmovl.s8    q5, d10             \n"// r01
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P5[0]      \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P5[0]      \n"
						"vmull.s16  q9, d10, %P5[1]     \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P5[1]    \n"
						"vmlal.s16  q7, d12, %P5[2]     \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P5[2]     \n"

						// r1
						"vld1.s8    {d30-d31}, [%3]        \n"// r1
						"add    %3, %3, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r10
						"vmovl.s8    q5, d10             \n"// r11
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P5[3]      \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P5[3]      \n"
						"vmlal.s16  q9, d10, %P6[0]     \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P6[0]    \n"
						"vmlal.s16  q7, d12, %P6[1]     \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P6[1]     \n"

						// r2
						"vld1.s8    {d30-d31}, [%4]        \n"// r2
						"add    %4, %4, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r20
						"vmovl.s8    q5, d10             \n"// r21
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P6[2]      \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P6[2]      \n"
						"vmlal.s16  q9, d10, %P6[3]      \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P6[3]     \n"
						"vmlal.s16  q7, d12, %P7[0]      \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P7[0]      \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"

						"vcvt.f32.s32 q7, q7             \n"
						"vcvt.f32.s32 q8, q8             \n"

						"vmla.f32    q5, q7, q3          \n"
						"vmla.f32    q6, q8, q3          \n"

						"vcvt.s32.f32 q5, q5             \n"
						"vcvt.s32.f32 q6, q6             \n"

						"vqmovn.s32   d10, q5            \n"
						"vqmovn.s32   d11, q6            \n"

						"vqmovn.s16   d10, q5            \n"

						"vmax.s8      d10, d10, d4       \n"

						"vst1.s8     {d10}, [%1]!        \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr0),  // %1
						"+r"(r0),       // %2
						"+r"(r1),       // %3
						"+r"(r2)        // %4
						: "w"(_k0123),    // %5
						"w"(_k4567),    // %6
						"w"(_k8xxx),    // %7
						"r"(scale),     // %8
						"r"(bias)       // %9
						: "cc", "memory", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					int sum = 0;

					sum += (int)r0[0] * kernel[0];
					sum += (int)r0[1] * kernel[1];
					sum += (int)r0[2] * kernel[2];
					sum += (int)r1[0] * kernel[3];
					sum += (int)r1[1] * kernel[4];
					sum += (int)r1[2] * kernel[5];
					sum += (int)r2[0] * kernel[6];
					sum += (int)r2[1] * kernel[7];
					sum += (int)r2[2] * kernel[8];

					int val;
					val = (int)(sum * scale + bias); if (val < 0) val = 0; if (val > 127) val = 127; *outptr0 = (signed char)val;

					r0++;
					r1++;
					r2++;
					outptr0++;
				}

				r0 += 2;
				r1 += 2;
				r2 += 2;
			}
		}
	}

	// static void convdw3x3s1_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat &_kernel, const Option& opt)
	// null
	// 0
	void convd_k3s1_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;

		int outw = dim_out;
		int outh = dim_out;

		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* out = data_out + p * size_out; // Mat out = top_blob.channel(p);

			const signed char* kernel = weight + p * 9; // (const signed char *)_kernel + p * 9;

			float* outptr0 = out;
			float* outptr0n = outptr0 + outw;

			const signed char* img0 = data_in + p * size_in; // bottom_blob.channel(p);

			const signed char* r0 = img0;
			const signed char* r1 = img0 + w;
			const signed char* r2 = img0 + w * 2;
			const signed char* r3 = img0 + w * 3;

			float scale = mem_scale[p];
			float bias = mem_bias[p];

			int i = 0;

#if __ARM_NEON
			int8x16_t _k0123456789x = vld1q_s8(kernel);
			int16x8_t _k_s16 = vmovl_s8(vget_low_s8(_k0123456789x));
			int16x8_t _kn_s16 = vmovl_s8(vget_high_s8(_k0123456789x));

			int16x4_t _k0123 = vget_low_s16(_k_s16);
			int16x4_t _k4567 = vget_high_s16(_k_s16);
			int16x4_t _k8xxx = vget_low_s16(_kn_s16);
#endif // __ARM_NEON 

			for (; i + 1 < outh; i += 2)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"vdup.32    q3, %10              \n"
						"vdup.32    q4, %11              \n"

						"0:                              \n"
						// r0
						"vld1.s8    {d30-d31}, [%3]      \n"// r0
						"add    %3, %3, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r00
						"vmovl.s8    q5, d10             \n"// r01
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P7[0]      \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P7[0]      \n"
						"vmull.s16  q9, d10, %P7[1]      \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P7[1]     \n"
						"vmlal.s16  q7, d12, %P7[2]      \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P7[2]      \n"

						// r1
						"vld1.s8    {d30-d31}, [%4]      \n"// r1
						"add    %4, %4, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r10
						"vmovl.s8    q5, d10             \n"// r11
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P7[3]      \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P7[3]      \n"
						"vmlal.s16  q9, d10, %P8[0]      \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P8[0]     \n"
						"vmlal.s16  q7, d12, %P8[1]      \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P8[1]      \n"
						// sum1
						"vmull.s16  q11, d30, %P7[0]     \n"// (r10 - r17) * k00
						"vmull.s16  q12, d31, %P7[0]     \n"
						"vmull.s16  q13, d10, %P7[1]     \n"// (r11 - r18) * k01
						"vmull.s16  q14, d11, %P7[1]     \n"
						"vmlal.s16  q11, d12, %P7[2]     \n"// (r12 - r19) * k02
						"vmlal.s16  q12, d13, %P7[2]     \n"

						// r2
						"vld1.s8    {d30-d31}, [%5]      \n"// r2
						"add    %5, %5, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r20
						"vmovl.s8    q5, d10             \n"// r21
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P8[2]      \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P8[2]      \n"
						"vmlal.s16  q9, d10, %P8[3]      \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P8[3]     \n"
						"vmlal.s16  q7, d12, %P9[0]      \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P9[0]      \n"
						// sum1
						"vmlal.s16  q11, d30, %P7[3]     \n"// (r20 - r27) * k03
						"vmlal.s16  q12, d31, %P7[3]     \n"
						"vmlal.s16  q13, d10, %P8[0]     \n"// (r21 - r28) * k04
						"vmlal.s16  q14, d11, %P8[0]     \n"
						"vmlal.s16  q11, d12, %P8[1]     \n"// (r22 - r29) * k05
						"vmlal.s16  q12, d13, %P8[1]     \n"

						// r3
						"vld1.s8    {d30-d31}, [%6]      \n"// r3
						"add    %6, %6, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r30
						"vmovl.s8    q5, d10             \n"// r31
						"vmovl.s8    q6, d12             \n"// r32

															// sum1
						"vmlal.s16  q11, d30, %P8[2]     \n"// (r30 - r37) * k06
						"vmlal.s16  q12, d31, %P8[2]     \n"
						"vmlal.s16  q13, d10, %P8[3]     \n"// (r31 - r38) * k07
						"vmlal.s16  q14, d11, %P8[3]     \n"
						"vmlal.s16  q11, d12, %P9[0]     \n"// (r32 - r39) * k08
						"vmlal.s16  q12, d13, %P9[0]     \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"
						"vadd.s32    q11, q11, q13       \n"
						"vadd.s32    q12, q12, q14       \n"

						"vcvt.f32.s32 q9, q7             \n"
						"vcvt.f32.s32 q10, q8            \n"
						"vcvt.f32.s32 q11, q11           \n"
						"vcvt.f32.s32 q12, q12           \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"
						"vmov.32     q7, q4              \n"
						"vmov.32     q8, q4              \n"

						"vmla.f32    q5, q9, q3          \n"
						"vmla.f32    q6, q10, q3         \n"
						"vmla.f32    q7, q11, q3         \n"
						"vmla.f32    q8, q12, q3         \n"

						"vst1.s32    {d10-d13}, [%1]!    \n"
						"vst1.s32    {d14-d17}, [%2]!    \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr0),  // %1
						"+r"(outptr0n), // %2
						"+r"(r0),       // %3
						"+r"(r1),       // %4
						"+r"(r2),       // %5
						"+r"(r3)        // %6
						: "w"(_k0123),    // %7
						"w"(_k4567),    // %8
						"w"(_k8xxx),    // %9
						"r"(scale),	  // %10
						"r"(bias)       // %11
						: "cc", "memory", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					// TODO NEON
					int sum0 = 0;
					int sum0n = 0;

					sum0 += (int)r0[0] * kernel[0];
					sum0 += (int)r0[1] * kernel[1];
					sum0 += (int)r0[2] * kernel[2];
					sum0 += (int)r1[0] * kernel[3];
					sum0 += (int)r1[1] * kernel[4];
					sum0 += (int)r1[2] * kernel[5];
					sum0 += (int)r2[0] * kernel[6];
					sum0 += (int)r2[1] * kernel[7];
					sum0 += (int)r2[2] * kernel[8];

					sum0n += (int)r1[0] * kernel[0];
					sum0n += (int)r1[1] * kernel[1];
					sum0n += (int)r1[2] * kernel[2];
					sum0n += (int)r2[0] * kernel[3];
					sum0n += (int)r2[1] * kernel[4];
					sum0n += (int)r2[2] * kernel[5];
					sum0n += (int)r3[0] * kernel[6];
					sum0n += (int)r3[1] * kernel[7];
					sum0n += (int)r3[2] * kernel[8];

					*outptr0 = sum0 * scale + bias;
					*outptr0n = sum0n * scale + bias;

					r0++;
					r1++;
					r2++;
					r3++;
					outptr0++;
					outptr0n++;
				}

				r0 += 2 + w;
				r1 += 2 + w;
				r2 += 2 + w;
				r3 += 2 + w;

				outptr0 += outw;
				outptr0n += outw;
			}

			for (; i < outh; i++)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"vdup.32    q3, %8               \n"
						"vdup.32    q4, %9               \n"

						"0:                              \n"
						// r0
						"vld1.s8    {d30-d31}, [%2]        \n"// r0
						"add    %2, %2, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r00
						"vmovl.s8    q5, d10             \n"// r01
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P5[0]      \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P5[0]      \n"
						"vmull.s16  q9, d10, %P5[1]     \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P5[1]    \n"
						"vmlal.s16  q7, d12, %P5[2]     \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P5[2]     \n"

						// r1
						"vld1.s8    {d30-d31}, [%3]        \n"// r1
						"add    %3, %3, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r10
						"vmovl.s8    q5, d10             \n"// r11
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P5[3]      \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P5[3]      \n"
						"vmlal.s16  q9, d10, %P6[0]     \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P6[0]    \n"
						"vmlal.s16  q7, d12, %P6[1]     \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P6[1]     \n"

						// r2
						"vld1.s8    {d30-d31}, [%4]        \n"// r2
						"add    %4, %4, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r20
						"vmovl.s8    q5, d10             \n"// r21
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P6[2]      \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P6[2]      \n"
						"vmlal.s16  q9, d10, %P6[3]      \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P6[3]     \n"
						"vmlal.s16  q7, d12, %P7[0]      \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P7[0]      \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"

						"vcvt.f32.s32 q7, q7             \n"
						"vcvt.f32.s32 q8, q8             \n"

						"vmla.f32    q5, q7, q3          \n"
						"vmla.f32    q6, q8, q3          \n"

						"vst1.s32    {d10-d13}, [%1]!    \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr0),  // %1
						"+r"(r0),       // %2
						"+r"(r1),       // %3
						"+r"(r2)        // %4
						: "w"(_k0123),    // %5
						"w"(_k4567),    // %6
						"w"(_k8xxx),    // %7
						"r"(scale),     // %8
						"r"(bias)       // %9
						: "cc", "memory", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					int sum = 0;

					sum += (int)r0[0] * kernel[0];
					sum += (int)r0[1] * kernel[1];
					sum += (int)r0[2] * kernel[2];
					sum += (int)r1[0] * kernel[3];
					sum += (int)r1[1] * kernel[4];
					sum += (int)r1[2] * kernel[5];
					sum += (int)r2[0] * kernel[6];
					sum += (int)r2[1] * kernel[7];
					sum += (int)r2[2] * kernel[8];

					*outptr0 = sum * scale + bias;

					r0++;
					r1++;
					r2++;
					outptr0++;
				}

				r0 += 2;
				r1 += 2;
				r2 += 2;
			}
		}
	}

	// static void convdw3x3s1_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat &_kernel, const Option& opt)
	// null
	// 0
	void convd_k3s1_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;

		int outw = dim_out;
		int outh = dim_out;

		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			signed char* out = data_out + p * size_out; // Mat out = top_blob.channel(p);

			const signed char* kernel = weight + p * 9; // (const signed char *)_kernel + p * 9;

			signed char* outptr0 = out;
			signed char* outptr0n = outptr0 + outw;

			const signed char* img0 = data_in + p * size_in; // bottom_blob.channel(p);

			const signed char* r0 = img0;
			const signed char* r1 = img0 + w;
			const signed char* r2 = img0 + w * 2;
			const signed char* r3 = img0 + w * 3;

			float scale = mem_scale[p];
			float bias = mem_bias[p];

			int i = 0;

#if __ARM_NEON
			int8x16_t _k0123456789x = vld1q_s8(kernel);
			int16x8_t _k_s16 = vmovl_s8(vget_low_s8(_k0123456789x));
			int16x8_t _kn_s16 = vmovl_s8(vget_high_s8(_k0123456789x));

			int16x4_t _k0123 = vget_low_s16(_k_s16);
			int16x4_t _k4567 = vget_high_s16(_k_s16);
			int16x4_t _k8xxx = vget_low_s16(_kn_s16);
#endif // __ARM_NEON 

			for (; i + 1 < outh; i += 2)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"vdup.32   q3, %10               \n"
						"vdup.32   q4, %11               \n"

						"0:                              \n"
						// r0
						"vld1.s8    {d30-d31}, [%3]      \n"// r0
						"add    %3, %3, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r00
						"vmovl.s8    q5, d10             \n"// r01
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P7[0]      \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P7[0]      \n"
						"vmull.s16  q9, d10, %P7[1]      \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P7[1]     \n"
						"vmlal.s16  q7, d12, %P7[2]      \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P7[2]      \n"

						// r1
						"vld1.s8    {d30-d31}, [%4]      \n"// r1
						"add    %4, %4, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r10
						"vmovl.s8    q5, d10             \n"// r11
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P7[3]      \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P7[3]      \n"
						"vmlal.s16  q9, d10, %P8[0]      \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P8[0]     \n"
						"vmlal.s16  q7, d12, %P8[1]      \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P8[1]      \n"
						// sum1
						"vmull.s16  q11, d30, %P7[0]     \n"// (r10 - r17) * k00
						"vmull.s16  q12, d31, %P7[0]     \n"
						"vmull.s16  q13, d10, %P7[1]     \n"// (r11 - r18) * k01
						"vmull.s16  q14, d11, %P7[1]     \n"
						"vmlal.s16  q11, d12, %P7[2]     \n"// (r12 - r19) * k02
						"vmlal.s16  q12, d13, %P7[2]     \n"

						// r2
						"vld1.s8    {d30-d31}, [%5]      \n"// r2
						"add    %5, %5, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r20
						"vmovl.s8    q5, d10             \n"// r21
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P8[2]      \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P8[2]      \n"
						"vmlal.s16  q9, d10, %P8[3]      \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P8[3]     \n"
						"vmlal.s16  q7, d12, %P9[0]      \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P9[0]      \n"
						// sum1
						"vmlal.s16  q11, d30, %P7[3]     \n"// (r20 - r27) * k03
						"vmlal.s16  q12, d31, %P7[3]     \n"
						"vmlal.s16  q13, d10, %P8[0]     \n"// (r21 - r28) * k04
						"vmlal.s16  q14, d11, %P8[0]     \n"
						"vmlal.s16  q11, d12, %P8[1]     \n"// (r22 - r29) * k05
						"vmlal.s16  q12, d13, %P8[1]     \n"

						// r3
						"vld1.s8    {d30-d31}, [%6]      \n"// r3
						"add    %6, %6, #8               \n"

						"vext.s8    d10, d30, d31, #1    \n"
						"vext.s8    d12, d30, d31, #2    \n"

						"vmovl.s8    q15, d30            \n"// r30
						"vmovl.s8    q5, d10             \n"// r31
						"vmovl.s8    q6, d12             \n"// r32

															// sum1
						"vmlal.s16  q11, d30, %P8[2]     \n"// (r30 - r37) * k06
						"vmlal.s16  q12, d31, %P8[2]     \n"
						"vmlal.s16  q13, d10, %P8[3]     \n"// (r31 - r38) * k07
						"vmlal.s16  q14, d11, %P8[3]     \n"
						"vmlal.s16  q11, d12, %P9[0]     \n"// (r32 - r39) * k08
						"vmlal.s16  q12, d13, %P9[0]     \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"
						"vadd.s32    q11, q11, q13       \n"
						"vadd.s32    q12, q12, q14       \n"

						"vcvt.f32.s32 q9, q7             \n"
						"vcvt.f32.s32 q10, q8            \n"
						"vcvt.f32.s32 q11, q11           \n"
						"vcvt.f32.s32 q12, q12           \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"
						"vmov.32     q7, q4              \n"
						"vmov.32     q2, q4              \n"

						"vmla.f32    q5, q9, q3          \n"
						"vmla.f32    q6, q10, q3         \n"
						"vmla.f32    q7, q11, q3         \n"
						"vmla.f32    q2, q12, q3         \n"

						"vcvtr.s32.f32 s20, s20			 \n"
						"vcvtr.s32.f32 s21, s21			 \n"
						"vcvtr.s32.f32 s22, s22			 \n"
						"vcvtr.s32.f32 s23, s23			 \n"

						"vcvtr.s32.f32 s24, s24			 \n"
						"vcvtr.s32.f32 s25, s25			 \n"
						"vcvtr.s32.f32 s26, s26			 \n"
						"vcvtr.s32.f32 s27, s27			 \n"

						"vcvtr.s32.f32 s28, s28			 \n"
						"vcvtr.s32.f32 s29, s29			 \n"
						"vcvtr.s32.f32 s30, s30			 \n"
						"vcvtr.s32.f32 s31, s31			 \n"

						"vcvtr.s32.f32 s8,  s8 			 \n"
						"vcvtr.s32.f32 s9,  s9			 \n"
						"vcvtr.s32.f32 s10, s10			 \n"
						"vcvtr.s32.f32 s11, s11			 \n"

						"vcvt.s32.f32 q5, q5			 \n"
						"vcvt.s32.f32 q6, q6			 \n"
						"vcvt.s32.f32 q7, q7			 \n"
						"vcvt.s32.f32 q8, q2			 \n"

						"vqmovn.s32  d10, q5             \n"
						"vqmovn.s32  d11, q6             \n"
						"vqmovn.s32  d12, q7             \n"
						"vqmovn.s32  d13, q8             \n"

						"vqmovn.s16  d10, q5             \n"
						"vqmovn.s16  d11, q6             \n"

						"vst1.s8     {d10}, [%1]!        \n"
						"vst1.s8     {d11}, [%2]!        \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr0),  // %1
						"+r"(outptr0n), // %2
						"+r"(r0),       // %3
						"+r"(r1),       // %4
						"+r"(r2),       // %5
						"+r"(r3)        // %6
						: "w"(_k0123),    // %7
						"w"(_k4567),    // %8
						"w"(_k8xxx),    // %9
						"r"(scale),	  // %10
						"r"(bias)       // %11
						: "cc", "memory", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					// TODO NEON
					int sum0 = 0;
					int sum0n = 0;

					sum0 += (int)r0[0] * kernel[0];
					sum0 += (int)r0[1] * kernel[1];
					sum0 += (int)r0[2] * kernel[2];
					sum0 += (int)r1[0] * kernel[3];
					sum0 += (int)r1[1] * kernel[4];
					sum0 += (int)r1[2] * kernel[5];
					sum0 += (int)r2[0] * kernel[6];
					sum0 += (int)r2[1] * kernel[7];
					sum0 += (int)r2[2] * kernel[8];

					sum0n += (int)r1[0] * kernel[0];
					sum0n += (int)r1[1] * kernel[1];
					sum0n += (int)r1[2] * kernel[2];
					sum0n += (int)r2[0] * kernel[3];
					sum0n += (int)r2[1] * kernel[4];
					sum0n += (int)r2[2] * kernel[5];
					sum0n += (int)r3[0] * kernel[6];
					sum0n += (int)r3[1] * kernel[7];
					sum0n += (int)r3[2] * kernel[8];

					*outptr0 = float2int8(sum0 * scale + bias);
					*outptr0n = float2int8(sum0n * scale + bias);

					r0++;
					r1++;
					r2++;
					r3++;
					outptr0++;
					outptr0n++;
				}

				r0 += 2 + w;
				r1 += 2 + w;
				r2 += 2 + w;
				r3 += 2 + w;

				outptr0 += outw;
				outptr0n += outw;
			}

			for (; i < outh; i++)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"vdup.32   q3, %8                \n"
						"vdup.32   q4, %9                \n"

						"0:                              \n"
						// r0
						"vld1.s8    {d30-d31}, [%2]        \n"// r0
						"add    %2, %2, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r00
						"vmovl.s8    q5, d10             \n"// r01
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P5[0]      \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P5[0]      \n"
						"vmull.s16  q9, d10, %P5[1]     \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P5[1]    \n"
						"vmlal.s16  q7, d12, %P5[2]     \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P5[2]     \n"

						// r1
						"vld1.s8    {d30-d31}, [%3]        \n"// r1
						"add    %3, %3, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r10
						"vmovl.s8    q5, d10             \n"// r11
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P5[3]      \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P5[3]      \n"
						"vmlal.s16  q9, d10, %P6[0]     \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P6[0]    \n"
						"vmlal.s16  q7, d12, %P6[1]     \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P6[1]     \n"

						// r2
						"vld1.s8    {d30-d31}, [%4]        \n"// r2
						"add    %4, %4, #8               \n"

						"vext.s8    d10, d30, d31, #1      \n"
						"vext.s8    d12, d30, d31, #2      \n"

						"vmovl.s8    q15, d30              \n"// r20
						"vmovl.s8    q5, d10             \n"// r21
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P6[2]      \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P6[2]      \n"
						"vmlal.s16  q9, d10, %P6[3]      \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P6[3]     \n"
						"vmlal.s16  q7, d12, %P7[0]      \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P7[0]      \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"

						"vcvt.f32.s32 q7, q7             \n"
						"vcvt.f32.s32 q8, q8             \n"

						"vmla.f32    q5, q7, q3          \n"
						"vmla.f32    q6, q8, q3          \n"

						"vcvtr.s32.f32 s20, s20          \n"
						"vcvtr.s32.f32 s21, s21          \n"
						"vcvtr.s32.f32 s22, s22          \n"
						"vcvtr.s32.f32 s23, s23          \n"

						"vcvtr.s32.f32 s24, s24          \n"
						"vcvtr.s32.f32 s25, s25          \n"
						"vcvtr.s32.f32 s26, s26          \n"
						"vcvtr.s32.f32 s27, s27          \n"

						"vqmovn.s32   d10, q5            \n"
						"vqmovn.s32   d11, q6            \n"

						"vqmovn.s16   d10, q5            \n"

						"vst1.s8     {d10}, [%1]!        \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr0),  // %1
						"+r"(r0),       // %2
						"+r"(r1),       // %3
						"+r"(r2)        // %4
						: "w"(_k0123),    // %5
						"w"(_k4567),    // %6
						"w"(_k8xxx),    // %7
						"r"(scale),     // %8
						"r"(bias)       // %9
						: "cc", "memory", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					int sum = 0;

					sum += (int)r0[0] * kernel[0];
					sum += (int)r0[1] * kernel[1];
					sum += (int)r0[2] * kernel[2];
					sum += (int)r1[0] * kernel[3];
					sum += (int)r1[1] * kernel[4];
					sum += (int)r1[2] * kernel[5];
					sum += (int)r2[0] * kernel[6];
					sum += (int)r2[1] * kernel[7];
					sum += (int)r2[2] * kernel[8];

					*outptr0 = float2int8(sum * scale + bias);

					r0++;
					r1++;
					r2++;
					outptr0++;
				}

				r0 += 2;
				r1 += 2;
				r2 += 2;
			}
		}
	}

	// static void convdw3x3s2_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat &_kernel, const Option& opt)
	// null
	// 0
	void convd_k3s2_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;

		int outw = dim_out;
		int outh = dim_out;
		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

		const int tailstep = w - 2 * outw + w;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			float* out = data_out + p * size_out; // Mat out = top_blob.channel(p);

			const signed char* kernel = weight + p * 9; // (const signed char*)_kernel + p * 9;

			float* outptr = out;

			const signed char* img = data_in + p * size_in; // bottom_blob.channel(p);

			const signed char* r0 = img;
			const signed char* r1 = img + w;
			const signed char* r2 = img + w * 2;

			int i = 0;
#if __ARM_NEON 
			float scale = mem_scale[p];
			float bias = mem_bias[p];

			int8x16_t _k0123456789x = vld1q_s8(kernel);
			int16x8_t _k_s16 = vmovl_s8(vget_low_s8(_k0123456789x));
			int16x8_t _kn_s16 = vmovl_s8(vget_high_s8(_k0123456789x));

			int16x4_t _k0123 = vget_low_s16(_k_s16);
			int16x4_t _k4567 = vget_high_s16(_k_s16);
			int16x4_t _k8xxx = vget_low_s16(_kn_s16);
#endif // __ARM_NEON 
			for (; i < outh; i++)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"vdup.32    q3, %8               \n"
						"vdup.32    q4, %9               \n"

						"0:                              \n"
						// r0
						"vld2.s8    {d30-d31}, [%2]!     \n"// r0
						"vld2.s8    {d10-d11}, [%2]      \n"
						"vext.s8    d12, d30, d10, #1    \n"

						"vmovl.s8    q5, d31             \n"// r01
						"vmovl.s8    q15, d30            \n"// r00
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P5[0]     \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P5[0]     \n"
						"vmull.s16  q9, d10, %P5[1]     \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P5[1]    \n"
						"vmlal.s16  q7, d12, %P5[2]     \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P5[2]     \n"

						// r1
						"vld2.s8    {d30-d31}, [%3]!     \n"// r1
						"vld2.s8    {d10-d11}, [%3]      \n"
						"vext.s8    d12, d30, d10, #1    \n"

						"vmovl.s8    q5, d31             \n"// r11
						"vmovl.s8    q15, d30            \n"// r10
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P5[3]     \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P5[3]     \n"
						"vmlal.s16  q9, d10, %P6[0]     \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P6[0]    \n"
						"vmlal.s16  q7, d12, %P6[1]     \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P6[1]     \n"

						// r2
						"vld2.s8    {d30-d31}, [%4]!     \n"// r2
						"vld2.s8    {d10-d11}, [%4]      \n"
						"vext.s8    d12, d30, d10, #1    \n"

						"vmovl.s8    q5, d31             \n"// r21
						"vmovl.s8    q15, d30            \n"// r20
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P6[2]     \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P6[2]     \n"
						"vmlal.s16  q9, d10, %P6[3]     \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P6[3]    \n"
						"vmlal.s16  q7, d12, %P7[0]     \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P7[0]     \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"

						"vcvt.f32.s32 q7, q7             \n"
						"vcvt.f32.s32 q8, q8             \n"

						"vmla.f32    q5, q7, q3          \n"
						"vmla.f32    q6, q8, q3          \n"

						"vst1.s32    {d10-d13}, [%1]!    \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr),   // %1
						"+r"(r0),       // %2
						"+r"(r1),       // %3
						"+r"(r2)        // %4
						: "w"(_k0123),    // %5
						"w"(_k4567),    // %6
						"w"(_k8xxx),    // %7
						"r"(scale),     // %8
						"r"(bias)       // %9
						: "cc", "memory", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					int sum = 0;

					sum += (int)r0[0] * kernel[0];
					sum += (int)r0[1] * kernel[1];
					sum += (int)r0[2] * kernel[2];
					sum += (int)r1[0] * kernel[3];
					sum += (int)r1[1] * kernel[4];
					sum += (int)r1[2] * kernel[5];
					sum += (int)r2[0] * kernel[6];
					sum += (int)r2[1] * kernel[7];
					sum += (int)r2[2] * kernel[8];

					*outptr = sum * mem_scale[p] + mem_bias[p];

					r0 += 2;
					r1 += 2;
					r2 += 2;
					outptr++;
				}

				r0 += tailstep;
				r1 += tailstep;
				r2 += tailstep;
			}
		}
	}

	// static void convdw3x3s2_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat &_kernel, const Option& opt)
	// null
	// 0
	void convd_k3s2_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;

		int outw = dim_out;
		int outh = dim_out;
		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

		const int tailstep = w - 2 * outw + w;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < outch; p++)
		{
			signed char* out = data_out + p * size_out; // Mat out = top_blob.channel(p);

			const signed char* kernel = weight + p * 9; // (const signed char*)_kernel + p * 9;

			signed char* outptr = out;

			const signed char* img = data_in + p * size_in; // bottom_blob.channel(p);

			const signed char* r0 = img;
			const signed char* r1 = img + w;
			const signed char* r2 = img + w * 2;

			int i = 0;
#if __ARM_NEON 
			float scale = mem_scale[p];
			float bias = mem_bias[p];
			int8x16_t _k0123456789x = vld1q_s8(kernel);
			int16x8_t _k_s16 = vmovl_s8(vget_low_s8(_k0123456789x));
			int16x8_t _kn_s16 = vmovl_s8(vget_high_s8(_k0123456789x));

			int16x4_t _k0123 = vget_low_s16(_k_s16);
			int16x4_t _k4567 = vget_high_s16(_k_s16);
			int16x4_t _k8xxx = vget_low_s16(_kn_s16);
#endif // __ARM_NEON 
			for (; i < outh; i++)
			{
#if __ARM_NEON
				int nn = outw >> 3;
				int remain = outw & 7;
				if (nn > 0)
				{
					asm volatile(
						"vdup.32    q3, %8               \n"
						"vdup.32    q4, %9               \n"

						"0:                              \n"
						// r0
						"vld2.s8    {d30-d31}, [%2]!     \n"// r0
						"vld2.s8    {d10-d11}, [%2]      \n"
						"vext.s8    d12, d30, d10, #1    \n"

						"vmovl.s8    q5, d31             \n"// r01
						"vmovl.s8    q15, d30            \n"// r00
						"vmovl.s8    q6, d12             \n"// r02
															// sum0
						"vmull.s16  q7, d30, %P5[0]     \n"// (r00 - r07) * k00
						"vmull.s16  q8, d31, %P5[0]     \n"
						"vmull.s16  q9, d10, %P5[1]     \n"// (r01 - r08) * k01
						"vmull.s16  q10, d11, %P5[1]    \n"
						"vmlal.s16  q7, d12, %P5[2]     \n"// (r02 - r09) * k02
						"vmlal.s16  q8, d13, %P5[2]     \n"

						// r1
						"vld2.s8    {d30-d31}, [%3]!     \n"// r1
						"vld2.s8    {d10-d11}, [%3]      \n"
						"vext.s8    d12, d30, d10, #1    \n"

						"vmovl.s8    q5, d31             \n"// r11
						"vmovl.s8    q15, d30            \n"// r10
						"vmovl.s8    q6, d12             \n"// r12
															// sum0
						"vmlal.s16  q7, d30, %P5[3]     \n"// (r10 - r17) * k03
						"vmlal.s16  q8, d31, %P5[3]     \n"
						"vmlal.s16  q9, d10, %P6[0]     \n"// (r11 - r18) * k04
						"vmlal.s16  q10, d11, %P6[0]    \n"
						"vmlal.s16  q7, d12, %P6[1]     \n"// (r12 - r19) * k05
						"vmlal.s16  q8, d13, %P6[1]     \n"

						// r2
						"vld2.s8    {d30-d31}, [%4]!     \n"// r2
						"vld2.s8    {d10-d11}, [%4]      \n"
						"vext.s8    d12, d30, d10, #1    \n"

						"vmovl.s8    q5, d31             \n"// r21
						"vmovl.s8    q15, d30            \n"// r20
						"vmovl.s8    q6, d12             \n"// r22

															// sum0
						"vmlal.s16  q7, d30, %P6[2]     \n"// (r20 - r27) * k06
						"vmlal.s16  q8, d31, %P6[2]     \n"
						"vmlal.s16  q9, d10, %P6[3]     \n"// (r21 - r28) * k07
						"vmlal.s16  q10, d11, %P6[3]    \n"
						"vmlal.s16  q7, d12, %P7[0]     \n"// (r22 - r29) * k08
						"vmlal.s16  q8, d13, %P7[0]     \n"

						"subs   %0, %0, #1               \n"

						// add and save
						"vadd.s32    q7, q7, q9          \n"
						"vadd.s32    q8, q8, q10         \n"

						"vmov.32     q5, q4              \n"
						"vmov.32     q6, q4              \n"

						"vcvt.f32.s32 q7, q7             \n"
						"vcvt.f32.s32 q8, q8             \n"

						"vmla.f32    q5, q7, q3          \n"
						"vmla.f32    q6, q8, q3          \n"

						"veor        d14, d14, d14       \n"

						"vcvt.s32.f32 q5, q5             \n"
						"vcvt.s32.f32 q6, q6             \n"

						"vqmovn.s32   d10, q5            \n"
						"vqmovn.s32   d11, q6            \n"
						"vqmovn.s16   d10, q5            \n"

						"vmax.s8     d10, d10, d14       \n"

						"vst1.s8     {d10}, [%1]!        \n"

						"bne    0b                       \n"

						: "+r"(nn),       // %0
						"+r"(outptr),   // %1
						"+r"(r0),       // %2
						"+r"(r1),       // %3
						"+r"(r2)        // %4
						: "w"(_k0123),    // %5
						"w"(_k4567),    // %6
						"w"(_k8xxx),    // %7
						"r"(scale),     // %8
						"r"(bias)       // %9
						: "cc", "memory", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
				}
#else
				int remain = outw;
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					int sum = 0;

					sum += (int)r0[0] * kernel[0];
					sum += (int)r0[1] * kernel[1];
					sum += (int)r0[2] * kernel[2];
					sum += (int)r1[0] * kernel[3];
					sum += (int)r1[1] * kernel[4];
					sum += (int)r1[2] * kernel[5];
					sum += (int)r2[0] * kernel[6];
					sum += (int)r2[1] * kernel[7];
					sum += (int)r2[2] * kernel[8];

					int val;
					val = (int)(sum * mem_scale[p] + mem_bias[p]); if (val < 0) val = 0; if (val > 127) val = 127;  *outptr = (signed char)val;

					r0 += 2;
					r1 += 2;
					r2 += 2;
					outptr++;
				}

				r0 += tailstep;
				r1 += tailstep;
				r2 += tailstep;
			}
		}
	}

	// static void conv_im2col_sgemm_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat & kernel_tm, const int kernel_w, const int kernel_h, const int stride_w, const int stride_h, const Option& opt)
	// conv_im2col_sgemm_transform_kernel_int8_neon
	// (8 * kernel_size) * inch * (out_size/8 + out_size%8)
	void conv_kxsx_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;
		int inch = ch_in;

		int outw = dim_out;
		int outh = dim_out;
		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

		int tm_cstep = 8 * ch_in * dim_kernel * dim_kernel;
		int kernel_cstep = 4 * dim_kernel * dim_kernel * ch_in;

		// im2col
		// bottom_im2col memory packed 8 x 8
		{
			int i, ic, kx, ky;
			int x, x0, x1, x2, x3, x4, x5, x6, x7;
			int y, y0, y1, y2, y3, y4, y5, y6, y7;
			int xy0, xy1, xy2, xy3, xy4, xy5, xy6, xy7;

			int nn = size_out >> 3;
			int remain = nn << 3;

			for (x = 0, y = 0, i = 0; i < nn; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + i * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x1 = x; y1 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x2 = x; y2 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x3 = x; y3 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x4 = x; y4 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x5 = x; y5 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x6 = x; y6 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x7 = x; y7 = y; x++; if (x >= dim_out) { x = 0; y++; }

				xy0 = (x0 + y0 * dim_in) * stride;
				xy1 = (x1 + y1 * dim_in) * stride;
				xy2 = (x2 + y2 * dim_in) * stride;
				xy3 = (x3 + y3 * dim_in) * stride;
				xy4 = (x4 + y4 * dim_in) * stride;
				xy5 = (x5 + y5 * dim_in) * stride;
				xy6 = (x6 + y6 * dim_in) * stride;
				xy7 = (x7 + y7 * dim_in) * stride;

				const signed char* imin_iter_ic = data_in;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < ch_in; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							*tm_iter++ = imin_iter_kx[xy1];
							*tm_iter++ = imin_iter_kx[xy2];
							*tm_iter++ = imin_iter_kx[xy3];
							*tm_iter++ = imin_iter_kx[xy4];
							*tm_iter++ = imin_iter_kx[xy5];
							*tm_iter++ = imin_iter_kx[xy6];
							*tm_iter++ = imin_iter_kx[xy7];
							imin_iter_kx++;
						}
						imin_iter_ky += dim_in;
					}
					imin_iter_ic += size_in;
				}
			}

			for (i = remain; i < size_out; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + (i / 8 + i % 8) * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= dim_out) { x = 0; y++; }
				xy0 = (x0 + y0 * dim_in) * stride;

				const signed char* imin_iter_ic = data_in;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < ch_in; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							imin_iter_kx++;
						}
						imin_iter_ky += dim_in;
					}
					imin_iter_ic += size_in;
				}
			}
		}

		// sgemm(int M, int N, int L, float* A, float* B, float* C)
		{
			//int M = outch;  // outch
			int N = outw * outh; // outsize or out stride
			int L = dim_kernel * dim_kernel * inch; // ksize * inch

			int nn_outch = 0;
			int remain_outch_start = 0;

			nn_outch = (outch - remain_outch_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int pp = 0; pp < nn_outch; pp++)
			{
				int i = remain_outch_start + pp * 4;

				float* output0 = data_out + (i + 0) * size_out; // top_blob.channel(i);
				float* output1 = data_out + (i + 1) * size_out; // top_blob.channel(i + 1);
				float* output2 = data_out + (i + 2) * size_out; // top_blob.channel(i + 2);
				float* output3 = data_out + (i + 3) * size_out; // top_blob.channel(i + 3);

				int j = 0;
				for (; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// K loop
						"vmov.s32    q8, #0             \n"
						"vmov.s32    q9, #0             \n"
						"vmov.s32    q10, #0            \n"
						"vmov.s32    q11, #0            \n"
						"vmov.s32    q12, #0            \n"
						"vmov.s32    q13, #0            \n"
						"vmov.s32    q14, #0            \n"
						"vmov.s32    q15, #0            \n"

						"lsr         r4, %6, #3         \n"// r4 = nn = L >> 3
						"cmp         r4, #0             \n"
						"beq         1f                 \n"

						"0:                             \n"// for(; nn != 0; nn--)
						"pld         [%4, #128]         \n"
						"vld1.s8     {d8-d11}, [%4]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q7, d11            \n"// a30-a37
						"vmovl.s8    q6, d10            \n"// a20-a27                    
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"pld         [%5, #128]         \n"
						"vld1.s8     {d0-d3}, [%5]!     \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q3, d3             \n"// k06-k36,k07-k37
						"vmovl.s8    q2, d2             \n"// k04-k34,k05-k35
						"vmovl.s8    q1, d1             \n"// k02-k32,k03-k33
						"vmovl.s8    q0, d0             \n"// k00-k30,k01-k31

						"vmlal.s16   q8, d8, d0[0]      \n"// sum0 = (a00-a07) * k00
						"vmlal.s16   q9, d9, d0[0]      \n"
						"vmlal.s16   q10, d8, d0[1]     \n"// sum1 = (a00-a07) * k10
						"vmlal.s16   q11, d9, d0[1]     \n"
						"vmlal.s16   q12, d8, d0[2]     \n"// sum2 = (a00-a07) * k20
						"vmlal.s16   q13, d9, d0[2]     \n"
						"vmlal.s16   q14, d8, d0[3]     \n"// sum3 = (a00-a07) * k30
						"vmlal.s16   q15, d9, d0[3]     \n"

						"vmlal.s16   q8, d10, d1[0]     \n"// sum0 += (a10-a17) * k01
						"vmlal.s16   q9, d11, d1[0]     \n"
						"vmlal.s16   q10, d10, d1[1]    \n"// sum1 += (a10-a17) * k11
						"vmlal.s16   q11, d11, d1[1]    \n"
						"vmlal.s16   q12, d10, d1[2]    \n"// sum2 += (a10-a17) * k21
						"vmlal.s16   q13, d11, d1[2]    \n"
						"vmlal.s16   q14, d10, d1[3]    \n"// sum3 += (a10-a17) * k31
						"vmlal.s16   q15, d11, d1[3]    \n"

						"pld         [%4, #128]         \n"
						"vld1.s8     {d8-d9}, [%4]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"vmlal.s16   q8, d12, d2[0]     \n"// sum0 += (a20-a27) * k02
						"vmlal.s16   q9, d13, d2[0]     \n"
						"vmlal.s16   q10, d12, d2[1]    \n"// sum1 += (a20-a27) * k12
						"vmlal.s16   q11, d13, d2[1]    \n"
						"vmlal.s16   q12, d12, d2[2]    \n"// sum2 += (a20-a27) * k22
						"vmlal.s16   q13, d13, d2[2]    \n"
						"vmlal.s16   q14, d12, d2[3]    \n"// sum3 += (a20-a27) * k32
						"vmlal.s16   q15, d13, d2[3]    \n"

						"vmlal.s16   q8, d14, d3[0]     \n"// sum0 += (a30-a37) * k03
						"vmlal.s16   q9, d15, d3[0]     \n"
						"vmlal.s16   q10, d14, d3[1]    \n"// sum1 += (a30-a37) * k13
						"vmlal.s16   q11, d15, d3[1]    \n"
						"vmlal.s16   q12, d14, d3[2]    \n"// sum2 += (a30-a37) * k23
						"vmlal.s16   q13, d15, d3[2]    \n"
						"vmlal.s16   q14, d14, d3[3]    \n"// sum3 += (a30-a37) * k33
						"vmlal.s16   q15, d15, d3[3]    \n"

						"pld         [%4, #128]         \n"
						"vld1.s8     {d0-d1}, [%4]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q1, d1             \n"// a10-a17
						"vmovl.s8    q0, d0             \n"// a00-a07

						"vmlal.s16   q8, d8, d4[0]      \n"// sum0 += (a40-a47) * k04
						"vmlal.s16   q9, d9, d4[0]      \n"
						"vmlal.s16   q10, d8, d4[1]     \n"// sum1 += (a40-a47) * k14
						"vmlal.s16   q11, d9, d4[1]     \n"
						"vmlal.s16   q12, d8, d4[2]     \n"// sum2 += (a40-a47) * k24
						"vmlal.s16   q13, d9, d4[2]     \n"
						"vmlal.s16   q14, d8, d4[3]     \n"// sum3 += (a40-a47) * k34
						"vmlal.s16   q15, d9, d4[3]     \n"

						"vmlal.s16   q8, d10, d5[0]     \n"// sum0 += (a50-a57) * k05
						"vmlal.s16   q9, d11, d5[0]     \n"
						"vmlal.s16   q10, d10, d5[1]    \n"// sum1 += (a50-a57) * k15
						"vmlal.s16   q11, d11, d5[1]    \n"
						"vmlal.s16   q12, d10, d5[2]    \n"// sum2 += (a50-a57) * k25
						"vmlal.s16   q13, d11, d5[2]    \n"
						"vmlal.s16   q14, d10, d5[3]    \n"// sum3 += (a50-a57) * k35
						"vmlal.s16   q15, d11, d5[3]    \n"

						"vmlal.s16   q8, d0, d6[0]      \n"// sum0 += (a60-a67) * k06
						"vmlal.s16   q9, d1, d6[0]      \n"
						"vmlal.s16   q10, d0, d6[1]     \n"// sum1 += (a60-a67) * k16
						"vmlal.s16   q11, d1, d6[1]     \n"
						"vmlal.s16   q12, d0, d6[2]     \n"// sum2 += (a60-a67) * k26
						"vmlal.s16   q13, d1, d6[2]     \n"
						"vmlal.s16   q14, d0, d6[3]     \n"// sum3 += (a60-a67) * k36
						"vmlal.s16   q15, d1, d6[3]     \n"

						"vmlal.s16   q8, d2, d7[0]      \n"// sum0 += (a70-a77) * k07
						"vmlal.s16   q9, d3, d7[0]      \n"
						"vmlal.s16   q10, d2, d7[1]     \n"// sum1 += (a70-a77) * k17
						"vmlal.s16   q11, d3, d7[1]     \n"
						"vmlal.s16   q12, d2, d7[2]     \n"// sum2 += (a70-a77) * k27
						"vmlal.s16   q13, d3, d7[2]     \n"
						"vmlal.s16   q14, d2, d7[3]     \n"// sum3 += (a70-a77) * k37
						"vmlal.s16   q15, d3, d7[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         0b                 \n"// end for

						"1:                             \n"
						// remain loop
						"and         r4, %6, #7         \n"// r4 = remain = inch & 7
						"cmp         r4, #0             \n"
						"beq         3f                 \n"

						"2:                             \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%4]!        \n"// tmpr a00-a70    a(inch)(data)
						"vld1.s8     {d0}, [%5]         \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2             \n"
						"vmovl.s8    q0, d0             \n"
						"add         %5, #4             \n"

						"vmlal.s16   q8, d2, d0[0]      \n"// sum0 += (a00-a70) * k00
						"vmlal.s16   q9, d3, d0[0]      \n"
						"vmlal.s16   q10, d2, d0[1]     \n"// sum1 += (a00-a70) * k10
						"vmlal.s16   q11, d3, d0[1]     \n"
						"vmlal.s16   q12, d2, d0[2]     \n"// sum2 += (a00-a70) * k20
						"vmlal.s16   q13, d3, d0[2]     \n"
						"vmlal.s16   q14, d2, d0[3]     \n"// sum3 += (a00-a70) * k30
						"vmlal.s16   q15, d3, d0[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         2b                 \n"

						"3:                             \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%7]      \n"

						"vcvt.f32.s32 q8, q8            \n"
						"vcvt.f32.s32 q9, q9            \n"
						"vcvt.f32.s32 q10, q10          \n"
						"vcvt.f32.s32 q11, q11          \n"

						"vdup.32     q1, %8             \n"
						"vdup.32     q3, %9             \n"
						"vmov.32     q2, q1             \n"
						"vmov.32     q4, q3             \n"

						"vmla.f32    q1, q8, d0[0]      \n"
						"vmla.f32    q2, q9, d0[0]      \n"
						"vmla.f32    q3, q10, d0[1]     \n"
						"vmla.f32    q4, q11, d0[1]     \n"

						"vcvt.f32.s32 q12, q12          \n"
						"vcvt.f32.s32 q13, q13          \n"
						"vcvt.f32.s32 q14, q14          \n"
						"vcvt.f32.s32 q15, q15          \n"

						"vdup.32     q5, %10            \n"
						"vdup.32     q7, %11            \n"
						"vmov.32     q6, q5             \n"
						"vmov.32     q8, q7             \n"

						"vmla.f32    q5, q12, d1[0]     \n"
						"vmla.f32    q6, q13, d1[0]     \n"
						"vmla.f32    q7, q14, d1[1]     \n"
						"vmla.f32    q8, q15, d1[1]     \n"

						"vst1.s32    {d2-d5}, [%0]      \n"
						"vst1.s32    {d6-d9}, [%1]      \n"
						"vst1.s32    {d10-d13}, [%2]    \n"
						"vst1.s32    {d14-d17}, [%3]    \n"

						: "+r"(output0), // %0
						"+r"(output1), // %1
						"+r"(output2), // %2
						"+r"(output3), // %3
						"+r"(vb),      // %4
						"+r"(va)       // %5
						: "r"(L),        // %6  
						"r"(mem_scale + i),   // %7
						"r"(mem_bias[i]),     // %8
						"r"(mem_bias[i + 1]),   // %9
						"r"(mem_bias[i + 2]),   // %10
						"r"(mem_bias[i + 3])    // %11
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
#else
					int sum0[8] = { 0 };
					int sum1[8] = { 0 };
					int sum2[8] = { 0 };
					int sum3[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 8];
							sum1[n] += (int)va[1] * vb[n + 8];
							sum2[n] += (int)va[2] * vb[n + 8];
							sum3[n] += (int)va[3] * vb[n + 8];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 16];
							sum1[n] += (int)va[1] * vb[n + 16];
							sum2[n] += (int)va[2] * vb[n + 16];
							sum3[n] += (int)va[3] * vb[n + 16];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 24];
							sum1[n] += (int)va[1] * vb[n + 24];
							sum2[n] += (int)va[2] * vb[n + 24];
							sum3[n] += (int)va[3] * vb[n + 24];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 32];
							sum1[n] += (int)va[1] * vb[n + 32];
							sum2[n] += (int)va[2] * vb[n + 32];
							sum3[n] += (int)va[3] * vb[n + 32];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 40];
							sum1[n] += (int)va[1] * vb[n + 40];
							sum2[n] += (int)va[2] * vb[n + 40];
							sum3[n] += (int)va[3] * vb[n + 40];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 48];
							sum1[n] += (int)va[1] * vb[n + 48];
							sum2[n] += (int)va[2] * vb[n + 48];
							sum3[n] += (int)va[3] * vb[n + 48];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 56];
							sum1[n] += (int)va[1] * vb[n + 56];
							sum2[n] += (int)va[2] * vb[n + 56];
							sum3[n] += (int)va[3] * vb[n + 56];
							va -= 28;
						}

						va += 32;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
						}

						va += 4;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						output0[n] = sum0[n] * mem_scale[i] + mem_bias[i];
						output1[n] = sum1[n] * mem_scale[i + 1] + mem_bias[i + 1];
						output2[n] = sum2[n] * mem_scale[i + 2] + mem_bias[i + 2];
						output3[n] = sum3[n] * mem_scale[i + 3] + mem_bias[i + 3];
					}
#endif // __ARM_NEON
					output0 += 8;
					output1 += 8;
					output2 += 8;
					output3 += 8;
				}

				for (; j < N; j++)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8 + j % 8) * tm_cstep; // bottom_tm.channel(j / 8 + j % 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// inch loop
						"veor        q6, q6, q6        \n"
						"veor        q7, q7, q7        \n"
						"veor        q8, q8, q8        \n"
						"veor        q9, q9, q9        \n"
						"veor        q10, q10, q10     \n"
						"veor        q11, q11, q11     \n"
						"veor        q12, q12, q12     \n"
						"veor        q13, q13, q13     \n"
						"vmov.s32    q14, #0           \n"

						"lsr         r4, %6, #3        \n"// r4 = nn = L >> 2
						"cmp         r4, #0            \n"
						"beq         1f                \n"

						"0:                            \n"// for(; nn != 0; nn--)
						"pld         [%4, #128]        \n"
						"vld1.s8     {d0}, [%4]!       \n"// tmpr a00,a10,a20,a30    a(inch)(data)
						"vmovl.s8    q0, d0            \n"// a00-a07

						"pld         [%5, #128]        \n"
						"vld1.s8     {d2-d5}, [%5]!    \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q4, d5            \n"// k06-k36,k07-k37
						"vmovl.s8    q3, d4            \n"// k04-k34,k05-k35
						"vmovl.s8    q2, d3            \n"// k02-k32,k03-k33
						"vmovl.s8    q1, d2            \n"// k00-k30,k01-k31

						"vmlal.s16   q6, d2, d0[0]     \n"// (k00-k30) * a00
						"vmlal.s16   q7, d3, d0[1]     \n"// (k01-k31) * a01
						"vmlal.s16   q8, d4, d0[2]     \n"// (k02-k32) * a02
						"vmlal.s16   q9, d5, d0[3]     \n"// (k03-k33) * a03
						"vmlal.s16   q10, d6, d1[0]    \n"// (k04-k34) * a04
						"vmlal.s16   q11, d7, d1[1]    \n"// (k05-k35) * a05
						"vmlal.s16   q12, d8, d1[2]    \n"// (k06-k36) * a06
						"vmlal.s16   q13, d9, d1[3]    \n"// (k07-k37) * a07                    

						"subs        r4, r4, #1        \n"
						"bne         0b                \n"// end for

						"vadd.s32    q6, q6, q7        \n"
						"vadd.s32    q9, q9, q8        \n"
						"vadd.s32    q11, q11, q10     \n"
						"vadd.s32    q13, q13, q12     \n"

						"vadd.s32    q9, q9, q6        \n"
						"vadd.s32    q13, q13, q11     \n"
						"vadd.s32    q14, q13, q9      \n"

						"1:                            \n"
						// remain loop
						"and         r4, %6, #7        \n"// r4 = remain = inch & 3
						"cmp         r4, #0            \n"
						"beq         3f                \n"

						"2:                            \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%4]        \n"// tmpr a00        a(inch)(data)
						"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2            \n"
						"vmovl.s8    q0, d0            \n"
						"add         %4, #1            \n"
						"add         %5, #4            \n"

						"vmlal.s16   q14, d0, d2[0]    \n"

						"subs        r4, r4, #1        \n"
						"bne         2b                \n"

						"3:                            \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%7]     \n"
						"vld1.f32    {d2-d3}, [%8]     \n"

						"vcvt.f32.s32 q14, q14         \n"

						"vmla.f32    q1, q14, q0       \n"

						"vst1.s32    {d2[0]}, [%0]     \n"
						"vst1.s32    {d2[1]}, [%1]     \n"
						"vst1.s32    {d3[0]}, [%2]     \n"
						"vst1.s32    {d3[1]}, [%3]     \n"

						: "+r"(output0), // %0
						"+r"(output1), // %1
						"+r"(output2), // %2
						"+r"(output3), // %3
						"+r"(vb),      // %4
						"+r"(va)       // %5
						: "r"(L),        // %6  
						"r"(mem_scale + i),  // %7
						"r"(mem_bias + i)    // %8
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14"
						);
#else
					int sum0 = 0;
					int sum1 = 0;
					int sum2 = 0;
					int sum3 = 0;

					for (int k = 0; k < L; k++)
					{
						sum0 += (int)va[0] * vb[0];
						sum1 += (int)va[1] * vb[0];
						sum2 += (int)va[2] * vb[0];
						sum3 += (int)va[3] * vb[0];

						va += 4;
						vb += 1;
					}

					output0[0] = sum0 * mem_scale[i + 0] + mem_bias[i + 0];
					output1[0] = sum1 * mem_scale[i + 1] + mem_bias[i + 1];
					output2[0] = sum2 * mem_scale[i + 2] + mem_bias[i + 2];
					output3[0] = sum3 * mem_scale[i + 3] + mem_bias[i + 3];
#endif // __ARM_NEON
					output0++;
					output1++;
					output2++;
					output3++;
				}
			}

			remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = remain_outch_start; i < outch; i++)
			{
				float* output = data_out + i * size_out; // top_blob.channel(i);

				int j = 0;
				for (; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4 + i % 4) * kernel_cstep; // kernel_tm.channel(i / 4 + i % 4);

#if __ARM_NEON
					asm volatile(
						// inch loop
						"vmov.s32    q6, #0            \n"
						"vmov.s32    q7, #0            \n"

						"lsr         r4, %3, #3        \n"// r4 = nn = inch >> 3
						"cmp         r4, #0            \n"
						"beq         1f                \n"

						"0:                            \n"// for(; nn != 0; nn--)
						"pld         [%1, #128]        \n"
						"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d7            \n"// a30-a37
						"vmovl.s8    q4, d6            \n"// a20-a27
						"vmovl.s8    q3, d5            \n"// a10-a17
						"vmovl.s8    q2, d4            \n"// a00-a07

						"pld         [%2, #128]        \n"
						"vld1.s8     {d0}, [%2]!       \n"// kptr k00-k07    k(outch)(inch)
						"vmovl.s8    q1, d1            \n"// k04,k05,k06,k07
						"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03

						"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d5, d0[0]     \n"
						"vmlal.s16   q6, d6, d0[1]     \n"// (a10-a17) * k01
						"vmlal.s16   q7, d7, d0[1]     \n"
						"vmlal.s16   q6, d8, d0[2]     \n"// (a20-a27) * k02
						"vmlal.s16   q7, d9, d0[2]     \n"
						"vmlal.s16   q6, d10, d0[3]    \n"// (a30-a37) * k03
						"vmlal.s16   q7, d11, d0[3]    \n"

						"pld         [%1, #128]        \n"
						"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a40-a47,a50-a57,a60-a67,a70-a77    a(inch)(data)
						"vmovl.s8    q5, d7            \n"// a70-a77
						"vmovl.s8    q4, d6            \n"// a60-a67
						"vmovl.s8    q3, d5            \n"// a50-a57
						"vmovl.s8    q2, d4            \n"// a40-a47

						"vmlal.s16   q6, d4, d1[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d5, d1[0]     \n"
						"vmlal.s16   q6, d6, d1[1]     \n"// (a10-a17) * k01
						"vmlal.s16   q7, d7, d1[1]     \n"
						"vmlal.s16   q6, d8, d1[2]     \n"// (a20-a27) * k02
						"vmlal.s16   q7, d9, d1[2]     \n"
						"vmlal.s16   q6, d10, d1[3]    \n"// (a30-a37) * k03
						"vmlal.s16   q7, d11, d1[3]    \n"

						"subs        r4, r4, #1        \n"
						"bne         0b                \n"// end for

						"1:                            \n"
						// remain loop
						"and         r4, %3, #7        \n"// r4 = remain = inch & 7
						"cmp         r4, #0            \n"
						"beq         3f                \n"

						"2:                            \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%1]!       \n"// tmpr a00-a07    a(inch)(data)
						"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
						"vmovl.s8    q1, d2            \n"
						"vmovl.s8    q0, d0            \n"
						"add         %2, #1            \n"

						"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d3, d0[0]     \n"

						"subs        r4, r4, #1        \n"
						"bne         2b                \n"

						"3:                            \n"// store the result to memory

						"vdup.32   q0, %4              \n"
						"vdup.32   q1, %5              \n"
						"vmov.32   q2, q1              \n"

						"vcvt.f32.s32  q6, q6          \n"
						"vcvt.f32.s32  q7, q7          \n"

						"vmla.f32   q1, q6, q0         \n"
						"vmla.f32   q2, q7, q0         \n"

						"vst1.s32    {d2-d5}, [%0]     \n"

						: "+r"(output), // %0
						"+r"(vb),     // %1
						"+r"(va)      // %2
						: "r"(L),       // %3  
						"r"(mem_scale[i]), // %4
						"r"(mem_bias[i])   // %5
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7"
						);
#else                
					int sum[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum[n] += (int)va[0] * vb[n];
							sum[n] += (int)va[1] * vb[n + 8];
							sum[n] += (int)va[2] * vb[n + 16];
							sum[n] += (int)va[3] * vb[n + 24];
							sum[n] += (int)va[4] * vb[n + 32];
							sum[n] += (int)va[5] * vb[n + 40];
							sum[n] += (int)va[6] * vb[n + 48];
							sum[n] += (int)va[7] * vb[n + 56];
						}

						va += 8;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum[n] += (int)va[0] * vb[n];
						}

						va += 1;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						output[n] = sum[n] * mem_scale[i] + mem_bias[i];
					}
#endif // __ARM_NEON
					output += 8;
				}

				for (; j < N; j++)
				{
					int sum = 0;

					signed char* vb = ((signed char*)mem_tm) + (j / 8 + j % 8) * tm_cstep; // bottom_tm.channel(j / 8 + j % 8);
					const signed char* va = weight + (i / 4 + i % 4) * kernel_cstep; // kernel_tm.channel(i / 4 + i % 4);

					for (int k = 0; k < L; k++)
					{
						sum += (int)va[0] * vb[0];

						va += 1;
						vb += 1;
					}
					output[0] = sum * mem_scale[i] + mem_bias[i];
					output++;
				}
			}
		}
	}

	// static void conv3x3s2_packed_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& _kernel, const Option& opt)
	// conv3x3s2_transform_kernel_int8_neon
	// 0
	void conv_k3s2_nf_r(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;
		int inch = ch_in;

		int outw = dim_out;
		int outh = dim_out;
		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

		const int tailstep = w - 2 * outw + w;

		int nn_outch = outch >> 3;
		int remain_outch_start = nn_outch << 3;

		memset(data_out, 0, ch_out * size_out * 4);

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pp = 0; pp < nn_outch; pp++)
		{
			int p = pp * 8;

			int* out0 = ((int*)data_out) + size_out * p;  // top_blob.channel(p + 0);
			int* out1 = out0 + size_out;				  // top_blob.channel(p + 1);
			int* out2 = out1 + size_out;				  // top_blob.channel(p + 2);
			int* out3 = out2 + size_out;				  // top_blob.channel(p + 3);
			int* out4 = out3 + size_out;				  // top_blob.channel(p + 4);
			int* out5 = out4 + size_out;				  // top_blob.channel(p + 5);
			int* out6 = out5 + size_out;				  // top_blob.channel(p + 6);
			int* out7 = out6 + size_out;				  // top_blob.channel(p + 7);

			const signed char* ktmp = weight + pp * (72 * ch_in); //_kernel.channel(p / 8);

			for (int q = 0; q < inch; q++)
			{
				int* outptr0 = out0;
				int* outptr1 = out1;
				int* outptr2 = out2;
				int* outptr3 = out3;
				int* outptr4 = out4;
				int* outptr5 = out5;
				int* outptr6 = out6;
				int* outptr7 = out7;

				const signed char* img0 = data_in + q * size_in; // bottom_blob.channel(q);

				const signed char* r0 = img0;
				const signed char* r1 = img0 + w;
				const signed char* r2 = img0 + w * 2;

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
							"pld        [%1, #128]          \n"
							"vld1.s32   {d16-d17}, [%1]     \n"// out0
							"pld        [%2, #128]          \n"
							"vld1.s32   {d18-d19}, [%2]     \n"// out1
							"pld        [%3, #128]          \n"
							"vld1.s32   {d20-d21}, [%3]     \n"// out2
							"pld        [%4, #128]          \n"
							"vld1.s32   {d22-d23}, [%4]     \n"// out3 

															   // r0
							"pld        [%9, #64]          \n"
							"vld2.s8    {d8-d9}, [%9]       \n"// d8(a00 a02 a04 a06 a08 a010 a012 a014), d9(a01 a03 a05 a07 a09 a011 a013 a015)
							"add        %9, #8              \n"
							"pld        [%12, #64]         \n"
							"vld1.s8    {d0-d2}, [%12]!     \n"// d0(k00-k70) d1(k01-k71) d2(k02-k72)

							"pld        [%5, #128]          \n"
							"vld1.s32   {d24-d25}, [%5]     \n"// out4
							"pld        [%6, #128]          \n"
							"vld1.s32   {d26-d27}, [%6]     \n"// out5

							"vmovl.s8   q2, d2              \n"// q2(k02-k72)
							"vmovl.s8   q1, d1              \n"// q1(k01-k71)
							"vmovl.s8   q0, d0              \n"// q0(k00-k70)
							"vext.s8    d12, d8, d8, #1     \n"// d12(a02 a04 a06 a08 x x x x)

							"pld        [%7, #128]          \n"
							"vld1.s32   {d28-d29}, [%7]     \n"// out6

							"vmovl.s8   q5, d9              \n"// q5(a01 a03 a05 a07 a09 a011 a013 a015) d11
							"vmovl.s8   q4, d8              \n"// q4(a00 a02 a04 a06 a08 a010 a012 a014) d9
							"vmovl.s8   q6, d12             \n"// q6(a02 a04 a06 a08 a010 a012 a014 a016) d13

							"pld        [%8, #128]          \n"
							"vld1.s32   {d30-d31}, [%8]     \n"// out7

							"vmlal.s16  q8, d8, d0[0]       \n"// sum0 += (a00 a02 a04 a06) * k00
							"vmlal.s16  q9, d8, d0[1]       \n"// sum1 += (a00 a02 a04 a06) * k10
							"vmlal.s16  q10, d8, d0[2]      \n"// sum2 += (a00 a02 a04 a06) * k20
							"vmlal.s16  q11, d8, d0[3]      \n"// sum3 += (a00 a02 a04 a06) * k30
							"vmlal.s16  q12, d8, d1[0]      \n"// sum4 += (a00 a02 a04 a06) * k40
							"vmlal.s16  q13, d8, d1[1]      \n"// sum5 += (a00 a02 a04 a06) * k50
							"vmlal.s16  q14, d8, d1[2]      \n"// sum6 += (a00 a02 a04 a06) * k60
							"vmlal.s16  q15, d8, d1[3]      \n"// sum7 += (a00 a02 a04 a06) * k70

							"vmlal.s16  q8, d10, d2[0]      \n"// sum0 += (a01-a07) * k01
							"vmlal.s16  q9, d10, d2[1]      \n"// sum1 += (a01-a07) * k11
							"vmlal.s16  q10, d10, d2[2]     \n"// sum2 += (a01-a07) * k21
							"vmlal.s16  q11, d10, d2[3]     \n"// sum3 += (a01-a07) * k31
							"vmlal.s16  q12, d10, d3[0]     \n"// sum4 += (a01-a07) * k41
							"vmlal.s16  q13, d10, d3[1]     \n"// sum5 += (a01-a07) * k51
							"vmlal.s16  q14, d10, d3[2]     \n"// sum6 += (a01-a07) * k61
							"vmlal.s16  q15, d10, d3[3]     \n"// sum7 += (a01-a07) * k71

							"pld        [%10, #64]         \n"
							"vld2.s8    {d8-d9}, [%10]      \n"// d8(a10 a12 a14 a16 a18 a110 a112 a114), d9(a11 a13 a15 a17 a19 a111 a113 a115)
							"add        %10, #8             \n"

							"vmlal.s16  q8, d12, d4[0]      \n"// sum0 += (a02-a08) * k02
							"vmlal.s16  q9, d12, d4[1]      \n"// sum1 += (a02-a08) * k12
							"vmlal.s16  q10, d12, d4[2]     \n"// sum2 += (a02-a08) * k22
							"vmlal.s16  q11, d12, d4[3]     \n"// sum3 += (a02-a08) * k32

							"pld        [%12, #64]         \n"
							"vld1.s8    {d0-d2}, [%12]!     \n"// d0(k03-k73) d1(k04-k74) d2(k05-k75)

							"vmlal.s16  q12, d12, d5[0]     \n"// sum4 += (a02-a08) * k42
							"vmlal.s16  q13, d12, d5[1]     \n"// sum5 += (a02-a08) * k52
							"vmlal.s16  q14, d12, d5[2]     \n"// sum6 += (a02-a08) * k62
							"vmlal.s16  q15, d12, d5[3]     \n"// sum7 += (a02-a08) * k72

															   // r1
							"vext.s8    d12, d8, d8, #1     \n"// d12(a12 a14 a16 a18 x x x x)

							"vmovl.s8   q2, d2              \n"// q2(k05-k75)
							"vmovl.s8   q1, d1              \n"// q1(k04-k74)
							"vmovl.s8   q0, d0              \n"// q0(k03-k73)
							"vmovl.s8   q5, d9              \n"// q5(a11-a115)
							"vmovl.s8   q4, d8              \n"// q4(a10-a114)
							"vmovl.s8   q6, d12             \n"// q6(a12-a116)

							"vmlal.s16  q8, d8, d0[0]       \n"// sum0 += (a10-a16) * k03
							"vmlal.s16  q9, d8, d0[1]       \n"// sum1 += (a10-a16) * k13
							"vmlal.s16  q10, d8, d0[2]      \n"// sum2 += (a10-a16) * k23
							"vmlal.s16  q11, d8, d0[3]      \n"// sum3 += (a10-a16) * k33
							"vmlal.s16  q12, d8, d1[0]      \n"// sum4 += (a10-a16) * k43
							"vmlal.s16  q13, d8, d1[1]      \n"// sum5 += (a10-a16) * k53
							"vmlal.s16  q14, d8, d1[2]      \n"// sum6 += (a10-a16) * k63
							"vmlal.s16  q15, d8, d1[3]      \n"// sum7 += (a10-a16) * k73

							"vmlal.s16  q8, d10, d2[0]      \n"// sum0 += (a11-a17) * k04
							"vmlal.s16  q9, d10, d2[1]      \n"// sum1 += (a11-a17) * k14
							"vmlal.s16  q10, d10, d2[2]     \n"// sum2 += (a11-a17) * k24
							"vmlal.s16  q11, d10, d2[3]     \n"// sum3 += (a11-a17) * k34
							"vmlal.s16  q12, d10, d3[0]     \n"// sum4 += (a11-a17) * k44
							"vmlal.s16  q13, d10, d3[1]     \n"// sum5 += (a11-a17) * k54
							"vmlal.s16  q14, d10, d3[2]     \n"// sum6 += (a11-a17) * k64
							"vmlal.s16  q15, d10, d3[3]     \n"// sum7 += (a11-a17) * k74

							"pld        [%11, #64]         \n"
							"vld2.s8    {d8-d9}, [%11]      \n"// d8(a20 a22 a24 a26 a28 a210 a212 a214), d9(a21 a23 a25 a27 a29 a211 a213 a215)
							"add        %11, #8             \n"

							"vmlal.s16  q8, d12, d4[0]      \n"// sum0 += (a12-a18) * k05
							"vmlal.s16  q9, d12, d4[1]      \n"// sum1 += (a12-a18) * k15
							"vmlal.s16  q10, d12, d4[2]     \n"// sum2 += (a12-a18) * k25
							"vmlal.s16  q11, d12, d4[3]     \n"// sum3 += (a12-a18) * k35

							"pld        [%12, #64]         \n"
							"vld1.s8    {d0-d2}, [%12]!     \n"// d0(k06-k76) d1(k07-k77) d2(k08-k78)

							"vmlal.s16  q12, d12, d5[0]     \n"// sum4 += (a12-a18) * k45
							"vmlal.s16  q13, d12, d5[1]     \n"// sum5 += (a12-a18) * k55
							"vmlal.s16  q14, d12, d5[2]     \n"// sum6 += (a12-a18) * k65
							"vmlal.s16  q15, d12, d5[3]     \n"// sum7 += (a12-a18) * k75

															   // r2
							"vext.s8    d12, d8, d8, #1     \n"// d12(a22 a24 a26 a28 x x x x)

							"vmovl.s8   q2, d2              \n"// q2(k08-k78)
							"vmovl.s8   q1, d1              \n"// q1(k07-k77)
							"vmovl.s8   q0, d0              \n"// q0(k06-k76) 
							"vmovl.s8   q5, d9              \n"// q5(a21-a215)
							"vmovl.s8   q4, d8              \n"// q4(a20-a214)
							"vmovl.s8   q6, d12             \n"// q6(a22-a216)

							"vmlal.s16  q8, d8, d0[0]       \n"// sum0 += (a20-a26) * k06
							"vmlal.s16  q9, d8, d0[1]       \n"// sum1 += (a20-a26) * k16
							"vmlal.s16  q10, d8, d0[2]      \n"// sum2 += (a20-a26) * k26
							"vmlal.s16  q11, d8, d0[3]      \n"// sum3 += (a20-a26) * k36
							"vmlal.s16  q12, d8, d1[0]      \n"// sum4 += (a20-a26) * k46
							"vmlal.s16  q13, d8, d1[1]      \n"// sum5 += (a20-a26) * k56
							"vmlal.s16  q14, d8, d1[2]      \n"// sum6 += (a20-a26) * k66
							"vmlal.s16  q15, d8, d1[3]      \n"// sum7 += (a20-a26) * k76

							"vmlal.s16  q8, d10, d2[0]      \n"// sum0 += (a21-a27) * k07
							"vmlal.s16  q9, d10, d2[1]      \n"// sum1 += (a21-a27) * k17
							"vmlal.s16  q10, d10, d2[2]     \n"// sum2 += (a21-a27) * k27
							"vmlal.s16  q11, d10, d2[3]     \n"// sum3 += (a21-a27) * k37
							"vmlal.s16  q12, d10, d3[0]     \n"// sum4 += (a21-a27) * k47
							"vmlal.s16  q13, d10, d3[1]     \n"// sum5 += (a21-a27) * k57
							"vmlal.s16  q14, d10, d3[2]     \n"// sum6 += (a21-a27) * k67
							"vmlal.s16  q15, d10, d3[3]     \n"// sum7 += (a21-a27) * k77

							"vmlal.s16  q8, d12, d4[0]      \n"// sum0 += (a22-a28) * k08
							"vmlal.s16  q9, d12, d4[1]      \n"// sum1 += (a22-a28) * k18
							"vmlal.s16  q10, d12, d4[2]     \n"// sum2 += (a22-a28) * k28
							"vmlal.s16  q11, d12, d4[3]     \n"// sum3 += (a22-a28) * k38
							"vmlal.s16  q12, d12, d5[0]     \n"// sum4 += (a22-a28) * k48
							"vmlal.s16  q13, d12, d5[1]     \n"// sum5 += (a22-a28) * k58
							"vmlal.s16  q14, d12, d5[2]     \n"// sum6 += (a22-a28) * k68
							"vmlal.s16  q15, d12, d5[3]     \n"// sum7 += (a22-a28) * k78

															   // save s32 to memory
							"sub        %12, %12, #72       \n"
							"vst1.s32   {d16-d17}, [%1]!    \n"// out0
							"vst1.s32   {d18-d19}, [%2]!    \n"// out1
							"vst1.s32   {d20-d21}, [%3]!    \n"// out2
							"vst1.s32   {d22-d23}, [%4]!    \n"// out3
							"subs       %0, #1              \n"
							"vst1.s32   {d24-d25}, [%5]!    \n"// out4
							"vst1.s32   {d26-d27}, [%6]!    \n"// out5
							"vst1.s32   {d28-d29}, [%7]!    \n"// out6
							"vst1.s32   {d30-d31}, [%8]!    \n"// out7

							"bne        0b                  \n"
							: "=r"(nn),         // %0
							"=r"(outptr0),    // %1
							"=r"(outptr1),    // %2
							"=r"(outptr2),    // %3
							"=r"(outptr3),    // %4
							"=r"(outptr4),    // %5
							"=r"(outptr5),    // %6
							"=r"(outptr6),    // %7
							"=r"(outptr7),    // %8
							"=r"(r0),         // %9
							"=r"(r1),         // %10
							"=r"(r2),         // %11
							"=r"(ktmp)        // %12
							: "0"(nn),
							"1"(outptr0),
							"2"(outptr1),
							"3"(outptr2),
							"4"(outptr3),
							"5"(outptr4),
							"6"(outptr5),
							"7"(outptr6),
							"8"(outptr7),
							"9"(r0),
							"10"(r1),
							"11"(r2),
							"12"(ktmp)
							: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#else
					int remain = outw;
#endif // __ARM_NEON
					for (; remain > 0; remain--)
					{
#if __ARM_NEON
						asm volatile(
							"pld        [%8, #64]          \n"
							"vld1.s8    {d0}, [%8]         \n"// d0(a00 a01 a02 ....)
							"pld        [%9, #64]          \n"
							"vld1.s8    {d2}, [%9]         \n"// d2(a10 a11 a12 ....)
							"pld        [%10, #64]         \n"
							"vld1.s8    {d4}, [%10]        \n"// d4(a20 a21 a22 ....)

							"pld        [%11, #64]         \n"
							"vld1.s8    {d6-d8}, [%11]!    \n"// d6(k00-k70) d7(k01-k71) d8(k02-k72)

							"vmovl.s8   q0, d0             \n"// d0(a00 a01 a02 x) 
							"vmovl.s8   q1, d2             \n"// d2(a10 a11 a12 x)
							"vmovl.s8   q2, d4             \n"// d4(a20 a21 a22 x)

							"vmovl.s8   q5, d8             \n"// d10(k02-k32) d11(k42-k72)
							"vmovl.s8   q4, d7             \n"// d8(k01-k31) d9(k41-k71)
							"vmovl.s8   q3, d6             \n"// d6(k00-k30) d7(k40-k70)

							"vld1.s32   {d20[0]}, [%0]     \n"// out0 q10
							"vld1.s32   {d20[1]}, [%1]     \n"// out1
							"vld1.s32   {d21[0]}, [%2]     \n"// out2 
							"vld1.s32   {d21[1]}, [%3]     \n"// out3

							"pld        [%11, #64]         \n"
							"vld1.s8    {d24-d26}, [%11]!  \n"
							"vmovl.s8   q14, d26           \n"// d28(k05-k35) d29(k45-k75)
							"vmovl.s8   q13, d25           \n"// d26(k04-k34) d27(k44-k74)
							"vmovl.s8   q12, d24           \n"// d24(k03-k33) d25(k43-k73)

							"vld1.s32   {d22[0]}, [%4]     \n"// out4 q11
							"vld1.s32   {d22[1]}, [%5]     \n"// out5
							"vld1.s32   {d23[0]}, [%6]     \n"// out6
							"vld1.s32   {d23[1]}, [%7]     \n"// out7

							"vmull.s16  q6, d6, d0[0]      \n"// a00 x (k00-k30)
							"vmull.s16  q7, d7, d0[0]      \n"// a00 x (k40-k70)
							"vmull.s16  q8, d8, d0[1]      \n"// a01 x (k01-k31)
							"vmull.s16  q9, d9, d0[1]      \n"// a01 x (k41-k71)
							"vmlal.s16  q10, d10, d0[2]    \n"// a02 x (k02-k32)
							"vmlal.s16  q11, d11, d0[2]    \n"// a02 x (k42-k72)

							"pld        [%11, #64]         \n"
							"vld1.s8    {d6-d8}, [%11]!    \n"
							"vmovl.s8   q5, d8             \n"// d10(k08-k38) d11(k48-k78)
							"vmovl.s8   q4, d7             \n"// d8(k07-k37) d9(k47-k77)
							"vmovl.s8   q3, d6             \n"// d6(k06-k36) d7(k46-k76)

							"vmlal.s16  q6, d24, d2[0]     \n"// a10 x (k03-k33)
							"vmlal.s16  q7, d25, d2[0]     \n"// a10 x (k43-k73)
							"vmlal.s16  q8, d26, d2[1]     \n"// a11 x (k04-k34)
							"vmlal.s16  q9, d27, d2[1]     \n"// a11 x (k44-k74)
							"vmlal.s16  q10, d28, d2[2]    \n"// a12 x (k05-k35)
							"vmlal.s16  q11, d29, d2[2]    \n"// a12 x (k45-k75)

							"vmlal.s16  q6, d6, d4[0]      \n"// a20 x (k06-k36)
							"vmlal.s16  q7, d7, d4[0]      \n"// a20 x (k46-k76)
							"vmlal.s16  q8, d8, d4[1]      \n"// a21 x (k07-k37)
							"vmlal.s16  q9, d9, d4[1]      \n"// a21 x (k47-k77)
							"vmlal.s16  q10, d10, d4[2]    \n"// a22 x (k08-k38)
							"vmlal.s16  q11, d11, d4[2]    \n"// a22 x (k48-k78)

							"vadd.s32   q8, q8, q6         \n"
							"vadd.s32   q9, q9, q7         \n"

							"sub        %11, %11, #72      \n"

							"vadd.s32   q10, q10, q8       \n"
							"vadd.s32   q11, q11, q9       \n"

							"vst1.s32   {d20[0]}, [%0]!    \n"// out0
							"vst1.s32   {d20[1]}, [%1]!    \n"// out1
							"vst1.s32   {d21[0]}, [%2]!    \n"// out2
							"vst1.s32   {d21[1]}, [%3]!    \n"// out3
							"vst1.s32   {d22[0]}, [%4]!    \n"// out4
							"vst1.s32   {d22[1]}, [%5]!    \n"// out5
							"vst1.s32   {d23[0]}, [%6]!    \n"// out6
							"vst1.s32   {d23[1]}, [%7]!    \n"// out7

							: "=r"(outptr0),    // %0
							"=r"(outptr1),    // %1
							"=r"(outptr2),    // %2
							"=r"(outptr3),    // %3
							"=r"(outptr4),    // %4
							"=r"(outptr5),    // %5
							"=r"(outptr6),    // %6
							"=r"(outptr7),    // %7
							"=r"(r0),         // %8
							"=r"(r1),         // %9
							"=r"(r2),         // %10
							"=r"(ktmp)        // %11
							: "0"(outptr0),
							"1"(outptr1),
							"2"(outptr2),
							"3"(outptr3),
							"4"(outptr4),
							"5"(outptr5),
							"6"(outptr6),
							"7"(outptr7),
							"8"(r0),
							"9"(r1),
							"10"(r2),
							"11"(ktmp)
							: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
#else // __ARM_NEON
						int sum0 = 0;
						int sum1 = 0;
						int sum2 = 0;
						int sum3 = 0;
						int sum4 = 0;
						int sum5 = 0;
						int sum6 = 0;
						int sum7 = 0;

						sum0 += (int)r0[0] * ktmp[0];
						sum1 += (int)r0[0] * ktmp[1];
						sum2 += (int)r0[0] * ktmp[2];
						sum3 += (int)r0[0] * ktmp[3];
						sum4 += (int)r0[0] * ktmp[4];
						sum5 += (int)r0[0] * ktmp[5];
						sum6 += (int)r0[0] * ktmp[6];
						sum7 += (int)r0[0] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r0[1] * ktmp[0];
						sum1 += (int)r0[1] * ktmp[1];
						sum2 += (int)r0[1] * ktmp[2];
						sum3 += (int)r0[1] * ktmp[3];
						sum4 += (int)r0[1] * ktmp[4];
						sum5 += (int)r0[1] * ktmp[5];
						sum6 += (int)r0[1] * ktmp[6];
						sum7 += (int)r0[1] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r0[2] * ktmp[0];
						sum1 += (int)r0[2] * ktmp[1];
						sum2 += (int)r0[2] * ktmp[2];
						sum3 += (int)r0[2] * ktmp[3];
						sum4 += (int)r0[2] * ktmp[4];
						sum5 += (int)r0[2] * ktmp[5];
						sum6 += (int)r0[2] * ktmp[6];
						sum7 += (int)r0[2] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r1[0] * ktmp[0];
						sum1 += (int)r1[0] * ktmp[1];
						sum2 += (int)r1[0] * ktmp[2];
						sum3 += (int)r1[0] * ktmp[3];
						sum4 += (int)r1[0] * ktmp[4];
						sum5 += (int)r1[0] * ktmp[5];
						sum6 += (int)r1[0] * ktmp[6];
						sum7 += (int)r1[0] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r1[1] * ktmp[0];
						sum1 += (int)r1[1] * ktmp[1];
						sum2 += (int)r1[1] * ktmp[2];
						sum3 += (int)r1[1] * ktmp[3];
						sum4 += (int)r1[1] * ktmp[4];
						sum5 += (int)r1[1] * ktmp[5];
						sum6 += (int)r1[1] * ktmp[6];
						sum7 += (int)r1[1] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r1[2] * ktmp[0];
						sum1 += (int)r1[2] * ktmp[1];
						sum2 += (int)r1[2] * ktmp[2];
						sum3 += (int)r1[2] * ktmp[3];
						sum4 += (int)r1[2] * ktmp[4];
						sum5 += (int)r1[2] * ktmp[5];
						sum6 += (int)r1[2] * ktmp[6];
						sum7 += (int)r1[2] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r2[0] * ktmp[0];
						sum1 += (int)r2[0] * ktmp[1];
						sum2 += (int)r2[0] * ktmp[2];
						sum3 += (int)r2[0] * ktmp[3];
						sum4 += (int)r2[0] * ktmp[4];
						sum5 += (int)r2[0] * ktmp[5];
						sum6 += (int)r2[0] * ktmp[6];
						sum7 += (int)r2[0] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r2[1] * ktmp[0];
						sum1 += (int)r2[1] * ktmp[1];
						sum2 += (int)r2[1] * ktmp[2];
						sum3 += (int)r2[1] * ktmp[3];
						sum4 += (int)r2[1] * ktmp[4];
						sum5 += (int)r2[1] * ktmp[5];
						sum6 += (int)r2[1] * ktmp[6];
						sum7 += (int)r2[1] * ktmp[7];
						ktmp += 8;

						sum0 += (int)r2[2] * ktmp[0];
						sum1 += (int)r2[2] * ktmp[1];
						sum2 += (int)r2[2] * ktmp[2];
						sum3 += (int)r2[2] * ktmp[3];
						sum4 += (int)r2[2] * ktmp[4];
						sum5 += (int)r2[2] * ktmp[5];
						sum6 += (int)r2[2] * ktmp[6];
						sum7 += (int)r2[2] * ktmp[7];
						ktmp += 8;

						*outptr0 += sum0;
						*outptr1 += sum1;
						*outptr2 += sum2;
						*outptr3 += sum3;
						*outptr4 += sum4;
						*outptr5 += sum5;
						*outptr6 += sum6;
						*outptr7 += sum7;

						ktmp -= 8 * 9;

						outptr0++;
						outptr1++;
						outptr2++;
						outptr3++;
						outptr4++;
						outptr5++;
						outptr6++;
						outptr7++;
#endif // __ARM_NEON
						r0 += 2;
						r1 += 2;
						r2 += 2;
					}

					r0 += tailstep;
					r1 += tailstep;
					r2 += tailstep;
				}

				ktmp += 72; // 8 * 9;
			}
		}

#if ((ENGINE_THREAD_COUNT) != 1)
		nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = remain_outch_start; p < outch; p++)
		{
			int* out = ((int*)data_out) + p * size_out; // top_blob.channel(p);

														// out.fill(0);

			const signed char* ktmp = weight + (p / 8 + p % 8) * (72 * ch_in); // _kernel.channel(p / 8 + p % 8);

			for (int q = 0; q < inch; q++)
			{
				int* outptr = out;

				const signed char* img0 = data_in + q * size_in; // bottom_blob.channel(q);

				const signed char* r0 = img0;
				const signed char* r1 = img0 + w;
				const signed char* r2 = img0 + w * 2;

				int i = 0;

				for (; i < outh; i++)
				{
#if __ARM_NEON
					int nn = outw >> 3;
					int remain = outw & 7;
#else
					int remain = outw;
#endif // __ARM_NEON

#if __ARM_NEON
					if (nn > 0)
					{
						asm volatile(
							"vld1.s8    {d0-d1}, [%5]       \n"// d0(k0 - k7) d1(k8 ...)
							"vmovl.s8   q1, d1              \n"// d2(k8 ...)
							"vmovl.s8   q0, d0              \n"// d0(k0 - k3) d1(k4 - k7)
							"0:                             \n"
							"pld        [%2, #192]          \n"
							"vld2.s8    {d4-d5}, [%2]!      \n"// r0 d4(a00 a02 ... a014) d5(a01 a03 ... a015)
							"vld2.s8    {d8-d9}, [%2]       \n"//    d8(a016 ....)
							"vld2.s8    {d10-d11}, [%3]!    \n"// r1 d10(a10 a12 ... a114) d11(a11 a13 ... a115)
							"vld2.s8    {d14-d15}, [%3]     \n"//    d14(a116 ....)
							"vld2.s8    {d16-d17}, [%4]!    \n"// r2 d16(a20 a22 ... a214) d17(a21 a23 ... a215)
							"vld2.s8    {d20-d21}, [%4]     \n"//    d20(a216 ....)
							"vld1.s32   {d22-d25}, [%1]     \n"// q11(out0 - out3) q12(out4 - out7)

							"vext.s8    d8, d4, d8, #1      \n"//  d8(a02 a04 ... a016)
							"vext.s8    d14, d10, d14, #1   \n"// d14(a12 a14 ... a116)
							"vext.s8    d20, d16, d20, #1   \n"// d20(a22 a24 ... a216)

							"vmovl.s8   q3, d5              \n"// q3(a01 a03 ... a015)
							"vmovl.s8   q2, d4              \n"// q2(a00 a02 ... a014)
							"vmovl.s8   q4, d8              \n"// q4(a02 a04 ... a016)

							"vmovl.s8   q6, d11             \n"// q6(a11 a13 ... a115)
							"vmovl.s8   q5, d10             \n"// q5(a10 a12 ... a114)
							"vmovl.s8   q7, d14             \n"// q7(a12 a14 ... a116)

							"vmovl.s8   q9, d17             \n"// q9(a21 a23 ... a215)
							"vmovl.s8   q8, d16             \n"// q8(a20 a22 ... a214)
							"vmovl.s8   q10, d20            \n"// q10(a22 a24 ... a216)

							"vmlal.s16  q11, d4, d0[0]      \n"// k0
							"vmlal.s16  q12, d5, d0[0]      \n"
							"vmull.s16  q13, d6, d0[1]      \n"// k1
							"vmull.s16  q14, d7, d0[1]      \n"
							"vmlal.s16  q11, d8, d0[2]      \n"// k2
							"vmlal.s16  q12, d9, d0[2]      \n"

							"vmlal.s16  q13, d12, d1[0]     \n"// k4
							"vmlal.s16  q14, d13, d1[0]     \n"
							"vmlal.s16  q11, d10, d0[3]     \n"// k3
							"vmlal.s16  q12, d11, d0[3]     \n"
							"vmlal.s16  q13, d14, d1[1]     \n"// k5
							"vmlal.s16  q14, d15, d1[1]     \n"

							"vmlal.s16  q11, d16, d1[2]     \n"// k6
							"vmlal.s16  q12, d17, d1[2]     \n"
							"vmlal.s16  q13, d18, d1[3]     \n"// k7 
							"vmlal.s16  q14, d19, d1[3]     \n"
							"vmlal.s16  q11, d20, d2[0]     \n"// k8 
							"vmlal.s16  q12, d21, d2[0]     \n"

							"vadd.s32   q11, q11, q13       \n"
							"vadd.s32   q12, q12, q14       \n"

							"vst1.32    {d22-d25}, [%1]!    \n"

							"subs       %0, #1              \n"
							"bne        0b                  \n"
							: "=r"(nn),     // %0
							"=r"(outptr), // %1
							"=r"(r0),     // %2
							"=r"(r1),     // %3
							"=r"(r2),     // %4
							"=r"(ktmp)    // %5
							: "0"(nn),
							"1"(outptr),
							"2"(r0),
							"3"(r1),
							"4"(r2),
							"5"(ktmp)
							: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
							);
					}
#endif // __ARM_NEON
					if (remain > 0)
					{
#if __ARM_NEON
						int8x8_t _k01234567s8 = vld1_s8(ktmp);
						int8x8_t _k8xxxxxxxs8 = vld1_s8(ktmp + 8);
						int8x8_t _k34567xxxs8 = vext_s8(_k01234567s8, _k01234567s8, 3);
						int8x8_t _k678xxxxxs8 = vext_s8(_k01234567s8, _k8xxxxxxxs8, 6);
						int16x8_t _k0123_s16 = vmovl_s8(_k01234567s8);
						int16x8_t _k3456_s16 = vmovl_s8(_k34567xxxs8);
						int16x8_t _k678x_s16 = vmovl_s8(_k678xxxxxs8);
#endif
						for (; remain > 0; remain--)
						{
#if __ARM_NEON
							int8x8_t _r00s8 = vld1_s8(r0);
							int8x8_t _r10s8 = vld1_s8(r1);
							int8x8_t _r20s8 = vld1_s8(r2);

							int16x8_t _r00s16 = vmovl_s8(_r00s8);
							int16x8_t _r10s16 = vmovl_s8(_r10s8);
							int16x8_t _r20s16 = vmovl_s8(_r20s8);

							int32x4_t _sum = vmull_s16(vget_low_s16(_r00s16), vget_low_s16(_k0123_s16));
							_sum = vmlal_s16(_sum, vget_low_s16(_r10s16), vget_low_s16(_k3456_s16));
							_sum = vmlal_s16(_sum, vget_low_s16(_r20s16), vget_low_s16(_k678x_s16));

							_sum = vsetq_lane_s32(*outptr, _sum, 3);

							int32x2_t _ss = vadd_s32(vget_low_s32(_sum), vget_high_s32(_sum));
							_ss = vpadd_s32(_ss, _ss);

							*outptr = vget_lane_s32(_ss, 0);
#else
							int sum = 0;

							sum += (int)r0[0] * ktmp[0];
							sum += (int)r0[1] * ktmp[1];
							sum += (int)r0[2] * ktmp[2];
							sum += (int)r1[0] * ktmp[3];
							sum += (int)r1[1] * ktmp[4];
							sum += (int)r1[2] * ktmp[5];
							sum += (int)r2[0] * ktmp[6];
							sum += (int)r2[1] * ktmp[7];
							sum += (int)r2[2] * ktmp[8];

							*outptr += sum;
#endif // __ARM_NEON
							r0 += 2;
							r1 += 2;
							r2 += 2;
							outptr++;
						}
					}

					r0 += tailstep;
					r1 += tailstep;
					r2 += tailstep;
				}

				ktmp += 9;
			}
		}

#if ((ENGINE_THREAD_COUNT) != 1)
		nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < ch_out; p++)
		{
			float* outptr = data_out + p * size_out;
			float scale = mem_scale[p];
			float bias = mem_bias[p];

#if __ARM_NEON
			int nn = size_out >> 2;
			int remain = size_out & 3;
#else
			int remain = size_out;
#endif // __ARM_NEON

#if __ARM_NEON
			if (nn > 0)
			{
				asm volatile(
					"vdup.f32   q1, %2              \n"
					"vdup.f32   q2, %3              \n"
					"veor       q4, q4, q4          \n"

					"0:                             \n"
					"pld        [%1, #128]          \n"
					"vld1.s32	{d0-d1}, [%1] 		\n"
					"vmov.32    q3, q2				\n"
					"vcvt.f32.s32 q0, q0			\n"
					"vmla.f32   q3, q0, q1          \n"
					"vmax.f32   q3, q3, q4          \n"

					"vst1.f32   {d6-d7}, [%1]!		\n"
					"subs       %0, #1              \n"
					"bne        0b                  \n"
					: "+r"(nn),				// %0
					"+r"(outptr)			// %1
					: "r"(scale),			// %2
					"r"(bias)				// %3
					: "cc", "memory", "q0", "q1", "q2", "q3", "q4"
					);
			}
#endif // __ARM_NEON
			for (; remain > 0; remain--)
			{
				float val = *((int*)outptr) * scale + bias;
				if (val < 0.0f) val = 0.0f;
				*outptr = val;
				outptr++;
			}
		}
	}

	// static void conv_im2col_sgemm_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat & kernel_tm, const int kernel_w, const int kernel_h, const int stride_w, const int stride_h, const Option& opt)
	// conv_im2col_sgemm_transform_kernel_int8_neon
	// (8 * kernel_size) * inch * (out_size/8 + out_size%8)
	void conv_kxsx_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;
		int inch = ch_in;

		int outw = dim_out;
		int outh = dim_out;
		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

		int tm_cstep = 8 * ch_in * dim_kernel * dim_kernel;
		int kernel_cstep = 4 * dim_kernel * dim_kernel * ch_in;

		// im2col
		// bottom_im2col memory packed 8 x 8
		{
			int i, ic, kx, ky;
			int x, x0, x1, x2, x3, x4, x5, x6, x7;
			int y, y0, y1, y2, y3, y4, y5, y6, y7;
			int xy0, xy1, xy2, xy3, xy4, xy5, xy6, xy7;

			int nn = size_out >> 3;
			int remain = nn << 3;

			for (x = 0, y = 0, i = 0; i < nn; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + i * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x1 = x; y1 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x2 = x; y2 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x3 = x; y3 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x4 = x; y4 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x5 = x; y5 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x6 = x; y6 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x7 = x; y7 = y; x++; if (x >= dim_out) { x = 0; y++; }

				xy0 = (x0 + y0 * dim_in) * stride;
				xy1 = (x1 + y1 * dim_in) * stride;
				xy2 = (x2 + y2 * dim_in) * stride;
				xy3 = (x3 + y3 * dim_in) * stride;
				xy4 = (x4 + y4 * dim_in) * stride;
				xy5 = (x5 + y5 * dim_in) * stride;
				xy6 = (x6 + y6 * dim_in) * stride;
				xy7 = (x7 + y7 * dim_in) * stride;

				const signed char* imin_iter_ic = data_in;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < ch_in; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							*tm_iter++ = imin_iter_kx[xy1];
							*tm_iter++ = imin_iter_kx[xy2];
							*tm_iter++ = imin_iter_kx[xy3];
							*tm_iter++ = imin_iter_kx[xy4];
							*tm_iter++ = imin_iter_kx[xy5];
							*tm_iter++ = imin_iter_kx[xy6];
							*tm_iter++ = imin_iter_kx[xy7];
							imin_iter_kx++;
						}
						imin_iter_ky += dim_in;
					}
					imin_iter_ic += size_in;
				}
			}

			for (i = remain; i < size_out; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + (i / 8 + i % 8) * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= dim_out) { x = 0; y++; }
				xy0 = (x0 + y0 * dim_in) * stride;

				const signed char* imin_iter_ic = data_in;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < ch_in; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							imin_iter_kx++;
						}
						imin_iter_ky += dim_in;
					}
					imin_iter_ic += size_in;
				}
			}
		}

		// sgemm(int M, int N, int L, float* A, float* B, float* C)
		{
			//int M = outch;  // outch
			int N = outw * outh; // outsize or out stride
			int L = dim_kernel * dim_kernel * inch; // ksize * inch

			int nn_outch = 0;
			int remain_outch_start = 0;

			nn_outch = (outch - remain_outch_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int pp = 0; pp < nn_outch; pp++)
			{
				int i = remain_outch_start + pp * 4;

				signed char* output0 = data_out + (i + 0) * size_out; // top_blob.channel(i);
				signed char* output1 = data_out + (i + 1) * size_out; // top_blob.channel(i + 1);
				signed char* output2 = data_out + (i + 2) * size_out; // top_blob.channel(i + 2);
				signed char* output3 = data_out + (i + 3) * size_out; // top_blob.channel(i + 3);

				int j = 0;
				for (; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// K loop
						"vmov.s32    q8, #0             \n"
						"vmov.s32    q9, #0             \n"
						"vmov.s32    q10, #0            \n"
						"vmov.s32    q11, #0            \n"
						"vmov.s32    q12, #0            \n"
						"vmov.s32    q13, #0            \n"
						"vmov.s32    q14, #0            \n"
						"vmov.s32    q15, #0            \n"

						"lsr         r4, %6, #3         \n"// r4 = nn = L >> 3
						"cmp         r4, #0             \n"
						"beq         1f                 \n"

						"0:                             \n"// for(; nn != 0; nn--)
						"pld         [%4, #128]         \n"
						"vld1.s8     {d8-d11}, [%4]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q7, d11            \n"// a30-a37
						"vmovl.s8    q6, d10            \n"// a20-a27                    
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"pld         [%5, #128]         \n"
						"vld1.s8     {d0-d3}, [%5]!     \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q3, d3             \n"// k06-k36,k07-k37
						"vmovl.s8    q2, d2             \n"// k04-k34,k05-k35
						"vmovl.s8    q1, d1             \n"// k02-k32,k03-k33
						"vmovl.s8    q0, d0             \n"// k00-k30,k01-k31

						"vmlal.s16   q8, d8, d0[0]      \n"// sum0 = (a00-a07) * k00
						"vmlal.s16   q9, d9, d0[0]      \n"
						"vmlal.s16   q10, d8, d0[1]     \n"// sum1 = (a00-a07) * k10
						"vmlal.s16   q11, d9, d0[1]     \n"
						"vmlal.s16   q12, d8, d0[2]     \n"// sum2 = (a00-a07) * k20
						"vmlal.s16   q13, d9, d0[2]     \n"
						"vmlal.s16   q14, d8, d0[3]     \n"// sum3 = (a00-a07) * k30
						"vmlal.s16   q15, d9, d0[3]     \n"

						"vmlal.s16   q8, d10, d1[0]     \n"// sum0 += (a10-a17) * k01
						"vmlal.s16   q9, d11, d1[0]     \n"
						"vmlal.s16   q10, d10, d1[1]    \n"// sum1 += (a10-a17) * k11
						"vmlal.s16   q11, d11, d1[1]    \n"
						"vmlal.s16   q12, d10, d1[2]    \n"// sum2 += (a10-a17) * k21
						"vmlal.s16   q13, d11, d1[2]    \n"
						"vmlal.s16   q14, d10, d1[3]    \n"// sum3 += (a10-a17) * k31
						"vmlal.s16   q15, d11, d1[3]    \n"

						"pld         [%4, #128]         \n"
						"vld1.s8     {d8-d9}, [%4]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"vmlal.s16   q8, d12, d2[0]     \n"// sum0 += (a20-a27) * k02
						"vmlal.s16   q9, d13, d2[0]     \n"
						"vmlal.s16   q10, d12, d2[1]    \n"// sum1 += (a20-a27) * k12
						"vmlal.s16   q11, d13, d2[1]    \n"
						"vmlal.s16   q12, d12, d2[2]    \n"// sum2 += (a20-a27) * k22
						"vmlal.s16   q13, d13, d2[2]    \n"
						"vmlal.s16   q14, d12, d2[3]    \n"// sum3 += (a20-a27) * k32
						"vmlal.s16   q15, d13, d2[3]    \n"

						"vmlal.s16   q8, d14, d3[0]     \n"// sum0 += (a30-a37) * k03
						"vmlal.s16   q9, d15, d3[0]     \n"
						"vmlal.s16   q10, d14, d3[1]    \n"// sum1 += (a30-a37) * k13
						"vmlal.s16   q11, d15, d3[1]    \n"
						"vmlal.s16   q12, d14, d3[2]    \n"// sum2 += (a30-a37) * k23
						"vmlal.s16   q13, d15, d3[2]    \n"
						"vmlal.s16   q14, d14, d3[3]    \n"// sum3 += (a30-a37) * k33
						"vmlal.s16   q15, d15, d3[3]    \n"

						"pld         [%4, #128]         \n"
						"vld1.s8     {d0-d1}, [%4]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q1, d1             \n"// a10-a17
						"vmovl.s8    q0, d0             \n"// a00-a07

						"vmlal.s16   q8, d8, d4[0]      \n"// sum0 += (a40-a47) * k04
						"vmlal.s16   q9, d9, d4[0]      \n"
						"vmlal.s16   q10, d8, d4[1]     \n"// sum1 += (a40-a47) * k14
						"vmlal.s16   q11, d9, d4[1]     \n"
						"vmlal.s16   q12, d8, d4[2]     \n"// sum2 += (a40-a47) * k24
						"vmlal.s16   q13, d9, d4[2]     \n"
						"vmlal.s16   q14, d8, d4[3]     \n"// sum3 += (a40-a47) * k34
						"vmlal.s16   q15, d9, d4[3]     \n"

						"vmlal.s16   q8, d10, d5[0]     \n"// sum0 += (a50-a57) * k05
						"vmlal.s16   q9, d11, d5[0]     \n"
						"vmlal.s16   q10, d10, d5[1]    \n"// sum1 += (a50-a57) * k15
						"vmlal.s16   q11, d11, d5[1]    \n"
						"vmlal.s16   q12, d10, d5[2]    \n"// sum2 += (a50-a57) * k25
						"vmlal.s16   q13, d11, d5[2]    \n"
						"vmlal.s16   q14, d10, d5[3]    \n"// sum3 += (a50-a57) * k35
						"vmlal.s16   q15, d11, d5[3]    \n"

						"vmlal.s16   q8, d0, d6[0]      \n"// sum0 += (a60-a67) * k06
						"vmlal.s16   q9, d1, d6[0]      \n"
						"vmlal.s16   q10, d0, d6[1]     \n"// sum1 += (a60-a67) * k16
						"vmlal.s16   q11, d1, d6[1]     \n"
						"vmlal.s16   q12, d0, d6[2]     \n"// sum2 += (a60-a67) * k26
						"vmlal.s16   q13, d1, d6[2]     \n"
						"vmlal.s16   q14, d0, d6[3]     \n"// sum3 += (a60-a67) * k36
						"vmlal.s16   q15, d1, d6[3]     \n"

						"vmlal.s16   q8, d2, d7[0]      \n"// sum0 += (a70-a77) * k07
						"vmlal.s16   q9, d3, d7[0]      \n"
						"vmlal.s16   q10, d2, d7[1]     \n"// sum1 += (a70-a77) * k17
						"vmlal.s16   q11, d3, d7[1]     \n"
						"vmlal.s16   q12, d2, d7[2]     \n"// sum2 += (a70-a77) * k27
						"vmlal.s16   q13, d3, d7[2]     \n"
						"vmlal.s16   q14, d2, d7[3]     \n"// sum3 += (a70-a77) * k37
						"vmlal.s16   q15, d3, d7[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         0b                 \n"// end for

						"1:                             \n"
						// remain loop
						"and         r4, %6, #7         \n"// r4 = remain = inch & 7
						"cmp         r4, #0             \n"
						"beq         3f                 \n"

						"2:                             \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%4]!        \n"// tmpr a00-a70    a(inch)(data)
						"vld1.s8     {d0}, [%5]         \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2             \n"
						"vmovl.s8    q0, d0             \n"
						"add         %5, #4             \n"

						"vmlal.s16   q8, d2, d0[0]      \n"// sum0 += (a00-a70) * k00
						"vmlal.s16   q9, d3, d0[0]      \n"
						"vmlal.s16   q10, d2, d0[1]     \n"// sum1 += (a00-a70) * k10
						"vmlal.s16   q11, d3, d0[1]     \n"
						"vmlal.s16   q12, d2, d0[2]     \n"// sum2 += (a00-a70) * k20
						"vmlal.s16   q13, d3, d0[2]     \n"
						"vmlal.s16   q14, d2, d0[3]     \n"// sum3 += (a00-a70) * k30
						"vmlal.s16   q15, d3, d0[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         2b                 \n"

						"3:                             \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%7]      \n"

						"vcvt.f32.s32 q8, q8            \n"
						"vcvt.f32.s32 q9, q9            \n"
						"vcvt.f32.s32 q10, q10          \n"
						"vcvt.f32.s32 q11, q11          \n"

						"vdup.32     q1, %8             \n"
						"vdup.32     q3, %9             \n"
						"vmov.32     q2, q1             \n"
						"vmov.32     q4, q3             \n"

						"vmla.f32    q1, q8, d0[0]      \n"
						"vmla.f32    q2, q9, d0[0]      \n"
						"vmla.f32    q3, q10, d0[1]     \n"
						"vmla.f32    q4, q11, d0[1]     \n"

						"vcvt.f32.s32 q12, q12          \n"
						"vcvt.f32.s32 q13, q13          \n"
						"vcvt.f32.s32 q14, q14          \n"
						"vcvt.f32.s32 q15, q15          \n"

						"vdup.32     q5, %10            \n"
						"vdup.32     q7, %11            \n"
						"vmov.32     q6, q5             \n"
						"vmov.32     q8, q7             \n"

						"vmla.f32    q5, q12, d1[0]    \n"
						"vmla.f32    q6, q13, d1[0]    \n"
						"vmla.f32    q7, q14, d1[1]    \n"
						"vmla.f32    q8, q15, d1[1]    \n"

						"veor       q9, q9              \n"

						"vcvt.s32.f32 q1, q1            \n"
						"vcvt.s32.f32 q2, q2            \n"
						"vcvt.s32.f32 q3, q3            \n"
						"vcvt.s32.f32 q4, q4            \n"
						"vcvt.s32.f32 q5, q5            \n"
						"vcvt.s32.f32 q6, q6            \n"
						"vcvt.s32.f32 q7, q7            \n"
						"vcvt.s32.f32 q8, q8            \n"

						"vqmovn.s32 d0, q1              \n"
						"vqmovn.s32 d1, q2              \n"
						"vqmovn.s32 d2, q3              \n"
						"vqmovn.s32 d3, q4              \n"
						"vqmovn.s32 d4, q5              \n"
						"vqmovn.s32 d5, q6              \n"
						"vqmovn.s32 d6, q7              \n"
						"vqmovn.s32 d7, q8              \n"

						"vqmovn.s16 d0, q0              \n"
						"vqmovn.s16 d1, q1              \n"
						"vqmovn.s16 d2, q2              \n"
						"vqmovn.s16 d3, q3              \n"

						"vmax.s8    q0, q0, q9          \n"
						"vmax.s8    q1, q1, q9          \n"

						"vst1.s32    {d0}, [%0]         \n"
						"vst1.s32    {d1}, [%1]         \n"
						"vst1.s32    {d2}, [%2]         \n"
						"vst1.s32    {d3}, [%3]         \n"

						: "+r"(output0), // %0
						"+r"(output1), // %1
						"+r"(output2), // %2
						"+r"(output3), // %3
						"+r"(vb),      // %4
						"+r"(va)       // %5
						: "r"(L),        // %6  
						"r"(mem_scale + i),   // %7
						"r"(mem_bias[i]),     // %8
						"r"(mem_bias[i + 1]),   // %9
						"r"(mem_bias[i + 2]),   // %10
						"r"(mem_bias[i + 3])    // %11
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
#else
					int sum0[8] = { 0 };
					int sum1[8] = { 0 };
					int sum2[8] = { 0 };
					int sum3[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 8];
							sum1[n] += (int)va[1] * vb[n + 8];
							sum2[n] += (int)va[2] * vb[n + 8];
							sum3[n] += (int)va[3] * vb[n + 8];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 16];
							sum1[n] += (int)va[1] * vb[n + 16];
							sum2[n] += (int)va[2] * vb[n + 16];
							sum3[n] += (int)va[3] * vb[n + 16];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 24];
							sum1[n] += (int)va[1] * vb[n + 24];
							sum2[n] += (int)va[2] * vb[n + 24];
							sum3[n] += (int)va[3] * vb[n + 24];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 32];
							sum1[n] += (int)va[1] * vb[n + 32];
							sum2[n] += (int)va[2] * vb[n + 32];
							sum3[n] += (int)va[3] * vb[n + 32];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 40];
							sum1[n] += (int)va[1] * vb[n + 40];
							sum2[n] += (int)va[2] * vb[n + 40];
							sum3[n] += (int)va[3] * vb[n + 40];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 48];
							sum1[n] += (int)va[1] * vb[n + 48];
							sum2[n] += (int)va[2] * vb[n + 48];
							sum3[n] += (int)va[3] * vb[n + 48];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 56];
							sum1[n] += (int)va[1] * vb[n + 56];
							sum2[n] += (int)va[2] * vb[n + 56];
							sum3[n] += (int)va[3] * vb[n + 56];
							va -= 28;
						}

						va += 32;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
						}

						va += 4;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						int val;
						val = (int)(sum0[n] * mem_scale[i + 0] + mem_bias[i + 0]); if (val < 0) val = 0; if (val > 127) val = 127; output0[n] = (signed char)val;
						val = (int)(sum1[n] * mem_scale[i + 1] + mem_bias[i + 1]); if (val < 0) val = 0; if (val > 127) val = 127; output1[n] = (signed char)val;
						val = (int)(sum2[n] * mem_scale[i + 2] + mem_bias[i + 2]); if (val < 0) val = 0; if (val > 127) val = 127; output2[n] = (signed char)val;
						val = (int)(sum3[n] * mem_scale[i + 3] + mem_bias[i + 3]); if (val < 0) val = 0; if (val > 127) val = 127; output3[n] = (signed char)val;
					}
#endif // __ARM_NEON
					output0 += 8;
					output1 += 8;
					output2 += 8;
					output3 += 8;
				}

				for (; j < N; j++)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8 + j % 8) * tm_cstep; // bottom_tm.channel(j / 8 + j % 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// inch loop
						"veor        q6, q6, q6        \n"
						"veor        q7, q7, q7        \n"
						"veor        q8, q8, q8        \n"
						"veor        q9, q9, q9        \n"
						"veor        q10, q10, q10     \n"
						"veor        q11, q11, q11     \n"
						"veor        q12, q12, q12     \n"
						"veor        q13, q13, q13     \n"
						"vmov.s32    q14, #0           \n"

						"lsr         r4, %6, #3        \n"// r4 = nn = L >> 2
						"cmp         r4, #0            \n"
						"beq         1f                \n"

						"0:                            \n"// for(; nn != 0; nn--)
						"pld         [%4, #128]        \n"
						"vld1.s8     {d0}, [%4]!       \n"// tmpr a00,a10,a20,a30    a(inch)(data)
						"vmovl.s8    q0, d0            \n"// a00-a07

						"pld         [%5, #128]        \n"
						"vld1.s8     {d2-d5}, [%5]!    \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q4, d5            \n"// k06-k36,k07-k37
						"vmovl.s8    q3, d4            \n"// k04-k34,k05-k35
						"vmovl.s8    q2, d3            \n"// k02-k32,k03-k33
						"vmovl.s8    q1, d2            \n"// k00-k30,k01-k31

						"vmlal.s16   q6, d2, d0[0]     \n"// (k00-k30) * a00
						"vmlal.s16   q7, d3, d0[1]     \n"// (k01-k31) * a01
						"vmlal.s16   q8, d4, d0[2]     \n"// (k02-k32) * a02
						"vmlal.s16   q9, d5, d0[3]     \n"// (k03-k33) * a03
						"vmlal.s16   q10, d6, d1[0]    \n"// (k04-k34) * a04
						"vmlal.s16   q11, d7, d1[1]    \n"// (k05-k35) * a05
						"vmlal.s16   q12, d8, d1[2]    \n"// (k06-k36) * a06
						"vmlal.s16   q13, d9, d1[3]    \n"// (k07-k37) * a07                    

						"subs        r4, r4, #1        \n"
						"bne         0b                \n"// end for

						"vadd.s32    q6, q6, q7        \n"
						"vadd.s32    q9, q9, q8        \n"
						"vadd.s32    q11, q11, q10     \n"
						"vadd.s32    q13, q13, q12     \n"

						"vadd.s32    q9, q9, q6        \n"
						"vadd.s32    q13, q13, q11     \n"
						"vadd.s32    q14, q13, q9      \n"

						"1:                            \n"
						// remain loop
						"and         r4, %6, #7        \n"// r4 = remain = inch & 3
						"cmp         r4, #0            \n"
						"beq         3f                \n"

						"2:                            \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%4]        \n"// tmpr a00        a(inch)(data)
						"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2            \n"
						"vmovl.s8    q0, d0            \n"
						"add         %4, #1            \n"
						"add         %5, #4            \n"

						"vmlal.s16   q14, d0, d2[0]    \n"

						"subs        r4, r4, #1        \n"
						"bne         2b                \n"

						"3:                            \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%7]     \n"
						"vld1.f32    {d2-d3}, [%8]     \n"

						"vcvt.f32.s32 q14, q14         \n"

						"vmla.f32    q1, q14, q0       \n"

						"vcvt.s32.f32 q1, q1           \n"
						"veor       d4, d4, d4         \n"
						"vqmovn.s32 d0, q1             \n"
						"vqmovn.s16 d0, q0             \n"

						"vmax.s8    d0, d0, d4         \n"

						"vst1.s8     {d0[0]}, [%0]     \n"
						"vst1.s8     {d0[1]}, [%1]     \n"
						"vst1.s8     {d0[2]}, [%2]     \n"
						"vst1.s8     {d0[3]}, [%3]     \n"

						: "+r"(output0), // %0
						"+r"(output1), // %1
						"+r"(output2), // %2
						"+r"(output3), // %3
						"+r"(vb),      // %4
						"+r"(va)       // %5
						: "r"(L),        // %6  
						"r"(mem_scale + i),  // %7
						"r"(mem_bias + i)    // %8
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14"
						);
#else
					int sum0 = 0;
					int sum1 = 0;
					int sum2 = 0;
					int sum3 = 0;

					for (int k = 0; k < L; k++)
					{
						sum0 += (int)va[0] * vb[0];
						sum1 += (int)va[1] * vb[0];
						sum2 += (int)va[2] * vb[0];
						sum3 += (int)va[3] * vb[0];

						va += 4;
						vb += 1;
					}

					int val;
					val = (int)(sum0 * mem_scale[i + 0] + mem_bias[i + 0]); if (val < 0) val = 0; if (val > 127) val = 127; output0[0] = (signed char)val;
					val = (int)(sum1 * mem_scale[i + 1] + mem_bias[i + 1]); if (val < 0) val = 0; if (val > 127) val = 127; output1[0] = (signed char)val;
					val = (int)(sum2 * mem_scale[i + 2] + mem_bias[i + 2]); if (val < 0) val = 0; if (val > 127) val = 127; output2[0] = (signed char)val;
					val = (int)(sum3 * mem_scale[i + 3] + mem_bias[i + 3]); if (val < 0) val = 0; if (val > 127) val = 127; output3[0] = (signed char)val;
#endif // __ARM_NEON
					output0++;
					output1++;
					output2++;
					output3++;
				}
			}

			remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = remain_outch_start; i < outch; i++)
			{
				signed char* output = data_out + i * size_out; // top_blob.channel(i);

				int j = 0;
				for (; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4 + i % 4) * kernel_cstep; // kernel_tm.channel(i / 4 + i % 4);

#if __ARM_NEON
					asm volatile(
						// inch loop
						"vmov.s32    q6, #0            \n"
						"vmov.s32    q7, #0            \n"

						"lsr         r4, %3, #3        \n"// r4 = nn = inch >> 3
						"cmp         r4, #0            \n"
						"beq         1f                \n"

						"0:                            \n"// for(; nn != 0; nn--)
						"pld         [%1, #128]        \n"
						"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d7            \n"// a30-a37
						"vmovl.s8    q4, d6            \n"// a20-a27
						"vmovl.s8    q3, d5            \n"// a10-a17
						"vmovl.s8    q2, d4            \n"// a00-a07

						"pld         [%2, #128]        \n"
						"vld1.s8     {d0}, [%2]!       \n"// kptr k00-k07    k(outch)(inch)
						"vmovl.s8    q1, d1            \n"// k04,k05,k06,k07
						"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03

						"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d5, d0[0]     \n"
						"vmlal.s16   q6, d6, d0[1]     \n"// (a10-a17) * k01
						"vmlal.s16   q7, d7, d0[1]     \n"
						"vmlal.s16   q6, d8, d0[2]     \n"// (a20-a27) * k02
						"vmlal.s16   q7, d9, d0[2]     \n"
						"vmlal.s16   q6, d10, d0[3]    \n"// (a30-a37) * k03
						"vmlal.s16   q7, d11, d0[3]    \n"

						"pld         [%1, #128]        \n"
						"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a40-a47,a50-a57,a60-a67,a70-a77    a(inch)(data)
						"vmovl.s8    q5, d7            \n"// a70-a77
						"vmovl.s8    q4, d6            \n"// a60-a67
						"vmovl.s8    q3, d5            \n"// a50-a57
						"vmovl.s8    q2, d4            \n"// a40-a47

						"vmlal.s16   q6, d4, d1[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d5, d1[0]     \n"
						"vmlal.s16   q6, d6, d1[1]     \n"// (a10-a17) * k01
						"vmlal.s16   q7, d7, d1[1]     \n"
						"vmlal.s16   q6, d8, d1[2]     \n"// (a20-a27) * k02
						"vmlal.s16   q7, d9, d1[2]     \n"
						"vmlal.s16   q6, d10, d1[3]    \n"// (a30-a37) * k03
						"vmlal.s16   q7, d11, d1[3]    \n"

						"subs        r4, r4, #1        \n"
						"bne         0b                \n"// end for

						"1:                            \n"
						// remain loop
						"and         r4, %3, #7        \n"// r4 = remain = inch & 7
						"cmp         r4, #0            \n"
						"beq         3f                \n"

						"2:                            \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%1]!       \n"// tmpr a00-a07    a(inch)(data)
						"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
						"vmovl.s8    q1, d2            \n"
						"vmovl.s8    q0, d0            \n"
						"add         %2, #1            \n"

						"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d3, d0[0]     \n"

						"subs        r4, r4, #1        \n"
						"bne         2b                \n"

						"3:                            \n"// store the result to memory

						"vdup.32   q0, %4              \n"
						"vdup.32   q1, %5              \n"
						"vmov.32   q2, q1              \n"

						"vcvt.f32.s32  q6, q6          \n"
						"vcvt.f32.s32  q7, q7          \n"

						"vmla.f32   q1, q6, q0         \n"
						"vmla.f32   q2, q7, q0         \n"

						"vcvt.s32.f32 q1, q1           \n"
						"vcvt.s32.f32 q2, q2           \n"

						"veor       d6, d6, d6         \n"

						"vqmovn.s32  d0, q1            \n"
						"vqmovn.s32  d1, q2            \n"

						"vqmovn.s16  d0, q0            \n"
						"vmax.s8     d0, d0, d6        \n"

						"vst1.s32    {d0}, [%0]        \n"

						: "+r"(output), // %0
						"+r"(vb),     // %1
						"+r"(va)      // %2
						: "r"(L),       // %3  
						"r"(mem_scale[i]), // %4
						"r"(mem_bias[i])   // %5
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7"
						);
#else                 
					int sum[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum[n] += (int)va[0] * vb[n];
							sum[n] += (int)va[1] * vb[n + 8];
							sum[n] += (int)va[2] * vb[n + 16];
							sum[n] += (int)va[3] * vb[n + 24];
							sum[n] += (int)va[4] * vb[n + 32];
							sum[n] += (int)va[5] * vb[n + 40];
							sum[n] += (int)va[6] * vb[n + 48];
							sum[n] += (int)va[7] * vb[n + 56];
						}

						va += 8;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum[n] += (int)va[0] * vb[n];
						}

						va += 1;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						int val;
						val = (int)(sum[n] * mem_scale[i] + mem_bias[i]); if (val < 0) val = 0; if (val > 127) val = 127; output[n] = (signed char)val;
					}
#endif // __ARM_NEON
					output += 8;
				}

				for (; j < N; j++)
				{
					int sum = 0;

					signed char* vb = ((signed char*)mem_tm) + (j / 8 + j % 8) * tm_cstep; // bottom_tm.channel(j / 8 + j % 8);
					const signed char* va = weight + (i / 4 + i % 4) * kernel_cstep; // kernel_tm.channel(i / 4 + i % 4);

					for (int k = 0; k < L; k++)
					{
						sum += (int)va[0] * vb[0];

						va += 1;
						vb += 1;
					}
					int val;
					val = (int)(sum * mem_scale[i] + mem_bias[i]); if (val < 0) val = 0; if (val > 127) val = 127; output[0] = (signed char)val;
					output++;
				}
			}
		}
	}

	// static void conv_im2col_sgemm_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat & kernel_tm, const int kernel_w, const int kernel_h, const int stride_w, const int stride_h, const Option& opt)
	// conv_im2col_sgemm_transform_kernel_int8_neon
	// (8 * kernel_size) * inch * (out_size/8 + out_size%8)
	void conv_kxsx_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = dim_in;
		int h = dim_in;
		int inch = ch_in;

		int outw = dim_out;
		int outh = dim_out;
		int outch = ch_out;

		int size_in = w * h;
		int size_out = outw * outh;

		int tm_cstep = 8 * ch_in * dim_kernel * dim_kernel;
		int kernel_cstep = 4 * dim_kernel * dim_kernel * ch_in;

		// im2col
		// bottom_im2col memory packed 8 x 8
		{
			int i, ic, kx, ky;
			int x, x0, x1, x2, x3, x4, x5, x6, x7;
			int y, y0, y1, y2, y3, y4, y5, y6, y7;
			int xy0, xy1, xy2, xy3, xy4, xy5, xy6, xy7;

			int nn = size_out >> 3;
			int remain = nn << 3;

			for (x = 0, y = 0, i = 0; i < nn; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + i * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x1 = x; y1 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x2 = x; y2 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x3 = x; y3 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x4 = x; y4 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x5 = x; y5 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x6 = x; y6 = y; x++; if (x >= dim_out) { x = 0; y++; }
				x7 = x; y7 = y; x++; if (x >= dim_out) { x = 0; y++; }

				xy0 = (x0 + y0 * dim_in) * stride;
				xy1 = (x1 + y1 * dim_in) * stride;
				xy2 = (x2 + y2 * dim_in) * stride;
				xy3 = (x3 + y3 * dim_in) * stride;
				xy4 = (x4 + y4 * dim_in) * stride;
				xy5 = (x5 + y5 * dim_in) * stride;
				xy6 = (x6 + y6 * dim_in) * stride;
				xy7 = (x7 + y7 * dim_in) * stride;

				const signed char* imin_iter_ic = data_in;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < ch_in; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							*tm_iter++ = imin_iter_kx[xy1];
							*tm_iter++ = imin_iter_kx[xy2];
							*tm_iter++ = imin_iter_kx[xy3];
							*tm_iter++ = imin_iter_kx[xy4];
							*tm_iter++ = imin_iter_kx[xy5];
							*tm_iter++ = imin_iter_kx[xy6];
							*tm_iter++ = imin_iter_kx[xy7];
							imin_iter_kx++;
						}
						imin_iter_ky += dim_in;
					}
					imin_iter_ic += size_in;
				}
			}

			for (i = remain; i < size_out; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + (i / 8 + i % 8) * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= dim_out) { x = 0; y++; }
				xy0 = (x0 + y0 * dim_in) * stride;

				const signed char* imin_iter_ic = data_in;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < ch_in; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							imin_iter_kx++;
						}
						imin_iter_ky += dim_in;
					}
					imin_iter_ic += size_in;
				}
			}
		}

		// sgemm(int M, int N, int L, float* A, float* B, float* C)
		{
			//int M = outch;  // outch
			int N = outw * outh; // outsize or out stride
			int L = dim_kernel * dim_kernel * inch; // ksize * inch

			int nn_outch = 0;
			int remain_outch_start = 0;

			nn_outch = (outch - remain_outch_start) >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int pp = 0; pp < nn_outch; pp++)
			{
				int i = remain_outch_start + pp * 4;

				signed char* output0 = data_out + (i + 0) * size_out; // top_blob.channel(i);
				signed char* output1 = data_out + (i + 1) * size_out; // top_blob.channel(i + 1);
				signed char* output2 = data_out + (i + 2) * size_out; // top_blob.channel(i + 2);
				signed char* output3 = data_out + (i + 3) * size_out; // top_blob.channel(i + 3);

				int j = 0;
				for (; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// K loop
						"vmov.s32    q8, #0             \n"
						"vmov.s32    q9, #0             \n"
						"vmov.s32    q10, #0            \n"
						"vmov.s32    q11, #0            \n"
						"vmov.s32    q12, #0            \n"
						"vmov.s32    q13, #0            \n"
						"vmov.s32    q14, #0            \n"
						"vmov.s32    q15, #0            \n"

						"lsr         r4, %6, #3         \n"// r4 = nn = L >> 3
						"cmp         r4, #0             \n"
						"beq         1f                 \n"

						"0:                             \n"// for(; nn != 0; nn--)
						"pld         [%4, #128]         \n"
						"vld1.s8     {d8-d11}, [%4]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q7, d11            \n"// a30-a37
						"vmovl.s8    q6, d10            \n"// a20-a27                    
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"pld         [%5, #128]         \n"
						"vld1.s8     {d0-d3}, [%5]!     \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q3, d3             \n"// k06-k36,k07-k37
						"vmovl.s8    q2, d2             \n"// k04-k34,k05-k35
						"vmovl.s8    q1, d1             \n"// k02-k32,k03-k33
						"vmovl.s8    q0, d0             \n"// k00-k30,k01-k31

						"vmlal.s16   q8, d8, d0[0]      \n"// sum0 = (a00-a07) * k00
						"vmlal.s16   q9, d9, d0[0]      \n"
						"vmlal.s16   q10, d8, d0[1]     \n"// sum1 = (a00-a07) * k10
						"vmlal.s16   q11, d9, d0[1]     \n"
						"vmlal.s16   q12, d8, d0[2]     \n"// sum2 = (a00-a07) * k20
						"vmlal.s16   q13, d9, d0[2]     \n"
						"vmlal.s16   q14, d8, d0[3]     \n"// sum3 = (a00-a07) * k30
						"vmlal.s16   q15, d9, d0[3]     \n"

						"vmlal.s16   q8, d10, d1[0]     \n"// sum0 += (a10-a17) * k01
						"vmlal.s16   q9, d11, d1[0]     \n"
						"vmlal.s16   q10, d10, d1[1]    \n"// sum1 += (a10-a17) * k11
						"vmlal.s16   q11, d11, d1[1]    \n"
						"vmlal.s16   q12, d10, d1[2]    \n"// sum2 += (a10-a17) * k21
						"vmlal.s16   q13, d11, d1[2]    \n"
						"vmlal.s16   q14, d10, d1[3]    \n"// sum3 += (a10-a17) * k31
						"vmlal.s16   q15, d11, d1[3]    \n"

						"pld         [%4, #128]         \n"
						"vld1.s8     {d8-d9}, [%4]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"vmlal.s16   q8, d12, d2[0]     \n"// sum0 += (a20-a27) * k02
						"vmlal.s16   q9, d13, d2[0]     \n"
						"vmlal.s16   q10, d12, d2[1]    \n"// sum1 += (a20-a27) * k12
						"vmlal.s16   q11, d13, d2[1]    \n"
						"vmlal.s16   q12, d12, d2[2]    \n"// sum2 += (a20-a27) * k22
						"vmlal.s16   q13, d13, d2[2]    \n"
						"vmlal.s16   q14, d12, d2[3]    \n"// sum3 += (a20-a27) * k32
						"vmlal.s16   q15, d13, d2[3]    \n"

						"vmlal.s16   q8, d14, d3[0]     \n"// sum0 += (a30-a37) * k03
						"vmlal.s16   q9, d15, d3[0]     \n"
						"vmlal.s16   q10, d14, d3[1]    \n"// sum1 += (a30-a37) * k13
						"vmlal.s16   q11, d15, d3[1]    \n"
						"vmlal.s16   q12, d14, d3[2]    \n"// sum2 += (a30-a37) * k23
						"vmlal.s16   q13, d15, d3[2]    \n"
						"vmlal.s16   q14, d14, d3[3]    \n"// sum3 += (a30-a37) * k33
						"vmlal.s16   q15, d15, d3[3]    \n"

						"pld         [%4, #128]         \n"
						"vld1.s8     {d0-d1}, [%4]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q1, d1             \n"// a10-a17
						"vmovl.s8    q0, d0             \n"// a00-a07

						"vmlal.s16   q8, d8, d4[0]      \n"// sum0 += (a40-a47) * k04
						"vmlal.s16   q9, d9, d4[0]      \n"
						"vmlal.s16   q10, d8, d4[1]     \n"// sum1 += (a40-a47) * k14
						"vmlal.s16   q11, d9, d4[1]     \n"
						"vmlal.s16   q12, d8, d4[2]     \n"// sum2 += (a40-a47) * k24
						"vmlal.s16   q13, d9, d4[2]     \n"
						"vmlal.s16   q14, d8, d4[3]     \n"// sum3 += (a40-a47) * k34
						"vmlal.s16   q15, d9, d4[3]     \n"

						"vmlal.s16   q8, d10, d5[0]     \n"// sum0 += (a50-a57) * k05
						"vmlal.s16   q9, d11, d5[0]     \n"
						"vmlal.s16   q10, d10, d5[1]    \n"// sum1 += (a50-a57) * k15
						"vmlal.s16   q11, d11, d5[1]    \n"
						"vmlal.s16   q12, d10, d5[2]    \n"// sum2 += (a50-a57) * k25
						"vmlal.s16   q13, d11, d5[2]    \n"
						"vmlal.s16   q14, d10, d5[3]    \n"// sum3 += (a50-a57) * k35
						"vmlal.s16   q15, d11, d5[3]    \n"

						"vmlal.s16   q8, d0, d6[0]      \n"// sum0 += (a60-a67) * k06
						"vmlal.s16   q9, d1, d6[0]      \n"
						"vmlal.s16   q10, d0, d6[1]     \n"// sum1 += (a60-a67) * k16
						"vmlal.s16   q11, d1, d6[1]     \n"
						"vmlal.s16   q12, d0, d6[2]     \n"// sum2 += (a60-a67) * k26
						"vmlal.s16   q13, d1, d6[2]     \n"
						"vmlal.s16   q14, d0, d6[3]     \n"// sum3 += (a60-a67) * k36
						"vmlal.s16   q15, d1, d6[3]     \n"

						"vmlal.s16   q8, d2, d7[0]      \n"// sum0 += (a70-a77) * k07
						"vmlal.s16   q9, d3, d7[0]      \n"
						"vmlal.s16   q10, d2, d7[1]     \n"// sum1 += (a70-a77) * k17
						"vmlal.s16   q11, d3, d7[1]     \n"
						"vmlal.s16   q12, d2, d7[2]     \n"// sum2 += (a70-a77) * k27
						"vmlal.s16   q13, d3, d7[2]     \n"
						"vmlal.s16   q14, d2, d7[3]     \n"// sum3 += (a70-a77) * k37
						"vmlal.s16   q15, d3, d7[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         0b                 \n"// end for

						"1:                             \n"
						// remain loop
						"and         r4, %6, #7         \n"// r4 = remain = inch & 7
						"cmp         r4, #0             \n"
						"beq         3f                 \n"

						"2:                             \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%4]!        \n"// tmpr a00-a70    a(inch)(data)
						"vld1.s8     {d0}, [%5]         \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2             \n"
						"vmovl.s8    q0, d0             \n"
						"add         %5, #4             \n"

						"vmlal.s16   q8, d2, d0[0]      \n"// sum0 += (a00-a70) * k00
						"vmlal.s16   q9, d3, d0[0]      \n"
						"vmlal.s16   q10, d2, d0[1]     \n"// sum1 += (a00-a70) * k10
						"vmlal.s16   q11, d3, d0[1]     \n"
						"vmlal.s16   q12, d2, d0[2]     \n"// sum2 += (a00-a70) * k20
						"vmlal.s16   q13, d3, d0[2]     \n"
						"vmlal.s16   q14, d2, d0[3]     \n"// sum3 += (a00-a70) * k30
						"vmlal.s16   q15, d3, d0[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         2b                 \n"

						"3:                             \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%7]      \n"

						"vcvt.f32.s32 q8, q8            \n"
						"vcvt.f32.s32 q9, q9            \n"
						"vcvt.f32.s32 q10, q10          \n"
						"vcvt.f32.s32 q11, q11          \n"

						"vdup.32     q1, %8             \n"
						"vdup.32     q3, %9             \n"
						"vmov.32     q2, q1             \n"
						"vmov.32     q4, q3             \n"

						"vmla.f32    q1, q8, d0[0]      \n"
						"vmla.f32    q2, q9, d0[0]      \n"
						"vmla.f32    q3, q10, d0[1]     \n"
						"vmla.f32    q4, q11, d0[1]     \n"

						"vcvt.f32.s32 q12, q12          \n"
						"vcvt.f32.s32 q13, q13          \n"
						"vcvt.f32.s32 q14, q14          \n"
						"vcvt.f32.s32 q15, q15          \n"

						"vdup.32     q5, %10            \n"
						"vdup.32     q7, %11            \n"
						"vmov.32     q6, q5             \n"
						"vmov.32     q8, q7             \n"

						"vmla.f32    q5, q12, d1[0]    \n"
						"vmla.f32    q6, q13, d1[0]    \n"
						"vmla.f32    q7, q14, d1[1]    \n"
						"vmla.f32    q0, q15, d1[1]    \n"

						"vcvtr.s32.f32 s4,s4            \n"
						"vcvtr.s32.f32 s5,s5            \n"
						"vcvtr.s32.f32 s6,s6            \n"
						"vcvtr.s32.f32 s7,s7            \n"

						"vcvtr.s32.f32 s8,s8            \n"
						"vcvtr.s32.f32 s9,s9            \n"
						"vcvtr.s32.f32 s10,s10          \n"
						"vcvtr.s32.f32 s11,s11          \n"

						"vcvtr.s32.f32 s12,s12          \n"
						"vcvtr.s32.f32 s13,s13          \n"
						"vcvtr.s32.f32 s14,s14          \n"
						"vcvtr.s32.f32 s15,s15          \n"

						"vcvtr.s32.f32 s16,s16          \n"
						"vcvtr.s32.f32 s17,s17          \n"
						"vcvtr.s32.f32 s18,s18          \n"
						"vcvtr.s32.f32 s19,s19          \n"

						"vcvtr.s32.f32 s20,s20          \n"
						"vcvtr.s32.f32 s21,s21          \n"
						"vcvtr.s32.f32 s22,s22          \n"
						"vcvtr.s32.f32 s23,s23          \n"

						"vcvtr.s32.f32 s24,s24          \n"
						"vcvtr.s32.f32 s25,s25          \n"
						"vcvtr.s32.f32 s26,s26          \n"
						"vcvtr.s32.f32 s27,s27          \n"

						"vcvtr.s32.f32 s0,s0            \n"
						"vcvtr.s32.f32 s1,s1            \n"
						"vcvtr.s32.f32 s2,s2            \n"
						"vcvtr.s32.f32 s3,s3            \n"

						"vcvtr.s32.f32 s28,s28          \n"
						"vcvtr.s32.f32 s29,s29          \n"
						"vcvtr.s32.f32 s30,s30          \n"
						"vcvtr.s32.f32 s31,s31          \n"

						"vmov.32   q8, q0               \n"

						"vqmovn.s32 d0, q1              \n"
						"vqmovn.s32 d1, q2              \n"
						"vqmovn.s32 d2, q3              \n"
						"vqmovn.s32 d3, q4              \n"
						"vqmovn.s32 d4, q5              \n"
						"vqmovn.s32 d5, q6              \n"
						"vqmovn.s32 d6, q7              \n"
						"vqmovn.s32 d7, q8              \n"

						"vqmovn.s16 d0, q0              \n"
						"vqmovn.s16 d1, q1              \n"
						"vqmovn.s16 d2, q2              \n"
						"vqmovn.s16 d3, q3              \n"

						"vst1.s32    {d0}, [%0]         \n"
						"vst1.s32    {d1}, [%1]         \n"
						"vst1.s32    {d2}, [%2]         \n"
						"vst1.s32    {d3}, [%3]         \n"

						: "+r"(output0), // %0
						"+r"(output1), // %1
						"+r"(output2), // %2
						"+r"(output3), // %3
						"+r"(vb),      // %4
						"+r"(va)       // %5
						: "r"(L),        // %6  
						"r"(mem_scale + i),   // %7
						"r"(mem_bias[i]),     // %8
						"r"(mem_bias[i + 1]),   // %9
						"r"(mem_bias[i + 2]),   // %10
						"r"(mem_bias[i + 3])    // %11
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
#else
					int sum0[8] = { 0 };
					int sum1[8] = { 0 };
					int sum2[8] = { 0 };
					int sum3[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 8];
							sum1[n] += (int)va[1] * vb[n + 8];
							sum2[n] += (int)va[2] * vb[n + 8];
							sum3[n] += (int)va[3] * vb[n + 8];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 16];
							sum1[n] += (int)va[1] * vb[n + 16];
							sum2[n] += (int)va[2] * vb[n + 16];
							sum3[n] += (int)va[3] * vb[n + 16];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 24];
							sum1[n] += (int)va[1] * vb[n + 24];
							sum2[n] += (int)va[2] * vb[n + 24];
							sum3[n] += (int)va[3] * vb[n + 24];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 32];
							sum1[n] += (int)va[1] * vb[n + 32];
							sum2[n] += (int)va[2] * vb[n + 32];
							sum3[n] += (int)va[3] * vb[n + 32];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 40];
							sum1[n] += (int)va[1] * vb[n + 40];
							sum2[n] += (int)va[2] * vb[n + 40];
							sum3[n] += (int)va[3] * vb[n + 40];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 48];
							sum1[n] += (int)va[1] * vb[n + 48];
							sum2[n] += (int)va[2] * vb[n + 48];
							sum3[n] += (int)va[3] * vb[n + 48];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 56];
							sum1[n] += (int)va[1] * vb[n + 56];
							sum2[n] += (int)va[2] * vb[n + 56];
							sum3[n] += (int)va[3] * vb[n + 56];
							va -= 28;
						}

						va += 32;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
						}

						va += 4;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						output0[n] = float2int8(sum0[n] * mem_scale[i + 0] + mem_bias[i + 0]);
						output1[n] = float2int8(sum1[n] * mem_scale[i + 1] + mem_bias[i + 1]);
						output2[n] = float2int8(sum2[n] * mem_scale[i + 2] + mem_bias[i + 2]);
						output3[n] = float2int8(sum3[n] * mem_scale[i + 3] + mem_bias[i + 3]);
					}
#endif // __ARM_NEON
					output0 += 8;
					output1 += 8;
					output2 += 8;
					output3 += 8;
				}

				for (; j < N; j++)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8 + j % 8) * tm_cstep; // bottom_tm.channel(j / 8 + j % 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// inch loop
						"veor        q6, q6, q6        \n"
						"veor        q7, q7, q7        \n"
						"veor        q8, q8, q8        \n"
						"veor        q9, q9, q9        \n"
						"veor        q10, q10, q10     \n"
						"veor        q11, q11, q11     \n"
						"veor        q12, q12, q12     \n"
						"veor        q13, q13, q13     \n"
						"vmov.s32    q14, #0           \n"

						"lsr         r4, %6, #3        \n"// r4 = nn = L >> 2
						"cmp         r4, #0            \n"
						"beq         1f                \n"

						"0:                            \n"// for(; nn != 0; nn--)
						"pld         [%4, #128]        \n"
						"vld1.s8     {d0}, [%4]!       \n"// tmpr a00,a10,a20,a30    a(inch)(data)
						"vmovl.s8    q0, d0            \n"// a00-a07

						"pld         [%5, #128]        \n"
						"vld1.s8     {d2-d5}, [%5]!    \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q4, d5            \n"// k06-k36,k07-k37
						"vmovl.s8    q3, d4            \n"// k04-k34,k05-k35
						"vmovl.s8    q2, d3            \n"// k02-k32,k03-k33
						"vmovl.s8    q1, d2            \n"// k00-k30,k01-k31

						"vmlal.s16   q6, d2, d0[0]     \n"// (k00-k30) * a00
						"vmlal.s16   q7, d3, d0[1]     \n"// (k01-k31) * a01
						"vmlal.s16   q8, d4, d0[2]     \n"// (k02-k32) * a02
						"vmlal.s16   q9, d5, d0[3]     \n"// (k03-k33) * a03
						"vmlal.s16   q10, d6, d1[0]    \n"// (k04-k34) * a04
						"vmlal.s16   q11, d7, d1[1]    \n"// (k05-k35) * a05
						"vmlal.s16   q12, d8, d1[2]    \n"// (k06-k36) * a06
						"vmlal.s16   q13, d9, d1[3]    \n"// (k07-k37) * a07                    

						"subs        r4, r4, #1        \n"
						"bne         0b                \n"// end for

						"vadd.s32    q6, q6, q7        \n"
						"vadd.s32    q9, q9, q8        \n"
						"vadd.s32    q11, q11, q10     \n"
						"vadd.s32    q13, q13, q12     \n"

						"vadd.s32    q9, q9, q6        \n"
						"vadd.s32    q13, q13, q11     \n"
						"vadd.s32    q14, q13, q9      \n"

						"1:                            \n"
						// remain loop
						"and         r4, %6, #7        \n"// r4 = remain = inch & 3
						"cmp         r4, #0            \n"
						"beq         3f                \n"

						"2:                            \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%4]        \n"// tmpr a00        a(inch)(data)
						"vld1.s8     {d0}, [%5]        \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2            \n"
						"vmovl.s8    q0, d0            \n"
						"add         %4, #1            \n"
						"add         %5, #4            \n"

						"vmlal.s16   q14, d0, d2[0]    \n"

						"subs        r4, r4, #1        \n"
						"bne         2b                \n"

						"3:                            \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%7]     \n"
						"vld1.f32    {d2-d3}, [%8]     \n"

						"vcvt.f32.s32 q14, q14         \n"

						"vmla.f32    q1, q14, q0       \n"

						"vcvtr.s32.f32 s4, s4          \n"
						"vcvtr.s32.f32 s5, s5          \n"
						"vcvtr.s32.f32 s6, s6          \n"
						"vcvtr.s32.f32 s7, s7          \n"

						"vqmovn.s32 d0, q1             \n"
						"vqmovn.s16 d0, q0             \n"

						"vst1.s8     {d0[0]}, [%0]     \n"
						"vst1.s8     {d0[1]}, [%1]     \n"
						"vst1.s8     {d0[2]}, [%2]     \n"
						"vst1.s8     {d0[3]}, [%3]     \n"

						: "+r"(output0), // %0
						"+r"(output1), // %1
						"+r"(output2), // %2
						"+r"(output3), // %3
						"+r"(vb),      // %4
						"+r"(va)       // %5
						: "r"(L),        // %6  
						"r"(mem_scale + i),  // %7
						"r"(mem_bias + i)    // %8
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14"
						);
#else
					int sum0 = 0;
					int sum1 = 0;
					int sum2 = 0;
					int sum3 = 0;

					for (int k = 0; k < L; k++)
					{
						sum0 += (int)va[0] * vb[0];
						sum1 += (int)va[1] * vb[0];
						sum2 += (int)va[2] * vb[0];
						sum3 += (int)va[3] * vb[0];

						va += 4;
						vb += 1;
					}

					output0[0] = float2int8(sum0 * mem_scale[i + 0] + mem_bias[i + 0]);
					output1[0] = float2int8(sum1 * mem_scale[i + 1] + mem_bias[i + 1]);
					output2[0] = float2int8(sum2 * mem_scale[i + 2] + mem_bias[i + 2]);
					output3[0] = float2int8(sum3 * mem_scale[i + 3] + mem_bias[i + 3]);
#endif // __ARM_NEON
					output0++;
					output1++;
					output2++;
					output3++;
				}
			}

			remain_outch_start += nn_outch << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int i = remain_outch_start; i < outch; i++)
			{
				signed char* output = data_out + i * size_out; // top_blob.channel(i);

				int j = 0;
				for (; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4 + i % 4) * kernel_cstep; // kernel_tm.channel(i / 4 + i % 4);

#if __ARM_NEON
					asm volatile(
						// inch loop
						"vmov.s32    q6, #0            \n"
						"vmov.s32    q7, #0            \n"

						"lsr         r4, %3, #3        \n"// r4 = nn = inch >> 3
						"cmp         r4, #0            \n"
						"beq         1f                \n"

						"0:                            \n"// for(; nn != 0; nn--)
						"pld         [%1, #128]        \n"
						"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d7            \n"// a30-a37
						"vmovl.s8    q4, d6            \n"// a20-a27
						"vmovl.s8    q3, d5            \n"// a10-a17
						"vmovl.s8    q2, d4            \n"// a00-a07

						"pld         [%2, #128]        \n"
						"vld1.s8     {d0}, [%2]!       \n"// kptr k00-k07    k(outch)(inch)
						"vmovl.s8    q1, d1            \n"// k04,k05,k06,k07
						"vmovl.s8    q0, d0            \n"// k00,k01,k02,k03

						"vmlal.s16   q6, d4, d0[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d5, d0[0]     \n"
						"vmlal.s16   q6, d6, d0[1]     \n"// (a10-a17) * k01
						"vmlal.s16   q7, d7, d0[1]     \n"
						"vmlal.s16   q6, d8, d0[2]     \n"// (a20-a27) * k02
						"vmlal.s16   q7, d9, d0[2]     \n"
						"vmlal.s16   q6, d10, d0[3]    \n"// (a30-a37) * k03
						"vmlal.s16   q7, d11, d0[3]    \n"

						"pld         [%1, #128]        \n"
						"vld1.s8     {d4-d7}, [%1]!    \n"// tmpr a40-a47,a50-a57,a60-a67,a70-a77    a(inch)(data)
						"vmovl.s8    q5, d7            \n"// a70-a77
						"vmovl.s8    q4, d6            \n"// a60-a67
						"vmovl.s8    q3, d5            \n"// a50-a57
						"vmovl.s8    q2, d4            \n"// a40-a47

						"vmlal.s16   q6, d4, d1[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d5, d1[0]     \n"
						"vmlal.s16   q6, d6, d1[1]     \n"// (a10-a17) * k01
						"vmlal.s16   q7, d7, d1[1]     \n"
						"vmlal.s16   q6, d8, d1[2]     \n"// (a20-a27) * k02
						"vmlal.s16   q7, d9, d1[2]     \n"
						"vmlal.s16   q6, d10, d1[3]    \n"// (a30-a37) * k03
						"vmlal.s16   q7, d11, d1[3]    \n"

						"subs        r4, r4, #1        \n"
						"bne         0b                \n"// end for

						"1:                            \n"
						// remain loop
						"and         r4, %3, #7        \n"// r4 = remain = inch & 7
						"cmp         r4, #0            \n"
						"beq         3f                \n"

						"2:                            \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%1]!       \n"// tmpr a00-a07    a(inch)(data)
						"vld1.s8     {d0}, [%2]        \n"// kptr k00        k(outch)(inch)
						"vmovl.s8    q1, d2            \n"
						"vmovl.s8    q0, d0            \n"
						"add         %2, #1            \n"

						"vmlal.s16   q6, d2, d0[0]     \n"// (a00-a07) * k00
						"vmlal.s16   q7, d3, d0[0]     \n"

						"subs        r4, r4, #1        \n"
						"bne         2b                \n"

						"3:                            \n"// store the result to memory

						"vdup.32   q0, %4              \n"
						"vdup.32   q1, %5              \n"
						"vmov.32   q2, q1              \n"

						"vcvt.f32.s32  q6, q6          \n"
						"vcvt.f32.s32  q7, q7          \n"

						"vmla.f32   q1, q6, q0         \n"
						"vmla.f32   q2, q7, q0         \n"

						"vcvtr.s32.f32 s4, s4          \n"
						"vcvtr.s32.f32 s5, s5          \n"
						"vcvtr.s32.f32 s6, s6          \n"
						"vcvtr.s32.f32 s7, s7          \n"

						"vcvtr.s32.f32 s8, s8          \n"
						"vcvtr.s32.f32 s9, s9          \n"
						"vcvtr.s32.f32 s10, s10        \n"
						"vcvtr.s32.f32 s11, s11        \n"

						"vqmovn.s32  d0, q1            \n"
						"vqmovn.s32  d1, q2            \n"

						"vqmovn.s16  d0, q0            \n"

						"vst1.s32    {d0}, [%0]        \n"

						: "+r"(output), // %0
						"+r"(vb),     // %1
						"+r"(va)      // %2
						: "r"(L),       // %3  
						"r"(mem_scale[i]), // %4
						"r"(mem_bias[i])   // %5
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7"
						);
#else                
					int sum[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum[n] += (int)va[0] * vb[n];
							sum[n] += (int)va[1] * vb[n + 8];
							sum[n] += (int)va[2] * vb[n + 16];
							sum[n] += (int)va[3] * vb[n + 24];
							sum[n] += (int)va[4] * vb[n + 32];
							sum[n] += (int)va[5] * vb[n + 40];
							sum[n] += (int)va[6] * vb[n + 48];
							sum[n] += (int)va[7] * vb[n + 56];
						}

						va += 8;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum[n] += (int)va[0] * vb[n];
						}

						va += 1;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						output[n] = float2int8(sum[n] * mem_scale[i] + mem_bias[i]);
					}
#endif // __ARM_NEON
					output += 8;
				}

				for (; j < N; j++)
				{
					int sum = 0;

					signed char* vb = ((signed char*)mem_tm) + (j / 8 + j % 8) * tm_cstep; // bottom_tm.channel(j / 8 + j % 8);
					const signed char* va = weight + (i / 4 + i % 4) * kernel_cstep; // kernel_tm.channel(i / 4 + i % 4);

					for (int k = 0; k < L; k++)
					{
						sum += (int)va[0] * vb[0];

						va += 1;
						vb += 1;
					}

					output[0] = float2int8(sum * mem_scale[i] + mem_bias[i]);
					output++;
				}
			}
		}
	}

	void conv_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		if (dim_kernel == 3 && stride == 2)
		{
			conv_kxsx_nf(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else if (dim_kernel == 1 && stride == 1)
		{
			conv_k1s1_nf(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else
		{
			; // printf("%d %s\r\n", __LINE__, __FUNCTION__);
		}
	}

	void conv_nf_r(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		if (dim_kernel == 3 && stride == 2)
		{
			conv_k3s2_nf_r(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else
		{
			; // printf("%d %s\r\n", __LINE__, __FUNCTION__);
		}
	}

	void conv_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		if (dim_kernel == 3 && stride == 1)
		{
			conv_kxsx_nn_r(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else if (dim_kernel == 3 && stride == 2)
		{
			conv_kxsx_nn_r(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else if (dim_kernel == 1 && stride == 1)
		{
			conv_k1s1_nn_r(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else
		{
			; // printf("%d %s\r\n", __LINE__, __FUNCTION__);
		}
	}

	void conv_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		if (dim_kernel == 1 && stride == 1)
		{
			conv_k1s1_nn(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else
		{
			; // printf("%d %s\r\n", __LINE__, __FUNCTION__);
		}
	}

	void convd_nf(signed char* data_in, int ch_in, int dim_in, float* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		if (dim_kernel == 3 && stride == 1)
		{
			convd_k3s1_nf(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else if (dim_kernel == 3 && stride == 2)
		{
			convd_k3s2_nf(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else if ((dim_kernel == 5 && stride == 1) || (dim_kernel == 5 && stride == 2) || (dim_kernel == 8 && stride == 1))
		{
			for (int q = 0; q < ch_in; q++)
			{
				conv_kxsx_nf(data_in + dim_in * dim_in * q, 1, dim_in, data_out + dim_out * dim_out * q, 1, dim_out, dim_kernel, stride, weight + dim_kernel * dim_kernel * q, mem_scale + q, mem_bias + q, mem_tm);
			}
		}
		else
		{
			; // printf("%d %s\r\n", __LINE__, __FUNCTION__);
		}
	}

	void convd_nn_r(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		if (dim_kernel == 3 && stride == 1)
		{
			convd_k3s1_nn_r(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else if (dim_kernel == 3 && stride == 2)
		{
			convd_k3s2_nn_r(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else
		{
			; // printf("%d %s\r\n", __LINE__, __FUNCTION__);
		}
	}

	void convd_nn(signed char* data_in, int ch_in, int dim_in, signed char* data_out, int ch_out, int dim_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		if (dim_kernel == 3 && stride == 1)
		{
			convd_k3s1_nn(data_in, ch_in, dim_in, data_out, ch_out, dim_out, dim_kernel, stride, weight, mem_scale, mem_bias, mem_tm);
		}
		else if ((dim_kernel == 8 && stride == 1) || (dim_kernel == 4 && stride == 1))
		{
			for (int q = 0; q < ch_in; q++)
			{
				conv_kxsx_nn(data_in + dim_in * dim_in * q, 1, dim_in, data_out + dim_out * dim_out * q, 1, dim_out, dim_kernel, stride, weight + dim_kernel * dim_kernel * q, mem_scale + q, mem_bias + q, mem_tm);
			}
		}
		else
		{
			; // printf("%d %s\r\n", __LINE__, __FUNCTION__);
		}
	}

	//  (size / 2) 
	void conv_k1s1_nn_r_pack8(ennq_blob* in_blob, ennq_blob* out_blob, int outch, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		// assert(inch % 8 == 0 and outch % 8 == 0 and size % 2 = 0)
		int w = in_blob->w;
		int h = in_blob->h;
		int c = in_blob->c;
		int cstep = in_blob->cstep;

		int outw = w;
		int outh = h;

		int size = outw * outh;
		int m_cstep = c * 2;
		int nn_c = c / 8;
		int nn_size = size >> 1;

		for (int ii = 0; ii < nn_size; ii++)
		{
			signed char* tmpptr = ((signed char*)mem_tm) + m_cstep * ii;
			signed char* inptr = ((signed char*)in_blob->mem) + (ii << 4);
			for (int qq = 0; qq < nn_c; qq++)
			{
#if __ARM_NEON
				asm volatile(
					"pld        [%1, #256]			\n"
					"vld1.s8   {d0-d1}, [%1]		\n"
					"vst1.s8   {d0-d1}, [%0]!		\n"
					: "+r"(tmpptr)		// %0
					: "r"(inptr)		// %1
					: "memory", "q0"
					);
				inptr += cstep;
#else
				memcpy(tmpptr, inptr, 16);
				tmpptr += 16;
				inptr += cstep;
#endif
			}
		}

		int out_cstep = get_blob_size(8 * size);
		int k_cstep = 4 * c;
		int nn_oc = outch / 8;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->c = outch;
		out_blob->cstep = out_cstep;
		out_blob->packp = 8;
		out_blob->elemc = 1;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pq = 0; pq < nn_oc; pq++)
		{
			signed char* out_mem_ch = ((signed char*)out_blob->mem) + pq * out_cstep;

			for (int it = 0; it < 2; it++)
			{
				const signed char* kmem_ch = weight + (pq * 2 + it) * k_cstep;

				signed char* outptr0 = out_mem_ch + it * 4; // out_mem + pq * out_cstep;

				const signed char* in_mem_ch = (signed char*)mem_tm;

				const float* sc0 = mem_scale + pq * 8 + it * 4;
				const float* bs0 = mem_bias + pq * 8 + it * 4;

				int i = 0;
				for (; i + 1 < size; i += 2)
				{
					const signed char* tmpptr = in_mem_ch;	// in_mem + (i / 2) * cstep;						// tmp.channel(i / 2);
					const signed char* kptr0 = kmem_ch;		// weight + (pq * 2 + it) * k_cstep; 				// kernel.channel(p);
#if __ARM_NEON
					asm volatile(
						"veor       q0, q0              \n"
						"veor       q1, q1              \n"
						"veor       q2, q2              \n"
						"veor       q3, q3              \n"
						"veor       q4, q4              \n"
						"veor       q5, q5              \n"
						"veor       q6, q6              \n"
						"veor       q7, q7              \n"

						"mov        r6, #32             \n"
						"add        r5, %2, #16         \n"

						"lsr        r4, %3, #1          \n" // r4 = nn = size >> 1
						"cmp        r4, #0              \n"
						"beq        1f                  \n"

						"0:                             \n"

						"pld        [%2, #256]          \n"
						"pld        [%1, #128]          \n"

						"vld1.s8    {d20-d21}, [%2 :128], r6 \n" // _w01
						"vld1.s8    {d16-d19}, [%1 :128]! \n" // _val0 _val1
						"vld1.s8    {d22-d23}, [%2 :128], r6 \n" // _w45

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vmlal.s8   q12, d18, d22       \n"
						"vmlal.s8   q13, d18, d23       \n"
						"vmlal.s8   q14, d19, d22       \n"
						"vmlal.s8   q15, d19, d23       \n"

						"vpadal.s16 q0, q12             \n"
						"vpadal.s16 q1, q13             \n"
						"vpadal.s16 q4, q14             \n"
						"vpadal.s16 q5, q15             \n"

						"pld		[r5, #256]			\n"
						"vld1.s8    {d20-d21}, [r5 :128], r6 \n" // _w23
						"vld1.s8    {d22-d23}, [r5 :128], r6 \n" // _w67

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vmlal.s8   q12, d18, d22       \n"
						"vmlal.s8   q13, d18, d23       \n"
						"vmlal.s8   q14, d19, d22       \n"
						"vmlal.s8   q15, d19, d23       \n"

						"vpadal.s16 q2, q12             \n"
						"vpadal.s16 q3, q13             \n"
						"vpadal.s16 q6, q14             \n"
						"vpadal.s16 q7, q15             \n"

						"subs       r4, r4, #1          \n"
						"bne        0b                  \n"

						"1:                             \n"
						"ands        r4, %3, #1          \n" // r4 = remain = size & 1
						"beq        2f                  \n"

						"pld		[%1, #64]			\n"
						"vld1.s8    {d16-d17}, [%1 :128]! \n" // _val
						"pld		[%2, #128]			\n"
						"vld1.s8    {d20-d21}, [%2 :128]! \n" // _w01
						"vld1.s8    {d22-d23}, [%2 :128]! \n" // _w23

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vpadal.s16 q0, q12             \n"
						"vpadal.s16 q1, q13             \n"
						"vpadal.s16 q4, q14             \n"
						"vpadal.s16 q5, q15             \n"

						"vmull.s8   q12, d16, d22       \n"
						"vmull.s8   q13, d16, d23       \n"
						"vmull.s8   q14, d17, d22       \n"
						"vmull.s8   q15, d17, d23       \n"

						"vpadal.s16 q2, q12             \n"
						"vpadal.s16 q3, q13             \n"
						"vpadal.s16 q6, q14             \n"
						"vpadal.s16 q7, q15             \n"

						"2:                             \n"

						"vpadd.s32  d16, d0, d1         \n"
						"vpadd.s32  d17, d2, d3         \n"
						"vpadd.s32  d18, d4, d5         \n"
						"vpadd.s32  d19, d6, d7         \n"

						"vld1.f32	{d4-d5}, [%4 :128]	\n"

						"vpadd.s32  d20, d8, d9         \n"
						"vpadd.s32  d21, d10, d11       \n"
						"vpadd.s32  d22, d12, d13       \n"
						"vpadd.s32  d23, d14, d15       \n"

						"vld1.f32	{d0-d1}, [%5 :128]	\n"

						"vpadd.s32  d6, d16, d17        \n"
						"vpadd.s32  d7, d18, d19        \n"

						"vmov		q1, q0				\n"
						"mov		r4, #8				\n"
						"veor       d10, d10			\n"

						"vpadd.s32  d8, d20, d21        \n"
						"vpadd.s32  d9, d22, d23        \n"

						"vcvt.f32.s32	q3, q3			\n"
						"vcvt.f32.s32	q4, q4			\n"

						"vmla.f32	q0, q3, q2			\n"
						"vmla.f32	q1, q4, q2			\n"

						"vcvt.s32.f32 q0, q0            \n"
						"vcvt.s32.f32 q1, q1            \n"

						"vqmovn.s32 d0, q0              \n"
						"vqmovn.s32 d1, q1              \n"

						"vqmovn.s16 d0, q0              \n"
						"vmax.s8    d0, d0, d10			\n"

						"vst1.s32   {d0[0]}, [%0], r4	\n"
						"vst1.s32   {d0[1]}, [%0], r4	\n"

						:
					"+r"(outptr0),		// %0
						"+r"(tmpptr),		// %1
						"+r"(kptr0)			// %2
						:
						"r"(nn_c),			// %3
						"r"(sc0),			// %4
						"r"(bs0)			// %5
						: "memory", "r4", "r5", "r6", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15");
#else
					int sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0, sum6 = 0, sum7 = 0;

					for (int j = 0; j < nn_c; j++)
					{
						sum0 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum1 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum2 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum3 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 -= 24;
						tmpptr += 8;
						sum4 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum5 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum6 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum7 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						tmpptr += 8;
					}

					int val;

					val = (int)(sum0 * sc0[0] + bs0[0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[0] = val;
					val = (int)(sum1 * sc0[1] + bs0[1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[1] = val;
					val = (int)(sum2 * sc0[2] + bs0[2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[2] = val;
					val = (int)(sum3 * sc0[3] + bs0[3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[3] = val;
					val = (int)(sum4 * sc0[0] + bs0[0]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[8] = val;
					val = (int)(sum5 * sc0[1] + bs0[1]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[9] = val;
					val = (int)(sum6 * sc0[2] + bs0[2]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[10] = val;
					val = (int)(sum7 * sc0[3] + bs0[3]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[11] = val;

					outptr0 += 16;
#endif
					in_mem_ch += m_cstep;
				}
			}
		}
	}

	void conv_k1s1_nn_p_pack8(ennq_blob* in_blob, ennq_blob* out_blob, int outch, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu, void* mem_tm)
	{
		// assert(inch % 8 == 0 and outch % 8 == 0 and size % 2 = 0)
		int w = in_blob->w;
		int h = in_blob->h;
		int c = in_blob->c;
		int cstep = in_blob->cstep;

		int outw = w;
		int outh = h;

		int size = outw * outh;
		int m_cstep = c * 2;
		int nn_c = c / 8;
		int nn_size = size >> 1;

		for (int ii = 0; ii < nn_size; ii++)
		{
			signed char* tmpptr = ((signed char*)mem_tm) + m_cstep * ii;
			signed char* inptr = ((signed char*)in_blob->mem) + (ii << 4);
			for (int qq = 0; qq < nn_c; qq++)
			{
#if __ARM_NEON
				asm volatile(
					"pld        [%1, #256]			\n"
					"vld1.s8   {d0-d1}, [%1]		\n"
					"vst1.s8   {d0-d1}, [%0]!		\n"
					: "+r"(tmpptr)		// %0
					: "r"(inptr)		// %1
					: "memory", "q0"
					);
				inptr += cstep;
#else
				memcpy(tmpptr, inptr, 16);
				tmpptr += 16;
				inptr += cstep;
#endif
			}
		}

		int out_cstep = get_blob_size(8 * size);
		int k_cstep = 4 * c;
		int nn_oc = outch / 8;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->c = outch;
		out_blob->cstep = out_cstep;
		out_blob->packp = 8;
		out_blob->elemc = 1;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pq = 0; pq < nn_oc; pq++)
		{
			signed char* out_mem_ch = ((signed char*)out_blob->mem) + pq * out_cstep;

			for (int it = 0; it < 2; it++)
			{
				const signed char* kmem_ch = weight + (pq * 2 + it) * k_cstep;

				signed char* outptr0 = out_mem_ch + it * 4; // out_mem + pq * out_cstep;

				const signed char* in_mem_ch = (signed char*)mem_tm;

				const float* sc0 = mem_scale + pq * 8 + it * 4;
				const float* bs0 = mem_bias + pq * 8 + it * 4;
				const float* pr0 = mem_prelu + pq * 8 + it * 4;

				int i = 0;
				for (; i + 1 < size; i += 2)
				{
					const signed char* tmpptr = in_mem_ch;	// in_mem + (i / 2) * cstep;						// tmp.channel(i / 2);
					const signed char* kptr0 = kmem_ch;		// weight + (pq * 2 + it) * k_cstep; 				// kernel.channel(p);
#if __ARM_NEON
					asm volatile(
						"veor       q0, q0              \n"
						"veor       q1, q1              \n"
						"veor       q2, q2              \n"
						"veor       q3, q3              \n"
						"veor       q4, q4              \n"
						"veor       q5, q5              \n"
						"veor       q6, q6              \n"
						"veor       q7, q7              \n"

						"mov        r6, #32             \n"
						"add        r5, %2, #16         \n"

						"lsr        r4, %3, #1          \n" // r4 = nn = size >> 1
						"cmp        r4, #0              \n"
						"beq        1f                  \n"

						"0:                             \n"

						"pld        [%2, #256]          \n"
						"pld        [%1, #128]          \n"

						"vld1.s8    {d20-d21}, [%2 :128], r6 \n" // _w01
						"vld1.s8    {d16-d19}, [%1 :128]! \n" // _val0 _val1
						"vld1.s8    {d22-d23}, [%2 :128], r6 \n" // _w45

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vmlal.s8   q12, d18, d22       \n"
						"vmlal.s8   q13, d18, d23       \n"
						"vmlal.s8   q14, d19, d22       \n"
						"vmlal.s8   q15, d19, d23       \n"

						"vpadal.s16 q0, q12             \n"
						"vpadal.s16 q1, q13             \n"
						"vpadal.s16 q4, q14             \n"
						"vpadal.s16 q5, q15             \n"

						"pld		[r5, #256]			\n"
						"vld1.s8    {d20-d21}, [r5 :128], r6 \n" // _w23
						"vld1.s8    {d22-d23}, [r5 :128], r6 \n" // _w67

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vmlal.s8   q12, d18, d22       \n"
						"vmlal.s8   q13, d18, d23       \n"
						"vmlal.s8   q14, d19, d22       \n"
						"vmlal.s8   q15, d19, d23       \n"

						"vpadal.s16 q2, q12             \n"
						"vpadal.s16 q3, q13             \n"
						"vpadal.s16 q6, q14             \n"
						"vpadal.s16 q7, q15             \n"

						"subs       r4, r4, #1          \n"
						"bne        0b                  \n"

						"1:                             \n"
						"ands        r4, %3, #1          \n" // r4 = remain = size & 1
						"beq        2f                  \n"

						"pld		[%1, #64]			\n"
						"vld1.s8    {d16-d17}, [%1 :128]! \n" // _val
						"pld		[%2, #128]			\n"
						"vld1.s8    {d20-d21}, [%2 :128]! \n" // _w01
						"vld1.s8    {d22-d23}, [%2 :128]! \n" // _w23

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vpadal.s16 q0, q12             \n"
						"vpadal.s16 q1, q13             \n"
						"vpadal.s16 q4, q14             \n"
						"vpadal.s16 q5, q15             \n"

						"vmull.s8   q12, d16, d22       \n"
						"vmull.s8   q13, d16, d23       \n"
						"vmull.s8   q14, d17, d22       \n"
						"vmull.s8   q15, d17, d23       \n"

						"vpadal.s16 q2, q12             \n"
						"vpadal.s16 q3, q13             \n"
						"vpadal.s16 q6, q14             \n"
						"vpadal.s16 q7, q15             \n"

						"2:                             \n"

						"vpadd.s32  d16, d0, d1         \n"
						"vpadd.s32  d17, d2, d3         \n"
						"vpadd.s32  d18, d4, d5         \n"
						"vpadd.s32  d19, d6, d7         \n"

						"vld1.f32	{d4-d5}, [%4 :128]	\n"

						"vpadd.s32  d20, d8, d9         \n"
						"vpadd.s32  d21, d10, d11       \n"
						"vpadd.s32  d22, d12, d13       \n"
						"vpadd.s32  d23, d14, d15       \n"

						"vld1.f32	{d0-d1}, [%5 :128]	\n"

						"vpadd.s32  d6, d16, d17        \n"
						"vpadd.s32  d7, d18, d19        \n"

						"vmov		q1, q0				\n"
						"mov		r4, #8				\n"

						"veor       q8, q8				\n"

						"vpadd.s32  d8, d20, d21        \n"
						"vpadd.s32  d9, d22, d23        \n"

						"vld1.f32	{d10-d11}, [%6 :128] \n"

						"vcvt.f32.s32	q3, q3			\n"
						"vcvt.f32.s32	q4, q4			\n"

						"vmla.f32	q0, q3, q2			\n"
						"vmla.f32	q1, q4, q2			\n"

						"vcle.f32	q2, q0, q8			\n"
						"vcle.f32	q3, q1, q8			\n"
						"vmul.f32	q6, q0, q5			\n"
						"vmul.f32	q7, q1, q5			\n"
						"vbit.f32	q0, q6, q2			\n"
						"vbit.f32	q1, q7, q3			\n"

						"vcvtr.s32.f32 s0, s0            \n"
						"vcvtr.s32.f32 s1, s1            \n"
						"vcvtr.s32.f32 s2, s2            \n"
						"vcvtr.s32.f32 s3, s3            \n"

						"vcvtr.s32.f32 s4, s4            \n"
						"vcvtr.s32.f32 s5, s5            \n"
						"vcvtr.s32.f32 s6, s6            \n"
						"vcvtr.s32.f32 s7, s7            \n"

						"vqmovn.s32 d0, q0              \n"
						"vqmovn.s32 d1, q1              \n"

						"vqmovn.s16 d0, q0              \n"

						"vst1.s32   {d0[0]}, [%0], r4	\n"
						"vst1.s32   {d0[1]}, [%0], r4	\n"

						:
					"+r"(outptr0),		// %0
						"+r"(tmpptr),		// %1
						"+r"(kptr0)			// %2
						:
						"r"(nn_c),			// %3
						"r"(sc0),			// %4
						"r"(bs0),			// %5
						"r"(pr0)			// %6
						: "memory", "r4", "r5", "r6", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15");
#else
					int sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0, sum6 = 0, sum7 = 0;

					for (int j = 0; j < nn_c; j++)
					{
						sum0 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum1 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum2 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum3 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 -= 24;
						tmpptr += 8;
						sum4 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum5 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum6 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum7 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						tmpptr += 8;
					}

					float rVal;

					rVal = (sum0 * sc0[0] + bs0[0]); if (rVal < 0.0f) rVal *= pr0[0]; outptr0[0] = float2int8(rVal);
					rVal = (sum1 * sc0[1] + bs0[1]); if (rVal < 0.0f) rVal *= pr0[1]; outptr0[1] = float2int8(rVal);
					rVal = (sum2 * sc0[2] + bs0[2]); if (rVal < 0.0f) rVal *= pr0[2]; outptr0[2] = float2int8(rVal);
					rVal = (sum3 * sc0[3] + bs0[3]); if (rVal < 0.0f) rVal *= pr0[3]; outptr0[3] = float2int8(rVal);
					rVal = (sum4 * sc0[0] + bs0[0]); if (rVal < 0.0f) rVal *= pr0[0]; outptr0[8] = float2int8(rVal);
					rVal = (sum5 * sc0[1] + bs0[1]); if (rVal < 0.0f) rVal *= pr0[1]; outptr0[9] = float2int8(rVal);
					rVal = (sum6 * sc0[2] + bs0[2]); if (rVal < 0.0f) rVal *= pr0[2]; outptr0[10] = float2int8(rVal);
					rVal = (sum7 * sc0[3] + bs0[3]); if (rVal < 0.0f) rVal *= pr0[3]; outptr0[11] = float2int8(rVal);

					outptr0 += 16;
#endif
					in_mem_ch += m_cstep;
				}
			}
		}
	}

	void conv_k1s1_nf_pack8(ennq_blob* in_blob, ennq_blob* out_blob, int outch, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		// assert(inch % 8 == 0 and outch % 8 == 0 and size % 2 = 0)
		int w = in_blob->w;
		int h = in_blob->h;
		int c = in_blob->c;
		int cstep = in_blob->cstep;

		int outw = w;
		int outh = h;

		int size = outw * outh;
		int m_cstep = c * 2;
		int nn_c = c / 8;
		int nn_size = size >> 1;

		for (int ii = 0; ii < nn_size; ii++)
		{
			signed char* tmpptr = ((signed char*)mem_tm) + m_cstep * ii;
			signed char* inptr = ((signed char*)in_blob->mem) + (ii << 4);
			for (int qq = 0; qq < nn_c; qq++)
			{
#if __ARM_NEON
				asm volatile(
					"pld        [%1, #256]			\n"
					"vld1.s8   {d0-d1}, [%1]		\n"
					"vst1.s8   {d0-d1}, [%0]!		\n"
					: "+r"(tmpptr)		// %0
					: "r"(inptr)		// %1
					: "memory", "q0"
					);
				inptr += cstep;
#else
				memcpy(tmpptr, inptr, 16);
				tmpptr += 16;
				inptr += cstep;
#endif
			}
		}

		int out_cstep = 8 * size;
		int k_cstep = 4 * c;
		int nn_oc = outch / 8;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->c = outch;
		out_blob->cstep = out_cstep;
		out_blob->packp = 8;
		out_blob->elemc = 4;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int pq = 0; pq < nn_oc; pq++)
		{
			float* out_mem_ch = ((float*)out_blob->mem) + pq * out_cstep;

			for (int it = 0; it < 2; it++)
			{
				const signed char* kmem_ch = weight + (pq * 2 + it) * k_cstep;
				float* outptr0 = out_mem_ch + it * 4; // out_mem + pq * out_cstep;

				const signed char* in_mem_ch = (signed char*)mem_tm;

				const float* sc0 = mem_scale + pq * 8 + it * 4;
				const float* bs0 = mem_bias + pq * 8 + it * 4;

				int i = 0;
				for (; i + 1 < size; i += 2)
				{
					const signed char* tmpptr = in_mem_ch;	// in_mem + (i / 2) * cstep;						// tmp.channel(i / 2);
					const signed char* kptr0 = kmem_ch;		// weight + (pq * 2 + it) * k_cstep; 				// kernel.channel(p);
#if __ARM_NEON
					asm volatile(
						"veor       q0, q0              \n"
						"veor       q1, q1              \n"
						"veor       q2, q2              \n"
						"veor       q3, q3              \n"
						"veor       q4, q4              \n"
						"veor       q5, q5              \n"
						"veor       q6, q6              \n"
						"veor       q7, q7              \n"

						"mov        r6, #32             \n"
						"add        r5, %2, #16         \n"

						"lsr        r4, %3, #1          \n" // r4 = nn = size >> 1
						"cmp        r4, #0              \n"
						"beq        1f                  \n"

						"0:                             \n"

						"pld        [%2, #128]          \n"
						"pld        [%1, #64]          \n"

						"vld1.s8    {d20-d21}, [%2 :128], r6 \n" // _w01
						"vld1.s8    {d16-d19}, [%1 :128]! \n" // _val0 _val1
						"vld1.s8    {d22-d23}, [%2 :128], r6 \n" // _w45

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vmlal.s8   q12, d18, d22       \n"
						"vmlal.s8   q13, d18, d23       \n"
						"vmlal.s8   q14, d19, d22       \n"
						"vmlal.s8   q15, d19, d23       \n"

						"vpadal.s16 q0, q12             \n"
						"vpadal.s16 q1, q13             \n"
						"vpadal.s16 q4, q14             \n"
						"vpadal.s16 q5, q15             \n"

						"pld		[r5, #128]			\n"
						"vld1.s8    {d20-d21}, [r5 :128], r6 \n" // _w23
						"vld1.s8    {d22-d23}, [r5 :128], r6 \n" // _w67

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vmlal.s8   q12, d18, d22       \n"
						"vmlal.s8   q13, d18, d23       \n"
						"vmlal.s8   q14, d19, d22       \n"
						"vmlal.s8   q15, d19, d23       \n"

						"vpadal.s16 q2, q12             \n"
						"vpadal.s16 q3, q13             \n"
						"vpadal.s16 q6, q14             \n"
						"vpadal.s16 q7, q15             \n"

						"subs       r4, r4, #1          \n"
						"bne        0b                  \n"

						"1:                             \n"
						"ands        r4, %3, #1          \n" // r4 = remain = size & 1
						"beq        2f                  \n"

						"pld		[%1, #64]			\n"
						"vld1.s8    {d16-d17}, [%1 :128]! \n" // _val
						"pld		[%2, #128]			\n"
						"vld1.s8    {d20-d21}, [%2 :128]! \n" // _w01
						"vld1.s8    {d22-d23}, [%2 :128]! \n" // _w23

						"vmull.s8   q12, d16, d20       \n"
						"vmull.s8   q13, d16, d21       \n"
						"vmull.s8   q14, d17, d20       \n"
						"vmull.s8   q15, d17, d21       \n"

						"vpadal.s16 q0, q12             \n"
						"vpadal.s16 q1, q13             \n"
						"vpadal.s16 q4, q14             \n"
						"vpadal.s16 q5, q15             \n"

						"vmull.s8   q12, d16, d22       \n"
						"vmull.s8   q13, d16, d23       \n"
						"vmull.s8   q14, d17, d22       \n"
						"vmull.s8   q15, d17, d23       \n"

						"vpadal.s16 q2, q12             \n"
						"vpadal.s16 q3, q13             \n"
						"vpadal.s16 q6, q14             \n"
						"vpadal.s16 q7, q15             \n"

						"2:                             \n"

						"vpadd.s32  d16, d0, d1         \n"
						"vpadd.s32  d17, d2, d3         \n"
						"vpadd.s32  d18, d4, d5         \n"
						"vpadd.s32  d19, d6, d7         \n"

						"vld1.f32	{d4-d5}, [%4 :128]	\n"

						"vpadd.s32  d20, d8, d9         \n"
						"vpadd.s32  d21, d10, d11       \n"
						"vpadd.s32  d22, d12, d13       \n"
						"vpadd.s32  d23, d14, d15       \n"

						"vld1.f32	{d0-d1}, [%5 :128]	\n"

						"vpadd.s32  d6, d16, d17        \n"
						"vpadd.s32  d7, d18, d19        \n"

						"vmov		q1, q0				\n"
						"mov		r4, #32				\n"

						"vpadd.s32  d8, d20, d21        \n"
						"vpadd.s32  d9, d22, d23        \n"

						"vcvt.f32.s32	q3, q3			\n"
						"vcvt.f32.s32	q4, q4			\n"

						"vmla.f32	q0, q3, q2			\n"
						"vmla.f32	q1, q4, q2			\n"

						"vst1.f32   {d0-d1}, [%0 :128], r4	\n"
						"vst1.f32   {d2-d3}, [%0 :128], r4	\n"

						:
					"+r"(outptr0),		// %0
						"+r"(tmpptr),		// %1
						"+r"(kptr0)			// %2
						:
						"r"(nn_c),			// %3
						"r"(sc0),			// %4
						"r"(bs0)			// %5
						: "memory", "r4", "r5", "r6", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15");
#else
					int sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0, sum6 = 0, sum7 = 0;

					for (int j = 0; j < nn_c; j++)
					{
						sum0 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum1 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum2 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum3 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 -= 24;
						tmpptr += 8;
						sum4 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum5 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum6 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						sum7 += tmpptr[0] * kptr0[0] + tmpptr[1] * kptr0[1] + tmpptr[2] * kptr0[2] + tmpptr[3] * kptr0[3] + tmpptr[4] * kptr0[4] + tmpptr[5] * kptr0[5] + tmpptr[6] * kptr0[6] + tmpptr[7] * kptr0[7]; kptr0 += 8;
						tmpptr += 8;
					}

					outptr0[0] = (sum0 * sc0[0] + bs0[0]);
					outptr0[1] = (sum1 * sc0[1] + bs0[1]);
					outptr0[2] = (sum2 * sc0[2] + bs0[2]);
					outptr0[3] = (sum3 * sc0[3] + bs0[3]);
					outptr0[8] = (sum4 * sc0[0] + bs0[0]);
					outptr0[9] = (sum5 * sc0[1] + bs0[1]);
					outptr0[10] = (sum6 * sc0[2] + bs0[2]);
					outptr0[11] = (sum7 * sc0[3] + bs0[3]);

					outptr0 += 16;
#endif
					in_mem_ch += m_cstep;
				}
			}
		}
	}

	// conv_k3s2_nn_r_pack1to8
	// static void conv_im2col_sgemm_int8_neon(const Mat &bottom_blob, Mat &top_blob, const Mat & kernel_tm, const int kernel_w, const int kernel_h, const int stride_w, const int stride_h, const Option& opt)
	// conv_im2col_sgemm_transform_kernel_int8_neon
	// (8 * kernel_size) * inch * (out_size/8 + out_size%8)
	void conv_kxsx_nn_r_pack1to8(ennq_blob* in_blob, ennq_blob* out_blob, int ch_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, void* mem_tm)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		signed char* inmem = (signed char*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int outch = ch_out;
		int outcstep = get_blob_size(outw * outh * 8);
		signed char* outmem = (signed char*)out_blob->mem;

		int size_out = outw * outh;

		int tm_cstep = 8 * inch * dim_kernel * dim_kernel;
		int kernel_cstep = 4 * dim_kernel * dim_kernel * inch;

		out_blob->c = ch_out;
		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = get_blob_size(8 * size_out);
		out_blob->packp = 8;
		out_blob->elemc = 1;

		// im2col
		// bottom_im2col memory packed 8 x 8
		{
			int i, ic, kx, ky;
			int x, x0, x1, x2, x3, x4, x5, x6, x7;
			int y, y0, y1, y2, y3, y4, y5, y6, y7;
			int xy0, xy1, xy2, xy3, xy4, xy5, xy6, xy7;

			int nn = size_out >> 3;

			for (x = 0, y = 0, i = 0; i < nn; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + i * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= outw) { x = 0; y++; }
				x1 = x; y1 = y; x++; if (x >= outw) { x = 0; y++; }
				x2 = x; y2 = y; x++; if (x >= outw) { x = 0; y++; }
				x3 = x; y3 = y; x++; if (x >= outw) { x = 0; y++; }
				x4 = x; y4 = y; x++; if (x >= outw) { x = 0; y++; }
				x5 = x; y5 = y; x++; if (x >= outw) { x = 0; y++; }
				x6 = x; y6 = y; x++; if (x >= outw) { x = 0; y++; }
				x7 = x; y7 = y; x++; if (x >= outw) { x = 0; y++; }

				xy0 = (x0 + y0 * w) * stride;
				xy1 = (x1 + y1 * w) * stride;
				xy2 = (x2 + y2 * w) * stride;
				xy3 = (x3 + y3 * w) * stride;
				xy4 = (x4 + y4 * w) * stride;
				xy5 = (x5 + y5 * w) * stride;
				xy6 = (x6 + y6 * w) * stride;
				xy7 = (x7 + y7 * w) * stride;

				const signed char* imin_iter_ic = inmem;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < inch; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							*tm_iter++ = imin_iter_kx[xy1];
							*tm_iter++ = imin_iter_kx[xy2];
							*tm_iter++ = imin_iter_kx[xy3];
							*tm_iter++ = imin_iter_kx[xy4];
							*tm_iter++ = imin_iter_kx[xy5];
							*tm_iter++ = imin_iter_kx[xy6];
							*tm_iter++ = imin_iter_kx[xy7];
							imin_iter_kx++;
						}
						imin_iter_ky += w;
					}
					imin_iter_ic += cstep;
				}
			}
		}

		// sgemm(int M, int N, int L, float* A, float* B, float* C)
		{
			//int M = outch;  // outch
			int N = outw * outh; // outsize or out stride
			int L = dim_kernel * dim_kernel * inch; // ksize * inch

			int nn_outch = 0;

			nn_outch = outch >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int pp = 0; pp < nn_outch; pp++)
			{
				int i = pp * 4;

				signed char* output0 = outmem + (pp / 2) * outcstep; // top_blob.channel(i);
				if ((pp & 1) == 1) output0 += 4;

				for (int j = 0; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// K loop
						"vmov.s32    q8, #0             \n"
						"vmov.s32    q9, #0             \n"
						"vmov.s32    q10, #0            \n"
						"vmov.s32    q11, #0            \n"
						"vmov.s32    q12, #0            \n"
						"vmov.s32    q13, #0            \n"
						"vmov.s32    q14, #0            \n"
						"vmov.s32    q15, #0            \n"

						"lsr         r4, %3, #3         \n"// r4 = nn = L >> 3
						"cmp         r4, #0             \n"
						"beq         1f                 \n"

						"0:                             \n"// for(; nn != 0; nn--)
						"pld         [%1, #128]         \n"
						"vld1.s8     {d8-d11}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q7, d11            \n"// a30-a37
						"vmovl.s8    q6, d10            \n"// a20-a27                    
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"pld         [%2, #128]         \n"
						"vld1.s8     {d0-d3}, [%2]!     \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q3, d3             \n"// k06-k36,k07-k37
						"vmovl.s8    q2, d2             \n"// k04-k34,k05-k35
						"vmovl.s8    q1, d1             \n"// k02-k32,k03-k33
						"vmovl.s8    q0, d0             \n"// k00-k30,k01-k31

						"vmlal.s16   q8, d8, d0[0]      \n"// sum0 = (a00-a07) * k00
						"vmlal.s16   q9, d9, d0[0]      \n"
						"vmlal.s16   q10, d8, d0[1]     \n"// sum1 = (a00-a07) * k10
						"vmlal.s16   q11, d9, d0[1]     \n"
						"vmlal.s16   q12, d8, d0[2]     \n"// sum2 = (a00-a07) * k20
						"vmlal.s16   q13, d9, d0[2]     \n"
						"vmlal.s16   q14, d8, d0[3]     \n"// sum3 = (a00-a07) * k30
						"vmlal.s16   q15, d9, d0[3]     \n"

						"vmlal.s16   q8, d10, d1[0]     \n"// sum0 += (a10-a17) * k01
						"vmlal.s16   q9, d11, d1[0]     \n"
						"vmlal.s16   q10, d10, d1[1]    \n"// sum1 += (a10-a17) * k11
						"vmlal.s16   q11, d11, d1[1]    \n"
						"vmlal.s16   q12, d10, d1[2]    \n"// sum2 += (a10-a17) * k21
						"vmlal.s16   q13, d11, d1[2]    \n"
						"vmlal.s16   q14, d10, d1[3]    \n"// sum3 += (a10-a17) * k31
						"vmlal.s16   q15, d11, d1[3]    \n"

						"pld         [%1, #128]         \n"
						"vld1.s8     {d8-d9}, [%1]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"vmlal.s16   q8, d12, d2[0]     \n"// sum0 += (a20-a27) * k02
						"vmlal.s16   q9, d13, d2[0]     \n"
						"vmlal.s16   q10, d12, d2[1]    \n"// sum1 += (a20-a27) * k12
						"vmlal.s16   q11, d13, d2[1]    \n"
						"vmlal.s16   q12, d12, d2[2]    \n"// sum2 += (a20-a27) * k22
						"vmlal.s16   q13, d13, d2[2]    \n"
						"vmlal.s16   q14, d12, d2[3]    \n"// sum3 += (a20-a27) * k32
						"vmlal.s16   q15, d13, d2[3]    \n"

						"vmlal.s16   q8, d14, d3[0]     \n"// sum0 += (a30-a37) * k03
						"vmlal.s16   q9, d15, d3[0]     \n"
						"vmlal.s16   q10, d14, d3[1]    \n"// sum1 += (a30-a37) * k13
						"vmlal.s16   q11, d15, d3[1]    \n"
						"vmlal.s16   q12, d14, d3[2]    \n"// sum2 += (a30-a37) * k23
						"vmlal.s16   q13, d15, d3[2]    \n"
						"vmlal.s16   q14, d14, d3[3]    \n"// sum3 += (a30-a37) * k33
						"vmlal.s16   q15, d15, d3[3]    \n"

						"pld         [%1, #128]         \n"
						"vld1.s8     {d0-d1}, [%1]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q1, d1             \n"// a10-a17
						"vmovl.s8    q0, d0             \n"// a00-a07

						"vmlal.s16   q8, d8, d4[0]      \n"// sum0 += (a40-a47) * k04
						"vmlal.s16   q9, d9, d4[0]      \n"
						"vmlal.s16   q10, d8, d4[1]     \n"// sum1 += (a40-a47) * k14
						"vmlal.s16   q11, d9, d4[1]     \n"
						"vmlal.s16   q12, d8, d4[2]     \n"// sum2 += (a40-a47) * k24
						"vmlal.s16   q13, d9, d4[2]     \n"
						"vmlal.s16   q14, d8, d4[3]     \n"// sum3 += (a40-a47) * k34
						"vmlal.s16   q15, d9, d4[3]     \n"

						"vmlal.s16   q8, d10, d5[0]     \n"// sum0 += (a50-a57) * k05
						"vmlal.s16   q9, d11, d5[0]     \n"
						"vmlal.s16   q10, d10, d5[1]    \n"// sum1 += (a50-a57) * k15
						"vmlal.s16   q11, d11, d5[1]    \n"
						"vmlal.s16   q12, d10, d5[2]    \n"// sum2 += (a50-a57) * k25
						"vmlal.s16   q13, d11, d5[2]    \n"
						"vmlal.s16   q14, d10, d5[3]    \n"// sum3 += (a50-a57) * k35
						"vmlal.s16   q15, d11, d5[3]    \n"

						"vmlal.s16   q8, d0, d6[0]      \n"// sum0 += (a60-a67) * k06
						"vmlal.s16   q9, d1, d6[0]      \n"
						"vmlal.s16   q10, d0, d6[1]     \n"// sum1 += (a60-a67) * k16
						"vmlal.s16   q11, d1, d6[1]     \n"
						"vmlal.s16   q12, d0, d6[2]     \n"// sum2 += (a60-a67) * k26
						"vmlal.s16   q13, d1, d6[2]     \n"
						"vmlal.s16   q14, d0, d6[3]     \n"// sum3 += (a60-a67) * k36
						"vmlal.s16   q15, d1, d6[3]     \n"

						"vmlal.s16   q8, d2, d7[0]      \n"// sum0 += (a70-a77) * k07
						"vmlal.s16   q9, d3, d7[0]      \n"
						"vmlal.s16   q10, d2, d7[1]     \n"// sum1 += (a70-a77) * k17
						"vmlal.s16   q11, d3, d7[1]     \n"
						"vmlal.s16   q12, d2, d7[2]     \n"// sum2 += (a70-a77) * k27
						"vmlal.s16   q13, d3, d7[2]     \n"
						"vmlal.s16   q14, d2, d7[3]     \n"// sum3 += (a70-a77) * k37
						"vmlal.s16   q15, d3, d7[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         0b                 \n"// end for

						"1:                             \n"
						// remain loop
						"and         r4, %3, #7         \n"// r4 = remain = inch & 7
						"cmp         r4, #0             \n"
						"beq         3f                 \n"

						"2:                             \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%1]!        \n"// tmpr a00-a70    a(inch)(data)
						"vld1.s8     {d0}, [%2]         \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2             \n"
						"vmovl.s8    q0, d0             \n"
						"add         %2, #4             \n"

						"vmlal.s16   q8, d2, d0[0]      \n"// sum0 += (a00-a70) * k00
						"vmlal.s16   q9, d3, d0[0]      \n"
						"vmlal.s16   q10, d2, d0[1]     \n"// sum1 += (a00-a70) * k10
						"vmlal.s16   q11, d3, d0[1]     \n"
						"vmlal.s16   q12, d2, d0[2]     \n"// sum2 += (a00-a70) * k20
						"vmlal.s16   q13, d3, d0[2]     \n"
						"vmlal.s16   q14, d2, d0[3]     \n"// sum3 += (a00-a70) * k30
						"vmlal.s16   q15, d3, d0[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         2b                 \n"

						"3:                             \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%4]      \n"

						"vcvt.f32.s32 q8, q8            \n"
						"vcvt.f32.s32 q9, q9            \n"
						"vcvt.f32.s32 q10, q10          \n"
						"vcvt.f32.s32 q11, q11          \n"

						"vdup.32     q1, %5             \n"
						"vdup.32     q3, %6             \n"
						"vmov.32     q2, q1             \n"
						"vmov.32     q4, q3             \n"

						"vmla.f32    q1, q8, d0[0]      \n"
						"vmla.f32    q2, q9, d0[0]      \n"
						"vmla.f32    q3, q10, d0[1]     \n"
						"vmla.f32    q4, q11, d0[1]     \n"

						"vcvt.f32.s32 q12, q12          \n"
						"vcvt.f32.s32 q13, q13          \n"
						"vcvt.f32.s32 q14, q14          \n"
						"vcvt.f32.s32 q15, q15          \n"

						"vdup.32     q5, %7             \n"
						"vdup.32     q7, %8            \n"
						"vmov.32     q6, q5             \n"
						"vmov.32     q8, q7             \n"

						"vmla.f32    q5, q12, d1[0]    \n"
						"vmla.f32    q6, q13, d1[0]    \n"
						"vmla.f32    q7, q14, d1[1]    \n"
						"vmla.f32    q8, q15, d1[1]    \n"

						"veor       q9, q9              \n"

						"vcvt.s32.f32 q1, q1            \n"
						"vcvt.s32.f32 q2, q2            \n"
						"vcvt.s32.f32 q3, q3            \n"
						"vcvt.s32.f32 q4, q4            \n"
						"vcvt.s32.f32 q5, q5            \n"
						"vcvt.s32.f32 q6, q6            \n"
						"vcvt.s32.f32 q7, q7            \n"
						"vcvt.s32.f32 q8, q8            \n"

						"vqmovn.s32 d0, q1              \n"
						"vqmovn.s32 d1, q2              \n"
						"vqmovn.s32 d2, q3              \n"
						"vqmovn.s32 d3, q4              \n"
						"vqmovn.s32 d4, q5              \n"
						"vqmovn.s32 d5, q6              \n"
						"vqmovn.s32 d6, q7              \n"
						"vqmovn.s32 d7, q8              \n"

						"vzip.16	q0, q1				\n"
						"vzip.16	q2, q3				\n"
						"vzip.32	q0, q2				\n"
						"vzip.32	q1, q3				\n"

						"vqmovn.s16 d0, q0              \n"
						"vqmovn.s16 d1, q2              \n"
						"vqmovn.s16 d2, q1              \n"
						"vqmovn.s16 d3, q3              \n"

						"mov		r4, #8				\n"

						"vmax.s8    q0, q0, q9          \n"
						"vmax.s8    q1, q1, q9          \n"

						"vst1.s32	{d0[0]}, [%0], r4	\n"
						"vst1.s32	{d0[1]}, [%0], r4	\n"
						"vst1.s32	{d1[0]}, [%0], r4	\n"
						"vst1.s32	{d1[1]}, [%0], r4	\n"
						"vst1.s32	{d2[0]}, [%0], r4	\n"
						"vst1.s32	{d2[1]}, [%0], r4	\n"
						"vst1.s32	{d3[0]}, [%0], r4	\n"
						"vst1.s32	{d3[1]}, [%0], r4	\n"

						: "+r"(output0), // %0
						"+r"(vb),      // %1
						"+r"(va)       // %2
						: "r"(L),        // %3  
						"r"(mem_scale + i),   // %4
						"r"(mem_bias[i]),     // %5
						"r"(mem_bias[i + 1]),   // %6
						"r"(mem_bias[i + 2]),   // %7
						"r"(mem_bias[i + 3])    // %8
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
#else
					int sum0[8] = { 0 };
					int sum1[8] = { 0 };
					int sum2[8] = { 0 };
					int sum3[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 8];
							sum1[n] += (int)va[1] * vb[n + 8];
							sum2[n] += (int)va[2] * vb[n + 8];
							sum3[n] += (int)va[3] * vb[n + 8];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 16];
							sum1[n] += (int)va[1] * vb[n + 16];
							sum2[n] += (int)va[2] * vb[n + 16];
							sum3[n] += (int)va[3] * vb[n + 16];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 24];
							sum1[n] += (int)va[1] * vb[n + 24];
							sum2[n] += (int)va[2] * vb[n + 24];
							sum3[n] += (int)va[3] * vb[n + 24];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 32];
							sum1[n] += (int)va[1] * vb[n + 32];
							sum2[n] += (int)va[2] * vb[n + 32];
							sum3[n] += (int)va[3] * vb[n + 32];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 40];
							sum1[n] += (int)va[1] * vb[n + 40];
							sum2[n] += (int)va[2] * vb[n + 40];
							sum3[n] += (int)va[3] * vb[n + 40];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 48];
							sum1[n] += (int)va[1] * vb[n + 48];
							sum2[n] += (int)va[2] * vb[n + 48];
							sum3[n] += (int)va[3] * vb[n + 48];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 56];
							sum1[n] += (int)va[1] * vb[n + 56];
							sum2[n] += (int)va[2] * vb[n + 56];
							sum3[n] += (int)va[3] * vb[n + 56];
							va -= 28;
						}

						va += 32;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
						}

						va += 4;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						int val;
						val = (int)(sum0[n] * mem_scale[i + 0] + mem_bias[i + 0]); if (val < 0) val = 0; if (val > 127) val = 127; output0[0] = (signed char)val;
						val = (int)(sum1[n] * mem_scale[i + 1] + mem_bias[i + 1]); if (val < 0) val = 0; if (val > 127) val = 127; output0[1] = (signed char)val;
						val = (int)(sum2[n] * mem_scale[i + 2] + mem_bias[i + 2]); if (val < 0) val = 0; if (val > 127) val = 127; output0[2] = (signed char)val;
						val = (int)(sum3[n] * mem_scale[i + 3] + mem_bias[i + 3]); if (val < 0) val = 0; if (val > 127) val = 127; output0[3] = (signed char)val;
						output0 += 8;
					}
#endif // __ARM_NEON
				}
			}
		}
	}

	void conv_kxsx_nn_p_pack1to8(ennq_blob* in_blob, ennq_blob* out_blob, int ch_out, int dim_kernel, int stride, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu, void* mem_tm)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		signed char* inmem = (signed char*)in_blob->mem;

		int outw = (w - dim_kernel) / stride + 1;
		int outh = (h - dim_kernel) / stride + 1;
		int outch = ch_out;
		int outcstep = get_blob_size(outw * outh * 8);
		signed char* outmem = (signed char*)out_blob->mem;

		int size_out = outw * outh;

		int tm_cstep = 8 * inch * dim_kernel * dim_kernel;
		int kernel_cstep = 4 * dim_kernel * dim_kernel * inch;

		out_blob->c = ch_out;
		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = get_blob_size(8 * size_out);
		out_blob->packp = 8;
		out_blob->elemc = 1;

		// im2col
		// bottom_im2col memory packed 8 x 8
		{
			int i, ic, kx, ky;
			int x, x0, x1, x2, x3, x4, x5, x6, x7;
			int y, y0, y1, y2, y3, y4, y5, y6, y7;
			int xy0, xy1, xy2, xy3, xy4, xy5, xy6, xy7;

			int nn = size_out >> 3;

			for (x = 0, y = 0, i = 0; i < nn; i++)
			{
				signed char* tm_iter = ((signed char*)mem_tm) + i * tm_cstep;

				x0 = x; y0 = y; x++; if (x >= outw) { x = 0; y++; }
				x1 = x; y1 = y; x++; if (x >= outw) { x = 0; y++; }
				x2 = x; y2 = y; x++; if (x >= outw) { x = 0; y++; }
				x3 = x; y3 = y; x++; if (x >= outw) { x = 0; y++; }
				x4 = x; y4 = y; x++; if (x >= outw) { x = 0; y++; }
				x5 = x; y5 = y; x++; if (x >= outw) { x = 0; y++; }
				x6 = x; y6 = y; x++; if (x >= outw) { x = 0; y++; }
				x7 = x; y7 = y; x++; if (x >= outw) { x = 0; y++; }

				xy0 = (x0 + y0 * w) * stride;
				xy1 = (x1 + y1 * w) * stride;
				xy2 = (x2 + y2 * w) * stride;
				xy3 = (x3 + y3 * w) * stride;
				xy4 = (x4 + y4 * w) * stride;
				xy5 = (x5 + y5 * w) * stride;
				xy6 = (x6 + y6 * w) * stride;
				xy7 = (x7 + y7 * w) * stride;

				const signed char* imin_iter_ic = inmem;
				const signed char* imin_iter_ky;
				const signed char* imin_iter_kx;
				for (ic = 0; ic < inch; ic++)
				{
					imin_iter_ky = imin_iter_ic;
					for (ky = 0; ky < dim_kernel; ky++)
					{
						imin_iter_kx = imin_iter_ky;
						for (kx = 0; kx < dim_kernel; kx++)
						{
							*tm_iter++ = imin_iter_kx[xy0];
							*tm_iter++ = imin_iter_kx[xy1];
							*tm_iter++ = imin_iter_kx[xy2];
							*tm_iter++ = imin_iter_kx[xy3];
							*tm_iter++ = imin_iter_kx[xy4];
							*tm_iter++ = imin_iter_kx[xy5];
							*tm_iter++ = imin_iter_kx[xy6];
							*tm_iter++ = imin_iter_kx[xy7];
							imin_iter_kx++;
						}
						imin_iter_ky += w;
					}
					imin_iter_ic += cstep;
				}
			}
		}

		// sgemm(int M, int N, int L, float* A, float* B, float* C)
		{
			//int M = outch;  // outch
			int N = outw * outh; // outsize or out stride
			int L = dim_kernel * dim_kernel * inch; // ksize * inch

			int nn_outch = 0;

			nn_outch = outch >> 2;

#if ((ENGINE_THREAD_COUNT) != 1)
			int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
			for (int pp = 0; pp < nn_outch; pp++)
			{
				int i = pp * 4;

				signed char* output0 = outmem + (pp / 2) * outcstep; // top_blob.channel(i);
				if ((pp & 1) == 1) output0 += 4;

				for (int j = 0; j + 7 < N; j = j + 8)
				{
					signed char* vb = ((signed char*)mem_tm) + (j / 8) * tm_cstep; // bottom_tm.channel(j / 8);
					const signed char* va = weight + (i / 4) * kernel_cstep; // kernel_tm.channel(i / 4);
#if __ARM_NEON
					asm volatile(
						// K loop
						"vmov.s32    q8, #0             \n"
						"vmov.s32    q9, #0             \n"
						"vmov.s32    q10, #0            \n"
						"vmov.s32    q11, #0            \n"
						"vmov.s32    q12, #0            \n"
						"vmov.s32    q13, #0            \n"
						"vmov.s32    q14, #0            \n"
						"vmov.s32    q15, #0            \n"

						"lsr         r4, %3, #3         \n"// r4 = nn = L >> 3
						"cmp         r4, #0             \n"
						"beq         1f                 \n"

						"0:                             \n"// for(; nn != 0; nn--)
						"pld         [%1, #128]         \n"
						"vld1.s8     {d8-d11}, [%1]!    \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q7, d11            \n"// a30-a37
						"vmovl.s8    q6, d10            \n"// a20-a27                    
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"pld         [%2, #128]         \n"
						"vld1.s8     {d0-d3}, [%2]!     \n"// kptr k00-k30,k01-k31, k02-k32,k03-k33, k04-k34,k05-k35, k06-k36,k07-k37    k(outch)(inch)
						"vmovl.s8    q3, d3             \n"// k06-k36,k07-k37
						"vmovl.s8    q2, d2             \n"// k04-k34,k05-k35
						"vmovl.s8    q1, d1             \n"// k02-k32,k03-k33
						"vmovl.s8    q0, d0             \n"// k00-k30,k01-k31

						"vmlal.s16   q8, d8, d0[0]      \n"// sum0 = (a00-a07) * k00
						"vmlal.s16   q9, d9, d0[0]      \n"
						"vmlal.s16   q10, d8, d0[1]     \n"// sum1 = (a00-a07) * k10
						"vmlal.s16   q11, d9, d0[1]     \n"
						"vmlal.s16   q12, d8, d0[2]     \n"// sum2 = (a00-a07) * k20
						"vmlal.s16   q13, d9, d0[2]     \n"
						"vmlal.s16   q14, d8, d0[3]     \n"// sum3 = (a00-a07) * k30
						"vmlal.s16   q15, d9, d0[3]     \n"

						"vmlal.s16   q8, d10, d1[0]     \n"// sum0 += (a10-a17) * k01
						"vmlal.s16   q9, d11, d1[0]     \n"
						"vmlal.s16   q10, d10, d1[1]    \n"// sum1 += (a10-a17) * k11
						"vmlal.s16   q11, d11, d1[1]    \n"
						"vmlal.s16   q12, d10, d1[2]    \n"// sum2 += (a10-a17) * k21
						"vmlal.s16   q13, d11, d1[2]    \n"
						"vmlal.s16   q14, d10, d1[3]    \n"// sum3 += (a10-a17) * k31
						"vmlal.s16   q15, d11, d1[3]    \n"

						"pld         [%1, #128]         \n"
						"vld1.s8     {d8-d9}, [%1]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q5, d9             \n"// a10-a17
						"vmovl.s8    q4, d8             \n"// a00-a07

						"vmlal.s16   q8, d12, d2[0]     \n"// sum0 += (a20-a27) * k02
						"vmlal.s16   q9, d13, d2[0]     \n"
						"vmlal.s16   q10, d12, d2[1]    \n"// sum1 += (a20-a27) * k12
						"vmlal.s16   q11, d13, d2[1]    \n"
						"vmlal.s16   q12, d12, d2[2]    \n"// sum2 += (a20-a27) * k22
						"vmlal.s16   q13, d13, d2[2]    \n"
						"vmlal.s16   q14, d12, d2[3]    \n"// sum3 += (a20-a27) * k32
						"vmlal.s16   q15, d13, d2[3]    \n"

						"vmlal.s16   q8, d14, d3[0]     \n"// sum0 += (a30-a37) * k03
						"vmlal.s16   q9, d15, d3[0]     \n"
						"vmlal.s16   q10, d14, d3[1]    \n"// sum1 += (a30-a37) * k13
						"vmlal.s16   q11, d15, d3[1]    \n"
						"vmlal.s16   q12, d14, d3[2]    \n"// sum2 += (a30-a37) * k23
						"vmlal.s16   q13, d15, d3[2]    \n"
						"vmlal.s16   q14, d14, d3[3]    \n"// sum3 += (a30-a37) * k33
						"vmlal.s16   q15, d15, d3[3]    \n"

						"pld         [%1, #128]         \n"
						"vld1.s8     {d0-d1}, [%1]!     \n"// tmpr a00-a07,a10-a17,a20-a27,a30-a37    a(inch)(data)
						"vmovl.s8    q1, d1             \n"// a10-a17
						"vmovl.s8    q0, d0             \n"// a00-a07

						"vmlal.s16   q8, d8, d4[0]      \n"// sum0 += (a40-a47) * k04
						"vmlal.s16   q9, d9, d4[0]      \n"
						"vmlal.s16   q10, d8, d4[1]     \n"// sum1 += (a40-a47) * k14
						"vmlal.s16   q11, d9, d4[1]     \n"
						"vmlal.s16   q12, d8, d4[2]     \n"// sum2 += (a40-a47) * k24
						"vmlal.s16   q13, d9, d4[2]     \n"
						"vmlal.s16   q14, d8, d4[3]     \n"// sum3 += (a40-a47) * k34
						"vmlal.s16   q15, d9, d4[3]     \n"

						"vmlal.s16   q8, d10, d5[0]     \n"// sum0 += (a50-a57) * k05
						"vmlal.s16   q9, d11, d5[0]     \n"
						"vmlal.s16   q10, d10, d5[1]    \n"// sum1 += (a50-a57) * k15
						"vmlal.s16   q11, d11, d5[1]    \n"
						"vmlal.s16   q12, d10, d5[2]    \n"// sum2 += (a50-a57) * k25
						"vmlal.s16   q13, d11, d5[2]    \n"
						"vmlal.s16   q14, d10, d5[3]    \n"// sum3 += (a50-a57) * k35
						"vmlal.s16   q15, d11, d5[3]    \n"

						"vmlal.s16   q8, d0, d6[0]      \n"// sum0 += (a60-a67) * k06
						"vmlal.s16   q9, d1, d6[0]      \n"
						"vmlal.s16   q10, d0, d6[1]     \n"// sum1 += (a60-a67) * k16
						"vmlal.s16   q11, d1, d6[1]     \n"
						"vmlal.s16   q12, d0, d6[2]     \n"// sum2 += (a60-a67) * k26
						"vmlal.s16   q13, d1, d6[2]     \n"
						"vmlal.s16   q14, d0, d6[3]     \n"// sum3 += (a60-a67) * k36
						"vmlal.s16   q15, d1, d6[3]     \n"

						"vmlal.s16   q8, d2, d7[0]      \n"// sum0 += (a70-a77) * k07
						"vmlal.s16   q9, d3, d7[0]      \n"
						"vmlal.s16   q10, d2, d7[1]     \n"// sum1 += (a70-a77) * k17
						"vmlal.s16   q11, d3, d7[1]     \n"
						"vmlal.s16   q12, d2, d7[2]     \n"// sum2 += (a70-a77) * k27
						"vmlal.s16   q13, d3, d7[2]     \n"
						"vmlal.s16   q14, d2, d7[3]     \n"// sum3 += (a70-a77) * k37
						"vmlal.s16   q15, d3, d7[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         0b                 \n"// end for

						"1:                             \n"
						// remain loop
						"and         r4, %3, #7         \n"// r4 = remain = inch & 7
						"cmp         r4, #0             \n"
						"beq         3f                 \n"

						"2:                             \n"// for(; remain != 0; remain--)
						"vld1.s8     {d2}, [%1]!        \n"// tmpr a00-a70    a(inch)(data)
						"vld1.s8     {d0}, [%2]         \n"// kptr k00-k30    k(outch)(inch)
						"vmovl.s8    q1, d2             \n"
						"vmovl.s8    q0, d0             \n"
						"add         %2, #4             \n"

						"vmlal.s16   q8, d2, d0[0]      \n"// sum0 += (a00-a70) * k00
						"vmlal.s16   q9, d3, d0[0]      \n"
						"vmlal.s16   q10, d2, d0[1]     \n"// sum1 += (a00-a70) * k10
						"vmlal.s16   q11, d3, d0[1]     \n"
						"vmlal.s16   q12, d2, d0[2]     \n"// sum2 += (a00-a70) * k20
						"vmlal.s16   q13, d3, d0[2]     \n"
						"vmlal.s16   q14, d2, d0[3]     \n"// sum3 += (a00-a70) * k30
						"vmlal.s16   q15, d3, d0[3]     \n"

						"subs        r4, r4, #1         \n"
						"bne         2b                 \n"

						"3:                             \n"// store the result to memory

						"vld1.f32    {d0-d1}, [%4]      \n"

						"vcvt.f32.s32 q8, q8            \n"
						"vcvt.f32.s32 q9, q9            \n"
						"vcvt.f32.s32 q10, q10          \n"
						"vcvt.f32.s32 q11, q11          \n"

						"vdup.32     q1, %5             \n"
						"vdup.32     q3, %6             \n"
						"vmov.32     q2, q1             \n"
						"vmov.32     q4, q3             \n"

						"vmla.f32    q1, q8, d0[0]      \n"
						"vmla.f32    q2, q9, d0[0]      \n"
						"vmla.f32    q3, q10, d0[1]     \n"
						"vmla.f32    q4, q11, d0[1]     \n"

						"vcvt.f32.s32 q12, q12          \n"
						"vcvt.f32.s32 q13, q13          \n"
						"vcvt.f32.s32 q14, q14          \n"
						"vcvt.f32.s32 q15, q15          \n"

						"vdup.32     q5, %7             \n"
						"vdup.32     q7, %8            \n"
						"vmov.32     q6, q5             \n"
						"vmov.32     q8, q7             \n"

						"vmla.f32    q5, q12, d1[0]    \n"
						"vmla.f32    q6, q13, d1[0]    \n"
						"vmla.f32    q7, q14, d1[1]    \n"
						"vmla.f32    q8, q15, d1[1]    \n"

						"vld1.f32   {d0-d1}, [%9]		\n"
						"veor		q15, q15			\n"

						"vcle.f32	q9, q1, q15			\n"
						"vcle.f32	q10, q2, q15		\n"
						"vmul.f32	q11, q1, d0[0]		\n"
						"vmul.f32	q12, q2, d0[0]		\n"
						"vbit.f32	q1, q11, q9			\n"
						"vbit.f32	q2, q12, q10		\n"

						"vcle.f32	q9, q3, q15			\n"
						"vcle.f32	q10, q4, q15		\n"
						"vmul.f32	q11, q3, d0[1]		\n"
						"vmul.f32	q12, q4, d0[1]		\n"
						"vbit.f32	q3, q11, q9			\n"
						"vbit.f32	q4, q12, q10		\n"

						"vcle.f32	q9, q5, q15			\n"
						"vcle.f32	q10, q6, q15		\n"
						"vmul.f32	q11, q5, d1[0]		\n"
						"vmul.f32	q12, q6, d1[0]		\n"
						"vbit.f32	q5, q11, q9			\n"
						"vbit.f32	q6, q12, q10		\n"

						"vcle.f32	q9, q7, q15			\n"
						"vcle.f32	q10, q8, q15		\n"
						"vmul.f32	q11, q7, d1[1]		\n"
						"vmul.f32	q12, q8, d1[1]		\n"
						"vbit.f32	q7, q11, q9			\n"
						"vbit.f32	q8, q12, q10		\n"

						"vcvtr.s32.f32 s0, s4			\n"
						"vcvtr.s32.f32 s1, s5			\n"
						"vcvtr.s32.f32 s2, s6			\n"
						"vcvtr.s32.f32 s3, s7			\n"

						"vcvtr.s32.f32 s4, s8			\n"
						"vcvtr.s32.f32 s5, s9			\n"
						"vcvtr.s32.f32 s6, s10			\n"
						"vcvtr.s32.f32 s7, s11			\n"

						"vcvtr.s32.f32 s8, s12			\n"
						"vcvtr.s32.f32 s9, s13			\n"
						"vcvtr.s32.f32 s10, s14			\n"
						"vcvtr.s32.f32 s11, s15			\n"

						"vcvtr.s32.f32 s12, s16			\n"
						"vcvtr.s32.f32 s13, s17			\n"
						"vcvtr.s32.f32 s14, s18			\n"
						"vcvtr.s32.f32 s15, s19			\n"

						"vcvtr.s32.f32 s16, s20			\n"
						"vcvtr.s32.f32 s17, s21			\n"
						"vcvtr.s32.f32 s18, s22			\n"
						"vcvtr.s32.f32 s19, s23			\n"

						"vcvtr.s32.f32 s20, s24			\n"
						"vcvtr.s32.f32 s21, s25			\n"
						"vcvtr.s32.f32 s22, s26			\n"
						"vcvtr.s32.f32 s23, s27			\n"

						"vcvtr.s32.f32 s24, s28			\n"
						"vcvtr.s32.f32 s25, s29			\n"
						"vcvtr.s32.f32 s26, s30			\n"
						"vcvtr.s32.f32 s27, s31			\n"

						"vmov.f32	q7, q8				\n"

						"vcvtr.s32.f32 s28, s28			\n"
						"vcvtr.s32.f32 s29, s29			\n"
						"vcvtr.s32.f32 s30, s30			\n"
						"vcvtr.s32.f32 s31, s31			\n"

						"vqmovn.s32 d0, q0              \n"
						"vqmovn.s32 d1, q1              \n"
						"vqmovn.s32 d2, q2              \n"
						"vqmovn.s32 d3, q3              \n"
						"vqmovn.s32 d4, q4              \n"
						"vqmovn.s32 d5, q5              \n"
						"vqmovn.s32 d6, q6              \n"
						"vqmovn.s32 d7, q7              \n"

						"vzip.16	q0, q1				\n"
						"vzip.16	q2, q3				\n"
						"vzip.32	q0, q2				\n"
						"vzip.32	q1, q3				\n"

						"vqmovn.s16 d0, q0              \n"
						"vqmovn.s16 d1, q2              \n"
						"vqmovn.s16 d2, q1              \n"
						"vqmovn.s16 d3, q3              \n"

						"mov		r4, #8				\n"

						"vst1.s32	{d0[0]}, [%0], r4	\n"
						"vst1.s32	{d0[1]}, [%0], r4	\n"
						"vst1.s32	{d1[0]}, [%0], r4	\n"
						"vst1.s32	{d1[1]}, [%0], r4	\n"
						"vst1.s32	{d2[0]}, [%0], r4	\n"
						"vst1.s32	{d2[1]}, [%0], r4	\n"
						"vst1.s32	{d3[0]}, [%0], r4	\n"
						"vst1.s32	{d3[1]}, [%0], r4	\n"

						: "+r"(output0), // %0
						"+r"(vb),      // %1
						"+r"(va)       // %2
						: "r"(L),        // %3  
						"r"(mem_scale + i),   // %4
						"r"(mem_bias[i]),     // %5
						"r"(mem_bias[i + 1]),   // %6
						"r"(mem_bias[i + 2]),   // %7
						"r"(mem_bias[i + 3]),   // %8
						"r"(mem_prelu + i)			// %9
						: "cc", "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
#else
					int sum0[8] = { 0 };
					int sum1[8] = { 0 };
					int sum2[8] = { 0 };
					int sum3[8] = { 0 };

					int k = 0;
					for (; k + 7 < L; k = k + 8)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 8];
							sum1[n] += (int)va[1] * vb[n + 8];
							sum2[n] += (int)va[2] * vb[n + 8];
							sum3[n] += (int)va[3] * vb[n + 8];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 16];
							sum1[n] += (int)va[1] * vb[n + 16];
							sum2[n] += (int)va[2] * vb[n + 16];
							sum3[n] += (int)va[3] * vb[n + 16];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 24];
							sum1[n] += (int)va[1] * vb[n + 24];
							sum2[n] += (int)va[2] * vb[n + 24];
							sum3[n] += (int)va[3] * vb[n + 24];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 32];
							sum1[n] += (int)va[1] * vb[n + 32];
							sum2[n] += (int)va[2] * vb[n + 32];
							sum3[n] += (int)va[3] * vb[n + 32];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 40];
							sum1[n] += (int)va[1] * vb[n + 40];
							sum2[n] += (int)va[2] * vb[n + 40];
							sum3[n] += (int)va[3] * vb[n + 40];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 48];
							sum1[n] += (int)va[1] * vb[n + 48];
							sum2[n] += (int)va[2] * vb[n + 48];
							sum3[n] += (int)va[3] * vb[n + 48];
							va += 4;

							sum0[n] += (int)va[0] * vb[n + 56];
							sum1[n] += (int)va[1] * vb[n + 56];
							sum2[n] += (int)va[2] * vb[n + 56];
							sum3[n] += (int)va[3] * vb[n + 56];
							va -= 28;
						}

						va += 32;
						vb += 64;
					}

					for (; k < L; k++)
					{
						for (int n = 0; n < 8; n++)
						{
							sum0[n] += (int)va[0] * vb[n];
							sum1[n] += (int)va[1] * vb[n];
							sum2[n] += (int)va[2] * vb[n];
							sum3[n] += (int)va[3] * vb[n];
						}

						va += 4;
						vb += 8;
					}

					for (int n = 0; n < 8; n++)
					{
						float rVal;
						rVal = (sum0[n] * mem_scale[i + 0] + mem_bias[i + 0]); if (rVal < 0.0f) rVal *= mem_prelu[i + 0]; output0[0] = float2int8(rVal);
						rVal = (sum1[n] * mem_scale[i + 1] + mem_bias[i + 1]); if (rVal < 0.0f) rVal *= mem_prelu[i + 1]; output0[1] = float2int8(rVal);
						rVal = (sum2[n] * mem_scale[i + 2] + mem_bias[i + 2]); if (rVal < 0.0f) rVal *= mem_prelu[i + 2]; output0[2] = float2int8(rVal);
						rVal = (sum3[n] * mem_scale[i + 3] + mem_bias[i + 3]); if (rVal < 0.0f) rVal *= mem_prelu[i + 3]; output0[3] = float2int8(rVal);
						output0 += 8;
					}
#endif // __ARM_NEON
				}
			}
		}
	}

	// static void convdw3x3s1_pack8_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& kernel, const Option& opt)
	void convd_k3s1_nn_r_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias)
	{
		int w = in_blob->w; // bottom_blob.w;
		int h = in_blob->h;
		int ch = in_blob->c;
		int cstep = in_blob->cstep;
		signed char* in_mem = (signed char*)in_blob->mem;

		int outw = w - 2; // top_blob.w;
		int outh = h - 2; // top_blob.h;
		int out_cstep = get_blob_size(outw * outh * 8);
		signed char* out_mem = (signed char*)out_blob->mem;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->c = ch;
		out_blob->cstep = out_cstep;
		out_blob->packp = 8;
		out_blob->elemc = 1;

		const int group = ch / 8; // bottom_blob.c;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < group; g++)
		{
			signed char* out = out_mem + g * out_cstep;		//  Mat out = top_blob.channel(g);

			const signed char* k0 = weight + g * 72;		// const signed char* k0 = kernel.row<const signed char>(g);

			signed char* outptr0 = out;						// int* outptr0 = out.row<int>(0);
			signed char* outptr1 = out + 8 * outw;			// int* outptr1 = out.row<int>(1);

			signed char* img0 = in_mem + g * cstep;			// const Mat img0 = bottom_blob.channel(g);

			const signed char* r0 = img0 + 0 * w;			// const signed char* r0 = img0.row<const signed char>(0);
			const signed char* r1 = img0 + 8 * w;			// const signed char* r1 = img0.row<const signed char>(1);
			const signed char* r2 = img0 + 16 * w;			// const signed char* r2 = img0.row<const signed char>(2);
			const signed char* r3 = img0 + 24 * w;			// const signed char* r3 = img0.row<const signed char>(3);

#if __ARM_NEON
			int8x8_t _k00 = vld1_s8(k0);
			int8x8_t _k01 = vld1_s8(k0 + 8);
			int8x8_t _k02 = vld1_s8(k0 + 16);
			int8x8_t _k10 = vld1_s8(k0 + 24);
			int8x8_t _k11 = vld1_s8(k0 + 32);
			int8x8_t _k12 = vld1_s8(k0 + 40);
			int8x8_t _k20 = vld1_s8(k0 + 48);
			int8x8_t _k21 = vld1_s8(k0 + 56);
			int8x8_t _k22 = vld1_s8(k0 + 64);
#endif
			int i = 0;
			for (; i + 1 < outh; i += 2)
			{
				int j = 0;
				for (; j + 1 < outw; j += 2)
				{
#if __ARM_NEON
					asm volatile(
						"mov			r4, #16				\n"

						"pld			[%2, #128]          \n"
						"vld1.s8		{d10-d13}, [%2 :128], r4	\n"
						"pld			[%3, #128]          \n"
						"vld1.s8		{d14-d17}, [%3 :128], r4	\n"

						"vmull.s8		q3, d10, %P8		\n"
						"vmull.s8		q4, d11, %P8		\n"
						"vmlal.s8		q3, d11, %P9		\n"
						"vmlal.s8		q4, d12, %P9		\n"

						"vmull.s8		q9,  d12, %P10		\n"
						"vmull.s8		q10, d13, %P10		\n"
						"vmlal.s8		q9,  d14, %P11		\n"
						"vmlal.s8		q10, d15, %P11		\n"

						"vaddl.s16		q0, d6, d18			\n"
						"vaddl.s16		q1, d7, d19			\n"
						"vaddl.s16		q2, d8, d20			\n"
						"vaddl.s16		q3, d9, d21			\n"

						"pld			[%4, #128]          \n"
						"vld1.s8		{d18-d21}, [%4 :128], r4	\n"

						"vmull.s8		q5, d15, %P12		\n"
						"vmull.s8		q6, d16, %P12		\n"
						"vmlal.s8		q5, d16, %P13		\n"
						"vmlal.s8		q6, d17, %P13		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d18, %P14		\n"
						"vmull.s8		q6, d19, %P14		\n"
						"vmlal.s8		q5, d19, %P15		\n"
						"vmlal.s8		q6, d20, %P15		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d20, %P16		\n"
						"vmull.s8		q6, d21, %P16		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.f32		{d8-d11}, [%6 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"
						"veor			q6, q6				\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"vcvt.s32.f32	q0, q0				\n"
						"vcvt.s32.f32	q1, q1				\n"
						"vcvt.s32.f32	q2, q2				\n"
						"vcvt.s32.f32	q3, q3				\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vmax.s8		q0, q0, q6			\n"
						"vst1.s8		{d0-d1}, [%0 :128]!		\n"

						"vmull.s8		q3, d14, %P8		\n"
						"vmull.s8		q4, d15, %P8		\n"
						"vmlal.s8		q3, d15, %P9		\n"
						"vmlal.s8		q4, d16, %P9		\n"

						"vmull.s8		q5, d16, %P10		\n"
						"vmull.s8		q6, d17, %P10		\n"
						"vmlal.s8		q5, d18, %P11		\n"
						"vmlal.s8		q6, d19, %P11		\n"

						"vaddl.s16		q0, d6, d10			\n"
						"vaddl.s16		q1, d7, d11			\n"
						"vaddl.s16		q2, d8, d12			\n"
						"vaddl.s16		q3, d9, d13			\n"

						"vld1.s8		{d14-d17}, [%5 :128], r4	\n"

						"vmull.s8		q4, d19, %P12		\n"
						"vmull.s8		q5, d20, %P12		\n"
						"vmlal.s8		q4, d20, %P13		\n"
						"vmlal.s8		q5, d21, %P13		\n"

						"vaddw.s16		q0, q0, d8			\n"
						"vaddw.s16		q1, q1, d9			\n"
						"vaddw.s16		q2, q2, d10			\n"
						"vaddw.s16		q3, q3, d11			\n"

						"vmull.s8		q4, d14, %P14		\n"
						"vmull.s8		q5, d15, %P14		\n"
						"vmlal.s8		q4, d15, %P15		\n"
						"vmlal.s8		q5, d16, %P15		\n"

						"vaddw.s16		q0, q0, d8			\n"
						"vaddw.s16		q1, q1, d9			\n"
						"vaddw.s16		q2, q2, d10			\n"
						"vaddw.s16		q3, q3, d11			\n"

						"vmull.s8		q4, d16, %P16		\n"
						"vmull.s8		q5, d17, %P16		\n"

						"vaddw.s16		q0, q0, d8			\n"
						"vaddw.s16		q1, q1, d9			\n"
						"vaddw.s16		q2, q2, d10			\n"
						"vaddw.s16		q3, q3, d11			\n"

						"vld1.f32		{d8-d11}, [%6 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"
						"veor			q6, q6				\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"vcvt.s32.f32	q0, q0				\n"
						"vcvt.s32.f32	q1, q1				\n"
						"vcvt.s32.f32	q2, q2				\n"
						"vcvt.s32.f32	q3, q3				\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vmax.s8		q0, q0, q6			\n"
						"vst1.s8		{d0-d1}, [%1 :128]!		\n"

						:
					"+r"(outptr0),			// %0
						"+r"(outptr1),			// %1
						"+r"(r0),				// %2
						"+r"(r1),				// %3
						"+r"(r2),				// %4
						"+r"(r3)				// %5
						:
						"r"(mem_scale + g * 8),	// %6
						"r"(mem_bias + g * 8),	// %7
						"w"(_k00),				// %8
						"w"(_k01),				// %9
						"w"(_k02),				// %10
						"w"(_k10),				// %11
						"w"(_k11),				// %12
						"w"(_k12),				// %13
						"w"(_k20),				// %14
						"w"(_k21),				// %15
						"w"(_k22)				// %16
						: "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10");
#else
					for (int u = 0; u < 8; u++)
					{
						int sum0 = r0[u] * k0[u + 0] + r0[u + 8] * k0[u + 8] + r0[u + 16] * k0[u + 16] +
							r1[u] * k0[u + 24] + r1[u + 8] * k0[u + 32] + r1[u + 16] * k0[u + 40] +
							r2[u] * k0[u + 48] + r2[u + 8] * k0[u + 56] + r2[u + 16] * k0[u + 64];
						int sum1 = r0[u + 8] * k0[u + 0] + r0[u + 16] * k0[u + 8] + r0[u + 24] * k0[u + 16] +
							r1[u + 8] * k0[u + 24] + r1[u + 16] * k0[u + 32] + r1[u + 24] * k0[u + 40] +
							r2[u + 8] * k0[u + 48] + r2[u + 16] * k0[u + 56] + r2[u + 24] * k0[u + 64];
						int sum2 = r1[u] * k0[u + 0] + r1[u + 8] * k0[u + 8] + r1[u + 16] * k0[u + 16] +
							r2[u] * k0[u + 24] + r2[u + 8] * k0[u + 32] + r2[u + 16] * k0[u + 40] +
							r3[u] * k0[u + 48] + r3[u + 8] * k0[u + 56] + r3[u + 16] * k0[u + 64];
						int sum3 = r1[u + 8] * k0[u + 0] + r1[u + 16] * k0[u + 8] + r1[u + 24] * k0[u + 16] +
							r2[u + 8] * k0[u + 24] + r2[u + 16] * k0[u + 32] + r2[u + 24] * k0[u + 40] +
							r3[u + 8] * k0[u + 48] + r3[u + 16] * k0[u + 56] + r3[u + 24] * k0[u + 64];

						int val;
						val = (int)(sum0 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[u] = val;
						val = (int)(sum1 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[u + 8] = val;
						val = (int)(sum2 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[u] = val;
						val = (int)(sum3 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[u + 8] = val;
					}
					r0 += 16;
					r1 += 16;
					r2 += 16;
					r3 += 16;
					outptr0 += 16;
					outptr1 += 16;
#endif
				}

				r0 += 2 * 8 + w * 8;
				r1 += 2 * 8 + w * 8;
				r2 += 2 * 8 + w * 8;
				r3 += 2 * 8 + w * 8;

				outptr0 += outw * 8;
				outptr1 += outw * 8;
			}
		}
	}

	// static void convdw3x3s1_pack8_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& kernel, const Option& opt)
	void convd_k3s1_nn_p_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu)
	{
		int w = in_blob->w; // bottom_blob.w;
		int h = in_blob->h;
		int ch = in_blob->c;
		int cstep = in_blob->cstep;
		signed char* in_mem = (signed char*)in_blob->mem;

		int outw = w - 2; // top_blob.w;
		int outh = h - 2; // top_blob.h;
		int out_cstep = get_blob_size(outw * outh * 8);
		signed char* out_mem = (signed char*)out_blob->mem;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->c = ch;
		out_blob->cstep = out_cstep;
		out_blob->packp = 8;
		out_blob->elemc = 1;

		const int group = ch / 8; // bottom_blob.c;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < group; g++)
		{
			signed char* out = out_mem + g * out_cstep;		//  Mat out = top_blob.channel(g);

			const signed char* k0 = weight + g * 72;		// const signed char* k0 = kernel.row<const signed char>(g);

			signed char* outptr0 = out;						// int* outptr0 = out.row<int>(0);
			signed char* outptr1 = out + 8 * outw;			// int* outptr1 = out.row<int>(1);

			signed char* img0 = in_mem + g * cstep;			// const Mat img0 = bottom_blob.channel(g);

			const signed char* r0 = img0 + 0 * w;			// const signed char* r0 = img0.row<const signed char>(0);
			const signed char* r1 = img0 + 8 * w;			// const signed char* r1 = img0.row<const signed char>(1);
			const signed char* r2 = img0 + 16 * w;			// const signed char* r2 = img0.row<const signed char>(2);
			const signed char* r3 = img0 + 24 * w;			// const signed char* r3 = img0.row<const signed char>(3);

#if __ARM_NEON
			int8x8_t _k00 = vld1_s8(k0);
			int8x8_t _k01 = vld1_s8(k0 + 8);
			int8x8_t _k02 = vld1_s8(k0 + 16);
			int8x8_t _k10 = vld1_s8(k0 + 24);
			int8x8_t _k11 = vld1_s8(k0 + 32);
			int8x8_t _k12 = vld1_s8(k0 + 40);
			int8x8_t _k20 = vld1_s8(k0 + 48);
			int8x8_t _k21 = vld1_s8(k0 + 56);
			int8x8_t _k22 = vld1_s8(k0 + 64);
#endif
			int i = 0;
			for (; i + 1 < outh; i += 2)
			{
				int j = 0;
				for (; j + 1 < outw; j += 2)
				{
#if __ARM_NEON
					asm volatile(
						"mov			r4, #16				\n"

						"pld			[%2, #128]          \n"
						"vld1.s8		{d10-d13}, [%2 :128], r4	\n"
						"pld			[%3, #128]          \n"
						"vld1.s8		{d14-d17}, [%3 :128], r4	\n"

						"vmull.s8		q3, d10, %P8		\n"
						"vmull.s8		q4, d11, %P8		\n"
						"vmlal.s8		q3, d11, %P9		\n"
						"vmlal.s8		q4, d12, %P9		\n"

						"vmull.s8		q9,  d12, %P10		\n"
						"vmull.s8		q10, d13, %P10		\n"
						"vmlal.s8		q9,  d14, %P11		\n"
						"vmlal.s8		q10, d15, %P11		\n"

						"vaddl.s16		q0, d6, d18			\n"
						"vaddl.s16		q1, d7, d19			\n"
						"vaddl.s16		q2, d8, d20			\n"
						"vaddl.s16		q3, d9, d21			\n"

						"pld			[%4, #128]          \n"
						"vld1.s8		{d18-d21}, [%4 :128], r4	\n"

						"vmull.s8		q5, d15, %P12		\n"
						"vmull.s8		q6, d16, %P12		\n"
						"vmlal.s8		q5, d16, %P13		\n"
						"vmlal.s8		q6, d17, %P13		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d18, %P14		\n"
						"vmull.s8		q6, d19, %P14		\n"
						"vmlal.s8		q5, d19, %P15		\n"
						"vmlal.s8		q6, d20, %P15		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d20, %P16		\n"
						"vmull.s8		q6, d21, %P16		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.f32		{d8-d11}, [%6 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"mov			r5, %17				\n"
						"vld1.f32		{d8-d9}, [r5 :128]!  \n"

						"veor			q6, q6				\n"
						"vmul.f32		q5, q0, q4			\n"
						"vcle.f32		q6, q0, q6			\n"
						"vbit.f32		q0, q5, q6			\n"

						"veor			q6, q6				\n"
						"vmul.f32		q5, q2, q4			\n"
						"vcle.f32		q6, q2, q6			\n"
						"vbit.f32		q2, q5, q6			\n"

						"vld1.f32		{d8-d9}, [r5 :128]  \n"

						"veor			q6, q6				\n"
						"vmul.f32		q5, q1, q4			\n"
						"vcle.f32		q6, q1, q6			\n"
						"vbit.f32		q1, q5, q6			\n"

						"veor			q6, q6				\n"
						"vmul.f32		q5, q3, q4			\n"
						"vcle.f32		q6, q3, q6			\n"
						"vbit.f32		q3, q5, q6			\n"

						"vcvtr.s32.f32	s0, s0				\n"
						"vcvtr.s32.f32	s1, s1				\n"
						"vcvtr.s32.f32	s2, s2				\n"
						"vcvtr.s32.f32	s3, s3				\n"

						"vcvtr.s32.f32	s4, s4				\n"
						"vcvtr.s32.f32	s5, s5				\n"
						"vcvtr.s32.f32	s6, s6				\n"
						"vcvtr.s32.f32	s7, s7				\n"

						"vcvtr.s32.f32	s8, s8				\n"
						"vcvtr.s32.f32	s9, s9				\n"
						"vcvtr.s32.f32	s10, s10			\n"
						"vcvtr.s32.f32	s11, s11			\n"

						"vcvtr.s32.f32	s12, s12			\n"
						"vcvtr.s32.f32	s13, s13			\n"
						"vcvtr.s32.f32	s14, s14			\n"
						"vcvtr.s32.f32	s15, s15			\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vst1.s8		{d0-d1}, [%0 :128]!		\n"

						"vmull.s8		q3, d14, %P8		\n"
						"vmull.s8		q4, d15, %P8		\n"
						"vmlal.s8		q3, d15, %P9		\n"
						"vmlal.s8		q4, d16, %P9		\n"

						"vmull.s8		q5, d16, %P10		\n"
						"vmull.s8		q6, d17, %P10		\n"
						"vmlal.s8		q5, d18, %P11		\n"
						"vmlal.s8		q6, d19, %P11		\n"

						"vaddl.s16		q0, d6, d10			\n"
						"vaddl.s16		q1, d7, d11			\n"
						"vaddl.s16		q2, d8, d12			\n"
						"vaddl.s16		q3, d9, d13			\n"

						"vld1.s8		{d14-d17}, [%5 :128], r4	\n"

						"vmull.s8		q4, d19, %P12		\n"
						"vmull.s8		q5, d20, %P12		\n"
						"vmlal.s8		q4, d20, %P13		\n"
						"vmlal.s8		q5, d21, %P13		\n"

						"vaddw.s16		q0, q0, d8			\n"
						"vaddw.s16		q1, q1, d9			\n"
						"vaddw.s16		q2, q2, d10			\n"
						"vaddw.s16		q3, q3, d11			\n"

						"vmull.s8		q4, d14, %P14		\n"
						"vmull.s8		q5, d15, %P14		\n"
						"vmlal.s8		q4, d15, %P15		\n"
						"vmlal.s8		q5, d16, %P15		\n"

						"vaddw.s16		q0, q0, d8			\n"
						"vaddw.s16		q1, q1, d9			\n"
						"vaddw.s16		q2, q2, d10			\n"
						"vaddw.s16		q3, q3, d11			\n"

						"vmull.s8		q4, d16, %P16		\n"
						"vmull.s8		q5, d17, %P16		\n"

						"vaddw.s16		q0, q0, d8			\n"
						"vaddw.s16		q1, q1, d9			\n"
						"vaddw.s16		q2, q2, d10			\n"
						"vaddw.s16		q3, q3, d11			\n"

						"vld1.f32		{d8-d11}, [%6 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"
						"veor			q6, q6				\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%17 :128]  \n"

						"vcle.f32		q7, q0, q6			\n"
						"vcle.f32		q8, q1, q6			\n"
						"vmul.f32		q9, q0, q4			\n"
						"vmul.f32		q10, q1, q5			\n"
						"vbit.f32		q0, q9, q7			\n"
						"vbit.f32		q1, q10, q8			\n"

						"vcle.f32		q7, q2, q6			\n"
						"vcle.f32		q8, q3, q6			\n"
						"vmul.f32		q9, q2, q4			\n"
						"vmul.f32		q10, q3, q5			\n"
						"vbit.f32		q2, q9, q7			\n"
						"vbit.f32		q3, q10, q8			\n"

						"vcvtr.s32.f32	s0, s0				\n"
						"vcvtr.s32.f32	s1, s1				\n"
						"vcvtr.s32.f32	s2, s2				\n"
						"vcvtr.s32.f32	s3, s3				\n"

						"vcvtr.s32.f32	s4, s4				\n"
						"vcvtr.s32.f32	s5, s5				\n"
						"vcvtr.s32.f32	s6, s6				\n"
						"vcvtr.s32.f32	s7, s7				\n"

						"vcvtr.s32.f32	s8, s8				\n"
						"vcvtr.s32.f32	s9, s9				\n"
						"vcvtr.s32.f32	s10, s10			\n"
						"vcvtr.s32.f32	s11, s11			\n"

						"vcvtr.s32.f32	s12, s12			\n"
						"vcvtr.s32.f32	s13, s13			\n"
						"vcvtr.s32.f32	s14, s14			\n"
						"vcvtr.s32.f32	s15, s15			\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vst1.s8		{d0-d1}, [%1 :128]!		\n"

						:
					"+r"(outptr0),			// %0
						"+r"(outptr1),			// %1
						"+r"(r0),				// %2
						"+r"(r1),				// %3
						"+r"(r2),				// %4
						"+r"(r3)				// %5
						:
						"r"(mem_scale + g * 8),	// %6
						"r"(mem_bias + g * 8),	// %7
						"w"(_k00),				// %8
						"w"(_k01),				// %9
						"w"(_k02),				// %10
						"w"(_k10),				// %11
						"w"(_k11),				// %12
						"w"(_k12),				// %13
						"w"(_k20),				// %14
						"w"(_k21),				// %15
						"w"(_k22),				// %16
						"r"(mem_prelu + g * 8)			// %17
						: "memory", "r4", "r5", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10");
#else
					for (int u = 0; u < 8; u++)
					{
						int sum0 = r0[u] * k0[u + 0] + r0[u + 8] * k0[u + 8] + r0[u + 16] * k0[u + 16] +
							r1[u] * k0[u + 24] + r1[u + 8] * k0[u + 32] + r1[u + 16] * k0[u + 40] +
							r2[u] * k0[u + 48] + r2[u + 8] * k0[u + 56] + r2[u + 16] * k0[u + 64];
						int sum1 = r0[u + 8] * k0[u + 0] + r0[u + 16] * k0[u + 8] + r0[u + 24] * k0[u + 16] +
							r1[u + 8] * k0[u + 24] + r1[u + 16] * k0[u + 32] + r1[u + 24] * k0[u + 40] +
							r2[u + 8] * k0[u + 48] + r2[u + 16] * k0[u + 56] + r2[u + 24] * k0[u + 64];
						int sum2 = r1[u] * k0[u + 0] + r1[u + 8] * k0[u + 8] + r1[u + 16] * k0[u + 16] +
							r2[u] * k0[u + 24] + r2[u + 8] * k0[u + 32] + r2[u + 16] * k0[u + 40] +
							r3[u] * k0[u + 48] + r3[u + 8] * k0[u + 56] + r3[u + 16] * k0[u + 64];
						int sum3 = r1[u + 8] * k0[u + 0] + r1[u + 16] * k0[u + 8] + r1[u + 24] * k0[u + 16] +
							r2[u + 8] * k0[u + 24] + r2[u + 16] * k0[u + 32] + r2[u + 24] * k0[u + 40] +
							r3[u + 8] * k0[u + 48] + r3[u + 16] * k0[u + 56] + r3[u + 24] * k0[u + 64];

						float rVal;
						rVal = (sum0 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr0[u] = float2int8(rVal);
						rVal = (sum1 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr0[u + 8] = float2int8(rVal);
						rVal = (sum2 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr1[u] = float2int8(rVal);
						rVal = (sum3 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr1[u + 8] = float2int8(rVal);
					}
					r0 += 16;
					r1 += 16;
					r2 += 16;
					r3 += 16;
					outptr0 += 16;
					outptr1 += 16;
#endif
				}

				r0 += 2 * 8 + w * 8;
				r1 += 2 * 8 + w * 8;
				r2 += 2 * 8 + w * 8;
				r3 += 2 * 8 + w * 8;

				outptr0 += outw * 8;
				outptr1 += outw * 8;
			}
		}
	}


	// static void convdw3x3s2_pack8_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& kernel, const Option& opt)
	void convd_k3s2_nn_r_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias)
	{
		int w = in_blob->w; // bottom_blob.w;
		int h = in_blob->h;
		int ch = in_blob->c;
		int cstep = in_blob->cstep;
		signed char* in_mem = (signed char*)in_blob->mem;

		int outw = (w - 2) / 2; // top_blob.w;
		int outh = (h - 2) / 2; // top_blob.h;
		int out_cstep = get_blob_size(outw * outh * 8);
		signed char* out_mem = (signed char*)out_blob->mem;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->c = ch;
		out_blob->cstep = out_cstep;
		out_blob->packp = 8;
		out_blob->elemc = 1;

		const int group = ch / 8; // bottom_blob.c;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < group; g++)
		{
			signed char* out = out_mem + g * out_cstep;		//  Mat out = top_blob.channel(g);

			const signed char* k0 = weight + g * 72;		// const signed char* k0 = kernel.row<const signed char>(g);

			signed char* outptr0 = out;						// int* outptr0 = out.row<int>(0);
			signed char* outptr1 = out + 8 * outw;			// int* outptr1 = out.row<int>(1);

			signed char* img0 = in_mem + g * cstep;			// const Mat img0 = bottom_blob.channel(g);

			const signed char* r0 = img0 + 0 * w;			// const signed char* r0 = img0.row<const signed char>(0);
			const signed char* r1 = img0 + 8 * w;			// const signed char* r1 = img0.row<const signed char>(1);
			const signed char* r2 = img0 + 16 * w;			// const signed char* r2 = img0.row<const signed char>(2);
			const signed char* r3 = img0 + 24 * w;			// const signed char* r3 = img0.row<const signed char>(3);
			const signed char* r4 = img0 + 32 * w;			// const signed char* r4 = img0.row<const signed char>(4);

#if __ARM_NEON
			int8x8_t _k00 = vld1_s8(k0);
			int8x8_t _k01 = vld1_s8(k0 + 8);
			int8x8_t _k02 = vld1_s8(k0 + 16);
			int8x8_t _k10 = vld1_s8(k0 + 24);
			int8x8_t _k11 = vld1_s8(k0 + 32);
			int8x8_t _k12 = vld1_s8(k0 + 40);
			int8x8_t _k20 = vld1_s8(k0 + 48);
			int8x8_t _k21 = vld1_s8(k0 + 56);
			int8x8_t _k22 = vld1_s8(k0 + 64);
#endif
			int i = 0;
			for (; i + 1 < outh; i += 2)
			{
				int j = 0;
				for (; j + 1 < outw; j += 2)
				{
#if __ARM_NEON
					asm volatile(
						"mov			r4, #32				\n"

						"pld			[%2, #128]          \n"
						"vld1.s8		{d14-d17}, [%2 :128]!	\n"
						"vld1.s8		{d22}, [%2 :64]	\n"
						"pld			[%3, #128]          \n"
						"vld1.s8		{d18-d21}, [%3 :128]!	\n"

						"vmull.s8		q3, d14, %P9		\n"
						"vmull.s8		q4, d16, %P9		\n"
						"vmlal.s8		q3, d15, %P10		\n"
						"vmlal.s8		q4, d17, %P10		\n"

						"vmull.s8		q5, d16, %P11		\n"
						"vmull.s8		q6, d22, %P11		\n"
						"vmlal.s8		q5, d18, %P12		\n"
						"vmlal.s8		q6, d20, %P12		\n"

						"vaddl.s16		q0, d6, d10			\n"
						"vaddl.s16		q1, d7, d11			\n"
						"vaddl.s16		q2, d8, d12			\n"
						"vaddl.s16		q3, d9, d13			\n"

						"vld1.s8		{d22}, [%3 :64]	\n"
						"pld			[%4, #128]          \n"
						"vld1.s8		{d14-d17}, [%4 :128]!	\n"

						"vmull.s8		q5, d19, %P13		\n"
						"vmull.s8		q6, d21, %P13		\n"
						"vmlal.s8		q5, d20, %P14		\n"
						"vmlal.s8		q6, d22, %P14		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.s8		{d22}, [%4 :64]	\n"

						"vmull.s8		q5, d14, %P15		\n"
						"vmull.s8		q6, d16, %P15		\n"
						"vmlal.s8		q5, d15, %P16		\n"
						"vmlal.s8		q6, d17, %P16		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d16, %P17		\n"
						"vmull.s8		q6, d22, %P17		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%8 :128]	\n"
						"veor			q6, q6				\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"vcvt.s32.f32	q0, q0				\n"
						"vcvt.s32.f32	q1, q1				\n"
						"vcvt.s32.f32	q2, q2				\n"
						"vcvt.s32.f32	q3, q3				\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vmax.s8		q0, q0, q6			\n"
						"vst1.s8		{d0-d1}, [%0 :128]!		\n"

						"pld			[%5, #128]          \n"
						"vld1.s8		{d18-d21}, [%5 :128]!	\n"

						"vmull.s8		q3, d14, %P9		\n"
						"vmull.s8		q4, d16, %P9		\n"
						"vmlal.s8		q3, d15, %P10		\n"
						"vmlal.s8		q4, d17, %P10		\n"

						"vmull.s8		q5, d16, %P11		\n"
						"vmull.s8		q6, d22, %P11		\n"
						"vmlal.s8		q5, d18, %P12		\n"
						"vmlal.s8		q6, d20, %P12		\n"

						"vaddl.s16		q0, d6, d10			\n"
						"vaddl.s16		q1, d7, d11			\n"
						"vaddl.s16		q2, d8, d12			\n"
						"vaddl.s16		q3, d9, d13			\n"

						"vld1.s8		{d22}, [%5 :64]		\n"
						"pld			[%6, #128]          \n"
						"vld1.s8		{d14-d17}, [%6 :128]!	\n"

						"vmull.s8		q5, d19, %P13		\n"
						"vmull.s8		q6, d21, %P13		\n"
						"vmlal.s8		q5, d20, %P14		\n"
						"vmlal.s8		q6, d22, %P14		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.s8		{d22}, [%6 :64]		\n"

						"vmull.s8		q5, d14, %P15		\n"
						"vmull.s8		q6, d16, %P15		\n"
						"vmlal.s8		q5, d15, %P16		\n"
						"vmlal.s8		q6, d17, %P16		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d16, %P17		\n"
						"vmull.s8		q6, d22, %P17		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%8 :128]	\n"
						"veor			q6, q6				\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"vcvt.s32.f32	q0, q0				\n"
						"vcvt.s32.f32	q1, q1				\n"
						"vcvt.s32.f32	q2, q2				\n"
						"vcvt.s32.f32	q3, q3				\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vmax.s8		q0, q0, q6			\n"
						"vst1.s8		{d0-d1}, [%1 :128]!		\n"
						:
					"+r"(outptr0),			// %0
						"+r"(outptr1),			// %1
						"+r"(r0),				// %2
						"+r"(r1),				// %3
						"+r"(r2),				// %4
						"+r"(r3),				// %5
						"+r"(r4)				// %6
						:
						"r"(mem_scale + g * 8),	// %7
						"r"(mem_bias + g * 8),	// %8
						"w"(_k00),				// %9
						"w"(_k01),				// %10
						"w"(_k02),				// %11
						"w"(_k10),				// %12
						"w"(_k11),				// %13
						"w"(_k12),				// %14
						"w"(_k20),				// %15
						"w"(_k21),				// %16
						"w"(_k22)				// %17
						: "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "d22");
#else
					for (int u = 0; u < 8; u++)
					{
						int sum0 = r0[u] * k0[u + 0] + r0[u + 8] * k0[u + 8] + r0[u + 16] * k0[u + 16] +
							r1[u] * k0[u + 24] + r1[u + 8] * k0[u + 32] + r1[u + 16] * k0[u + 40] +
							r2[u] * k0[u + 48] + r2[u + 8] * k0[u + 56] + r2[u + 16] * k0[u + 64];
						int sum1 = r0[u + 16] * k0[u + 0] + r0[u + 24] * k0[u + 8] + r0[u + 32] * k0[u + 16] +
							r1[u + 16] * k0[u + 24] + r1[u + 24] * k0[u + 32] + r1[u + 32] * k0[u + 40] +
							r2[u + 16] * k0[u + 48] + r2[u + 24] * k0[u + 56] + r2[u + 32] * k0[u + 64];
						int sum2 = r2[u] * k0[u + 0] + r2[u + 8] * k0[u + 8] + r2[u + 16] * k0[u + 16] +
							r3[u] * k0[u + 24] + r3[u + 8] * k0[u + 32] + r3[u + 16] * k0[u + 40] +
							r4[u] * k0[u + 48] + r4[u + 8] * k0[u + 56] + r4[u + 16] * k0[u + 64];
						int sum3 = r2[u + 16] * k0[u + 0] + r2[u + 24] * k0[u + 8] + r2[u + 32] * k0[u + 16] +
							r3[u + 16] * k0[u + 24] + r3[u + 24] * k0[u + 32] + r3[u + 32] * k0[u + 40] +
							r4[u + 16] * k0[u + 48] + r4[u + 24] * k0[u + 56] + r4[u + 32] * k0[u + 64];

						int val;
						val = (int)(sum0 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[u] = val;
						val = (int)(sum1 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr0[u + 8] = val;
						val = (int)(sum2 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[u] = val;
						val = (int)(sum3 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (val < 0) val = 0; if (val > 127) val = 127; outptr1[u + 8] = val;
					}
					r0 += 32;
					r1 += 32;
					r2 += 32;
					r3 += 32;
					r4 += 32;
					outptr0 += 16;
					outptr1 += 16;
#endif
				}

				r0 += (2 * w - outw) * 16;
				r1 += (2 * w - outw) * 16;
				r2 += (2 * w - outw) * 16;
				r3 += (2 * w - outw) * 16;
				r4 += (2 * w - outw) * 16;

				outptr0 += outw * 8;
				outptr1 += outw * 8;
			}
		}
	}

	// static void convdw3x3s2_pack8_int8_neon(const Mat& bottom_blob, Mat& top_blob, const Mat& kernel, const Option& opt)
	void convd_k3s2_nn_p_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias, const float* mem_prelu)
	{
		int w = in_blob->w; // bottom_blob.w;
		int h = in_blob->h;
		int ch = in_blob->c;
		int cstep = in_blob->cstep;
		signed char* in_mem = (signed char*)in_blob->mem;

		int outw = (w - 2) / 2; // top_blob.w;
		int outh = (h - 2) / 2; // top_blob.h;
		int out_cstep = get_blob_size(outw * outh * 8);
		signed char* out_mem = (signed char*)out_blob->mem;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->c = ch;
		out_blob->cstep = out_cstep;
		out_blob->packp = 8;
		out_blob->elemc = 1;

		const int group = ch / 8; // bottom_blob.c;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < group; g++)
		{
			signed char* out = out_mem + g * out_cstep;		//  Mat out = top_blob.channel(g);

			const signed char* k0 = weight + g * 72;		// const signed char* k0 = kernel.row<const signed char>(g);

			signed char* outptr0 = out;						// int* outptr0 = out.row<int>(0);
			signed char* outptr1 = out + 8 * outw;			// int* outptr1 = out.row<int>(1);

			signed char* img0 = in_mem + g * cstep;			// const Mat img0 = bottom_blob.channel(g);

			const signed char* r0 = img0 + 0 * w;			// const signed char* r0 = img0.row<const signed char>(0);
			const signed char* r1 = img0 + 8 * w;			// const signed char* r1 = img0.row<const signed char>(1);
			const signed char* r2 = img0 + 16 * w;			// const signed char* r2 = img0.row<const signed char>(2);
			const signed char* r3 = img0 + 24 * w;			// const signed char* r3 = img0.row<const signed char>(3);
			const signed char* r4 = img0 + 32 * w;			// const signed char* r4 = img0.row<const signed char>(4);

#if __ARM_NEON
			int8x8_t _k00 = vld1_s8(k0);
			int8x8_t _k01 = vld1_s8(k0 + 8);
			int8x8_t _k02 = vld1_s8(k0 + 16);
			int8x8_t _k10 = vld1_s8(k0 + 24);
			int8x8_t _k11 = vld1_s8(k0 + 32);
			int8x8_t _k12 = vld1_s8(k0 + 40);
			int8x8_t _k20 = vld1_s8(k0 + 48);
			int8x8_t _k21 = vld1_s8(k0 + 56);
			int8x8_t _k22 = vld1_s8(k0 + 64);
#endif
			int i = 0;
			for (; i + 1 < outh; i += 2)
			{
				int j = 0;
				for (; j + 1 < outw; j += 2)
				{
#if __ARM_NEON
					asm volatile(
						"mov			r4, #32				\n"

						"pld			[%2, #128]          \n"
						"vld1.s8		{d14-d17}, [%2 :128]!	\n"
						"vld1.s8		{d22}, [%2 :64]	\n"
						"pld			[%3, #128]          \n"
						"vld1.s8		{d18-d21}, [%3 :128]!	\n"

						"vmull.s8		q3, d14, %P9		\n"
						"vmull.s8		q4, d16, %P9		\n"
						"vmlal.s8		q3, d15, %P10		\n"
						"vmlal.s8		q4, d17, %P10		\n"

						"vmull.s8		q5, d16, %P11		\n"
						"vmull.s8		q6, d22, %P11		\n"
						"vmlal.s8		q5, d18, %P12		\n"
						"vmlal.s8		q6, d20, %P12		\n"

						"vaddl.s16		q0, d6, d10			\n"
						"vaddl.s16		q1, d7, d11			\n"
						"vaddl.s16		q2, d8, d12			\n"
						"vaddl.s16		q3, d9, d13			\n"

						"vld1.s8		{d22}, [%3 :64]	\n"
						"pld			[%4, #128]          \n"
						"vld1.s8		{d14-d17}, [%4 :128]!	\n"

						"vmull.s8		q5, d19, %P13		\n"
						"vmull.s8		q6, d21, %P13		\n"
						"vmlal.s8		q5, d20, %P14		\n"
						"vmlal.s8		q6, d22, %P14		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.s8		{d22}, [%4 :64]	\n"

						"vmull.s8		q5, d14, %P15		\n"
						"vmull.s8		q6, d16, %P15		\n"
						"vmlal.s8		q5, d15, %P16		\n"
						"vmlal.s8		q6, d17, %P16		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d16, %P17		\n"
						"vmull.s8		q6, d22, %P17		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%8 :128]	\n"
						"veor			q6, q6				\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"vld1.f32		{d18-d21}, [%18 :128] \n"

						"vcle.f32	q4, q0, q6			\n"
						"vmul.f32	q5, q0, q9			\n"
						"vbit.f32	q0, q5, q4			\n"

						"vcle.f32	q4, q1, q6			\n"
						"vmul.f32	q5, q1, q10			\n"
						"vbit.f32	q1, q5, q4			\n"

						"vcle.f32	q4, q2, q6			\n"
						"vmul.f32	q5, q2, q9			\n"
						"vbit.f32	q2, q5, q4			\n"

						"vcle.f32	q4, q3, q6			\n"
						"vmul.f32	q5, q3, q10			\n"
						"vbit.f32	q3, q5, q4			\n"

						"vcvtr.s32.f32	s0, s0				\n"
						"vcvtr.s32.f32	s1, s1				\n"
						"vcvtr.s32.f32	s2, s2				\n"
						"vcvtr.s32.f32	s3, s3				\n"

						"vcvtr.s32.f32	s4, s4				\n"
						"vcvtr.s32.f32	s5, s5				\n"
						"vcvtr.s32.f32	s6, s6				\n"
						"vcvtr.s32.f32	s7, s7				\n"

						"vcvtr.s32.f32	s8, s8				\n"
						"vcvtr.s32.f32	s9, s9				\n"
						"vcvtr.s32.f32	s10, s10			\n"
						"vcvtr.s32.f32	s11, s11			\n"

						"vcvtr.s32.f32	s12, s12			\n"
						"vcvtr.s32.f32	s13, s13			\n"
						"vcvtr.s32.f32	s14, s14			\n"
						"vcvtr.s32.f32	s15, s15			\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vst1.s8		{d0-d1}, [%0 :128]!		\n"

						"pld			[%5, #128]          \n"
						"vld1.s8		{d18-d21}, [%5 :128]!	\n"

						"vmull.s8		q3, d14, %P9		\n"
						"vmull.s8		q4, d16, %P9		\n"
						"vmlal.s8		q3, d15, %P10		\n"
						"vmlal.s8		q4, d17, %P10		\n"

						"vmull.s8		q5, d16, %P11		\n"
						"vmull.s8		q6, d22, %P11		\n"
						"vmlal.s8		q5, d18, %P12		\n"
						"vmlal.s8		q6, d20, %P12		\n"

						"vaddl.s16		q0, d6, d10			\n"
						"vaddl.s16		q1, d7, d11			\n"
						"vaddl.s16		q2, d8, d12			\n"
						"vaddl.s16		q3, d9, d13			\n"

						"vld1.s8		{d22}, [%5 :64]		\n"
						"pld			[%6, #128]          \n"
						"vld1.s8		{d14-d17}, [%6 :128]!	\n"

						"vmull.s8		q5, d19, %P13		\n"
						"vmull.s8		q6, d21, %P13		\n"
						"vmlal.s8		q5, d20, %P14		\n"
						"vmlal.s8		q6, d22, %P14		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.s8		{d22}, [%6 :64]		\n"

						"vmull.s8		q5, d14, %P15		\n"
						"vmull.s8		q6, d16, %P15		\n"
						"vmlal.s8		q5, d15, %P16		\n"
						"vmlal.s8		q6, d17, %P16		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vmull.s8		q5, d16, %P17		\n"
						"vmull.s8		q6, d22, %P17		\n"

						"vaddw.s16		q0, q0, d10			\n"
						"vaddw.s16		q1, q1, d11			\n"
						"vaddw.s16		q2, q2, d12			\n"
						"vaddw.s16		q3, q3, d13			\n"

						"vld1.f32		{d8-d11}, [%7 :128]	\n"

						"vcvt.f32.s32	q0, q0				\n"
						"vcvt.f32.s32	q1, q1				\n"
						"vcvt.f32.s32	q2, q2				\n"
						"vcvt.f32.s32	q3, q3				\n"

						"vmul.f32		q0, q0, q4			\n"
						"vmul.f32		q1, q1, q5			\n"
						"vmul.f32		q2, q2, q4			\n"
						"vmul.f32		q3, q3, q5			\n"

						"vld1.f32		{d8-d11}, [%8 :128]	\n"
						"veor			q6, q6				\n"

						"vadd.f32		q0, q0, q4			\n"
						"vadd.f32		q1, q1, q5			\n"
						"vadd.f32		q2, q2, q4			\n"
						"vadd.f32		q3, q3, q5			\n"

						"veor			q6, q6				\n"
						"vld1.f32		{d18-d21}, [%18 :128] \n"

						"vcle.f32	q4, q0, q6			\n"
						"vcle.f32	q5, q1, q6			\n"
						"vmul.f32	q7, q0, q9			\n"
						"vmul.f32	q8, q1, q10			\n"
						"vbit.f32	q0, q7, q4			\n"
						"vbit.f32	q1, q8, q5			\n"

						"vcle.f32	q4, q2, q6			\n"
						"vcle.f32	q5, q3, q6			\n"
						"vmul.f32	q7, q2, q9			\n"
						"vmul.f32	q8, q3, q10			\n"
						"vbit.f32	q2, q7, q4			\n"
						"vbit.f32	q3, q8, q5			\n"

						"vcvtr.s32.f32	s0, s0				\n"
						"vcvtr.s32.f32	s1, s1				\n"
						"vcvtr.s32.f32	s2, s2				\n"
						"vcvtr.s32.f32	s3, s3				\n"

						"vcvtr.s32.f32	s4, s4				\n"
						"vcvtr.s32.f32	s5, s5				\n"
						"vcvtr.s32.f32	s6, s6				\n"
						"vcvtr.s32.f32	s7, s7				\n"

						"vcvtr.s32.f32	s8, s8				\n"
						"vcvtr.s32.f32	s9, s9				\n"
						"vcvtr.s32.f32	s10, s10			\n"
						"vcvtr.s32.f32	s11, s11			\n"

						"vcvtr.s32.f32	s12, s12			\n"
						"vcvtr.s32.f32	s13, s13			\n"
						"vcvtr.s32.f32	s14, s14			\n"
						"vcvtr.s32.f32	s15, s15			\n"

						"vqmovn.s32		d0, q0				\n"
						"vqmovn.s32		d1, q1				\n"
						"vqmovn.s32		d2, q2				\n"
						"vqmovn.s32		d3, q3				\n"

						"vqmovn.s16		d0, q0				\n"
						"vqmovn.s16		d1, q1				\n"

						"vst1.s8		{d0-d1}, [%1 :128]!		\n"
						:
					"+r"(outptr0),			// %0
						"+r"(outptr1),			// %1
						"+r"(r0),				// %2
						"+r"(r1),				// %3
						"+r"(r2),				// %4
						"+r"(r3),				// %5
						"+r"(r4)				// %6
						:
						"r"(mem_scale + g * 8),	// %7
						"r"(mem_bias + g * 8),	// %8
						"w"(_k00),				// %9
						"w"(_k01),				// %10
						"w"(_k02),				// %11
						"w"(_k10),				// %12
						"w"(_k11),				// %13
						"w"(_k12),				// %14
						"w"(_k20),				// %15
						"w"(_k21),				// %16
						"w"(_k22),				// %17
						"r"(mem_prelu + g * 8)	// %18
						: "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "d22");
#else
					for (int u = 0; u < 8; u++)
					{
						int sum0 = r0[u] * k0[u + 0] + r0[u + 8] * k0[u + 8] + r0[u + 16] * k0[u + 16] +
							r1[u] * k0[u + 24] + r1[u + 8] * k0[u + 32] + r1[u + 16] * k0[u + 40] +
							r2[u] * k0[u + 48] + r2[u + 8] * k0[u + 56] + r2[u + 16] * k0[u + 64];
						int sum1 = r0[u + 16] * k0[u + 0] + r0[u + 24] * k0[u + 8] + r0[u + 32] * k0[u + 16] +
							r1[u + 16] * k0[u + 24] + r1[u + 24] * k0[u + 32] + r1[u + 32] * k0[u + 40] +
							r2[u + 16] * k0[u + 48] + r2[u + 24] * k0[u + 56] + r2[u + 32] * k0[u + 64];
						int sum2 = r2[u] * k0[u + 0] + r2[u + 8] * k0[u + 8] + r2[u + 16] * k0[u + 16] +
							r3[u] * k0[u + 24] + r3[u + 8] * k0[u + 32] + r3[u + 16] * k0[u + 40] +
							r4[u] * k0[u + 48] + r4[u + 8] * k0[u + 56] + r4[u + 16] * k0[u + 64];
						int sum3 = r2[u + 16] * k0[u + 0] + r2[u + 24] * k0[u + 8] + r2[u + 32] * k0[u + 16] +
							r3[u + 16] * k0[u + 24] + r3[u + 24] * k0[u + 32] + r3[u + 32] * k0[u + 40] +
							r4[u + 16] * k0[u + 48] + r4[u + 24] * k0[u + 56] + r4[u + 32] * k0[u + 64];

						float rVal;
						rVal = (sum0 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr0[u] = float2int8(rVal);
						rVal = (sum1 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr0[u + 8] = float2int8(rVal);
						rVal = (sum2 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr1[u] = float2int8(rVal);
						rVal = (sum3 * mem_scale[8 * g + u] + mem_bias[8 * g + u]); if (rVal < 0.0f) rVal *= mem_prelu[8 * g + u]; outptr1[u + 8] = float2int8(rVal);
					}
					r0 += 32;
					r1 += 32;
					r2 += 32;
					r3 += 32;
					r4 += 32;
					outptr0 += 16;
					outptr1 += 16;
#endif
				}

				r0 += (2 * w - outw) * 16;
				r1 += (2 * w - outw) * 16;
				r2 += (2 * w - outw) * 16;
				r3 += (2 * w - outw) * 16;
				r4 += (2 * w - outw) * 16;

				outptr0 += outw * 8;
				outptr1 += outw * 8;
			}
		}
	}

	// width = height = kernel = 8, c % 8 = 0
	void convd_k8s1_nf_pack8(ennq_blob* in_blob, ennq_blob* out_blob, const signed char* weight, const float* mem_scale, const float* mem_bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int c = in_blob->c;
		int cstep = in_blob->cstep;
		int insize = w * h;
		const signed char* in_mem = (signed char*)in_blob->mem;

		int out_cstep = 8;
		float* out_mem = (float*)out_blob->mem;
		int k_cstep = get_blob_size(insize * 8);
		int group = c / 8;

		out_blob->c = c;
		out_blob->w = 1;
		out_blob->h = 1;
		out_blob->cstep = 8;
		out_blob->packp = 8;
		out_blob->elemc = 4;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int g = 0; g < group; g++)
		{
			const signed char* inptr0 = in_mem + g * cstep;
			float* outptr0 = out_mem + g * out_cstep;
			const signed char* kmem = weight + g * k_cstep;
			const float* sc0 = mem_scale + g * 8;
			const float* bs0 = mem_bias + g * 8;

#if __ARM_NEON
			float32x4_t		_sc0 = vld1q_f32(sc0);
			float32x4_t		_sc1 = vld1q_f32(sc0 + 4);
			float32x4_t		_bs0 = vld1q_f32(bs0);
			float32x4_t		_bs1 = vld1q_f32(bs0 + 4);

			asm volatile(
				"veor       q0, q0              \n"
				"veor       q1, q1              \n"

				"lsr		r4, %3, #1			\n"
				"cmp		r4, #0				\n"
				"beq		1f					\n"

				"0:								\n"

				"pld		[%0, #128]			\n"
				"vld1.s8	{d6-d7}, [%0 :128]!	\n"

				"pld		[%1, #128]			\n"
				"vld1.s8	{d8-d9}, [%1 :128]!	\n"

				"vmull.s8	q2, d6, d8			\n"
				"vmlal.s8	q2, d7, d9			\n"

				"vaddw.s16	q0, q0, d4			\n"
				"vaddw.s16	q1, q1, d5			\n"

				"subs       r4, r4, #1          \n"
				"bne        0b                  \n"

				"1:								\n"

				"and		r4, %3, #1			\n"
				"cmp		r4, #0				\n"
				"beq		2f					\n"

				"pld		[%0, #128]			\n"
				"vld1.s8	{d6}, [%0 :64]!		\n"
				"pld		[%1, #128]			\n"
				"vld1.s8	{d8}, [%1 :64]!		\n"

				"vmull.s8	q2, d6, d8			\n"
				"vaddw.s16	q0, q0, d4			\n"
				"vaddw.s16	q1, q1, d5			\n"

				"2:								\n"

				"vmov.f32		q3,	%q6			\n"
				"vmov.f32		q4,	%q7			\n"
				"vcvt.f32.s32	q0, q0			\n"
				"vcvt.f32.s32	q1, q1			\n"
				"vmla.f32		q3, q0, %q4		\n"
				"vmla.f32		q4, q1, %q5		\n"

				"vst1.f32		{d6-d9}, [%2 :128]!	\n"

				:
			"+r"(inptr0),		// %0
				"+r"(kmem),			// %1
				"+r"(outptr0)		// %2
				:
				"r"(insize),		// %3
				"w"(_sc0),			// %4
				"w"(_sc1),			// %5
				"w"(_bs0),			// %6
				"w"(_bs1)			// %7
				:
				"memory", "cc", "r4", "q0", "q1", "q2", "q3", "q4"
				);
#else
			int sum[8] = { 0 };
			for (int j = 0; j < insize; j++)
			{
				sum[0] += inptr0[0] * kmem[0];
				sum[1] += inptr0[1] * kmem[1];
				sum[2] += inptr0[2] * kmem[2];
				sum[3] += inptr0[3] * kmem[3];
				sum[4] += inptr0[4] * kmem[4];
				sum[5] += inptr0[5] * kmem[5];
				sum[6] += inptr0[6] * kmem[6];
				sum[7] += inptr0[7] * kmem[7];
				inptr0 += 8;
				kmem += 8;
			}
			outptr0[0] = (sum[0] * sc0[0] + bs0[0]);
			outptr0[1] = (sum[1] * sc0[1] + bs0[1]);
			outptr0[2] = (sum[2] * sc0[2] + bs0[2]);
			outptr0[3] = (sum[3] * sc0[3] + bs0[3]);
			outptr0[4] = (sum[4] * sc0[4] + bs0[4]);
			outptr0[5] = (sum[5] * sc0[5] + bs0[5]);
			outptr0[6] = (sum[6] * sc0[6] + bs0[6]);
			outptr0[7] = (sum[7] * sc0[7] + bs0[7]);
#endif
		}
	}
}
