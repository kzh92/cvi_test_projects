#include "jiwei_base.h"

#if (USE_WIFI_MODULE)

#include "st_common.h"
#include "st_vif.h"
#include "st_vpe.h"
#include "st_venc.h"
// #include "st_uvc.h"
#include "mi_divp.h"
#include "cam_base.h"
#include "common_types.h"
#include "my_uvc.h"
#include "cam_base.h"

// #include <stdlib.h>
// #include <signal.h>
// #include <sys/types.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
// #include <poll.h>

#ifdef ALIGN_UP
#undef ALIGN_UP
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#else
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (((val)/(alignment))*(alignment))
#endif

static int g_iJwMoInited = 0;

int JW_BaseModuleInit(int id, int iRotate, bool iFlip, bool iMirror)
{
    MI_U32 u32Width = 0, u32Height = 0;
    MI_U32 u32CropWidth = 0, u32CropHeight = 0;

    MI_DIVP_ChnAttr_t stAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;
    MI_U32 u32Divp1DevId = 0;
    MI_U32 u32Divp1ChnId = JW_DIVP1_CHN;
    MI_U32 u32Divp1PortId = 0;

    MI_U32 u32Divp2DevId = 0;
    MI_U32 u32Divp2ChnId = JW_DIVP2_CHN;
    MI_U32 u32Divp2PortId = 0;
    ST_Sys_BindInfo_T stDivp1ToDivp2BindInfo;

    MI_U32 u32VencDev = 1;
    MI_U32 u32VencChn = JW_VENC_CHN;
    MI_VENC_ChnAttr_t stChnAttr;
//    MI_U32  u32VenBitRate =0;
    ST_Sys_BindInfo_T stDivp2ToVencBindInfo;

    my_printf("[%s] start,%d,%d\n", __func__, g_iJwMoInited, id);
    my_mi_use_lock();

    if (g_iJwMoInited != 0)
    {
        my_mi_use_unlock();
        return MI_SUCCESS;
    }

    if (iRotate >= E_MI_SYS_ROTATE_NUM || iRotate < E_MI_SYS_ROTATE_NONE)
        iRotate = E_MI_SYS_ROTATE_NONE;

    u32Width = (id == DVP_CAM) ? WIDTH_1280 : WIDTH_1280;
    u32Height = (id == DVP_CAM) ? HEIGHT_960 : HEIGHT_720;

    /************************************************
     ************************************************

      init DIVP_1 (for Rotation)

     ************************************************
    *************************************************/

#if (USE_222MODE == 0)
    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    stAttr.bHorMirror = FALSE;
    stAttr.bVerMirror = FALSE;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = (MI_SYS_Rotate_e)iRotate;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
    stAttr.u32MaxWidth = u32Width;
    stAttr.u32MaxHeight = u32Height;
#else // USE_222MODE == 0
    u32CropWidth = (id == DVP_CAM) ?  WIDTH_1280 : WIDTH_960;
    u32CropHeight = (id == DVP_CAM) ?  HEIGHT_960 : HEIGHT_720;
    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    stAttr.bHorMirror = iMirror;
    stAttr.bVerMirror = iFlip;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
//    if(id == TC_MIPI_CAM)
    {
        stAttr.stCropRect.u16X = (u32Width - u32CropWidth) / 2;
        stAttr.stCropRect.u16Y = (u32Height - u32CropHeight) / 2;
        stAttr.stCropRect.u16Width = u32CropWidth;
        stAttr.stCropRect.u16Height = u32CropHeight;
    }
//    else
//    {
//        stAttr.stCropRect.u16X = 0;
//        stAttr.stCropRect.u16Y = 0;
//        stAttr.stCropRect.u16Width = u32Width;
//        stAttr.stCropRect.u16Height = u32Height;
//    }
    stAttr.u32MaxWidth = u32Width;
    stAttr.u32MaxHeight = u32Height;
#endif // USE_222MODE == 0
    if(MI_DIVP_CreateChn(u32Divp1ChnId, &stAttr) != MI_SUCCESS)
    {
        my_printf("[%s] MI_DIVP_CreateChn %d failed\n", __func__, u32Divp1ChnId);
        goto err_divp_1_create;
    }

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;

#if (USE_222MODE == 0)
    stOutputPortAttr.u32Width = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? u32Height : u32Width;
    stOutputPortAttr.u32Height = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? u32Width : u32Height;
#else // USE_222MODE
    stOutputPortAttr.u32Width = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? JIWEI_HEIGHT : JIWEI_WIDTH;
    stOutputPortAttr.u32Height = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? JIWEI_WIDTH : JIWEI_HEIGHT;
#endif // USE_222MODE
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    STCHECKRESULT(MI_DIVP_SetOutputPortAttr(u32Divp1ChnId, &stOutputPortAttr));
    if(MI_DIVP_StartChn(u32Divp1ChnId) != MI_SUCCESS)
    {
        my_printf("[%s] MI_DIVP_StartChn %d failed\n", __func__, u32Divp1ChnId);
        goto err_divp_1_start;
    }

    /************************************************
     ************************************************

      init DIVP_2 (for Scale, Mirror/Flip)

     ************************************************
    *************************************************/

#if (USE_222MODE == 0)

    u32Width = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? ((id == DVP_CAM) ?  HEIGHT_960 : HEIGHT_720) : ((id == DVP_CAM) ?  WIDTH_1280 : WIDTH_1280);
    u32Height = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? ((id == DVP_CAM) ?  WIDTH_1280 : WIDTH_1280) : ((id == DVP_CAM) ?  HEIGHT_960 : HEIGHT_720);
    u32CropWidth = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? ((id == DVP_CAM) ?  HEIGHT_960 : HEIGHT_720) : ((id == DVP_CAM) ?  WIDTH_1280 : WIDTH_960);
    u32CropHeight = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? ((id == DVP_CAM) ?  WIDTH_1280 : WIDTH_960) : ((id == DVP_CAM) ?  HEIGHT_960 : HEIGHT_720);

    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    stAttr.bHorMirror = iMirror;
    stAttr.bVerMirror = iFlip;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
//    if(id == TC_MIPI_CAM)
//    {
        stAttr.stCropRect.u16X = (u32Width - u32CropWidth) / 2;
        stAttr.stCropRect.u16Y = (u32Height - u32CropHeight) / 2;
        stAttr.stCropRect.u16Width = u32CropWidth;
        stAttr.stCropRect.u16Height = u32CropHeight;
//    }
//    else
//    {
//        stAttr.stCropRect.u16X = 0;
//        stAttr.stCropRect.u16Y = 0;
//        stAttr.stCropRect.u16Width = u32Width;
//        stAttr.stCropRect.u16Height = u32Height;
//    }
#else // USE_222MODE
    u32Width = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? JIWEI_HEIGHT : JIWEI_WIDTH;
    u32Height = (iRotate == E_MI_SYS_ROTATE_90 || iRotate == E_MI_SYS_ROTATE_270) ? JIWEI_WIDTH : JIWEI_HEIGHT;
    memset(&stAttr, 0, sizeof(stAttr));
    memset(&stOutputPortAttr, 0, sizeof(stOutputPortAttr));
    stAttr.bHorMirror = FALSE;
    stAttr.bVerMirror = FALSE;
    stAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
    stAttr.eRotateType = (MI_SYS_Rotate_e)iRotate;
    stAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
#endif // USE_222MODE

    stAttr.u32MaxWidth = u32Width;
    stAttr.u32MaxHeight = u32Height;
    if(MI_DIVP_CreateChn(u32Divp2ChnId, &stAttr) != MI_SUCCESS)
    {
        my_printf("[%s] MI_DIVP_CreateChn %d failed\n", __func__, u32Divp2ChnId);
        goto err_divp_2_create;
    }

    stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.u32Width = JIWEI_WIDTH;
    stOutputPortAttr.u32Height = JIWEI_HEIGHT;
    stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    STCHECKRESULT(MI_DIVP_SetOutputPortAttr(u32Divp2ChnId, &stOutputPortAttr));
    if(MI_DIVP_StartChn(u32Divp2ChnId) != MI_SUCCESS)
    {
        my_printf("[%s] MI_DIVP_StartChn %d failed\n", __func__, u32Divp2ChnId);
        goto err_divp_2_start;
    }

    // bind DIVP1 to DIVP2
    memset(&stDivp1ToDivp2BindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stDivp1ToDivp2BindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivp1ToDivp2BindInfo.stSrcChnPort.u32DevId = u32Divp1DevId;
    stDivp1ToDivp2BindInfo.stSrcChnPort.u32ChnId = u32Divp1ChnId;
    stDivp1ToDivp2BindInfo.stSrcChnPort.u32PortId = u32Divp1PortId;
    stDivp1ToDivp2BindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivp1ToDivp2BindInfo.stDstChnPort.u32DevId = u32Divp2DevId;
    stDivp1ToDivp2BindInfo.stDstChnPort.u32ChnId = u32Divp2ChnId;
    stDivp1ToDivp2BindInfo.stDstChnPort.u32PortId = u32Divp2PortId;
    stDivp1ToDivp2BindInfo.u32SrcFrmrate = 30;
    stDivp1ToDivp2BindInfo.u32DstFrmrate = 30;
    stDivp1ToDivp2BindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    if(ST_Sys_Bind(&stDivp1ToDivp2BindInfo) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Sys_Bind XXX to DIVP2 failed\n", __func__);
        goto err_divp_2_bind;
    }

    /************************************************
     ************************************************

      init VENC

     ************************************************
    *************************************************/

    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

//    u32VenBitRate = 1024 * 1024 * 2;
    stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = 30;
    stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;
    stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = JIWEI_WIDTH;
    stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = JIWEI_HEIGHT;
    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = JIWEI_WIDTH;
    stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = ALIGN_UP(JIWEI_HEIGHT, 16);
    stChnAttr.stVeAttr.stAttrJpeg.bByFrame = true;
    stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;

    if(ST_Venc_CreateChannel(u32VencChn, &stChnAttr) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Venc_CreateChannel failed\n", __func__);
        goto err_venc_create;
    }

    STCHECKRESULT(MI_VENC_GetChnDevid(u32VencChn, &u32VencDev));
    memset(&stDivp2ToVencBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stDivp2ToVencBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivp2ToVencBindInfo.stSrcChnPort.u32DevId = 0;
    stDivp2ToVencBindInfo.stSrcChnPort.u32ChnId = u32Divp2ChnId;
    stDivp2ToVencBindInfo.stSrcChnPort.u32PortId = u32Divp2PortId;
    stDivp2ToVencBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stDivp2ToVencBindInfo.stDstChnPort.u32ChnId = u32VencChn;
    stDivp2ToVencBindInfo.stDstChnPort.u32PortId = 0;
    stDivp2ToVencBindInfo.u32SrcFrmrate = 30;
    stDivp2ToVencBindInfo.u32DstFrmrate = 30;
    stDivp2ToVencBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    stDivp2ToVencBindInfo.u32BindParam = 0;
    stDivp2ToVencBindInfo.stDstChnPort.u32DevId = u32VencDev;

    if(ST_Sys_Bind(&stDivp2ToVencBindInfo) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Sys_Bind DIVP2 to VENC failed\n", __func__);
        goto err_venc_bind;
    }

    STCHECKRESULT(MI_VENC_SetMaxStreamCnt(u32VencChn, 3));
    if (ST_Venc_StartChannel(u32VencChn) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Venc_StartChannel failed\n", __func__);
        goto err_venc_start;
    }

    g_iJwMoInited = 1;
    my_mi_use_unlock();

    return MI_SUCCESS;

err_venc_start:
    STCHECKRESULT(ST_Sys_UnBind(&stDivp2ToVencBindInfo));
err_venc_bind:
    STCHECKRESULT(ST_Venc_DestoryChannel(u32VencChn));
err_venc_create:
    STCHECKRESULT(ST_Sys_UnBind(&stDivp1ToDivp2BindInfo));
err_divp_2_bind:
    STCHECKRESULT(MI_DIVP_StopChn(u32Divp2ChnId));
err_divp_2_start:
    STCHECKRESULT(MI_DIVP_DestroyChn(u32Divp2ChnId));
err_divp_2_create:
    STCHECKRESULT(MI_DIVP_StopChn(u32Divp1ChnId));
err_divp_1_start:
    STCHECKRESULT(MI_DIVP_DestroyChn(u32Divp1ChnId));
err_divp_1_create:
    my_mi_use_unlock();
    return -1;
}

int JW_BaseModuleUnInit(int id)
{
    MI_U32 u32Divp1DevId = 0;
    MI_U32 u32Divp1ChnId = JW_DIVP1_CHN;
    MI_U32 u32Divp1PortId = 0;
    MI_U32 u32Divp2DevId = 0;
    MI_U32 u32Divp2ChnId = JW_DIVP2_CHN;
    MI_U32 u32Divp2PortId = 0;
    MI_U32 u32VencDev = 1;
    MI_U32 u32VencChn = JW_VENC_CHN;

    ST_Sys_BindInfo_T stDivp1ToDivp2BindInfo;
    ST_Sys_BindInfo_T stDivp2ToVencBindInfo;

    my_printf("[%s] start,%d\n", __func__, g_iJwMoInited);
    my_mi_use_lock();

    if (g_iJwMoInited == 0)
    {
        my_mi_use_unlock();
        return 0;
    }

    g_iJwMoInited = 0;

    memset(&stDivp1ToDivp2BindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stDivp1ToDivp2BindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivp1ToDivp2BindInfo.stSrcChnPort.u32DevId = u32Divp1DevId;
    stDivp1ToDivp2BindInfo.stSrcChnPort.u32ChnId = u32Divp1ChnId;
    stDivp1ToDivp2BindInfo.stSrcChnPort.u32PortId = u32Divp1PortId;
    stDivp1ToDivp2BindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivp1ToDivp2BindInfo.stDstChnPort.u32DevId = u32Divp2DevId;
    stDivp1ToDivp2BindInfo.stDstChnPort.u32ChnId = u32Divp2ChnId;
    stDivp1ToDivp2BindInfo.stDstChnPort.u32PortId = u32Divp2PortId;
    stDivp1ToDivp2BindInfo.u32SrcFrmrate = 30;
    stDivp1ToDivp2BindInfo.u32DstFrmrate = 30;
    stDivp1ToDivp2BindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;

    memset(&stDivp2ToVencBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stDivp2ToVencBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_DIVP;
    stDivp2ToVencBindInfo.stSrcChnPort.u32DevId = 0;
    stDivp2ToVencBindInfo.stSrcChnPort.u32ChnId = u32Divp2ChnId;
    stDivp2ToVencBindInfo.stSrcChnPort.u32PortId = u32Divp2PortId;
    stDivp2ToVencBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    STCHECKRESULT(MI_VENC_GetChnDevid(u32VencChn, &u32VencDev));
    stDivp2ToVencBindInfo.stDstChnPort.u32DevId = u32VencDev;
    stDivp2ToVencBindInfo.stDstChnPort.u32ChnId = u32VencChn;
    stDivp2ToVencBindInfo.stDstChnPort.u32PortId = 0;
    stDivp2ToVencBindInfo.u32SrcFrmrate = 30;
    stDivp2ToVencBindInfo.u32DstFrmrate = 30;
    stDivp2ToVencBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;

    STCHECKRESULT(ST_Sys_UnBind(&stDivp2ToVencBindInfo));
    STCHECKRESULT(ST_Venc_StopChannel(u32VencChn));
    STCHECKRESULT(ST_Venc_DestoryChannel(u32VencChn));
    STCHECKRESULT(ST_Sys_UnBind(&stDivp1ToDivp2BindInfo));
    STCHECKRESULT(MI_DIVP_StopChn(u32Divp2ChnId));
    STCHECKRESULT(MI_DIVP_DestroyChn(u32Divp2ChnId));
    STCHECKRESULT(MI_DIVP_StopChn(u32Divp1ChnId));
    STCHECKRESULT(MI_DIVP_DestroyChn(u32Divp1ChnId));

    my_mi_use_unlock();

    return MI_SUCCESS;
}

int JW_InsertFrame2DIVP1(int id, unsigned char* pbBuf)
{
    MI_U32 u32Width = 0, u32Height = 0;
    MI_SYS_ChnPort_t stVpeChnInputPort;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufInfo_t stBufInfo;
    MI_U32 u32DevId = 0;
//    MI_U32 u32FrameSize = 0;
    MI_U32 u32YSize = 0;

    u32Width = (id == DVP_CAM) ? WIDTH_1280 : WIDTH_1280;
    u32Height = (id == DVP_CAM) ? HEIGHT_960 : HEIGHT_720;

    u32YSize = u32Width * u32Height;
//    u32FrameSize = (u32YSize >> 1) + u32YSize;

    memset(&stVpeChnInputPort, 0, sizeof(MI_SYS_ChnPort_t));
    stVpeChnInputPort.eModId = E_MI_MODULE_ID_DIVP;
    stVpeChnInputPort.u32DevId = u32DevId;
    stVpeChnInputPort.u32ChnId = JW_DIVP1_CHN;
    stVpeChnInputPort.u32PortId = 0;

    memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
    MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
    stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.stFrameCfg.eFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stBufConf.stFrameCfg.u16Width = u32Width;
    stBufConf.stFrameCfg.u16Height = u32Height;

    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));

    s32Ret = MI_SYS_ChnInputPortGetBuf(&stVpeChnInputPort, &stBufConf, &stBufInfo, &hHandle, 40);
    if(MI_SUCCESS == s32Ret)
    {
        memcpy(stBufInfo.stFrameData.pVirAddr[0], pbBuf, u32YSize);
        if(id == TC_MIPI_CAM)
            memset(stBufInfo.stFrameData.pVirAddr[1], 0x80, u32YSize >> 1);
        else if(id == DVP_CAM)
            memcpy(stBufInfo.stFrameData.pVirAddr[1], pbBuf + u32YSize, u32YSize >> 1);

        s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
        if (MI_SUCCESS != s32Ret)
        {
            my_printf("[%s] MI_SYS_ChnInputPortPutBuf error, %X\n", __func__, s32Ret);
            return -1;
        }
    }
    else
    {
        my_printf("[%s] MI_SYS_ChnInputPortGetBuf error, %X\n", __func__, s32Ret);
        return -1;
    }

    return 0;
}

int JW_GetFrameFromVENC(unsigned char* pbBuf, int* iSize)
{
    // MI_SYS_ChnPort_t stVencChnInputPort;
    MI_U32 u32DevId = 0;
    MI_U32 u32ChnId = JW_VENC_CHN;
//    MI_S32 vencFd = -1;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack[4];
    MI_S32 s32Ret = MI_SUCCESS;
    // struct timeval TimeoutVal;
//    fd_set read_fds;
    int iJpegSize = 0;
    int time_count = 10;//30ms * 10

    // if (g_iJwMoInited == 0)
    // {
    //     if (g_iDvpCamInited == 0)
    //         JW_BaseModuleInit(DVP_CAM, E_MI_SYS_ROTATE_90, false, true);
    //     else
    //         JW_BaseModuleInit(TC_MIPI_CAM, E_MI_SYS_ROTATE_90, true, true);
    //     if (g_iJwMoInited == 0)
    //         return -1;
    // }

    MI_VENC_GetChnDevid(u32ChnId, &u32DevId);
    // stVencChnInputPort.eModId = E_MI_MODULE_ID_VENC;
    // stVencChnInputPort.u32DevId = u32DevId;
    // stVencChnInputPort.u32ChnId = u32ChnId;
    // stVencChnInputPort.u32PortId = 0;
    // vencFd = MI_VENC_GetFd(u32ChnId);
    // if(vencFd <= 0)
    // {
    //     my_printf("[%s]  Unable to get FD:%d for chn:%2d\n", __func__, vencFd, u32ChnId);
    //     if(iSize)
    //         *iSize = 0;
    //     return -1;
    // }

    // TimeoutVal.tv_sec  = 0;
    // TimeoutVal.tv_usec = 300*1000;
    // FD_ZERO(&read_fds);
    // FD_SET(vencFd, &read_fds);
    // s32Ret = select(vencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
    // if (s32Ret < 0)
    //     my_printf("[%s]  select failed\n", __func__);
    // else if (0 == s32Ret)
    //     my_printf("[%s]  select timeout\n", __func__);
    // else
    // {
    //     if (FD_ISSET(vencFd, &read_fds))
    //     {
    while(time_count > 0)
    {
            memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
            memset(&stPack, 0, sizeof(MI_VENC_Pack_t) * 4);
            stStream.pstPack = stPack;

            s32Ret = MI_VENC_Query(u32ChnId, &stStat);
            if(s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
            {
                if (time_count < 5)
                    my_printf("[%s]  MI_VENC_Query failed  %x, %0.3f, %d\n", __func__, s32Ret, Now(), time_count);
            }
            else
            {
                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = MI_VENC_GetStream(u32ChnId, &stStream, 40);
                if (MI_SUCCESS == s32Ret)
                {
                    if(pbBuf)
                    {
                        for(int i = 0;i < (int)stStat.u32CurPacks; i++)
                        {
                            MI_S32 u32Size = stStream.pstPack[i].u32Len;
                            memcpy(pbBuf, stStream.pstPack[i].pu8Addr, u32Size);
                            pbBuf += u32Size;
                            iJpegSize += u32Size;
                        }
                    }

                    if(iSize)
                        *iSize = iJpegSize;

                    s32Ret = MI_VENC_ReleaseStream(u32ChnId, &stStream);
                    if (MI_SUCCESS != s32Ret)
                    {
                        my_printf("[%s] Release Frame Failed\n", __func__);
                    }

                    s32Ret = MI_VENC_CloseFd(u32ChnId);
                    if(s32Ret != 0)
                        my_printf("[%s] MI_VENC_CloseFd Chn%02d CloseFd error, Ret:%X\n", __func__, u32ChnId, s32Ret);

                    return MI_SUCCESS;
                }
                else
                    my_printf("[%s]  MI_VENC_GetStream failed  %x\n", __func__, s32Ret);
            }
            my_usleep(30 * 1000);
            time_count--;
    }
    //     }
    //     else
    //         my_printf("[%s] FD_ISSET Failed\n", __func__);
    // }

    s32Ret = MI_VENC_CloseFd(u32ChnId);
    if(s32Ret != 0)
        my_printf("[%s] MI_VENC_CloseFd Chn%02d CloseFd error, Ret:%X\n", __func__, u32ChnId, s32Ret);

    if(iSize)
        *iSize = 0;

    return -1;
}

#endif // USE_WIFI_MODULE
