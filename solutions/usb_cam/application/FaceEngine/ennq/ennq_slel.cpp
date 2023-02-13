
#include <algorithm>

#include "ennq_slel.h"

namespace ENNQ
{
	void slice_eltwise_ff(const float* in, int size, float* out)
	{
		int sz = size >> 1;

		const float* ptr = in;
		const float* ptr1 = in + sz;
		float* outptr = out;

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
			*outptr = std::max(*ptr, *ptr1);
			ptr++;
			ptr1++;
			outptr++;
		}
	}

	void eltwise_sum_ff(const float* data_in1, const float* data_in2, int size, float* data_out)
	{
		const float* ptr = data_in1;
		const float* ptr1 = data_in2;
		float* outptr = data_out;

#if __ARM_NEON
		int nn = size >> 2;
		int remain = size - (nn << 2);
		if (nn > 0)
		{
			asm volatile(
				"0:                             \n"
				"pld        [%1, #128]          \n"
				"pld        [%2, #128]          \n"
				"vld1.f32   {d0-d1}, [%1]!      \n"
				"vld1.f32   {d2-d3}, [%2]!      \n"
				"vadd.f32   q0, q0, q1          \n"
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
		int remain = size;
#endif // __ARM_NEON

		for (; remain > 0; remain--)
		{
			*outptr = *ptr1 + *ptr;
			ptr++;
			ptr1++;
			outptr++;
		}
	}

	void eltwise_sum_ff_packed(ennq_blob* in_blob1, ennq_blob* in_blob2, ennq_blob* out_blob)
	{
		int ch = in_blob1->c;
		int width = in_blob1->w;
		int height = in_blob1->h;
		int cstep = in_blob1->cstep;
		int pack = in_blob1->packp;
		int elem = in_blob1->elemc; // elem == 4

		int size = width * height;
		int p_sz = size * pack;

		out_blob->c = ch;
		out_blob->w = width;
		out_blob->h = height;
		out_blob->cstep = cstep;
		out_blob->packp = pack;
		out_blob->elemc = elem;

		int nnc = ch / pack;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < nnc; c++)
		{
			float* in_mem1 = ((float*)in_blob1->mem) + c * cstep;
			float* in_mem2 = ((float*)in_blob2->mem) + c * cstep;
			float* out_mem = ((float*)out_blob->mem) + c * cstep;
#if __ARM_NEON
			int nn = p_sz >> 2;
			int remain = p_sz - (nn << 2);
			if (nn > 0)
			{
				asm volatile(
					"0:                             \n"
					"pld        [%1, #128]          \n"
					"vld1.f32   {d0-d1}, [%1]!      \n"
					"pld        [%2, #128]          \n"
					"vld1.f32   {d2-d3}, [%2]!      \n"
					"vadd.f32   q0, q0, q1          \n"
					"subs       %0, #1              \n"
					"vst1.f32   {d0-d1}, [%3]!      \n"
					"bne        0b                  \n"
					:
				"+r"(nn),		// %0
					"+r"(in_mem1),  // %1
					"+r"(in_mem2),  // %2
					"+r"(out_mem)	// %3
					:
					: "cc", "memory", "q0", "q1"
					);
			}
#else
			int remain = p_sz;
#endif // __ARM_NEON

			for (; remain > 0; remain--)
			{
				*out_mem = *in_mem1 + *in_mem2;
				in_mem1++;
				in_mem2++;
				out_mem++;
			}
		}
	}
}
