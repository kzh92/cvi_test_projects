
#if __ARM_NEON
#include <arm_neon.h>
#endif
#include <stdio.h>

#define		BAYER_TYPE_BGGR		(0)
#define		BAYER_TYPE_RGGB		(1)

#define		BAYER_TYPE	BAYER_TYPE_BGGR
// #define		BAYER_TYPE	BAYER_TYPE_RGGB

#if BAYER_TYPE == BAYER_TYPE_BGGR
# define RGB2Y(r, g, b) (unsigned char)(((76 * (r) + 149 * (g) + 29 * (b) + 128) >> 8))
# define RGB2YCOEFF		(0x1D954C)
#endif
#if BAYER_TYPE == BAYER_TYPE_RGGB
# define RGB2Y(r, g, b) (unsigned char)(((29 * (r) + 149 * (g) + 76 * (b) + 128) >> 8))
# define RGB2YCOEFF		(0x4C951D)
#endif

void convert_bayer2y_rotate_cm(const unsigned char* bayer, unsigned char* yuv, int width, int height, int iCamFlip)
{
//#if __ARM_NEON
//    printf("Neon Neon Neon!!!\n");
//#endif
	int n_start_offset;
	int n_line_offset;
	int n_y_delta;

	if (iCamFlip)
	{
		n_start_offset = (height - 1);
		n_line_offset = height;
		n_y_delta = -1;
	}
	else
	{
		n_start_offset = height * (width - 1);
		n_line_offset = -height;
		n_y_delta = 1;
	}

	unsigned char* pY_start = yuv + n_start_offset;

	int width_2 = width - 2;
	int height_2 = height - 2;

	{
// 		int y = 0;

		int b00, b01, b10, b11;
		int g00, g01, g10, g11;
		int r00, r01, r10, r11;

		unsigned char* pY = pY_start;

		const unsigned char* pixel_0 = bayer;
		const unsigned char* pixel_m = pixel_0 - width;
		const unsigned char* pixel_1 = pixel_0 + width;
		const unsigned char* pixel_2 = pixel_1 + width;

		{
//			int x = 0;

			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p02 = pixel_0[2];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p12 = pixel_1[2];
			int p20 = pixel_2[0];
			int p21 = pixel_2[1];
			int p22 = pixel_2[2];

			b00 = p00;
			g00 = (p10 + p01) / 2;
			r00 = p11;

			b01 = (p00 + p02) / 2;
			g01 = p01;
			r01 = p11;

			b10 = (p00 + p20) / 2;
			g10 = p10;
			r10 = p11;

			b11 = (p00 + p20 + p02 + p22) / 4;
			g11 = (p21 + p01 + p10 + p12) / 4;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}

		for (int x = 2; x < width_2; x += 2)
		{
			int p0m = pixel_0[-1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p02 = pixel_0[2];
			int p1m = pixel_1[-1];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p12 = pixel_1[2];
			int p20 = pixel_2[0];
			int p21 = pixel_2[1];
			int p22 = pixel_2[2];

			b00 = p00;
			g00 = (p10 + p0m + p01) / 3;
			r00 = (p1m + p11) / 2;

			b01 = (p00 + p02) / 2;
			g01 = p01;
			r01 = p11;

			b10 = (p00 + p20) / 2;
			g10 = p10;
			r10 = (p1m + p11) / 2;

			b11 = (p00 + p20 + p02 + p22) / 4;
			g11 = (p21 + p01 + p10 + p12) / 4;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}

		{
//			int x = width_2;

			int p0m = pixel_0[-1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p1m = pixel_1[-1];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p20 = pixel_2[0];
			int p21 = pixel_2[1];

			b00 = p00;
			g00 = (p10 + p0m + p01) / 3;
			r00 = (p1m + p11) / 2;

			b01 = p00;
			g01 = p01;
			r01 = p11;

			b10 = (p00 + p20) / 2;
			g10 = p10;
			r10 = (p1m + p11) / 2;

			b11 = (p00 + p20) / 2;
			g11 = (p21 + p01 + p10) / 3;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}
	}

//#pragma omp parallel for num_threads(2)
	for (int y = 2; y < height_2; y += 2)
	{
		int b00, b01, b10, b11;
		int g00, g01, g10, g11;
		int r00, r01, r10, r11;

		unsigned char* pY = pY_start + y * n_y_delta;

		const unsigned char* pixel_0 = bayer + y * width;
		const unsigned char* pixel_m = pixel_0 - width;
		const unsigned char* pixel_1 = pixel_0 + width;
		const unsigned char* pixel_2 = pixel_1 + width;

		{
//			int x = 0;

			int pm0 = pixel_m[0];
			int pm1 = pixel_m[1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p02 = pixel_0[2];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p12 = pixel_1[2];
			int p20 = pixel_2[0];
			int p21 = pixel_2[1];
			int p22 = pixel_2[2];

			b00 = p00;
			g00 = (p10 + pm0 + p01) / 3;
			r00 = (pm1 + p11) / 2;

			b01 = (p00 + p02) / 2;
			g01 = p01;
			r01 = (pm1 + p11) / 2;

			b10 = (p00 + p20) / 2;
			g10 = p10;
			r10 = p11;

			b11 = (p00 + p20 + p02 + p22) / 4;
			g11 = (p21 + p01 + p10 + p12) / 4;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}

		int count = (width_2 - 2) / 2;
#if __ARM_NEON
		int nn = count >> 3;
		int remain = (count & 7);

		asm volatile(

			"cmp		%8, #0				\n"
			"beq		0f					\n"

			"sub		%4, #1				\n"

			"0:								\n"

			"sub		r4, %0, #1			\n"
			"pld        [r4, #256]          \n"
			"vld2.u8	{d10-d11}, [r4]		\n"
			"vmovl.u8	q6, d10				\n"		// q6 = pmm;
			"vmovl.u8	q5, d11				\n"		// q5 = pm0;
			"add		r4, r4, #2			\n"
			"vld2.u8	{d18-d19}, [r4]		\n"
			"vmovl.u8	q9, d18				\n"		// q9 = pm1;

			"add		%0, %0, #16			\n"

			"sub		r4, %1, #1			\n"
			"pld        [r4, #256]          \n"
			"vld2.u8	{d8-d9}, [r4]		\n"
			"vmovl.u8	q3, d8				\n"		// q3 = p0m;
			"vmovl.u8	q4, d9				\n"		// q4 = p00;
			"add		r4, r4, #2			\n"
			"vld2.u8	{d14-d15}, [r4]		\n"
			"vmovl.u8   q8, d14				\n"		// q8 = p01;
			"vmovl.u8   q7, d15				\n"		// q7 = p02;

			"vadd.s16	q5, q5, q3			\n"		// q5 = q5 + q3;
			"vadd.s16	q7, q7, q4			\n"		// q7 = q7 + q4;

			"add		%1, %1, #16			\n"

			"sub		r4, %2, #1			\n"
			"pld        [r4, #256]          \n"
			"vld2.u8	{d22-d23}, [r4]		\n"
			"vmovl.u8	q12, d22			\n"		// q12 = p1m;
			"vmovl.u8	q11, d23			\n"		// q11 = p10;
			"add		r4, r4, #2			\n"
			"vld2.u8	{d28-d29}, [r4]		\n"
			"vmovl.u8   q15, d28			\n"		// q15 = p11;
			"vmovl.u8   q14, d29			\n"		// q14 = p12;

			"vadd.s16	q3, q11, q8			\n"		// q3 = q11 + q8;
			"vadd.s16	q6, q6, q12			\n"		// q6 = q6 + q12;
			"vadd.s16	q9, q9, q15			\n"		// q9 = q9 + q15;
			"vadd.s16	q12, q12, q15		\n"		// q12 = q12 + q15;
			"vadd.s16	q5, q5, q3			\n"		// q5 = q5 + q3;
			"vadd.s16	q6, q6, q9			\n"		// q6 = q6 + q9;
			"vadd.s16	q14, q14, q3		\n"		// q14 = q14 + q3;

			"add		%2, %2, #16			\n"

			"pld        [%3, #256]          \n"
			"vld2.u8	{d6-d7}, [%3]		\n"
			"vmovl.u8	q10, d6				\n"		// q10 = p20;
			"vmovl.u8	q3, d7				\n"		// q3 = p21;
			"add		r4, %3, #2			\n"
			"vld2.u8	{d26-d27}, [r4]		\n"
			"vmovl.u8	q13, d26			\n"		// q13 = p22;

			"vadd.s16	q14, q14, q3		\n"		// q14 = q14 + q3;
			"vadd.s16	q13, q13, q10		\n"		// q13 = q13 + q10;
			"vadd.s16	q10, q10, q4		\n"		// q10 = q10 + q4;
			"vadd.s16	q13, q13, q7		\n"		// q13 = q13 + q7;

			"add		%3, %3, #16			\n"

			// "vshr.u16	q4,  q4,  0			\n"
			"vshr.u16	q5,  q5,  2			\n"
			"vshr.u16	q6,  q6,  2			\n"
			"vshr.u16	q7,  q7,  1			\n"
			// "vshr.u16	q8,  q8,  0			\n"
			"vshr.u16	q9,  q9,  1			\n"
			"vshr.u16	q10, q10, 1			\n"
			// "vshr.u16	q11, q11, 0			\n"
			"vshr.u16	q12, q12, 1			\n"
			"vshr.u16	q13, q13, 2			\n"
			"vshr.u16	q14, q14, 2			\n"
			// "vshr.u16	q15, q15, 0			\n"

			"vdup.u32	d6, %7				\n"
			"vmovl.u8	q3, d6				\n"

			"vmov.s32	q1, #128			\n"
			"vmov.s32	q2, #128			\n"

			"vmlal.s16	q1, d12, d6[0]		\n"
			"vmlal.s16	q2, d13, d6[0]		\n"

			"vmlal.s16	q1, d10, d6[1]		\n"
			"vmlal.s16	q2, d11, d6[1]		\n"

			"vmlal.s16	q1, d8, d6[2]		\n"
			"vmlal.s16	q2, d9, d6[2]		\n"

			"vshrn.s32 d2, q1, #8			\n"
			"vshrn.s32 d3, q2, #8			\n"
			"vqmovn.u16	d0, q1				\n"

			"vmov.s32	q1, #128			\n"
			"vmov.s32	q2, #128			\n"

			"vmlal.s16	q1, d18, d6[0]		\n"
			"vmlal.s16	q2, d19, d6[0]		\n"

			"vmlal.s16	q1, d16, d6[1]		\n"
			"vmlal.s16	q2, d17, d6[1]		\n"

			"vmlal.s16	q1, d14, d6[2]		\n"
			"vmlal.s16	q2, d15, d6[2]		\n"

			"vshrn.s32 d2, q1, #8			\n"
			"vshrn.s32 d3, q2, #8			\n"
			"vqmovn.u16	d1, q1				\n"

			"vzip.u8    d0, d1				\n"

			"vmov.s32	q4, #128			\n"
			"vmov.s32	q5, #128			\n"

			"vmlal.s16	q4, d24, d6[0]		\n"
			"vmlal.s16	q5, d25, d6[0]		\n"

			"vmlal.s16	q4, d22, d6[1]		\n"
			"vmlal.s16	q5, d23, d6[1]		\n"

			"vmlal.s16	q4, d20, d6[2]		\n"
			"vmlal.s16	q5, d21, d6[2]		\n"

			"vshrn.s32 d8, q4, #8			\n"
			"vshrn.s32 d9, q5, #8			\n"
			"vqmovn.u16	d2, q4				\n"

			"vmov.s32	q4, #128			\n"
			"vmov.s32	q5, #128			\n"

			"vmlal.s16	q4, d30, d6[0]		\n"
			"vmlal.s16	q5, d31, d6[0]		\n"

			"vmlal.s16	q4, d28, d6[1]		\n"
			"vmlal.s16	q5, d29, d6[1]		\n"

			"vmlal.s16	q4, d26, d6[2]		\n"
			"vmlal.s16	q5, d27, d6[2]		\n"

			"vshrn.s32 d8, q4, #8			\n"
			"vshrn.s32 d9, q5, #8			\n"
			"vqmovn.u16	d3, q4				\n"

			"vzip.u8    d2, d3				\n"

			"cmp		%8, #0				\n"
			"beq		1f					\n"
			"vswp		q0, q1				\n"

			"1:								\n"

			"vzip.u8	q0, q1				\n"

			"vst1.u16	{d0[0]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d0[1]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d0[2]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d0[3]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d1[0]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d1[1]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d1[2]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d1[3]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d2[0]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d2[1]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d2[2]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d2[3]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d3[0]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d3[1]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d3[2]}, [%4]		\n"
			"add		%4, %6				\n"
			"vst1.u16	{d3[3]}, [%4]		\n"
			"add		%4, %6				\n"

			"subs       %5, #1				\n"
			"bne        0b					\n"
			
			"cmp		%8, #0				\n"
			"beq		2f					\n"
			"add		%4, #1				\n"

			"2:								\n"
			: "+r"(pixel_m),			// %0
			  "+r"(pixel_0),			// %1
			  "+r"(pixel_1),			// %2
			  "+r"(pixel_2),			// %3
			  "+r"(pY),					// %4
			  "+r"(nn)					// %5
			: "r"(n_line_offset),		// %6
			  "r"(RGB2YCOEFF),			// %7
			  "r"(iCamFlip)				// %8
			: "memory", "r4", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
			);
#else
		int remain = count;
#endif
		for (; remain > 0; remain--)
		{
			int pmm = pixel_m[-1];
			int pm0 = pixel_m[0];
			int pm1 = pixel_m[1];
			int p0m = pixel_0[-1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p02 = pixel_0[2];
			int p1m = pixel_1[-1];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p12 = pixel_1[2];
			int p20 = pixel_2[0];
			int p21 = pixel_2[1];
			int p22 = pixel_2[2];

			int q3, q4, q5, q6, q7, q8, q9, q10, q11, q12, q13, q14, q15;

			q6 = pmm;							// q6(pmm)
			q5 = pm0;							// q5(pm0), q6(pmm)
			q9 = pm1;							// q5(pm0), q6(pmm), q9(pm1)

			q3 = p0m;							// q3(p0m), q5(pm0), q6(pmm), q9(pm1)
			q4 = p00;							// q3(p0m), q5(pm0), q6(pmm), q9(pm1)																							////			q4(p00)
			q8 = p01;							// q3(p0m), q5(pm0), q6(pmm), q9(pm1)																							////			q8(p01)
			q7 = p02;							// q3(p0m), q5(pm0), q6(pmm), q7(p02), q9(pm1)

			q5 = q5 + q3;						// q5(pm0+p0m), q6(pmm), q7(p02), q9(pm1)					// del(q3)
			q7 = q7 + q4;						// q5(pm0+p0m), q6(pmm), q9(pm1)																								////			q7(p00+p02)

			q12 = p1m;							// q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)
			q11 = p10;							// q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)																						////			q11(p10)
			q15 = p11;							// q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)																						////			q15(p11)
			q14 = p12;							// q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)

			q3 = q11 + q8;						// q3(p10+p01), q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)
			q6 = q6 + q12;						// q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q9(pm1), q12(p1m), q14(p12)
			q9 = q9 + q15;						// q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q12(p1m), q14(p12)																	////			q9(pm1+p11)
			q12 = q12 + q15;					// q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q14(p12)																				////			q12(p1m+p11)
			q5 = q5 + q3;						// q3(p10+p01), q6(pmm+p1m), q14(p12)																							////			q5(p10+p01+pm0+p0m)
			q6 = q6 + q9;						// q3(p10+p01), q14(p12)																										////			q6(pmm+p1m+pm1+p11)
			q14 = q14 + q3;						// q14(p10+p01+p12)											// del(q3)

			q10 = p20;							// q10(p20), q14(p10+p01+p12)
			q3 = p21;							// q3(p21), q10(p20), q14(p10+p01+p12)
			q13 = p22;							// q3(p21), q10(p20), q13(p22), q14(p10+p01+p12)

			q14 = q14 + q3;						// q10(p20), q13(p22)										// del(q3)															////			q14(p10+p01+p12+p21)
			q13 = q13 + q10;					// q10(p20), q13(p22+p20)
			q10 = q10 + q4;						// q13(p22+p20)																													////			q10(p20+p00)
			q13 = q13 + q7;						//																																////			q13(p22+p20+p00+p02)

			b00 = q4;
			g00 = q5 >> 2;
			r00 = q6 >> 2;

			b01 = q7 >> 1;
			g01 = q8;
			r01 = q9 >> 1;

			b10 = q10 >> 1;
			g10 = q11;
			r10 = q12 >> 1;

			b11 = q13 >> 2;
			g11 = q14 >> 2;
			r11 = q15;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}

		{
//			int x = width_2;

			int pmm = pixel_m[-1];
			int pm0 = pixel_m[0];
			int pm1 = pixel_m[1];
			int p0m = pixel_0[-1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p1m = pixel_1[-1];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p20 = pixel_2[0];
			int p21 = pixel_2[1];

			b00 = p00;
			g00 = (p10 + pm0 + p0m + p01) / 4;
			r00 = (pmm + p1m + pm1 + p11) / 4;

			b01 = p00;
			g01 = p01;
			r01 = (pm1 + p11) / 2;

			b10 = (p00 + p20) / 2;
			g10 = p10;
			r10 = (p1m + p11) / 2;

			b11 = (p00 + p20) / 2;
			g11 = (p21 + p01 + p10) / 3;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}
	}

	{
//		int y = height_2;

		int b00, b01, b10, b11;
		int g00, g01, g10, g11;
		int r00, r01, r10, r11;

		unsigned char* pY = pY_start + height_2 * n_y_delta;

		const unsigned char* pixel_0 = bayer + height_2 * width;
		const unsigned char* pixel_m = pixel_0 - width;
		const unsigned char* pixel_1 = pixel_0 + width;
		const unsigned char* pixel_2 = pixel_1 + width;

		{
//			int x = 0;

			int pm0 = pixel_m[0];
			int pm1 = pixel_m[1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p02 = pixel_0[2];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p12 = pixel_1[2];

			b00 = p00;
			g00 = (p10 + pm0 + p01) / 3;
			r00 = (pm1 + p11) / 2;

			b01 = (p00 + p02) / 2;
			g01 = p01;
			r01 = (pm1 + p11) / 2;

			b10 = p00;
			g10 = p10;
			r10 = p11;

			b11 = (p00 + p02) / 2;
			g11 = (p01 + p10 + p12) / 3;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}

		for (int x = 2; x < width_2; x += 2)
		{
			int pmm = pixel_m[-1];
			int pm0 = pixel_m[0];
			int pm1 = pixel_m[1];
			int p0m = pixel_0[-1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p02 = pixel_0[2];
			int p1m = pixel_1[-1];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];
			int p12 = pixel_1[2];

			b00 = p00;
			g00 = (p10 + pm0 + p0m + p01) / 4;
			r00 = (pmm + p1m + pm1 + p11) / 4;

			b01 = (p00 + p02) / 2;
			g01 = p01;
			r01 = (pm1 + p11) / 2;

			b10 = p00;
			g10 = p10;
			r10 = (p1m + p11) / 2;

			b11 = (p00 + p02) / 2;
			g11 = (p01 + p10 + p12) / 3;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;

			pixel_m += 2;
			pixel_0 += 2;
			pixel_1 += 2;
			pixel_2 += 2;
		}

		{
//			int x = width_2;

			int pmm = pixel_m[-1];
			int pm0 = pixel_m[0];
			int pm1 = pixel_m[1];
			int p0m = pixel_0[-1];
			int p00 = pixel_0[0];
			int p01 = pixel_0[1];
			int p1m = pixel_1[-1];
			int p10 = pixel_1[0];
			int p11 = pixel_1[1];

			b00 = p00;
			g00 = (p10 + pm0 + p0m + p01) / 4;
			r00 = (pmm + p1m + pm1 + p11) / 4;

			b01 = p00;
			g01 = p01;
			r01 = (pm1 + p11) / 2;

			b10 = p00;
			g10 = p10;
			r10 = (p1m + p11) / 2;

			b11 = p00;
			g11 = (p01 + p10) / 2;
			r11 = p11;

			pY[0] = RGB2Y(r00, g00, b00);	// pY[0]
			pY[n_y_delta] = RGB2Y(r10, g10, b10);	// pY_[0]
			pY += n_line_offset;
			pY[0] = RGB2Y(r01, g01, b01);	// pY[1]
			pY[n_y_delta] = RGB2Y(r11, g11, b11);	// pY_[1]
			pY += n_line_offset;
		}
	}
}
