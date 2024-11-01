/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
extern unsigned char rgb_color_mode_param[];
PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
    {
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = 0x30,
        .s8I2cDev = 1,
        .u32Rst_port_idx = 2,
        .u32Rst_pin = 7,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        .bSetDevAttrMipi = CVI_TRUE,
        .as16LaneId = {3, 4, -1, -1, -1},
        .as8PNSwap = {0, 0, 0, 0, 0},
        .bSetDevAttr = CVI_TRUE,
        .s16MacClk = RX_MAC_CLK_200M,
        .u8MclkCam = 0,
        .u8MclkFreq = CAMPLL_FREQ_24M,
    }
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
#if 0    
    {
        .bMonoSet = {0},
        .bUseSingleBin = 1,
        .stPQBinDes =
        {
            .pIspBinData = rgb_color_mode_param,
            .u32IspBinDataLen = BIN_DATA_SIZE,
        },
    },
#endif
};

PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,VIDEVCFG,CTX,VI)[] = {
    {
        .pViDmaBuf = NULL,
        .u32ViDmaBufSize = 0,
    #if CONFIG_SENSOR_DUAL_SWITCH
        .isMux = true,
        .u8AttachDev = 0,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
    #endif
    },
    {
    #if CONFIG_SENSOR_DUAL_SWITCH
        .isMux = true,
        .u8AttachDev = 1,
        .switchGpioIdx = -1,
        .switchGpioPin = -1,
        .switchGPioPol = -1,
    #endif
    }
};

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .bFastConverge = 1,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(VIDEVCFG,CTX,VI)
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    return &g_stViCtx;
}







