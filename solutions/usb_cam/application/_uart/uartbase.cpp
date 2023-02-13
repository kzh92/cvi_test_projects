
#include "uartbase.h"
#include "uartcomm.h"
#include "mutex.h"
#include "DBManager.h"
#include "settings.h"
#include "msg.h"

// #include <stdio.h>
// #include <memory.h>
// #include <unistd.h>
// #include <termios.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <getopt.h>
// #include <sys/ioctl.h>
// #include <linux/types.h>
// #include <sys/mman.h>
#include <string.h>


mymutex_ptr g_xUartMutex;

UARTReceiver::UARTReceiver()
{
    m_iRunning = 0;
}

UARTReceiver::~UARTReceiver()
{


}

void UARTReceiver::Start()
{
    m_iRunning = 1;
    Thread::Start();
}

void UARTReceiver::Stop()
{
    m_iRunning = 0;
    Thread::Wait();
}


void UARTReceiver::run()
{
    while(m_iRunning)
    {
        UART_MSG xMSG;
        xMSG.iLen = UART_Recv(xMSG.abData, sizeof(xMSG.abData));

        if(xMSG.iLen > 0 && m_iRunning)
        {
            UART_MSG* msg = (UART_MSG*)message_queue_message_alloc(&g_uart);
            memcpy(msg, &xMSG, sizeof(UART_MSG));

            message_queue_write(&g_uart, msg);
        }
        my_usleep(1000);
    }
}


UARTCallBack::UARTCallBack()
{

}

UARTCallBack::~UARTCallBack()
{

}

void UARTCallBack::CallBack(UART_MSG* pxComm)
{

}


UARTProc::UARTProc()
{
    Reset();
}


UARTProc::~UARTProc()
{
}

void UARTProc::SetCmdType(int iType)
{
    m_iCmdType = iType;
}

void UARTProc::SetData(unsigned char* pbData, int iLen)
{
    if(pbData == NULL || iLen == 0)
        return;

    memcpy(m_abData, pbData, iLen);
    m_iLen = iLen;
}

void UARTProc::SetStep(int iStep)
{
    m_iStep = iStep;
}

void UARTProc::SetEnd(int iEnd)
{
    m_iEnd = iEnd;
}

void UARTProc::Reset()
{
    m_iStep = 0;
    m_iEnd = 0;

    m_iCmdType = 0;
    m_iLen = 0;
    memset(m_abData, 0, sizeof(m_abData));

    // m_mutex = PTHREAD_MUTEX_INITIALIZER;
    // m_cond = PTHREAD_COND_INITIALIZER;
}


int UARTProc::GetCmdType()
{
    return m_iCmdType;
}

unsigned char* UARTProc::GetData()
{
    return m_abData;
}

int UARTProc::GetLen()
{
    return m_iLen;
}

int UARTProc::GetStep()
{
    return m_iStep;
}

int UARTProc::GetEnd()
{
    return m_iEnd;
}


int UARTProc::IsEnd()
{
    return (m_iStep == m_iEnd);
}

// int UARTProc::WaitTimeout(int iTimeout)
// {
//     struct timespec ts = {0, 0};
//     clock_gettime(CLOCK_REALTIME, &ts);
//     ts.tv_nsec += (iTimeout % 1000) * 1000 * 1000;
//     if (ts.tv_nsec >= 1000000000)
//     {
//         ts.tv_nsec -= 1000000000;
//         ts.tv_sec += 1;
//     }

//     ts.tv_sec += iTimeout / 1000;
//     // my_mutex_lock(&m_mutex);
//     // pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
//     // my_mutex_unlock(&m_mutex);

//     return 0;
// }

// int UARTProc::WaitCancel()
// {
//     my_mutex_lock(&m_mutex);
//     pthread_cond_signal(&m_cond);
//     my_mutex_unlock(&m_mutex);
//     return 0;
// }

int UARTProc::Start()
{
    return 0;
}

int UARTProc::Stop()
{
    return 0;
}


int UARTProc::Continue(UART_MSG* pxMSG)
{
    return 0;
}

// int UART_Baudrate(int iBaudRate)
// {
//     int iInBaudrate = B9600;
//     if(iBaudRate == Baud_Rate_19200)
//         iInBaudrate = B19200;
//     else if(iBaudRate == Baud_Rate_38400)
//         iInBaudrate = B38400;
//     else if(iBaudRate == Baud_Rate_57600)
//         iInBaudrate = B57600;
//     else if(iBaudRate == Baud_Rate_115200)
//         iInBaudrate = B115200;

//     return iInBaudrate;
// }
