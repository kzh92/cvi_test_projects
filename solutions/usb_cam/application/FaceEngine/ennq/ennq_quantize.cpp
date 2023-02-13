
#include <math.h>

#include "ennq_quantize.h"

namespace ENNQ
{
	static inline signed char float2int8(float v)
	{
		int int32 = (int)round(v);
		if (int32 > 127) return 127;
		if (int32 < -128) return -128;
		return (signed char)int32;
	}

	void quantize(float* data_in, int size, signed char* data_out, const float* pNextQuantScale)
	{
		float scale = *pNextQuantScale;
		float* ptr = data_in;
		signed char* outptr = data_out;

#if __ARM_NEON
		int nn = size >> 3;
		int remain = size & 7;
		asm volatile(
			"pld        [%1, #256]          \n"
			"vld1.f32   {d0-d3}, [%1]!      \n"
			"vdup.32    q10, %6             \n"

			"0:                             \n"
			"vmul.f32   q0,q0,q10           \n"
			"vmul.f32   q1,q1,q10           \n"

			"vcvtr.s32.f32 s0,s0            \n"
			"vcvtr.s32.f32 s1,s1            \n"
			"vcvtr.s32.f32 s2,s2            \n"
			"vcvtr.s32.f32 s3,s3            \n"
			"vcvtr.s32.f32 s4,s4            \n"
			"vcvtr.s32.f32 s5,s5            \n"
			"vcvtr.s32.f32 s6,s6            \n"
			"vcvtr.s32.f32 s7,s7            \n"

			"vqmovn.s32 d4,q0               \n"
			"vqmovn.s32 d5,q1               \n"

			"pld        [%1, #256]          \n"
			"vld1.f32   {d0-d3}, [%1]!      \n"

			"vqmovn.s16 d4, q2              \n"
			"vst1.8     {d4}, [%2]!         \n"

			"subs       %0, #1              \n"
			"bne        0b                  \n"

			"sub        %1, #32             \n"
			: "=r"(nn),         // %0
			"=r"(ptr),        // %1
			"=r"(outptr)      // %2
			: "0"(nn),
			"1"(ptr),
			"2"(outptr),
			"r"(scale)        // %6
			: "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q10", "q11"
			);
#else
		int remain = size;
#endif // __ARM_NEON
		for (; remain > 0; remain--)
		{
			*outptr = float2int8(*ptr * scale);

			ptr++;
			outptr++;
		}
	}

	void quantize_packed(ennq_blob* in_blob, ennq_blob* out_blob, const float* pNextQuantScale)
	{
		int ch = in_blob->c;
		int width = in_blob->w;
		int height = in_blob->h;
		int cstep = in_blob->cstep;
		int pack = in_blob->packp;
		int size = width * height;
		int p_sz = size * pack;

		int out_cstep = get_blob_size(p_sz);

		out_blob->w = width;
		out_blob->h = height;
		out_blob->c = ch;
		out_blob->cstep = out_cstep;
		out_blob->packp = pack;
		out_blob->elemc = 1;

		int nnc = ch / pack;

		float scale = *pNextQuantScale;

#if ((ENGINE_THREAD_COUNT) != 1)
		int nThreadCount = g_nThreadCount;
#pragma omp parallel for num_threads(nThreadCount)
#endif
		for (int c = 0; c < nnc; c++)
		{
			float* in_mem = ((float*)in_blob->mem) + c * cstep;
			signed char* out_mem = ((signed char*)out_blob->mem) + c * out_cstep;
#if __ARM_NEON
			int nn = p_sz >> 3;
			int remain = p_sz & 7;

			asm volatile(

				"vdup.32    q10, %3             \n"

				"0:                             \n"
				"pld        [%1, #128]          \n"
				"vld1.f32   {d0-d3}, [%1 :128]!      \n"

				"vmul.f32   q0,q0,q10           \n"
				"vmul.f32   q1,q1,q10           \n"

				"vcvtr.s32.f32 s0,s0            \n"
				"vcvtr.s32.f32 s1,s1            \n"
				"vcvtr.s32.f32 s2,s2            \n"
				"vcvtr.s32.f32 s3,s3            \n"
				"vcvtr.s32.f32 s4,s4            \n"
				"vcvtr.s32.f32 s5,s5            \n"
				"vcvtr.s32.f32 s6,s6            \n"
				"vcvtr.s32.f32 s7,s7            \n"

				"vqmovn.s32 d4,q0               \n"
				"vqmovn.s32 d5,q1               \n"

				"vqmovn.s16 d4, q2              \n"
				"subs       %0, #1              \n"
				"vst1.8     {d4}, [%2 :64]!         \n"

				"bne        0b                  \n"

				:
				"+r"(nn),         // %0
				"+r"(in_mem),     // %1
				"+r"(out_mem)     // %2
				:
				"r"(scale)        // %3
				: "cc", "memory", "q0", "q1", "q2", "q10"
				);
#else
			int remain = p_sz;
#endif // __ARM_NEON
			for (; remain > 0; remain--)
			{
				*out_mem++ = float2int8(*in_mem * scale);
				in_mem++;
			}
		}
	}
}
