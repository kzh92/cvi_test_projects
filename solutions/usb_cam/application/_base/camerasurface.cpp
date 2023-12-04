#include "camerasurface.h"

//#if (!USE_VDBTASK)

#include "EngineStruct.h"
#include "settings.h"
#include "shared.h"
#include "i2cbase.h"
#include "drv_gpio.h"
#include "msg.h"
#include "engineparam.h"
#include "DBManager.h"
#include "FaceRetrievalSystem.h"
#include "ImageProcessing.h"
#include "jpge.h"
#include "mutex.h"
#include <cvi_base.h>
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_buffer.h"
#include "vfs.h"
#include "cvi_vpss.h"
#include "media_video.h"
#include <fcntl.h>

#include <errno.h>
#include <string.h>

#if __riscv_vector
#include <riscv_vector.h>
#endif

#define VALID_V_VALUE               40
#define GOOD_RATE                   0.1
#define CERTAIN_INVALID_V_VALUE     20

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif  //  MIN

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif  //  MAX

mythread_ptr       g_capture0 = 0;
mythread_ptr       g_capture1 = 0;
mythread_ptr       g_capture2 = 0;
// pthread_mutex_t g_faceDetectionMutex = PTHREAD_MUTEX_INITIALIZER;

mymutex_ptr         g_irWriteLock = 0;
int                 g_irWriteCond = 0;
mymutex_ptr         g_irWriteLock2 = 0;
int                 g_irWriteCond2 = 0;
mymutex_ptr         g_irOffWriteLock = 0;
int                 g_irOffWriteCond = 0;
mymutex_ptr         g_irOffWriteLock2 = 0;
int                 g_irOffWriteCond2 = 0;

mymutex_ptr g_captureLock = 0;

unsigned char*  g_irOnData1 = NULL;
unsigned char*  g_irOnData2 = NULL;
#if (USE_WHITE_LED != 1)
unsigned char*  g_iUVCIRDataY = NULL;
unsigned char*  g_iUVCIRDataU = NULL;
#endif
int             g_iCamCounter = 0;
int             g_nGainClr = 0;
int             g_nExposureClr = 0;

int             g_iDvpCamInited = -1;
int             g_iMipiCamInited = -1;
float           g_rAppStartTime = 0;
int             g_iTwoCamFlag = -1;
int             g_iFirstCamFlag = 0;
float           g_rFirstCamTime = 0;
int             g_iLedOffFrameFlag = 0;
int             g_iLedOffFlag = 0;

int             g_iLedOnStatus = 0;
int             g_iFirstCam1Frame = 0;
int             g_iFirstCam2Frame = 0;

#if (USE_VDBTASK)
#include "check_camera_pattern.h"
mymutex_ptr         g_clrWriteLock = 0;
int                 g_clrWriteCond = 0;
int             g_iClrAuto = 0;

unsigned char*  g_clrYuvData_tmp = NULL;
unsigned char*  g_clrYuvData_tmp1 = NULL;
unsigned char*  g_clrYuvData = NULL;
unsigned char*  g_clrRgbData = NULL;

int             g_iCapturedClrFlag = CLR_CAPTURE_NONE;
int             g_nClrFramePassCount = 0;
int             g_nPrevYAVGSetten = 0;
int             g_nPrevEntireYAVG = 0;
float           g_rPrevValue = 0;
int             g_nValueSettenPassed = 0;

#define LIMIT_INCREASE_RATE_UPPER       2.0f
#define LIMIT_INCREASE_RATE_UPPER_FACE  1.5f
#define LIMIT_DECREASE_RATE_UNDER       0.3f
#define LIMIT_DECREASE_RATE_UNDER_FACE  0.5f


#define FACE_MIN_CLR_USER_LUM       100//60
#define FACE_MAX_CLR_USER_LUM       150//160
#define SCREEN_MIN_CLR_USER_LUM     100
#define SCREEN_MAX_CLR_USER_LUM     150//165
#define SCREEN_MID_CLR_USER_LUM     130//150

#define SCREEN_CLR_LUM_LIMMIT       220
#endif // USE_VDBTASK

#define wait_camera_ready_with_param(st_port, st_buf, st_handle, n_timeout_ms, ret) \
    do { \
        int _oldTime = (int)Now(); \
        ret = -1; \
        while(1) \
        { \
            if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&(st_port), &(st_buf), &(st_handle))) \
            { \
                ret = 0; \
                break; \
            } \
            if ((int)Now() - _oldTime - n_timeout_ms > 0 ) \
                break; \
            my_usleep(1000); \
        } \
    } while(0)

void genIROffData10bit(void* bufOrg, void* bufDst, int width, int height);

void StartFirstCam()
{
    dbug_printf("[%s] start, %0.3f\n", __func__, Now());
    g_irWriteLock = my_mutex_init();
    g_irWriteLock2 = my_mutex_init();
    g_captureLock = my_mutex_init();

    g_irOnData1 = (unsigned char*)my_malloc(IR_BUFFER_SIZE * 2);
    if (g_irOnData1 == NULL)
        my_printf("malloc fail(%s:%d)", __FILE__, __LINE__);
    g_irOnData2 = g_irOnData1 + IR_BUFFER_SIZE;
#if (USE_WHITE_LED != 1)
    g_iUVCIRDataY = g_irOnData2;
#endif
#if (USE_VDBTASK)
    // g_clrWriteLock = my_mutex_init();
    // g_clrYuvData_tmp = (unsigned char*)my_malloc(CLR_CAM_WIDTH * ALIGN_16B(CLR_CAM_HEIGHT) * 2);
    // if (g_clrYuvData_tmp == NULL)
    //     my_printf("malloc fail(%s:%d)", __FILE__, __LINE__);
    // g_clrYuvData = (unsigned char*)my_malloc(CLR_CAM_WIDTH * ALIGN_16B(CLR_CAM_HEIGHT) * 2);
    // if (g_clrYuvData == NULL)
    //     my_printf("malloc fail(%s:%d)", __FILE__, __LINE__);
#endif // USE_VDBTASK
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
    g_iMipiCamInited = camera_init(MIPI_1_CAM, IR_CAM_WIDTH, IR_CAM_HEIGHT, MIPI_CAM_S2RIGHT);
#else
    g_iMipiCamInited = camera_init(MIPI_1_CAM, IR_CAM_WIDTH, IR_CAM_HEIGHT, MIPI_CAM_S2LEFT);
#endif
    if(g_iMipiCamInited == 0)
    {
        fr_InitIRCamera_ExpGain();
        CalcNextExposure();
    }
}

#if (USE_VDBTASK)
void StartClrCam()
{
    // if(g_xSS.iRunningDvpCam == 0)
    // {
    //     g_xSS.iRunningDvpCam = 1;
    //     my_thread_create_ext(&g_capture1, 0, ProcessDVPCapture, NULL, (char*)"getdvp1", 8192, 0);
    // }
}

void StopClrCam()
{
    // g_xSS.iRunningDvpCam = 0;
    // if(g_capture1)
    // {
    //     my_thread_join(&g_capture1);
    //     g_capture1 = 0;
    // }
}
#endif // USE_VDBTASK

void StartCamSurface(int iMode)
{
#if (USE_VDBTASK && 0)
#else // USE_VDBTASK
    g_xSS.iRunningCamSurface = 1;

    g_iDvpCamInited = 0;

    //init tc mipi camera
    if(g_iMipiCamInited == -1)
    {
        float r = Now();
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
        g_iMipiCamInited = camera_init(MIPI_1_CAM, IR_CAM_WIDTH, IR_CAM_HEIGHT, MIPI_CAM_S2RIGHT);
#else
        g_iMipiCamInited = camera_init(MIPI_1_CAM, IR_CAM_WIDTH, IR_CAM_HEIGHT, MIPI_CAM_S2LEFT);
#endif
        if(Now() - r > 500)
            my_printf("$$$$$$$$$$$$$$$$$  MIPI_1_ERROR:   %f\n", Now() - r);

        if(g_iMipiCamInited == -1)
        {
            g_xSS.iCamError |= CAM_ERROR_MIPI1;
        }
    }

    if(iMode == 0 && g_iMipiCamInited == 0)
    {
        fr_InitIRCamera_ExpGain();
        CalcNextExposure();
    }

    if(g_iMipiCamInited == 0 && g_capture0 == 0)
        my_thread_create_ext(&g_capture0, 0, ProcessTCMipiCapture, NULL, (char*)"getmipi1", 8192, MYTHREAD_PRIORITY_HIGH);

#endif // USE_VDBTASK
}

void StopCamSurface()
{
    if (g_xSS.bCheckFirmware && g_irOnData1)
        my_free(g_irOnData1);
    return; //kkk test
    g_xSS.iRunningCamSurface = 0;
#if (USE_VDBTASK)
    if(g_capture0)
    {
        my_thread_join(&g_capture0);
        g_capture0 = 0;
    }

    if(g_iMipiCamInited != -1)
    {
        camera_switch(TC_MIPI_CAM, MIPI_CAM_S2RIGHT);
        my_usleep(100 * 1000);

        camera_release(TC_MIPI_CAM);
    }
#else // USE_VDBTASK
    if(g_capture1)
    {
        my_thread_join(&g_capture1);
        g_capture1 = 0;
    }

    if(g_iDvpCamInited != -1)
        camera_release(MIPI_0_CAM);

    if(g_capture0)
    {
        my_thread_join(&g_capture0);
        g_capture0 = 0;
    }

    if(g_iMipiCamInited != -1)
        camera_release(MIPI_1_CAM);

    g_iDvpCamInited = -1;
#endif // USE_VDBTASK

    WaitIRCancel();
    WaitIRCancel2();

    g_iMipiCamInited = -1;

    GPIO_fast_setvalue(IR_LED, OFF);
}

void* ProcessDVPCapture(void */*param*/)
{
    dbug_printf("ProcessDVPCapture\n");
#if (USE_VDBTASK)
#if 0
    int iTestMode = 0;
    g_iClrAuto = 0;
    int ret;
    //int iCheckFPS = 0;

    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;

    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32DevId = DVP_CAM;
    stChnPort.u32ChnId = DVP_CAM * 4;
    stChnPort.u32PortId = 0;

    int iSwitchToIRFlag = 0;
    float rDarkFirstTime = 0;
#if 0 //darkhorse
    for(int i = 0; i < 100; i++)
    {
        if(g_iFirstIRCamInit == 1 && g_xSS.rAppStartTime != 0) //색카메라초기화가 첫인증을 위한 적외선카메라초기화보다 후에 진행되게 한다.
            break;

        my_usleep(10 * 1000);
    }
#endif
    int iCamInited = -1;
    for(int i = 0; i < 2; i++)
    {
        iCamInited = camera_init(DVP_CAM, CLR_CAM_WIDTH, CLR_CAM_HEIGHT, 0);
        if(iCamInited == -1)
            break;

        ret = wait_camera_ready(DVP_CAM);
        if(ret == 0)
        {
            my_printf("[%s] clr frame is ok, %d\n", __FUNCTION__, i);
            break;
        }
        else
        {
            iCamInited = -1;
            camera_release(DVP_CAM);
        }
    }
    
    g_iDvpCamInited = iCamInited;
    if(g_iDvpCamInited == -1)
    {
#if 0
        if(g_xSS.iAppType == APP_MAIN)
            SendGlobalMsg(MSG_ERROR, ERROR_CAMERA_DVP, 0, 0);
#endif
        g_xSS.iCamError |= CAM_ERROR_DVP2;

        g_xSS.iRunningDvpCam = 0;
        my_thread_exit(NULL);
        return NULL;
    }

    if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
    {
        g_xSS.iCamError &= ~CAM_ERROR_DVP2;
    }

    my_mi_use_lock();
    camera_set_regval(DVP_CAM, 0xfe, 0); //select page
    camera_set_regval(DVP_CAM, 0xb6, 1); //AEC enable

#if (USE_SHENAO_VDB)
    camera_set_regval(DVP_CAM, 0xfe, 0x00);
    camera_set_regval(DVP_CAM, 0xad, 0x80);//R_ratio
    camera_set_regval(DVP_CAM, 0xae, 0x80);//G_ratio
    camera_set_regval(DVP_CAM, 0xaf, 0x80);//B_ratio

    camera_set_regval(DVP_CAM, 0xb3, 0x54);//R_Gain
    camera_set_regval(DVP_CAM, 0xb5, 0x58);//B_Gain

    camera_set_regval(DVP_CAM, 0xfe, 0x01);
    camera_set_regval(DVP_CAM, 0x13, UVC_CLR_LUMINANCE);//luminance
#elif (USE_SHENAO_NEW_VDB)
    camera_set_regval(DVP_CAM, 0xfe, 0x00);
    camera_set_regval(DVP_CAM, 0xad, 0x80);//R_ratio
    camera_set_regval(DVP_CAM, 0xae, 0x80);//G_ratio
    camera_set_regval(DVP_CAM, 0xaf, 0x80);//B_ratio

    camera_set_regval(DVP_CAM, 0xb3, UVC_CLR_R_GAIN);//R_Gain
    camera_set_regval(DVP_CAM, 0xb4, UVC_CLR_G_GAIN);//G_Gain
    camera_set_regval(DVP_CAM, 0xb5, UVC_CLR_B_GAIN);//B_Gain

    camera_set_regval(DVP_CAM, 0xfe, 0x01);
    camera_set_regval(DVP_CAM, 0x13, UVC_CLR_LUMINANCE);//luminance

    camera_set_regval(DVP_CAM, 0xfe, 0x02);
    camera_set_regval(DVP_CAM, 0xd0, UVC_CLR_SAT_Gl);//Global saturation
    camera_set_regval(DVP_CAM, 0xd1, UVC_CLR_SAT_Cb);//Cb saturation
    camera_set_regval(DVP_CAM, 0xd2, UVC_CLR_SAT_Cr);//Cr saturation

    camera_set_regval(DVP_CAM, 0x97, UVC_CLR_SHARP);//sharpness
#else // USE_SHENAO_VDB
    camera_set_regval(DVP_CAM, 0xfe, 0x01);
    camera_set_regval(DVP_CAM, 0x13, UVC_CLR_LUMINANCE);//luminance
#endif // USE_SHENAO_VDB
    camera_set_regval(DVP_CAM, 0xfe, 0x01);

    camera_set_regval(DVP_CAM, 0x01, 0x1b);//AEC measure window x1
    camera_set_regval(DVP_CAM, 0x02, 0x85);//AEC measure window x2
    camera_set_regval(DVP_CAM, 0x03, 0x11);//AEC measure window y1
    camera_set_regval(DVP_CAM, 0x04, 0x6b);//AEC measure window y2
    my_mi_use_unlock();

    int iFrameCount = 0;
    int iFirst = 0;
    int iShowFirstFram = 1;
    int nAverageIdx = 0;
    int anAverageValues[CHECK_CLR_AVERAGE_NUM];
#if (USE_WIFI_MODULE)
    int iRecogQRMode = 0;
#endif
    //float rOld = Now();
    while(g_xSS.iRunningDvpCam || g_iCapturedClrFlag == CLR_CAPTURE_START)
    {
        if (g_xSS.iStartOta || g_xSS.iMState == MS_OTA) break;

#if (USE_WIFI_MODULE)
        // if(g_xSS.iSwitchToIR == 1 && g_xSS.iRecogQRMode != 1)
        // {
        //     if(g_xSS.iRunningDvpCam == 0)
        //         break;

        //     my_usleep(100*1000);
        //     continue;
        // }
        if (g_xSS.iRecogQRMode && iRecogQRMode == 0)
        {
            iRecogQRMode = 1;
            my_mi_use_lock();
            camera_set_regval(DVP_CAM, 0xfe, 0x01);
            camera_set_regval(DVP_CAM, 0x13, 0x40);
            my_mi_use_unlock();
        }
#endif // USE_WIFI_MODULE

        ret = wait_camera_ready (DVP_CAM);
        if (ret == -1 || ret == -2)
        {
            SendGlobalMsg(MSG_ERROR, ERROR_CAMERA_DVP, 0, 0);
            break;
        }

        if (MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &hHandle) != MI_SUCCESS)
        {
            //my_printf("[DVP]  Getting Clr Buffer is failed\n");
            my_usleep(5000);
            continue;
        }

        unsigned char* pbSrc = (unsigned char*)stBufInfo.stFrameData.pVirAddr[0];
        if(pbSrc)
        {
            lockClrBuffer();
            memcpy(g_clrYuvData_tmp, pbSrc, stBufInfo.stFrameData.u32BufSize);
            unlockClrBuffer();
            //my_printf("dvp capture: %d, %d, %f, %f\n", iFrameCount, stBufInfo.stFrameData.u32BufSize, Now() - rOld, Now());
            //rOld = Now();
        }

        if(iFirst == 0)
        {
            if(iFrameCount < 4) //첫 4개프레임에서 등록기값반영이 안되는 문제가 있어서 리용하지 않음
            {
                MI_SYS_ChnOutputPortPutBuf(hHandle);
                iFrameCount++;
                continue;
            }
            //첫 색카메라화상이면 발기를 판정하여 적외선카메라로 절환할것을 결정한다.
            int iAverage = 0;
            int nImgSize = WIDTH_1280 * HEIGHT_960;

            for(int i = 0; i < nImgSize; i += 2)
                iAverage += *(g_clrYuvData_tmp + i * 2);

            iAverage = iAverage / (WIDTH_1280 * HEIGHT_960 / 2);
            my_printf("[Clr] avg=%d\n", iAverage);

            if (iAverage == 255) //may be error on image.
            {
                my_printf(" ---------------- first color camera error ------ \n");
                MI_SYS_ChnOutputPortPutBuf(hHandle);
                iFrameCount ++;
                continue;
            }
            if (iAverage < CHECK_CLR_IR_SWITCH_THR && g_xSS.iUsbHostMode == 0 && g_xSS.iDemoMode != N_DEMO_FACTORY_MODE
#if (USE_WIFI_MODULE)
                && iRecogQRMode == 0
#endif // USE_WIFI_MODULE
            )
            {
                my_printf("[Clr] s1.IR\n");
                iFirst = 1;
                break;
            }
#if 0
//            //카메라화상이 너무 밝을때 카메라조종을 진행한다.
            if (iAverage >= 200)
            {
                iShowFirstFram = -2;

                camera_set_regval(DVP_CAM, 0xfe, 1); //select page
                camera_set_regval(DVP_CAM, 0x13, 0x5A); //luminance
            }
            else if (iAverage >= 180)
            {
                iShowFirstFram = -2;
                camera_set_regval(DVP_CAM, 0xfe, 1); //select page
                camera_set_regval(DVP_CAM, 0x13, 0x5A); //luminance
            }
#endif
            iFirst = 1;
            my_printf("[Clr] 1.frame=%0.3f, %0.3f\n", Now() - g_rAppStartTime, GetMonoTime());

            if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
            {
                camera_set_pattern_mode(CAM_ID_CLR, 1);
                iTestMode = 1;
            }
        }
        else
        {
            int nCurrAverage = CalcClrNextExposure(g_clrYuvData_tmp);
            if (nAverageIdx >= CHECK_CLR_AVERAGE_NUM)
            {
                memmove(anAverageValues, anAverageValues + 1, (CHECK_CLR_AVERAGE_NUM - 1) * 4);
                nAverageIdx = CHECK_CLR_AVERAGE_NUM - 1;
            }
            anAverageValues[nAverageIdx] = nCurrAverage;
            nAverageIdx ++;
            if (nAverageIdx >= CHECK_CLR_AVERAGE_NUM && g_xSS.iUsbHostMode == 0 && g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
            {
                int i = 0;
                for ( ; i < nAverageIdx ; i ++)
                {
                    if (anAverageValues[i] > CHECK_CLR_IR_SWITCH_THR)
                        break;
                }
                if (i >= nAverageIdx)   //switch ir camera
                {
                    if (iShowFirstFram > 4 && g_iClrAuto == 0)
                    {
                        if (iSwitchToIRFlag == 0)
                        {
                            rDarkFirstTime = Now();
                            iSwitchToIRFlag = 1;
                        }
                        else if(Now() - rDarkFirstTime > 1000)
                        {
                            //적외선카메라로 절환한다.
                            my_printf("[Clr] s2.IR\n");
                            break;
                        }
                    }
                    nAverageIdx = 0;
                }
                else
                    iSwitchToIRFlag = 0;
            }
        }

        if (iTestMode > 0 && iFrameCount - iTestMode >= 6 && g_xSS.rFaceEngineTime != 0)
        {
            int res1 = 0;
            float _tmpTime = Now();
            res1 = checkCameraPattern_GC2145_clr(g_clrYuvData_tmp);
            my_printf("** check clr cam, %d, ret: %d, %0.3f, %0.3f\n", iFrameCount, res1, Now() - _tmpTime, Now());
            if (res1 && ((g_xSS.iCamError & CAM_ERROR_CLR_PATTERN) == 0))
            {
                if (res1 == CAMERA_ERROR)
                    g_xSS.iCamError |= CAM_ERROR_CLR_PATTERN;
                else
                    g_xSS.iCamError |= CAM_ERROR_CLR_PATTERN_2;
#if 0//darkhorse
                mount(TMP_DEVNAME, "/db1", TMP_FSTYPE, MS_NOATIME, "");;
                FILE* _fp = fopen("/db1/clr1.bin", "w");
                my_printf("going to save clr cam, %p.\n", _fp);
                if (_fp != NULL)
                {
                    fwrite(g_clrYuvData_tmp, 1, WIDTH_1280*HEIGHT_960*2, _fp);
                    fsync(fileno(_fp));
                    fclose(_fp);
                }
#endif
            }
            g_xSS.iCamError |= CAM_ERROR_CLR_CHECKED;
        }
        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE && iTestMode == 0)
        {
            camera_set_pattern_mode(CAM_ID_CLR, 1);
            iTestMode = iFrameCount;
        }

        if(iShowFirstFram > 0)
        {
            if(g_iCapturedClrFlag == CLR_CAPTURE_START)
            {
                LOG_PRINT("captured clr face %f\n", Now());
                g_iCapturedClrFlag = CLR_CAPTURE_END;
            }
            WaitClrCancel();
        }

        MI_SYS_ChnOutputPortPutBuf(hHandle);

        iFrameCount ++;
        iShowFirstFram ++;
        my_usleep(5000);
    }

    if(g_iDvpCamInited != -1)
    {
#if (AUTO_CTRL_CLR_CAM == 0)
        camera_set_regval(DVP_CAM, 0x13, 0x0a);
#endif
        camera_release(DVP_CAM);
    }

    g_iDvpCamInited = -1;

    WaitClrCancel();

    my_thread_exit(NULL);
#endif
#else // USE_VDBTASK

#endif // USE_VDBTASK
    return NULL;
}

#if (USE_WHITE_LED != 1)
int PrepareDataForVenc(uint8_t* idstBuf, uint8_t* isrcBuf, int iWidth, int iHeight, int rotate)
{
    // memcpy(idstBuf, isrcBuf, iWidth * iHeight);
    // if (rotate)
    // 	rotateImage_inner(idstBuf, iWidth, iHeight, 180);
    memset(idstBuf, 0, iWidth * iHeight);
    int crop_width, crop_height;

    if(iWidth * g_xSS.iUvcHeight / g_xSS.iUvcWidth > iHeight)
    {
        crop_height = iHeight;
        crop_width = iHeight * g_xSS.iUvcWidth / g_xSS.iUvcHeight;
    }
    else
    {
        crop_width = iWidth;
        crop_height =  iWidth * g_xSS.iUvcHeight / g_xSS.iUvcWidth;
    }

    CVI_U32 xStart = 0, yStart = 0;

    xStart = (iWidth - crop_width) / 2;
    yStart = (iHeight - crop_height) / 2;

    for(int i = 0; i < crop_height; i++)
    {
        for(int j = 0; j < crop_width; j++)
        {
            if (rotate == 0)
                idstBuf[i * crop_width + j] = isrcBuf[(yStart + i) * iWidth + (xStart + j)];
            else
                idstBuf[i * crop_width + j] = isrcBuf[((iHeight - yStart) - i) * iWidth + ((iWidth - xStart) - j)];
        }
    }
    remove_white_point_riscv(idstBuf, crop_width, crop_height);
    convert_bayer2y_rotate_cm(idstBuf, crop_width, crop_height);
    Shrink_Grey(idstBuf, crop_height, crop_width, idstBuf, g_xSS.iUvcHeight, g_xSS.iUvcWidth);

    if (g_xSS.iUvcResChanged)
    {
        free(g_iUVCIRDataU);
        g_iUVCIRDataU = NULL;
        g_xSS.iUvcResChanged = 0;
    }

    if(g_iUVCIRDataU == NULL)
    {
        g_iUVCIRDataU = (unsigned char*)malloc(g_xSS.iUvcWidth * g_xSS.iUvcHeight / 2);
        if (g_iUVCIRDataU == NULL)
            return CVI_FALSE;
    }

    return CVI_SUCCESS;
}
#endif

void* ProcessTCMipiCapture(void */*param*/)
{
    VIDEO_FRAME_INFO_S stVideoFrame[2];
    VI_DUMP_ATTR_S attr[2];
    int frm_num = 1;
    CVI_U32 dev = 0;
    CVI_S32 s_ret = 0;
    // int pat_set = 0;
    int iFrameCount = 0;
    int iNeedNext = 0;
    float rOld = Now();
#if (USE_VDBTASK)
    int iClrCheck = 0;
#endif
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
    unsigned int iClrFrameCount = 0;
#endif

    dbug_printf("[%s]\n", __func__);

    CVI_U32 devCount = (CONFIG_SNS0_TYPE > 0) + (CONFIG_SNS1_TYPE > 0);
    for (dev = 0; dev < devCount; dev ++)
    {
        attr[dev].bEnable = 1;
        attr[dev].u32Depth = 0;
        attr[dev].enDumpType = VI_DUMP_TYPE_RAW;

        if (CVI_VI_SetPipeDumpAttr(dev, &attr[dev]) != CVI_SUCCESS)
            my_printf("dev=%d SetPipeDumpAttr failed\n", dev);
    }

    while (g_xSS.iRunningCamSurface)
    {
        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE/* && pat_set == 0*/)
        {
            // pat_set = 1;
            camera_set_pattern_mode(TC_MIPI_CAM, 1);
        }
#if (USE_WHITE_LED != 1)
        if (g_xSS.iUvcSensor != DEFAULT_SNR4UVC && g_xSS.bUVCRunning && g_xSS.rFaceEngineTime == 0 && g_iTwoCamFlag == IR_CAMERA_STEP_IDLE && g_xSS.iUVCIRDataReady == 0)
        {
            camera_set_irled_on(1);
            g_iTwoCamFlag = IR_CAMERA_STEP0;
        }
#endif
        if (g_xSS.iStartOta || g_xSS.iMState == MS_OTA || g_xSS.bCheckFirmware) break;

        frm_num = 1;

        memset(stVideoFrame, 0, sizeof(stVideoFrame));
        stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
        stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

#if (DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_121)
        dev = (camera_get_actIR() == MIPI_CAM_S2LEFT ? (DEFAULT_SNR4UVC + 1) % 2: DEFAULT_SNR4UVC);
#else
        dev = 0;
#endif
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
        if (g_xSS.rFaceEngineTime == 0 && camera_get_actIR() == MIPI_CAM_S2LEFT && g_xSS.iUvcSensor == DEFAULT_SNR4UVC)
        {
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2RIGHT);
        }
        if (iClrFrameCount)
        {
            iClrFrameCount--;
            CVI_ISP_GetVDTimeOut(0, ISP_VD_FE_END, 1000);
            if (iClrFrameCount == 0)
                camera_switch(TC_MIPI_CAM, MIPI_CAM_S2LEFT);
            continue;
        }
#endif
        if (g_iTwoCamFlag == IR_CAMERA_STEP_IDLE && g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
        {
            my_usleep(1000);
            continue;
        }
#if (USE_VDBTASK)
        if (iClrCheck == 0 && g_xSS.rFaceEngineTime == 0 && g_iTwoCamFlag == IR_CAMERA_STEP_IDLE && g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
        {
            dev = USE_3M_MODE ? 0 : 1;
        }
#endif
#if (ENGINE_USE_TWO_CAM == EUTC_3V4_MODE || ENGINE_USE_TWO_CAM == EUTC_3M_MODE)
        if(g_iTwoCamFlag == IR_CAMERA_STEP4 && g_iLedOnStatus == 1)
        {
#if (!USE_3M_MODE)
            gpio_irled_on(ON);
#endif
            dev = 0;
            g_iLedOnStatus = 0;
        }
#endif // ENGINE_USE_TWO_CAM
        s_ret = CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);
        if (s_ret != CVI_SUCCESS)
        {
#if (USE_3M_MODE)
            g_xSS.iCamError = (dev == 0 ? CAM_ERROR_DVP2 : CAM_ERROR_MIPI2);
#elif (USE_VDBTASK) // USE_3M_MODE
            if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE && dev == 1)
            {
                g_xSS.iCamError |= CAM_ERROR_DVP2;
                my_usleep(5000);
                continue;
            }
            else
#else // USE_3M_MODE
            {
                g_xSS.iCamError = (camera_get_actIR() == MIPI_CAM_S2LEFT ? CAM_ERROR_MIPI1 : CAM_ERROR_MIPI2);
            }
#endif // USE_3M_MODE
            GPIO_fast_setvalue(IR_LED, OFF);
            break;
        }
        if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
            frm_num = 2;

        if (iFrameCount == 0)
            dbug_printf("image size %d, %d, %0.3f\n", stVideoFrame[0].stVFrame.u32Length[0], frm_num, rOld);
        if (frm_num >= 2)
            dbug_printf("image size2. %d, %d, %0.3f\n", stVideoFrame[0].stVFrame.u32Length[0], frm_num, rOld);

        size_t image_size = stVideoFrame[0].stVFrame.u32Length[0];

        stVideoFrame[0].stVFrame.pu8VirAddr[0] = (CVI_U8 *)stVideoFrame[0].stVFrame.u64PhyAddr[0];

        unsigned char *ptr = (unsigned char*)stVideoFrame[0].stVFrame.pu8VirAddr[0];

        if (g_iTwoCamFlag != -1 && rOld != 0)
            dbug_printf("[%0.1f]mc: %do, %dc, %dt\n", Now(), g_iLedOnStatus, camera_get_actIR(), g_iTwoCamFlag);
        rOld = Now();

#if (USE_VDBTASK)
        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE && iClrCheck == 0 && g_xSS.rFaceEngineTime == 0 && g_iTwoCamFlag == IR_CAMERA_STEP_IDLE && 
            (g_xSS.iAutoUserAdd == 0 || g_xSS.iAutoUserAdd >= 6))
        {
            lockIRBuffer();
            size_t test_pos = 0;
            for (int k = (int)image_size/8 ; k < (int)image_size; k+=3)
            {
                g_irOnData2[test_pos++] = ptr[k];
                g_irOnData2[test_pos++] = ptr[k+1];
                if (test_pos >= IR_CAM_WIDTH * IR_CAM_HEIGHT)
                    break;
            }
            unlockIRBuffer();
            float rTime = Now();
            int res = checkCameraPattern(g_irOnData2);
            my_printf("*** check clr cam=%d, %0.3f\n", res, Now() - rTime);
            if (res)
            {
                if (res == CAMERA_ERROR)
                    g_xSS.iCamError |= CAM_ERROR_CLR_PATTERN;
                else
                    g_xSS.iCamError |= CAM_ERROR_CLR_PATTERN_2;
            }
            g_xSS.iCamError |= CAM_ERROR_CLR_CHECKED;
            iClrCheck = 1;
        }
#endif // USE_VDBTASK
        if((g_iTwoCamFlag == IR_CAMERA_STEP0 && camera_get_actIR() == MIPI_CAM_S2RIGHT))
        {
            //카메라절환할때 등록기설정명령과 app에서 내려보내는 등록기설정명령이 겹치면서 카메라오유가 나오댔음
            //camera_switch를 내려보낸 다음 프레임의 dqbuf하기 전부터 10ms미만에는 카메라등록기설정을 하지 않게 함
            //swtich to id->1
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2LEFT);
#if ((USE_3M_MODE == U3M_DEFAULT || USE_3M_MODE == U3M_SEMI) && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
            if (g_xSS.iFirstFlag == 0)
            {
                g_xSS.iFirstFlag = 2; // get first color frame
                lockIRBuffer();
                size_t test_pos = 0;
                for (int k = (int)image_size/8 ; k < (int)image_size; k+=3)
                {
                    g_irOnData2[test_pos++] = ptr[k];
                    g_irOnData2[test_pos++] = ptr[k+1];
                    if (test_pos >= IR_CAM_WIDTH * IR_CAM_HEIGHT)
                        break;
                }
                unlockIRBuffer();
                WaitIRCancel2();
            }
            else
                g_xSS.iFirstFlag = 1;
#endif // USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122
            iNeedNext = 1;
        }

        ///Sub0카메라로 동작할때 레드켜기지령을 받았으면 Sub0화상을 얻고 Sub1로 절환하여 두번째화상을 얻은다음 다시 Sub0으로 카메라를 절환한다.
        if(g_iTwoCamFlag == IR_CAMERA_STEP0 && iNeedNext == 0/* && reserved == 1 && id == MIPI_CAM_SUB0 && iOldReserved == 0*/)
        {
            if(g_iLedOnStatus == 1)
            {
                g_iLedOnStatus = 0;
                gpio_irled_on(ON);
#if (USE_WHITE_LED != 1)
                if (g_xSS.bUVCRunning && g_xSS.iUvcSensor != DEFAULT_SNR4UVC)
                {
                    CVI_VI_StartPipe(0);
                }
#endif // USE_WHITE_LED
            }

            lockIROffBuffer();
            genIROffData10bit(ptr + (int)image_size/8, fr_GetOffImageBuffer(), IR_CAM_WIDTH, IR_CAM_HEIGHT);
            unlockIROffBuffer();
            g_iLedOffFrameFlag = LEFT_IROFF_CAM_RECVED;
            WaitIROffCancel();
            g_iTwoCamFlag ++;
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
            if (g_xSS.bUVCRunning && g_xSS.iUvcSensor == DEFAULT_SNR4UVC)
            {
                camera_switch(TC_MIPI_CAM, MIPI_CAM_S2RIGHT);
                iClrFrameCount = DEFAULT_CLR_IR_FRAME_RATIO;
            }
#endif
        }
        else if(g_iTwoCamFlag == IR_CAMERA_STEP1 && g_iTwoCamFlag != IR_CAMERA_STEP2)
        {
            g_iTwoCamFlag ++;
        }
        else if(g_iTwoCamFlag == IR_CAMERA_STEP2)
        {
#if (USE_3M_MODE)
#if (USE_WHITE_LED != 1)
            if (g_xSS.bUVCRunning && g_xSS.iUvcSensor != DEFAULT_SNR4UVC)
                CVI_VI_StopPipe(0);
#endif // USE_WHITE_LED
            gpio_irled_on(OFF);
#if (DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
            if (g_xSS.iUvcSensor != DEFAULT_SNR4UVC)
            {
            }
            else
                camera_switch(TC_MIPI_CAM, MIPI_CAM_S2RIGHT);
#endif
#else
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2RIGHT);
#endif
            lockIRBuffer();
            size_t test_pos = 0;
            for (int k = (int)image_size/8 ; k < (int)image_size; k+=3)
            {
                g_irOnData1[test_pos++] = ptr[k];
                g_irOnData1[test_pos++] = ptr[k+1];
                if (test_pos >= IR_CAM_WIDTH * IR_CAM_HEIGHT)
                    break;
            }
            unlockIRBuffer();

#if (USE_VDBTASK && !USE_3M_MODE)
            if (g_xSS.rFaceEngineTime == 0 && g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
            {
                fr_CalcScreenValue(g_irOnData1, IR_SCREEN_CAMERAVIEW_MODE);
                CalcNextExposure();
            }
#endif // USE_VDBTASK
            WaitIRCancel();
#if (ENGINE_USE_TWO_CAM != EUTC_3V4_MODE && ENGINE_USE_TWO_CAM != EUTC_3M_MODE)
            g_iTwoCamFlag ++;
#else // ENGINE_USE_TWO_CAM
            WaitIRCancel();
#if (USE_3M_MODE == U3M_DEFAULT || USE_3M_MODE == U3M_SEMI)
            if (g_xSS.iFirstFlag == 0 && g_iTwoCamFlag == IR_CAMERA_STEP2)
            {
                g_xSS.iFirstFlag = 1;
                g_iTwoCamFlag ++;
            }
            else
#endif // USE_3M_MODE
            {
                gpio_irled_on(OFF);
                g_iTwoCamFlag = IR_CAMERA_STEP_IDLE;
#if (USE_3M_MODE != 1 && USE_WHITE_LED != 1)
                if (g_xSS.bUVCRunning && !g_xSS.iUVCIRDataReady && g_xSS.iUvcSensor != DEFAULT_SNR4UVC)
                {
#if 0
                    if (PrepareDataForVenc(g_iUVCIRDataY, g_irOnData1, IR_CAM_WIDTH, IR_CAM_HEIGHT, 1) == CVI_SUCCESS)
                        g_xSS.iUVCIRDataReady = 1;
                    InsertDataForVenc(g_iUVCIRDataY, g_iUVCIRDataU);
#else
                    g_xSS.iUVCIRDataReady = 0;
                    my_usleep(100*1000);
#endif
                }
#endif // !USE_3M_MODE
            }
#endif // ENGINE_USE_TWO_CAM
        }
#if (ENGINE_USE_TWO_CAM != 0)
        else if(g_iTwoCamFlag == IR_CAMERA_STEP3 && g_iTwoCamFlag != IR_CAMERA_STEP4)
        {
            g_iTwoCamFlag ++;
        }
        else if(g_iTwoCamFlag == IR_CAMERA_STEP4)
        {
#if (ENGINE_USE_TWO_CAM == EUTC_3V4_MODE)
            if (g_iLedOnStatus != 0)
            {
                //irled is still off
                //skip frame
                CVI_VI_ReleasePipeFrame(dev, stVideoFrame);
                continue;
            }
#endif // ENGINE_USE_TWO_CAM == EUTC_3V4_MODE
            g_iLedOnStatus = 0;
            gpio_irled_on(OFF);
#if (!USE_3M_MODE || DEFAULT_CAM_MIPI_TYPE != CAM_MIPI_TY_122)
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2LEFT);
#endif
#if (USE_WHITE_LED != 1)
            g_xSS.iUVCIRDataReady = 0;
#endif
            lockIRBuffer();
            size_t test_pos = 0;
            for (int k = (int)image_size/8 ; k < (int)image_size; k+=3)
            {
                g_irOnData2[test_pos++] = ptr[k];
                g_irOnData2[test_pos++] = ptr[k+1];
                if (test_pos >= IR_CAM_WIDTH * IR_CAM_HEIGHT)
                    break;
            }
            unlockIRBuffer();
            WaitIRCancel2();
            g_iTwoCamFlag = IR_CAMERA_STEP_IDLE;
        }
#endif // ENGINE_USE_TWO_CAM
        else if(iNeedNext == 1)
        {
            iNeedNext = 0;
        }

        CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

        iFrameCount ++;
    }

    if (g_irOnData1)
        my_free(g_irOnData1);

    g_xSS.iShowIrCamera = 0;
    my_thread_exit(NULL);

    return NULL;
}

void CalcNextExposure()
{
    if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
        return;
    
    if(fr_GetExposure())
    {
        camera_set_exp_byreg(TC_MIPI_CAM_LEFT, *fr_GetExposure());
    }
    if(fr_GetExposure2())
    {
        camera_set_exp_byreg(TC_MIPI_CAM_RIGHT, *fr_GetExposure2());
    }
    if(fr_GetGain() && fr_GetFineGain())
    {
        camera_set_gain_byreg(TC_MIPI_CAM_LEFT, *fr_GetGain(), *fr_GetFineGain());
    }
    if(fr_GetGain2() && fr_GetFineGain2())
    {
        camera_set_gain_byreg(TC_MIPI_CAM_RIGHT, *fr_GetGain2(), *fr_GetFineGain2());
    }
}

void reset_ir_exp_gain()
{
    my_mutex_lock(g_captureLock);
    if(g_iMipiCamInited == 0)
    {
        fr_InitIRCamera_ExpGain();
        CalcNextExposure();
    }
    my_mutex_unlock(g_captureLock);
}

#if (USE_VDBTASK)
void rotateYUV420SP_flip(unsigned char* src, int width, int height, unsigned char* dst, int angle, int flip)
{
    int wh = width * height;
    int i, j, k = 0;

    if(angle == 90)
    {
        for(i = 0; i < width; i++) {
            for(j = 0; j < height; j++) {
                if(flip == 0)
                    dst[k + j] = src[width * j + (width - i - 1)];
                else
                    dst[k + height - j - 1] = src[width * j + (width - i - 1)];
            }
            k+= (height);
        }

        for(i = 0; i < width; i += 2) {
            for(j = 0; j < height / 2; j++) {
                if(flip == 0)
                {
                    dst[j * 2 + k + 1] = src[wh + width * j + (width - i - 1)];
                    dst[j * 2 + k] = src[wh + width * j + (width - i - 1) + 1];
                }
                else
                {
                    dst[(height/2 - j - 1) * 2 + k + 1] = src[wh + width * j + (width - i - 1)];
                    dst[(height/2 - j - 1) * 2 + k] = src[wh + width * j + (width - i - 1) + 1];
                }
            }
            k += (height);
        }
    }
    else if(angle == 270)
    {
        for(i = 0; i < width; i++) {
            for(j = 0; j < height; j++) {
                if(flip == 0)
                    dst[k + height - j - 1] = src[width * j + i];
                else
                    dst[k + j] = src[width * j + i];
            }
            k+= (height);
        }

        for(i = 0; i < width; i += 2) {
            for(j = 0; j < height / 2; j++) {
                if(flip == 0)
                {
                    dst[(height/2 - j - 1) * 2 + k] = src[wh + width * j + i];
                    dst[(height/2 - j - 1) * 2 + k + 1] = src[wh + width * j + i + 1];
                }
                else
                {
                    dst[j * 2 + k] = src[wh + width * j + i];
                    dst[j * 2 + k + 1] = src[wh + width * j + i + 1];
                }
            }
            k += (height);
        }
    }
}

void ConvertYUYV_toYUV420(unsigned char* data, int width, int height, unsigned char* dstData)
{
    unsigned char* pUV = dstData + width * height;
    int iUVFlag = 0;
    for(int i = 0; i < height; i++)
    {
        iUVFlag = (i % 2 == 0) ? 1 : 0;
        for(int j = 0; j < (width >> 1); j++)
        {
            dstData[j * 2] = data[j * 4];
            dstData[j * 2 + 1] = data[j * 4 + 2];

            if(iUVFlag == 1)
            {
                *pUV = data[j * 4 + 1];
                pUV++;
                *pUV = data[j * 4 + 3];
                pUV++;
            }
        }
        data = data + width * 2;
        dstData = dstData + width;
    }
}

int GetYAVGValueOfClr(unsigned char* pbClrBuf)
{
    int nY, nX;
    int nInvaliedPixelCount = 0;
    int nEntireValue = 0;

    int nEntirePixelCount = 0;
    int nEntireTotalValue = 0;

    unsigned char* pbSrcPtr = pbClrBuf;

    for (nX = 0; nX < CLR_CAM_HEIGHT; nX += 2)
    {
        for (nY = 0; nY < CLR_CAM_WIDTH; nY += 2)
        {
            unsigned char bYData = pbSrcPtr[nY * 2];
            if (nX > 120 && nX < CLR_CAM_HEIGHT - 120 && nY > 216 && nY < CLR_CAM_WIDTH - 216)
            {
                nEntirePixelCount++;

                if (bYData < 0)
                {
                    nInvaliedPixelCount++;
                }
                else
                {
                    nEntireTotalValue += bYData;
                }
            }
        }
        pbSrcPtr += CLR_CAM_WIDTH * 2 * 2;
    }

    if (nEntirePixelCount)
    {
        nEntireValue = nEntireTotalValue / nEntirePixelCount;
    }
    else
    {
        nEntireValue = 30;
    }
    return nEntireValue;
}

int CalcClrNextExposure(unsigned char* pbClrBuf, int width, int height)
{
    int sum = 0;
    for (int i = 0; i < width*height; i ++)
        sum += pbClrBuf[i];
    sum = sum / (width * height);
    return sum;
}

int WaitClrTimeout(int iTimeout)
{
    float _oldTime = Now();
    if (iTimeout <= 0)
        return ETIMEDOUT;

    while(1)
    {
        float _diff = Now() - _oldTime;
        // dbug_printf("[%s] _diff = %0.3f\n", __func__, _diff);
        if (_diff > (float)iTimeout)
            break;
        my_mutex_lock(g_clrWriteLock);
        if (g_clrWriteCond == 1)
        {
            g_clrWriteCond = 0;
            my_mutex_unlock(g_clrWriteLock);
            // dbug_printf("[%s] return 0\n", __func__);
            return 0;
        }
        my_mutex_unlock(g_clrWriteLock);
        my_usleep(1000);
    }

    return ETIMEDOUT;
}

int WaitClrCancel()
{
    my_mutex_lock(g_clrWriteLock);
    g_clrWriteCond = 1;
    //dbug_printf("[%s] set 1, %0.3f\n", __func__, Now());
    my_mutex_unlock(g_clrWriteLock);
    return 0;
}

#else// USE_VDBTASK

void* ProcessDVPCaptureFirst(void*)
{
#if 0 //kkk
    if(g_iDvpCamInited == 0)
    {
        int ret;
 
        camera_set_exp_byreg(MIPI_0_CAM, INIT_EXP);
        camera_set_gain_byreg(MIPI_0_CAM, INIT_GAIN);

        for(int i = 0; i < 1; i ++)
        {
            MI_SYS_ChnPort_t stChnPort;
            MI_SYS_BufInfo_t stBufInfo;
            MI_SYS_BUF_HANDLE hHandle;

            memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
            memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
            memset(&hHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
            stChnPort.eModId = E_MI_MODULE_ID_VIF;
            stChnPort.u32DevId = MIPI_0_CAM;
            stChnPort.u32ChnId = MIPI_0_CAM * 4;
            stChnPort.u32PortId = 0;

            wait_camera_ready_with_param(stChnPort, stBufInfo, hHandle, 2000, ret);
            if (ret < 0)
            {
                g_xSS.iCamError |= CAM_ERROR_DVP2;
                GPIO_fast_setvalue(IR_LED, OFF);

                g_iDvpCamInited = -1;
                camera_release(MIPI_0_CAM);
                my_thread_exit(NULL);
                dbug_printf("\t [mipi 0] get first buf timeout  (%f)\n", Now());
                return NULL;
            }

            {
                //if(i == 0)
                {
                    g_iFirstCamFlag |= LEFT_IR_CAM_RECVED;
#if (IR_LED_ONOFF_MODE == 1)
                    if(g_iFirstCamFlag & RIGHT_IR_CAM_RECVED)
                        GPIO_fast_setvalue(IR_LED, OFF);
#endif
                    unsigned short* pbSrc = (unsigned short*)stBufInfo.stFrameData.pVirAddr[0];
                    lockIRBuffer();
                    for(int iIdx = 0; iIdx < WIDTH_1280 * HEIGHT_720; iIdx ++)
                        g_irOnData1[iIdx] = (pbSrc[iIdx] >> 2);
                    unlockIRBuffer();
#if 0
                    FILE* fp = fopen("/mnt/ir_mipi0.bin", "wb");
                    if(fp)
                    {
                        fwrite(g_irOnData1, sizeof(g_irOnData1), 1, fp);
                        fclose(fp);
                    }
#endif
                    g_rFirstCamTime = Now();
                }

                dbug_printf("\t [mipi 0] get first buf ok %d(%f)\n", i, Now());
                MI_SYS_ChnOutputPortPutBuf(hHandle);
            }
        }
#if (IR_LED_ONOFF_MODE == 1)
        camera_set_irled(2, 0);
#endif
    }
    else
    {
        GPIO_fast_setvalue(IR_LED, OFF);
        g_xSS.iCamError |= CAM_ERROR_DVP1;
    }
#endif
    my_thread_exit(NULL);
    return NULL;
}

#endif // USE_VDBTASK

void rotate_image_inplace_square_private(unsigned char* pbBuffer, int nLineStep, int nSize, int nChannel, int nDegreeAngle, int flip)
{
    int nIndex = ((nDegreeAngle + 360) / 90) % 4;
    if (nIndex == 0) return;

    int x, y;
    int nSizeHalf = (nSize + 1) / 2;
    int nSizeHalf_1 = nSize / 2;

    unsigned char *p0, *p1, *p2, *p3;

    int nStep0 = nChannel;                  // TopLet => TopRight
    int nStep1 = nLineStep * nChannel;      // TopRight => BottomRight
    int nStep2 = -nChannel;                 // BottomRight => BottomLeft
    int nStep3 = -nLineStep * nChannel;     // BottomLeft => TopLeft

    int nNextStep0 = (nLineStep - nSizeHalf) * nChannel;
    int nNextStep1 =  (-1 - nLineStep * nSizeHalf) * nChannel;
    int nNextStep2 = (-nLineStep + nSizeHalf) * nChannel;
    int nNextStep3 = (1 + nLineStep * nSizeHalf) * nChannel;

    p0 = pbBuffer;
    p1 = pbBuffer + (nSize - 1) * nChannel;
    p2 = p1 + nLineStep * (nSize - 1) * nChannel;
    p3 = p0 + nLineStep * (nSize - 1) * nChannel;

    if (nIndex == 1)
    {
        if (flip)
        {
            for (y = 0; y < nSize; y++)
            {
                for (x = y + 1; x < nSize; x++)
                {
                    unsigned char t[3];
                    p0 = pbBuffer + y * nLineStep * nChannel + x * nChannel;
                    p1 = pbBuffer + x * nLineStep * nChannel + y * nChannel;
                    memcpy(t, p0, nChannel);
                    memcpy(p0, p1, nChannel);
                    memcpy(p1, t, nChannel);
                }
            }
        }
        else
        {
            for (y = 0; y < nSizeHalf; y++)
            {
                for (x = 0; x < nSizeHalf_1; x++)
                {
                    unsigned char t[3];

                    memcpy(t, p0, nChannel);
                    memcpy(p0, p3, nChannel);
                    memcpy(p3, p2, nChannel);
                    memcpy(p2, p1, nChannel);
                    memcpy(p1, t, nChannel);

                    p0 += nStep0;
                    p1 += nStep1;
                    p2 += nStep2;
                    p3 += nStep3;
                }

                p0 += nNextStep0;
                p1 += nNextStep1;
                p2 += nNextStep2;
                p3 += nNextStep3;
            }
        }
    }
    else if (nIndex == 3)
    {
        if (flip)
        {
            for (y = 0; y < nSize; y++)
            {
                for (x = 0; x < nSize - y - 1; x++)
                {
                    unsigned char t[3];
                    p0 = pbBuffer + y * nLineStep * nChannel + x * nChannel;
                    p1 = pbBuffer + (nSize - x - 1) * nLineStep * nChannel + (nSize - y - 1) * nChannel;
                    memcpy(t, p0, nChannel);
                    memcpy(p0, p1, nChannel);
                    memcpy(p1, t, nChannel);
                }
            }
        }
        else
        {
            for (y = 0; y < nSizeHalf; y++)
            {
                for (x = 0; x < nSizeHalf_1; x++)
                {
                    unsigned char t[3];
                    memcpy(t, p0, nChannel);
                    memcpy(p0, p1, nChannel);
                    memcpy(p1, p2, nChannel);
                    memcpy(p2, p3, nChannel);
                    memcpy(p3, t, nChannel);

                    p0 += nStep0;
                    p1 += nStep1;
                    p2 += nStep2;
                    p3 += nStep3;
                }

                p0 += nNextStep0;
                p1 += nNextStep1;
                p2 += nNextStep2;
                p3 += nNextStep3;
            }
        }
    }
}

int rotate_image_private_gcd(int n1, int n2)
{
    int a, b = n1, d = n2;

    do 
    {
        a = b;
        b = d;
        d = a % b;
    } while (d > 0);

    return b;
}

bool rotate_image_inplace(unsigned char* pbBuffer, int nSrcWidth, int nSrcHeight, int nChannel, int nDegreeAngle, int flip)
{
    int i, nx, ny, s;

    int nIndex = ((nDegreeAngle + 360) / 90) % 4;

    if (nIndex == 0)
    {
        if (flip)
        {
            int nSrcWidth_2 = nSrcWidth / 2;
            int y, x;
            unsigned char t[3];
            unsigned char* p0;
            unsigned char* p1;
            for (y = 0; y < nSrcHeight; y++)
            {
                p0 = pbBuffer + nSrcWidth * nChannel * y;
                p1 = p0 + (nSrcWidth - 1) * nChannel;

                for (x = 0; x < nSrcWidth_2; x++)
                {
                    memcpy(t, p0, nChannel);
                    memcpy(p0, p1, nChannel);
                    memcpy(p1, t, nChannel);
                    p0 += nChannel;
                    p1 -= nChannel;
                }
            }
        }
        return true;
    }

    if (nIndex == 2)
    {
        if (flip)
        {
            int nSrcHeight_2 = nSrcHeight / 2;
            unsigned char* p0 = pbBuffer;
            unsigned char* p1 = pbBuffer + (nSrcHeight - 1) * nSrcWidth * nChannel;
            unsigned char t[3];

            for (i = 0; i < nSrcHeight_2; i++)
            {
                for (int j = 0; j < nSrcWidth; j++)
                {
                    memcpy(t, p0, nChannel);
                    memcpy(p0, p1, nChannel);
                    memcpy(p1, t, nChannel);
                    p0 += nChannel;
                    p1 += nChannel;
                }
                p1 -= (2 * nSrcWidth * nChannel);
            }
        }
        else
        {
            int nSize = nSrcWidth * nSrcHeight;
            int nHalfSize = nSize / 2;

            unsigned char* p0 = pbBuffer;
            unsigned char* p1 = pbBuffer + (nSize - 1) * nChannel;
            unsigned char t[3];

            for (i = 0; i < nHalfSize; i++)
            {
                memcpy(t, p0, nChannel);
                memcpy(p0, p1, nChannel);
                memcpy(p1, t, nChannel);
                p0 += nChannel;
                p1 -= nChannel;
            }
        }
        return true;
    }

    int gcd = rotate_image_private_gcd(nSrcWidth, nSrcHeight);

    if (gcd < 16)
        return false;

    int nSrcWidthSec = nSrcWidth / gcd;
    int nSrcHeightSec = nSrcHeight / gcd;
    int nDstWidthSec = nSrcHeightSec;
    int nDstHeightSec = nSrcWidthSec;
    int nSizeSec = nSrcHeightSec * nSrcWidthSec * gcd; // 14400

    if (nSizeSec > 65535)
        return false;

    unsigned char* pbBufferIt = pbBuffer;
    unsigned char* pb_alloc_buf = (unsigned char*)malloc(nSizeSec * 3 + gcd * nChannel);

    unsigned short *n_rep_idx = (unsigned short*)pb_alloc_buf;                  // (unsigned short*)malloc(nSizeSec * 2);
    unsigned char  *b_rep_idx = (unsigned char*)(pb_alloc_buf + nSizeSec * 2);  // (unsigned char*)malloc(nSizeSec);
    unsigned char  *pb_temp = (unsigned char*)(pb_alloc_buf + nSizeSec * 3);    // (unsigned char*)malloc(gcd * nChannel);

    memset(b_rep_idx, 0, nSizeSec);

    for (ny = 0; ny < nSrcHeightSec; ny++)
    {
        for (nx = 0; nx < nSrcWidthSec; nx++)
        {
            rotate_image_inplace_square_private(pbBufferIt, nSrcWidth, gcd, nChannel, nDegreeAngle, flip);
            pbBufferIt += gcd * nChannel;
        }
        pbBufferIt += (gcd - 1) * nSrcWidth * nChannel;
    }

    for (ny = 0; ny < nSrcHeightSec; ny++)
    {
        for (s = 0; s < gcd; s++)
        {
            int ny_ = (ny * gcd + s);
            for (nx = 0; nx < nSrcWidthSec; nx++)
            {
                int nSrc = ny_ * nSrcWidthSec + nx;
                int nDst = 0;
                int my = 0;
                int mx = 0;

                if (nIndex == 1)
                {
                    if (flip)
                    {
                        mx = ny;
                        my = nx;
                    }
                    else
                    {
                        mx = nDstWidthSec - 1 - ny;
                        my = nx;
                    }
                }
                if (nIndex == 3)
                {
                    if (flip)
                    {
                        mx = nDstWidthSec - 1 - ny;
                        my = nDstHeightSec - 1 - nx;
                    }
                    else
                    {
                        mx = ny;
                        my = nDstHeightSec - 1 - nx;
                    }
                }

                nDst = (my * gcd + s) * nDstWidthSec + mx;
                n_rep_idx[nDst] = nSrc;
            }
        }
    }

    int st = 0, cu, ne;
    
    while (1)
    {
        for (; st < nSizeSec; st++)
        {
            if (b_rep_idx[st] == 0)
                break;
        }
        if (st == nSizeSec) break;

        memcpy(pb_temp, pbBuffer + st * gcd * nChannel, gcd * nChannel);

        for (cu = st; ; cu = ne)
        {
            ne = n_rep_idx[cu];
            if (ne == st)
            {
                memcpy(pbBuffer + cu * gcd * nChannel, pb_temp, gcd * nChannel);
                b_rep_idx[cu] = 1;
                break;
            }
            
            memcpy(pbBuffer + cu * gcd * nChannel, pbBuffer + ne * gcd * nChannel, gcd * nChannel);
            b_rep_idx[cu] = 1;
        }
    }
    free(pb_alloc_buf);
    return true;
}

inline void YUV420ToRGB(unsigned char y, unsigned char u, unsigned char v, unsigned char &r, unsigned char &g, unsigned char &b)
{
    int Y = y - 16;
    if (Y < 0) Y = 0;
    int U = u - 128;
    int V = v - 128;

    int R = (Y * 1192 + V * 1634) >> 10;
    int G = (Y * 1192 - V * 834 - 400 * U) >> 10;
    int B = (Y * 1192 + U * 2066) >> 10;

    r = R > 255 ? 255 : R < 0 ? 0 : R;
    g = G > 255 ? 255 : G < 0 ? 0 : G;
    b = B > 255 ? 255 : B < 0 ? 0 : B;
}

void yuv_to_rgb_shrink(unsigned char* yuv, int n_src_width, int n_src_height, unsigned char* rgb, int n_dst_width, int n_dst_height)
{
    int* pnPosDiff = (int*)malloc((n_dst_width + n_dst_height) * 4 * sizeof(int));

    int* posX0 = pnPosDiff;                 // posX0[320];
    int* posX1 = posX0 + n_dst_width;       // posX1[320];
    int* posY0 = posX1 + n_dst_width;       // posY0[180];
    int* posY1 = posY0 + n_dst_height;      // posY1[180];

    int* diffX0 = posY1 + n_dst_height;     // diffX0[320];
    int* diffX1 = diffX0 + n_dst_width;     // diffX1[320];
    int* diffY0 = diffX1 + n_dst_width;     // diffY0[180];
    int* diffY1 = diffY0 + n_dst_height;    // diffY1[180];

    int n_y_size = n_src_height * n_src_width;
    int nRateXDesToSrc = ((n_src_width - 1) << 10) / (n_dst_width - 1);
    int nRateYDesToSrc = ((n_src_height - 1) << 10) / (n_dst_height - 1);

    int nX, nY;
    int nSrcX_10 = 0;
    for (nX = 0; nX < n_dst_width; nX++)
    {
        int nSrcX = nSrcX_10 >> 10;
        if (n_src_width - 1 <= nSrcX)
        {
            posX0[nX] = n_src_width - 1;
            posX1[nX] = n_src_width - 1;
            diffX0[nX] = 0;
            diffX1[nX] = 0x400;
        }
        else
        {
            int nDiffX = nSrcX_10 - (nSrcX << 10);
            posX0[nX] = nSrcX;
            posX1[nX] = nSrcX + 1;
            diffX0[nX] = nDiffX;
            diffX1[nX] = 0x400 - nDiffX;
        }
        nSrcX_10 += nRateXDesToSrc;
    }

    int nSrcY_10 = 0;
    for (nY = 0; nY < n_dst_height; nY++)
    {
        int nSrcY = nSrcY_10 >> 10;

        if (n_src_height - 1 <= nSrcY)
        {
            posY0[nY] = n_src_height - 1;
            posY1[nY] = n_src_height - 1;
            diffY0[nY] = 0;
            diffY1[nY] = 0x400;
        }
        else
        {
            int nDiffY = nSrcY_10 - (nSrcY << 10);
            posY0[nY] = nSrcY;
            posY1[nY] = nSrcY + 1;
            diffY0[nY] = nDiffY;
            diffY1[nY] = 0x400 - nDiffY;
        }
        nSrcY_10 += nRateYDesToSrc;
    }

    unsigned char* pDst = rgb;
    for (nY = 0; nY < n_dst_height; nY++)
    {
        unsigned char* py0 = yuv + n_src_width * posY0[nY];
        unsigned char* py1 = yuv + n_src_width * posY1[nY];

        unsigned char* puv0 = yuv + n_y_size + n_src_width * (posY0[nY] / 2);
        unsigned char* puv1 = yuv + n_y_size + n_src_width * (posY1[nY] / 2);

        int dY0 = diffY0[nY];
        int dY1 = diffY1[nY];

        for (nX = 0; nX < n_dst_width; nX++)
        {
            int nSrcIndexX0 = posX0[nX];
            int nSrcIndexX1 = posX1[nX];
            int dX0 = diffX0[nX];
            int dX1 = diffX1[nX];

            unsigned char* py00 = py0 + nSrcIndexX0;
            unsigned char* py01 = py0 + nSrcIndexX1;
            unsigned char* py10 = py1 + nSrcIndexX0;
            unsigned char* py11 = py1 + nSrcIndexX1;

            unsigned char* puv00 = puv0 + (nSrcIndexX0 & 0xFFFFFFFE);
            unsigned char* puv01 = puv0 + (nSrcIndexX1 & 0xFFFFFFFE);
            unsigned char* puv10 = puv1 + (nSrcIndexX0 & 0xFFFFFFFE);
            unsigned char* puv11 = puv1 + (nSrcIndexX1 & 0xFFFFFFFE);

            unsigned char r00, g00, b00, r01, g01, b01, r10, g10, b10, r11, g11, b11;

            YUV420ToRGB(*py00, puv00[1], puv00[0], r00, g00, b00);
            YUV420ToRGB(*py01, puv01[1], puv01[0], r01, g01, b01);
            YUV420ToRGB(*py10, puv10[1], puv10[0], r10, g10, b10);
            YUV420ToRGB(*py11, puv11[1], puv11[0], r11, g11, b11);

            pDst[0] = (dX1 * dY1 * r00 + dY0 * dX1 * r10 + dX0 * dY1 * r01 + dY0 * dX0 * r11 + 0x80000) >> 20;
            pDst[1] = (dX1 * dY1 * g00 + dY0 * dX1 * g10 + dX0 * dY1 * g01 + dY0 * dX0 * g11 + 0x80000) >> 20;
            pDst[2] = (dX1 * dY1 * b00 + dY0 * dX1 * b10 + dX0 * dY1 * b01 + dY0 * dX0 * b11 + 0x80000) >> 20;

            pDst += 3;
        }
    }
    free(pnPosDiff);
}

void rotateImage_inner(unsigned char* pbBuffer, int nOrgWidth, int nOrgHeight, int nDegreeAngle)
{
    rotate_image_inplace(pbBuffer, nOrgWidth, nOrgHeight, 1, nDegreeAngle, 0);
}

//gamma = 0.65
unsigned char bGammaValue_065[256] =
{
0, 6, 10, 14, 17, 19, 22, 24, 26, 29, 31, 33, 34, 36, 38, 40, 42, 43, 45, 47, 48, 50, 51, 53, 54, 56, 57, 59, 60, 62, 63, 64, 66, 67, 68,
70, 71, 72, 73, 75, 76, 77, 78, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90, 91, 92, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105,
106, 107, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
131, 132, 133, 134, 135, 136, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 145, 146, 147, 148, 149, 150, 151, 151, 152,
153, 154, 155, 156, 157, 157, 158, 159, 160, 161, 162, 162, 163, 164, 165, 166, 167, 167, 168, 169, 170, 171, 171, 172, 173,
174, 175, 175, 176, 177, 178, 179, 179, 180, 181, 182, 182, 183, 184, 185, 186, 186, 187, 188, 189, 189, 190, 191, 192, 192,
193, 194, 195, 195, 196, 197, 198, 198, 199, 200, 201, 201, 202, 203, 204, 204, 205, 206, 206, 207, 208, 209, 209, 210, 211,
212, 212, 213, 214, 214, 215, 216, 217, 217, 218, 219, 219, 220, 221, 221, 222, 223, 224, 224, 225, 226, 226, 227, 228, 228,
229, 230, 230, 231, 232, 233, 233, 234, 235, 235, 236, 237, 237, 238, 239, 239, 240, 241, 241, 242, 243, 243, 244, 245, 245,
246, 247, 247, 248, 249, 249, 250, 251, 251, 252, 253, 253, 254, 255,
};

void gammaCorrection_screen(unsigned char* pBuffer, int nWidth, int nHeight)
{

#if 1
    int nImageSize = nWidth * nHeight;
    int nBufferIndex;

    for (nBufferIndex = 0; nBufferIndex < nImageSize; nBufferIndex++)
    {
        unsigned char bOrgValue, bNewValue;
        bOrgValue = pBuffer[nBufferIndex];
        bNewValue = bGammaValue_065[bOrgValue];
        pBuffer[nBufferIndex] = bNewValue;
    }
#endif

    return;
}

int WaitIRTimeout(int iTimeout)
{
    float _oldTime = Now();
    if (iTimeout <= 0)
        return ETIMEDOUT;

    while(1)
    {
        float _diff = Now() - _oldTime;
        // dbug_printf("[%s] _diff = %0.3f\n", __func__, _diff);
        if (_diff > (float)iTimeout)
            break;
        my_mutex_lock(g_irWriteLock);
        if (g_irWriteCond == 1)
        {
            g_irWriteCond = 0;
            my_mutex_unlock(g_irWriteLock);
            // dbug_printf("[%s] return 0\n", __func__);
            return 0;
        }
        my_mutex_unlock(g_irWriteLock);
        my_usleep(1000);
    }

    return ETIMEDOUT;
}

int WaitIRCancel()
{
    my_mutex_lock(g_irWriteLock);
    g_irWriteCond = 1;
    // dbug_printf("[%s] set 1, %0.3f\n", __func__, Now());
    my_mutex_unlock(g_irWriteLock);
    return 0;
}

int WaitIRTimeout2(int iTimeout)
{
    float _oldTime = Now();
    if (iTimeout <= 0)
        return ETIMEDOUT;

    while(1)
    {
        float _diff = Now() - _oldTime;
        // dbug_printf("[%s] _diff = %0.3f\n", __func__, _diff);
        if (_diff > (float)iTimeout)
            break;
        my_mutex_lock(g_irWriteLock2);
        if (g_irWriteCond2 == 1)
        {
            g_irWriteCond2 = 0;
            my_mutex_unlock(g_irWriteLock2);
            // dbug_printf("[%s] return 0\n", __func__);
            return 0;
        }
        my_mutex_unlock(g_irWriteLock2);
        my_usleep(1000);
    }

    return ETIMEDOUT;
}

int WaitIRCancel2()
{
    my_mutex_lock(g_irWriteLock2);
    g_irWriteCond2 = 1;
    // dbug_printf("[%s] set 1, %0.3f\n", __func__, Now());
    my_mutex_unlock(g_irWriteLock2);
    return 0;
}

int camera_set_irled_on(int on)
{
    if (on)
    {
        my_mutex_lock(g_irWriteLock);
        g_irWriteCond = 0;
        my_mutex_unlock(g_irWriteLock);
        my_mutex_lock(g_irWriteLock2);
        g_irWriteCond2 = 0;
        my_mutex_unlock(g_irWriteLock2);
        g_iLedOnStatus = 1;
    }
    else
        g_iLedOnStatus = 0;
    return 0;
}

int WaitIROffTimeout(int iTimeout)
{

    float _oldTime = Now();
    if (iTimeout <= 0)
        return ETIMEDOUT;

    while(1)
    {
        float _diff = Now() - _oldTime;
        // dbug_printf("[%s] _diff = %0.3f\n", __func__, _diff);
        if (_diff > (float)iTimeout)
            break;
        my_mutex_lock(g_irOffWriteLock);
        if (g_irOffWriteCond == 1)
        {
            g_irOffWriteCond = 0;
            my_mutex_unlock(g_irOffWriteLock);
            // dbug_printf("[%s] return 0\n", __func__);
            return 0;
        }
        my_mutex_unlock(g_irOffWriteLock);
        my_usleep(1000);
    }

    return ETIMEDOUT;
}

int WaitIROffCancel()
{
    my_mutex_lock(g_irOffWriteLock);
    g_irOffWriteCond = 1;
    my_mutex_unlock(g_irOffWriteLock);
    return 0;
}

int WaitIROffTimeout2(int iTimeout)
{

    float _oldTime = Now();
    if (iTimeout <= 0)
        return ETIMEDOUT;

    while(1)
    {
        float _diff = Now() - _oldTime;
        // dbug_printf("[%s] _diff = %0.3f\n", __func__, _diff);
        if (_diff > (float)iTimeout)
            break;
        my_mutex_lock(g_irOffWriteLock2);
        if (g_irWriteCond == 1)
        {
            my_mutex_unlock(g_irOffWriteLock2);
            // dbug_printf("[%s] return 0\n", __func__);
            return 0;
        }
        my_mutex_unlock(g_irOffWriteLock2);
        my_usleep(1000);
    }

    return ETIMEDOUT;
}

int WaitIROffCancel2()
{
    my_mutex_lock(g_irOffWriteLock2);
    g_irOffWriteCond = 1;
    my_mutex_unlock(g_irOffWriteLock2);
    return 0;
}

inline int ConvertYUVtoARGB(int y, int u, int v, unsigned char* dstData, int index)
{
    y = MAX(0, y - 16);

    int r = (y * 1192 + v * 1634) >> 10;
    int g = (y * 1192 - v * 834 - 400 * u) >> 10;
    int b = (y * 1192 + u * 2066) >> 10;

    r = r > 255? 255 : r < 0 ? 0 : r;
    g = g > 255? 255 : g < 0 ? 0 : g;
    b = b > 255? 255 : b < 0 ? 0 : b;

    dstData[index * 3] = r;
    dstData[index * 3 + 1] = g;
    dstData[index * 3 + 2] = b;

    return 0;
}

void ConvertYUV420_NV21toRGB888(unsigned char* data, int width, int height, unsigned char* dstData)
{
    int size = width * height;
    int offset = size;
    int u, v, y1, y2, y3, y4;

    for(int i = 0, k = offset; i < size; i += 2, k += 2)
    {
        y1 = data[i];
        y2 = data[i + 1];
        if (width + i + 1 < size)
        {
            y3 = data[width + i];
            y4 = data[width + i + 1];
        }
        else
        {
            y3 = y1;
            y4 = y2;
        }

        u = data[k + 1];
        v = data[k];
        v = v - 128;
        u = u - 128;

        ConvertYUVtoARGB(y1, u, v, dstData, i);
        ConvertYUVtoARGB(y2, u, v, dstData, i + 1);
        if (width + i + 1 < size)
        {
            ConvertYUVtoARGB(y3, u, v, dstData, width + i);
            ConvertYUVtoARGB(y4, u, v, dstData, width + i + 1);
        }

        if (i != 0 && (i + 2) % width == 0)
            i += width;
    }
}

void lockIROffBuffer()
{
    //pthread_mutex_lock(&g_camIrOffBufLock);
}

void unlockIROffBuffer()
{
    //pthread_mutex_unlock(&g_camIrOffBufLock);
}

void genIROffData10bit(void* bufOrg, void* bufDst, int width, int height)
{
    if (bufOrg == NULL || bufDst == NULL)
    {
        return;
    }
    int nWidthInSrc = width * 3 / 2;
    unsigned char* spOrg = (unsigned char*)bufOrg;
    unsigned char* pIrOffData = (unsigned char*)bufDst;
    int iIdx = 0;
    int nDstIdx = 0;
    int nX, nY;
    for(nY = 0; nY < height; nY += LEDOFFIMAGE_REDUCE_RATE)
    {
        for(nX = 0; nX < width; nX += LEDOFFIMAGE_REDUCE_RATE)
        {
            int nXIndex = (int)(nX / 2) * 3 + (int)(nX % 2); 
            pIrOffData[nDstIdx] = spOrg[iIdx + nXIndex];
            nDstIdx ++;
        }
        iIdx += (nWidthInSrc * (LEDOFFIMAGE_REDUCE_RATE));
    }
}


void genIROffData10bit_Full(void* bufOrg, void* bufDst, int width, int height)
{
    if (bufOrg == NULL || bufDst == NULL)
    {
        return;
    }
    unsigned short* spOrg = (unsigned short*)bufOrg;
    unsigned char* pIrOffData = (unsigned char*)bufDst;
    int nX, nY;
    for(nY = 0; nY < height; nY ++)
    {
        for(nX = 0; nX < width; nX ++)
        {
            *pIrOffData = (int)(*spOrg) >> 2;
            pIrOffData ++;
            spOrg ++;
        }
    }
}

extern unsigned char*  g_abJpgData;
extern int             g_iJpgDataLen;
int vpss_width = 0, vpss_height = 0;

int test_vpss_dump(VPSS_GRP Grp, VPSS_CHN Chn, CVI_U32 u32FrameCnt, unsigned char* outBuf)
{
#if 1
    CVI_S32 s32MilliSec = 1000;
    CVI_U32 u32Cnt = u32FrameCnt;
    // CVI_CHAR szFrameName[128], szPixFrm[10];
    CVI_BOOL bFlag = CVI_TRUE;
    // int fd = 0;
    CVI_S32 i;
    CVI_U32 u32DataLen;
    VIDEO_FRAME_INFO_S stFrameInfo;
    int buf_offset = 0;

    /* get frame  */
    while (u32Cnt--) {
        if (CVI_VPSS_GetChnFrame(Grp, Chn, &stFrameInfo, s32MilliSec) != CVI_SUCCESS) {
            printf("Get frame fail \n");
            usleep(30*1000);
            continue;
        }
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
        if (u32Cnt)
        {
            CVI_VPSS_ReleaseChnFrame(Grp, Chn, &stFrameInfo);
            continue;
        }
#endif
        buf_offset = 0;
        if (bFlag) {
            /* make file name */
            // GetFmtName(stFrameInfo.stVFrame.enPixelFormat, szPixFrm);
            // snprintf(szFrameName, 128, SD_FATFS_MOUNTPOINT"/vpss_grp%d_chn%d_%dx%d_%s_%d.yuv", Grp, Chn,
            //          stFrameInfo.stVFrame.u32Width, stFrameInfo.stVFrame.u32Height,
            //          szPixFrm, u32FrameCnt);
            // printf("Dump frame of vpss chn %d to file: \"%s\"\n", Chn, szFrameName);

            // fd = aos_open(szFrameName, O_CREAT | O_RDWR | O_TRUNC);
            // if (fd <= 0) {
            //     printf("aos_open dst file failed\n");
            //     CVI_VPSS_ReleaseChnFrame(Grp, Chn, &stFrameInfo);
            //     return;
            // }
            dbug_printf("dump frame %dx%d, fmt=%d\n", stFrameInfo.stVFrame.u32Width, stFrameInfo.stVFrame.u32Height, stFrameInfo.stVFrame.enPixelFormat);
            vpss_width = stFrameInfo.stVFrame.u32Width;
            vpss_height = stFrameInfo.stVFrame.u32Height;

            bFlag = CVI_FALSE;
        }

        for (i = 0; i < 3; ++i) {
            u32DataLen = stFrameInfo.stVFrame.u32Stride[i] * stFrameInfo.stVFrame.u32Height;
            if (u32DataLen == 0)
                continue;
            if (i > 0 && ((stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
                (stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
                (stFrameInfo.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
                u32DataLen >>= 1;

            dbug_printf("plane(%d): paddr(%lx) vaddr(%p) stride(%d)\n",
                   i, stFrameInfo.stVFrame.u64PhyAddr[i],
                   stFrameInfo.stVFrame.pu8VirAddr[i],
                   stFrameInfo.stVFrame.u32Stride[i]);
            dbug_printf(" data_len(%d) plane_len(%d)\n",
                      u32DataLen, stFrameInfo.stVFrame.u32Length[i]);
            // aos_write(fd, (CVI_U8 *)stFrameInfo.stVFrame.u64PhyAddr[i], u32DataLen);
            memcpy(outBuf + buf_offset, (CVI_U8 *)stFrameInfo.stVFrame.u64PhyAddr[i], u32DataLen);
            buf_offset += u32DataLen;
        }

        if (CVI_VPSS_ReleaseChnFrame(Grp, Chn, &stFrameInfo) != CVI_SUCCESS)
            printf("CVI_VPSS_ReleaseChnFrame fail\n");
    }
    // if (fd) {
    //     aos_sync(fd);
    //     aos_close(fd);
    // }
    return buf_offset;
#else
    int buf_offset = 0;
    fr_ReadFileData(FN_FACE_CLR_BIN_PATH, 0, outBuf, FN_FACE_CLR_BIN_SIZE);
    buf_offset = FN_FACE_CLR_BIN_SIZE;
    vpss_width = 320;
    vpss_height = 240;
    return buf_offset;
#endif
}

int saveUvcScene()
{
    unsigned char* imgBuf = fr_GetInputImageBuffer1();
    lockIRBuffer();
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
    camera_switch(TC_MIPI_CAM, MIPI_CAM_S2RIGHT);
    int buf_len = test_vpss_dump(0, 0, 10, g_irOnData1);
#else
    int buf_len = test_vpss_dump(DEFAULT_SNR4UVC, 0, 1, g_irOnData1);
#endif  
    if (buf_len <= 0)
    {
        my_printf("dump vpss fail\n");
        unlockIRBuffer();
        return MR_FAILED4_CAMERA;
    }
    dbug_printf("dump vpss ok\n");
    if(!g_abJpgData)
    {
        g_abJpgData = (unsigned char*)my_malloc(128 * 1024);
        g_iJpgDataLen = 0;
    }
    if (!g_abJpgData)
    {
        unlockIRBuffer();
        return MR_FAILED4_NOMEMORY;
    }
    yuv_to_rgb_shrink(g_irOnData1, vpss_width, vpss_height, imgBuf, CAPTURE_HEIGHT, CAPTURE_WIDTH);
    rotate_image_inplace(imgBuf, CAPTURE_HEIGHT, CAPTURE_WIDTH, 3, g_xPS.x.bCamFlip == USE_3M_MODE ? 270: 90, 0);
    int iWriteLen = 0;
    for(int i = 60; i >= 10; i -= 10)
    {
        jpge::params params;
        params.m_quality = i;
        params.m_subsampling = jpge::H2V2;

        iWriteLen = 128 * 1024;
        if(!jpge::compress_image_to_jpeg_file_in_memory(g_abJpgData, iWriteLen, CAPTURE_WIDTH, CAPTURE_HEIGHT, 3, imgBuf, params, g_irOnData1))
        {
            iWriteLen = 0;
            break;
        }

        if(iWriteLen < SI_MAX_IMAGE_SIZE)
        {
            dbug_printf("[%s] size=%d\n", __func__, iWriteLen);
            break;
        }
    }

    g_iJpgDataLen = iWriteLen;
    unlockIRBuffer();
    return MR_SUCCESS;
}

void remove_white_point_riscv(unsigned char* raw, int nWidth, int nHeight)
{
    int th = 30;

    // int nCount = (nWidth - 4) >> 3;

    for (int y = 0; y < 2; y++)
    {
        unsigned char* p0 = raw + y * nWidth + 2;
        unsigned char* p4 = p0 - 2;
        unsigned char* p5 = p0 + 2;
        unsigned char* p7 = p0 + (nWidth << 1);
        unsigned char* p6 = p7 - 2;
        unsigned char* p8 = p7 + 2;

        for (int x = 0; x < 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v5 = *p5;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v5 + v7 + v8) / 3;
            }

            p0++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }

        for (int x = 2; x < nWidth - 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v4 = *p4;
            int v5 = *p5;
            int v6 = *p6;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v4 + v5 + v6 + v7 + v8) / 5;
            }

            p0++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;

        }

        for (int x = nWidth - 2; x < nWidth; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v4 = *p4;
            int v6 = *p6;
            int v7 = *p7;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v4 + v6 + v7) / 3;
            }

            p0++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }
    }

    for (int y = 2; y < nHeight - 2; y++)
    {
        unsigned char* p0 = raw + y * nWidth + 2;
        unsigned char* p2 = p0 - (nWidth << 1);
        unsigned char* p1 = p2 - 2;
        unsigned char* p3 = p2 + 2;
        unsigned char* p4 = p0 - 2;
        unsigned char* p5 = p0 + 2;
        unsigned char* p7 = p0 + (nWidth << 1);
        unsigned char* p6 = p7 - 2;
        unsigned char* p8 = p7 + 2;

        for (int x = 0; x < 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v2 = *p2;
            int v3 = *p3;
            int v5 = *p5;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v2 + v3 + v5 + v7 + v8) / 5;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }

#if __riscv_vector
        int n = nWidth - 4;
        while (n > 0)
        {
            int l = vsetvl_e8m4(n);

            vuint8m4_t vu8;
            vuint16m8_t vu16, sum;
            vint16m8_t vs16_0, vs16_1, diff;
            vbool2_t vb_p0, vb_m0, vb_p1, vb_m1;

            vu8 = vle8_v_u8m4((uint8_t*)p0, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_0 = vreinterpret_v_u16m8_i16m8(vu16);

            vu8 = vle8_v_u8m4((uint8_t*)p1, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vu16;
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p0 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m0 = vmslt_vx_i16m8_b2(diff, -th, l);

            vu8 = vle8_v_u8m4((uint8_t*)p2, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vadd_vv_u16m8(sum, vu16, l);
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p1 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m1 = vmslt_vx_i16m8_b2(diff, -th, l);
            vb_p0 = vmand_mm_b2(vb_p0, vb_p1, l);
            vb_m0 = vmand_mm_b2(vb_m0, vb_m1, l);

            vu8 = vle8_v_u8m4((uint8_t*)p3, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vadd_vv_u16m8(sum, vu16, l);
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p1 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m1 = vmslt_vx_i16m8_b2(diff, -th, l);
            vb_p0 = vmand_mm_b2(vb_p0, vb_p1, l);
            vb_m0 = vmand_mm_b2(vb_m0, vb_m1, l);

            vu8 = vle8_v_u8m4((uint8_t*)p4, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vadd_vv_u16m8(sum, vu16, l);
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p1 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m1 = vmslt_vx_i16m8_b2(diff, -th, l);
            vb_p0 = vmand_mm_b2(vb_p0, vb_p1, l);
            vb_m0 = vmand_mm_b2(vb_m0, vb_m1, l);

            vu8 = vle8_v_u8m4((uint8_t*)p5, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vadd_vv_u16m8(sum, vu16, l);
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p1 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m1 = vmslt_vx_i16m8_b2(diff, -th, l);
            vb_p0 = vmand_mm_b2(vb_p0, vb_p1, l);
            vb_m0 = vmand_mm_b2(vb_m0, vb_m1, l);

            vu8 = vle8_v_u8m4((uint8_t*)p6, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vadd_vv_u16m8(sum, vu16, l);
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p1 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m1 = vmslt_vx_i16m8_b2(diff, -th, l);
            vb_p0 = vmand_mm_b2(vb_p0, vb_p1, l);
            vb_m0 = vmand_mm_b2(vb_m0, vb_m1, l);

            vu8 = vle8_v_u8m4((uint8_t*)p7, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vadd_vv_u16m8(sum, vu16, l);
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p1 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m1 = vmslt_vx_i16m8_b2(diff, -th, l);
            vb_p0 = vmand_mm_b2(vb_p0, vb_p1, l);
            vb_m0 = vmand_mm_b2(vb_m0, vb_m1, l);

            vu8 = vle8_v_u8m4((uint8_t*)p8, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            vs16_1 = vreinterpret_v_u16m8_i16m8(vu16);
            sum = vadd_vv_u16m8(sum, vu16, l);
            diff = vsub_vv_i16m8(vs16_0, vs16_1, l);
            vb_p1 = vmsgt_vx_i16m8_b2(diff, th, l);
            vb_m1 = vmslt_vx_i16m8_b2(diff, -th, l);
            vb_p0 = vmand_mm_b2(vb_p0, vb_p1, l);
            vb_m0 = vmand_mm_b2(vb_m0, vb_m1, l);

            vb_p0 = vmor_mm_b2(vb_p0, vb_m0, l);
            vu8 = vnsrl_wx_u8m4(sum, 3, l);

            vse8_v_u8m4_m(vb_p0, (uint8_t*)p0, vu8, l);

            p0 += l;
            p1 += l;
            p2 += l;
            p3 += l;
            p4 += l;
            p5 += l;
            p6 += l;
            p7 += l;
            p8 += l;
            n -= l;
        }
        int m = nWidth - 2;

#elif __ARM_NEON
        int n = nCount;
        int m = 2 + nCount << 3;

        asm volatile(

            "vdup.s16   q11, %10        \n"
            "veor       q12, q12, q12   \n"
            "vsub.s16   q12, q12, q11   \n"


            "movw       r10, #65528     \n"
            "vdup.s16   q13, r10        \n"

            "0:                         \n"

            "vld1.u8    {d0}, [%0]      \n"
            "vld1.u8    {d2}, [%1]      \n"
            "vld1.u8    {d4}, [%2]      \n"
            "vld1.u8    {d6}, [%3]      \n"
            "vld1.u8    {d8}, [%4]      \n"
            "vld1.u8    {d10}, [%5]     \n"
            "vld1.u8    {d12}, [%6]     \n"
            "vld1.u8    {d14}, [%7]     \n"
            "vld1.u8    {d16}, [%8]     \n"

            "vmovl.u8   q0, d0          \n"
            "vmovl.u8   q1, d2          \n"
            "vmovl.u8   q2, d4          \n"
            "vmovl.u8   q3, d6          \n"
            "vmovl.u8   q4, d8          \n"
            "vmovl.u8   q5, d10         \n"
            "vmovl.u8   q6, d12         \n"
            "vmovl.u8   q7, d14         \n"
            "vmovl.u8   q8, d16         \n"

            "veor       q14, q14, q14   \n"

            "vsub.s16   q9, q1, q0      \n"
            "vsub.s16   q10, q2, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vsub.s16   q9, q3, q0      \n"
            "vsub.s16   q10, q4, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vsub.s16   q9, q5, q0      \n"
            "vsub.s16   q10, q6, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vsub.s16   q9, q7, q0      \n"
            "vsub.s16   q10, q8, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vceq.s16   q14, q14, q13   \n"


            "veor       q15, q15, q15   \n"

            "vsub.s16   q9, q1, q0      \n"
            "vsub.s16   q10, q2, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vsub.s16   q9, q3, q0      \n"
            "vsub.s16   q10, q4, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vsub.s16   q9, q5, q0      \n"
            "vsub.s16   q10, q6, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vsub.s16   q9, q7, q0      \n"
            "vsub.s16   q10, q8, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vceq.s16   q15, q15, q13   \n"
            "vadd.s16   q14, q14, q15   \n"

            "vadd.s16   d30, d28, d29   \n"
            "vpadd.s16  d30, d30, d30   \n"
            "vpadd.s16  d30, d30, d30   \n"

            "vmov.s16   r10, d30[0]     \n"
            "cmp        r10, #0         \n"
            "beq        1f              \n"

            "vadd.s16   q9, q1, q2      \n"
            "vadd.s16   q10, q3, q4     \n"
            "vadd.s16   q9, q9, q5      \n"
            "vadd.s16   q10, q10, q6    \n"
            "vadd.s16   q9, q9, q7      \n"
            "vadd.s16   q10, q10, q8    \n"
            "vadd.s16   q9, q9, q10     \n"
            "vshr.s16   q9, q9, #3      \n"
            "vbsl.s16   q14, q9, q0     \n"

            "vmovn.s16  d0, q14         \n"
            "vst1.u8    {d0}, [%0]      \n"

            "1:                         \n"
            "subs       %9, %9, #1      \n"

            "add        %0, #8          \n"
            "add        %1, #8          \n"
            "add        %2, #8          \n"
            "add        %3, #8          \n"
            "add        %4, #8          \n"
            "add        %5, #8          \n"
            "add        %6, #8          \n"
            "add        %7, #8          \n"
            "add        %8, #8          \n"

            "bne        0b              \n"

            :
        "+r"(p0),
            "+r"(p1),
            "+r"(p2),
            "+r"(p3),
            "+r"(p4),
            "+r"(p5),
            "+r"(p6),
            "+r"(p7),
            "+r"(p8),
            "+r"(n)
            :
            "r"(th)
            : "cc", "memory", "r10", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
            );
#else
        int m = 2;
#endif

        for (int x = m; x < nWidth - 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v3 = *p3;
            int v4 = *p4;
            int v5 = *p5;
            int v6 = *p6;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 8 || nN == 8)
            {
                *p0 = (v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8) / 8;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }

        for (int x = nWidth - 2; x < nWidth; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v4 = *p4;
            int v6 = *p6;
            int v7 = *p7;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v1 + v2 + v4 + v6 + v7) / 5;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }
    }

    for (int y = nHeight - 2; y < nHeight; y++)
    {
        unsigned char* p0 = raw + y * nWidth + 2;
        unsigned char* p2 = p0 - (nWidth << 1);
        unsigned char* p1 = p2 - 2;
        unsigned char* p3 = p2 + 2;
        unsigned char* p4 = p0 - 2;
        unsigned char* p5 = p0 + 2;

        for (int x = 0; x < 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v2 = *p2;
            int v3 = *p3;
            int v5 = *p5;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v2 + v3 + v5) / 3;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
        }

        for (int x = 2; x < nWidth - 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v3 = *p3;
            int v4 = *p4;
            int v5 = *p5;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v1 + v2 + v3 + v4 + v5) / 5;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
        }

        for (int x = nWidth - 2; x < nWidth; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v4 = *p4;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v1 + v2 + v4) / 3;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
        }
    }
}

void remove_white_point(unsigned char* raw, int nWidth, int nHeight)
{
    int th = 30;
    //int nCount = (nWidth - 4) >> 3;

    for (int y = 0; y < 2; y++)
    {
        unsigned char* p0 = raw + y * nWidth + 2;
        unsigned char* p4 = p0 - 2;
        unsigned char* p5 = p0 + 2;
        unsigned char* p7 = p0 + (nWidth << 1);
        unsigned char* p6 = p7 - 2;
        unsigned char* p8 = p7 + 2;

        for (int x = 0; x < 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v5 = *p5;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v5 + v7 + v8) / 3;
            }

            p0++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }

        for (int x = 2; x < nWidth - 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v4 = *p4;
            int v5 = *p5;
            int v6 = *p6;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v4 + v5 + v6 + v7 + v8) / 5;
            }

            p0++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;

        }

        for (int x = nWidth - 2; x < nWidth; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v4 = *p4;
            int v6 = *p6;
            int v7 = *p7;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v4 + v6 + v7) / 3;
            }

            p0++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }
    }

    for (int y = 2; y < nHeight - 2; y++)
    {
        unsigned char* p0 = raw + y * nWidth + 2;
        unsigned char* p2 = p0 - (nWidth << 1);
        unsigned char* p1 = p2 - 2;
        unsigned char* p3 = p2 + 2;
        unsigned char* p4 = p0 - 2;
        unsigned char* p5 = p0 + 2;
        unsigned char* p7 = p0 + (nWidth << 1);
        unsigned char* p6 = p7 - 2;
        unsigned char* p8 = p7 + 2;

        for (int x = 0; x < 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v2 = *p2;
            int v3 = *p3;
            int v5 = *p5;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v2 + v3 + v5 + v7 + v8) / 5;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }

#if __ARM_NEON
        int n = nCount;
        int m = 2 + nCount << 3;

        asm volatile(

            "vdup.s16   q11, %10        \n"
            "veor       q12, q12, q12   \n"
            "vsub.s16   q12, q12, q11   \n"


            "movw       r10, #65528     \n"
            "vdup.s16   q13, r10        \n"

            "0:                         \n"

            "vld1.u8    {d0}, [%0]      \n"
            "vld1.u8    {d2}, [%1]      \n"
            "vld1.u8    {d4}, [%2]      \n"
            "vld1.u8    {d6}, [%3]      \n"
            "vld1.u8    {d8}, [%4]      \n"
            "vld1.u8    {d10}, [%5]     \n"
            "vld1.u8    {d12}, [%6]     \n"
            "vld1.u8    {d14}, [%7]     \n"
            "vld1.u8    {d16}, [%8]     \n"

            "vmovl.u8   q0, d0          \n"
            "vmovl.u8   q1, d2          \n"
            "vmovl.u8   q2, d4          \n"
            "vmovl.u8   q3, d6          \n"
            "vmovl.u8   q4, d8          \n"
            "vmovl.u8   q5, d10         \n"
            "vmovl.u8   q6, d12         \n"
            "vmovl.u8   q7, d14         \n"
            "vmovl.u8   q8, d16         \n"

            "veor       q14, q14, q14   \n"

            "vsub.s16   q9, q1, q0      \n"
            "vsub.s16   q10, q2, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vsub.s16   q9, q3, q0      \n"
            "vsub.s16   q10, q4, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vsub.s16   q9, q5, q0      \n"
            "vsub.s16   q10, q6, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vsub.s16   q9, q7, q0      \n"
            "vsub.s16   q10, q8, q0     \n"
            "vcgt.s16   q9, q9, q11     \n"
            "vcgt.s16   q10, q10, q11   \n"
            "vadd.s16   q14, q14, q9    \n"
            "vadd.s16   q14, q14, q10   \n"

            "vceq.s16   q14, q14, q13   \n"


            "veor       q15, q15, q15   \n"

            "vsub.s16   q9, q1, q0      \n"
            "vsub.s16   q10, q2, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vsub.s16   q9, q3, q0      \n"
            "vsub.s16   q10, q4, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vsub.s16   q9, q5, q0      \n"
            "vsub.s16   q10, q6, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vsub.s16   q9, q7, q0      \n"
            "vsub.s16   q10, q8, q0     \n"
            "vclt.s16   q9, q9, q12     \n"
            "vclt.s16   q10, q10, q12   \n"
            "vadd.s16   q15, q15, q9    \n"
            "vadd.s16   q15, q15, q10   \n"

            "vceq.s16   q15, q15, q13   \n"
            "vadd.s16   q14, q14, q15   \n"

            "vadd.s16   d30, d28, d29   \n"
            "vpadd.s16  d30, d30, d30   \n"
            "vpadd.s16  d30, d30, d30   \n"

            "vmov.s16   r10, d30[0]     \n"
            "cmp        r10, #0         \n"
            "beq        1f              \n"

            "vadd.s16   q9, q1, q2      \n"
            "vadd.s16   q10, q3, q4     \n"
            "vadd.s16   q9, q9, q5      \n"
            "vadd.s16   q10, q10, q6    \n"
            "vadd.s16   q9, q9, q7      \n"
            "vadd.s16   q10, q10, q8    \n"
            "vadd.s16   q9, q9, q10     \n"
            "vshr.s16   q9, q9, #3      \n"
            "vbsl.s16   q14, q9, q0     \n"

            "vmovn.s16  d0, q14         \n"
            "vst1.u8    {d0}, [%0]      \n"

            "1:                         \n"
            "subs       %9, %9, #1      \n"

            "add        %0, #8          \n"
            "add        %1, #8          \n"
            "add        %2, #8          \n"
            "add        %3, #8          \n"
            "add        %4, #8          \n"
            "add        %5, #8          \n"
            "add        %6, #8          \n"
            "add        %7, #8          \n"
            "add        %8, #8          \n"

            "bne        0b              \n"

            :
        "+r"(p0),
            "+r"(p1),
            "+r"(p2),
            "+r"(p3),
            "+r"(p4),
            "+r"(p5),
            "+r"(p6),
            "+r"(p7),
            "+r"(p8),
            "+r"(n)
            :
            "r"(th)
            : "cc", "memory", "r10", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15"
            );
#else
        int m = 2;
#endif

        for (int x = m; x < nWidth - 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v3 = *p3;
            int v4 = *p4;
            int v5 = *p5;
            int v6 = *p6;
            int v7 = *p7;
            int v8 = *p8;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (v0 - v8 > th) nP++;
            if (v0 - v8 < -th) nN++;

            if (nP == 8 || nN == 8)
            {
                *p0 = (v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8) / 8;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }

        for (int x = nWidth - 2; x < nWidth; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v4 = *p4;
            int v6 = *p6;
            int v7 = *p7;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v6 > th) nP++;
            if (v0 - v6 < -th) nN++;

            if (v0 - v7 > th) nP++;
            if (v0 - v7 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v1 + v2 + v4 + v6 + v7) / 5;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
            p6++;
            p7++;
            p8++;
        }
    }

    for (int y = nHeight - 2; y < nHeight; y++)
    {
        unsigned char* p0 = raw + y * nWidth + 2;
        unsigned char* p2 = p0 - (nWidth << 1);
        unsigned char* p1 = p2 - 2;
        unsigned char* p3 = p2 + 2;
        unsigned char* p4 = p0 - 2;
        unsigned char* p5 = p0 + 2;

        for (int x = 0; x < 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v2 = *p2;
            int v3 = *p3;
            int v5 = *p5;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v2 + v3 + v5) / 3;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
        }

        for (int x = 2; x < nWidth - 2; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v3 = *p3;
            int v4 = *p4;
            int v5 = *p5;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v3 > th) nP++;
            if (v0 - v3 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (v0 - v5 > th) nP++;
            if (v0 - v5 < -th) nN++;

            if (nP == 5 || nN == 5)
            {
                *p0 = (v1 + v2 + v3 + v4 + v5) / 5;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
        }

        for (int x = nWidth - 2; x < nWidth; x++)
        {
            int nP = 0;
            int nN = 0;

            int v0 = *p0;
            int v1 = *p1;
            int v2 = *p2;
            int v4 = *p4;

            if (v0 - v1 > th) nP++;
            if (v0 - v1 < -th) nN++;

            if (v0 - v2 > th) nP++;
            if (v0 - v2 < -th) nN++;

            if (v0 - v4 > th) nP++;
            if (v0 - v4 < -th) nN++;

            if (nP == 3 || nN == 3)
            {
                *p0 = (v1 + v2 + v4) / 3;
            }

            p0++;
            p1++;
            p2++;
            p3++;
            p4++;
            p5++;
        }
    }
}