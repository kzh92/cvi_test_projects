#include "audiotask.h"

#if (USE_UAC_MODE)

#include <alsa/pcm.h>
#include "board.h"
#include "dirent.h"
#include "dw_i2s.h"
#include "fatfs_vfs.h"
#include "platform.h"
#include "cv181x_adc_dac.h"
#include "common_types.h"
#include "drv_gpio.h"
static aos_pcm_t *capture_handle;
static aos_pcm_t *playback_handle;

#define CAPTURE_SIZE 4096
#define PERIOD_FRAMES_SIZE 320
#define MAX_CAPTURE_SIZE 500 * 1024

mythread_ptr       recthread = 0;

struct timespec start, end;
unsigned int rate = 16000;
int byte_len = 0;
int captured_len = 0;
int capturing;

unsigned char *capture_buff = NULL;


void audio_capture_test_entry(void)
{
    int total_len;
    unsigned char *cap_buf;
    int capture_len;
    int ret;
    int dir = 1;
    aos_pcm_hw_params_t *capture_hw_params;
    cap_buf = (unsigned char *)my_malloc(CAPTURE_SIZE);
    capture_buff = (unsigned char *)my_malloc(MAX_CAPTURE_SIZE);
    if (cap_buf == NULL || capture_buff == NULL)
    {
        printf("%s malloc fail\n", __func__);
        return;
    }
    aos_pcm_open (&capture_handle, "pcmC0", AOS_PCM_STREAM_CAPTURE, 0);//打开设备“pcmC0
    aos_pcm_hw_params_alloca (&capture_hw_params);//申请硬件参数内存空间
    aos_pcm_hw_params_any (capture_handle, capture_hw_params);//初始化硬件参数
    capture_hw_params->period_size = PERIOD_FRAMES_SIZE;
    capture_hw_params->buffer_size = PERIOD_FRAMES_SIZE*4;
    aos_pcm_hw_params_set_access (capture_handle, capture_hw_params, AOS_PCM_ACCESS_RW_INTERLEAVED);// 设置音频数据参数为交错模式
    aos_pcm_hw_params_set_format (capture_handle, capture_hw_params, 16);//设置音频数据参数为小端16bit
    aos_pcm_hw_params_set_rate_near (capture_handle, capture_hw_params, &rate, &dir);//设置音频数据参数采样率为16K
    aos_pcm_hw_params_set_channels (capture_handle, capture_hw_params, 2);//设置音频数据参数为2通道
    aos_pcm_hw_params (capture_handle, capture_hw_params);//设置硬件参数到具体硬件中
    byte_len = aos_pcm_frames_to_bytes(capture_handle, PERIOD_FRAMES_SIZE);
    total_len = 0;
    while(capturing) {
        ret = aos_pcm_readi(capture_handle, cap_buf, PERIOD_FRAMES_SIZE);//接收交错音频数据
        capture_len = aos_pcm_frames_to_bytes(capture_handle, ret);
        memcpy(capture_buff + total_len, cap_buf, capture_len);
        total_len += capture_len;
        my_usleep(1000);
    }
    captured_len = total_len;
    my_free(cap_buf);
    // aos_pcm_close(capture_handle); //关闭设备
    // capture_handle = NULL;
}

static void audio_play_test_entry(u8 mode)
{
    unsigned char *cap_buf;
    unsigned char *play_buff = NULL;
    aos_pcm_hw_params_t *playback_hw_params;
    int total_len;
    int dir = 0;
    int play_len, play_cnt;
    int period_play_bytes = 0;
    cap_buf = (unsigned char *)my_malloc(CAPTURE_SIZE);
    if (cap_buf == NULL)
    {
        printf("%s malloc fail\n", __func__);
        return;
    }
    aos_pcm_open (&playback_handle, "pcmP0", AOS_PCM_STREAM_PLAYBACK, 0); //打开设备“pcmP0”
    aos_pcm_hw_params_alloca(&playback_hw_params); //申请硬件参数内存空间
    aos_pcm_hw_params_any(playback_handle, playback_hw_params); //初始化硬件参数
    playback_hw_params->period_size = PERIOD_FRAMES_SIZE;
    playback_hw_params->buffer_size = PERIOD_FRAMES_SIZE*4;
    aos_pcm_hw_params_set_access(playback_handle, playback_hw_params, AOS_PCM_ACCESS_RW_INTERLEAVED); // 设置音频数据参数为交错模式
    aos_pcm_hw_params_set_format(playback_handle, playback_hw_params, 16); //设置音频数据参数为小端16bit
    aos_pcm_hw_params_set_rate_near(playback_handle, playback_hw_params, &rate, &dir); //设置音频数据参数采样率为16K
    aos_pcm_hw_params_set_channels(playback_handle, playback_hw_params, 2); //设置音频数据参数为2通道
    aos_pcm_hw_params(playback_handle, playback_hw_params); //设置硬件参数到具体硬件?

    if (mode)//test audio pcm play
    {
        play_buff = (unsigned char *)my_malloc(FN_TESTAUDIO_PCM_SIZE);
        if (play_buff == NULL)
        {
            printf("%s malloc fail\n", __func__);
            return;
        }
        total_len = FN_TESTAUDIO_PCM_SIZE;
        fr_ReadFileData(FN_TESTAUDIO_PCM_PATH, 0, play_buff, FN_TESTAUDIO_PCM_SIZE);
    }
    else
        total_len = captured_len;
    play_cnt = 0;
    period_play_bytes = aos_pcm_frames_to_bytes(playback_handle, PERIOD_FRAMES_SIZE);

    while (total_len) {
        if (total_len > period_play_bytes)
            play_len = period_play_bytes;
        else
            play_len = total_len;
        if (mode)
            memcpy(cap_buf, play_buff + play_cnt * period_play_bytes, play_len);
        else
            memcpy(cap_buf, capture_buff + play_cnt * period_play_bytes, play_len);
        aos_pcm_writei(playback_handle,cap_buf, aos_pcm_bytes_to_frames(playback_handle,play_len));//发送交错音频数据
        total_len -= play_len;
        play_cnt++;
        my_usleep(1000);

        if(play_cnt == 5)
            GPIO_fast_setvalue(AUDIO_EN, ON);
    }

    GPIO_fast_setvalue(AUDIO_EN, OFF);
    my_usleep(100 * 1000);

    aos_pcm_close(playback_handle); //关闭设备
    playback_handle = NULL;
    my_free(cap_buf);
    if (mode == 0 && capture_buff)
    {
        my_free(capture_buff);
        capture_buff = NULL;
    }
    if (play_buff)
    {
        my_free(play_buff);
        play_buff = NULL;
    }
    //GPIO_fast_setvalue(15, 0);//SPK EN
}

void* test_record_thread(void*)
{
    dbug_printf("@@@ test record thread\n");
    audio_capture_test_entry();
    return NULL;
}

#if 0
void audio_test_vol_cmd(int32_t argc, char **argv)
{

    if(argc == 4){
        int ai_ao = atoi(argv[1]);
        int l_r = atoi(argv[2]);
        int val = atoi(argv[3]);
        u32 cmd;
        if(ai_ao){
            if(l_r)
                cmd = ACODEC_SET_DACR_VOL;
            else
                cmd = ACODEC_SET_DACL_VOL;
            cv182xdac_ioctl(cmd, (u64)&val);
        }else
        {
            if(l_r)
                cmd = ACODEC_SET_ADCR_VOL;
            else
                cmd = ACODEC_SET_ADCL_VOL;
            cv182xadc_ioctl(cmd, (u64)&val);
        }
        return;
    }
    else if(argc == 3)
    {
        int ai_ao = atoi(argv[1]);
        int val = atoi(argv[2]);

        if(ai_ao){
            cv182xdac_ioctl(ACODEC_SET_DACL_VOL, (u64)&val);
            cv182xdac_ioctl(ACODEC_SET_DACR_VOL, (u64)&val);
        }else
        {
            cv182xadc_ioctl(ACODEC_SET_ADCL_VOL, (u64)&val);
            cv182xadc_ioctl(ACODEC_SET_ADCR_VOL, (u64)&val);
        }
        return;
    }else if(argc == 2)
    {
        int ai_ao = atoi(argv[1]);
        u32 vol_l, vol_r;
        if(ai_ao){
            cv182xdac_ioctl(ACODEC_GET_DACL_VOL, (u64)&vol_l);
            cv182xdac_ioctl(ACODEC_GET_DACR_VOL, (u64)&vol_r);
            printf("ao vol:l = %d, r = %d\n", vol_l, vol_r);
        }else
        {
            cv182xadc_ioctl(ACODEC_GET_ADCL_VOL, (u64)&vol_l);
            cv182xadc_ioctl(ACODEC_GET_ADCR_VOL, (u64)&vol_r);
            printf("ai vol:l = %d, r = %d\n", vol_l, vol_r);
        }
        return;
    }else
    {
        printf("invalid cmd params.\n");
        printf("usage1:%s [0{ai},1{ao}] [0[l],1[r]] [val]\n", argv[0]);
        printf("usage2:%s [0{ai},1{ao}] [val]\n", argv[0]);
        printf("usage3:%s [0{ai},1{ao}]\n", argv[0]);
    }
    return;
}
#endif

void test_Audio()
{
    GPIO_fast_setvalue(AUDIO_EN, OFF);
    capturing = 1;
    if(my_thread_create_ext(&recthread, 0, test_record_thread, NULL, (char*)"record_test", 8192, MYTHREAD_PRIORITY_HIGH))
        printf("[record_thread]create thread error.\n");
    //aos_msleep(300);
    audio_play_test_entry(1);
    capturing = 0;
    aos_msleep(300);
    my_thread_join(&recthread);
    audio_play_test_entry(0);
}

#endif// USE_UAC_MODE