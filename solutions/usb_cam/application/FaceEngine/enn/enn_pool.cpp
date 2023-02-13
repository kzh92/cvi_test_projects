
#if __ARM_NEON
#include <arm_neon.h>
#endif

#include "enn_pool.h"

namespace ENN
{
	void pooling2x2s2_max_pack4_neon(enn_blob* in_blob, enn_blob* out_blob)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = (w - 2) / 2 + 1;
		int outh = (h - 2) / 2 + 1;
		int out_size = outw * outh;
		int out_cstep = out_size * 4;
		float* out_mem = (float*)out_blob->mem;

		int outch = inch;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = 4;

		const int tailstep = (w - 2 * outw + w) * 4;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int q = 0; q < inch; q++)
		{
			float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);
			float* outptr = out_mem + q * out_cstep; // top_blob.channel(q);

			const float* r0 = img0;			// img0.row(0);
			const float* r1 = img0 + 4 * w; // img0.row(1);

			for (int i = 0; i < outh; i++)
			{
				int j = 0;

				for (; j + 3 < outw; j += 4)
				{
#if __ARM_NEON
					asm volatile(
						"pld        [%1, #512]      \n"
						"vldm       %1!, {d0-d7}    \n"

						"vmax.f32   q0, q0, q1      \n"
						"vmax.f32   q2, q2, q3      \n"

						"pld        [%1, #512]      \n"
						"vldm       %1!, {d8-d15}   \n"

						"vmax.f32   q4, q4, q5      \n"
						"vmax.f32   q6, q6, q7      \n"

						"pld        [%2, #512]      \n"
						"vldm       %2!, {d16-d23}  \n"

						"vmax.f32   q8, q8, q9      \n"
						"vmax.f32   q10, q10, q11   \n"

						"pld        [%2, #512]      \n"
						"vldm       %2!, {d24-d31}  \n"

						"vmax.f32   q12, q12, q13   \n"
						"vmax.f32   q14, q14, q15   \n"

						"vmax.f32   q0, q0, q8      \n"
						"vmax.f32   q1, q2, q10     \n"
						"vmax.f32   q2, q4, q12     \n"
						"vmax.f32   q3, q6, q14     \n"

						"vstm       %0!, {d0-d7}    \n"

						: "=r"(outptr),     // %0
						"=r"(r0),         // %1
						"=r"(r1)          // %2
						: "0"(outptr),
						"1"(r0),
						"2"(r1)
						: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
						);
#else
					for (int u = 0; u < 4; u++)
					{
						for (int v = 0; v < 4; v++)
						{
							float val, max_val;
							max_val = r0[8 * u + v];
							val = r0[8 * u + v + 4];
							if (max_val < val) max_val = val;
							val = r1[8 * u + v];
							if (max_val < val) max_val = val;
							val = r1[8 * u + v + 4];
							if (max_val < val) max_val = val;
							outptr[u * 4 + v] = max_val;
						}
					}
					r0 += 32;
					r1 += 32;
					outptr += 16;
#endif
				}
				for (; j < outw; j++)
				{
#if __ARM_NEON
					float32x4_t _r00 = vld1q_f32(r0);
					float32x4_t _r01 = vld1q_f32(r0 + 4);
					float32x4_t _r10 = vld1q_f32(r1);
					float32x4_t _r11 = vld1q_f32(r1 + 4);

					float32x4_t _max0 = vmaxq_f32(_r00, _r01);
					float32x4_t _max1 = vmaxq_f32(_r10, _r11);
					float32x4_t _max = vmaxq_f32(_max0, _max1);

					vst1q_f32(outptr, _max);
#else
					for (int v = 0; v < 4; v++)
					{
						float val, max_val;
						max_val = r0[v];
						val = r0[v + 4];
						if (max_val < val) max_val = val;
						val = r1[v];
						if (max_val < val) max_val = val;
						val = r1[v + 4];
						if (max_val < val) max_val = val;
						outptr[v] = max_val;
					}
#endif
					r0 += 8;
					r1 += 8;
					outptr += 4;
				}

				r0 += tailstep;
				r1 += tailstep;
			}
		}
	}

	void pooling2x2s2_max_neon(enn_blob* in_blob, enn_blob* out_blob)
		// void pooling2x2s2_max_neon(const Mat& bottom_blob, Mat& top_blob, const Option& opt)
	{
		int w = in_blob->w; // bottom_blob.w;
		int h = in_blob->h;
		int inch = in_blob->c; // bottom_blob.c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;

		int outw = w / 2; // top_blob.w;
		int outh = h / 2;
		int out_size = outw * outh;
		int out_cstep = out_size;
		float* out_mem = (float*)out_blob->mem;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = inch;
		out_blob->pack = 1;

		const int tailstep = w - 2 * outw + w;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int q = 0; q < inch; q++)
		{
			const float* img0 = in_mem + q * cstep; // bottom_blob.channel(q);
			float* outptr = out_mem + q * out_cstep; // top_blob.channel(q);

			const float* r0 = img0;
			const float* r1 = img0 + w;

			for (int i = 0; i < outh; i++)
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
						"0:                             \n"
						"pld        [%1, #256]          \n"
						"pld        [%2, #256]          \n"
						"vld1.f32   {d0-d3}, [%1]!      \n"
						"vld1.f32   {d4-d7}, [%2]!      \n"
						"vmax.f32   q0, q0, q2          \n"
						"vmax.f32   q1, q1, q3          \n"
						"vpmax.f32  d4, d0, d1          \n"
						"vpmax.f32  d5, d2, d3          \n"
						"subs       %0, #1              \n"
						"vst1.f32   {d4-d5}, [%3]!      \n"
						"bne        0b                  \n"
						: "=r"(nn),     // %0
						"=r"(r0),     // %1
						"=r"(r1),     // %2
						"=r"(outptr)  // %3
						: "0"(nn),
						"1"(r0),
						"2"(r1),
						"3"(outptr)
						: "cc", "memory", "q0", "q1", "q2", "q3"
						);
				}
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					float max_val = r0[0];
					if (max_val < r0[1]) max_val = r0[1]; // std::max(r0[0], r0[1]);
					if (max_val < r1[0]) max_val = r1[0];
					if (max_val < r1[1]) max_val = r1[1]; // std::max(r1[0], r1[1]);

					*outptr = max_val;

					r0 += 2;
					r1 += 2;
					outptr++;
				}

				r0 += tailstep;
				r1 += tailstep;
			}
		}
	}

	void pooling_global_aver_pack4_neon(enn_blob* in_blob, enn_blob* out_blob)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int inch = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;
		float* out_mem = (float*)out_blob->mem;

		out_blob->w = 1;
		out_blob->h = 1;
		out_blob->cstep = 4;
		out_blob->c = inch;
		out_blob->pack = 4;

		int size = w * h;

		for (int q = 0; q < inch; q++)
		{
			float* img0 = in_mem + q * cstep; // const Mat img0 = bottom_blob.channel(q);
			float* outptr = out_mem + q * 4; // top_blob.channel(q);
#if __ARM_NEON
			int nn = size;
			asm volatile(
				"vdup.f32   q3, %3				\n"
				"veor		q2, q2, q2			\n"

				"0:								\n"
				"pld        [%0, #128]			\n"
				"vld1.f32   {d8-d9}, [%0 :128]!	\n"
				"subs       %1, #1				\n"
				"vadd.f32	q2, q2, q4			\n"
				"bne        0b					\n"
				"vmul.f32	q2, q2, q3			\n"
				"vst1.f32	{d4-d5}, [%2 :128]	\n"

				: "+r"(img0),			// %0
				  "+r"(nn),				// %1
				  "+r"(outptr)			// %2
				: "r"(1.0f / size)		// %3
				: "memory", "q2", "q3", "q4"
				);
#else
			float v0 = 0.0f, v1 = 0.0f, v2 = 0.0f, v3 = 0.0f;
			for (int v = 0; v < size; v++)
			{
				v0 += img0[0];
				v1 += img0[1];
				v2 += img0[2];
				v3 += img0[3];
				img0 += 4;
			}
			v0 /= size;
			v1 /= size;
			v2 /= size;
			v3 /= size;
			outptr[0] = v0;
			outptr[1] = v1;
			outptr[2] = v2;
			outptr[3] = v3;
#endif
		}
	}
}
