#ifndef _MY_UVC_H_
#define _MY_UVC_H_

#include "mutex.h"
#include "appdef.h"	

#ifdef __cplusplus
extern  "C"
{
#endif

#if (USE_VDBTASK)
int Get_Camera_my_Image(int PADId, unsigned char** buf);
int camera_IR_led_on(int on);
int my_WaitIRTimeout(int iTimeout);
void my_StartCamSurface(int iMode);
void my_StartClrCam();
int my_WaitClrTimeout(int iTimeout);
void my_ConvertToSceneJpeg(int PADId);
#if (USE_WIFI_MODULE)
#define OPT_LEN_MAX 256
void my_ConvertYUYV_toYUV420(unsigned char* data);
int my_scan_candidate4OTP(char* result);
void my_create_qrreader();
void my_destroy_qrreader();
#endif // USE_WIFI_MODULE
#endif // USE_VDBTASK

#ifdef __cplusplus
}
#endif

extern int g_iTwoCamFlag;
extern int g_iMipiCamInited;
extern int g_iDvpCamInited;

#endif // !_MY_UVC_H_