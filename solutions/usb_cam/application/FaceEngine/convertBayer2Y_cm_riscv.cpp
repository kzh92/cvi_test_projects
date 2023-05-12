#include <string.h>

#if __riscv_vector
#include <riscv_vector.h>
#endif

#define     BAYER_TYPE_BGGR     (0)
#define     BAYER_TYPE_RGGB     (1)

#define     BAYER_TYPE  BAYER_TYPE_BGGR
// #define      BAYER_TYPE  BAYER_TYPE_RGGB

#if BAYER_TYPE == BAYER_TYPE_BGGR
# define RGB2YCOEFF     (0x1D954C)
# define RGB2Y(r, g, b) (unsigned char)(((76 * (r) + 149 * (g) + 29 * (b) + 128) >> 8))
# define COEFF_B        (29)
# define COEFF_G        (149)
# define COEFF_R        (76)
#endif
#if BAYER_TYPE == BAYER_TYPE_RGGB
# define RGB2Y(r, g, b) (unsigned char)(((29 * (r) + 149 * (g) + 76 * (b) + 128) >> 8))
# define RGB2YCOEFF     (0x4C951D)
# define COEFF_B        (76)
# define COEFF_G        (149)
# define COEFF_R        (29)
#endif

#define     BAYER_Y_BATCH       (64)
#define     BAYER_Y_BATCH_2     (BAYER_Y_BATCH<<1)
#define     BAYER_Y_BATCH_4     (BAYER_Y_BATCH<<2)

unsigned char tb[256 * BAYER_Y_BATCH];

void convert_bayer2y_rotate_cm_riscv(const unsigned char* bayer, unsigned char* yuv, int width, int height, int iCamFlip)
{
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

#if __riscv_vector
    int n_line_offset_2 = n_line_offset * 2;
#endif

    unsigned char* pY_start = yuv + n_start_offset;

    int width_2 = width - 2;
    int height_2 = height - 2;

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

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pY += n_line_offset;
            pY[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pY[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
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

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pY += n_line_offset;
            pY[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pY[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += n_line_offset;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        {
//          int x = width_2;

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

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pY += n_line_offset;
            pY[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pY[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += n_line_offset;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }
    }

//#pragma omp parallel for num_threads(2)
    int step = BAYER_Y_BATCH_2;

    for (int y = 2; y < height_2; y += step)
    {
        if (y + step > height_2)
            step = height_2 - y;

        int b00, b01, b10, b11;
        int g00, g01, g10, g11;
        int r00, r01, r10, r11;

        unsigned char* pY = pY_start + y * n_y_delta;

        const unsigned char* pixel_m = bayer + (y - 1) * width;

        for (int s = 0; s < step; s += 2)
        {
//          int x = 0;
            const unsigned char* pixel_s = pixel_m + width * s;

            unsigned char pm0 = pixel_s[0];
            unsigned char pm1 = pixel_s[1]; pixel_s += width;
            unsigned char p00 = pixel_s[0];
            unsigned char p01 = pixel_s[1];
            unsigned char p02 = pixel_s[2]; pixel_s += width;
            unsigned char p10 = pixel_s[0];
            unsigned char p11 = pixel_s[1];
            unsigned char p12 = pixel_s[2]; pixel_s += width;
            unsigned char p20 = pixel_s[0];
            unsigned char p21 = pixel_s[1];
            unsigned char p22 = pixel_s[2];

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

            unsigned char* pYs = pY + n_y_delta * s;

            pYs[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pYs[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pYs += n_line_offset;
            pYs[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pYs[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
        }

        pY += (n_line_offset << 1);
        pixel_m += 2;

        int count = (width_2 - 2) / 2;
#if __riscv_vector

        while (count > 0)
        {
            int l = vsetvl_e8m4(count); // 64

            vuint8m4_t p0, p1, p2, p3;
            vuint16m8_t q3, q4, q5, q6, q7, q8, q9, q10, q11, q12, q13, q14, q15;
            vuint16m8_t t0, t1, t2, t4, t5, t6, t7;
            vuint16m8_t q0, q1;

            for (int s = 0; s < step; s += 2)
            {
                const unsigned char* pixel_s = pixel_m + width * s;

                if (s == 0)
                {
                    vlseg2e8_v_u8m4(&p0, &p1, pixel_s - 1, l);
                    vlseg2e8_v_u8m4(&p2, &p3, pixel_s + 1, l);

                    q6 = vwcvtu_x_x_v_u16m8(p0, l);                                             // q6 = pmm
                    q5 = vwcvtu_x_x_v_u16m8(p1, l);                                             // q5 = pm0
                    q9 = vwcvtu_x_x_v_u16m8(p2, l);                                             // q9 = pm1;
                }
                else
                {
                    q6 = t0;
                    q5 = t1;
                    q9 = t2;
                }

                pixel_s += width;

                if (s == 0)
                {
                    vlseg2e8_v_u8m4(&p0, &p1, pixel_s - 1, l);
                    vlseg2e8_v_u8m4(&p2, &p3, pixel_s + 1, l);

                    q3 = vwcvtu_x_x_v_u16m8(p0, l);                                             // q3 = p0m;                            // q3(p0m), q5(pm0), q6(pmm), q9(pm1)
                    q4 = vwcvtu_x_x_v_u16m8(p1, l);                                             // q4 = p00;                            // q3(p0m), q5(pm0), q6(pmm), q9(pm1)                                                                                           ////            q4(p00)
                    q8 = vwcvtu_x_x_v_u16m8(p2, l);                                             // q8 = p01;                            // q3(p0m), q5(pm0), q6(pmm), q9(pm1)                                                                                           ////            q8(p01)
                    q7 = vwcvtu_x_x_v_u16m8(p3, l);                                             // q7 = p02;                            // q3(p0m), q5(pm0), q6(pmm), q7(p02), q9(pm1)
                }
                else
                {
                    q3 = t4;
                    q4 = t5;
                    q8 = t6;
                    q7 = t7;
                }

                q5 = vadd_vv_u16m8(q5, q3, l);                                              // q5 = q5 + q3;                        // q5(pm0+p0m), q6(pmm), q7(p02), q9(pm1)                   // del(q3)
                q7 = vadd_vv_u16m8(q7, q4, l);                                              // q7 = q7 + q4;                        // q5(pm0+p0m), q6(pmm), q9(pm1)                                                                                                ////            q7(p00+p02)

                pixel_s += width;
                vlseg2e8_v_u8m4(&p0, &p1, pixel_s - 1, l);
                vlseg2e8_v_u8m4(&p2, &p3, pixel_s + 1, l);

                q12 = vwcvtu_x_x_v_u16m8(p0, l);                                            // q12 = p1m;                           // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)
                q11 = vwcvtu_x_x_v_u16m8(p1, l);                                            // q11 = p10;                           // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)                                                                                      ////            q11(p10)
                q15 = vwcvtu_x_x_v_u16m8(p2, l);                                            // q15 = p11;                           // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)                                                                                      ////            q15(p11)
                q14 = vwcvtu_x_x_v_u16m8(p3, l);                                            // q14 = p12;                           // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)
                t0 = q12; t1 = q11; t2 = q15;

                q3 = vadd_vv_u16m8(q11, q8, l);                                             // q3 = q11 + q8;                       // q3(p10+p01), q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)
                q6 = vadd_vv_u16m8(q6, q12, l);                                             // q6 = q6 + q12;                       // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q9(pm1), q12(p1m), q14(p12)
                q9 = vadd_vv_u16m8(q9, q15, l);                                             // q9 = q9 + q15;                       // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q12(p1m), q14(p12)                                                                    ////            q9(pm1+p11)
                q12 = vadd_vv_u16m8(q12, q15, l);                                           // q12 = q12 + q15;                 // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q14(p12)                                                                              ////            q12(p1m+p11)
                q5 = vadd_vv_u16m8(q5, q3, l);                                              // q5 = q5 + q3;                        // q3(p10+p01), q6(pmm+p1m), q14(p12)                                                                                           ////            q5(p10+p01+pm0+p0m)
                q6 = vadd_vv_u16m8(q6, q9, l);                                              // q6 = q6 + q9;                        // q3(p10+p01), q14(p12)                                                                                                        ////            q6(pmm+p1m+pm1+p11)
                q14 = vadd_vv_u16m8(q14, q3, l);                                            // q14 = q14 + q3;                      // q14(p10+p01+p12)                                         // del(q3)

                pixel_s += width;
                vlseg2e8_v_u8m4(&p0, &p1, pixel_s - 1, l);
                vlseg2e8_v_u8m4(&p2, &p3, pixel_s + 1, l);

                q10 = vwcvtu_x_x_v_u16m8(p1, l);                                            // q10 = p20;                           // q10(p20), q14(p10+p01+p12)
                q3 = vwcvtu_x_x_v_u16m8(p2, l);                                             // q3 = p21;                            // q3(p21), q10(p20), q14(p10+p01+p12)
                q13 = vwcvtu_x_x_v_u16m8(p3, l);                                            // q13 = p22;                           // q3(p21), q10(p20), q13(p22), q14(p10+p01+p12)
                t4 = vwcvtu_x_x_v_u16m8(p0, l); t5 = q10; t6 = q3; t7 = q13;

                q14 = vadd_vv_u16m8(q14, q3, l);                                            // q14 = q14 + q3;                      // q10(p20), q13(p22)                                       // del(q3)                                                          ////            q14(p10+p01+p12+p21)
                q13 = vadd_vv_u16m8(q13, q10, l);                                           // q13 = q13 + q10;                     // q10(p20), q13(p22+p20)
                q10 = vadd_vv_u16m8(q10, q4, l);                                            // q10 = q10 + q4;                      // q13(p22+p20)                                                                                                                 ////            q10(p20+p00)
                q13 = vadd_vv_u16m8(q13, q7, l);                                            // q13 = q13 + q7;                      //                                                                                                                              ////            q13(p22+p20+p00+p02)

                                                                                            // b00 = q4;
                q5 = vsrl_vx_u16m8(q5, 2, l);                                               // g00 = q5 >> 2;
                q6 = vsrl_vx_u16m8(q6, 2, l);                                               // r00 = q6 >> 2;

                q7 = vsrl_vx_u16m8(q7, 1, l);                                               // b01 = q7 >> 1;
                                                                                            // g01 = q8;
                q9 = vsrl_vx_u16m8(q9, 1, l);                                               // r01 = q9 >> 1;

                q10 = vsrl_vx_u16m8(q10, 1, l);                                             // b10 = q10 >> 1;
                                                                                            // g10 = q11;
                q12 = vsrl_vx_u16m8(q12, 1, l);                                             // r10 = q12 >> 1;

                q13 = vsrl_vx_u16m8(q13, 2, l);                                             // b11 = q13 >> 2;
                q14 = vsrl_vx_u16m8(q14, 2, l);                                             // g11 = q14 >> 2;
                                                                                            // r11 = q15;

                q1 = vmv_v_x_u16m8(128, l);

                q0 = vmacc_vx_u16m8(q1, COEFF_B, q4, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_G, q5, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_R, q6, l);
                p0 = vnsrl_wx_u8m4(q0, 8, l);

                q0 = vmacc_vx_u16m8(q1, COEFF_B, q7, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_G, q8, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_R, q9, l);
                p1 = vnsrl_wx_u8m4(q0, 8, l);

                q0 = vmacc_vx_u16m8(q1, COEFF_B, q10, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_G, q11, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_R, q12, l);
                p2 = vnsrl_wx_u8m4(q0, 8, l);

                q0 = vmacc_vx_u16m8(q1, COEFF_B, q13, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_G, q14, l);
                q0 = vmacc_vx_u16m8(q0, COEFF_R, q15, l);
                p3 = vnsrl_wx_u8m4(q0, 8, l);

                if (iCamFlip)
                {
                    vssseg2e8_v_u8m4((uint8_t*)tb + step - 2 - s, step * 2, p2, p0, l);
                    vssseg2e8_v_u8m4((uint8_t*)tb + step * 2 - 2 - s, step * 2, p3, p1, l);
                }
                else
                {
                    vssseg2e8_v_u8m4((uint8_t*)tb + step * 2 * l - step + s, - step * 2, p0, p2, l);
                    vssseg2e8_v_u8m4((uint8_t*)tb + step * 2 * l - step * 2 + s, - step * 2, p1, p3, l);
                }
            }

            if (iCamFlip)
            {

                for (int k = 0; k < 2 * l; k++)
                {
                    memcpy(pY - step + 1 + k * height, tb + k * step, step);
                }
            }
            else
            {
                for (int k = 0; k < 2 * l; k++)
                {
                    memcpy(pY - (2 * l - 1) * height + k * height, tb + k * step, step);
                }
            }

            pY += n_line_offset_2 * l;

            pixel_m += (l * 2);

            count -= l;
        }
#else // __riscv_vector
        int remain;
        for (remain = count; remain > 0; remain--)
        {
            for (int s = 0; s < step; s += 2)
            {
                const unsigned char* pixel_s = pixel_m + width * s;

                unsigned char pmm = pixel_s[-1];
                unsigned char pm0 = pixel_s[0];
                unsigned char pm1 = pixel_s[1]; pixel_s += width;
                unsigned char p0m = pixel_s[-1];
                unsigned char p00 = pixel_s[0];
                unsigned char p01 = pixel_s[1];
                unsigned char p02 = pixel_s[2]; pixel_s += width;
                unsigned char p1m = pixel_s[-1];
                unsigned char p10 = pixel_s[0];
                unsigned char p11 = pixel_s[1];
                unsigned char p12 = pixel_s[2]; pixel_s += width;
                unsigned char p20 = pixel_s[0];
                unsigned char p21 = pixel_s[1];
                unsigned char p22 = pixel_s[2];

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

                unsigned char* pYs = pY + n_y_delta * s;
                pYs[0] = RGB2Y(r00, g00, b00);   // pY[0]
                pYs[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
                pYs += n_line_offset;
                pYs[0] = RGB2Y(r01, g01, b01);   // pY[1]
                pYs[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
            }
            pY += (n_line_offset << 1);
            pixel_m += 2;
        }

#endif // __riscv_vector

        for (int s = 0; s < step; s += 2)
        {
//          int x = width_2;
            const unsigned char* pixel_s = pixel_m + width * s;

            int pmm = pixel_s[-1];
            int pm0 = pixel_s[0];
            int pm1 = pixel_s[1]; pixel_s += width;
            int p0m = pixel_s[-1];
            int p00 = pixel_s[0];
            int p01 = pixel_s[1]; pixel_s += width;
            int p1m = pixel_s[-1];
            int p10 = pixel_s[0];
            int p11 = pixel_s[1]; pixel_s += width;
            int p20 = pixel_s[0];
            int p21 = pixel_s[1];

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

            unsigned char* pYs = pY + n_y_delta * s;
            pYs[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pYs[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pYs += n_line_offset;
            pYs[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pYs[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
        }

        pY += (n_line_offset << 1);
        pixel_m += 2;
    }

    {
//      int y = height_2;

        int b00, b01, b10, b11;
        int g00, g01, g10, g11;
        int r00, r01, r10, r11;

        unsigned char* pY = pY_start + height_2 * n_y_delta;

        const unsigned char* pixel_0 = bayer + height_2 * width;
        const unsigned char* pixel_m = pixel_0 - width;
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

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pY += n_line_offset;
            pY[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pY[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
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

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pY += n_line_offset;
            pY[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pY[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += n_line_offset;

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

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[n_y_delta] = RGB2Y(r10, g10, b10);   // pY_[0]
            pY += n_line_offset;
            pY[0] = RGB2Y(r01, g01, b01);   // pY[1]
            pY[n_y_delta] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += n_line_offset;
        }
    }
}
