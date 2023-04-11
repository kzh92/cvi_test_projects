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
#include "KDNN_handValid.h"
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


//#define SHA_LEN 20
//static unsigned char g_abEncData[SHA_LEN] = { 0 };
//unsigned char*    spoof_data = 0;
//unsigned char*    spoof_3D_data = 0;

extern int g_nStopEngine;
extern float g_rSecurityValue;

////////////////////////////////////////////////
//Liveness g_2DLiveness;
//Liveness g_3DLiveness;
LiveMnSE    g_nCheckValid_Hand = { 0 };

extern void APP_LOG(const char * format, ...);

#define KDNN_INPUTSIZE 128

int     KdnnCreateCheckValid_Hand(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_CheckValid_Hand))
            return KDNN_FAILED;

    if (LiveMnSE_getEngineLoaded(&g_nCheckValid_Hand))
        return KDNN_SUCCESS;

    int nDicSize = LiveMnSE_dnn_dic_size();
    int ret = 0;
    ret = LiveMnSE_dnn_create_(&g_nCheckValid_Hand, g_dic_checkValid_hand, nDicSize, g_rSecurityValue, pMem);
    if(ret)
    {
        return KDNN_FAILED;
    }
    return KDNN_SUCCESS;
}


float KdnnCheckValid_Hand(unsigned char * pbImage)
{
    if(!LiveMnSE_getEngineLoaded(&g_nCheckValid_Hand) || !getDicChecSumChecked(MachineFlagIndex_DNN_CheckValid_Hand))
    {
        APP_LOG("[%d] 26-6-6\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    res = LiveMnSE_dnn_forward(&g_nCheckValid_Hand, pbImage);
    if (g_nStopEngine == 1)
    {
        return -1;
    }
    return res[1] - res[0];
}

int	KdnnFreeCheckValid_Hand()
{
    LiveMnSE_dnn_free(&g_nCheckValid_Hand);

    releaseMachineDic(MachineFlagIndex_DNN_CheckValid_Hand);
    return KDNN_SUCCESS;
}

