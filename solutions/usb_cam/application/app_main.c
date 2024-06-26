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
#include <debug/debug_overview.h>
#include "cvi_tpu_interface.h"
#include <aos/kernel.h>
#include <pthread.h>
#include <cvi_base.h>
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_buffer.h"
#include "vfs.h"
#include "cvi_vpss.h"
#include <cviruntime.h>
#include <yoc/partition.h>

#if __riscv_vector
#include <riscv_vector.h>
#endif

#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define TAG "app"
extern int MEDIA_AV_Init();
extern void cmd_ifconfig_func(char *wbuf, int wbuf_len, int argc, char **argv);
char *ifconfig_args[] = {"ifconfig"};

unsigned char* g_imgBuf = NULL;
unsigned char* g_modelData = NULL;
int g_modelLen = 0;

CVI_MODEL_HANDLE  sample_model = NULL;
CVI_TENSOR*       sample_input_tensors;
CVI_TENSOR*       sample_output_tensors;
CVI_TENSOR*       sample_input;
CVI_TENSOR*       sample_output;
int32_t           sample_input_num;
int32_t           sample_output_num;
int32_t           sample_height;
int32_t           sample_width;

int p__read(const char* part_name, unsigned int offset, void* buf, unsigned int length)
{
    int ret;
    partition_t partition = partition_open(part_name);
    if (partition < 0)
    {
        printf("[%s]part open fail: %s\n", __func__, part_name);
        return 0;
    }
    ret = partition_read(partition, offset, buf, length);
    if (ret)
    {
        printf("[%s]part read fail: %s\n", __func__, part_name);
        return 0;
    }
    partition_close(partition);
    if (ret)
        return 0;
    else
        return length;
}

void load_model()
{
  int sum = 0;
  printf("%s:%d\n", __FILE__, __LINE__);
  g_modelLen = 11557800;
  g_modelData = (unsigned char*)malloc(g_modelLen);
  printf("%s:%d\n", __FILE__, __LINE__);
  if (p__read("wgt", 0, g_modelData, g_modelLen) != g_modelLen)
  {
  	printf("read part failed\n");
  	return;
  }
  printf("%s:%d\n", __FILE__, __LINE__);
  for (int i = 0; i < g_modelLen; i ++)
  {
  	sum ^= g_modelData[i];
  	if (i < 32)
  		printf("%02x ", g_modelData[i]);
  }
  printf("--------------------------- %08x\n", sum);
  int ret = CVI_NN_RegisterModelFromBuffer((const int8_t *)g_modelData, g_modelLen, &sample_model);
  printf("%s:%d\n", __FILE__, __LINE__);
  if (CVI_RC_SUCCESS != ret) {
    printf("CVI_NN_RegisterModelFromBuffer failed, err %d\n", ret);
  }
  printf("CVI_NN_RegisterModelFromBuffer succeeded\n");
  free(g_modelData);
  CVI_NN_GetInputOutputTensors(sample_model, &sample_input_tensors, &sample_input_num, &sample_output_tensors, &sample_output_num);
  sample_input = CVI_NN_GetTensorByName(CVI_NN_DEFAULT_TENSOR, sample_input_tensors, sample_input_num);
  sample_output = CVI_NN_GetTensorByName(CVI_NN_DEFAULT_TENSOR, sample_output_tensors, sample_output_num);
  CVI_SHAPE shape = CVI_NN_TensorShape(sample_input);

  sample_height = shape.dim[2];
  sample_width = shape.dim[3];
  printf("load model ok\n");
}

float forward_model(unsigned char* pnImage, int nMean, float rStd)
{
  int i;
  int size = sample_width * sample_height;
  float* prInput = (float *)CVI_NN_TensorPtr(sample_input);

  float* prIter = prInput;
  unsigned char* pnIter = pnImage;

  for (i = 0; i < size; i++)
  {
      *prIter = (*pnIter - nMean) * rStd;
      prIter++;
      pnIter++;
  }

  memcpy(prInput + size, prInput, size * sizeof(float));
  memcpy(prInput + size * 2, prInput, size * sizeof(float));

  CVI_NN_Forward(sample_model, sample_input_tensors, sample_input_num, sample_output_tensors, sample_output_num);

  float* prOutput = (float *)CVI_NN_TensorPtr(sample_output);
  return prOutput[0];
}

int proc_1(unsigned char* buf, int nSize)
{
    int nCheckSum = 0;
    int i;
    for(i = 0; i + 3 < nSize; i += 3)
        nCheckSum ^= ((unsigned int*)buf)[i];
    nCheckSum = ~nCheckSum;
    return nCheckSum;
}

void proc_2(unsigned char* raw, int nWidth, int nHeight)
{
    for (int y = 2; y < nHeight - 2; y++)
    {
        unsigned char* p0 = raw + y * nWidth + 4;
        unsigned char* p2 = p0 - (nWidth << 1);
        unsigned char* p1 = p2 - 2;
        unsigned char* p3 = p2 + 2;
        unsigned char* p4 = p0 - 2;
        unsigned char* p5 = p0 + 2;
        unsigned char* p7 = p0 + (nWidth << 1);
        unsigned char* p6 = p7 - 2;
        unsigned char* p8 = p7 + 2;

        int n = nWidth - 4;
        while (n > 0)
        {
            int l = vsetvl_e8m4(n);

            vuint8m4_t vu8;
            vuint16m8_t vu16, sum;

            vu8 = vle8_v_u8m4((uint8_t*)p1, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vu16;

            vu8 = vle8_v_u8m4((uint8_t*)p2, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vadd_vv_u16m8(sum, vu16, l);

            vu8 = vle8_v_u8m4((uint8_t*)p3, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vadd_vv_u16m8(sum, vu16, l);

            vu8 = vle8_v_u8m4((uint8_t*)p4, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vadd_vv_u16m8(sum, vu16, l);

            vu8 = vle8_v_u8m4((uint8_t*)p5, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vadd_vv_u16m8(sum, vu16, l);

            vu8 = vle8_v_u8m4((uint8_t*)p6, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vadd_vv_u16m8(sum, vu16, l);

            vu8 = vle8_v_u8m4((uint8_t*)p7, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vadd_vv_u16m8(sum, vu16, l);

            vu8 = vle8_v_u8m4((uint8_t*)p8, l);
            vu16 = vwcvtu_x_x_v_u16m8(vu8, l);
            sum = vadd_vv_u16m8(sum, vu16, l);

            vu8 = vnsrl_wx_u8m4(sum, 3, l);

            vse8_v_u8m4((uint8_t*)p0, vu8, l);

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
    }
}

void* thread_func1(void* arg)
{
	//get camera
    VIDEO_FRAME_INFO_S stVideoFrame[2];
    VI_DUMP_ATTR_S attr[2];
    CVI_U32 dev = 0;
    CVI_S32 s_ret = 0;
    int rOldTime = (int)aos_now_ms();
    int iCount = 0;

    printf("[%s] start\n", __func__);

    int iFrameCount = 0;
    //for (dev = 0; dev < 2; dev ++)
    {
        attr[dev].bEnable = 1;
        attr[dev].u32Depth = 0;
        attr[dev].enDumpType = VI_DUMP_TYPE_RAW;

        if (CVI_VI_SetPipeDumpAttr(dev, &attr[dev]) != CVI_SUCCESS)
            printf("dev=%d SetPipeDumpAttr failed\n", dev);
    }

    while (1)
    {
        memset(stVideoFrame, 0, sizeof(stVideoFrame));
        stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
        stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

        dev = 0;
        //printf("[%d]geting frame: %d\n",(int)aos_now_ms(), dev);

        s_ret = CVI_VI_GetPipeFrame(dev, stVideoFrame, 100);
        if (s_ret != CVI_SUCCESS)
        {
        	printf("camera error!\n");
            break;
        }
        else if ((int)aos_now_ms() - rOldTime > 1000)
        {
        	//printf("thread1, get camera... %d\n", (int)aos_now_ms());
        	rOldTime = (int)aos_now_ms();
        	// if (iCount++ < 5)
        	// 	debug_mm_overview(printf);
        }

        stVideoFrame[0].stVFrame.pu8VirAddr[0] = (CVI_U8 *)stVideoFrame[0].stVFrame.u64PhyAddr[0];

        unsigned char *ptr = (unsigned char*)stVideoFrame[0].stVFrame.pu8VirAddr[0];
        memcpy(g_imgBuf, ptr, 1600*900);

        CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

        iFrameCount ++;
        aos_msleep(30);
    }
	return NULL;
}

int my_flash_part_read(const char* part_name, unsigned int offset, void* buf, unsigned int length)
{
    int ret;
    partition_t partition = partition_open(part_name);
    if (partition < 0)
    {
        aos_debug_printf("[%s]part open fail: %s\n", __func__, part_name);
        return 0;
    }
    ret = partition_read(partition, offset, buf, length);
    partition_close(partition);
    if (ret)
    {
        aos_debug_printf("[%s]part read fail: %s(%d)\n", __func__, part_name, ret);
        return 0;
    }
    if (ret)
        return 0;
    else
        return length;
}

int my_flash_part_write(const char* part_name, unsigned int offset, void* buf, unsigned int length)
{
    partition_t partition = partition_open(part_name);
    if (partition < 0)
    {
        aos_debug_printf("[%s]part open fail: %s\n", __func__, part_name);
        return 0;
    }

    const int flash_page_size = 4*1024;
    unsigned int start_off = offset - (offset % flash_page_size);
    unsigned int page_count = (((offset + length) - start_off) + (flash_page_size - 1)) / flash_page_size;
    unsigned int end_off = start_off + page_count * flash_page_size;
    unsigned char* _tmp_buf;
    unsigned int write_len;
    unsigned int write_off;
    unsigned int total_len = 0;
    _tmp_buf = (unsigned char*)malloc(flash_page_size*2);
    if (_tmp_buf == NULL)
    {
        return 0;
    }
    for (; start_off < end_off; start_off += flash_page_size)
    {
        write_len = flash_page_size;
        write_off = 0;
        if (start_off + flash_page_size > offset + length)
            write_len = offset + length - start_off;
        if (start_off < offset)
        {
            write_off = offset - start_off;
            write_len = write_len - write_off;
        }
        int rcount = 6;
        int wcount = 0;
for_retry_one:
        partition_read(partition, start_off, _tmp_buf, flash_page_size);
        partition_read(partition, start_off, _tmp_buf + flash_page_size, flash_page_size);
        if (memcmp(_tmp_buf, _tmp_buf + flash_page_size, flash_page_size))
        {
            aos_debug_printf("[%d]read error/%d %08x\n", (int)aos_now_ms(), rcount, start_off);
            if (rcount > 0)
            {
                rcount--;
                aos_msleep(1);
                goto for_retry_one;
            }
            else
            {
                aos_debug_printf("ftw.\n"); //data may be corrupted,force to write
            }
        }
        if (memcmp(_tmp_buf + write_off, (void*)((char*)buf + total_len), write_len))
        {
            if (wcount == 1)
            {
                aos_debug_printf("write check error %08x\n", start_off);
            }
            memcpy(_tmp_buf + write_off, (void*)((char*)buf + total_len), write_len);
            partition_erase_size(partition, start_off, flash_page_size);
            partition_write(partition, start_off, _tmp_buf, flash_page_size);
            wcount ++;
            if (rcount > 0)
            {
                rcount--;
                goto for_retry_one;
            }
        }
        else
            rcount = 0;
        total_len += write_len;
    }
    free(_tmp_buf);
    partition_close(partition);
    return total_len;
}

#define buf_size        (32*1024)
unsigned char tmp_buf[buf_size];
unsigned char read_buf[buf_size];
void* thread_func2(void* arg)
{
    int n_max_time = 0;
    int n_min_time = 999999;
	aos_debug_printf("[%s] start ok\n", __func__);
    //flash write test
    aos_debug_printf("------------ flash test start\n");
    for (int i = 0x450000; i < 0x570000; i += buf_size)
    {
        int n_start_time = aos_now_ms();
        srand(n_start_time);
        for (int i = 0; i < buf_size; i ++)
            tmp_buf[i] = rand();
        aos_debug_printf("%d", (i / buf_size)+1);
        my_flash_part_write("misc", i, tmp_buf, buf_size);
        memset(read_buf, 0, buf_size);
        my_flash_part_read("misc", i, read_buf, buf_size);
        if (memcmp(tmp_buf, read_buf, buf_size) == 0)
            aos_debug_printf(",");
        else
            aos_debug_printf("(error),");
        n_start_time = aos_now_ms() - n_start_time;
        if (n_start_time > n_max_time)
            n_max_time = n_start_time;
        if (n_start_time < n_min_time)
            n_min_time = n_start_time;
    }
    aos_debug_printf("\n------------ flash test end, min=%dms, max=%dms\n", n_min_time, n_max_time);

    return NULL;
	
    int i = 0;
	printf("%s:%d\n", __FILE__, __LINE__);
	while(1)
	{
		proc_2(g_imgBuf, 1600, 900);
        int nRet = proc_1(g_imgBuf, 1440000);
        float rRet = forward_model(g_imgBuf, 108, 0.017f);
        if (i % 1000 == 0)
        {
	        printf("%d: %d %f \n", i, nRet, rRet);
	        printf("%d: %d \n", i, nRet);
        }
        i ++;
	}
	return NULL;
}


void* thread_func3(void* arg)
{
	printf("[%s] start\n", __func__);
	//MEDIA_AV_Init();
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thd1 = 0;
	pthread_t thd2 = 0;
	pthread_t thd3 = 0;
    pthread_attr_t a;
    pthread_attr_init(&a);
    a.stacksize = 16384;

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
//	printf("****** uac sampling rate: %d\n", CONFIG_UAC_SAMPLE_RATE);
//    MEDIA_AV_Init();
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
	aos_msleep(12);
	isp_daemon2_init(5566);
	#endif
	LOGI(TAG, "app start........\n");
	cvi_tpu_init();
	aos_msleep(100);
	printf("init tpu ok\n");
	// load_model();
	g_imgBuf = (unsigned char*)malloc(1600*1200);
	printf("create thread1\n");
	pthread_create(&thd1, NULL, thread_func1, NULL);
	printf("create thread2\n");
	pthread_create(&thd2, &a, thread_func2, NULL);
	printf("create thread3\n");
	pthread_create(&thd3, NULL, thread_func3, NULL);
	APP_CustomEventStart();
	while (1) {
		aos_msleep(300);
	};
}
