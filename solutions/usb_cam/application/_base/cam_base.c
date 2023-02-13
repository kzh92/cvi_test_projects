#include "cam_base.h"
#include "common_types.h"
#include "mi_sys.h"
#include "mi_vpe.h"
#include "mi_vif.h"
#include "mi_sensor.h"
#include "st_common.h"
#include "st_vif.h"
#include "st_vpe.h"
#include "mi_venc.h"
#include "mi_sensor_datatype.h"
// #include <stdio.h>
// #include <stdlib.h>
#include <string.h>
// #include <assert.h>
// #include <stdbool.h>
// #include <stdint.h>
// #include <signal.h>
// #include <getopt.h>
// #include <pthread.h>
#include <fcntl.h>
// #include <errno.h>
// #include <memory.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/ioctl.h>
// #include <linux/i2c-dev.h>
#include "engineparam.h"

// pthread_mutex_t g_i2c0_reg_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t g_i2c1_reg_mutex = PTHREAD_MUTEX_INITIALIZER;
mymutex_ptr g_cam_init_mutex = NULL;
#if (USE_VDBTASK)
static int iActiveIRCam = -1;
mymutex_ptr g_clrBufLocker = 0;
#endif // USE_VDBTASK
mymutex_ptr g_camBufLock = 0;

#define RED_BOLD "\x1b[;31;1m"
#define GRN_BOLD "\x1b[;32;1m"
#define BLU_BOLD "\x1b[;34;1m"
#define PURPLE "\033[0;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define WHITE "\033[1;37m"

#define debug 0
#define undefined -1

#if (USE_SSD210)
#define MIPI0_I2C_PORT  1
#define MIPI1_I2C_PORT  1
#else
#define MIPI0_I2C_PORT  0
#define MIPI1_I2C_PORT  1
#endif

#define STCHECKRESULT02(result, execfunc)\
    result = execfunc;    \
    if (result != MI_SUCCESS)\
    {\
        my_printf("[%s %d]exec function failed ret x%X,\r\n", __FUNCTION__, __LINE__, result);\
        my_mi_use_unlock();\
        return 1;\
    }

int wait_camera_ready(int id)
{
#if 0 //kkk
    fd_set fds;
    struct timeval tv;
    int r;

    MI_S32 s32Fd = 0;
    MI_SYS_ChnPort_t stChnPort;
    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32ChnId = id * 4;
    stChnPort.u32DevId = id;
    stChnPort.u32PortId = 0;
    MI_SYS_GetFd(&stChnPort, &s32Fd);

    if (s32Fd < 0)
        return -1;

    FD_ZERO(&fds);
    FD_SET(s32Fd, &fds);

    /* Timeout */
    if(id == TC_MIPI_CAM)
    {
        tv.tv_sec  = 2;
        tv.tv_usec = 0;
    }
    else if(id == DVP_CAM)
    {
        tv.tv_sec  = 0;
        tv.tv_usec = 300 * 1000;
    }

    r = select(s32Fd + 1, &fds, NULL, NULL, &tv);
    if (r == -1)
    {
        my_printf("select err   %d\n", id);
        return -1;
    }
    else if (r == 0)
    {
        my_printf("select timeout  %d\n", id);
        return -2;
    }
#endif
    return 0;
}

typedef struct ST_Stream_Attr_s
{
    MI_BOOL    bEnable;
    MI_U32     u32Pipe;

    MI_U32     u32SnrPad;

    MI_U32     u32VifDev;
    MI_U32     u32VifChn;
    MI_U32     u32VifOutputPort;

    MI_U32     u32VpeChn;
    MI_U32     u32VpeOutputPort;
    MI_SYS_PixelFormat_e eVpeOutputPorPixelFormat;

    MI_U32     u32DivpChn;
    MI_U32     u32DivpOutputPort;
    MI_SYS_PixelFormat_e eDivpOutputPortPixelFormat;

    MI_U32         u32VencChn;
    MI_VENC_ModType_e eType;

    MI_U32    u32Width;
    MI_U32    u32Height;

    MI_U32*    Reserve;

}ST_Stream_Attr_T;

typedef struct _SnrInfo_t
{
    MI_U32 idx;
    MI_U16 u16SnrW;
    MI_U16 u16SnrH;
    MI_U32 u32SnrMaxFps;
    MI_U32 u32SnrMinFps;
} SnrInfo_t;

static int g_cam_inited[3] = {0};

#if (! USE_VDBTASK)

int camera_init(int id, int width, int height)
{
    my_mi_use_lock();
    if (g_cam_inited[id] != 0)
    {
        if (g_cam_inited[id] == 1)
        {
            my_mi_use_unlock();
            return 0;
        }
    }
    g_cam_inited[id] = -1;

    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;
    MI_SYS_PixelFormat_e ePixFormat;
    MI_VIF_FrameRate_e eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    MI_SYS_ChnPort_t stChnPort;

    MI_SNR_PAD_ID_e   ePADId      = (MI_SNR_PAD_ID_e)id;
    MI_U32            u32PlaneID  = 0;
    MI_SNR_Res_t      stSensorCurRes;
    MI_U32            u32SnrResCount;
    MI_U8             u8ResIndex;

    dbug_printf("[%s:%d] start\n", __func__, id);

    STCHECKRESULT02(s32Ret, MI_SNR_SetPlaneMode(ePADId, 0));

    STCHECKRESULT02(s32Ret, MI_SNR_QueryResCount(ePADId, &u32SnrResCount));
    for (u8ResIndex = 0; u8ResIndex < u32SnrResCount; u8ResIndex++)
    {
        memset(&stSensorCurRes, 0x00, sizeof(MI_SNR_Res_t));
        s32Ret = MI_SNR_GetRes(ePADId, u8ResIndex, &stSensorCurRes);
        if (MI_SUCCESS != s32Ret)
        {
            dbug_printf("[%s %d] Get sensor resolution index %d error!\n", __FUNCTION__, __LINE__, u8ResIndex);
            void my_mi_use_unlock();
            return s32Ret;
        }

        dbug_printf(BLU_BOLD"[%s %d] index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n"COLOR_NONE,__func__, __LINE__,
                      u8ResIndex,
                      stSensorCurRes.stCropRect.u16X,
                      stSensorCurRes.stCropRect.u16Y,
                      stSensorCurRes.stCropRect.u16Width,
                      stSensorCurRes.stCropRect.u16Height,
                      stSensorCurRes.stOutputSize.u16Width,
                      stSensorCurRes.stOutputSize.u16Height,
                      stSensorCurRes.u32MaxFps,
                      stSensorCurRes.u32MinFps,
                      stSensorCurRes.strResDesc);
    }

    MI_U8   u8FinalResIdx = u32SnrResCount - 1;

    dbug_printf(RED_BOLD"[%s %d] choose index %d\n"COLOR_NONE,__func__, __LINE__,u8FinalResIdx);

    STCHECKRESULT02(s32Ret, MI_SNR_SetRes(ePADId,  u8FinalResIdx));//imx415 4k@30fps

    if(MI_SNR_Enable(ePADId) != MI_SUCCESS)
    {
        my_printf("[%s] MI_SNR_Enable %d failed\n", __func__, id);
        goto err_snr;
    }
    MI_SNR_SetFps(ePADId, 30);

    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_SNR_PADInfo_t  stPad0Info;
    memset(&stPad0Info, 0, sizeof(MI_SNR_PADInfo_t));
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    if (MI_SNR_GetPlaneInfo(ePADId, u32PlaneID, &stSnrPlane0Info) != MI_SUCCESS)
    {
        my_printf("[%s] MI_SNR_GetPlaneInfo %d failed\n", __func__, id);
        goto err_vif;
    }
    if (MI_SNR_GetPadInfo(ePADId, &stPad0Info) != MI_SUCCESS)
    {
        my_printf("[%s] MI_SNR_GetPadInfo %d failed\n", __func__, id);
        goto err_vif;
    }
    u32CapWidth = stSnrPlane0Info.stCapRect.u16Width;
    u32CapHeight = stSnrPlane0Info.stCapRect.u16Height;
    dbug_printf(RED_BOLD"[%s %d]stSnrPlane0Info.stCapRect.u16Width=%d\n"COLOR_NONE,  __FUNCTION__, __LINE__, stSnrPlane0Info.stCapRect.u16Width);
    dbug_printf(RED_BOLD"[%s %d]stSnrPlane0Info.stCapRect.u16Height=%d\n"COLOR_NONE, __FUNCTION__, __LINE__, stSnrPlane0Info.stCapRect.u16Height);
    stSnrPlane0Info.ePixPrecision = E_MI_SYS_DATA_PRECISION_16BPP;
    ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);

    /************************************************
    Step1:    init VIF
    *************************************************/

#if 0
    MI_U32 u32VifDevId = 0;
    MI_U32 u32VifChnId = 0;
    MI_U32 u32VifPortId = 0;
    MI_VIF_DevAttr_t stVifDevAttr;
    MI_VIF_ChnPortAttr_t stVifChnPortAttr;
    memset(&stVifDevAttr, 0x0, sizeof(MI_VIF_DevAttr_t));
    stVifDevAttr.eIntfMode=E_MI_VIF_MODE_MIPI;
    stVifDevAttr.eWorkMode=E_MI_VIF_WORK_MODE_RGB_FRAMEMODE;//E_MI_VIF_WORK_MODE_RGB_REALTIME;
    stVifDevAttr.eHDRType=E_MI_VIF_HDR_TYPE_OFF;
    stVifDevAttr.eClkEdge=E_MI_VIF_CLK_EDGE_DOUBLE;
    stVifDevAttr.eDataSeq=E_MI_VIF_INPUT_DATA_YUYV;
    stVifDevAttr.eBitOrder=FALSE;
    STCHECKRESULT(s32Ret, MI_VIF_SetDevAttr(u32VifDevId, &stVifDevAttr));
    STCHECKRESULT(s32Ret, MI_VIF_EnableDev(u32VifDevId));
#endif

    //ST_Stream_Attr_T *pstStreamPipeLine = g_stStreamPipeLine;
    MI_U32 u32VifDevId = id;
    MI_U32 u32VifChnId = id * 4;
    MI_U32 u32VifPortId = 0;

    MI_VIF_WorkMode_e eVifWorkMode = E_MI_VIF_WORK_MODE_RGB_FRAMEMODE; //E_MI_VIF_WORK_MODE_RGB_REALTIME;
    MI_VIF_HDRType_e eVifHdrType = E_MI_VIF_HDR_TYPE_OFF;
    if(ST_Vif_EnableDev(u32VifDevId, eVifWorkMode, eVifHdrType, &stPad0Info) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vif_EnableDev %d failed\n", __func__, id);
        goto err_vif;
    }

    ST_VIF_PortInfo_T stVifPortInfoInfo;
    memset(&stVifPortInfoInfo, 0, sizeof(ST_VIF_PortInfo_T));
    stVifPortInfoInfo.u32RectX = 0;
    stVifPortInfoInfo.u32RectY = 0;
    stVifPortInfoInfo.u32RectWidth = u32CapWidth;
    stVifPortInfoInfo.u32RectHeight = u32CapHeight;
    stVifPortInfoInfo.u32DestWidth = u32CapWidth;
    stVifPortInfoInfo.u32DestHeight = u32CapHeight;
    stVifPortInfoInfo.eFrameRate = eFrameRate;
    stVifPortInfoInfo.ePixFormat = ePixFormat;

    if (ST_Vif_CreatePort(u32VifChnId, u32VifPortId, &stVifPortInfoInfo) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vif_CreatePort %d failed\n", __func__, id);
        goto err_vif_1;
    }
    if (ST_Vif_StartPort(u32VifDevId, u32VifChnId, u32VifPortId) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vif_StartPort %d failed\n", __func__, id);
        goto err_vif_1;
    }

    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32ChnId = u32VifChnId;
    stChnPort.u32DevId = u32VifDevId;
    stChnPort.u32PortId = u32VifPortId;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 5));
    /************************************************
    Step2:    init VPE
    *************************************************/
    MI_VPE_ChannelAttr_t stChannelVpeAttr;
    MI_VPE_ChannelAttr_t stVpeChnAttr;
    memset(&stChannelVpeAttr, 0, sizeof(MI_VPE_ChannelAttr_t));
    memset(&stVpeChnAttr, 0x0, sizeof(MI_VPE_ChannelAttr_t));
    stVpeChnAttr.u16MaxW = u32CapWidth;
    stVpeChnAttr.u16MaxH = u32CapHeight;
    stVpeChnAttr.bNrEn = FALSE;
    stVpeChnAttr.bEsEn = FALSE;
    stVpeChnAttr.bEdgeEn = FALSE;
    stVpeChnAttr.bUvInvert = FALSE;
    stVpeChnAttr.bContrastEn = FALSE;
    stVpeChnAttr.ePixFmt  = ePixFormat;
    stVpeChnAttr.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
    stVpeChnAttr.eRunningMode = E_MI_VPE_RUN_CAM_MODE; //E_MI_VPE_RUN_REALTIME_MODE;
    stVpeChnAttr.eSensorBindId = (MI_VPE_SensorChannel_e)(ePADId + 1);
#if 0
    pstEarlyInitParam = (MasterEarlyInitParam_t*) &stVpeChnAttr.tIspInitPara.u8Data[0];
    pstEarlyInitParam->u16SnrEarlyFps = pCameraBootSetting->u8SensorFrameRate;
    pstEarlyInitParam->u16SnrEarlyFlicker = pCameraBootSetting->u8AntiFlicker;
    pstEarlyInitParam->u32SnrEarlyShutter = pCameraBootSetting->u32shutter;
    pstEarlyInitParam->u32SnrEarlyGainX1024 = pCameraBootSetting->u32SensorGain;
    pstEarlyInitParam->u32SnrEarlyDGain = pCameraBootSetting->u32DigitalGain;
    pstEarlyInitParam->u16SnrEarlyAwbRGain = pCameraBootSetting->u16AWBRGain;
    pstEarlyInitParam->u16SnrEarlyAwbGGain = pCameraBootSetting->u16AWBGGain;
    pstEarlyInitParam->u16SnrEarlyAwbBGain = pCameraBootSetting->u16AWBBGain;
    stVpeChnAttr.tIspInitPara.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
    stVpeChnAttr.tIspInitPara.u32Size = sizeof(MasterEarlyInitParam_t);
#endif
    if(MI_VPE_CreateChannel(id, &stVpeChnAttr) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vpe_CreateChannel %d failed\n", __func__, id);
        goto err_vpe;
    }
    //STCHECKRESULT(MI_VPE_StartChannel(id));

#if 0
    /************************************************
    Step3:    Bind VIF & VPE
    *************************************************/
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = u32VifDevId;
    stBindInfo.stSrcChnPort.u32ChnId = u32VifChnId;
    stBindInfo.stSrcChnPort.u32PortId = u32VifPortId;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = pstStreamPipeLine[id].u32VpeChn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;//E_MI_SYS_BIND_TYPE_REALTIME;
    STCHECKRESULT02(s32Ret, ST_Sys_Bind(&stBindInfo));
#endif
    dbug_printf("[%s:%d] ended\n", __func__, id);
    g_cam_inited[id] = 1;
    my_mi_use_unlock();

    return 0;

err_vpe:
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));
    STCHECKRESULT(ST_Vif_StopPort(u32VifChnId, u32VifPortId));
err_vif_1:
    STCHECKRESULT(ST_Vif_DisableDev(u32VifDevId));
err_vif:
err_snr:
    STCHECKRESULT(MI_SNR_Disable(ePADId));
    my_mi_use_unlock();
    return -1;
}

int camera_release(int id)
{
    my_mi_use_lock();
    if (!g_cam_inited[id])
    {
        my_mi_use_unlock();
        return 0;
    }
    my_printf(" camera release   %d\n", id);

    MI_U32 u32VifDevId = 0;
    MI_U32 u32VifChnId = 0;
    MI_U32 u32VifPortId = 0;
    // MI_U32 u32VpeDevId = 0;
    MI_U32 u32VpeChnId = 0;
    // MI_U32 u32VpePortId = 0;
    MI_SYS_ChnPort_t stChnPort;

    /************************************************
      destory VPE
    *************************************************/

    MI_SNR_PAD_ID_e eSNRPad = (MI_SNR_PAD_ID_e)id;
    u32VifDevId = id;
    u32VifChnId = id * 4;
    u32VifPortId = 0;
//    u32VpeDevId = 0;
    u32VpeChnId = id;
//    u32VpePortId = 0;

    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32ChnId = id * 4;
    stChnPort.u32DevId = id;
    stChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));

    STCHECKRESULT(ST_Vpe_DestroyChannel(u32VpeChnId));
    /************************************************
      destory VIF
    *************************************************/
    STCHECKRESULT(ST_Vif_StopPort(u32VifChnId, u32VifPortId));
    STCHECKRESULT(ST_Vif_DisableDev(u32VifDevId));

    /************************************************
      destory SENSOR
    *************************************************/

    STCHECKRESULT(MI_SNR_Disable(eSNRPad));
    g_cam_inited[id] = 0;
    my_mi_use_unlock();
    return MI_SUCCESS;
}

#if (USE_SSD210)
#define I2C_ADDR_MIPI0_CAMERA 0x34
#define I2C_ADDR_MIPI1_CAMERA 0x36
#else
#define I2C_ADDR_MIPI0_CAMERA 0x36
#define I2C_ADDR_MIPI1_CAMERA 0x36
#endif

static myi2cdesc_ptr g_iMipiCam0 = 0;
static myi2cdesc_ptr g_iMipiCam1 = 0;

int camera_mipi0_set_regval(unsigned char regaddr, unsigned char regval)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam0 == 0) {
        my_i2c_open(MIPI0_I2C_PORT, &g_iMipiCam0);
    }
    if (g_iMipiCam0)
    {
        szBuf[0] = regaddr;
        szBuf[1] = regval;
        my_i2c_write8(g_iMipiCam0, I2C_ADDR_MIPI0_CAMERA, szBuf, 2);
    }
    my_usleep(1000);
    return 0;
}

int camera_mipi0_get_regval(unsigned char regaddr)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam0 == 0) {
        my_i2c_open(MIPI0_I2C_PORT, &g_iMipiCam0);
    }
    if (g_iMipiCam0)
    {
        szBuf[0] = regaddr;
        my_i2c_write8(g_iMipiCam0, I2C_ADDR_MIPI0_CAMERA, szBuf, 1);
        my_i2c_read8(g_iMipiCam0, I2C_ADDR_MIPI0_CAMERA, szBuf, 1);
    }
    return szBuf[0];
}


int camera_mipi1_set_regval(unsigned char regaddr, unsigned char regval)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam1 == 0) {
        my_i2c_open(MIPI1_I2C_PORT, &g_iMipiCam1);
    }
    if (g_iMipiCam1)
    {
        szBuf[0] = regaddr;
        szBuf[1] = regval;
        my_i2c_write8(g_iMipiCam1, I2C_ADDR_MIPI1_CAMERA, szBuf, 2);
    }
    my_usleep(1000);
    return 0;
}

int camera_mipi1_get_regval(unsigned char regaddr)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam1 == 0) {
        my_i2c_open(MIPI1_I2C_PORT, &g_iMipiCam1);
    }
    if (g_iMipiCam1)
    {
        szBuf[0] = regaddr;
        my_i2c_write8(g_iMipiCam1, I2C_ADDR_MIPI1_CAMERA, szBuf, 1);
        my_i2c_read8(g_iMipiCam1, I2C_ADDR_MIPI1_CAMERA, szBuf, 1);
    }
    return szBuf[0];
}

int camera_set_regval(int id, unsigned char regaddr, unsigned char regval)
{
    int ret = -1;
    if(id == MIPI_0_CAM)
        return camera_mipi0_set_regval(regaddr, regval);
    else if(id == MIPI_1_CAM)
        return camera_mipi1_set_regval(regaddr, regval);

    return ret;
}

int camera_get_regval(int id, unsigned char regaddr)
{
    if(id == MIPI_0_CAM)
        return camera_mipi0_get_regval(regaddr);
    else if(id == MIPI_1_CAM)
        return camera_mipi1_get_regval(regaddr);
    return 0;
}

int camera_set_exp_byreg(int id, int value)
{
    camera_set_regval(id, 0x01, (unsigned char)(value & 0xFF));
    camera_set_regval(id, 0x02, (unsigned char)((value >> 8) & 0x0F));
    //my_usleep(1000);

    dbug_printf("Set ExP: %d, %d\n", id, value);

    return 0;
}

int camera_get_exp_byreg(int id)
{
    int value = 0;
    value = camera_get_regval(id, 0x01) & 0xFF;
    value = value | ((camera_get_regval(id, 0x02) << 8) & 0xFF00);
    return value;
}

int camera_set_gain_byreg(int id, int value)
{
    camera_set_regval(id, 0x00, value);
    dbug_printf("Set Gain: %d, %d\n", id, value);
    return 0;
}

int camera_set_irled(int enable, int count)
{
    int ret = -1;
    //my_printf("----------  camera_set_irled\n");

    return ret;
}

#else // !USE_VDBTASK

// #include "cam_base_vdb.c"
int camera_init(int id, int width, int height, int switchIR_to)
{
    my_mi_use_lock();
    if (g_cam_inited[id] != 0)
    {
        if (g_cam_inited[id] == 1)
        {
            my_mi_use_unlock();
            return 0;
        }
        else if (id == TC_MIPI_CAM)
        {
            my_mi_use_unlock();
            return -1;
        }
    }
    g_cam_inited[id] = -1;

    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32CapWidth = 0, u32CapHeight = 0;
    MI_SYS_PixelFormat_e ePixFormat;
    MI_VIF_FrameRate_e eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    MI_SYS_ChnPort_t stChnPort;

    MI_SNR_PAD_ID_e   ePADId      = (MI_SNR_PAD_ID_e)id;
    MI_U32            u32PlaneID  = 0;
    MI_SNR_Res_t      stSensorCurRes;
    MI_U32            u32SnrResCount;
    MI_U8             u8ResIndex;

    dbug_printf("[%s:%d] start\n", __func__, id);

    STCHECKRESULT02(s32Ret, MI_SNR_SetPlaneMode(ePADId, 0));

    STCHECKRESULT02(s32Ret, MI_SNR_QueryResCount(ePADId, &u32SnrResCount));
    for (u8ResIndex = 0; u8ResIndex < u32SnrResCount; u8ResIndex++)
    {
        memset(&stSensorCurRes, 0x00, sizeof(MI_SNR_Res_t));
        s32Ret = MI_SNR_GetRes(ePADId, u8ResIndex, &stSensorCurRes);
        if (MI_SUCCESS != s32Ret)
        {
            dbug_printf("[%s %d] Get sensor resolution index %d error!\n", __FUNCTION__, __LINE__, u8ResIndex);
            my_mi_use_unlock();
            return s32Ret;
        }

        dbug_printf(BLU_BOLD"[%s %d] index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n"COLOR_NONE,__func__, __LINE__,
                      u8ResIndex,
                      stSensorCurRes.stCropRect.u16X,
                      stSensorCurRes.stCropRect.u16Y,
                      stSensorCurRes.stCropRect.u16Width,
                      stSensorCurRes.stCropRect.u16Height,
                      stSensorCurRes.stOutputSize.u16Width,
                      stSensorCurRes.stOutputSize.u16Height,
                      stSensorCurRes.u32MaxFps,
                      stSensorCurRes.u32MinFps,
                      stSensorCurRes.strResDesc);
    }

    MI_U8   u8FinalResIdx = u32SnrResCount - 1;

    dbug_printf(RED_BOLD"[%s %d] choose index %d\n"COLOR_NONE,__func__, __LINE__,u8FinalResIdx);

    STCHECKRESULT02(s32Ret, MI_SNR_SetRes(ePADId,  u8FinalResIdx));//imx415 4k@30fps

    if(MI_SNR_Enable(ePADId) != MI_SUCCESS)
    {
        my_printf("[%s] MI_SNR_Enable %d failed\n", __func__, id);
        goto err_snr;
    }
    MI_SNR_SetFps(ePADId, 30);

    if(id == TC_MIPI_CAM)
        camera_switch(TC_MIPI_CAM, switchIR_to);

    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_SNR_PADInfo_t  stPad0Info;
    memset(&stPad0Info, 0, sizeof(MI_SNR_PADInfo_t));
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    if (MI_SNR_GetPlaneInfo(ePADId, u32PlaneID, &stSnrPlane0Info) != MI_SUCCESS)
    {
        my_printf("[%s] MI_SNR_GetPlaneInfo %d failed\n", __func__, id);
        goto err_vif;
    }
    if (MI_SNR_GetPadInfo(ePADId, &stPad0Info) != MI_SUCCESS)
    {
        my_printf("[%s] MI_SNR_GetPadInfo %d failed\n", __func__, id);
        goto err_vif;
    }
    u32CapWidth = stSnrPlane0Info.stCapRect.u16Width;
    u32CapHeight = stSnrPlane0Info.stCapRect.u16Height;
    dbug_printf(RED_BOLD"[%s %d]stSnrPlane0Info.stCapRect.u16Width=%d\n"COLOR_NONE,  __FUNCTION__, __LINE__, stSnrPlane0Info.stCapRect.u16Width);
    dbug_printf(RED_BOLD"[%s %d]stSnrPlane0Info.stCapRect.u16Height=%d\n"COLOR_NONE, __FUNCTION__, __LINE__, stSnrPlane0Info.stCapRect.u16Height);
    if(id == DVP_CAM)
    {
        u32CapWidth = u32CapWidth * 2;//8BPP로 색카메라자료를 받을때 2바이트에 자료가 중복되는 문제가 있으므로 자료를 1600*600으로 받는다.
        stSnrPlane0Info.ePixPrecision = E_MI_SYS_DATA_PRECISION_8BPP;
    }
    else
        stSnrPlane0Info.ePixPrecision = E_MI_SYS_DATA_PRECISION_16BPP;
    ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);

    /************************************************
    Step1:    init VIF
    *************************************************/

#if 0
    MI_U32 u32VifDevId = 0;
    MI_U32 u32VifChnId = 0;
    MI_U32 u32VifPortId = 0;
    MI_VIF_DevAttr_t stVifDevAttr;
    MI_VIF_ChnPortAttr_t stVifChnPortAttr;
    memset(&stVifDevAttr, 0x0, sizeof(MI_VIF_DevAttr_t));
    stVifDevAttr.eIntfMode=E_MI_VIF_MODE_MIPI;
    stVifDevAttr.eWorkMode=E_MI_VIF_WORK_MODE_RGB_FRAMEMODE;//E_MI_VIF_WORK_MODE_RGB_REALTIME;
    stVifDevAttr.eHDRType=E_MI_VIF_HDR_TYPE_OFF;
    stVifDevAttr.eClkEdge=E_MI_VIF_CLK_EDGE_DOUBLE;
    stVifDevAttr.eDataSeq=E_MI_VIF_INPUT_DATA_YUYV;
    stVifDevAttr.eBitOrder=FALSE;
    STCHECKRESULT(s32Ret, MI_VIF_SetDevAttr(u32VifDevId, &stVifDevAttr));
    STCHECKRESULT(s32Ret, MI_VIF_EnableDev(u32VifDevId));
#endif

    //ST_Stream_Attr_T *pstStreamPipeLine = g_stStreamPipeLine;
    MI_U32 u32VifDevId = id;
    MI_U32 u32VifChnId = id * 4;
    MI_U32 u32VifPortId = 0;

    MI_VIF_WorkMode_e eVifWorkMode = E_MI_VIF_WORK_MODE_RGB_FRAMEMODE; //E_MI_VIF_WORK_MODE_RGB_REALTIME;
    MI_VIF_HDRType_e eVifHdrType = E_MI_VIF_HDR_TYPE_OFF;
    if(ST_Vif_EnableDev(u32VifDevId, eVifWorkMode, eVifHdrType, &stPad0Info) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vif_EnableDev %d failed\n", __func__, id);
        goto err_vif;
    }

    ST_VIF_PortInfo_T stVifPortInfoInfo;
    memset(&stVifPortInfoInfo, 0, sizeof(ST_VIF_PortInfo_T));
    stVifPortInfoInfo.u32RectX = 0;
    stVifPortInfoInfo.u32RectY = 0;
    stVifPortInfoInfo.u32RectWidth = u32CapWidth;
    stVifPortInfoInfo.u32RectHeight = u32CapHeight;
    stVifPortInfoInfo.u32DestWidth = u32CapWidth;
    stVifPortInfoInfo.u32DestHeight = u32CapHeight;
    stVifPortInfoInfo.eFrameRate = eFrameRate;
    stVifPortInfoInfo.ePixFormat = ePixFormat;

    if (ST_Vif_CreatePort(u32VifChnId, u32VifPortId, &stVifPortInfoInfo) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vif_CreatePort %d failed\n", __func__, id);
        goto err_vif_1;
    }
    if (ST_Vif_StartPort(u32VifDevId, u32VifChnId, u32VifPortId) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vif_StartPort %d failed\n", __func__, id);
        goto err_vif_1;
    }

    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32ChnId = u32VifChnId;
    stChnPort.u32DevId = u32VifDevId;
    stChnPort.u32PortId = u32VifPortId;
#if (USE_222MODE == 0)
    if(id == DVP_CAM)
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 3, 5));
    else
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 5));
#else // USE_222MODE == 0
    if(id == DVP_CAM)
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 2));
    else
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 1, 5));
#endif // USE_222MODE == 0
    /************************************************
    Step2:    init VPE
    *************************************************/
    MI_VPE_ChannelAttr_t stChannelVpeAttr;
    MI_VPE_ChannelAttr_t stVpeChnAttr;
    memset(&stChannelVpeAttr, 0, sizeof(MI_VPE_ChannelAttr_t));
    memset(&stVpeChnAttr, 0x0, sizeof(MI_VPE_ChannelAttr_t));
    stVpeChnAttr.u16MaxW = u32CapWidth;
    stVpeChnAttr.u16MaxH = u32CapHeight;
    stVpeChnAttr.bNrEn = FALSE;
    stVpeChnAttr.bEsEn = FALSE;
    stVpeChnAttr.bEdgeEn = FALSE;
    stVpeChnAttr.bUvInvert = FALSE;
    stVpeChnAttr.bContrastEn = FALSE;
    stVpeChnAttr.ePixFmt  = ePixFormat;
    stVpeChnAttr.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
    stVpeChnAttr.eRunningMode = E_MI_VPE_RUN_CAM_MODE; //E_MI_VPE_RUN_REALTIME_MODE;
    stVpeChnAttr.eSensorBindId = (MI_VPE_SensorChannel_e)(ePADId + 1);
#if 0
    pstEarlyInitParam = (MasterEarlyInitParam_t*) &stVpeChnAttr.tIspInitPara.u8Data[0];
    pstEarlyInitParam->u16SnrEarlyFps = pCameraBootSetting->u8SensorFrameRate;
    pstEarlyInitParam->u16SnrEarlyFlicker = pCameraBootSetting->u8AntiFlicker;
    pstEarlyInitParam->u32SnrEarlyShutter = pCameraBootSetting->u32shutter;
    pstEarlyInitParam->u32SnrEarlyGainX1024 = pCameraBootSetting->u32SensorGain;
    pstEarlyInitParam->u32SnrEarlyDGain = pCameraBootSetting->u32DigitalGain;
    pstEarlyInitParam->u16SnrEarlyAwbRGain = pCameraBootSetting->u16AWBRGain;
    pstEarlyInitParam->u16SnrEarlyAwbGGain = pCameraBootSetting->u16AWBGGain;
    pstEarlyInitParam->u16SnrEarlyAwbBGain = pCameraBootSetting->u16AWBBGain;
    stVpeChnAttr.tIspInitPara.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
    stVpeChnAttr.tIspInitPara.u32Size = sizeof(MasterEarlyInitParam_t);
#endif
    if(MI_VPE_CreateChannel(id, &stVpeChnAttr) != MI_SUCCESS)
    {
        my_printf("[%s] ST_Vpe_CreateChannel %d failed\n", __func__, id);
        goto err_vpe;
    }
    //STCHECKRESULT(MI_VPE_StartChannel(id));

#if 0
    /************************************************
    Step3:    Bind VIF & VPE
    *************************************************/
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = u32VifDevId;
    stBindInfo.stSrcChnPort.u32ChnId = u32VifChnId;
    stBindInfo.stSrcChnPort.u32PortId = u32VifPortId;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = pstStreamPipeLine[id].u32VpeChn;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;//E_MI_SYS_BIND_TYPE_REALTIME;
    STCHECKRESULT02(s32Ret, ST_Sys_Bind(&stBindInfo));
#endif
    dbug_printf("[%s:%d] ended\n", __func__, id);
    g_cam_inited[id] = 1;
    my_mi_use_unlock();
    return 0;

err_vpe:
#if (USE_222MODE == 0)
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));
#else
    if(id == DVP_CAM)
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 3));
    else
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 2));
#endif
    STCHECKRESULT(ST_Vif_StopPort(u32VifChnId, u32VifPortId));
err_vif_1:
    STCHECKRESULT(ST_Vif_DisableDev(u32VifDevId));
err_vif:
    STCHECKRESULT(MI_SNR_Disable(ePADId));
err_snr:
    STCHECKRESULT(MI_SNR_Disable(ePADId));
    my_mi_use_unlock();
    return -1;
}

int camera_release(int id)
{
    my_mi_use_lock();
    if (!g_cam_inited[id])
    {
        my_mi_use_unlock();
        return 0;
    }
    my_printf(" camera release   %d\n", id);

    MI_U32 u32VifDevId = 0;
    MI_U32 u32VifChnId = 0;
    MI_U32 u32VifPortId = 0;
    // MI_U32 u32VpeDevId = 0;
    MI_U32 u32VpeChnId = 0;
    // MI_U32 u32VpePortId = 0;
    MI_SYS_ChnPort_t stChnPort;

    /************************************************
      destory VPE
    *************************************************/

    MI_SNR_PAD_ID_e eSNRPad = (MI_SNR_PAD_ID_e)id;
    u32VifDevId = id;
    u32VifChnId = id * 4;
    u32VifPortId = 0;
//    u32VpeDevId = 0;
    u32VpeChnId = id;
//    u32VpePortId = 0;

    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32ChnId = id * 4;
    stChnPort.u32DevId = id;
    stChnPort.u32PortId = 0;
#if (USE_222MODE == 0)
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5));
#else // USE_222MODE == 0
    if(id == DVP_CAM)
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 3));
    else
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 2));
#endif // USE_222MODE == 0

    STCHECKRESULT(ST_Vpe_DestroyChannel(u32VpeChnId));
    /************************************************
      destory VIF
    *************************************************/
    STCHECKRESULT(ST_Vif_StopPort(u32VifChnId, u32VifPortId));
    STCHECKRESULT(ST_Vif_DisableDev(u32VifDevId));

    /************************************************
      destory SENSOR
    *************************************************/

    STCHECKRESULT(MI_SNR_Disable(eSNRPad));
    g_cam_inited[id] = 0;
    my_mi_use_unlock();
    return MI_SUCCESS;
}

#define I2C_ADDR_MIPI0_CAMERA 0x34
#define I2C_ADDR_MIPI1_CAMERA 0x36
#define I2C_ADDR_DVP_CAMERA   0x3C

static myi2cdesc_ptr g_iMipiCam0 = 0;
static myi2cdesc_ptr g_iMipiCam1 = 0;
static myi2cdesc_ptr g_iDvpCam = 0;

int camera_mipi0_set_regval(unsigned char regaddr, unsigned char regval)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam0 == 0) {
        my_i2c_open(MIPI1_I2C_PORT, &g_iMipiCam0);
    }
    if (g_iMipiCam0)
    {
        szBuf[0] = regaddr;
        szBuf[1] = regval;
        my_i2c_write8(g_iMipiCam0, I2C_ADDR_MIPI0_CAMERA, szBuf, 2);
    }
    my_usleep(1000);
    return 0;
}

int camera_mipi0_get_regval(unsigned char regaddr)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam0 == 0) {
        my_i2c_open(MIPI1_I2C_PORT, &g_iMipiCam0);
    }
    if (g_iMipiCam0)
    {
        szBuf[0] = regaddr;
        my_i2c_write8(g_iMipiCam0, I2C_ADDR_MIPI0_CAMERA, szBuf, 1);
        my_i2c_read8(g_iMipiCam0, I2C_ADDR_MIPI0_CAMERA, szBuf, 1);
    }
    return szBuf[0];
}


int camera_mipi1_set_regval(unsigned char regaddr, unsigned char regval)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam1 == 0) {
        my_i2c_open(MIPI1_I2C_PORT, &g_iMipiCam1);
    }
    if (g_iMipiCam1)
    {
        szBuf[0] = regaddr;
        szBuf[1] = regval;
        my_i2c_write8(g_iMipiCam1, I2C_ADDR_MIPI1_CAMERA, szBuf, 2);
    }
    my_usleep(1000);
    return 0;
}

int camera_mipi1_get_regval(unsigned char regaddr)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iMipiCam1 == 0) {
        my_i2c_open(MIPI1_I2C_PORT, &g_iMipiCam1);
    }
    if (g_iMipiCam1)
    {
        szBuf[0] = regaddr;
        my_i2c_write8(g_iMipiCam1, I2C_ADDR_MIPI1_CAMERA, szBuf, 1);
        my_i2c_read8(g_iMipiCam1, I2C_ADDR_MIPI1_CAMERA, szBuf, 1);
    }
    return szBuf[0];
}

int camera_dvp_set_regval(unsigned char regaddr, unsigned char regval)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iDvpCam == 0) {
        my_i2c_open(MIPI0_I2C_PORT, &g_iDvpCam);
    }
    if (g_iDvpCam)
    {
        szBuf[0] = regaddr;
        szBuf[1] = regval;
        my_i2c_write8(g_iDvpCam, I2C_ADDR_DVP_CAMERA, szBuf, 2);
    }
    return 0;
}

int camera_dvp_get_regval(unsigned char regaddr)
{
    unsigned char szBuf[2] = { 0 };
    if (g_iDvpCam == 0) {
        my_i2c_open(MIPI0_I2C_PORT, &g_iDvpCam);
    }
    if (g_iDvpCam)
    {
        szBuf[0] = regaddr;
        my_i2c_write8(g_iDvpCam, I2C_ADDR_DVP_CAMERA, szBuf, 1);
        my_i2c_read8(g_iDvpCam, I2C_ADDR_DVP_CAMERA, szBuf, 1);
    }
    return szBuf[0];
}

int camera_set_regval(int id, unsigned char regaddr, unsigned char regval)
{
    int i = 0;
    int ret = -1;
    for(i = 0; i < 3; i ++)
    {
        if(id == DVP_CAM)
        {
            //pthread_mutex_lock(&g_i2c0_reg_mutex);
            ret = camera_dvp_set_regval(regaddr, regval);
            //pthread_mutex_unlock(&g_i2c0_reg_mutex);
        }
        else
        {
            //pthread_mutex_lock(&g_i2c1_reg_mutex);
            if(id == TC_MIPI_CAM)
            {
                ret = camera_mipi0_set_regval(regaddr, regval);
            }
            else if(id == TC_MIPI_CAM1)
            {
                ret = camera_mipi1_set_regval(regaddr, regval);
            }
            //pthread_mutex_unlock(&g_i2c1_reg_mutex);
        }

        if(ret == 0)
            break;
    }

    return ret;
}

int camera_get_regval(int id, unsigned char regaddr)
{
    int ret = -1;
    int i = 0;
    for(i = 0; i < 3; i ++)
    {
        if(id == DVP_CAM)
        {
            //pthread_mutex_lock(&g_i2c0_reg_mutex);
            ret = camera_dvp_get_regval(regaddr);
            //pthread_mutex_unlock(&g_i2c0_reg_mutex);
        }
        else
        {
            //pthread_mutex_lock(&g_i2c1_reg_mutex);
            if(id == TC_MIPI_CAM)
            {
                ret = camera_mipi0_get_regval(regaddr);
            }
            else if(id == TC_MIPI_CAM1)
            {
                ret = camera_mipi1_get_regval(regaddr);
            }
            //pthread_mutex_unlock(&g_i2c1_reg_mutex);
        }

        if(ret == 0)
            break;
    }

    return ret;
}

int camera_set_exp_byreg(int id, int value)
{
    camera_set_regval(id, 0x01, (unsigned char)(value & 0xFF));
    camera_set_regval(id, 0x02, (unsigned char)((value >> 8) & 0x0F));

//  my_printf("Set ExP: %d, %d\n", id, value);

    return 0;
}

int camera_set_gain_byreg(int id, int value)
{
    camera_set_regval(id, 0x00, value);
//    printf("Set Gain: %d, %d\n", id, value);
    return 0;
}

int camera_set_irled(int id, int enable)
{
    int ret = -1;
//  my_printf("----------  camera_set_irled\n");

    return ret;
}

int camera_clr_set_exp(int value)
{
    camera_set_regval(DVP_CAM, 0xfe, 0);
    camera_set_regval(DVP_CAM, 0x04, (unsigned char)value);
    camera_set_regval(DVP_CAM, 0x03, (unsigned char)((value >> 8) & 0xFF));
    return 0;
}

int camera_clr_get_exp()
{
    unsigned char b1, b2;
    camera_set_regval(DVP_CAM, 0xfe, 0); //page select

    b1 = camera_get_regval(DVP_CAM, 0x04);
    b2 = camera_get_regval(DVP_CAM, 0x04);
    return b1 | (b2 << 8);
}

int camera_clr_set_gain(int value)
{
    camera_set_regval(DVP_CAM, 0xfe, 0); //page select
    camera_set_regval(DVP_CAM, 0xb0, (unsigned char)value);
    return 0;
}

int camera_clr_get_gain()
{
    unsigned char b1;
    camera_set_regval(DVP_CAM, 0xfe, 0); //page select
    b1 = camera_get_regval(DVP_CAM, 0xb0);
    return b1;
}

int camera_switch(int id, int camid)
{
    int ret = -1;

//    printf("camera_switch: %d\n", camid);
    do
    {
        if(camid == MIPI_CAM_SUB0) // right cam
        {
            if(camera_set_regval(TC_MIPI_CAM, 0x7A, 0xCC) < 0) //pause
                break;
            if(camera_set_regval(TC_MIPI_CAM1, 0x7A, 0x4C) < 0) //resume
                break;
        }
        else if(camid == MIPI_CAM_SUB1) // left cam
        {
            if(camera_set_regval(TC_MIPI_CAM1, 0x7A, 0xCC) < 0)
                break;
            if(camera_set_regval(TC_MIPI_CAM, 0x7A, 0x4C) < 0)
                break;
        }

        ret = 0;
    } while(0);

    if(ret < 0)
    {
        my_printf("switch err  %d\n", camid);
        return ret;
    }

    iActiveIRCam = camid;
    return ret;
}

int camera_get_actIR()
{
    return iActiveIRCam;
}

void lockClrBuffer()
{
    if (g_clrBufLocker == NULL)
        g_clrBufLocker = my_mutex_init();
    my_mutex_lock(g_clrBufLocker);
}

void unlockClrBuffer()
{
    if (g_clrBufLocker == NULL)
        g_clrBufLocker = my_mutex_init();
    my_mutex_unlock(g_clrBufLocker);
}

#endif // !USE_VDBTASK

int camera_set_pattern_mode(int cam_id, int enable)
{
    my_mi_use_lock();
#if (USE_VDBTASK)
    switch(cam_id)
    {
    case CAM_ID_CLR:
        if (enable)
        {
            camera_set_regval(DVP_CAM, 0xfe, 0x00);
            camera_set_regval(DVP_CAM, 0x82, 0xfa);
            camera_set_regval(DVP_CAM, 0xad, 0x80);
            camera_set_regval(DVP_CAM, 0xae, 0x80);
            camera_set_regval(DVP_CAM, 0xaf, 0x80);
            camera_set_regval(DVP_CAM, 0xed, 0x02);
            camera_set_regval(DVP_CAM, 0xee, 0x30);
            camera_set_regval(DVP_CAM, 0xef, 0x48);
            camera_set_regval(DVP_CAM, 0xfe, 0x01);
            camera_set_regval(DVP_CAM, 0x13, 0x78);
            camera_set_regval(DVP_CAM, 0x01, 0x04);//AEC measure window x1
            camera_set_regval(DVP_CAM, 0x02, 0x9e);//AEC measure window x2
            camera_set_regval(DVP_CAM, 0x03, 0x08);//AEC measure window y1
            camera_set_regval(DVP_CAM, 0x04, 0x75);//AEC measure window y2
            camera_set_regval(DVP_CAM, 0xfe, 0x02);
            camera_set_regval(DVP_CAM, 0x97, 0x48);
#if (1)
            camera_set_regval(DVP_CAM, 0xd0, 0x40);//Global saturation
            camera_set_regval(DVP_CAM, 0xd1, 0x32);//Cb saturation
            camera_set_regval(DVP_CAM, 0xd2, 0x32);//Cr saturation
#endif

            camera_set_regval(DVP_CAM, 0xfe, 0x00);
            camera_set_regval(DVP_CAM, 0xB6, 0x00);
            camera_set_regval(DVP_CAM, 0xB2, 0x50);
            camera_clr_set_exp(INIT_CLR_EXP);
            camera_clr_set_gain(INIT_CLR_GAIN);
            camera_set_regval(DVP_CAM, 0x8c, 0x0a); //pattern mode
            camera_set_regval(DVP_CAM, 0x8d, 0x58); //pattern mode
        }
        else
        {
            camera_set_regval(DVP_CAM, 0xfe, 0x00);
            camera_set_regval(DVP_CAM, 0xb6, 1); //AEC enable
            camera_set_regval(DVP_CAM, 0x8c, 0x00); //normal mode
            camera_set_regval(DVP_CAM, 0x8d, 0x01); //normal mode
        }
        break;
    case CAM_ID_IR1:
        camera_set_regval(TC_MIPI_CAM, 0x0C, enable ? 0x41: 0x40);
        break;
    case CAM_ID_IR2:
        camera_set_regval(TC_MIPI_CAM1, 0x0C, enable ? 0x41: 0x40);
        break;
    default:
        break;
    }
#else // USE_VDBTASK
    switch(cam_id)
    {
    case CAM_ID_IR1:
        camera_set_regval(MIPI_0_CAM, 0x0C, enable ? 0x41: 0x40);
        break;
    case CAM_ID_IR2:
        camera_set_regval(MIPI_1_CAM, 0x0C, enable ? 0x41: 0x40);
        break;
    default:
        break;
    }
#endif // USE_VDBTASK
    my_mi_use_unlock();
    return 0;
}

void my_mi_use_lock()
{
    if (g_cam_init_mutex == NULL)
        g_cam_init_mutex = my_mutex_init();
    my_mutex_lock(g_cam_init_mutex);
}

void my_mi_use_unlock()
{
    if (g_cam_init_mutex == NULL)
        g_cam_init_mutex = my_mutex_init();
    my_mutex_unlock(g_cam_init_mutex);
}

void lockIRBuffer()
{
    if (g_camBufLock == NULL)
        g_camBufLock = my_mutex_init();
    my_mutex_lock(g_camBufLock);
}

void unlockIRBuffer()
{
    if (g_camBufLock == NULL)
        g_camBufLock = my_mutex_init();
    my_mutex_unlock(g_camBufLock);
}
