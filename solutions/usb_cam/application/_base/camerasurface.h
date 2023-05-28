#ifndef CAMERASURFACE_H
#define CAMERASURFACE_H

#include "appdef.h"
#include "cam_base.h"
//#include <pthread.h>

#define TEST_DVP    0
#define TEST_TCMIPI 1
#define TEST_TWO    2
#define TEST_CAM_MODE       TEST_TWO

#define WAIT_CAM_FRAME(time_ms, wait_fn) \
    do { \
        int ret = 0; \
        float rOldTime = Now(); \
        while(Now() - rOldTime < time_ms) \
        { \
            ret = wait_fn(5); \
            if (ret != ETIMEDOUT) \
                break; \
            if (g_xSS.iResetFlag == 1) \
                break; \
        } \
        if (g_xSS.iResetFlag == 1) \
            break; \
        my_printf("wait(%s:%d) %0.3f, ret=%d\n", __FILE__, __LINE__, Now() - rOldTime, ret); \
    } while(0)

enum
{
    FIRST_IR_NONE = 0x0,
    LEFT_IR_CAM_RECVED = 0x01,
    RIGHT_IR_CAM_RECVED = 0x02,
    FIRST_IR_PROCESSED = 0x04,
    ALL_IR_CAM_RECVED = 0x08,
    LEFT_IROFF_CAM_RECVED = 0x10,
    RIGHT_IROFF_CAM_RECVED = 0x20,
};

enum
{
    CAM_ERROR_DVP1 = 0x01,
    CAM_ERROR_DVP2 = 0x02,
    CAM_ERROR_MIPI1 = 0x04,
    CAM_ERROR_MIPI2 = 0x08,
    CAM_ERROR_CLR_PATTERN = 0x10,
    CAM_ERROR_IR_PATTERN = 0x20,
    CAM_ERROR_IR_PATTERN_2 = 0x40,
    CAM_ERROR_CLR_PATTERN_2 = 0x80,
    CAM_ERROR_CLR_CHECKED = 0x100,
};

enum {
    IR_CAMERA_STEP_IDLE = -1,
    IR_CAMERA_STEP0 = 0, //left led off
    IR_CAMERA_STEP1 = IR_CAMERA_STEP0 + 1, //skip
    IR_CAMERA_STEP2 = IR_CAMERA_STEP1, //left led on
    IR_CAMERA_STEP3 = IR_CAMERA_STEP2 + 1, // skip
    IR_CAMERA_STEP4 = IR_CAMERA_STEP3, // right led on
    IR_CAMERA_STEP5 = IR_CAMERA_STEP4 + 1, // skip
    IR_CAMERA_STEP6 = IR_CAMERA_STEP5, // right led off
};

void    IncreaseCameraCounter();
int     GetCameraCounter();

void    StartFirstCam();
void    StartCamSurface(int iMode);
void    StopCamSurface();

void*   ProcessDVPCapture(void *param);
void*   ProcessTCMipiCapture(void *param);

void    CalcNextExposure();
void    ConvertYUV420_NV21toRGB888(unsigned char* data, int width, int height, unsigned char* dstData);
void    rotateImage_inner(unsigned char* pbBuffer, int nOrgWidth, int nOrgHeight, int nDegreeAngle);
void    gammaCorrection_screen(unsigned char* pBuffer, int nWidth, int nHeight);
int     camera_set_irled_on(int on);
int     WaitIRTimeout(int iTimeout);
int     WaitIRCancel();
int     WaitIRTimeout2(int iTimeout);
int     WaitIRCancel2();
int     WaitIROffTimeout(int iTimeout);
int     WaitIROffCancel();
int     WaitIROffTimeout2(int iTimeout);
int     WaitIROffCancel2();
void    lockIRBuffer();
void    unlockIRBuffer();
void    lockIROffBuffer();
void    unlockIROffBuffer();
void    reset_ir_exp_gain();
int     saveUvcScene();

extern int g_iDvpCamInited;
extern int g_iMipiCamInited;
extern int g_iTwoCamFlag;
extern int g_iFirstCamFlag;
extern int g_iLedOffFrameFlag;
extern int g_iLedOffFlag;
extern float g_rFirstCamTime;

extern unsigned char* g_irOnData1;
extern unsigned char* g_irOnData2;

//////////////////////////////////// for VDBTask
#if (USE_VDBTASK)

#define     CHECK_CLR_AVERAGE_NUM   4
enum
{
    CLR_CAPTURE_NONE = 0,
    CLR_CAPTURE_START = 1,
    CLR_CAPTURE_END = 2,
    CLR_CAPTURE_OK = 3,
    IR_CAPTURE_OK = 4,
};

void    ConvertYUYV_toYUV420(unsigned char* data, int width, int height, unsigned char* dstData);
void    rotateYUV420SP_flip(unsigned char* src, int width, int height, unsigned char* dst, int angle, int flip = 0);
int     CalcClrNextExposure(unsigned char* pbClrBuf);
int     WaitClrTimeout(int iTimeout);
int     WaitClrCancel();
void    StartClrCam();
void    StopClrCam();
extern unsigned char*  g_abCapturedClrFace;
extern unsigned char*  g_clrYuvData_tmp;
extern unsigned char*  g_clrYuvData_tmp1;
extern unsigned char*  g_clrYuvData;
extern unsigned char*  g_clrRgbData;
extern int g_iCapturedClrFlag;

#endif // USE_VDBTASK

#endif // CAMERASURFACE_H
