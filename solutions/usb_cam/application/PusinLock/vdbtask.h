#ifndef VDB_TASK_H
#define VDB_TASK_H

#include "thread.h"
#include "mutex.h"
#include "wifidef.h"

#define MAX_AUDIO_LEN       4000

#define VDB_TASK_FINISHED 1

#define VDB_CAPTURED_IMAGE       0
#define VDB_CAMERA_ERROR         1
#define VDB_STARTED                2

#define VIDEO_PACKET_LEN            (4 * 1024)

enum {
    SPI_COMM_FRAME = 0x5A,
    SPI_COMM_END = 0x5B
};

//  6 0x00's and 0x5A, len(4byte), checksum(1byte), data

#if (USE_VDBTASK)
int ConvertYuvToSceneJpeg(unsigned char* pbSrc, int iYUYVMode);
int ConvertIRToSceneJpeg(unsigned char* pbSrc);
extern unsigned char*  g_abJpgData;
extern int             g_iJpgDataLen;

#if (USE_WIFI_MODULE)
class MySpiThread: public Thread
{
public:
    MySpiThread();
    void    Init();
    void    Start();
    void    Stop();
    int     PopAudioBuf(char** buf);
    void    PushAudioBuf(const char* buf, const int len);
    void    ThreadProc();
    void    Wait();
protected:
    void    run();
    void    spiOpen();
    void    spiClose();
    int     spiWrite(char *buffer, int length);
    int     spiRead(unsigned char* buffer, int length);
    int     sendMedia(unsigned char* pbPic, int iPicLen, unsigned char* pbAudio, int iAudioLen);
    int     recvAudio(unsigned char* pbBuf, int iLen);
    int     sendUARTCmd(unsigned char bCmd, unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3
                        , unsigned char b4, unsigned char b5, int iTimeout, WIFI_CMD* pxRecvCmd);
    int     createUARTCmd(unsigned char bCmd, unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3
                        , unsigned char b4, unsigned char b5, WIFI_CMD* pxRetCmd);
private:
    int     m_iRunning;
    int     m_spiFd;

    char    m_sendAudioBuf[MAX_AUDIO_LEN];
    int     m_iSendAudioLen;

    char    m_recvAudioBuf[MAX_AUDIO_LEN];
    int     m_iRecvAudioLen;

    mymutex_ptr   m_pushMutex;
    mymutex_ptr   m_popMutex;
};
#endif // USE_WIFI_MODULE

class VDBTask : public Thread
{
public:
    VDBTask();
    ~VDBTask();

    static int  CaptureCam;

    void    Start();
    void    Stop();
    void    Pause();

    int     GetCounter(){return m_iCounter;}

    void    SendFrameThroughSPI(int fd, unsigned char* buf, int len, unsigned char cmdType = SPI_COMM_FRAME);
    int     IsStreaming();
protected:
    void    run();

    static int  m_iCounter;

    int     m_iRunning;
};
#endif // USE_VDBTASK

#endif // WATCHTASK_H
