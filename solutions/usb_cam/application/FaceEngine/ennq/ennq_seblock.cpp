
#include "ennq_seblock.h"
#include "ennq_inner.h"
#include "ennq_pool.h"
#include "ennq_quantize.h"
#include "ennq_activation.h"

namespace ENNQ
{
	void seblock(float* data_inout, int ch_in, int dim_in, int ch_mid, const signed char* fc1, const float* fc1_q, const float* fc1_sc, const float* fc1_bs, const signed char* fc2, const float* fc2_q, const float* fc2_sc, const float* fc2_bs, float* mem_tmp)
	{
		// (ch_in * 4 + ch_mid)
		signed char* mem_tmp_mid = (signed char*)(mem_tmp + (ch_in * 4));

		global_pooling(data_inout, ch_in, dim_in, (float*)mem_tmp); // ch_in * 4
		quantize((float*)mem_tmp, ch_in, (signed char*)mem_tmp, fc1_q);
		inner_nn_r6((signed char*)mem_tmp, ch_in, mem_tmp_mid, ch_mid, fc1, fc1_sc, fc1_bs, fc2_q); // ch_mid
		inner_nf(mem_tmp_mid, ch_mid, (float*)mem_tmp, ch_in, fc2, fc2_sc, fc2_bs); // ch_in * 4
		hardsigmoid((float*)mem_tmp, ch_in); // ch_in * 4
		multiply(data_inout, ch_in, dim_in, mem_tmp);
	}

	void multiply(float* data_inout, int ch_in, int dim_in, float* data_scale)
	{
		int size = dim_in * dim_in;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int q = 0; q < ch_in; q++)
		{
			float* ptr = data_inout + q * size;
			float scale = data_scale[q];
#if __ARM_NEON
			int nn = size >> 2;
			int remain = size - (nn << 2);
			if (nn > 0)
			{
				asm volatile(
					"vdup.f32   q2, %2              \n"
					"0:                             \n"
					"pld        [%1, #128]          \n"
					"vld1.f32   {d0-d1}, [%1]       \n"
					"vmul.f32   q1, q0, q2          \n"
					"subs       %0, #1              \n"
					"vst1.f32   {d2-d3}, [%1]!      \n"
					"bne        0b                  \n"
					: "+r"(nn),     // %0
					"+r"(ptr)     // %1
					: "r"(scale)        // %2
					: "cc", "memory", "q0", "q1", "q2"
					);
			}
#else
			int remain = size;
#endif // __ARM_NEON
			for (; remain > 0; remain--)
			{
				*ptr = scale * *ptr;
				ptr++;
			}
		}
	}
}
