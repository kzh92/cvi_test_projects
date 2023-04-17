#include "my_uvc.h"
#include "facerecogtask.h"
#include "drv_gpio.h"
#include "appdef.h"
#include "shared.h"
#include "DBManager.h"
#include "engineparam.h"
#include "ImageProcessing.h"
#include "FaceRetrievalSystem.h"
#include "settings.h"
#include "camerasurface.h"
#include "senselockmessage.h"
#include "faceengine.h"
#include "jpge.h"
#include "check_camera_pattern.h"
#include "sn.h"
#include "vdbtask.h"
#include "qrOTP_base.h"

// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <time.h>
#include <string.h>
// #include <math.h>
#include <fcntl.h>
#include <errno.h>

#if (USE_VDBTASK)
int Get_Camera_my_Image(int PADId, unsigned char** buf)
{
    if (PADId == DVP_CAM)
    {
        *buf = g_clrYuvData_tmp;
    }
    else
    {
        *buf = g_irOnData1;
    }
    return 0;
}

int camera_IR_led_on(int on)
{
    if (on)
        camera_set_irled_on(1);
    else
        camera_set_irled_on(0);
    return 0;
}

int my_WaitIRTimeout(int iTimeout)
{
    return WaitIRTimeout(iTimeout);
}

void my_StartCamSurface(int iMode)
{
    StartCamSurface(iMode);
}

void my_StartClrCam()
{
    StartClrCam();
}

int my_WaitClrTimeout(int iTimeout)
{
    WaitClrTimeout(iTimeout);
    return 0;
}

void my_ConvertToSceneJpeg(int PADId)
{
    int m_iCounter = 1;//VDBTask::m_iCounter
    if (PADId == DVP_CAM)
    {
        int iWriteLen = ConvertYuvToSceneJpeg(g_clrYuvData_tmp, 1);
        SendGlobalMsg(MSG_VDB_TASK, VDB_CAPTURED_IMAGE, iWriteLen, m_iCounter);
    }
    else
    {
        lockIRBuffer();
        for(int y = 0; y < HEIGHT_720; y ++)
            memcpy(g_clrYuvData + y * WIDTH_960, g_irOnData1 + y * WIDTH_1280 + 160, WIDTH_960);
        unlockIRBuffer();

        rotateImage_inner(g_clrYuvData, WIDTH_960, HEIGHT_720, g_xPS.x.bCamFlip == 0 ? 270: 90);
        memset(g_clrYuvData + WIDTH_960 * HEIGHT_720, 0x80, WIDTH_960 * HEIGHT_720 / 2);
        int iWriteLen = ConvertIRToSceneJpeg(g_clrYuvData);

        SendGlobalMsg(MSG_VDB_TASK, VDB_CAPTURED_IMAGE, iWriteLen, m_iCounter);
    }
}
#if (USE_WIFI_MODULE)
void my_ConvertYUYV_toYUV420(unsigned char* data)
{
    lockClrBuffer();
    ConvertYUYV_toYUV420(g_clrYuvData_tmp, WIDTH_1280, HEIGHT_960, data);
    unlockClrBuffer();
}
int my_scan_candidate4OTP(char* result)
{
    return scan_candidate4OTP(g_clrYuvData_tmp, result);
}
void my_create_qrreader()
{
    create_qrreader(WIDTH_1280, HEIGHT_960);
}
void my_destroy_qrreader()
{
    destroy_qrreader();
}
#endif
#endif // USE_VDBTASK