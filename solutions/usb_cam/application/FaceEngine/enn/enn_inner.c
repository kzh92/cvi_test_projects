
#if __ARM_NEON
#include <arm_neon.h>
#endif

#include "enn_inner.h"

extern int g_nThreadCount;

    void enn_innerproduct(enn_blob* in_blob, enn_blob* out_blob, int num_output, unsigned short* kernel, unsigned short* bias)
	{
		int w = in_blob->w;
		int h = in_blob->h;
		int channels = in_blob->c;
		int cstep = in_blob->cstep;
		float* in_mem = (float*)in_blob->mem;
		int size = w * h;

		int outw = 1;
		int outch = 1;
		int outh = num_output;
		int out_cstep = num_output;
		int out_pack = 1;

		out_blob->w = outw;
		out_blob->h = outh;
		out_blob->cstep = out_cstep;
		out_blob->c = outch;
		out_blob->pack = out_pack;
		float* out_mem = (float*)out_blob->mem;

		const unsigned short* weight_data_ptr = kernel;

		int nn_num_output = num_output >> 2;
		int remain_num_output_start = nn_num_output << 2;

#if ((ENGINE_THREAD_COUNT) != 1)
        int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
        for (int pp = 0; pp < nn_num_output; pp++)
		{
			int p = pp * 4;

			float sum0 = 0.f;
			float sum1 = 0.f;
			float sum2 = 0.f;
			float sum3 = 0.f;

			if (bias)
			{
				sum0 = _gnu_h2f_internal_f(bias[p]);
				sum1 = _gnu_h2f_internal_f(bias[p + 1]);
				sum2 = _gnu_h2f_internal_f(bias[p + 2]);
				sum3 = _gnu_h2f_internal_f(bias[p + 3]);
			}

			const unsigned short* w0 = weight_data_ptr + size * channels * p;
			const unsigned short* w1 = weight_data_ptr + size * channels * (p + 1);
			const unsigned short* w2 = weight_data_ptr + size * channels * (p + 2);
			const unsigned short* w3 = weight_data_ptr + size * channels * (p + 3);

#if __ARM_NEON
			float32x4_t _sum0 = vdupq_n_f32(0.f);
			float32x4_t _sum1 = vdupq_n_f32(0.f);
			float32x4_t _sum2 = vdupq_n_f32(0.f);
			float32x4_t _sum3 = vdupq_n_f32(0.f);
#else
			float _sum00 = 0.0f;
			float _sum01 = 0.0f;
			float _sum02 = 0.0f;
			float _sum03 = 0.0f;
			float _sum10 = 0.0f;
			float _sum11 = 0.0f;
			float _sum12 = 0.0f;
			float _sum13 = 0.0f;
			float _sum20 = 0.0f;
			float _sum21 = 0.0f;
			float _sum22 = 0.0f;
			float _sum23 = 0.0f;
			float _sum30 = 0.0f;
			float _sum31 = 0.0f;
			float _sum32 = 0.0f;
			float _sum33 = 0.0f;
#endif // __ARM_NEON

			// channels
			int q;
			for (q = 0; q < channels; q++)
			{
				const float* m = in_mem + q * cstep; // bottom_blob.channel(q);

				int nn = size >> 2;
				int remain = size & 3;

				for (; nn > 0; nn--)
				{
#if __ARM_NEON
					float32x4_t _m = vld1q_f32(m);

					float32x4_t _w0 = vcvt_f32_f16((float16x4_t)vld1_u16(w0));
					_sum0 = vmlaq_f32(_sum0, _m, _w0);

					float32x4_t _w1 = vcvt_f32_f16((float16x4_t)vld1_u16(w1));
					_sum1 = vmlaq_f32(_sum1, _m, _w1);

					float32x4_t _w2 = vcvt_f32_f16((float16x4_t)vld1_u16(w2));
					_sum2 = vmlaq_f32(_sum2, _m, _w2);

					float32x4_t _w3 = vcvt_f32_f16((float16x4_t)vld1_u16(w3));
					_sum3 = vmlaq_f32(_sum3, _m, _w3);
#else
					_sum00 += m[0] * _gnu_h2f_internal_f(w0[0]);
					_sum01 += m[1] * _gnu_h2f_internal_f(w0[1]);
					_sum02 += m[2] * _gnu_h2f_internal_f(w0[2]);
					_sum03 += m[3] * _gnu_h2f_internal_f(w0[3]);
					_sum10 += m[0] * _gnu_h2f_internal_f(w1[0]);
					_sum11 += m[1] * _gnu_h2f_internal_f(w1[1]);
					_sum12 += m[2] * _gnu_h2f_internal_f(w1[2]);
					_sum13 += m[3] * _gnu_h2f_internal_f(w1[3]);
					_sum20 += m[0] * _gnu_h2f_internal_f(w2[0]);
					_sum21 += m[1] * _gnu_h2f_internal_f(w2[1]);
					_sum22 += m[2] * _gnu_h2f_internal_f(w2[2]);
					_sum23 += m[3] * _gnu_h2f_internal_f(w2[3]);
					_sum30 += m[0] * _gnu_h2f_internal_f(w3[0]);
					_sum31 += m[1] * _gnu_h2f_internal_f(w3[1]);
					_sum32 += m[2] * _gnu_h2f_internal_f(w3[2]);
					_sum33 += m[3] * _gnu_h2f_internal_f(w3[3]);
#endif
					m += 4;
					w0 += 4;
					w1 += 4;
					w2 += 4;
					w3 += 4;
				}

				for (; remain > 0; remain--)
				{
					sum0 += *m * _gnu_h2f_internal_f(*w0);
					sum1 += *m * _gnu_h2f_internal_f(*w1);
					sum2 += *m * _gnu_h2f_internal_f(*w2);
					sum3 += *m * _gnu_h2f_internal_f(*w3);

					m++;
					w0++;
					w1++;
					w2++;
					w3++;
				}

			}

#if __ARM_NEON
			float32x2_t _sum0ss = vadd_f32(vget_low_f32(_sum0), vget_high_f32(_sum0));
			float32x2_t _sum1ss = vadd_f32(vget_low_f32(_sum1), vget_high_f32(_sum1));
			float32x2_t _sum2ss = vadd_f32(vget_low_f32(_sum2), vget_high_f32(_sum2));
			float32x2_t _sum3ss = vadd_f32(vget_low_f32(_sum3), vget_high_f32(_sum3));

			float32x2_t _sum01ss = vpadd_f32(_sum0ss, _sum1ss);
			float32x2_t _sum23ss = vpadd_f32(_sum2ss, _sum3ss);

			sum0 += vget_lane_f32(_sum01ss, 0);
			sum1 += vget_lane_f32(_sum01ss, 1);
			sum2 += vget_lane_f32(_sum23ss, 0);
			sum3 += vget_lane_f32(_sum23ss, 1);
#else
			sum0 += ((_sum00 + _sum02) + (_sum01 + _sum03));
			sum1 += ((_sum10 + _sum12) + (_sum11 + _sum13));
			sum2 += ((_sum20 + _sum22) + (_sum21 + _sum23));
			sum3 += ((_sum30 + _sum32) + (_sum31 + _sum33));
#endif // __ARM_NEON

			out_mem[p] = sum0;
			out_mem[p + 1] = sum1;
			out_mem[p + 2] = sum2;
			out_mem[p + 3] = sum3;
		}

		// num_output
#if ((ENGINE_THREAD_COUNT) != 1)
        nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
        for (int p = remain_num_output_start; p < num_output; p++)
		{
			float sum = 0.f;

			if (bias)
				sum = _gnu_h2f_internal_f(bias[p]);

			const unsigned short* w = weight_data_ptr + size * channels * p;

#if __ARM_NEON
			float32x4_t _sum = vdupq_n_f32(0.f);
			float32x4_t _sum2 = vdupq_n_f32(0.f);
#else
			float _sum00 = 0.0f;
			float _sum01 = 0.0f;
			float _sum02 = 0.0f;
			float _sum03 = 0.0f;
			float _sum10 = 0.0f;
			float _sum11 = 0.0f;
			float _sum12 = 0.0f;
			float _sum13 = 0.0f;
#endif // __ARM_NEON

			// channels
			int q;
			for (q = 0; q < channels; q++)
			{
				const float* m = in_mem + q * cstep; // bottom_blob.channel(q);

				int nn = size >> 3;
				int remain = size & 7;

#if __ARM_NEON
				if (nn > 0)
				{
					asm volatile(
						"0:                             \n"
						"pld        [%1, #256]          \n"
						"vld1.f32   {d0-d3}, [%1 :128]! \n"
						"pld        [%2, #256]          \n"
						// "vld1.f32   {d4-d7}, [%2]!      \n"
						"vld1.u16   {d4-d5}, [%2]!      \n"
						"vcvt.f32.f16 q3, d5			\n"
						"vcvt.f32.f16 q2, d4			\n"
						"vmla.f32   %q3, q0, q2         \n"
						"subs       %0, #1              \n"
						"vmla.f32   %q4, q1, q3         \n"
						"bne        0b                  \n"
						: "=r"(nn),     // %0
						"=r"(m),      // %1
						"=r"(w),      // %2
						"=w"(_sum),   // %3
						"=w"(_sum2)   // %4
						: "0"(nn),
						"1"(m),
						"2"(w),
						"3"(_sum),
						"4"(_sum2)
						: "cc", "memory", "q0", "q1", "q2", "q3"
						);
				}
#else
				for (; nn > 0; nn--)
				{
					_sum00 += m[0] * _gnu_h2f_internal_f(w[0]);
					_sum01 += m[1] * _gnu_h2f_internal_f(w[1]);
					_sum02 += m[2] * _gnu_h2f_internal_f(w[2]);
					_sum03 += m[3] * _gnu_h2f_internal_f(w[3]);
					_sum10 += m[4] * _gnu_h2f_internal_f(w[4]);
					_sum11 += m[5] * _gnu_h2f_internal_f(w[5]);
					_sum12 += m[6] * _gnu_h2f_internal_f(w[6]);
					_sum13 += m[7] * _gnu_h2f_internal_f(w[7]);
					m += 8;
					w += 8;

				}
#endif // __ARM_NEON
				for (; remain > 0; remain--)
				{
					sum += *m * _gnu_h2f_internal_f(*w);

					m++;
					w++;
				}
			}

#if __ARM_NEON
			_sum = vaddq_f32(_sum, _sum2);
			float32x2_t _sumss = vadd_f32(vget_low_f32(_sum), vget_high_f32(_sum));
			_sumss = vpadd_f32(_sumss, _sumss);
			sum += vget_lane_f32(_sumss, 0);
#else
			sum += ((_sum00 + _sum10) + (_sum02 + _sum12)) + ((_sum01 + _sum11) + (_sum03 + _sum13));
#endif // __ARM_NEON

			out_mem[p] = sum; // top_blob[p] = sum;
		}
	}
