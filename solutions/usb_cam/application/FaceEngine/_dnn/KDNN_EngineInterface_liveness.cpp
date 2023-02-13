/************************************************************************
*   KDNNFR : KCC-Single dnn model, for detect liveness SDK						*
************************************************************************/
/*
*  KDNNFR
*  Copyright 2015-2017 by  KCC
*  All rights reserved.
*  PyongYang, DPR of Korea
*/

/*!
* \file  KDNN_EngineInterface_liveness.cpp
* \brief
* \author Li CholMin, An YunChol
*/
#include "KDNN_EngineInterface.h"
#include <memory.h>
#include <time.h>

#include "livemn.h"
#include "livemnse.h"
#include "livemnse3.h"
#include "engine_inner_param.h"

#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include "DBManager.h"
#include "HAlign.h"
#include <stdio.h>
#include "dic_manage.h"
#include "common_types.h"


//#define SHA_LEN 20
//static unsigned char g_abEncData[SHA_LEN] = { 0 };
//unsigned char*    spoof_data = 0;
//unsigned char*    spoof_3D_data = 0;

extern int g_nStopEngine;
extern float g_rSecurityValue;

////////////////////////////////////////////////
//Liveness g_2DLiveness;
//Liveness g_3DLiveness;
LiveMnSE    g_n2DLive_A1;
LiveMnSE    g_n2DLive_A2;
LiveMnSE    g_n2DLive_B;
LiveMnSE3   g_n2DLive_B2;
LiveMn      g_n3DLive;

extern void APP_LOG(const char * format, ...);

#define KDNN_INPUTSIZE 128

int     KdnnCreateLivenessEngine_2DA1(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_A1))
            return KDNN_FAILED;

    if (g_n2DLive_A1.getEngineLoaded())
        return KDNN_SUCCESS;

    int nDicSize = g_n2DLive_A1.dnn_dic_size();
    int ret = 0;
    ret = g_n2DLive_A1.dnn_create(g_dic_live_a1, nDicSize, g_rSecurityValue, pMem);
    if(ret)
    {
        return KDNN_FAILED;
    }
    return KDNN_SUCCESS;
}

int     KdnnCreateLivenessEngine_2DA2(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_A2))
            return KDNN_FAILED;

    if (g_n2DLive_A2.getEngineLoaded())
        return KDNN_SUCCESS;

    int nDicSize = g_n2DLive_A2.dnn_dic_size();
    int ret = 0;
    ret = g_n2DLive_A2.dnn_create(g_dic_live_a2, nDicSize, g_rSecurityValue, pMem);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}

int     KdnnCreateLivenessEngine_2DB(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_B))
            return KDNN_FAILED;

    if (g_n2DLive_B.getEngineLoaded())
        return KDNN_SUCCESS;

    int nDicSize = g_n2DLive_B.dnn_dic_size();
    int ret = 0;
    ret = g_n2DLive_B.dnn_create(g_dic_live_b, nDicSize, g_rSecurityValue, pMem);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}


int     KdnnCreateLivenessEngine_2DB2(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_B2))
            return KDNN_FAILED;

    if (g_n2DLive_B2.getEngineLoaded())
        return KDNN_SUCCESS;

    int nDicSize = g_n2DLive_B2.dnn_dic_size();
    int ret = 0;
    ret = g_n2DLive_B2.dnn_create(g_dic_live_b2, nDicSize, g_rSecurityValue, pMem);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}


int     KdnnCreateLivenessEngine_3D(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_C))
            return KDNN_FAILED;

    if (g_n3DLive.getEngineLoaded())
        return KDNN_SUCCESS;

    int nDicSize = g_n3DLive.dnn_dic_size();
    int ret = 0;
    ret = g_n3DLive.dnn_create(g_dic_live_c, nDicSize, g_rSecurityValue, pMem);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}


float KdnnDetectLiveness2D_A(unsigned char * pbImage)
{
    if (pbImage == NULL)
        return -1;

    //float rStart = Now();
    if(!g_n2DLive_A1.getEngineLoaded() || !g_n2DLive_A2.getEngineLoaded() )
    {
        APP_LOG("[%d] 26-0-3\n", (int)Now());
        return -1;
    }

    if(!getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_A1))
    {
        APP_LOG("[%d] 26-0-4\n", (int)Now());
        return -1;
    }

    //printf("g_n2DLive_A checksum = %f\n", Now() - rStart);
    //rStart = Now();
    float *res1, *res2;
    float res1_backup[2], res2_backup[2];

    res1 = g_n2DLive_A1.dnn_forward(pbImage);
    //printf("g_n2DLive_A1 = %f\n", Now() - rStart);
    //rStart = Now();
    if (g_nStopEngine == 1)
    {
        return -1;
    }
    memcpy(res1_backup, res1, sizeof(float) * 2);
    if(!getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_A2))
    {
        APP_LOG("[%d] 26-0-5\n", (int)Now());
        return -1;
    }
    res2 = g_n2DLive_A2.dnn_forward(pbImage);
    //printf("g_n2DLive_A2 = %f\n", Now() - rStart);
    if (g_nStopEngine == 1)
    {
        return -1;
    }
    memcpy(res2_backup, res2, sizeof(float) * 2);

    return (res1_backup[1] + res2_backup[1]) / 2 - (res1_backup[0] + res2_backup[0]) / 2;
}


float KdnnDetectLiveness_2D_B(unsigned char * pbImage)
{
    if(!g_n2DLive_B.getEngineLoaded() || !getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B))
    {
        APP_LOG("[%d] 26-0-6\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    res = g_n2DLive_B.dnn_forward(pbImage);
    if (g_nStopEngine == 1)
    {
        return -1;
    }
    return res[1] - res[0];
}

float KdnnDetectLiveness_2D_B2(unsigned char * pbImage)
{
    if(!g_n2DLive_B2.getEngineLoaded() || !getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B2))
    {
        APP_LOG("[%d] 26-0-62\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    res = g_n2DLive_B2.dnn_forward(pbImage);
    if (g_nStopEngine == 1)
    {
        return -1;
    }
    return res[1] - res[0];
}


float KdnnDetectLiveness_3D(unsigned char * pbImage)
{
    if(!g_n3DLive.getEngineLoaded() || !getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_C))
    {
        APP_LOG("[%d] 26-0-7\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    res = g_n3DLive.dnn_forward(pbImage);
    if (g_nStopEngine == 1)
    {
        return -1;
    }

    return res[1] - res[0];
}

int	KdnnFreeEngine_liveness()
{
    g_n2DLive_A1.dnn_free();
    g_n2DLive_A2.dnn_free();
    g_n2DLive_B.dnn_free();
    g_n2DLive_B2.dnn_free();
    g_n3DLive.dnn_free();

    releaseMachineDic(MachineFlagIndex_DNN_Liveness_A1);
    releaseMachineDic(MachineFlagIndex_DNN_Liveness_A2);
    releaseMachineDic(MachineFlagIndex_DNN_Liveness_B);
    releaseMachineDic(MachineFlagIndex_DNN_Liveness_B2);
    releaseMachineDic(MachineFlagIndex_DNN_Liveness_C);
    return KDNN_SUCCESS;
}

void generateAlignImageForLiveness(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBufAC, unsigned char* pDstBufB, unsigned char* pDstBufB2, float* landmark_ptr)
{
    Align_Vertical_68(pSrcBuf, nSrcWidth, nSrcHeight, pDstBufAC, 128, 128, 1, landmark_ptr, 64, 64, 30);
    Align_Vertical_68(pSrcBuf, nSrcWidth, nSrcHeight, pDstBufB, 128, 128, 1, landmark_ptr,  12, 64, 46);
    Align_Vertical_68(pSrcBuf, nSrcWidth, nSrcHeight, pDstBufB2, 88, 128, 1, landmark_ptr,  14, 44, 44);
}

