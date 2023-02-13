#ifndef CAMERASURFACE_H
#define CAMERASURFACE_H

#include "appdef.h"
#include "cam_base.h"
//#include <pthread.h>

#define TEST_DVP    0
#define TEST_TCMIPI 1
#define TEST_TWO    2
#define TEST_CAM_MODE       TEST_TWO

enum
{
    FIRST_IR_NONE = 0x0,
    LEFT_IR_CAM_RECVED = 0x01,
    RIGHT_IR_CAM_RECVED = 0x02,
    FIRST_IR_PROCESSED = 0x04,
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

void    IncreaseCameraCounter();
int     GetCameraCounter();

void    StartFirstCam();
void    StartCamSurface(int iMode);
void    StopCamSurface();

void*   ProcessDVPCapture(void *param);
void*   ProcessTCMipiCapture(void *param);

void    CalcNextExposure();
void    Convert10bitRawBuffer_To8BitY(unsigned short* p10bitRawBuffer, int nBufferWidth, int nBufferHeight, unsigned char* pResultBuffer, int nRotate);
void    Convert8bitBayerRawBuffer_To8BitY_noRotate(unsigned char* p8bitRawBuffer, int nBufferWidth, int nBufferHeight, unsigned char* pResultBuffer, int nResetUV);
void    Convert8bitBayerRawBuffer_To8BitY_noRotate_FlipY(unsigned char* p8bitRawBuffer, int nBufferWidth, int nBufferHeight, unsigned char* pResultBuffer, int nResetUV);
void    Convert8bitBayerRawBuffer_To8BitY_noRotate_180(unsigned char* p8bitRawBuffer, int nBufferWidth, int nBufferHeight, unsigned char* pResultBuffer, int nResetUV);
void    ConvertYUV420_NV21toRGB888(unsigned char* data, int width, int height, unsigned char* dstData);
void    ConvertYUV422_To8Bit(unsigned char* data, int width, int height, unsigned char* dstData);
void    rotateImage_inner(unsigned char* pbBuffer, int nOrgWidth, int nOrgHeight, int nDegreeAngle);
void    gammaCorrection_screen(unsigned char* pBuffer, int nWidth, int nHeight);
int     camera_set_irled_on(int on);
int     WaitIRTimeout(int iTimeout);
int     WaitIRCancel();

#ifndef __RTK_OS__
int     WaitIROffTimeout(int iTimeout);
int     WaitIROffCancel();
#endif // !__RTK_OS__

extern int g_iDvpCamInited;
extern int g_iMipiCamInited;
extern int g_iTwoCamFlag;
extern int g_iFirstCamFlag;
// extern int g_iLedOffFlag;
extern float g_rFirstCamTime;

// extern int g_nFaceRectValid;
// extern int g_faceDetected;
// extern pthread_mutex_t g_faceDetectionMutex;

// extern pthread_t g_capture0;
// extern pthread_t g_capture1;
// extern pthread_t g_capture2;
// extern pthread_mutex_t g_irWriteLock;
// extern pthread_cond_t g_irWriteCond;

extern unsigned char* g_irOnData1;
extern unsigned char* g_irOnData2;
extern unsigned char* g_irOffData;
// extern unsigned char g_irOnData1[IR_BUFFER_SIZE];
// extern unsigned char g_irOnData2[IR_BUFFER_SIZE];
// extern unsigned char g_irOffData[IR_BUFFER_SIZE];

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
void    rotateYUV420SP_align_flip(unsigned char* src, int width, int height, unsigned char* dst, int angle, int flip = 0);
void    rotateYUV420SP_flip(unsigned char* src, int width, int height, unsigned char* dst, int angle, int flip = 0);
void    reset_ir_exp_gain();
int     CalcClrNextExposure(unsigned char* pbClrBuf);
int     WaitClrTimeout(int iTimeout);
int     WaitClrCancel();
void    StartClrCam();
void    StopClrCam();
// extern pthread_mutex_t g_camBufLock;
// extern unsigned char g_clrYuvData[WIDTH_1280 * ALIGN_16B(HEIGHT_960) * 2];
// extern unsigned char g_clrYuvData_tmp[WIDTH_1280 * ALIGN_16B(HEIGHT_960) * 2];
// extern unsigned char g_clrYuvData_tmp1[WIDTH_1280 * ALIGN_16B(HEIGHT_960) * 2];
// extern unsigned char g_clrRgbData[WIDTH_1280 * ALIGN_16B(HEIGHT_960) * 3];
// extern unsigned char g_abCapturedClrFace[WIDTH_1280 * HEIGHT_960];
extern unsigned char*  g_abCapturedClrFace;
extern unsigned char*  g_clrYuvData_tmp;
extern unsigned char*  g_clrYuvData_tmp1;
extern unsigned char*  g_clrYuvData;
extern unsigned char*  g_clrRgbData;

// extern int g_nGainClr;
// extern int g_nExposureClr;
extern int g_iCapturedClrFlag;

#endif // USE_VDBTASK

#endif // CAMERASURFACE_H
