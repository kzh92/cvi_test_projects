/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_viparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "appdef.h"
#define BIN_DATA_SIZE       347537
extern unsigned char rgb_color_mode_param[];
PARAM_CLASSDEFINE(PARAM_SNS_CFG_S,SENSORCFG,CTX,Sensor)[] = {
#if (DEFAULT_BOARD_TYPE == BD_TY_CV181xC_DEMO_V1v0)
    {
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 2,
        .u32Rst_pin = 13,//GPIOC_13
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
    },
#if (CONFIG_SNS1_TYPE)
    {
        .enSnsType = CONFIG_SNS1_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 2,
        .u32Rst_port_idx = 2,
        .u32Rst_pin = 13,//GPIOC_13
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
    }
#endif // CONFIG_SNS1_TYPE
#elif (DEFAULT_BOARD_TYPE == BD_TY_FMDASS_1V0J)
    {
        .enSnsType = CONFIG_SNS0_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 2, //iic2
        .u32Rst_port_idx = 2, //group
        .u32Rst_pin = 16,//GPIOC_16
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
    },
    {
        .enSnsType = CONFIG_SNS1_TYPE,
        .s32I2cAddr = -1,
        .s8I2cDev = 4, //iic4
        .u32Rst_port_idx = 2, //group
        .u32Rst_pin = 17,//GPIOC_17
        .u32Rst_pol = OF_GPIO_ACTIVE_LOW,
    }
#endif //DEFAULT_BOARD_TYPE
};

PARAM_CLASSDEFINE(PARAM_ISP_CFG_S,ISPCFG,CTX,ISP)[] = {
    {
        .bMonoSet = {0},
        .bUseSingleBin = 1,
        .stPQBinDes =
        {
            .pIspBinData = rgb_color_mode_param,
            .u32IspBinDataLen = BIN_DATA_SIZE,
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
    .u32WorkSnsCnt = (CONFIG_SNS0_TYPE > 0) + (CONFIG_SNS1_TYPE > 0),
    .pstSensorCfg = PARAM_CLASS(SENSORCFG,CTX,Sensor),
    .pstIspCfg = PARAM_CLASS(ISPCFG,CTX,ISP),
    .pstDevInfo = PARAM_CLASS(VIDEVCFG,CTX,VI)
};

PARAM_VI_CFG_S * PARAM_GET_VI_CFG(void) {
    return &g_stViCtx;
}







