/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_vpsscfg.c
 * Description:
 *   ....
 */
#include "custom_param.h"
#include "board_config.h"
#include "cvi_buffer.h"

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP0,CHN)[] = {
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = 1280,
            .u32Height = 720,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = 30,
            .stFrameRate.s32DstFrameRate = 25,
            .bFlip = CVI_TRUE,
            .bMirror = CVI_TRUE,
            .u32Depth  = 0,
            .stAspectRatio.enMode        = ASPECT_RATIO_AUTO,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
            //.stAspectRatio.u32BgColor    = COLOR_RGB_BLACK,
            .stNormalize.bEnable         = CVI_FALSE,
        },
        .stVpssChnCropInfo = {
            .bEnable = CVI_TRUE,
            .stCropRect = {0, 150, 1600, 900},
        }
    },
    {
        .u8Rotation = ROTATION_90,
        .stVpssChnAttr = {
            .u32Width = 480,
            .u32Height = 864,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = 30,
            .stFrameRate.s32DstFrameRate = 25,
            .bFlip = CVI_FALSE,
            .bMirror = CVI_FALSE,
            .u32Depth  = 0,
            .stAspectRatio.enMode        = ASPECT_RATIO_AUTO,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
            //.stAspectRatio.u32BgColor    = COLOR_RGB_BLACK,
            .stNormalize.bEnable         = CVI_FALSE,
        },
        .stVpssChnCropInfo = {
            .bEnable = CVI_TRUE,
            .stCropRect = {0, 150, 1600, 900},
        }
    },
};


PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    {
        .VpssGrp = 0,
        .u8ChnCnt = 2,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = 0,
        .stVpssGrpAttr = {
            .u8VpssDev = 0,
            .u32MaxW = -1,
            .u32MaxH = -1,
            .enPixelFormat = PIXEL_FORMAT_NV21,
            .stFrameRate.s32SrcFrameRate = -1,
            .stFrameRate.s32DstFrameRate = -1,
        },
        .bBindMode = CVI_TRUE,
        .astChn[0] = {
            .enModId = CVI_ID_VI,
            .s32DevId = 0,
            .s32ChnId = 0,
        },
        .astChn[1] = {
            .enModId = CVI_ID_VPSS,
            .s32DevId = 0,
            .s32ChnId = 0,
        }
    },
};

PARAM_VPSS_CFG_S  g_stVpssCtx = {
    .u8GrpCnt = 1,
    .pstVpssGrpCfg = PARAM_CLASS(GRPCFG,CTX,GRP),
};

PARAM_VPSS_CFG_S * PARAM_GET_VPSS_CFG(void) {
    return &g_stVpssCtx;
}