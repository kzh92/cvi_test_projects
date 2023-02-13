#ifndef FACE_MOUDLE_TASK_H
#define FACE_MOUDLE_TASK_H

#include "facemoduledef.h"
#include "senselockmessage.h"
#include "mutex.h"
#include "thread.h"

//#include <pthread.h>

#define TO_SHORT(a, b)  (((a) << 8) | (b))
#define HIGH_BYTE(a)    (((a) >> 8) & 0xFF)
#define LOW_BYTE(a)     ((a) & 0xFF)

#define MAX_OTA_FSIZE   (20 * 1024 * 1024)
#define MAX_OTA_PCK_SIZE    (4000)

enum
{
    E_SEND_TYPE_NONE,
    E_SEND_TYPE_CMD,
    E_SEND_TYPE_DATA
};

class FaceModuleTask : public Thread
{
public:
    FaceModuleTask();
    ~FaceModuleTask();

    void    Init();
    void    Deinit();
    int     Start();
    int     Stop();

    int     SendCmd(int iType, int iP1, int iP2, int iP3);
    int     SendData(int iType, unsigned char* pbData, int iLen);
    void    ThreadProc();

    static void             SendAck(int iType, int iSeqNum, int iQ1, int iQ2, int iQ3);
    static void             SendData(int iType, int iSeqNum, int iAck, int iDataLen, unsigned char* pbData);
    static unsigned char    GetCheckSum(FM_CMD* pxCmd);

    static unsigned char    Get_SenseCheckSum(unsigned char* pbData, int iLen);
    static int              Get_MsgLen(s_msg* msg);
    static unsigned char    GenSeq(int iFlag, unsigned char bSeqNum);
protected:
    void    run();

    FM_CMD              CommCmd(FM_CMD xCmd);
    FM_CMD              CommData(FM_CMD xCmd, unsigned char* pbData, int iLen);
    FM_CMD              ParseCmd(unsigned char* pbData, int iLen);


    int                 RecvCmd(FM_CMD* pxCmd);
    int                 RecvData(unsigned char* pbData, int iLen);

protected:
    int                 m_iStep;
    int                 m_iEnd;

    static unsigned char    m_bSeqNum;

    int                 m_iComm;
    mymutex_ptr         m_xCommMutex;

    // pthread_mutex_t     m_mutex;
    // pthread_cond_t      m_cond;
    FM_CMD              m_xSendCmd;
    int                 m_iSendType;
    unsigned char*      m_pSendData;
    int                 m_iSendLen;
};

#endif // FACE_MOUDLE_PROC_H
