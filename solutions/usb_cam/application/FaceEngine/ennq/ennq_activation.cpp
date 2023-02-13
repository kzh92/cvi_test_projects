
#if __ARM_NEON
#include <arm_neon.h>
#endif

#include "ennq_activation.h"

namespace ENNQ
{
	void hardswish(float* data_inout, int size)
	{
		float* ptr = data_inout;

#if __ARM_NEON
		int nn = size >> 2;
		int remain = size - (nn << 2);
		float32x4_t _zero = vdupq_n_f32(0.f);
		float32x4_t _one = vdupq_n_f32(1.f);
		while (nn--)
		{
			float32x4_t _p = vld1q_f32(ptr);
			float32x4_t _ans = vdupq_n_f32(0.5f);
			_ans = vmlaq_n_f32(_ans, _p, 0.1666666667f);
			_ans = vmaxq_f32(_ans, _zero);
			_ans = vminq_f32(_ans, _one);
			_ans = vmulq_f32(_ans, _p);
			vst1q_f32(ptr, _ans);

			ptr += 4;
		}
#else
		int remain = size;
#endif // __ARM_NEON
		for (; remain > 0; remain--)
		{
			if (*ptr < -3.0f) *ptr = 0.f;
			else if (*ptr > 3.0f);
			else *ptr = *ptr * (*ptr * 0.1666666667f + 0.5f);
			++ptr;
		}
	}

	void relu6(float* data_inout, int size)
	{
		float* ptr = data_inout;

#if __ARM_NEON
		int nn = size >> 2;
		int remain = size & 3;

		float32x4_t _max = vdupq_n_f32(6.0f);
		float32x4_t _min = vdupq_n_f32(0.0f);

		if (nn > 0)
		{
			asm volatile(
				"0:                             \n"
				"pld        [%1, #128]          \n"
				"vld1.f32   {d0-d1}, [%1: 128]  \n"

				"vmax.f32   q0, q0, %q2         \n"
				"vmin.f32   q0, q0, %q3         \n"

				"subs       %0, #1              \n"
				"vst1.f32   {d0-d1}, [%1: 128]! \n"

				"bne        0b                  \n"

				: "+r"(nn),     // %0
				  "+r"(ptr)     // %1
				: "w"(_min),    // %q2
				  "w"(_max)     // %q3
				: "cc", "memory", "q0"
				);
		}
#else
		int remain = size;
#endif // __ARM_NEON

		for (; remain > 0; remain--)
		{
			if (*ptr < 0.0f) *ptr = 0.0f;
			if (*ptr > 6.0f) *ptr = 6.0f;
			ptr++;
		}
	}

	void hardsigmoid(float* data_inout, int size)
	{
		float* ptr = data_inout;

#if __ARM_NEON
		int nn = size >> 2;
		int remain = size - (nn << 2);

		float32x4_t _zero = vdupq_n_f32(0.f);
		float32x4_t _one = vdupq_n_f32(1.f);
		while (nn--)
		{
			float32x4_t _p = vld1q_f32(ptr);
			float32x4_t _ans = vdupq_n_f32(0.5f);
			_ans = vmlaq_n_f32(_ans, _p, 0.166666667f);
			_ans = vmaxq_f32(_ans, _zero);
			_ans = vminq_f32(_ans, _one);
			vst1q_f32(ptr, _ans);

			ptr += 4;
		}
#else
		int remain = size;
#endif // __ARM_NEON
		for (; remain > 0; remain--)
		{
			if (*ptr < -3.0f)
				*ptr = 0.f;
			else if (*ptr > 3.0f)
				*ptr = 1.f;
			else
				*ptr = *ptr * 0.166666667f + 0.5f;
			++ptr;
		}
	}
}
