//
//  UltraFace.cpp
//  UltraFaceTest
//
//  Created by vealocia on 2019/10/17.
//  Copyright © 2019 vealocia. All rights reserved.
//


#include <math.h>

#include "modeling_interface.h"
#include "modeling.h"
#include <memory.h>
#include "DBManager.h"
#include "engine_inner_param.h"
#include "HAlign.h"
#include "dic_manage.h"
#include "common_types.h"

Modeling g_Modeling;

extern void APP_LOG(const char * format, ...);
extern int g_nStopEngine;

int createModelingEngine(unsigned char* pMem)
{
    if(g_Modeling.getEngineLoaded())
    {
        return 0;
    }
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Modeling))
    {
        return 1;
    }

    int nRet = 0;

    int nDicSize = g_Modeling.dnn_dic_size();
    nRet = g_Modeling.dnn_create(g_dic_model, nDicSize, pMem);
    if(nRet)
    {
        return nRet;
    }
    return 0;
}


int releaseModelingEngine()
{
    g_Modeling.dnn_free();
    releaseMachineDic(MachineFlagIndex_DNN_Modeling);

    return 0;
}

int getFaceModelPoint(unsigned char* pImageBuffer, int nImageWidth, int nImageHeight, unsigned char* tempCropBuffer, float* prFaceRect, float* prLandmarkPoint)
{
    if(!g_Modeling.getEngineLoaded() || !getDicChecSumChecked(MachineFlagIndex_DNN_Modeling))
    {
        return 1;
    }

    if (g_nStopEngine == 1)
    {
        return 1;
    }

    float rX, rY, rW, rH;
    //float aLeft, aTop, aWidth, aHeight;
    rX = prFaceRect[0];
    rY = prFaceRect[1];
    rW = prFaceRect[2];
    rH = prFaceRect[3];

    float rLeftMargin = 0.05f;
    float rTopMargin = 0.00f;
    float rRightMargin = 0.05f;
    float rBottomMargin = 0.1f;

    int iExFaceX = (int)(rX - rLeftMargin * rW);
    int iExFaceY = (int)(rY - rTopMargin * rH);
    int iExFaceW = (int)((1 + (rLeftMargin + rRightMargin)) * rW);
    int	iExFaceH = (int)((1 + (rTopMargin + rBottomMargin)) * rH);

    iExFaceX = iExFaceX > 0 ? (iExFaceX < nImageWidth ? iExFaceX : nImageWidth - 1) : 0;
    iExFaceY = iExFaceY > 0 ? (iExFaceY < nImageHeight ? iExFaceY : nImageHeight - 1) : 0;
    iExFaceW = (iExFaceX + iExFaceW) < nImageWidth ? iExFaceW : (nImageWidth - iExFaceX - 1);
    iExFaceH = (iExFaceY + iExFaceH) < nImageHeight ? iExFaceH : (nImageHeight - iExFaceY - 1);

    alignForModeling(pImageBuffer, nImageWidth, nImageHeight, iExFaceX, iExFaceY, iExFaceW, iExFaceH, tempCropBuffer, g_DNN_Modeling_input_width, g_DNN_Modeling_input_height);

    if (g_nStopEngine == 1)
    {
        return 1;
    }

    float *pTemp = g_Modeling.dnn_forward(tempCropBuffer, g_DNN_Modeling_input_width, g_DNN_Modeling_input_height);
    if (g_nStopEngine == 1)
    {
        return 1;
    }

    int nPointIndex;
    for(nPointIndex = 0; nPointIndex < 68; nPointIndex ++)
    {
        prLandmarkPoint[nPointIndex * 2] = pTemp[nPointIndex * 2] * iExFaceW + iExFaceX;
        prLandmarkPoint[nPointIndex * 2 + 1] = pTemp[nPointIndex * 2 + 1] * iExFaceH + iExFaceY;
    }

    return 0;
}

