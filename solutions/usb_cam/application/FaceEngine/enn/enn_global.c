
#include <stddef.h>
#include "enn_global.h"
#include <malloc.h>

#define _NEON_FP16

    int enn_get_blob_size(int size, int align_size) // int align_size = 4
	{
		return (size + (align_size - 1)) & (-align_size);
	}

    unsigned char* enn_aligned_malloc(int sz, int align_size) // int align_size = 16
	{
        unsigned char* mem_real = (unsigned char*)malloc(sz + align_size + sizeof(unsigned char*));
		if (!mem_real) return 0;

		unsigned char** adata = (unsigned char**)((((size_t)((unsigned char**)mem_real + 1)) + align_size - 1) & (-align_size));
		adata[-1] = mem_real;
		return (unsigned char*)adata;
	}

    void enn_aligned_free(void* adata)
	{
		if (!adata) return;
        free(((unsigned char**)adata)[-1]);
	}

	float _gnu_h2f_internal_f(unsigned short a)
	{
#if __ARM_NEON
        typedef union _u_fp16_tag
		{
			unsigned short u;
			__fp16 f;
        } _u_fp16;

		_u_fp16 u;
		u.u = a;
		float f = u.f;
		return f;
#else
		int exponent;
		int sign;
		int significand;

		exponent = (a >> 10) & 0x1F;
		sign = a & 0x8000;
		significand = a & 0x3FF;

		if (exponent == 0)
		{
			if (significand == 0)
			{
				significand = 0;
				exponent = 0;
			}
			else
			{
				exponent = 0;
				while ((significand & 0x400) == 0)
				{
					significand <<= 1;
					exponent--;
				}
				exponent++;
				significand &= 0x3FF;
				exponent += 112;
			}
		}
		else if (exponent < 31)
		{
			exponent += 112;
		}
		else
		{
			exponent = 255;
#ifdef _NEON_FP16
			if (significand) significand |= 0x200;
#endif
		}
        typedef union _u_float_tag
		{
			unsigned int u;
			float f;
        }_u_float;
		_u_float f;
		f.u = (sign << 16) | ((significand << 13) | (exponent << 23));
		return f.f;
#endif
	}

	unsigned short _gnu_f2h_internal_f(float v)
	{
#if __ARM_NEON
        typedef union _u_fp16_tag
		{
			unsigned short u;
			__fp16 f;
        }_u_fp16;
		_u_fp16 u;
		u.f = v;
		return u.u;
#else
        typedef union _u_float_tag
		{
			unsigned int u;
			float f;
        }_u_float;
		_u_float f;
		f.f = v;
		unsigned int a = f.u;
		unsigned int exponent;
		unsigned int significand;
		unsigned int sign;
		unsigned int mantissa;
		int realexp;
		unsigned int mask;
		unsigned int addval;
		unsigned int tail;

		exponent = (a >> 23) & 0xFF;
		significand = a & 0x7FFFFF;
		sign = (a >> 16) & 0x8000;

		if (exponent == 0)
		{
			return sign;
		}

		if (exponent == 255)
		{
#ifdef _NEON_FP16
			if (significand) significand |= 0x400000;
#else
			significand |= 0x400000;
#endif
			return sign | 0x7C00 | (significand >> 13);
		}

		realexp = exponent - 127;
		mantissa = significand | 0x800000;

		if (realexp < -25)
		{
			mask = 0xFFFFFF;
		}
		else if (realexp < -14)
		{
			mask = 0xFFFFFF >> (realexp + 25);
		}
		else
		{
			mask = 0x1FFF;
		}

		tail = mask & mantissa;

		if (tail)
		{
			addval = (unsigned int)(mask + 1) >> 1;
			if (tail == addval)
				addval = mantissa & (addval << 1);

			mantissa += addval;
			if (mantissa >= 0x1000000)
			{
				mantissa >>= 1;
				realexp++;
			}
		}

		if (realexp > 15)
		{
			realexp = 31;
			mantissa = 0;
		}
		else if (realexp < -14)
		{
			mantissa >>= (-14 - realexp);
			realexp = 0;
		}
		else
		{
			realexp += 14;
		}

		return sign | ((realexp << 10) + (mantissa >> 13));
#endif
	}

	void _gnu_h2f_internal_vector(const unsigned short* src, float* dst, int n)
	{
		int nn = 0;
#if __ARM_NEON
		int cnt = n / 4;
		asm volatile(
			"0:                             \n"
			"vld1.u32   {d0}, [%0]!			\n"
			"subs       %2, %2, #1          \n"
			"vcvt.f32.f16      q0, d0       \n"
			"vst1.u32   {d0-d1}, [%1]!		\n"
			"bne        0b                  \n"

			: "=r"(src),	// %0
			"=r"(dst),		// %1
			"=r"(cnt)		// %2
			: "0"(src),
			"1"(dst),
			"2"(cnt)
			: "memory", "q0"
			);
		nn = n & 3;
#else
		nn = n;
#endif
		int i;
		for (i = 0; i < nn; i++)
		{
			dst[i] = _gnu_h2f_internal_f(src[i]);
		}
	}

    void _gnu_f2h_internal_vector(const float* src, unsigned short* dst, int n)
    {
        int nn = 0;
#if __ARM_NEON
        int cnt = n / 4;
        asm volatile(
            "0:                             \n"
            "vld1.u16   {d0-d1}, [%0]!		\n"
            "subs       %2, %2, #1          \n"
            "vcvt.f16.f32      d0, q0       \n"
            "vst1.u32   {d0}, [%1]!			\n"
            "bne        0b                  \n"

            : "=r"(src),	// %0
            "=r"(dst),		// %1
            "=r"(cnt)		// %2
            : "0"(src),
            "1"(dst),
            "2"(cnt)
            : "memory", "q0"
            );
        nn = n & 3;
#else
        nn = n;
#endif
        int i;
        for (i = 0; i < nn; i++)
        {
            dst[i] = _gnu_f2h_internal_f(src[i]);
        }
    }
