#ifndef UART_BASE_H
#define UART_BASE_H

#define PACKET_UNIT_SIZE    (128)

#include "thread.h"
#include "mutex.h"
#include <pthread.h>

enum
{
    CMD_TYPE_NONE = 0,
    CMD_TYPE_FACE_MODULE = 1
};

enum
{
    HEAD_TYPE_NONE = 0,
    HEAD_TYPE_FACE_MODULE = 1,
};

typedef struct _tagUART_MSG
{
    unsigned char   abData[128];
    int             iLen;
} UART_MSG;

class UARTReceiver : public Thread
{
public:
    UARTReceiver();
    ~UARTReceiver();

    void    Start();
    void    Stop();

protected:
    void    run();

    int     m_iRunning;
};


class UARTCallBack
{
public:
    UARTCallBack();
    ~UARTCallBack();

    void    CallBack(UART_MSG* pxComm);
};

class UARTProc : public Thread
{
public:
    UARTProc();
    ~UARTProc();

    void    SetCmdType(int iType);
    void    SetData(unsigned char* pbData, int iLen);
    void    SetStep(int iStep);
    void    SetEnd(int iStep);

    int             GetCmdType();
    unsigned char*  GetData();
    int             GetLen();
    int             GetStep();
    int             GetEnd();

    int             IsEnd();
    // int             WaitTimeout(int iTimeout);
    // int             WaitCancel();

    int Start();
    int Stop();
    int Continue(UART_MSG* pxMSG);
private:
    void            Reset();

protected:

    int             m_iCmdType;
    int             m_iStep;
    int             m_iEnd;

    unsigned char   m_abData[1024];
    int             m_iLen;

    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;

    // mymutex_ptr           m_xCommMutex;
};

extern mymutex_ptr g_xUartMutex;
// int UART_Baudrate(int iBaudRate);

#endif // WATCHTASK_H
