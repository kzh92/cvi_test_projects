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
#include <cvimodel_proc.h>

//#define SHA_LEN 20
//static unsigned char g_abEncData[SHA_LEN] = { 0 };
//unsigned char*    spoof_data = 0;
//unsigned char*    spoof_3D_data = 0;

extern int g_nStopEngine;
extern float g_rSecurityValue;

////////////////////////////////////////////////
//Liveness g_2DLiveness;
//Liveness g_3DLiveness;
Cvimodel    g_n2DLive_A1 = { 0 };
Cvimodel    g_n2DLive_A2 = { 0 };
Cvimodel    g_n2DLive_B = { 0 };
Cvimodel    g_n2DLive_B2 = { 0 };
Cvimodel    g_n3DLive = { 0 };


extern void APP_LOG(const char * format, ...);

#define KDNN_INPUTSIZE 128

int     KdnnCreateLivenessEngine_2DA1(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_A1))
            return KDNN_FAILED;

    if (/*LiveMnSE_getEngineLoaded(&g_n2DLive_A1)*/g_n2DLive_A1.m_loaded)
        return KDNN_SUCCESS;

//    int nDicSize = LiveMnSE_dnn_dic_size();
    int nDicSize = DIC_LEN_FACE_LIVE_A1;
    int ret = 0;
    //ret = LiveMnSE_dnn_create_(&g_n2DLive_A1, g_dic_live_a1, nDicSize, g_rSecurityValue, pMem);
    ret = cvimodel_init(g_dic_live_a1, nDicSize, &g_n2DLive_A1);
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

    if (/*LiveMnSE_getEngineLoaded(&g_n2DLive_A2)*/g_n2DLive_A2.m_loaded)
        return KDNN_SUCCESS;

    //int nDicSize = LiveMnSE_dnn_dic_size();
    int nDicSize = DIC_LEN_FACE_LIVE_A2;
    int ret = 0;
//    ret = LiveMnSE_dnn_create_(&g_n2DLive_A2, g_dic_live_a2, nDicSize, g_rSecurityValue, pMem);
    ret = cvimodel_init(g_dic_live_a2, nDicSize, &g_n2DLive_A2);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}

int     KdnnCreateLivenessEngine_2DB(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_B))
            return KDNN_FAILED;

    if (/*LiveMnSE_getEngineLoaded(&g_n2DLive_B)*/g_n2DLive_B.m_loaded)
        return KDNN_SUCCESS;

//    int nDicSize = LiveMnSE_dnn_dic_size();
    int nDicSize = DIC_LEN_FACE_LIVE_B;
    int ret = 0;
    //ret = LiveMnSE_dnn_create_(&g_n2DLive_B, g_dic_live_b, nDicSize, g_rSecurityValue, pMem);
    ret = cvimodel_init(g_dic_live_b, nDicSize, &g_n2DLive_B);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}


int     KdnnCreateLivenessEngine_2DB2(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_B2))
            return KDNN_FAILED;

    if (/*LiveMnSE3_getEngineLoaded(&g_n2DLive_B2)*/g_n2DLive_B2.m_loaded)
        return KDNN_SUCCESS;

//    int nDicSize = LiveMnSE3_dnn_dic_size();
    int nDicSize = DIC_LEN_FACE_LIVE_B2;
    int ret = 0;
    // ret = LiveMnSE3_dnn_create_(&g_n2DLive_B2, g_dic_live_b2, nDicSize, g_rSecurityValue, pMem);
    ret = cvimodel_init(g_dic_live_b2, nDicSize, &g_n2DLive_B2);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}


int     KdnnCreateLivenessEngine_3D(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_Liveness_C))
            return KDNN_FAILED;

    if (/*LiveMnSE_getEngineLoaded(&g_n3DLive)*/g_n3DLive.m_loaded)
        return KDNN_SUCCESS;

//    int nDicSize = LiveMnSE_dnn_dic_size();
    int nDicSize = DIC_LEN_FACE_LIVE_C;
    int ret = 0;
    //ret = LiveMnSE_dnn_create_(&g_n3DLive, g_dic_live_c, nDicSize, g_rSecurityValue, pMem);
    ret = cvimodel_init(g_dic_live_c, nDicSize, &g_n3DLive);
    if(ret)
       return KDNN_FAILED;

    return KDNN_SUCCESS;
}


float KdnnDetectLiveness2D_A(unsigned char * pbImage)
{
    if (pbImage == NULL)
        return -1;

    //float rStart = Now();
    if(/*!LiveMnSE_getEngineLoaded(&g_n2DLive_A1)*/!g_n2DLive_A1.m_loaded || /*!LiveMnSE_getEngineLoaded(&g_n2DLive_A2)*/!g_n2DLive_A2.m_loaded )
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

    //res1 = LiveMnSE_dnn_forward(&g_n2DLive_A1, pbImage);
    cvimodel_forward(&g_n2DLive_A1, pbImage, 128, 128, &res1); 

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
    //res2 = LiveMnSE_dnn_forward(&g_n2DLive_A2, pbImage);
    cvimodel_forward(&g_n2DLive_A2, pbImage, 128, 128, &res2); 
    //printf("g_n2DLive_A2 = %f\n", Now() - rStart);
    if (g_nStopEngine == 1)
    {
        return -1;
    }
    memcpy(res2_backup, res2, sizeof(float) * 2);
    //my_printf("KdnnDetectLiveness2D_A %f %f %f %f\n", res1_backup[0], res1_backup[1], res2_backup[0], res2_backup[1]);
    return (res1_backup[1] + res2_backup[1]) / 2 - (res1_backup[0] + res2_backup[0]) / 2;
}


float KdnnDetectLiveness_2D_B(unsigned char * pbImage)
{
    if(/*!LiveMnSE_getEngineLoaded(&g_n2DLive_B)*/!g_n2DLive_B.m_loaded || !getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B))
    {
        APP_LOG("[%d] 26-0-6\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    //res = LiveMnSE_dnn_forward(&g_n2DLive_B, pbImage);
    cvimodel_forward(&g_n2DLive_B, pbImage, 128, 128, &res); // ret : box, ret1 : score
    
    if (g_nStopEngine == 1)
    {
        return -1;
    }
    //my_printf("KdnnDetectLiveness_2D_B %f %f\n", res[0], res[1]);
    return res[1] - res[0];
}

float KdnnDetectLiveness_2D_B2(unsigned char * pbImage)
{
    if(/*!LiveMnSE3_getEngineLoaded(&g_n2DLive_B2)*/!g_n2DLive_B2.m_loaded || !getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B2))
    {
        APP_LOG("[%d] 26-0-62\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    //res = LiveMnSE3_dnn_forward(&g_n2DLive_B2, pbImage);
    cvimodel_forward(&g_n2DLive_B2, pbImage, 88, 128, &res); // ret : box, ret1 : score

    if (g_nStopEngine == 1)
    {
        return -1;
    }
    //my_printf("KdnnDetectLiveness_2D_B2 %f %f\n", res[0], res[1]);
    return res[1] - res[0];
}


float KdnnDetectLiveness_3D(unsigned char * pbImage)
{
    if(/*!LiveMnSE_getEngineLoaded(&g_n3DLive)*/!g_n3DLive.m_loaded || !getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_C))
    {
        APP_LOG("[%d] 26-0-7\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    //res = LiveMnSE_dnn_forward(&g_n3DLive, pbImage);
    cvimodel_forward(&g_n3DLive, pbImage, 128, 128, &res); // ret : box, ret1 : score

    if (g_nStopEngine == 1)
    {
        return -1;
    }
    //my_printf("KdnnDetectLiveness_C %f %f\n", res[0], res[1]);
    return res[1] - res[0];
}

int	KdnnFreeEngine_liveness()
{
    // LiveMnSE_dnn_free(&g_n2DLive_A1);
    // LiveMnSE_dnn_free(&g_n2DLive_A2);
    // LiveMnSE_dnn_free(&g_n2DLive_B);
    // LiveMnSE3_dnn_free(&g_n2DLive_B2);
    // LiveMnSE_dnn_free(&g_n3DLive);

    cvimodel_release(&g_n2DLive_A1);
    cvimodel_release(&g_n2DLive_A2);
    cvimodel_release(&g_n2DLive_B);
    cvimodel_release(&g_n2DLive_B2);
    cvimodel_release(&g_n3DLive);

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

