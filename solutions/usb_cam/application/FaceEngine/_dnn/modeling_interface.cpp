//
//  UltraFace.cpp
//  UltraFaceTest
//
//  Created by vealocia on 2019/10/17.
//  Copyright © 2019 vealocia. All rights reserved.
//


#include <math.h>

#include "modeling_interface.h"
//#include "modeling.h"
#include <memory.h>
#include "DBManager.h"
#include "engine_inner_param.h"
#include "HAlign.h"
#include "dic_manage.h"
#include "common_types.h"

#include <cvimodel_proc.h>

#include <vfs.h>
#include <sys/fcntl.h>

Cvimodel g_Modeling = { 0 };
Cvimodel g_Modeling_Hand = { 0 };


extern void APP_LOG(const char * format, ...);
extern int g_nStopEngine;

int createModelingEngine(unsigned char* pMem, int nMode)
{
    Cvimodel* p_Modeling = &g_Modeling;
    int nModuleID = MachineFlagIndex_DNN_Modeling;
    unsigned char* p_dic_modeling = g_dic_model;
    int nDicSize = DIC_LEN_FACE_MODELING;

    if(nMode)
    {
//        Modeling_Modeling(&g_Modeling_Hand, 1);
        p_Modeling = &g_Modeling_Hand;
        nModuleID = MachineFlagIndex_DNN_Modeling_Hand;
        p_dic_modeling = g_dic_model_hand;
        nDicSize = DIC_LEN_HAND_MODELING;
    }


    if(p_Modeling->m_loaded/*Modeling_getEngineLoaded(p_Modeling)*/)
    {
        return 0;
    }
    if(!getLoadedDicFlag(nModuleID))
    {
        return 1;
    }

    int nRet = 0;

    nRet = cvimodel_init(p_dic_modeling, nDicSize, p_Modeling);
    //nRet = Modeling_dnn_create_(p_Modeling, p_dic_modeling, nDicSize, pMem);
    if(nRet)
    {
        return nRet;
    }
    return 0;
}


int releaseModelingEngine(int nMode)
{
    if(nMode == 0)
    {
        cvimodel_release(&g_Modeling);
        releaseMachineDic(MachineFlagIndex_DNN_Modeling);
    }
    else
    {
        cvimodel_release(&g_Modeling_Hand);
        releaseMachineDic(MachineFlagIndex_DNN_Modeling_Hand);
    }

    return 0;
}

int getFaceModelPoint(unsigned char* pImageBuffer, int nImageWidth, int nImageHeight, unsigned char* tempCropBuffer, float* prFaceRect, float* prLandmarkPoint)
{
    if(/*!Modeling_getEngineLoaded(&g_Modeling)*/!g_Modeling.m_loaded || !getDicChecSumChecked(MachineFlagIndex_DNN_Modeling))
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
    float rBottomMargin = 0.10f;

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
    
    float *pTemp = 0;
    cvimodel_forward(&g_Modeling, tempCropBuffer, g_DNN_Modeling_input_width, g_DNN_Modeling_input_height, &pTemp); // ret : box, ret1 : score
    //float *pTemp = Modeling_dnn_forward(&g_Modeling, tempCropBuffer, g_DNN_Modeling_input_width, g_DNN_Modeling_input_height);
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

int getHandModelPoint(unsigned char* pImageBuffer, int nImageWidth, int nImageHeight, unsigned char* tempCropBuffer, float* prHandRect, float* prHandLandmarkPoint)
{
    if(/*!Modeling_getEngineLoaded(&g_Modeling_Hand)*/!g_Modeling_Hand.m_loaded || !getDicChecSumChecked(MachineFlagIndex_DNN_Modeling_Hand))
    {
        return 1;
    }

    if (g_nStopEngine == 1)
    {
        return 1;
    }

    float rX, rY, rW, rH;
    //float aLeft, aTop, aWidth, aHeight;
    rX = prHandRect[0];
    rY = prHandRect[1];
    rW = prHandRect[2];
    rH = prHandRect[3];

    float cx = rX + rW / 2;
    float cy = rY + rH / 2;
    float sz = std::max(rW, rH);

    int sz_crop = (int)(sz * 1.4);

    int x1 = int(cx - sz_crop / 2);
    int y1 = int(cy - sz_crop / 2);

    x1 = x1 > 0 ? (x1 < nImageWidth ? x1 : nImageWidth - 1) : 0;
    y1 = y1 > 0 ? (y1 < nImageHeight ? y1 : nImageHeight - 1) : 0;

    int xw = (x1 + sz_crop) < nImageWidth ? sz_crop : (nImageWidth - x1 - 1);
    int yh = (y1 + sz_crop) < nImageHeight ? sz_crop : (nImageHeight - y1 - 1);

    alignForModeling(pImageBuffer, nImageWidth, nImageHeight, x1, y1, xw, yh, tempCropBuffer, 64, 64);

    // float *pTemp = Modeling_dnn_forward(&g_Modeling_Hand, tempCropBuffer, 64, 64);
    float *pTemp = 0;
    cvimodel_forward(&g_Modeling_Hand, tempCropBuffer, 64, 64, &pTemp); // ret : box, ret1 : score

#if 0
    {
        char szImageFilePath[255];
        sprintf(szImageFilePath, "/mnt/sd/model_buf.bin");
        int fd1 = aos_open(szImageFilePath, O_CREAT | O_RDWR);
        if(fd1 >= 0)
        {

            aos_write(fd1, tempCropBuffer, 64 * 64);
            aos_sync(fd1);
            aos_close(fd1);
            APP_LOG("%s saved\n", szImageFilePath);
        }
        else
        {
            APP_LOG("%s not saved\n", szImageFilePath);
        }
    }
#endif
#if 0
    {
        int nIndex;
        my_printf("points\n");
        for(nIndex = 0; nIndex < 7; nIndex ++)
        {
            my_printf("%f, %f\n", pTemp[nIndex * 2], pTemp[nIndex * 2 + 1]);
        }
    }
#endif
    int nPointIndex;
    for (nPointIndex = 0; nPointIndex < 7; nPointIndex++)
    {
        prHandLandmarkPoint[nPointIndex * 2] = pTemp[nPointIndex * 2] * xw + x1;
        prHandLandmarkPoint[nPointIndex * 2 + 1] = pTemp[nPointIndex * 2 + 1] * yh + y1;
    }
    return 0;
}

