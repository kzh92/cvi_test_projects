#include "cam_base.h"
#include "common_types.h"
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
#include "common_vi.h"

// pthread_mutex_t g_i2c0_reg_mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t g_i2c1_reg_mutex = PTHREAD_MUTEX_INITIALIZER;
mymutex_ptr g_cam_init_mutex = NULL;
static int iActiveIRCam = MIPI_CAM_S2LEFT;

#if (USE_VDBTASK)
mymutex_ptr g_clrBufLocker = 0;
#endif // USE_VDBTASK
mymutex_ptr g_camBufLock = 0;
// static int g_cam_inited[3] = {0};

#if 0
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
#endif

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
int camera_init(int id, int width, int height, int switchIR_to)
{
    camera_switch(TC_MIPI_CAM, switchIR_to);
    return 0;
}


#if (! USE_VDBTASK)

int camera_release(int id)
{
#if 0
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
#endif
    return 0;
}

#else // !USE_VDBTASK

int camera_release(int id)
{
#if 0
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
#endif
    return 0;
}

#define I2C_ADDR_MIPI0_CAMERA 0x34
#define I2C_ADDR_MIPI1_CAMERA 0x36
#define I2C_ADDR_DVP_CAMERA   0x3C

// static myi2cdesc_ptr g_iMipiCam0 = 0;
// static myi2cdesc_ptr g_iMipiCam1 = 0;
// static myi2cdesc_ptr g_iDvpCam = 0;

int camera_mipi0_set_regval(unsigned char regaddr, unsigned char regval)
{
#if 0
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
#endif    
    return 0;
}

int camera_mipi0_get_regval(unsigned char regaddr)
{
#if 0
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
#endif
    return 0;
}


int camera_mipi1_set_regval(unsigned char regaddr, unsigned char regval)
{
#if 0
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
#endif
    return 0;
}

int camera_mipi1_get_regval(unsigned char regaddr)
{
#if 0
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
#endif
    return 0;
}

int camera_dvp_set_regval(unsigned char regaddr, unsigned char regval)
{
#if 0
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
#endif
    return 0;
}

int camera_dvp_get_regval(unsigned char regaddr)
{
#if 0
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
#endif
    return 0;
}

int camera_clr_set_exp(int value)
{
    // camera_set_regval(DVP_CAM, 0xfe, 0);
    // camera_set_regval(DVP_CAM, 0x04, (unsigned char)value);
    // camera_set_regval(DVP_CAM, 0x03, (unsigned char)((value >> 8) & 0xFF));
    return 0;
}

int camera_clr_get_exp()
{
    // unsigned char b1, b2;
    // camera_set_regval(DVP_CAM, 0xfe, 0); //page select

    // b1 = camera_get_regval(DVP_CAM, 0x04);
    // b2 = camera_get_regval(DVP_CAM, 0x04);
    // return b1 | (b2 << 8);
    return 0;
}

int camera_clr_set_gain(int value)
{
    // camera_set_regval(DVP_CAM, 0xfe, 0); //page select
    // camera_set_regval(DVP_CAM, 0xb0, (unsigned char)value);
    return 0;
}

int camera_clr_get_gain()
{
    // unsigned char b1;
    // camera_set_regval(DVP_CAM, 0xfe, 0); //page select
    // b1 = camera_get_regval(DVP_CAM, 0xb0);
    // return b1;
    return 0;
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
    ISP_SNS_OBJ_S *pSnsObj = NULL;

    //if (cam_id == TC_MIPI_CAM)
    {
        my_mi_use_lock();
        pSnsObj = getSnsObj(SMS_SC201CS_MIPI_2M_30FPS_10BIT);
        if (!pSnsObj)
        {
            my_mi_use_unlock();
            return -1;
        }
        pSnsObj->pfnSnsPatternEn(0, enable);
        pSnsObj = getSnsObj(SMS_SC201CS_SLAVE_MIPI_2M_30FPS_10BIT);
        if (!pSnsObj)
        {
            my_mi_use_unlock();
            return -1;
        }
        pSnsObj->pfnSnsPatternEn(1, enable);
        my_mi_use_unlock();
    }

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

int camera_switch(int id, int camid)
{
#if (!USE_3M_MODE)
    my_mi_use_lock();
    ISP_SNS_OBJ_S *pSnsObj = NULL;

    pSnsObj = getSnsObj(SMS_SC201CS_MIPI_2M_30FPS_10BIT);
    if (!pSnsObj)
    {
        my_mi_use_unlock();
        return -1;
    }
    pSnsObj->pfnSnsSwitch(0, camid == MIPI_CAM_S2LEFT);
    my_mi_use_unlock();
#else // !USE_3M_MODE
    //if (iActiveIRCam != MIPI_0_CAM)
    //    camera_sleep(iActiveIRCam);
    //camera_wakeup(camid);
#endif // !USE_3M_MODE
    iActiveIRCam = camid;
    dbug_printf("[%s] %d:%s\n", __func__, iActiveIRCam, iActiveIRCam == MIPI_CAM_S2LEFT?"left":"right");
    return 0;
}

int camera_get_actIR()
{
    return iActiveIRCam;
}

int camera_sleep(int id)
{
    my_printf("*********[%s] id=%d\n", __func__, id);
#if (USE_IRSNR_SC2355)
    camera_set_regval(id, 0x3019,0xff);
    camera_set_regval(id, 0x0100,0x00);
#endif
    return 0;
}

int camera_wakeup(int id)
{
    my_printf("*********[%s] id=%d\n", __func__, id);
#if (USE_IRSNR_SC2355)
    camera_set_regval(id, 0x0100,0x01);
    camera_set_regval(id, 0x3019,0xfe);
#endif
    return 0;
}

int camera_set_regval(int id, int regaddr, int regval)
{
    int snr_id = 0;
    int vipe_no = 0;
    if (id == MIPI_0_CAM)
    {
        snr_id = SMS_SC201CS_MIPI_2M_30FPS_10BIT;
        vipe_no = 0;
    }
    else if (id == MIPI_1_CAM)
    {
        snr_id = SMS_SC201CS_SLAVE_MIPI_2M_30FPS_10BIT;
        vipe_no = 1;
    }
    else
    {
        my_printf("invalid camera id(%d)\n", id);
        return -1;
    }

    my_mi_use_lock();
    ISP_SNS_OBJ_S *pSnsObj = NULL;

    pSnsObj = getSnsObj(snr_id);
    if (!pSnsObj)
    {
        my_mi_use_unlock();
        return -1;
    }

    pSnsObj->pfnWriteRegEx(vipe_no, regaddr, regval, 0);

    my_mi_use_unlock();
    return 0;
}

int camera_get_regval(int id, int regaddr)
{
    // if(id == MIPI_0_CAM)
    //     return camera_mipi0_get_regval(regaddr);
    // else if(id == MIPI_1_CAM)
    //     return camera_mipi1_get_regval(regaddr);
    return 0;
}

int camera_set_exp_byreg(int id, int value)
{
    int vi_pipe = 0;
    ISP_SNS_OBJ_S *pSnsObj = NULL;

#if (USE_3M_MODE)
    if (id == MIPI_1_CAM)
        return 0;
    vi_pipe = 1;
#endif
    my_mi_use_lock();
    pSnsObj = getSnsObj(!USE_3M_MODE ? SMS_SC201CS_MIPI_2M_30FPS_10BIT : SMS_SC201CS_SLAVE_MIPI_2M_30FPS_10BIT);
    if (!pSnsObj)
    {
        my_mi_use_unlock();
        return -1;
    }
    unsigned char b0x3e00Value, b0x3e01Value, b0x3e02Value;

    b0x3e00Value = (unsigned char)(value >> 12);
    b0x3e01Value = (unsigned char)((value & 0xFF0) >> 4);
    b0x3e02Value = (unsigned char)((value & 0xF) << 4);

    pSnsObj->pfnWriteRegEx(vi_pipe, 0x3e00, b0x3e00Value, id == TC_MIPI_CAM_LEFT);
    pSnsObj->pfnWriteRegEx(vi_pipe, 0x3e01, b0x3e01Value, id == TC_MIPI_CAM_LEFT);
    pSnsObj->pfnWriteRegEx(vi_pipe, 0x3e02, b0x3e02Value, id == TC_MIPI_CAM_LEFT);

    my_mi_use_unlock();

    dbug_printf("Set ExP: %d, %d\n", id, value);

    return 0;
}
int camera_get_exp_byreg(int id)
{
    // int value = 0;
    // value = camera_get_regval(id, 0x01) & 0xFF;
    // value = value | ((camera_get_regval(id, 0x02) << 8) & 0xFF00);
    // return value;
    return 0;
}

int camera_set_gain_byreg(int id, int value, int nFineValue)
{
    int vi_pipe = 0;
    ISP_SNS_OBJ_S *pSnsObj = NULL;

#if (USE_3M_MODE)
    if (id == MIPI_1_CAM)
        return 0;
    vi_pipe = 1;
#endif
    my_mi_use_lock();
    pSnsObj = getSnsObj(!USE_3M_MODE ? SMS_SC201CS_MIPI_2M_30FPS_10BIT : SMS_SC201CS_SLAVE_MIPI_2M_30FPS_10BIT);
    if (!pSnsObj)
    {
        my_mi_use_unlock();
        return -1;
    }

    pSnsObj->pfnWriteRegEx(vi_pipe, 0x3e09, (unsigned char)value, id == TC_MIPI_CAM_LEFT);
    pSnsObj->pfnWriteRegEx(vi_pipe, 0x3e06, 0x00, id == TC_MIPI_CAM_LEFT);
    pSnsObj->pfnWriteRegEx(vi_pipe, 0x3e07, (unsigned char)nFineValue, id == TC_MIPI_CAM_LEFT);

    my_mi_use_unlock();

    dbug_printf("Set Gain: %d, %d\n", id, value);
    return 0;
}

int camera_set_irled(int enable, int count)
{
    int ret = -1;
    //my_printf("----------  camera_set_irled\n");

    return ret;
}
