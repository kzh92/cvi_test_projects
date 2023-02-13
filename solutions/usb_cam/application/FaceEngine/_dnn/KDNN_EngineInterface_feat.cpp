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

int	KdnnCreateEngine_feat(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Feature))
            return KDNN_FAILED;

    if (feat.getEngineLoaded())
        return KDNN_SUCCESS;

    int nDicSize = feat.dnn_dic_size();
    int ret = feat.dnn_create(g_dic_feature, nDicSize, 75.0f, pMem);
    if (ret)
    {
        APP_LOG("[%d] pecc 3-9-4\n", (int)Now());
        return KDNN_FAILED;
    }

    return KDNN_SUCCESS;
}

int KdnnDetect_feat(unsigned char * pbImage, float* prFeatArray)
{
    if (pbImage == NULL || prFeatArray == NULL)
        return KDNN_FAILED;
    
    int feat_loaded = feat.getEngineLoaded();
    if (!feat_loaded)
    {
        return KDNN_FAILED;
    }

	if (g_nStopEngine == 1)
		return 0;
    float *prRet = feat.dnn_forward(pbImage, 0);

    if (g_nStopEngine == 1)
        return 0;

    memcpy(prFeatArray, prRet, sizeof(float) * KDNN_FEAT_SIZE);
	
    ENNQ::normalize(prFeatArray, KDNN_FEAT_SIZE);
    
	return KDNN_SUCCESS;
}

static float Similarity_cosine(const float* pFeat1, const float* pFeat2, int nLength)
{
    float rSum = 0.0f;
    for (int i = 0; i < nLength; i++)
        rSum += pFeat1[i] * pFeat2[i];
    float rDistance = 1 - rSum;
    if (rDistance < 0)
        rDistance = 0;
    else if (rDistance > 2)
        rDistance = 2;
    return rDistance;
}

float KdnnGetDistance(const float* imgData1, const float* imgData2)
{
    if ((imgData1 == NULL) || (imgData2 == NULL))
        return KDNN_NOVALUE;

    float rDistance = 0;
    rDistance = Similarity_cosine(imgData1, imgData2, KDNN_FEAT_SIZE);
    return rDistance;
}

float KdnnGetSimilarity(const float* imgData1, const float* imgData2)
{
    if ((imgData1 == NULL) || (imgData2 == NULL))
        return KDNN_NOVALUE;

    float rDistance = 0;
    rDistance = Similarity_cosine(imgData1, imgData2, KDNN_FEAT_SIZE);
    return (1 - rDistance / 2) * 100;
}

int	KdnnFreeEngine_feat()
{
    feat.dnn_free();
    releaseMachineDic(MachineFlagIndex_DNN_Feature);
    APP_LOG("[%d] pecc 4-9-2\n", (int)Now());
    return KDNN_SUCCESS;
}

void generateAlignImageForFeature(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, float* landmark_ptr)
{
    Align_Vertical_68(pSrcBuf, nSrcWidth, nSrcHeight, pDstBuf, KDNN_FEAT_ALIGN_W, KDNN_FEAT_ALIGN_H, 1, landmark_ptr, KDNN_FEAT_DIST_V, KDNN_FEAT_POS_X, KDNN_FEAT_POS_Y);
}

int     getFeatEngineLoaded()
{
    return feat.getEngineLoaded();
}

