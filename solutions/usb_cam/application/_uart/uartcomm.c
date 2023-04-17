#include "uartcomm.h"
#include "appdef.h"
#include "common_types.h"
#include "settings.h"
#include <string.h>
#include "drv_gpio.h"
#include <drv/pin.h>
#include <pin.h>
#include "aos/hal/uart.h"

// #define UART_DEBUG_EN
uart_dev_t g_uart1;
int g_uart1_baud = 115200;
int g_uart1_inited = 0;

int UART_Init()
{
    if (g_uart1_inited)
        return 0;
    PINMUX_CONFIG(IIC0_SCL, UART1_TX);
    PINMUX_CONFIG(IIC0_SDA, UART1_RX);

    g_uart1.port                = 1;
    g_uart1.config.baud_rate    = g_uart1_baud;
    g_uart1.config.mode         = MODE_TX_RX;
    g_uart1.config.flow_control = FLOW_CONTROL_DISABLED;
    g_uart1.config.stop_bits    = STOP_BITS_1;
    g_uart1.config.parity       = NO_PARITY;
    g_uart1.config.data_width   = DATA_WIDTH_8BIT;

    int rc = hal_uart_init(&g_uart1);
    if (rc == 0)
    {
        dbug_printf("uart1 open ok\n");
        g_uart1_inited = 1;
        return 0;
    }
    else
    {
        printf("uart open fail\n");
        return 1;
    }
}

void UART_Quit(void)
{
    if (g_uart1_inited == 0)
        return;
    g_uart1_inited = 0;
    hal_uart_finalize(&g_uart1);
}

void UART_SetBaudrate(int iBaudrate)
{
    g_uart1_baud = iBaudrate;
    UART_Quit();
    UART_Init();
}

int UART_Send(unsigned char * pBuf, int nBufLen)
{
    if (g_uart1_inited == 0)
        return 0;
    if (hal_uart_send(&g_uart1, pBuf, nBufLen, 100))
        return 0;
    else
        return nBufLen;
}

int UART_Recv(unsigned char * pBuf, int nBufLen)
{
    if (g_uart1_inited == 0)
        return 0;
    int ret = hal_uart_recv(&g_uart1, pBuf, nBufLen, 0);
    if (ret == 0)
    {
#ifdef UART_DEBUG_EN
        my_printf("[%s] %d, ", __func__, nBufLen);
        for (int i = 0; i < nBufLen; i ++)
            my_printf("%02x ", pBuf[i]);
        my_printf("\n");
#endif // UART_DEBUG_EN
        return nBufLen;
    }
    else
        return 0;
}

int UART_RecvDataForWait(unsigned char* pBuf, int iBufLen, int iTimeOut, int iInterval)
{
    if (g_uart1_inited == 0)
        return 0;
    int ret = hal_uart_recv(&g_uart1, pBuf, iBufLen, iTimeOut);
    if (ret == 0)
    {
#ifdef UART_DEBUG_EN
        my_printf("[%s] %d, ", __func__, iBufLen);
        for (int i = 0; i < iBufLen; i ++)
            my_printf("%02x ", pBuf[i]);
        my_printf("\n");
#endif // UART_DEBUG_EN
        return iBufLen;
    }
    else
        return 0;
#if 0
    int iReceiveOff = 0;
    int iReceiveSize = 0;
    int iRet = 0;

    float rOldTime = Now();

    while(1)
    {
        iReceiveSize = 128;
        if (iReceiveSize > iBufLen -  iReceiveOff)
            iReceiveSize = iBufLen -  iReceiveOff;
        iRet = UART_Recv(pBuf + iReceiveOff, iReceiveSize);
        if(iRet > 0)
        {
            iReceiveOff += iRet;

            rOldTime = Now();
        }

        if(iReceiveOff == iBufLen)
            break;

        if(iReceiveOff > iBufLen) {
            dbug_printf("[uart] over data received\n\n");
            break;
        }

        if (iTimeOut > 0) {
            if(Now() - rOldTime >= iTimeOut)
            {
                return iReceiveOff;
            }
        }

        if (iInterval > 0)
            my_usleep(iInterval);
    }
#ifdef UART_DEBUG_EN
    my_printf("[%s] %d, ", __func__, iReceiveOff);
    for (int i = 0; i < iReceiveOff; i ++)
        my_printf("%02x ", pBuf[i]);
    my_printf("\n");
#endif // UART_DEBUG_EN

    return iReceiveOff;
#endif
}

int UART_Init2(void)
{
    return 0;
}

void UART_Quit2(void)
{
}

void UART_SetBaudrate2(int iBaudrate)
{
}

int UART_Send2(unsigned char * pBuf, int nBufLen)
{
    return 0;
}

int UART_NDelay()
{
    return 0;
}

int UART_Recv2(unsigned char * pBuf, int nBufLen)
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

int UART_Baudrate(int iBaudRate)
{
    int iInBaudrate = 115200;
    if(iBaudRate == Baud_Rate_230400)
        iInBaudrate = 230400;
    else if(iBaudRate == Baud_Rate_460800)
        iInBaudrate = 460800;
    else if(iBaudRate == Baud_Rate_1500000)
        iInBaudrate = 1500000;
    else if(iBaudRate == Baud_Rate_9600)
        iInBaudrate = 9600;
    else if(iBaudRate == Baud_Rate_19200)
        iInBaudrate = 19200;
    else if(iBaudRate == Baud_Rate_38400)
        iInBaudrate = 38400;
    else if(iBaudRate == Baud_Rate_57600)
        iInBaudrate = 57600;

    return iInBaudrate;
}