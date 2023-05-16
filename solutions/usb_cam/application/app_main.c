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
	MEDIA_UVC_Init();
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
#if 0
	extern int my_flash_part_read(const char* part_name, unsigned int offset, void* buf, unsigned int length);
	// void* temp_buf = NULL;
	printf("flash read test uvc...%lld\n", aos_now_ms());
	int idx = 0;
	float old_time = (float)aos_now_ms();
	int total_size = 0;
	float total_time = 0;
	int* file_data = NULL;
	while(g_part_files[idx].m_filename != NULL)
    {
        file_data = (int*)my_malloc(g_part_files[idx].m_filesize);
        if (!file_data)
        {
            my_printf("check firmware, malloc failed.\n");
            break;
        }
        old_time = (float)aos_now_ms();
        fr_ReadFileData(g_part_files[idx].m_filename, 0, file_data, g_part_files[idx].m_filesize);
        total_time += (float)aos_now_ms() - old_time;
        total_size += g_part_files[idx].m_filesize;
        int sum = 0;
        for (int k = 0; k < g_part_files[idx].m_filesize / (int)sizeof(int); k ++)
        {
            sum = sum ^ file_data[k];
        }
        my_free(file_data);
        if (sum != g_part_files[idx].m_checksum)
        {
            my_printf("error %s: %08x <> %08x\n", g_part_files[idx].m_filename, sum, g_part_files[idx].m_checksum);
        }
        else
        {
            my_printf("pass ok: %s\n", g_part_files[idx].m_filename);
        }

        idx ++;
    }
    printf("flash read end... %0.3fMB/s\n", total_size / total_time * 1000 / 1024 / 1024);
	printf("flash read end...%lld\n", aos_now_ms());
#endif
	fmMain();
	
	APP_CustomEventStart();
	while (1) {
		aos_msleep(3000);
	};
}
