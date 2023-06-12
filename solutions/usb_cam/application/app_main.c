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
#include "fm_main.h"
#include "uvc_func.h"
#include "cvi_tpu_interface.h"
#include "drv_gpio.h"
#include "common_types.h"

#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define TAG "app"

#include <soc.h>
#include <drv/wdt.h>

/**
 * The exact time-out can be calculated according to the formula
 * freq = 67500000 Hz
 * user changeable para is 10 (range is 0 ~ 15)
 * 994ms = ((0x10000 << 10)/ (67500000 / 1000))
 */
#define WDT_TIMEOUT_MS                  (500U)               ///< timeout: 0.94s

#define CHECK_RETURN(ret)                           \
        do {                                        \
            if (ret != 0) {                         \
            	printf("check return(%d), %s:%d\n", ret, __FILE__, __LINE__); \
                return -1;                          \
            }                                       \
        } while(0);

csi_wdt_t g_wdt;
static uint8_t cb_wdt_flag = 0;

void wdt_event_cb_fun(csi_wdt_t *wdt, void *arg){
    (void)arg;
    cb_wdt_flag = 1;
    csi_wdt_feed(wdt);
    printf("watchdog %d\n", (int)aos_now_ms());
}

int watch_dog_test()
{
    csi_error_t ret = 0;

    /* STEP 1: init wdt */
    ret = csi_wdt_init(&g_wdt, 0);
    CHECK_RETURN(ret);

    /* STEP 3: set timeout time(994ms) */
    ret = csi_wdt_set_timeout(&g_wdt, WDT_TIMEOUT_MS);
    CHECK_RETURN(ret);

    /* STEP 2: register callback func */
    //ret = csi_wdt_attach_callback(&g_wdt, wdt_event_cb_fun, NULL);
    //CHECK_RETURN(ret);

    cb_wdt_flag = 0;

    /* STEP 4: start work */
    ret = csi_wdt_start(&g_wdt);
    CHECK_RETURN(ret);
    printf("wd.\n");

    // /* STEP 5: delay 1194ms */
    // mdelay(WDT_TIMEOUT_MS + 200);

    // /* STEP 6:  system should enters the interrupt function */
    // if (0 == cb_wdt_flag) {
    //     ret = -1;
    // }

    // /* STEP 7: start work */
    // csi_wdt_detach_callback(&g_wdt);

    // /* STEP 8: delay 995ms */
    // mdelay(WDT_TIMEOUT_MS + 1);

    // /* !!!system should be restarted, and not be here! */
    // ret = -1;
    return ret;
}

int main(int argc, char *argv[])
{
	YOC_SYSTEM_Init();
	GPIO_fast_init();
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
	//watch_dog_test();
	fmMain();
	
	APP_CustomEventStart();
	while (1) {
		aos_msleep(3000);
	};
}
