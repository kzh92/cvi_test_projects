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

// #include "livemnse.h"
// #include "livemnse3.h"
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

#include <vfs.h>
#include <sys/fcntl.h>


//#define SHA_LEN 20
//static unsigned char g_abEncData[SHA_LEN] = { 0 };
//unsigned char*    spoof_data = 0;
//unsigned char*    spoof_3D_data = 0;

extern int g_nStopEngine;
extern float g_rSecurityValue;

////////////////////////////////////////////////
//Liveness g_2DLiveness;
//Liveness g_3DLiveness;
Cvimodel    g_nCheckValid_Hand = { 0 };

extern void APP_LOG(const char * format, ...);

#define KDNN_INPUTSIZE 128

int     KdnnCreateCheckValid_Hand(unsigned char* pMem)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_CheckValid_Hand))
            return KDNN_FAILED;

    if (g_nCheckValid_Hand.m_loaded)
        return KDNN_SUCCESS;

    if(!getDicChecSumChecked(MachineFlagIndex_DNN_CheckValid_Hand))
    {
        return KDNN_FAILED;
    }

    int nDicSize = DIC_LEN_HAND_CHECK;
    int ret = 0;
    ret = cvimodel_init(g_dic_checkValid_hand, nDicSize, &g_nCheckValid_Hand);
    if(ret)
    {
        return KDNN_FAILED;
    }
    return KDNN_SUCCESS;
}


float KdnnCheckValid_Hand(unsigned char * pbImage)
{
    if(!g_nCheckValid_Hand.m_loaded || !getDicChecSumChecked(MachineFlagIndex_DNN_CheckValid_Hand))
    {
        APP_LOG("[%d] 26-6-6\n", (int)Now());
        return -1;
    }

    if (pbImage == NULL)
        return -1;

    float* res;
    cvimodel_forward(&g_nCheckValid_Hand, pbImage, 128, 128, &res); 
    if (g_nStopEngine == 1)
    {
        return -1;
    }

#if 0
    {
        char szImageFilePath[255];
        sprintf(szImageFilePath, "/mnt/sd/handcheck_buf_%f_%f.bin", res[0], res[1]);
        int fd1 = aos_open(szImageFilePath, O_CREAT | O_RDWR);
        if(fd1 >= 0)
        {

            aos_write(fd1, pbImage, 128 * 128);
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

    return res[1] - res[0];
}

int	KdnnFreeCheckValid_Hand()
{
    cvimodel_release(&g_nCheckValid_Hand);

    releaseMachineDic(MachineFlagIndex_DNN_CheckValid_Hand);
    return KDNN_SUCCESS;
}

