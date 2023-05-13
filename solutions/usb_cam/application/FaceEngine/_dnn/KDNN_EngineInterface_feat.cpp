/************************************************************************
*   KDNNFR : KCC-Single dnn model, for detect liveness SDK						*
************************************************************************/
/*
*  KDNNFR
*  Copyright 2015-2017 by  KCC
*  All rights reserved.
*/

/*!
* \file  KDNN_EngineInterface_liveness.cpp
* \brief
* \author Li CholMin, An YunChol
*/
#include "KDNN_EngineInterface.h"
#include "engine_inner_param.h"
#include "FaceRetrievalSystem_base.h"
// #include "feat.h"
// #include "hand_feat.h"
// #include "ennq_normal.h"
#include "dic_manage.h"

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "common_types.h"
#include <cvimodel_proc.h>

#if __riscv_vector
#include <riscv_vector.h>
#endif

extern int g_nStopEngine;
extern float g_rSecurityValue;


Cvimodel feat;
Cvimodel feat_hand;

#define _NEON_FP16

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
#elif __riscv_vector
    typedef union _u_fp16_tag
    {
        unsigned short u;
        float16_t f;
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
#elif __riscv_vector
    typedef union _u_fp16_tag
    {
        unsigned short u;
        float16_t f;
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

float Similarity_cosine_fp16(const unsigned short* nF1, const unsigned short* nF2, int n)
{
    int nn = 0;
    float rSum = 0.0f;

#if __ARM_NEON
    float* prSum = &rSum;
    int cnt = n / 4;
    asm volatile(
        "veor       q2, q2              \n"
        "0:                             \n"
        "subs       %2, %2, #1          \n"
        "vld1.u32   {d0}, [%0]!         \n"
        "vld1.u32   {d2}, [%1]!         \n"
        "vcvt.f32.f16      q0, d0       \n"
        "vcvt.f32.f16      q1, d2       \n"
        "vmla.f32   q2, q0, q1          \n"
        "bne        0b                  \n"
        "vadd.f32   d0, d4, d5          \n"
        "vpadd.f32  d0, d0, d1          \n"
        "vst1.f32   {d0[0]}, [%6]       \n"

        : "=r"(nF1),        // %0
          "=r"(nF2),        // %1
          "=r"(cnt)         // %2
        : "0"(nF1),
          "1"(nF2),
          "2"(cnt),
          "r"(prSum)        // %6
        : "memory", "q0", "q1", "q2"
        );
    nn = n & 3;
#elif __riscv_vector
    float rF32[256];
    float32_t *prF32 = rF32;
    nn = n;
    while (nn > 0)
    {
        int l = vsetvl_e16m4(nn);
        vfloat16m4_t va = vle16_v_f16m4((float16_t*)nF1, l);
        vfloat16m4_t vb = vle16_v_f16m4((float16_t*)nF2, l);
        vfloat32m8_t va32 = vfwcvt_f_f_v_f32m8(va, l);
        vfloat32m8_t vb32 = vfwcvt_f_f_v_f32m8(vb, l);
        vfloat32m8_t vc32 = vfmul_vv_f32m8(va32, vb32, l);
        vse32_v_f32m8(prF32, vc32, l);

        nn -= l;
        nF1 += l;
        nF2 += l;
        prF32 += l;
    }
    for (int i = 0; i < n; i++)
    {
        rSum += rF32[i];
    }
    nn = 0;
#else
    nn = n;
#endif
    int i;
    for (i = nn; i > 0; i--)
    {
        rSum += _gnu_h2f_internal_f(*nF1) * _gnu_h2f_internal_f(*nF2);
        nF1++;
        nF2++;
    }

    float rDistance = 1 - rSum;
    if (rDistance < 0)
        rDistance = 0;
    else if (rDistance > 2)
        rDistance = 2;
    return rDistance;
}

void _gnu_h2f_internal_vector(const unsigned short* src, float* dst, int n)
{
    int nn = 0;
#if __ARM_NEON
    int cnt = n / 4;
    asm volatile(
        "0:                             \n"
        "vld1.u32   {d0}, [%0]!         \n"
        "subs       %2, %2, #1          \n"
        "vcvt.f32.f16      q0, d0       \n"
        "vst1.u32   {d0-d1}, [%1]!      \n"
        "bne        0b                  \n"

        : "=r"(src),    // %0
        "=r"(dst),      // %1
        "=r"(cnt)       // %2
        : "0"(src),
        "1"(dst),
        "2"(cnt)
        : "memory", "q0"
        );
    nn = n & 3;
#elif __riscv_vector
    nn = n;
    while (nn > 0)
    {
        int l = vsetvl_e16m4(nn);
        vfloat16m4_t v = vle16_v_f16m4((float16_t*)src, l);
        vfloat32m8_t v32 = vfwcvt_f_f_v_f32m8(v, l);
        vse32_v_f32m8((float32_t*)dst, v32, l);

        nn -= l;
        src += l;
        dst += l;
    }
    nn = 0;
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
        "vld1.u16   {d0-d1}, [%0]!      \n"
        "subs       %2, %2, #1          \n"
        "vcvt.f16.f32      d0, q0       \n"
        "vst1.u32   {d0}, [%1]!         \n"
        "bne        0b                  \n"

        : "=r"(src),    // %0
        "=r"(dst),      // %1
        "=r"(cnt)       // %2
        : "0"(src),
        "1"(dst),
        "2"(cnt)
        : "memory", "q0"
        );
    nn = n & 3;
#elif __riscv_vector
    nn = n;
    while (nn > 0)
    {
        int l = vsetvl_e32m8(nn);
        vfloat32m8_t v32 = vle32_v_f32m8((float32_t*)src, l);
        vfloat16m4_t v = vfncvt_f_f_w_f16m4(v32, l);
        vse16_v_f16m4((float16_t*)dst, v, l);

        nn -= l;
        src += l;
        dst += l;
    }
    nn = 0;
#else
    nn = n;
#endif
    int i;
    for (i = 0; i < nn; i++)
    {
        dst[i] = _gnu_f2h_internal_f(src[i]);
    }
}

void normalize_(float* feat, int sz)
{
        int i;
        float sum = 0.0f;
        for (i = 0; i < sz; i++)
        {
            sum += feat[i] * feat[i];
        }

        sum = sqrtf(sum);
        for (i = 0; i < sz; i++)
        {
            feat[i] = feat[i] / sum;
        }
}

int	KdnnCreateEngine_feat(unsigned char* pMem, int nMode)
{
    if(nMode == 0)
    {
        if(!getLoadedDicFlag(MachineFlagIndex_DNN_Feature))
                return KDNN_FAILED;

        if (/*feat.getEngineLoaded()*/feat.m_loaded)
            return KDNN_SUCCESS;

        //int nDicSize = feat.dnn_dic_size();
        //int ret = feat.dnn_create(g_dic_feature, nDicSize, 75.0f, pMem);
        int nDicSize = DIC_LEN_FACE_FEATURE;
        int ret = cvimodel_init(g_dic_feature, nDicSize, &feat);

        if (ret)
        {
            APP_LOG("[%d] pecc 3-9-4-%d\n", (int)Now(), nMode);
            return KDNN_FAILED;
        }
    }
    else
    {
        if(!getLoadedDicFlag(MachineFlagIndex_DNN_Feature_Hand))
                return KDNN_FAILED;

        if (/*HandFeat_getEngineLoaded(&feat_hand)*/feat_hand.m_loaded)
            return KDNN_SUCCESS;

        int nDicSize = DIC_LEN_HAND_FEATURE;
        //int ret = HandFeat_dnn_create_(&feat_hand, g_dic_feature_hand, nDicSize, 75.0f, pMem);
        int ret = cvimodel_init(g_dic_feature_hand, nDicSize, &feat_hand);
        if (ret)
        {
            APP_LOG("[%d] pecc 3-9-4-%d\n", (int)Now(), nMode);
            return KDNN_FAILED;
        }
    }

    return KDNN_SUCCESS;
}

int KdnnDetect_feat(unsigned char * pbImage, unsigned short* prFeatArray, int nMode)
{
    if (pbImage == NULL || prFeatArray == NULL)
        return KDNN_FAILED;

    if (g_nStopEngine == 1)
        return 0;

    float *prRet = 0;
    if(nMode == 0)
    {
        if (/*!feat.getEngineLoaded()*/!feat.m_loaded)
        {
            return KDNN_FAILED;
        }
        //prRet = feat.dnn_forward(pbImage, 0);
        cvimodel_forward(&feat, pbImage, KDNN_FEAT_ALIGN_W, KDNN_FEAT_ALIGN_H, &prRet); 
    }
    else
    {
        if (/*!HandFeat_getEngineLoaded(&feat_hand)*/!feat_hand.m_loaded)
        {
            return KDNN_FAILED;
        }
        //prRet = HandFeat_dnn_forward(&feat_hand, pbImage);
        cvimodel_forward(&feat_hand, pbImage, 128, 128, &prRet); 
    }
    if (g_nStopEngine == 1)
        return 0;

    if(!prRet)
        return KDNN_FAILED;

    normalize_(prRet, KDNN_FEAT_SIZE);
    _gnu_f2h_internal_vector(prRet, prFeatArray, KDNN_FEAT_SIZE);
    
	return KDNN_SUCCESS;
}

// static float Similarity_cosine(const float* pFeat1, const float* pFeat2, int nLength)
// {
//     float rSum = 0.0f;
//     for (int i = 0; i < nLength; i++)
//         rSum += pFeat1[i] * pFeat2[i];
//     float rDistance = 1 - rSum;
//     if (rDistance < 0)
//         rDistance = 0;
//     else if (rDistance > 2)
//         rDistance = 2;
//     return rDistance;
// }


float KdnnGetDistance(const unsigned short* imgData1, const unsigned short* imgData2)
{
    if ((imgData1 == NULL) || (imgData2 == NULL))
        return KDNN_NOVALUE;

    float rDistance = 0;
    rDistance = Similarity_cosine_fp16(imgData1, imgData2, KDNN_FEAT_SIZE);
    return rDistance;
}

float KdnnGetSimilarity(const unsigned short* imgData1, const unsigned short* imgData2)
{
    if ((imgData1 == NULL) || (imgData2 == NULL))
        return KDNN_NOVALUE;

    float rDistance = 0;
    // rDistance = Similarity_cosine(imgData1, imgData2, KDNN_FEAT_SIZE);
    rDistance = Similarity_cosine_fp16(imgData1, imgData2, KDNN_FEAT_SIZE);
    return (1 - rDistance / 2) * 100;
}

int	KdnnFreeEngine_feat(int nMode)
{
    if(nMode == 0)
    {
        //feat.dnn_free();
        cvimodel_release(&feat);
        releaseMachineDic(MachineFlagIndex_DNN_Feature);
    }
    else
    {
        //HandFeat_dnn_free(&feat_hand);
        cvimodel_release(&feat_hand);
        releaseMachineDic(MachineFlagIndex_DNN_Feature_Hand);
    }
    APP_LOG("[%d] pecc 4-9-2-%d\n", (int)Now(), nMode);
    return KDNN_SUCCESS;
}

void generateAlignImageForFeature(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, float* landmark_ptr)
{
    Align_Vertical_68(pSrcBuf, nSrcWidth, nSrcHeight, pDstBuf, KDNN_FEAT_ALIGN_W, KDNN_FEAT_ALIGN_H, 1, landmark_ptr, KDNN_FEAT_DIST_V, KDNN_FEAT_POS_X, KDNN_FEAT_POS_Y);
}

int     getFeatEngineLoaded(int nMode)
{
    if(nMode == 0)
    {
        //return feat.getEngineLoaded();
        return feat.m_loaded;
    }
    else
    {
        //return HandFeat_getEngineLoaded(&feat_hand);
        return feat_hand.m_loaded;

    }
}

