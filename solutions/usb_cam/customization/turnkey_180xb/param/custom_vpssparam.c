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
#include "appdef.h"

PARAM_CLASSDEFINE(PARAM_VPSS_CHN_CFG_S,CHNCFG,GRP0,CHN)[] = {
    {
        .u8Rotation = ROTATION_0,
        .stVpssChnAttr = {
            .u32Width = UVC_INIT_WIDTH,
            .u32Height = UVC_INIT_HEIGHT,
            .enVideoFormat = VIDEO_FORMAT_LINEAR,
            .enPixelFormat = (DEFAULT_UVC_PIXEL_FMT == UVC_PIXEL_FMT_NV21 ? PIXEL_FORMAT_NV21 : PIXEL_FORMAT_YUV_PLANAR_422),
            .stFrameRate.s32SrcFrameRate = 30,
            .stFrameRate.s32DstFrameRate = 25,
            .bFlip = (DEFAULT_UVC_DIR == UVC_ROTATION_270 ? CVI_TRUE:CVI_FALSE),
            .bMirror = (DEFAULT_UVC_DIR == UVC_ROTATION_270 ? CVI_TRUE:CVI_FALSE),
            .u32Depth  = 0,
            .stAspectRatio.enMode        = ASPECT_RATIO_AUTO,
            .stAspectRatio.bEnableBgColor = CVI_TRUE,
            //.stAspectRatio.u32BgColor    = COLOR_RGB_BLACK,
            .stNormalize.bEnable         = CVI_FALSE,
        }
    },
};


PARAM_CLASSDEFINE(PARAM_VPSS_GRP_CFG_S,GRPCFG,CTX,GRP)[] = {
    {
        .VpssGrp = 0,
        .u8ChnCnt = 1,
        .pstChnCfg = PARAM_CLASS(CHNCFG,GRP0,CHN),
        .u8ViRotation = 0,
        .s32BindVidev = 0,
        .stVpssGrpAttr = {
            .u8VpssDev = 1,
            .u32MaxW = -1,
            .u32MaxH = -1,
            .enPixelFormat = (DEFAULT_UVC_PIXEL_FMT == UVC_PIXEL_FMT_NV21 ? PIXEL_FORMAT_NV21 : PIXEL_FORMAT_YUV_PLANAR_422),
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
