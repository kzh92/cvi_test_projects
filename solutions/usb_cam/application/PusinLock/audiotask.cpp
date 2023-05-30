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
    GPIO_fast_setvalue(15, 1);//SPK EN
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
    }
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
    GPIO_fast_setvalue(15, 0);//SPK EN
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
    capturing = 1;
    if(my_thread_create_ext(&recthread, 0, test_record_thread, NULL, (char*)"record_test", 8192, MYTHREAD_PRIORITY_MEDIUM))
        printf("[record_thread]create thread error.\n");
    aos_msleep(300);
    audio_play_test_entry(1);
    capturing = 0;
    aos_msleep(300);
    my_thread_join(&recthread);
    audio_play_test_entry(0);
}

#endif// USE_UAC_MODE

#if (USE_WIFI_MODULE)

#include "drv_gpio.h"
#include "appdef.h"
//#include "asoundlib.h"
#include "noise_suppression.h"
#include "g711/g711_table.h"
#include "vdbtask.h"
#include "FaceRetrievalSystem_base.h"
#include "soundbase.h"

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ai.h"
#include "mi_ao.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>

#define NO_USE_ALSA

#define SAVE_RAW_FILE 0
#define SAMPLE_RATE 8000

#define USER_BUF_DEPTH      (4)
#define TOTAL_BUF_DEPTH		(8)

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif  //	MIN

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif  //	MAX

#define TEST_BUFFER_SIZE    2048
#define TEST_RECORD_SIZE    100 * 1024

enum nsLevel {
        kLow,
        kModerate,
        kHigh,
        kVeryHigh
    };

extern MySpiThread* g_pMySpiThread;

NsHandle *nsHandle = NULL;
int16_t *frameBuffer;
void denoise_create();
void denoise(int16_t *input, int samplesCount);
void denoise_free();
int play_test(unsigned int id);

int capturing;
int closed;
unsigned int play_test_id = 0;

#define SAMPLE_NUMBER (320)//320

char buffer[SAMPLE_NUMBER * 2] = {0};
char g711_buffer[SAMPLE_NUMBER] = {0};
char* out_buffer = NULL;
char* g711_play_buffer = NULL;
char *record_buffer = NULL;
unsigned int capture_len = 0;

mythread_ptr       recthread = 0;
mythread_ptr       playthread = 0;

typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    char chRIFF[4];
    unsigned int  dwRIFFLen;
    char chWAVE[4];
    char chFMT[4];
    unsigned int  dwFMTLen;
    WaveFormat_t wave;
    char chDATA[4];
    unsigned int  dwDATALen;
} WaveFileHeader_t;

unsigned int capture_sample();
void play_sample();
unsigned int capture_test(char * buff);

void* play_thread(void*)
{
    my_printf("play thread\n");

    play_sample();
    return NULL;
}

void* record_thread(void*)
{
    my_printf("record thread\n");

    capture_sample();
    return NULL;
}

void* test_record_thread(void*)
{
    my_printf("test record thread\n");

    capture_test(record_buffer);
    return NULL;
}

void* test_play_thread(void*)
{
    my_printf("test play thread\n");

    if (play_test_id)
    {
        play_test(play_test_id);
        play_test_id = 0;
    }
    return NULL;
}

void StartAudioTask()
{
    //GPIO_fast_setvalue(AUDIO_EN, ON);
    capturing = 1;
    closed = 0;

    denoise_create();

    pcm16_alaw_tableinit();
    alaw_pcm16_tableinit();

    if(my_thread_create_ext(&playthread, 0, play_thread, NULL, (char*)"play", 8192, 0))
        my_printf("[play_thread]create thread error.\n");
    if(my_thread_create_ext(&recthread, 0, record_thread, NULL, (char*)"record", 8192, 0))
        my_printf("[record_thread]create thread error.\n");
}

void StopAudioTask()
{
    capturing = 0;
    closed = 1;

    if(playthread)
    {
        my_thread_join(&playthread);
        playthread = 0;
    }

    if(recthread)
    {
        my_thread_join(&recthread);
        recthread = 0;
    }
    denoise_free();
    GPIO_fast_setvalue(AUDIO_EN, OFF);
}

void StartWAVPlay(unsigned int id)
{
    play_test_id = id;
    if(my_thread_create_ext(&recthread, 0, test_play_thread, NULL, (char*)"record_test", 8192, MYTHREAD_PRIORITY_HIGH))
        my_printf("[StartWAVPlay]create thread error.\n");
}

MI_S32 AIEnable(MI_U32 SampleRate)
{
    MI_AUDIO_DEV AiDevId = 0;
    MI_AUDIO_DEV AiChnId = 0;
    MI_S32              ret = MI_SUCCESS;
    MI_AUDIO_Attr_t     stAiSetAttr;
    MI_U32              u32ChnCnt;
    MI_SYS_ChnPort_t    stAiChnOutputPort0[MI_AUDIO_MAX_CHN_NUM];

    u32ChnCnt = 1;
    memset(&stAiSetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));
    stAiSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAiSetAttr.eSamplerate = (MI_AUDIO_SampleRate_e)SampleRate;
    stAiSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAiSetAttr.u32ChnCnt = u32ChnCnt;
    stAiSetAttr.u32PtNumPerFrm = SAMPLE_NUMBER; //stAiSetAttr.eSamplerate / 16; // for aec
    stAiSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE;
    stAiSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAiSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
    stAiSetAttr.WorkModeSetting.stI2sConfig.u32TdmSlots = 4;
    stAiSetAttr.WorkModeSetting.stI2sConfig.eI2sBitWidth = E_MI_AUDIO_BIT_WIDTH_16;
    ret = MI_AI_SetPubAttr(AiDevId, &stAiSetAttr);
    if(MI_SUCCESS != ret)
    {
        my_printf("set ai %d attr err:0x%x\n", AiDevId, ret);
        return ret;
    }
    ret = MI_AI_Enable(AiDevId);
    if(MI_SUCCESS != ret)
    {
        my_printf("enable ai %d err:0x%x\n", AiDevId, ret);
        return ret;
    }
    ret = MI_AI_SetVqeVolume(AiDevId, AiChnId, DEFAULT_MI_AI_VOLUME);
    if(MI_SUCCESS != ret)
    {
        my_printf("MI_AI_SetVqeVolume failed\n");
        return ret;
    }

    memset(&stAiChnOutputPort0, 0x0, sizeof(stAiChnOutputPort0));
    stAiChnOutputPort0[AiChnId].eModId = E_MI_MODULE_ID_AI;
    stAiChnOutputPort0[AiChnId].u32DevId = AiDevId;
    stAiChnOutputPort0[AiChnId].u32ChnId = AiChnId;
    stAiChnOutputPort0[AiChnId].u32PortId = 0;
    MI_SYS_SetChnOutputPortDepth(&stAiChnOutputPort0[AiChnId], USER_BUF_DEPTH, TOTAL_BUF_DEPTH);
    ret = MI_AI_EnableChn(AiDevId, AiChnId);
    if(MI_SUCCESS != ret)
    {
        my_printf("enable Dev%d Chn%d err:0x%x\n", AiDevId, AiChnId, ret);
        return ret;
    }

    return ret;
}

void AIDisable(void)
{
    MI_AUDIO_DEV AiDevId = 0;
    MI_AUDIO_DEV AiChnId = 0;
    MI_AI_DisableChn(AiDevId, AiChnId);
    MI_AI_Disable(AiDevId);
}

unsigned int capture_sample()
{
    MI_AUDIO_DEV AiDevId = 0;
    MI_AUDIO_DEV AiChnId = 0;
    MI_S32              s32Ret;
    MI_AUDIO_Frame_t stAiChFrame;
    MI_AUDIO_AecFrame_t stAecFrame;

    s32Ret = AIEnable(E_MI_AUDIO_SAMPLE_RATE_8000);
    if(s32Ret != MI_SUCCESS)
        return s32Ret;

    memset(&stAiChFrame, 0, sizeof(MI_AUDIO_Frame_t));
    memset(&stAecFrame, 0, sizeof(MI_AUDIO_AecFrame_t));

    while (capturing) {
        s32Ret = MI_AI_GetFrame(AiDevId, AiChnId, &stAiChFrame, &stAecFrame, -1);
        if (MI_SUCCESS == s32Ret)
        {
            memcpy(buffer, stAiChFrame.apVirAddr[0], stAiChFrame.u32Len[0]);
            denoise((short*)buffer, SAMPLE_NUMBER);
            pcm16_to_alaw(SAMPLE_NUMBER * 2, (const char*)buffer, g711_buffer);
            if(g_pMySpiThread)
                g_pMySpiThread->PushAudioBuf(g711_buffer, SAMPLE_NUMBER);
#if (SAVE_RAW_FILE == 1)
            FILE* _fp;
            _fp = fopen("/mnt/rec_denoise.raw", "a+");
            if (_fp)
            {
                fwrite(stAiChFrame.apVirAddr[0], stAiChFrame.u32Len[0], 1, _fp);
                fclose(_fp);
            }
#endif

            MI_AI_ReleaseFrame(AiDevId, AiChnId,  &stAiChFrame,  NULL);
        }
        else
        {
            my_printf("Dev%dChn%d get frame failed!!!error:0x%x\n", AiDevId, AiChnId, s32Ret);
        }
    }

    AIDisable();

    return 1;
}

unsigned int capture_test(char * buff)
{
    MI_AUDIO_DEV AiDevId = 0;
    MI_AUDIO_DEV AiChnId = 0;
    MI_S32              s32Ret;
    MI_AUDIO_Frame_t stAiChFrame;
    MI_AUDIO_AecFrame_t stAecFrame;

    s32Ret = AIEnable(E_MI_AUDIO_SAMPLE_RATE_8000);
    if(s32Ret != MI_SUCCESS)
        return s32Ret;

    memset(&stAiChFrame, 0, sizeof(MI_AUDIO_Frame_t));
    memset(&stAecFrame, 0, sizeof(MI_AUDIO_AecFrame_t));

    while (capturing) {
        s32Ret = MI_AI_GetFrame(AiDevId, AiChnId, &stAiChFrame, &stAecFrame, -1);
        if (MI_SUCCESS == s32Ret)
        {
            capture_len += stAiChFrame.u32Len[0];
            memcpy(buff, stAiChFrame.apVirAddr[0], stAiChFrame.u32Len[0]);
            buff += stAiChFrame.u32Len[0];
            MI_AI_ReleaseFrame(AiDevId, AiChnId,  &stAiChFrame,  NULL);
        }
        else
        {
            my_printf("Dev%dChn%d get frame failed!!!error:0x%x\n", AiDevId, AiChnId, s32Ret);
        }
    }

    AIDisable();

    return 1;
}

MI_S32 AOEnable(MI_U32 SampleRate)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;
    MI_S32 ret;
    MI_AUDIO_Attr_t stAoSetAttr;
    MI_S32 s32AoGetVolume;
    MI_U32 u32PhyChnNum;
    MI_S32 s32NeedSize = 0;

    memset(&stAoSetAttr, 0x0, sizeof(MI_AUDIO_Attr_t));
    stAoSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAoSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_SLAVE;
    stAoSetAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE;
    stAoSetAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAoSetAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
    stAoSetAttr.WorkModeSetting.stI2sConfig.u32TdmSlots = 4;
    stAoSetAttr.WorkModeSetting.stI2sConfig.eI2sBitWidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAoSetAttr.u32PtNumPerFrm = SAMPLE_NUMBER;
    stAoSetAttr.u32FrmNum = 1024;
    stAoSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    stAoSetAttr.u32ChnCnt = 1;
    stAoSetAttr.eSamplerate = (MI_AUDIO_SampleRate_e)SampleRate;
    ret = MI_AO_SetPubAttr(AoDevId, &stAoSetAttr);
    if(MI_SUCCESS != ret)
    {
        my_printf("set ao %d attr err:0x%x\n", AoDevId, ret);
        return ret;
    }
    ret = MI_AO_Enable(AoDevId);
    if(MI_SUCCESS != ret)
    {
        my_printf("enable ao dev %d err:0x%x\n", AoDevId, ret);
        return ret;
    }
    ret = MI_AO_EnableChn(AoDevId, AoChn);
    if(MI_SUCCESS != ret)
    {
        my_printf("enable ao dev %d chn %d err:0x%x\n", AoDevId, AoChn, ret);
        return ret;
    }
    MI_AO_SetVolume(AoDevId, 0, DEFAULT_MI_AO_VOLUME, E_MI_AO_GAIN_FADING_OFF);
    MI_AO_GetVolume(AoDevId, 0, &s32AoGetVolume);

    switch (stAoSetAttr.eSoundmode)
    {
        case E_MI_AUDIO_SOUND_MODE_MONO:
            u32PhyChnNum = 1;
            break;

        case E_MI_AUDIO_SOUND_MODE_STEREO:
            u32PhyChnNum = 2;
            break;

        default:
            break;
    }

    s32NeedSize = SAMPLE_NUMBER * 2 * u32PhyChnNum * stAoSetAttr.u32ChnCnt;
    s32NeedSize = s32NeedSize / (stAoSetAttr.u32ChnCnt * 2 * u32PhyChnNum) * (stAoSetAttr.u32ChnCnt * 2 * u32PhyChnNum);

    return MI_SUCCESS;
}

void AODisable()
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;
    MI_AO_DisableChn(AoDevId, AoChn);
    MI_AO_Disable(AoDevId);
}

int idx = 0;
void play_sample()
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;
    int iGPIOEnabled = 0;

    s32Ret = AOEnable(E_MI_AUDIO_SAMPLE_RATE_8000);
    if(s32Ret != MI_SUCCESS)
        return;

    do {
        int iBufLen = 0;
        if(g_pMySpiThread)
            iBufLen = g_pMySpiThread->PopAudioBuf(&g711_play_buffer);
        if(iBufLen > 0)
        {
            if (iGPIOEnabled == 0)
            {
                GPIO_fast_setvalue(AUDIO_EN, ON);
                iGPIOEnabled = 1;
            }
            out_buffer = (char*)my_malloc(iBufLen * 2);
            alaw_to_pcm16(iBufLen, (const char*)g711_play_buffer, out_buffer);


            memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
            stAoSendFrame.u32Len[0] = iBufLen * 2;
            stAoSendFrame.apVirAddr[0] = out_buffer;
            stAoSendFrame.apVirAddr[1] = NULL;

            do{
                s32Ret = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, -1);
            }while(s32Ret == MI_AO_ERR_NOBUF);

            if (s32Ret != MI_SUCCESS)
            {
                my_printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
            }

            while(1)
            {
                MI_AO_ChnState_t stStatus;
                s32Ret = MI_AO_QueryChnStat(AoDevId, AoChn, &stStatus);
                if (MI_SUCCESS != s32Ret)
                {
                    my_printf("query chn status ao dev %d chn %d err:0x%x\n", AoDevId, AoChn, s32Ret);
                    break;
                }

                if(stStatus.u32ChnBusyNum < SAMPLE_NUMBER * 2)
                    break;

                my_usleep(1000);
            }

            my_free(out_buffer);
            out_buffer = NULL;
        }
        else
            my_usleep(20*1000);

        if(g711_play_buffer)
        {
            my_free(g711_play_buffer);
            g711_play_buffer = NULL;
        }

#if (SAVE_RAW_FILE == 1 && 0)
        FILE* _fp;
        _fp = fopen("/mnt/play_out.raw", "a+");
        if (_fp)
        {
            fwrite(out_buffer, iBufLen * 2, 1, _fp);
            fclose(_fp);
        }
#endif
    } while (!closed);

    AODisable();

    return;
}

MI_S32 play_file_test(const char *WavAudioFile)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;
    int iGPIOEnabled = 0;
    MI_S32 s32RemainSize;
    MI_S32 s32NeedSize = 0;
    myfdesc_ptr AoReadFd;
    WaveFileHeader_t stWavHeaderInput;

    AoReadFd = my_open((const char *)WavAudioFile, O_RDONLY, 0);
    if(AoReadFd <= 0)
    {
        my_printf("Open input file failed:%s \n", WavAudioFile);
        return -1;
    }

    s32Ret = my_read(AoReadFd, &stWavHeaderInput, sizeof(WaveFileHeader_t));
    if (s32Ret < 0)
    {
        my_printf("Read wav header failed!!!\n");
        return -1;
    }

    s32RemainSize = stWavHeaderInput.dwDATALen;

    s32Ret = AOEnable(44100);
    if(s32Ret != MI_SUCCESS)
        return s32Ret;

    while (s32RemainSize > 0) {
        if (iGPIOEnabled == 0)
        {
            GPIO_fast_setvalue(AUDIO_EN, ON);
            iGPIOEnabled = 1;
        }

        s32NeedSize = s32RemainSize > TEST_BUFFER_SIZE ? TEST_BUFFER_SIZE : s32RemainSize;

        out_buffer = (char*)my_malloc(s32NeedSize);

        s32Ret = my_read(AoReadFd, &out_buffer, s32NeedSize);
        if(s32Ret != s32NeedSize)
        {
            my_printf("Input file does not has enough data!!!\n");
            my_free(out_buffer);
            out_buffer = NULL;
            break;
        }

        memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
        stAoSendFrame.u32Len[0] = s32NeedSize;
        stAoSendFrame.apVirAddr[0] = out_buffer;
        stAoSendFrame.apVirAddr[1] = NULL;

        do{
            s32Ret = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, -1);
        }while(s32Ret == MI_AO_ERR_NOBUF);

        if (s32Ret != MI_SUCCESS)
        {
            my_printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
        }

        while(1)
        {
            MI_AO_ChnState_t stStatus;
            s32Ret = MI_AO_QueryChnStat(AoDevId, AoChn, &stStatus);
            if (MI_SUCCESS != s32Ret)
            {
                my_printf("query chn status ao dev %d chn %d err:0x%x\n", AoDevId, AoChn, s32Ret);
                break;
            }

            if(stStatus.u32ChnBusyNum < SAMPLE_NUMBER * 2)
                break;

            my_usleep(1000);
        }

        s32RemainSize -= s32NeedSize;

        my_free(out_buffer);
        out_buffer = NULL;
    }

    my_close(AoReadFd);

    AODisable();

    return MI_SUCCESS;
}

int play_test(unsigned int id)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;
    unsigned int play_size;
    WaveFileHeader_t stWavHeaderInput;

    char filename[128];
    unsigned int play_size_list[6] = {FN_031TTS_WAV_SIZE, FN_032TTS_WAV_SIZE, FN_033TTS_WAV_SIZE, FN_034TTS_WAV_SIZE, FN_035TTS_WAV_SIZE, FN_036TTS_WAV_SIZE};
    if (id == 255)
        play_size = FN_TEST_WAV_SIZE;
    else if (id < 7 && id > 0)
        play_size = play_size_list[id - 1];
    else
        return 0;

    out_buffer = (char*)my_malloc(play_size);
    if (out_buffer == NULL)
    {
        my_printf("@@@ play_test buffer malloc fail\n");
        return -1;
    }

    if (id > 6)
        fr_ReadFileData(FN_TEST_WAV_PATH, 0, out_buffer, play_size);
    else
    {
        sprintf(filename, "sound/%03dTTS.wav", SID_WIFI_SOUND_BASE + id);
        fr_ReadFileData(filename, 0, out_buffer, play_size);
    }
    char * tmp_buf = out_buffer;

    memcpy(&stWavHeaderInput, out_buffer, sizeof(stWavHeaderInput));

    s32Ret = AOEnable(stWavHeaderInput.wave.dwSamplesPerSec);
    if(s32Ret != MI_SUCCESS)
        return s32Ret;

    GPIO_fast_setvalue(AUDIO_EN, ON);

    while (1) {
        memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
        if (play_size <= TEST_BUFFER_SIZE)
            stAoSendFrame.u32Len[0] = play_size;
        else
            stAoSendFrame.u32Len[0] = TEST_BUFFER_SIZE;
        stAoSendFrame.apVirAddr[0] = tmp_buf;
        stAoSendFrame.apVirAddr[1] = NULL;

        do{
            s32Ret = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, -1);
        }while(s32Ret == MI_AO_ERR_NOBUF);

        if (s32Ret != MI_SUCCESS)
        {
            my_printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
        }

        while(1)
        {
            MI_AO_ChnState_t stStatus;
            s32Ret = MI_AO_QueryChnStat(AoDevId, AoChn, &stStatus);
            if (MI_SUCCESS != s32Ret)
            {
                my_printf("query chn status ao dev %d chn %d err:0x%x\n", AoDevId, AoChn, s32Ret);
                break;
            }

            if(stStatus.u32ChnBusyNum < SAMPLE_NUMBER * 2)
                break;

            my_usleep(1000);
        }

        if (play_size <= TEST_BUFFER_SIZE)
            break;

        play_size -= TEST_BUFFER_SIZE;
        tmp_buf += TEST_BUFFER_SIZE;
    }
    GPIO_fast_setvalue(AUDIO_EN, OFF);
    my_free(out_buffer);
    AODisable();

    return MI_SUCCESS;
}

int play_buf_test(char* out_buffer, unsigned int size)
{
    MI_AUDIO_DEV AoDevId = 0;
    MI_AO_CHN AoChn = 0;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_AUDIO_Frame_t stAoSendFrame;
    unsigned int play_size = size;

    s32Ret = AOEnable(E_MI_AUDIO_SAMPLE_RATE_8000);
    if(s32Ret != MI_SUCCESS)
        return s32Ret;

    char * tmp_buf = out_buffer;

    GPIO_fast_setvalue(AUDIO_EN, ON);

    while (1) {
        memset(&stAoSendFrame, 0x0, sizeof(MI_AUDIO_Frame_t));
        if (play_size <= TEST_BUFFER_SIZE)
            stAoSendFrame.u32Len[0] = play_size;
        else
            stAoSendFrame.u32Len[0] = TEST_BUFFER_SIZE;
        stAoSendFrame.apVirAddr[0] = tmp_buf;
        stAoSendFrame.apVirAddr[1] = NULL;

        do{
            s32Ret = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, -1);
        }while(s32Ret == MI_AO_ERR_NOBUF);

        if (s32Ret != MI_SUCCESS)
        {
            my_printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n", s32Ret);
        }

        while(1)
        {
            MI_AO_ChnState_t stStatus;
            s32Ret = MI_AO_QueryChnStat(AoDevId, AoChn, &stStatus);
            if (MI_SUCCESS != s32Ret)
            {
                my_printf("query chn status ao dev %d chn %d err:0x%x\n", AoDevId, AoChn, s32Ret);
                break;
            }

            if(stStatus.u32ChnBusyNum < SAMPLE_NUMBER * 2)
                break;

            my_usleep(1000);
        }

        if (play_size <= TEST_BUFFER_SIZE)
            break;

        play_size -= TEST_BUFFER_SIZE;
        tmp_buf += TEST_BUFFER_SIZE;
    }
    GPIO_fast_setvalue(AUDIO_EN, OFF);
    AODisable();

    return MI_SUCCESS;
}

void test_Audio()
{
    record_buffer = (char*)my_malloc(TEST_RECORD_SIZE);
    memset(record_buffer, 0, TEST_RECORD_SIZE);
    if (record_buffer == NULL)
    {
        my_printf("@@@ test_Audio malloc fail\n");
        return;
    }

    capturing = 1;
    if(my_thread_create_ext(&recthread, 0, test_record_thread, NULL, (char*)"record_test", 8192, 0))
        my_printf("[record_thread]create thread error.\n");

    play_test(255);

    capturing = 0;
    my_usleep(300 * 1000);
    my_thread_join(&recthread);

    my_usleep(1000 * 1000);
    if (capture_len)
    {
        play_buf_test(record_buffer, capture_len);
    }

    my_free(record_buffer);
}
void denoise_create() {
    enum nsLevel level = kVeryHigh;
    size_t samples = MIN(160, SAMPLE_RATE / 100);
    frameBuffer = (int16_t *)my_malloc(sizeof(*frameBuffer) * samples);

    nsHandle = WebRtcNs_Create();
    //int status = 
    WebRtcNs_Init(nsHandle, SAMPLE_RATE);
    //status = 
    WebRtcNs_set_policy(nsHandle, level);
}

void denoise(int16_t *input, int samplesCount)
{
    size_t samples = MIN(160, SAMPLE_RATE / 100);
    size_t frames = (samplesCount / samples);

    for (int i = 0; i < (int)frames; i++)
    {
        memcpy(frameBuffer, input, samples * sizeof(int16_t));

        int16_t *nsIn[1] = { frameBuffer };   //ns input[band][data]
        int16_t *nsOut[1] = { frameBuffer };  //ns output[band][data]
        WebRtcNs_Analyze(nsHandle, nsIn[0]);
        WebRtcNs_Process(nsHandle, (const int16_t *const *)nsIn, 1, nsOut);

        memcpy(input, frameBuffer, samples * sizeof(int16_t));
        input += samples;
    }
}

void denoise_free() {
    if (nsHandle)
    {
        WebRtcNs_Free(nsHandle);
        nsHandle = NULL;
        my_free(frameBuffer);
    }
}

#endif // USE_WIFI_MODULE