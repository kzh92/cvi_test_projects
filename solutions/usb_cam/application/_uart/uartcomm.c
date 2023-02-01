#include "uartcomm.h"
#include "appdef.h"
//#include "drv_uart_api.h"
#include "common_types.h"

#include <string.h>

#include "drv_gpio.h"

int UART_Init()
{
    return 0;
}

int UART_Init2(void)
{
    return 0;
}

void UART_Quit(void)
{
}

void UART_Quit2(void)
{
}

void UART_SetBaudrate(int iBaudrate)
{
}

void UART_SetBaudrate2(int iBaudrate)
{
}

int UART_Send(unsigned char * pBuf, int nBufLen)
{
    return 0;
}

int UART_Send2(unsigned char * pBuf, int nBufLen)
{
    return 0;
}

int UART_NDelay()
{
    return 0;
}

int UART_Recv(unsigned char * pBuf, int nBufLen)
{
    return 0;
}

int UART_Recv2(unsigned char * pBuf, int nBufLen)
{
    return 0;
}

int UART_RecvDataForWait(unsigned char* pBuf, int iBufLen, int iTimeOut, int iInterval)
{
    return 0;
}

int UART_RecvDataForWait2(unsigned char* pBuf, int iBufLen, int iTimeOut, int iInterval)
{
    return 0;
}

int uart3Open()
{
    UART_Init2();
    return 0;
}

void uart3Close()
{
    UART_Quit2();
}

int uart3Read(char *buffer, int length, int timeout)
{
    return UART_RecvDataForWait2((unsigned char*)buffer, length, timeout, 0);
}

int uart3Write(char *buffer, int length)
{
    return UART_Send2((unsigned char*)buffer, length);
}