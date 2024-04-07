#include "vdbtask.h"
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
#include "uvc_func.h"
#if (USE_WIFI_MODULE)
#include "audiotask.h"
#include "uartcomm.h"
#include "jiwei_base.h"
#include "soundbase.h"
#endif // USE_WIFI_MODULE

// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <time.h>
#include <string.h>
// #include <math.h>
#include <fcntl.h>
#include <errno.h>

FaceRecogTask* g_pFaceRecogTask = NULL;
mythread_ptr       spi_thread = NULL;

//#define     N_MAX_JPEG_SEND_LEN     (60 * 1024)
#define     N_MAX_JPEG_SEND_LEN     (UVC_WIDTH * UVC_HEIGHT * 3 / 2)

extern "C" int MEDIA_AV_Init();
extern "C" int MEDIA_UVC_Init();

#if (USE_VDBTASK)
int VDBTask::CaptureCam = -1;
int VDBTask::m_iCounter = 0;

void* SpiThread_ThreadProc1(void* param);
#endif // USE_VDBTASK

unsigned char*  g_abJpgData = NULL;
int             g_iJpgDataLen = 0;

#if (USE_VDBTASK)
void* vdbTask_ThreadProc1(void* param);
int ConvertYuvToSceneJpeg(unsigned char* pbSrc, int iYUYVMode)
{
    unsigned char* pbTmp = pbSrc;
    unsigned char* pbYUV = NULL;
    g_clrRgbData = (unsigned char*)my_malloc(CLR_CAM_WIDTH * ALIGN_16B(CLR_CAM_HEIGHT) * 3);
    if (g_clrRgbData == NULL)
    {
        my_printf("@@@ g_clrRgbData my_malloc fail\n");
        return 0;
    }
    if(g_abJpgData == NULL)
    {
        g_abJpgData = (unsigned char*)my_malloc(128 * 1024);
        g_iJpgDataLen = 0;
    }

    if(iYUYVMode == 1)
    {
        pbYUV = (unsigned char*)my_malloc(CLR_CAM_WIDTH * CLR_CAM_HEIGHT * 3 >> 1);
        if(!pbYUV)
        {
            my_free(g_clrRgbData);
            return 0;
        }

        ConvertYUYV_toYUV420(pbSrc, CLR_CAM_WIDTH, CLR_CAM_HEIGHT, pbYUV);
        pbTmp = pbYUV;
    }
    rotateYUV420SP_flip(pbTmp, CLR_CAM_WIDTH, CLR_CAM_HEIGHT, g_clrYuvData, g_xPS.x.bCamFlip == 0 ? 270: 90, 1);

    ConvertYUV420_NV21toRGB888(g_clrYuvData, CLR_CAM_HEIGHT, CLR_CAM_WIDTH, g_clrRgbData);

    Shrink_RGB(g_clrRgbData, CLR_CAM_WIDTH, CLR_CAM_HEIGHT, g_clrYuvData, CAPTURE_HEIGHT, CAPTURE_WIDTH);

    int iWriteLen = 0;
    for(int i = 90; i >= 10; i -= 10)
    {
        jpge::params params;
        params.m_quality = i;
        params.m_subsampling = jpge::H2V2;

        iWriteLen = 128 * 1024;
        if(!jpge::compress_image_to_jpeg_file_in_memory(g_abJpgData, iWriteLen, CAPTURE_WIDTH, CAPTURE_HEIGHT, 3, g_clrYuvData, params))
        {
            iWriteLen = 0;
            break;
        }

        if(iWriteLen < 10 * 1024)
        {
            my_printf("Jpeg Quality: %d\n", i);
            break;
        }
    }

    g_iJpgDataLen = iWriteLen;
    if(iYUYVMode == 1)
        my_free(pbYUV);
    my_free(g_clrRgbData);

    return iWriteLen;
}

int ConvertIRToSceneJpeg(unsigned char* pbSrc)
{
    g_clrRgbData = (unsigned char*)my_malloc(IR_CAM_WIDTH * ALIGN_16B(IR_CAM_HEIGHT) * 3);
    if (g_clrRgbData == NULL)
    {
        my_printf("@@@ g_clrRgbData my_malloc fail\n");
        return 0;
    }
    Shrink_Grey(pbSrc, IR_CAM_HEIGHT / 3 * 4, IR_CAM_HEIGHT, g_clrRgbData, CAPTURE_HEIGHT, CAPTURE_WIDTH);

    int iWriteLen = 0;
    for(int i = 90; i >= 10; i -= 10)
    {
        jpge::params params;
        params.m_quality = i;
        params.m_subsampling = jpge::Y_ONLY;

        iWriteLen = 128 * 1024;
        if(!jpge::compress_image_to_jpeg_file_in_memory(g_abJpgData, iWriteLen, CAPTURE_WIDTH, CAPTURE_HEIGHT, 1, g_clrRgbData, params))
        {
            iWriteLen = 0;
            break;
        }

        if(iWriteLen < 10 * 1024)
        {
            my_printf("Jpeg Quality: %d\n", i);
            break;
        }
    }

    g_iJpgDataLen = iWriteLen;
    my_free(g_clrRgbData);
    return iWriteLen;
}


VDBTask::VDBTask()
{
    m_iRunning = 0;
    m_thread = 0;
}

VDBTask::~VDBTask()
{

}

void VDBTask::Start()
{
    VDBTask::CaptureCam = -1;

    m_iCounter ++;
    m_iRunning = 1;
    if(my_thread_create_ext(&m_thread, NULL, vdbTask_ThreadProc1, this, (char*)"vdbtask", 128*1024, MYTHREAD_PRIORITY_HIGH))
        my_printf("[VDBTask]create thread error.\n");
    // Thread::Start();
}

void VDBTask::Stop()
{
#ifdef AUDIO_EN
    GPIO_fast_setvalue(AUDIO_EN, OFF);
#endif
    m_iRunning = 0;
    if (m_thread != NULL)
    {
        my_thread_join(&m_thread);
        m_thread = NULL;
    }
    // Thread::Wait();
}

void VDBTask::Pause()
{
    m_iRunning = 0;
}

int VDBTask::IsStreaming()
{
    return 0;//darkhorse
    //return UVC_IsStreaming(0);
}

void VDBTask::run()
{
    // start uvc
    MEDIA_AV_Init();
    StartCamSurface(0);
    my_printf("vdb task end\n");
}

void VDBTask::ThreadProc()
{
    run();
}

void* vdbTask_ThreadProc1(void* param)
{
    VDBTask* pThread = (VDBTask*)(param);
    pThread->ThreadProc();
    return NULL;
}

#if (USE_WIFI_MODULE)

MySpiThread::MySpiThread()
{
    Init();
}

void MySpiThread::Init()
{
    m_iRunning = 0;
    m_spiFd = -1;
    m_pushMutex = my_mutex_init();
    m_popMutex = my_mutex_init();
}

void MySpiThread::Start()
{
    m_iRunning = 1;
    memset(m_sendAudioBuf, 0, sizeof(m_sendAudioBuf));
    m_iSendAudioLen = 0;
    memset(m_recvAudioBuf, 0, sizeof(m_recvAudioBuf));
    m_iRecvAudioLen = 0;

#if 1
    if(my_thread_create_ext(&spi_thread, NULL, SpiThread_ThreadProc1, this, (char*)"spi", 8192, 0))
        my_printf("[SpiThread]create thread error.\n");
#else
    run();
#endif
}

void MySpiThread::Stop()
{
    m_iRunning = 0;
    //Thread::Wait();
    if(spi_thread)
        my_thread_join(&spi_thread);
}

unsigned char GetYinghuaJiWeiCheckSum(WIFI_CMD cmd)
{
    return (cmd.xYongHua_JiWei.bHeader1 + cmd.xYongHua_JiWei.bHeader2 + cmd.xYongHua_JiWei.bCmd + cmd.xYongHua_JiWei.bData[0] +
            cmd.xYongHua_JiWei.bData[1] + cmd.xYongHua_JiWei.bData[2] + cmd.xYongHua_JiWei.bData[3] + cmd.xYongHua_JiWei.bData[4] +
            cmd.xYongHua_JiWei.bData[5]) & 0xFF;
}

unsigned char GetWifiCommonCheckSum(WIFI_CMD xCmd, int iLen)
{
    unsigned char bCheckSum = 0;
    for(int i = 0; i < iLen - 1; i ++)
        bCheckSum += xCmd.abData[i];

    return bCheckSum;
}

void MySpiThread::run()
{
    unsigned char uart_cmd_buffer[256];
    int read_len = 0;
    int copy_len = 0;
    int i = 0;
    WIFI_CMD xCmd;
    // WIFI_CMD xSendCmd;
    char szQRCode[256] = {0};

    unsigned char *abJpegBuf = NULL;
    abJpegBuf = (unsigned char*)my_malloc(128*1024);
    if (abJpegBuf == NULL)
        my_printf("@@@ abJpegBuf malloc fail\n");

    int iJpegSize = 0;

    int uart_ret = uart3Open();
    spiOpen();
    my_printf("[spithread]uart_ret=%d, spi=%d\n", uart_ret, m_spiFd);

    float rSendPicTime = 0;
    float rRecogQRStart = 0;

    while(m_iRunning && uart_ret > -1)
    {
        if (g_xSS.iStartOta) break;
        if(rSendPicTime != 0 && Now() - rSendPicTime > 2000)
        {
            my_printf("[spithread] JIWEI END\n");
            g_xSS.iJiweiStart = 0;
            StopAudioTask();
            rSendPicTime = 0;
        }

        if(rRecogQRStart != 0 && Now() - rRecogQRStart > g_xSS.iRecogQRTimeout * 1000)
        {
            my_printf("[spithread] The mode to recog QR Code has finished\n");
            g_xSS.iRecogQRMode = 0;
            g_xSS.iRecogQRTimeout = 0;
            memset(g_xSS.strQRContents, 0, sizeof(g_xSS.strQRContents));
            memset(szQRCode, 0, sizeof(szQRCode));
            rRecogQRStart = 0;
        }

        if(g_xSS.iRecogQRMode == 1 && strlen(g_xSS.strQRContents) > 0 && strcmp(g_xSS.strQRContents, szQRCode) != 0)
        {
            my_printf("E_YINGHUA_JIWEI_WIFI_REPORT_QR_RES   len: %d   %s\n", strlen(g_xSS.strQRContents), g_xSS.strQRContents);
            strcpy(szQRCode, g_xSS.strQRContents);

            int iStrLen = strlen(szQRCode);
            int iCmdLen = 6 + iStrLen;
            char* pSendCmd = (char*)my_malloc(iCmdLen);
            memset(pSendCmd, 0, iCmdLen);
            pSendCmd[0] = YINGHUA_JIWEI_WIFI_HEADER1;
            pSendCmd[1] = YINGHUA_JIWEI_WIFI_HEADER2;
            pSendCmd[2] = E_YINGHUA_JIWEI_WIFI_REPORT_QR_RES;
            pSendCmd[3] = iStrLen & 0xFF;
            pSendCmd[4] = (iStrLen >> 8) & 0xFF;
            memcpy(pSendCmd + 5, szQRCode, iStrLen);
            for(int i = 0; i < iCmdLen - 1; i++)
                pSendCmd[iCmdLen - 1] += pSendCmd[i];

            uart3Write(pSendCmd, iCmdLen);
            my_free(pSendCmd);
        }

        read_len = 0;
        memset(uart_cmd_buffer, 0, sizeof(uart_cmd_buffer));

        if (!m_iRunning)
            break;

        if (read_len < (int)sizeof(YINGHUA_JIWEI_WIFI_CMD))
        {
            if (g_xSS.iRecogQRMode == 1)
                read_len = uart3Read((char*)uart_cmd_buffer + read_len, sizeof(YINGHUA_JIWEI_WIFI_CMD) - read_len, 100);
            else
                read_len = uart3Read((char*)uart_cmd_buffer + read_len, sizeof(YINGHUA_JIWEI_WIFI_CMD) - read_len, 1);
        }
        if (read_len > 0)
        {
            read_len = 256;
            for (i = 0; i < read_len - 1; i++)
            {
                if (uart_cmd_buffer[i] == YINGHUA_JIWEI_WIFI_HEADER1 &&
                        uart_cmd_buffer[i+1] == YINGHUA_JIWEI_WIFI_HEADER2)
                {
                    break;
                }
            }
//            my_printf("[spithread] readlen=%d, i=%d\n", read_len, i);

            if (i < read_len - 1) //found uart header
            {
                memset(&xCmd, 0, sizeof(xCmd));
                copy_len = (read_len - i > (int)sizeof(xCmd)) ? ((int)sizeof(xCmd)) : (read_len - i);
                memcpy(&xCmd, uart_cmd_buffer + i, copy_len);
//                my_printf("[spithread] cmd=%02x, chksum=%02x, calccheck=%02x\n",
//                       xCmd.xYongHua_JiWei.bCmd, xCmd.xYongHua_JiWei.bCheckSum, GetYinghuaJiWeiCheckSum(xCmd));
                if (GetYinghuaJiWeiCheckSum(xCmd) == xCmd.xYongHua_JiWei.bCheckSum || 1)
                {
                    g_xSS.rLastSenseCmdTime = Now();
                    //parse command
                    if (xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_GET_CATEYE_STATUS)
                    {
                        my_printf("E_YINGHUA_JIWEI_WIFI_GET_CATEYE_STATUS_ACK\n");
                        sendUARTCmd(E_YINGHUA_JIWEI_WIFI_GET_CATEYE_STATUS_ACK,
                                    0x14, //g711, jpg, not rotate
                                    0x0a, //10 fps
                                    0, 0, 0, 0, 0, NULL);
                    }
                    else if(xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_NET_STATUS)
                    {
                        my_printf("[spithread] %0.3f, net status: %d\n", Now(), xCmd.xYongHua_JiWei.bData[0]);
                    }
                    else if(xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_MEDIA_REQ)
                    {
//                        my_printf("E_YINGHUA_JIWEI_WIFI_MEDIA_REQ\n");

                        // if(g_xSS.iVDBStart == 0)
                        //     StartVDB();

                        if (abJpegBuf == NULL)
                            continue;

                        if(g_xSS.iJiweiStart == 0)
                        {
                            g_xSS.iJiweiStart = 1;
                            StartAudioTask();

                            for(int i = 0; i < 20; i++)
                            {
                                if(JW_GetFrameFromVENC(abJpegBuf, &iJpegSize) == MI_SUCCESS)
                                    break;

                                my_usleep(100 * 1000);
                            }
                        }
                        else
                            JW_GetFrameFromVENC(abJpegBuf, &iJpegSize);

                        rSendPicTime = Now();

                        if(iJpegSize > 0)
                        {
                            my_mutex_lock(m_pushMutex);
                            char abTmp[MAX_AUDIO_LEN] = {0};
                            memcpy(abTmp, m_sendAudioBuf, m_iSendAudioLen);
                            int iLen = m_iSendAudioLen;
                            memset(m_sendAudioBuf, 0, sizeof(m_sendAudioBuf));
                            m_iSendAudioLen = 0;
                            my_mutex_unlock(m_pushMutex);

                            if(iLen >= MAX_AUDIO_LEN)
                                iLen = 0;

                            sendMedia(abJpegBuf, iJpegSize, (unsigned char*)abTmp, iLen);
                        }

                        // my_free(abJpegBuf);

//                        memset(m_sendAudioBuf, 0, sizeof(m_sendAudioBuf));
//                        m_iSendAudioLen = 0;

//                        my_printf("send media\n");
                    }
                    else if(xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_AUDIO_PUSH)
                    {
                        int iSpiReadLen = (xCmd.xYongHua_JiWei.bData[1] << 0) | (xCmd.xYongHua_JiWei.bData[2] << 8) | (xCmd.xYongHua_JiWei.bData[3] << 16) | (xCmd.xYongHua_JiWei.bData[4] << 24);
                        unsigned char* pbAudioBuf = (unsigned char*)my_malloc(iSpiReadLen);
                        memset(pbAudioBuf, 0, iSpiReadLen);
                        int iAudioLen = recvAudio(pbAudioBuf, iSpiReadLen);
                        if(iAudioLen > 0)
                        {
                            dbug_printf("################  read audio len %d\n", iAudioLen);
//                            FILE* fp = fopen("/tmp/audio.raw", "a+");
//                            if(fp)
//                            {
//                                fwrite(pbAudioBuf, iAudioLen, 1, fp);
//                                fflush(fp);
//                                fclose(fp);
//                            }
                            my_mutex_lock(m_popMutex);
                            memcpy(m_recvAudioBuf, pbAudioBuf, iAudioLen);
                            m_iRecvAudioLen = iAudioLen;
                            my_mutex_unlock(m_popMutex);
                        }
                        my_free(pbAudioBuf);
                    }
                    else if(xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_WIFI_MODULE_STATUS)
                    {
//                        my_printf("[spithread] %0.3f, module status: %s\n", Now(), xCmd.xYongHua_JiWei.bData[0] == 0 ? "idle" : "active");
                        sendUARTCmd(E_YINGHUA_JIWEI_WIFI_WIFI_MODULE_STATUS_ACK, 0, 0, 0, 0, 0, 0, 0, NULL);
                    }
                    else if(xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_SCAN_QR)
                    {
                        g_xSS.iRecogQRMode = xCmd.xYongHua_JiWei.bData[0];
                        g_xSS.iRecogQRTimeout = xCmd.xYongHua_JiWei.bData[1];
                        if(g_xSS.iRecogQRMode == 0)
                        {
                            rRecogQRStart = 0;
                            g_xSS.iRecogQRTimeout = 0;
                        }
                        else
                            rRecogQRStart = Now();

                        memset(g_xSS.strQRContents, 0, sizeof(g_xSS.strQRContents));
                        memset(szQRCode, 0, sizeof(szQRCode));

                        my_printf("E_YINGHUA_JIWEI_WIFI_SCAN_QR  Mode: %d,  Timeout: %d\n", g_xSS.iRecogQRMode, g_xSS.iRecogQRTimeout);
                        sendUARTCmd(E_YINGHUA_JIWEI_WIFI_SCAN_QR_ACK, 0, 0, 0, 0, 0, 0, 0, NULL);
                    }
                    else if(xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_PLAY_AUDIO)
                    {
                        int iAudioIdx = xCmd.xYongHua_JiWei.bData[0];
                        my_printf("E_YINGHUA_JIWEI_WIFI_PLAY_AUDIO  AudioIdx: %d\n", iAudioIdx);
                        sendUARTCmd(E_YINGHUA_JIWEI_WIFI_PLAY_AUDIO_ACK, 0, 0, 0, 0, 0, 0, 0, NULL);
                        StartWAVPlay(iAudioIdx);
                    }
                    else if(xCmd.xYongHua_JiWei.bCmd == E_YINGHUA_JIWEI_WIFI_REPORT_QR_RES_ACK)
                    {
                        my_printf("E_YINGHUA_JIWEI_WIFI_REPORT_QR_RES_ACK\n");
                    }
                    else
                        my_printf("[spithread] unkown cmd!\n");
                }
                else
                {
                    //checksum error
                    my_printf("[spithread]checksum error: cmd=%02x, sum=%02x\n",
                           xCmd.xYongHua_JiWei.bCmd, xCmd.xYongHua_JiWei.bCheckSum);
                }

            }
            else
            {
                //not found uart header
            }
        }
        else
        {
            //invalid data
        }
        my_usleep(10*1000);
    }
    g_xSS.iJiweiStart = 0;
    g_xSS.iRecogQRMode = 0;
    memset(g_xSS.strQRContents, 0, sizeof(g_xSS.strQRContents));

    my_free(abJpegBuf);
    StopAudioTask();
    spiClose();
    uart3Close();
}


/*  ---------------------   spi base  -----------------  */

#define SPI_PACKET_MAX_SIZE    1024

void MySpiThread::spiOpen()
{
#if 0
    int ret = 0;
    uint8_t mode = 0; /* SPI通信使用全双工，设置CPOL＝0，CPHA＝0。 */
    uint8_t bits = 8; /* ８ｂiｔｓ读写，MSB first。*/
    uint32_t speed = 40*1000*1000;/* 设置12M传输速度 */

    m_spiFd = open("/dev/spidev0.0", O_RDWR);
    if (m_spiFd < 0)
    {
        my_printf("failed to open spi dev");
        return;
    }

    ret = ioctl(m_spiFd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        my_printf("can't set spi mode\n");


    ret = ioctl(m_spiFd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        my_printf("can't get spi mode\n");


    /*
     * bits per word
     */
    ret = ioctl(m_spiFd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        my_printf("can't set bits per word\n");


    ret = ioctl(m_spiFd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        my_printf("can't get bits per word\n");


    /*
     * max speed hz
     */
    ret = ioctl(m_spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        my_printf("can't set max speed hz\n");


    ret = ioctl(m_spiFd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        my_printf("can't get max speed hz\n");


    my_printf("spi mode: %d\n", mode);
    my_printf("bits per word: %d\n", bits);
    my_printf("max speed: %d KHz (%d MHz)\n", speed / 1000, speed / 1000 / 1000);
#endif
}

void MySpiThread::spiClose()
{
    if (m_spiFd > -1)
    {
        // close(m_spiFd);
        m_spiFd = -1;
    }
}

int MySpiThread::spiWrite(char* buffer, int length)
{
    my_spi_write(buffer, length);
    return 0;
    if (m_spiFd < 0)
        return -1;
    int iRet = 0;
#if 0
    int cnt = length / SPI_PACKET_MAX_SIZE;
    int tail = length % SPI_PACKET_MAX_SIZE;
    for(int i = 0; i < cnt; i++)
    {
        int ret = write(m_spiFd, buffer + SPI_PACKET_MAX_SIZE * i, SPI_PACKET_MAX_SIZE);
        iRet += ret;
        if(ret != SPI_PACKET_MAX_SIZE)
        {
            my_printf("[SPI] spi write1 failed %d\n", ret);
            return iRet;
        }
    }

    if(tail > 0)
    {
        int ret = write(m_spiFd, buffer + SPI_PACKET_MAX_SIZE * cnt, tail);
        iRet += ret;
        if(ret != tail)
        {
            my_printf("[SPI] spi write2 failed %d\n", ret);
            return iRet;
        }
    }
#endif
    return iRet;
}

int MySpiThread::spiRead(unsigned char* buffer, int length)
{
    return my_spi_read(buffer, length);
}

static unsigned char* jpeg_temp_buffer = NULL;

int MySpiThread::sendMedia(unsigned char* pbPic, int iPicLen, unsigned char* pbAudio, int iAudioLen)
{
    int video_len = 0;
    int audio_len = iAudioLen;
    int off = 0;
    unsigned char media_info[8]={0,0,0,0,0,0,0,0};

    if (iPicLen > 0)
        video_len = iPicLen;
    else
        return 0;

    jpeg_temp_buffer = (unsigned char *)my_malloc(30*1024);
    if (jpeg_temp_buffer == NULL)
    {
        my_printf("@@@ jpeg_temp_buffer malloc fail\n");
        return 0;
    }

    jpeg_temp_buffer[off++] = YINGHUA_JIWEI_WIFI_HEADER1;
    jpeg_temp_buffer[off++] = YINGHUA_JIWEI_WIFI_HEADER2;
    jpeg_temp_buffer[off++] = E_YINGHUA_JIWEI_WIFI_MEDIA_REQ_ACK;
    jpeg_temp_buffer[off++] = video_len >> 0;
    jpeg_temp_buffer[off++] = video_len >> 8;
    jpeg_temp_buffer[off++] = video_len >> 16;
    jpeg_temp_buffer[off++] = video_len >> 24;
    jpeg_temp_buffer[off++] = audio_len >> 0;
    jpeg_temp_buffer[off++] = audio_len >> 8;
    jpeg_temp_buffer[off++] = audio_len >> 16;
    jpeg_temp_buffer[off++] = audio_len >> 24;
    memcpy(jpeg_temp_buffer + off, media_info, 8);
    off += 8;

    if(pbPic)
        memcpy(jpeg_temp_buffer + off, pbPic, video_len);

    off += video_len;

    if (pbAudio)
        memcpy(jpeg_temp_buffer + off, pbAudio, audio_len);
    off += audio_len;

    int checksum = 0;
    for (int i = 0; i < off; i++)
        checksum = checksum + jpeg_temp_buffer[i];
    jpeg_temp_buffer[off] = checksum & 0xFF;
//    GPIO_fast_setvalue(SPI_CS, 0);
    my_usleep(2*1000);
    spiWrite((char*)jpeg_temp_buffer, video_len + audio_len + 20);
//    GPIO_fast_setvalue(SPI_CS, 1);

    if (jpeg_temp_buffer != NULL)
        my_free(jpeg_temp_buffer);

//    my_printf("spi write: %d(%d+%d)\n", ret, video_len, audio_len);
    return 0;
}

int MySpiThread::recvAudio(unsigned char* pbBuf, int iLen)
{
    int i;
    int iAudioLen = 0;
    GPIO_fast_setvalue(SPI_CS, 0);
    int ret = spiRead(pbBuf, iLen);
    if(ret <= 0)
    {
        my_printf("[%s]  Read Audio Data is failed\n", __FUNCTION__);
        return -1;
    }

    for(i = 0; i < ret; i ++)
    {
        if(pbBuf[i] == YINGHUA_JIWEI_WIFI_HEADER1 &&
                pbBuf[i + 1] == YINGHUA_JIWEI_WIFI_HEADER2)
            break;
    }

    if(i < ret - 1)
    {
        unsigned char bCheckSum = 0;
        for(int j = i; j < ret - 1; j ++)
            bCheckSum += pbBuf[j];
        bCheckSum &= 0xFF;
        if(bCheckSum == pbBuf[ret - 1])
        {
            if(ret < i + 15)
            {
                my_printf("[%s]  Insufficient buf 1\n", __FUNCTION__);
                return -1;
            }

            iAudioLen = (pbBuf[i + 12] << 0) | (pbBuf[i + 13] << 8) | (pbBuf[i + 14] << 16) | (pbBuf[i + 15] << 24);
            if(ret < i + 15 + iAudioLen)
            {
                my_printf("[%s]  Insufficient buf 2\n", __FUNCTION__);
                return -1;
            }

            memcpy(pbBuf, pbBuf + i + 16, iAudioLen);
        }
        else
        {
            my_printf("[%s]  check sum error %x  %x\n", __FUNCTION__, bCheckSum, pbBuf[ret - 1]);
            return -1;
        }
    }
    else
    {
        my_printf("[%s]   not found jiwei header\n", __FUNCTION__);
        return -1;
    }

    GPIO_fast_setvalue(SPI_CS, 1);
    return iAudioLen;
}

int MySpiThread::sendUARTCmd(unsigned char bCmd, unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3
                          , unsigned char b4, unsigned char b5, int iTimeout, WIFI_CMD* pxRecvCmd)
{
    if(pxRecvCmd)
        memset(pxRecvCmd, 0, sizeof(WIFI_CMD));

    int iCmdLen = sizeof(YINGHUA_JIWEI_WIFI_CMD);

    WIFI_CMD xSendCmd;
    createUARTCmd(bCmd, b0, b1, b2, b3, b4, b5, &xSendCmd);

    uart3Write((char*)&xSendCmd, iCmdLen);
    return 1;
}

int MySpiThread::PopAudioBuf(char** buf)
{
    int iRet = 0;
    my_mutex_lock(m_popMutex);

    if(m_iRecvAudioLen > 0)
    {
        *buf = (char*)my_malloc(m_iRecvAudioLen);
        memcpy(*buf, m_recvAudioBuf, m_iRecvAudioLen);
        iRet = m_iRecvAudioLen;

        memset(m_recvAudioBuf, 0, sizeof(m_recvAudioBuf));
        m_iRecvAudioLen = 0;

        my_mutex_unlock(m_popMutex);
        return iRet;
    }

    my_mutex_unlock(m_popMutex);
    return iRet;
}

void MySpiThread::PushAudioBuf(const char* buf, const int len)
{
    my_mutex_lock(m_pushMutex);
    if(buf == NULL)
    {
        my_mutex_unlock(m_pushMutex);
        return;
    }

    if(m_iSendAudioLen + len > MAX_AUDIO_LEN)
    {
        memmove(m_sendAudioBuf, m_sendAudioBuf + len, MAX_AUDIO_LEN - len);
        m_iSendAudioLen -= len;
    }

    memcpy(m_sendAudioBuf + m_iSendAudioLen, buf, len);
    m_iSendAudioLen += len;
    my_mutex_unlock(m_pushMutex);
}

int MySpiThread::createUARTCmd(unsigned char bCmd, unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3
                          , unsigned char b4, unsigned char b5, WIFI_CMD* pxRetCmd)
{
    int iRet = 0;

    if(pxRetCmd)
        memset(pxRetCmd, 0, sizeof(WIFI_CMD));

    int iCmdLen = sizeof(YINGHUA_JIWEI_WIFI_CMD);

    WIFI_CMD xSendCmd;
    memset(&xSendCmd, 0, sizeof(xSendCmd));

    xSendCmd.xYongHua_JiWei.bHeader1 = YINGHUA_JIWEI_WIFI_HEADER1;
    xSendCmd.xYongHua_JiWei.bHeader2 = YINGHUA_JIWEI_WIFI_HEADER2;
    xSendCmd.xYongHua_JiWei.bCmd = bCmd;
    xSendCmd.xYongHua_JiWei.bData[0] = b0;
    xSendCmd.xYongHua_JiWei.bData[1] = b1;
    xSendCmd.xYongHua_JiWei.bData[2] = b2;
    xSendCmd.xYongHua_JiWei.bData[3] = b3;
    xSendCmd.xYongHua_JiWei.bData[4] = b4;
    xSendCmd.xYongHua_JiWei.bData[5] = b5;

    if(iCmdLen == sizeof(YINGHUA_JIWEI_WIFI_CMD))
        xSendCmd.xYongHua_JiWei.bCheckSum = GetYinghuaJiWeiCheckSum(xSendCmd);
    else
        xSendCmd.abData[iCmdLen - 1] = GetWifiCommonCheckSum(xSendCmd, iCmdLen - 1);

    if (pxRetCmd)
        memcpy(pxRetCmd, &xSendCmd, sizeof(xSendCmd));

    return iRet;
}

void MySpiThread::ThreadProc()
{
    run();
}

void* SpiThread_ThreadProc1(void* param)
{
    MySpiThread* pThread = (MySpiThread*)(param);
    pThread->ThreadProc();
    return NULL;
}

#endif // USE_WIFI_MODULE

#endif // USE_VDBTASK