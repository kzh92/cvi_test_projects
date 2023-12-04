#include "ImageProcessing.h"
#include "common_types.h"
#include <string.h>

void Shrink_RGB(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width)
{    
    int* E05BFF1C_2 = (int*)my_malloc((dst_width + dst_height) * 2 * sizeof(int));
    int* E05C83D4_2 = (int*)my_malloc((dst_width + dst_height) * 2 * sizeof(int));//E05C83D4;
    int nRateXDesToSrc = ((src_width - 1) << 10) / (dst_width - 1);
    int nRateYDesToSrc = ((src_height - 1) << 10) / (dst_height - 1);

    //LOGE("In shrink 0 = %d, %d", dst_height, dst_width);
    int nX, nY;
    if (dst_width > 0)
    {

        int nSrcX_10 = 0;
        for(nX = 0; nX < dst_width; nX ++)
        {
            int nSrcX = nSrcX_10 >> 10;
            int nDiffX = nSrcX_10 - (nSrcX << 10);
            int nSrcX_1 = nSrcX + 1;
            int n1_DiffX = 0x400 - nDiffX;

            if (src_width - 1 <= nSrcX)
            {
                E05C83D4_2[(dst_height + nX) * 2] = 0x400;
                E05BFF1C_2[(dst_height + nX) * 2] = src_width - 1;
                E05BFF1C_2[(dst_height + nX) * 2 + 1] = src_width - 1;
                E05C83D4_2[(dst_height + nX) * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[(dst_height + nX) * 2] = nSrcX;
                E05C83D4_2[(dst_height + nX) * 2] = n1_DiffX;
                E05BFF1C_2[(dst_height + nX) * 2 + 1] = nSrcX_1;
                E05C83D4_2[(dst_height + nX) * 2 + 1] =  nDiffX;
            }
            nSrcX_10 += nRateXDesToSrc;//
        } //while (R5 != dst_width);//BNE		loc_E0315FAC
    }

    //LOGE("In shrink 1");

    if (dst_height > 0)
    {
        int nSrcY_10 = 0;

        for(nY = 0; nY < dst_height; nY ++)
        {
            int nSrcY = nSrcY_10 >> 10;
            int nDiffY = nSrcY_10 - (nSrcY << 10);
            int n1_DiffY = 0x400 - nDiffY;
            int nSrcY_1 = nSrcY + 1;

            if (src_height - 1 <= nSrcY)
            {
                E05BFF1C_2[nY * 2] = src_height - 1;
                E05C83D4_2[nY * 2] = 0x400;
                E05BFF1C_2[nY * 2 + 1] = src_height - 1;
                E05C83D4_2[nY * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[nY * 2] = nSrcY;
                E05C83D4_2[nY * 2] = n1_DiffY;
                E05BFF1C_2[nY * 2 + 1] = nSrcY_1;
                E05C83D4_2[nY * 2 + 1] = nDiffY;
            }
            nSrcY_10 += nRateYDesToSrc;
        }// while (R5 != R7);//BNE		loc_E0316060
        //LOGE("In shrink 2");
        unsigned char* pDst = dst;
        for(nY = 0; nY < dst_height; nY ++)
        {
            //LOGE("In shrink 4");
            int nSrcIndexY1, nSrcIndexY2;
            nSrcIndexY1 = src_width * E05BFF1C_2[nY * 2];
            nSrcIndexY2 = src_width * E05BFF1C_2[nY * 2 + 1];
            int nYAlpha1, nYAlpha2;
            nYAlpha1 = E05C83D4_2[nY * 2];
            nYAlpha2 = E05C83D4_2[nY * 2 + 1];

            if (dst_width > 0)
            {
                for(nX = 0; nX < dst_width; nX ++)
                {
                    int nSrcIndexX1 = E05BFF1C_2[(dst_height + nX) * 2 + 1];
                    int nSrcIndexX2 = E05BFF1C_2[(dst_height + nX) * 2];
                    int nAlpha1, nAlpha2;
                    nAlpha1 = E05C83D4_2[(dst_height + nX) * 2 + 1];
                    nAlpha2 = E05C83D4_2[(dst_height + nX) * 2];

                    int nColorIndex;
                    for(nColorIndex = 0; nColorIndex < 3; nColorIndex ++)
                    {
                        *pDst = (nYAlpha2 * src[(nSrcIndexY2 + nSrcIndexX2) * 3 + nColorIndex] * nAlpha2 + nYAlpha2 * src[(nSrcIndexY2 + nSrcIndexX1) * 3 + nColorIndex] * nAlpha1 + nAlpha2 * nYAlpha1 * src[(nSrcIndexY1 + nSrcIndexX2) * 3 + nColorIndex] + nAlpha1 * nYAlpha1 * src[(nSrcIndexY1 + nSrcIndexX1) * 3 + nColorIndex] + 0x80000) >> 20;//STRB		R1, [R11],#1
                        pDst ++;
                    }

                    //CMP		R11, R10
                } //while (R11 != R10);//BNE		loc_E0316138
            }
        } //while (R12 != R2);//BNE		loc_E03160F4

    }

    //LOGE("In shrink 3");
    my_free(E05BFF1C_2);
    my_free(E05C83D4_2);
}

#define     BAYER_TYPE_BGGR     (0)
#define     BAYER_TYPE_RGGB     (1)

#define     BAYER_TYPE  BAYER_TYPE_BGGR
// #define      BAYER_TYPE  BAYER_TYPE_RGGB

#if BAYER_TYPE == BAYER_TYPE_BGGR
# define RGB2Y(r, g, b) (unsigned char)(((76 * (r) + 149 * (g) + 29 * (b) + 128) >> 8))
# define RGB2YCOEFF     (0x1D954C)
#endif
#if BAYER_TYPE == BAYER_TYPE_RGGB
# define RGB2Y(r, g, b) (unsigned char)(((29 * (r) + 149 * (g) + 76 * (b) + 128) >> 8))
# define RGB2YCOEFF     (0x4C951D)
#endif

void convert_bayer2y_rotate_cm(unsigned char* bayer, int width, int height)
{
    unsigned char* pY_start = bayer;

    int width_2 = width - 2;
    int height_2 = height - 2;

    int t1, t2;
    unsigned char tp[2][1920];
    int tidx1, tidx2;

    {
        //      int y = 0;

        int b00, b01, b10, b11;
        int g00, g01, g10, g11;
        int r00, r01, r10, r11;

        unsigned char* pY = pY_start;

        const unsigned char* pixel_0 = bayer;
        const unsigned char* pixel_m = pixel_0 - width;
        const unsigned char* pixel_1 = pixel_0 + width;
        const unsigned char* pixel_2 = pixel_1 + width;

        tidx1 = 0; tidx2 = 1;
        memcpy(tp[tidx1], pixel_1, width);

        {
            //          int x = 0;

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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        for (int x = 2; x < width_2; x += 2)
        {
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p1m = t2; // pixel_1[-1];
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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        {
            //          int x = width_2;

            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p1m = t2; // pixel_1[-1];
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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

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

        unsigned char* pY = pY_start + y * width;

        const unsigned char* pixel_0 = bayer + y * width;
        const unsigned char* pixel_m = tp[tidx1]; // pixel_0 - width;
        const unsigned char* pixel_1 = pixel_0 + width;
        const unsigned char* pixel_2 = pixel_1 + width;

        tidx1 = tidx2; tidx2 = 1 - tidx2;
        memcpy(tp[tidx1], pixel_1, width);

        {
            //          int x = 0;

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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        int count = (width_2 - 2) / 2;
        int remain = count;
        for (; remain > 0; remain--)
        {
            int pmm = pixel_m[-1];
            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p1m = t2; // pixel_1[-1];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p12 = pixel_1[2];
            int p20 = pixel_2[0];
            int p21 = pixel_2[1];
            int p22 = pixel_2[2];

            int q3, q4, q5, q6, q7, q8, q9, q10, q11, q12, q13, q14, q15;

            q6 = pmm;                           // q6(pmm)
            q5 = pm0;                           // q5(pm0), q6(pmm)
            q9 = pm1;                           // q5(pm0), q6(pmm), q9(pm1)

            q3 = p0m;                           // q3(p0m), q5(pm0), q6(pmm), q9(pm1)
            q4 = p00;                           // q3(p0m), q5(pm0), q6(pmm), q9(pm1)                                                                                           ////            q4(p00)
            q8 = p01;                           // q3(p0m), q5(pm0), q6(pmm), q9(pm1)                                                                                           ////            q8(p01)
            q7 = p02;                           // q3(p0m), q5(pm0), q6(pmm), q7(p02), q9(pm1)

            q5 = q5 + q3;                       // q5(pm0+p0m), q6(pmm), q7(p02), q9(pm1)                   // del(q3)
            q7 = q7 + q4;                       // q5(pm0+p0m), q6(pmm), q9(pm1)                                                                                                ////            q7(p00+p02)

            q12 = p1m;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)
            q11 = p10;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)                                                                                      ////            q11(p10)
            q15 = p11;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)                                                                                      ////            q15(p11)
            q14 = p12;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)

            q3 = q11 + q8;                      // q3(p10+p01), q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)
            q6 = q6 + q12;                      // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q9(pm1), q12(p1m), q14(p12)
            q9 = q9 + q15;                      // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q12(p1m), q14(p12)                                                                    ////            q9(pm1+p11)
            q12 = q12 + q15;                    // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q14(p12)                                                                              ////            q12(p1m+p11)
            q5 = q5 + q3;                       // q3(p10+p01), q6(pmm+p1m), q14(p12)                                                                                           ////            q5(p10+p01+pm0+p0m)
            q6 = q6 + q9;                       // q3(p10+p01), q14(p12)                                                                                                        ////            q6(pmm+p1m+pm1+p11)
            q14 = q14 + q3;                     // q14(p10+p01+p12)                                         // del(q3)

            q10 = p20;                          // q10(p20), q14(p10+p01+p12)
            q3 = p21;                           // q3(p21), q10(p20), q14(p10+p01+p12)
            q13 = p22;                          // q3(p21), q10(p20), q13(p22), q14(p10+p01+p12)

            q14 = q14 + q3;                     // q10(p20), q13(p22)                                       // del(q3)                                                          ////            q14(p10+p01+p12+p21)
            q13 = q13 + q10;                    // q10(p20), q13(p22+p20)
            q10 = q10 + q4;                     // q13(p22+p20)                                                                                                                 ////            q10(p20+p00)
            q13 = q13 + q7;                     //                                                                                                                              ////            q13(p22+p20+p00+p02)

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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        {
            //          int x = width_2;

            int pmm = pixel_m[-1];
            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p1m = t2; // pixel_1[-1];
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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }
    }

    {
        //      int y = height_2;

        int b00, b01, b10, b11;
        int g00, g01, g10, g11;
        int r00, r01, r10, r11;

        unsigned char* pY = pY_start + height_2 * width;

        const unsigned char* pixel_0 = bayer + height_2 * width;
        const unsigned char* pixel_m = tp[tidx1]; // pixel_0 - width;
        const unsigned char* pixel_1 = pixel_0 + width;
        const unsigned char* pixel_2 = pixel_1 + width;

        {
            //          int x = 0;

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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

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
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p1m = t2; // pixel_1[-1];
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

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        {
            //          int x = width_2;

            int pmm = pixel_m[-1];
            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p1m = t2; // pixel_1[-1];
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

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;
        }
    }
}

int E05BFF1C_2[(1920 + 1080) * 2];
int E05C83D4_2[(1920 + 1080) * 2];
void Shrink_Grey(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width)
{
    int nRateXDesToSrc = ((src_width - 1) << 10) / (dst_width - 1);
    int nRateYDesToSrc = ((src_height - 1) << 10) / (dst_height - 1);

    //LOGE("In shrink 0 = %d, %d", dst_height, dst_width);
    int nX, nY;
    if (dst_width > 0)
    {

        int nSrcX_10 = 0;
        for (nX = 0; nX < dst_width; nX++)
        {
            int nSrcX = nSrcX_10 >> 10;
            // int nDiffX = nSrcX_10 - (nSrcX << 10);
            // int nSrcX_1 = nSrcX + 1;
            // int n1_DiffX = 0x400 - nDiffX;

            if (src_width - 1 <= nSrcX)
            {
                E05BFF1C_2[(dst_height + nX) * 2] = src_width - 1;
                //E05C83D4_2[(dst_height + nX) * 2] = 0x400;
                //E05BFF1C_2[(dst_height + nX) * 2 + 1] = src_width - 1;
                //E05C83D4_2[(dst_height + nX) * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[(dst_height + nX) * 2] = nSrcX;
                //E05C83D4_2[(dst_height + nX) * 2] = n1_DiffX;
                //E05BFF1C_2[(dst_height + nX) * 2 + 1] = nSrcX_1;
                //E05C83D4_2[(dst_height + nX) * 2 + 1] = nDiffX;
            }
            nSrcX_10 += nRateXDesToSrc;//
        } //while (R5 != dst_width);//BNE       loc_E0315FAC
    }

    //LOGE("In shrink 1");

    if (dst_height > 0)
    {
        int nSrcY_10 = 0;

        for (nY = 0; nY < dst_height; nY++)
        {
            int nSrcY = nSrcY_10 >> 10;
            // int nDiffY = nSrcY_10 - (nSrcY << 10);
            // int n1_DiffY = 0x400 - nDiffY;
            // int nSrcY_1 = nSrcY + 1;

            if (src_height - 1 <= nSrcY)
            {
                E05BFF1C_2[nY * 2] = src_height - 1;
                //E05C83D4_2[nY * 2] = 0x400;
                //E05BFF1C_2[nY * 2 + 1] = src_height - 1;
                //E05C83D4_2[nY * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[nY * 2] = nSrcY;
                //E05C83D4_2[nY * 2] = n1_DiffY;
                //E05BFF1C_2[nY * 2 + 1] = nSrcY_1;
                //E05C83D4_2[nY * 2 + 1] = nDiffY;
            }
            nSrcY_10 += nRateYDesToSrc;
        }// while (R5 != R7);//BNE      loc_E0316060
        //LOGE("In shrink 2");
        unsigned char* pDst = dst;
        for (nY = 0; nY < dst_height; nY++)
        {
            //LOGE("In shrink 4");

            int nSrcY = E05BFF1C_2[nY * 2];
            int nSrcm1Y = nSrcY - 1;
            int nSrcp1Y = nSrcY + 1;

            if (nSrcY == 0)
            {
                nSrcm1Y = 0;
            }
            if (nSrcY >= src_height)
            {
                nSrcp1Y = src_height - 1;
            }

            int nSrcYWidth = nSrcY * src_width;
            int nSrcm1YWidth = nSrcm1Y * src_width;
            int nSrcp1YWidth = nSrcp1Y * src_width;

            if (dst_width > 0)
            {
                for (nX = 0; nX < dst_width; nX++)
                {
                    int nSrcX = E05BFF1C_2[(dst_height + nX) * 2];
                    int nSrcm1X = nSrcX - 1;
                    int nSrcp1X = nSrcX + 1;

                    if (nSrcX == 0)
                    {
                        nSrcm1X = 0;
                    }
                    if (nSrcX >= src_width)
                    {
                        nSrcp1X = src_width - 1;
                    }

                    *pDst = (unsigned char)(((int)src[nSrcYWidth + nSrcX] + src[nSrcYWidth + nSrcm1X] + src[nSrcYWidth + nSrcp1X] + src[nSrcm1YWidth + nSrcX] + src[nSrcp1YWidth + nSrcX]) / 5);//STRB      R1, [R11],#1
                    pDst++;
                    //CMP       R11, R10
                } //while (R11 != R10);//BNE        loc_E0316138
            }
        } //while (R12 != R2);//BNE     loc_E03160F4

    }
}
