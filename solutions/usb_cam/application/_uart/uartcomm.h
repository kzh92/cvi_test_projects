#ifndef UART_COMM_H
#define UART_COMM_H

#include "appdef.h"

#define  B0	0000000		/* hang up */
#define  B50	0000001
#define  B75	0000002
#define  B110	0000003
#define  B134	0000004
#define  B150	0000005
#define  B200	0000006
#define  B300	0000007
#define  B600	0000010
#define  B1200	0000011
#define  B1800	0000012
#define  B2400	0000013
#define  B4800	0000014
#define  B9600	0000015
#define  B19200	0000016
#define  B38400	0000017
#define  B57600    0010001
#define  B115200   0010002
#define  B230400   0010003
#define  B460800   0010004
#define  B500000   0010005
#define  B576000   0010006
#define  B921600   0010007
#define  B1000000  0010010
#define  B1152000  0010011
#define  B1500000  0010012
#define  B2000000  0010013
#define  B2500000  0010014
#define  B3000000  0010015
#define  B3500000  0010016
#define  B4000000  0010017

#ifdef __cplusplus
extern	"C"
{
#endif

float   UART_Now(void);
int     UART_Init(void);
int     UART_Init2(void);
void    UART_Quit(void);
void    UART_Quit2(void);

int     UART_Send(unsigned char * pBuf, int nBufLen);
int     UART_Send2(unsigned char * pBuf, int nBufLen);
int     UART_Recv(unsigned char * pBuf, int nBufLen);
int     UART_Recv2(unsigned char * pBuf, int nBufLen);
int     UART_NDelay();

int     UART_Baudrate(int iBaudRate);
void    UART_SetBaudrate(int iBaudrate);
void    UART_SetBaudrate2(int iBaudrate);

int     UART_RecvDataForWait(unsigned char* pBuf, int iBufLen, int iTimeOut, int iInterval);
int     UART_RecvDataForWait2(unsigned char* pBuf, int iBufLen, int iTimeOut, int iInterval);

int     uart3Open();
void    uart3Close();
int     uart3Read(char *buffer, int length, int timeout);
int     uart3Write(char *buffer, int length);

#ifdef __cplusplus
}
#endif

#endif //_UART_H_
