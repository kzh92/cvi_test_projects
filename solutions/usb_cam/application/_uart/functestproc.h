#ifndef FUNC_TEST_PROC_H
#define FUNC_TEST_PROC_H

// #include "mainbackproc.h"
// #include "wifidef.h"
 #include "uartbase.h"

/////////////////////////FUNC TEST////////////////////////////////

#define PACKET_DATA_MAX     128
#define FUNC_TEST_HEADER    0xDDCC557E
#define FUNC_TEST_HEADER_PRE    0x7E

enum
{
    E_TEST_TYPE_M7 = 0,
    E_TEST_TYPE_M8 = 1,
    E_TEST_TYPE_S3 = 2,
    E_TEST_TYPE_S5_AUTO = 3,
    E_TEST_TYPE_S5_SEMI = 4,
    E_TEST_TYPE_S8 = 5,
    E_TEST_TYPE_S10 = 6,
    E_TEST_TYPE_FM = 7,
};

typedef enum
{
    E_SMT_SN 			= 0x5,
    E_FUNC_HUMEN		= 0x20,
    E_FUNC_FP			= 0x21,
    E_FUNC_MOTOR		= 0x22,
    E_FUNC_POWER		= 0x23,
    E_FUNC_AUDIO_W		= 0x24,
    E_FUNC_AUDIO_A		= 0x25,
    E_FUNC_CARD_W		= 0x26,
    E_FUNC_CARD_A		= 0x27,
    E_CURRENT_mA        = 0x28,
    E_CURRENT_uA        = 0x29,
    E_FUNC_GPIO         = 0x2A,
    E_FUNC_IRLED        = 0x2B,
    E_FUNC_KEYLED		= 0x2C,
    E_FUNC_BUZZER		= 0x2D,
    E_FUNC_VC0363		= 0x2E,
    E_CURRENT_mA_START	= 0x2F,
    E_FUNC_CAM_PORT     = 0x50,
    E_FUNC_MIC_SPEAKER  = 0x53,
    E_FUNC_UART0        = 0x54,

    E_REPORT_START		= 0x40,
    E_REPORT_TEST		= 0x41,
    E_REPORT_END        = 0x42,
    E_GET_REQ_KEY		= 0x43,
    E_SEND_REQ_KEY		= 0x44,
    E_GET_ACT_KEY		= 0x45,
    E_SEND_ACT_KEY		= 0x46,
} E_Func_CommandID;

#pragma pack(push, 1)

typedef struct _tagFUNC_TEST_UART_CMD
{
    unsigned int    header;
    unsigned char   cmdID;
    unsigned char   dataLen;
    unsigned char   chksum;
    unsigned char   dataBuf[PACKET_DATA_MAX];
} FUNC_TEST_UART_CMD;

#pragma pack(pop)

// class UARTTask;
class FuncTestProc : public UARTProc
{
public:
    FuncTestProc();
    ~FuncTestProc();

    void    SetActivated(int iActivated);
    int     GetResult() {return m_iResult;}

    int     Start();
    int     Stop();
    void    ThreadProc();

    static unsigned char CaclFuncTestCheckSum(FUNC_TEST_UART_CMD* pxCmd);
    static int CheckFuncTestCmd(FUNC_TEST_UART_CMD* pxCmd);

protected:
    void    run();
    int     RecvCmd(FUNC_TEST_UART_CMD* pxCmd);

    int     m_iActivated;
    int     m_iResult;
};

#endif // WIFI_PROC_H
