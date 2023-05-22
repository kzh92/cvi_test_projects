/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#if 1
void * g_ViDmaBuf = NULL;
unsigned int g_ViDmaBufSize = 13 * 1024 * 1024;
#endif
PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
    {
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 1,
        .u32Rst_port_idx = 2,
        .u32Rst_pin = 8,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
    }
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0},
        .bUseSingleBin = 0,
        .stPQBinDes =
        {
            .pIspBinData = NULL,
            .u32IspBinDataLen = 0,
        },
    },
};

PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,VIDEVCFG,CTX,VI)[] = {
    {
        .pViDmaBuf = NULL,
        .u32ViDmaBufSize = 0,
    }
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(VIDEVCFG,CTX,VI)
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
#if 1
    if(g_ViDmaBuf == NULL) {
        g_ViDmaBuf = (void *)malloc(g_ViDmaBufSize);
    }
    g_stViCtx.pstDevInfo[0].pViDmaBuf = g_ViDmaBuf;
    g_stViCtx.pstDevInfo[0].u32ViDmaBufSize = g_ViDmaBufSize;
#endif
    return &g_stViCtx;
}







