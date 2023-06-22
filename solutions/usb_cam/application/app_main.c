/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include "common_yocsystem.h"
#include "media_video.h"
#include "gui_display.h"
#include "media_audio.h"
#include "platform.h"
#include "custom_event.h"
#include "cvi_param.h"
#include "wifi_if.h"
#include "ethernet_init.h"
#include "cvi_tpu_interface.h"
#include "cvi_vi.h"
#include "common_vi.h"
#include "cvi_tempsen.h"

#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define TAG "app"

static int g_iCurCam = 1;
int g_iLedFlag = 0;
int g_iCounter = 0;

int camera_switch(int camid)
{
    ISP_SNS_OBJ_S *pSnsObj = NULL;

    pSnsObj = getSnsObj(SMS_SC201CS_MIPI_2M_30FPS_10BIT);
    if (!pSnsObj)
    {
        return -1;
    }
    pSnsObj->pfnSnsSwitch(0, camid == 1);
    g_iCurCam = camid;
    return 0;
}

void* test_camera(void* arg)
{
    VIDEO_FRAME_INFO_S stVideoFrame[2];
    VI_DUMP_ATTR_S attr;
    int frm_num = 1;
    CVI_U32 dev = 0;
    CVI_S32 s_ret = 0;
    int iFrameCount = 0;
    printf("[%s] start\n", __func__);

    attr.bEnable = 1;
    attr.u32Depth = 0;
    attr.enDumpType = VI_DUMP_TYPE_RAW;

    CVI_VI_SetPipeDumpAttr(dev, &attr);

    while (1)
    {
        frm_num = 1;

        memset(stVideoFrame, 0, sizeof(stVideoFrame));
        stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
        stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

        s_ret = CVI_VI_GetPipeFrame(dev, stVideoFrame, 300);
        if (s_ret != CVI_SUCCESS)
        {
            if (g_iCurCam == 1)
                printf("camera error 1\n");
            else
                printf("camera error 2\n");
            break;
        }
        if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
            frm_num = 2;

        if (iFrameCount == 0)
            printf("image size %d, %d\n", stVideoFrame[0].stVFrame.u32Length[0], frm_num);
        if (frm_num >= 2)
            printf("image size2. %d, %d\n", stVideoFrame[0].stVFrame.u32Length[0], frm_num);

        stVideoFrame[0].stVFrame.pu8VirAddr[0] = (CVI_U8 *)stVideoFrame[0].stVFrame.u64PhyAddr[0];

        unsigned char *ptr = (unsigned char*)stVideoFrame[0].stVFrame.pu8VirAddr[0];

        printf("[%d]mc: %do, %dc, %dt\n", (int)aos_now_ms(), g_iLedFlag, g_iCurCam, g_iCounter);

        if(g_iCounter == 0)
        {
            printf("get 1-0 ok, %02x.\n", ptr[0]);
            g_iCounter ++;
        }
        else if(g_iCounter == 1)
        {
            camera_switch(0);
            printf("get 1-2 ok, %02x.\n", ptr[0]);
            g_iCounter ++;
        }
        else if(g_iCounter == 2)
        {
            camera_switch(1);
            printf("get 2 ok, %02x.\n", ptr[0]);

            g_iCounter = 0;
        }

        CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

        iFrameCount ++;
    }
    return NULL;
}

extern void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level);

int main(int argc, char *argv[])
{
    cvi_tempsen_t tps;
	YOC_SYSTEM_Init();
	//board pinmux init
	PLATFORM_IoInit();
	//Fs init
	YOC_SYSTEM_FsVfsInit();
	//load cfg
	PARAM_LoadCfg();
	//media video sys init
	MEDIA_VIDEO_SysInit();
	//custom_evenet_pre
	//media video
	MEDIA_VIDEO_Init();
	//media_audio
	MEDIA_AUDIO_Init();
	//network
	#if (CONFIG_APP_ETHERNET_SUPPORT == 1)
	ethernet_init();
	#endif
	#if (CONFIG_APP_WIFI_SUPPORT == 1)
	APP_WifiInit();
	#endif
	//cli and ulog init
	YOC_SYSTEM_ToolInit();
	#if (CONFIG_PQTOOL_SUPPORT == 1)
	usleep(12 * 1000);
	isp_daemon2_init(5566);
	#endif
	//init tpu
	cvi_tpu_init();
	printf("init tpu ok.\n");

	aos_msleep(100);
	camera_switch(g_iCurCam);
	aos_msleep(100);

    g_iCounter = 0;
    g_iLedFlag = 1;

	pthread_attr_t a;
	pthread_t thd;
    pthread_attr_init(&a);
    a.stacksize = 8192;
    
    pthread_create(&thd, &a, test_camera, NULL);
	cvi_tempsen_init(&tps);
	unsigned int temp;
	while (1) {
		temp = cvi_tempsen_read_temp_mC(&tps, 1000);
		printf("******* temper(%08d): %u\n", (int)aos_now_ms(), temp);
        _GPIOSetValue(4, 21, g_iLedFlag);
        g_iLedFlag = (g_iLedFlag + 1) % 2;
		aos_msleep(200);
	};
}
