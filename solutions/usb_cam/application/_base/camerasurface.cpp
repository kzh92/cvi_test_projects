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
#include "mutex.h"
#include <cvi_base.h>
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_buffer.h"
#include "vfs.h"
#include <fcntl.h>

#include <errno.h>
#include <string.h>

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

    g_irOnData1 = (unsigned char*)my_malloc(IR_BUFFER_SIZE);
    if (g_irOnData1 == NULL)
        my_printf("malloc fail(%s:%d)", __FILE__, __LINE__);
    g_irOnData2 = (unsigned char*)my_malloc(IR_BUFFER_SIZE);
    if (g_irOnData2 == NULL)
        my_printf("malloc fail(%s:%d)", __FILE__, __LINE__);

#if (USE_VDBTASK)
    g_clrWriteLock = my_mutex_init();
    g_clrYuvData_tmp = (unsigned char*)my_malloc(CLR_CAM_WIDTH * ALIGN_16B(CLR_CAM_HEIGHT) * 2);
    if (g_clrYuvData_tmp == NULL)
        my_printf("malloc fail(%s:%d)", __FILE__, __LINE__);
    g_clrYuvData = (unsigned char*)my_malloc(CLR_CAM_WIDTH * ALIGN_16B(CLR_CAM_HEIGHT) * 2);
    if (g_clrYuvData == NULL)
        my_printf("malloc fail(%s:%d)", __FILE__, __LINE__);
#endif // USE_VDBTASK

    g_iMipiCamInited = camera_init(MIPI_1_CAM, IR_CAM_WIDTH, IR_CAM_HEIGHT, MIPI_CAM_S2LEFT);

    if(g_iMipiCamInited == 0)
    {
        camera_set_exp_byreg(MIPI_0_CAM, INIT_EXP);
        camera_set_gain_byreg(MIPI_0_CAM, INIT_GAIN, INIT_FINEGAIN);
        camera_set_exp_byreg(MIPI_1_CAM, INIT_EXP_1);
        camera_set_gain_byreg(MIPI_1_CAM, INIT_GAIN_1, INIT_FINEGAIN_1);
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
#if (USE_VDBTASK)
    g_xSS.iRunningCamSurface = 1;

    if(iMode == 0)
    {
        fr_InitIRCamera_ExpGain();
    }

#if (TEST_CAM_MODE == TEST_TCMIPI || TEST_CAM_MODE == TEST_TWO)
    //init tc mipi camera
    if(g_iMipiCamInited == -1)
    {
        my_mutex_lock(g_captureLock);
        if(g_iMipiCamInited == -1)
        {
            g_iMipiCamInited = camera_init(TC_MIPI_CAM, IR_CAM_WIDTH, IR_CAM_HEIGHT, MIPI_CAM_S2LEFT);
            if(g_iMipiCamInited == -1)
            {
                my_mutex_unlock(g_captureLock);
                g_xSS.iCamError |= CAM_ERROR_MIPI1;
            }
            else
            {
                if(iMode == 0)
                {
                    camera_set_exp_byreg(TC_MIPI_CAM_LEFT, INIT_EXP);
                    camera_set_gain_byreg(TC_MIPI_CAM_LEFT, INIT_GAIN);

                    camera_set_exp_byreg(TC_MIPI_CAM_RIGHT, INIT_EXP_1);
                    camera_set_gain_byreg(TC_MIPI_CAM_RIGHT, INIT_GAIN_1);
                }
                my_mutex_unlock(g_captureLock);
            }
        }
        else
            my_mutex_unlock(g_captureLock);
    }

    if (g_iMipiCamInited == 0)
    {
        my_mutex_lock(g_captureLock);
        if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
        {
            //test pattern
            camera_set_pattern_mode(TC_MIPI_CAM);
        }
        my_mutex_unlock(g_captureLock);
    }

    if(g_iMipiCamInited == 0 && g_capture0 == 0)
    {
        my_thread_create_ext(&g_capture0, 0, ProcessTCMipiCapture, NULL, (char*)"getmipi1", 8192, 0);
    }
#endif
#else // USE_VDBTASK
    g_xSS.iRunningCamSurface = 1;

    if(iMode == 0)
    {
        fr_InitIRCamera_ExpGain();
    }

    g_iDvpCamInited = 0;

    //init tc mipi camera
    if(g_iMipiCamInited == -1)
    {
        float r = Now();
        g_iMipiCamInited = camera_init(MIPI_1_CAM, IR_CAM_WIDTH, IR_CAM_HEIGHT, MIPI_CAM_S2LEFT);
        if(Now() - r > 500)
            my_printf("$$$$$$$$$$$$$$$$$  MIPI_1_ERROR:   %f\n", Now() - r);

        if(g_iMipiCamInited == -1)
        {
            g_xSS.iCamError |= CAM_ERROR_MIPI1;
        }
    }

    if(iMode == 0 && g_iMipiCamInited == 0)
    {
        camera_set_exp_byreg(MIPI_0_CAM, INIT_EXP);
        camera_set_gain_byreg(MIPI_0_CAM, INIT_GAIN, INIT_FINEGAIN);
        camera_set_exp_byreg(MIPI_1_CAM, INIT_EXP_1);
        camera_set_gain_byreg(MIPI_1_CAM, INIT_GAIN_1, INIT_FINEGAIN_1);
    }

    if(g_iMipiCamInited == 0 && g_capture0 == 0)
        my_thread_create_ext(&g_capture0, 0, ProcessTCMipiCapture, NULL, (char*)"getmipi1", 8192, MYTHREAD_PRIORITY_MEDIUM);

#endif // USE_VDBTASK
}

void StopCamSurface()
{
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

void* ProcessTCMipiCapture(void */*param*/)
{
    VIDEO_FRAME_INFO_S stVideoFrame[2];
    VI_DUMP_ATTR_S attr[2];
    int frm_num = 1;
    CVI_U32 dev = 0;
    CVI_S32 s_ret = 0;

    dbug_printf("[%s]\n", __func__);

    int iFrameCount = 0;
    int iNeedNext = 0;
    float rOld = Now();
    dev = 0;
    for (dev = 0; dev < 2; dev ++)
    {
        attr[dev].bEnable = 1;
        attr[dev].u32Depth = 0;
        attr[dev].enDumpType = VI_DUMP_TYPE_RAW;

        if (CVI_VI_SetPipeDumpAttr(dev, &attr[dev]) != CVI_SUCCESS)
            my_printf("dev=%d SetPipeDumpAttr failed\n", dev);
    }
    if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
    {
        camera_set_pattern_mode(TC_MIPI_CAM, 1);
    }

    while (g_xSS.iRunningCamSurface)
    {
        if (g_xSS.iStartOta || g_xSS.iMState == MS_OTA) break;

        frm_num = 1;

        memset(stVideoFrame, 0, sizeof(stVideoFrame));
        stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
        stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

        dev = (camera_get_actIR() == MIPI_CAM_S2LEFT ? 0: 1);

        s_ret = CVI_VI_GetPipeFrame(dev, stVideoFrame, 100);
        if (s_ret != CVI_SUCCESS)
        {
            g_xSS.iCamError = CAM_ERROR_MIPI2;
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

        if(g_iTwoCamFlag == IR_CAMERA_STEP0 && camera_get_actIR() == MIPI_CAM_S2RIGHT)
        {
            //카메라절환할때 등록기설정명령과 app에서 내려보내는 등록기설정명령이 겹치면서 카메라오유가 나오댔음
            //camera_switch를 내려보낸 다음 프레임의 dqbuf하기 전부터 10ms미만에는 카메라등록기설정을 하지 않게 함
            //swtich to id->1
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2LEFT);
            iNeedNext = 1;
        }

        ///Sub0카메라로 동작할때 레드켜기지령을 받았으면 Sub0화상을 얻고 Sub1로 절환하여 두번째화상을 얻은다음 다시 Sub0으로 카메라를 절환한다.
        if(g_iTwoCamFlag == IR_CAMERA_STEP0 && iNeedNext == 0/* && reserved == 1 && id == MIPI_CAM_SUB0 && iOldReserved == 0*/)
        {
            if(g_iLedOnStatus == 1)
                gpio_irled_on(ON);

            lockIROffBuffer();
            genIROffData10bit(ptr + (int)image_size/8, fr_GetOffImageBuffer(), IR_CAM_WIDTH, IR_CAM_HEIGHT);
            unlockIROffBuffer();
            g_iLedOffFrameFlag = LEFT_IROFF_CAM_RECVED;
            WaitIROffCancel();
            g_iTwoCamFlag ++;
        }
        else if(g_iTwoCamFlag == IR_CAMERA_STEP1 && g_iTwoCamFlag != IR_CAMERA_STEP2)
        {
            g_iTwoCamFlag ++;
        }
        else if(g_iTwoCamFlag == IR_CAMERA_STEP2)
        {
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2RIGHT);

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

#if (USE_VDBTASK)
            if (g_xSS.rFaceEngineTime == 0 && g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
            {
                fr_CalcScreenValue(g_irOnData1, IR_SCREEN_CAMERAVIEW_MODE);
                CalcNextExposure();
            }
#endif // USE_VDBTASK
            WaitIRCancel();
#if (!USE_3M_MODE)
            g_iTwoCamFlag ++;
#else
            g_iLedOnStatus = 0;
            gpio_irled_on(OFF);
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2LEFT);
            WaitIRCancel2();
            g_iTwoCamFlag = IR_CAMERA_STEP_IDLE;
#endif
        }
#if (!USE_3M_MODE)
        else if(g_iTwoCamFlag == IR_CAMERA_STEP3 && g_iTwoCamFlag != IR_CAMERA_STEP4)
        {
            g_iTwoCamFlag ++;
        }
        else if(g_iTwoCamFlag == IR_CAMERA_STEP4)
        {
            g_iLedOnStatus = 0;
            gpio_irled_on(OFF);
            camera_switch(TC_MIPI_CAM, MIPI_CAM_S2LEFT);

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
#endif // !USE_3M_MODE
        else if(iNeedNext == 1)
        {
            iNeedNext = 0;
        }

        CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

        iFrameCount ++;
    }

    g_xSS.iShowIrCamera = 0;
    my_thread_exit(NULL);

    return NULL;
}

void CalcNextExposure()
{
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
#if (USE_VDBTASK)
    my_mutex_lock(g_captureLock);
    if(g_iMipiCamInited == 0)
    {
        camera_set_exp_byreg(TC_MIPI_CAM_LEFT, INIT_EXP);
        camera_set_gain_byreg(TC_MIPI_CAM_LEFT, INIT_GAIN);

        camera_set_exp_byreg(TC_MIPI_CAM_RIGHT, INIT_EXP_1);
        camera_set_gain_byreg(TC_MIPI_CAM_RIGHT, INIT_GAIN_1);

        fr_InitIRCamera_ExpGain();
    }
    my_mutex_unlock(g_captureLock);
#endif // USE_VDBTASK
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

int CalcClrNextExposure(unsigned char* pbClrBuf)
{
    //changed by KSB 20180711
    int nNewGain, nNewExposure;
    int nEntireCurYAVG = 0;
    int nFaceCurYAVG = 0;
    float rDeltaValue = 1.0f;

    g_nClrFramePassCount++;
    g_nValueSettenPassed++;

    if (g_nClrFramePassCount < 1)
    {
//        return;
    }

    g_nClrFramePassCount = 0;
    nEntireCurYAVG = GetYAVGValueOfClr(pbClrBuf);
    nFaceCurYAVG = nEntireCurYAVG;
    return nEntireCurYAVG;
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

void rotateImage_inner(unsigned char* pbBuffer, int nOrgWidth, int nOrgHeight, int nDegreeAngle)
{
    int nIndex = ((nDegreeAngle + 360) / 90) % 4;

    if (nIndex == 0)
        return;

    unsigned char* pTempBuffer = (unsigned char*)my_malloc(nOrgWidth * nOrgHeight);

    int nDstWidth = nOrgWidth;
    int nDstHeight = nOrgHeight;

    if (nIndex % 2 == 1)
    {
        nDstWidth = nOrgHeight;
        nDstHeight = nOrgWidth;
    }

    int nStartOffset = 0;
    int nNextOffset = 0;
    int nLineOffset = 0;

    if (nIndex == 1)
    {
        // 90
        nStartOffset = nOrgWidth * (nOrgHeight - 1);
        nNextOffset = -nOrgWidth;
        nLineOffset = (nOrgWidth * nOrgHeight + 1);
    }

    if (nIndex == 2)
    {
        // 180
        nStartOffset = nOrgWidth * nOrgHeight - 1;
        nNextOffset = -1;
        nLineOffset = 0;
    }

    if (nIndex == 3)
    {
        // 270
        nStartOffset = nOrgWidth - 1;
        nNextOffset = nOrgWidth;
        nLineOffset = -nOrgWidth * nOrgHeight - 1;
    }

    unsigned char* pSrcBuf = pbBuffer + nStartOffset;
    unsigned char* pDstBuf = pTempBuffer;

    int nMinPos = 10000000, nMaxPos = -1;
    for (int nY = 0; nY < nDstHeight; nY++)
    {
        for (int nX = 0; nX < nDstWidth; nX++)
        {
            int nDstOffset = pDstBuf - pTempBuffer;
            int nSrcOffset = pSrcBuf - pbBuffer;

            if (nDstOffset > nMaxPos) nMaxPos = nDstOffset;
            if (nDstOffset < nMinPos) nMinPos = nDstOffset;
            if (nSrcOffset > nMaxPos) nMaxPos = nSrcOffset;
            if (nSrcOffset < nMinPos) nMinPos = nSrcOffset;

            *pDstBuf = *pSrcBuf;
            pDstBuf++;
            pSrcBuf += nNextOffset;
        }

        pSrcBuf += nLineOffset;
    }

    dbug_printf("[%s]%d %d \n", __func__, nMaxPos, nMinPos);

    memcpy(pbBuffer, pTempBuffer, nOrgWidth * nOrgHeight);
    my_free(pTempBuffer);
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
        g_iLedOnStatus = 1;
        // my_mutex_lock(g_irWriteLock);
        // g_irWriteCond = 0;
        // my_mutex_unlock(g_irWriteLock);
        // my_mutex_lock(g_irWriteLock2);
        // g_irWriteCond2 = 0;
        // my_mutex_unlock(g_irWriteLock2);
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

int ConvertYUVtoARGB(int y, int u, int v, unsigned char* dstData, int index)
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

        u = data[k];
        v = data[k + 1];
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
        iIdx += (nWidthInSrc * (LEDOFFIMAGE_REDUCE_RATE - 1));
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
