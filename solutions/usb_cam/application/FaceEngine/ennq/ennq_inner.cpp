
#include "ennq_inner.h"

namespace ENNQ
{
	void inner_nn_r(const signed char* data_in, int dim_in, signed char* data_out, int dim_out, const signed char* kernel, const float* mem_scale, const float* mem_bias)
	{
#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < dim_out; p++)
		{
			int sum = 0;
			const signed char* w = kernel + dim_in * p;
			const signed char* m = data_in;

			for (int i = 0; i < dim_in; i++)
			{
				sum += m[i] * w[i];
			}
			int nValue = (int)(sum * mem_scale[p] + mem_bias[p]);

			if (nValue < 0) nValue = 0;
			if (nValue > 127) nValue = 127;
			data_out[p] = (signed char)nValue;
		}
	}

	void inner_nn_r6(const signed char* data_in, int dim_in, signed char* data_out, int dim_out, const signed char* kernel, const float* mem_scale, const float* mem_bias, const float* prev_scale)
	{
		int maxValue = int(6.0f * prev_scale[0] + 0.5f);
		if (maxValue > 127) maxValue = 127;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < dim_out; p++)
		{
			int sum = 0;
			const signed char* w = kernel + dim_in * p;
			const signed char* m = data_in;

			for (int i = 0; i < dim_in; i++)
			{
				sum += m[i] * w[i];
			}
			int nValue = (int)(sum * mem_scale[p] + mem_bias[p]);

			if (nValue < 0) nValue = 0;
			if (nValue > maxValue) nValue = maxValue;
			data_out[p] = (signed char)nValue;
		}
	}

	void inner_nf(const signed char* data_in, int dim_in, float* data_out, int dim_out, const signed char* kernel, const float* mem_scale, const float* mem_bias)
	{
#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int p = 0; p < dim_out; p++)
		{
			int sum = 0;
			const signed char* w = kernel + dim_in * p;
			const signed char* m = data_in;

			for (int i = 0; i < dim_in; i++)
			{
				sum += m[i] * w[i];
			}
			data_out[p] = sum * mem_scale[p] + mem_bias[p];
		}
	}

	void inner_ff_packed(ennq_blob* in_blob, ennq_blob* out_blob, int dim_out, const float* kernel, const float* mem_scale, const float* mem_bias)
	{
		out_blob->c = 1;
		out_blob->w = dim_out;
		out_blob->h = 1;
		out_blob->cstep = get_blob_size(dim_out, 4);
		out_blob->packp = 1;
		out_blob->elemc = 4;

		const float* data_in = (float*)in_blob->mem;
		float* data_out = (float*)out_blob->mem;
		int dim_in = in_blob->c;

#if __ARM_NEON
#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int oc = 0; oc < dim_out; oc += 4)
		{
			const float* imin_iter = data_in;
			float* imout_iter = data_out + oc;
			const float* kernel_iter0 = kernel + oc * dim_in;
			const float* kernel_iter1 = kernel_iter0 + dim_in;
			const float* kernel_iter2 = kernel_iter1 + dim_in;
			const float* kernel_iter3 = kernel_iter2 + dim_in;
			const float* mem_scale_iter = mem_scale + oc;
			const float* mem_bias_iter = mem_bias + oc;
			int  cnt = dim_in;

			asm volatile(

				"vmov.s32 q0, #0            \n" // q0 = 0
				"vmov.s32 q1, #0            \n" // q1 = 0
				"vmov.s32 q2, #0            \n" // q2 = 0
				"vmov.s32 q3, #0            \n" // q3 = 0

				"1:                         \n"
				"pld      [%4, #128]        \n"
				"vld1.f32 {d8,d9}, [%4]!    \n" // q4 = [in]

				"pld      [%0, #128]        \n"
				"vld1.f32 {d10,d11}, [%0]!  \n" // q5 = k0

				"pld      [%1, #128]        \n"
				"vld1.f32 {d12,d13}, [%1]!  \n" // q6 = k1

				"pld      [%2, #128]        \n"
				"vld1.f32 {d14,d15}, [%2]!  \n" // q7 = k2

				"pld      [%3, #128]        \n"
				"vld1.f32 {d16,d17}, [%3]!  \n" // q8 = k3

				"vmla.f32 q0, q4, q5        \n"
				"vmla.f32 q1, q4, q6        \n"
				"vmla.f32 q2, q4, q7        \n"
				"vmla.f32 q3, q4, q8        \n"

				"subs     %5, #4	        \n"
				"bne      1b                \n"

				"vpadd.f32 d0, d0, d1       \n"
				"vpadd.f32 d1, d2, d3       \n"
				"vpadd.f32 d2, d4, d5       \n"
				"vpadd.f32 d3, d6, d7       \n"

				"vld1.f32 {d4,d5}, [%6]     \n" // scale
				"vld1.f32 {d6,d7}, [%7]     \n" // bias

				"vpadd.f32 d0, d0, d1       \n"
				"vpadd.f32 d1, d2, d3       \n"

				"vmla.f32  q3, q0, q2       \n"
				"vst1.f32 {d6, d7}, [%8]    \n"

				: "+r"(kernel_iter0),		// %0
				"+r"(kernel_iter1),		// %1
				"+r"(kernel_iter2),		// %2
				"+r"(kernel_iter3),		// %3
				"+r"(imin_iter),			// %4
				"+r"(cnt)					// %5
				: "r"(mem_scale_iter),		// %6
				"r"(mem_bias_iter),		// %7
				"r"(imout_iter)			// %8
				: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8"
				);
		}

#else
#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int oc = 0; oc < dim_out; oc++)
		{
			float out_val = 0.0f;
			const float* imin_iter = data_in;
			const float* kernel_iter = kernel + oc * dim_in;

			for (int ic = 0; ic < dim_in; ic++)
			{
				out_val += *imin_iter++ * *kernel_iter++;
			}

			data_out[oc] = (out_val * mem_scale[oc] + mem_bias[oc]);
		}
#endif //__ARM_NEON
	}
}
