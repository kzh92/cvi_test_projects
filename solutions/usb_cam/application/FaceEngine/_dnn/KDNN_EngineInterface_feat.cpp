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
#include "feat.h"
#include "hand_feat.h"
#include "ennq_normal.h"
#include "dic_manage.h"

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "common_types.h"


extern int g_nStopEngine;
extern float g_rSecurityValue;

Feature feat;
HandFeat feat_hand;

int	KdnnCreateEngine_feat(unsigned char* pMem, int nMode)
{
    if(nMode == 0)
    {
        if(!getLoadedDicFlag(MachineFlagIndex_DNN_Feature))
                return KDNN_FAILED;

        if (feat.getEngineLoaded())
            return KDNN_SUCCESS;

        int nDicSize = feat.dnn_dic_size();
        int ret = feat.dnn_create(g_dic_feature, nDicSize, 75.0f, pMem);
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

        if (HandFeat_getEngineLoaded(&feat_hand))
            return KDNN_SUCCESS;

        int nDicSize = HandFeat_dnn_dic_size();
        int ret = HandFeat_dnn_create_(&feat_hand, g_dic_feature_hand, nDicSize, 75.0f, pMem);
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
        if (!feat.getEngineLoaded())
        {
            return KDNN_FAILED;
        }
        prRet = feat.dnn_forward(pbImage, 0);
    }
    else
    {
        if (!HandFeat_getEngineLoaded(&feat_hand))
        {
            return KDNN_FAILED;
        }
        prRet = HandFeat_dnn_forward(&feat_hand, pbImage);
    }
    if (g_nStopEngine == 1)
        return 0;

    if(!prRet)
        return KDNN_FAILED;

    ENNQ::normalize(prRet, KDNN_FEAT_SIZE);
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

float Similarity_cosine_fp16(const unsigned short* nF1, const unsigned short* nF2, int n)
{
    int nn = 0;
    float rSum = 0.0f;
    // float* prSum = &rSum;

#if __ARM_NEON
    int cnt = n / 4;
    asm volatile(
        "veor       q2, q2				\n"
        "0:                             \n"
        "subs       %2, %2, #1          \n"
        "vld1.u32   {d0}, [%0]!			\n"
        "vld1.u32   {d2}, [%1]!			\n"
        "vcvt.f32.f16      q0, d0       \n"
        "vcvt.f32.f16      q1, d2       \n"
        "vmla.f32   q2, q0, q1			\n"
        "bne        0b                  \n"
        "vadd.f32	d0, d4, d5			\n"
        "vpadd.f32	d0, d0, d1			\n"
        "vst1.f32   {d0[0]}, [%6]		\n"

        : "=r"(nF1),		// %0
          "=r"(nF2),		// %1
          "=r"(cnt)			// %2
        : "0"(nF1),
          "1"(nF2),
          "2"(cnt),
          "r"(prSum)		// %6
        : "memory", "q0", "q1", "q2"
        );
    nn = n & 3;
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
        feat.dnn_free();
        releaseMachineDic(MachineFlagIndex_DNN_Feature);
    }
    else
    {
        HandFeat_dnn_free(&feat_hand);
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
        return feat.getEngineLoaded();

    }
    else
    {
        return HandFeat_getEngineLoaded(&feat_hand);
    }
}

