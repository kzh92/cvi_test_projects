/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"

extern unsigned char rgb_color_mode_param[];
const unsigned int rgb_color_len = 174025;
extern unsigned char rgb_mono_mode_param[];
extern unsigned int rgb_mono_len;
#if 0
void * g_ViDmaBuf = NULL;
unsigned int g_ViDmaBufSize = 13 * 1024 * 1024;
#endif

PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
    {
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = 0x30,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 2,//GPIOC_13
        .u32Rst_pin = 13,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        .bSetDevAttrMipi = CVI_TRUE,
        .as16LaneId = {0, 1, 2, -1, -1},
        .as8PNSwap = {1, 1, 1, 0, 0},

        .bSetDevAttr = CVI_TRUE,
        .s16MacClk = RX_MAC_CLK_200M,
        .u8MclkCam = 0,
        .u8MclkFreq = CAMPLL_FREQ_27M,
    },
    {
        .enSnsType = CONFIG_SNS1_TYPE,
        .s32I2cAddr = 0x30,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 2,//GPIOC_13
        .u32Rst_pin = 13,
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
        .bSetDevAttrMipi = CVI_TRUE,
        .as16LaneId = {3, 4, -1, -1, -1},
        .as8PNSwap = {0, 0, 0, 0, 0},

        .bSetDevAttr = CVI_TRUE,
        .s16MacClk = RX_MAC_CLK_200M,
        .u8MclkCam = 1,
        .u8MclkFreq = CAMPLL_FREQ_27M,
    },
    // {
    //     .enSnsType = CONFIG_SNS1_TYPE,
    //     .s32I2cAddr = -1,
    //     .s8I2cDev = 2,
    //     //.u32Rst_port_idx = 2,//GPIOC_13
    //     //.u32Rst_pin = 13,
    //     //.u32Rst_pol = OF_GPIO_ACTIVE_LOW,
    // }
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0},
        .bUseSingleBin = 0,
        .astPQBinDes[0] =
        {
            .binID = CVI_BIN_ID_ISP0,
            .pIspBinData = rgb_color_mode_param,
            .u32IspBinDataLen = rgb_color_len,
        },
        .astPQBinDes[1] =
        {
            .binID = CVI_BIN_ID_ISP1,
            .pIspBinData = rgb_color_mode_param,
            .u32IspBinDataLen = rgb_color_len,
        },
    },
};

#if 0
PARAM_CLASSDEFINE(PARAM_DEV_CFG_S,VIDEVCFG,CTX,VI)[] = {
    {
        .pViDmaBuf = NULL,
        .u32ViDmaBufSize = 0,
    }
};
#endif

PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 2,
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
#if 0
    if(g_ViDmaBuf == NULL) {
        g_ViDmaBuf = (void *)malloc(g_ViDmaBufSize);
    }
    g_stViCtx.pstDevInfo[0].pViDmaBuf = g_ViDmaBuf;
    g_stViCtx.pstDevInfo[0].u32ViDmaBufSize = g_ViDmaBufSize;
#endif
    // g_stViCtx.pstIspCfg[0].stPQBinDes.u32IspBinDataLen = rgb_color_len;
    return &g_stViCtx;
}







