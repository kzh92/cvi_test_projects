
#include "functestproc.h"
#include "settings.h"
#include "DBManager.h"
#include "uartcomm.h"
#include "msg.h"
// #include "lcdtask.h"
#include "i2cbase.h"
#include "drv_gpio.h"
// #include "soundbase.h"
//#include "camera_api.h"
#include "shared.h"
#include "systembase.h"

#if (USE_WIFI_MODULE)
#include "audiotask.h"
#include "jiwei_base.h"
#include "soundbase.h"
#include "wifidef.h"
#endif // USE_WIFI_MODULE

// #include <pthread.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <memory.h>
// #include <termios.h>
// #include <my_malloc.h>
#include <string.h>

#include "KeyGenBase.h"

int ProcessActivation(char* pbUID, int iUniqueID);
int checkCamPort(unsigned char* pbResult);
void* funcTest_ThreadProc(void* param);
mythread_ptr       ft_thread = NULL;

static unsigned char CaclFuncTestCheckSum(FUNC_TEST_UART_CMD* pxCmd)
{
    int i = 0;
    unsigned char checkSum = 0;

    checkSum += pxCmd->cmdID;
    checkSum += pxCmd->dataLen;
    for (i = 0; i < pxCmd->dataLen; i ++)
        checkSum += pxCmd->dataBuf[i];

    return 0xFF - checkSum;
}

static int CheckFuncTestCmd(FUNC_TEST_UART_CMD* pxCmd)
{
    if(pxCmd == NULL)
        return 0;

    if(pxCmd->header != FUNC_TEST_HEADER)
    {
        my_printf("[FuncTest] Head Error! %x\n", pxCmd->header);
        return 0;
    }

    if(pxCmd->chksum != CaclFuncTestCheckSum(pxCmd))
    {
        my_printf("[FuncTest] CheckSum Error! %x, %x\n", pxCmd->chksum, CaclFuncTestCheckSum(pxCmd));
        return 0;
    }

    return 1;
}

FuncTestProc::FuncTestProc()
{
    m_iActivated = 0;
    m_iResult = 0;
}


FuncTestProc::~FuncTestProc()
{

}

void FuncTestProc::SetActivated(int iActivated)
{
    m_iActivated = iActivated;
}

int FuncTestProc::Start()
{
    my_printf("FuncTestProc::Start\n");
    m_iStep = 0;
    m_iEnd = 1;

//    Thread::Start();
    if(my_thread_create_ext(&ft_thread, NULL, funcTest_ThreadProc, this, (char*)"funcTest", 8192, 0))
        my_printf("[FuncTestProc]create thread error.\n");
    return 1;
}

int FuncTestProc::Stop()
{
    // Thread::Wait();
    if(ft_thread)
        my_thread_join(&ft_thread);
    return 1;
}

int FuncTestProc::RecvCmd(FUNC_TEST_UART_CMD* pxCmd)
{
    int iRet = 0;
    int iRecvHeader = 0;
    if(pxCmd == NULL)
        return 0;

    for(int i = 0; i < 1000; i ++)
    {
        int iFirstOK = 0;
        iRet = UART_Recv((unsigned char*)pxCmd, 1);
        if(iRet > 0)
        {
            if(*(unsigned char*)pxCmd == FUNC_TEST_HEADER_PRE)
                iFirstOK = 1;
        }

        if(iFirstOK == 1)
        {
            iRet = UART_Recv((unsigned char*)pxCmd + 1, 3);
            if(iRet >= 3)
            {
                if(pxCmd->header == FUNC_TEST_HEADER)
                {
                    iRecvHeader = 1;
                    break;
                }
            }
        }

        my_usleep(1000);
    }

    if(iRecvHeader == 0)
        return 0;

    iRet = UART_Recv((unsigned char*)pxCmd + 4, sizeof(FUNC_TEST_UART_CMD) - 4);
    if(iRet < 3)
        return 0;

    if(!CheckFuncTestCmd(pxCmd))
    {
        my_printf("[FuncTest] Check Failed! \n");
        return 0;
    }

    return 1;
}

void FuncTestProc::run()
{
    m_iResult = 0;

    int iBaudRate = Baud_Rate_115200;
    FUNC_TEST_UART_CMD xRecvCmd = { 0 };

    my_printf("iBaudRate = %d\n", iBaudRate);
    int iInBaudrate = (iBaudRate == Baud_Rate_9600) ? B9600 : B115200;
    UART_SetBaudrate(iInBaudrate);

    float rOld = 0;

//    unsigned char abCamPortRes[19] = {0};
//    checkCamPort(abCamPortRes);
//    for(int i = 0; i < 19; i++)
//        my_printf("%d,", abCamPortRes[i]);
//    my_printf("\n");

    while(m_iStep < m_iEnd)
    {
        if(rOld != 0 && Now() - rOld > 1000)
            break;

        memset(&xRecvCmd, 0, sizeof(FUNC_TEST_UART_CMD));
        if(!RecvCmd(&xRecvCmd))
            continue;

        GPIO_fast_setvalue(IR_LED, OFF);
        if(xRecvCmd.cmdID == E_SMT_SN)
        {
            my_printf("[FuncTest] E_SMT_SN\n");
            char szSerial[256] = { 0 };
            GetSerialNumber(szSerial);

            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = E_SMT_SN;
            xSendCmd.dataLen = 18;
            xSendCmd.dataBuf[0] = 1;
            xSendCmd.dataBuf[17] = E_TEST_TYPE_FM;
            memcpy(xSendCmd.dataBuf + 1, szSerial, strlen(szSerial));
            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            rOld = 0;
        }
        else if(xRecvCmd.cmdID == E_FUNC_CAM_PORT)
        {
            my_printf("[FuncTest] E_FUNC_CAM_PORT\n");
            unsigned char abCamPortRes[19];
            memset(abCamPortRes, 1, sizeof(abCamPortRes));
            int iCamSucc = checkCamPort(abCamPortRes);

            for(int i = 0; i < 19; i++)
                my_printf("%d,", abCamPortRes[i]);
            my_printf("\n");

            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = E_FUNC_CAM_PORT;
            xSendCmd.dataLen = 20;
            xSendCmd.dataBuf[0] = 1 - iCamSucc;
            memcpy(xSendCmd.dataBuf + 1, abCamPortRes, 19);
            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            GPIO_fast_config(143, OUT); // for CSI_D7(PE15)
            GPIO_fast_setvalue(143, 1);

            rOld = Now();
        }
        else if(xRecvCmd.cmdID == E_FUNC_IRLED)
        {
            my_printf("[FuncTest] E_FUNC_IRLED\n");
            GPIO_fast_setvalue(IR_LED, ON);

            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = E_FUNC_IRLED;
            xSendCmd.dataLen = 1;
            xSendCmd.dataBuf[0] = 0;    //success
            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            rOld = 0;
        }
#if (USE_WIFI_MODULE)
        else if(xRecvCmd.cmdID == E_FUNC_MIC_SPEAKER)
        {
            my_printf("[FuncTest] E_FUNC_SPEAKER\n");
            test_Audio();

            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = E_FUNC_MIC_SPEAKER;
            xSendCmd.dataLen = 1;
            xSendCmd.dataBuf[0] = 0;    //success
            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            //system("rm -f /db1/record.wav");

            rOld = 0;
        }
        else if(xRecvCmd.cmdID == E_FUNC_UART0)
        {
            my_printf("[FuncTest] E_FUNC_UART0\n");

            uart3Open();
            WIFI_CMD xWiFiCmd;
            unsigned char uart_cmd_buffer[256];
            memset(&xWiFiCmd, 0, sizeof(xWiFiCmd));
            xWiFiCmd.xYongHua_JiWei.bHeader1 = YINGHUA_JIWEI_WIFI_HEADER1;
            xWiFiCmd.xYongHua_JiWei.bHeader2 = YINGHUA_JIWEI_WIFI_HEADER2;
            xWiFiCmd.xYongHua_JiWei.bCmd = E_YINGHUA_JIWEI_WIFI_CE_STATUS;

            xWiFiCmd.xYongHua_JiWei.bCheckSum = (xWiFiCmd.xYongHua_JiWei.bHeader1 + xWiFiCmd.xYongHua_JiWei.bHeader2 + xWiFiCmd.xYongHua_JiWei.bCmd + xWiFiCmd.xYongHua_JiWei.bData[0] +
                    xWiFiCmd.xYongHua_JiWei.bData[1] + xWiFiCmd.xYongHua_JiWei.bData[2] + xWiFiCmd.xYongHua_JiWei.bData[3] + xWiFiCmd.xYongHua_JiWei.bData[4] +
                    xWiFiCmd.xYongHua_JiWei.bData[5]) & 0xFF;

            int iRecvAck = 0;
            for(int i = 0; i < 3; i ++)
            {
                uart3Write((char*)&xWiFiCmd, sizeof(YINGHUA_JIWEI_WIFI_CMD));
                for(int j = 0; j < 3; j ++)
                {
                    int read_len = uart3Read((char*)uart_cmd_buffer, sizeof(YINGHUA_JIWEI_WIFI_CMD), 100);
                    if(read_len > 2)
                    {
                        if (uart_cmd_buffer[0] == YINGHUA_JIWEI_WIFI_HEADER1 &&
                                uart_cmd_buffer[1] == YINGHUA_JIWEI_WIFI_HEADER2 &&
                                uart_cmd_buffer[2] == E_YINGHUA_JIWEI_WIFI_CE_STATUS_ACK)
                        {
                            iRecvAck = 1;
                            break;
                        }
                    }
                    my_usleep(50*1000);
                }

                if(iRecvAck == 1)
                    break;
            }

            uart3Close();
            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = E_FUNC_UART0;
            xSendCmd.dataLen = 1;
            xSendCmd.dataBuf[0] = iRecvAck == 1 ? 0 : 1;    //success
            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            rOld = 0;
        }
#endif
        else if(xRecvCmd.cmdID == E_GET_REQ_KEY)
        {
            my_printf("[FuncTest] E_GET_REQ_KEY\n");
            char szSerial[32] = { 0 };
            GetSerialNumber(szSerial);

            char szVersion[32] = { 0 };
            strcpy(szVersion, DEVICE_FIRMWARE_VERSION_INNER);

            // stm version is null
            strcat(szVersion, ",");

            char szUniquID[1024] = { 0 };
            GetUniquID(szUniquID);

            char szCode[1280] = { 0 };
            sprintf(szCode, "%s\n%s\n%s\n%s", szSerial, DEVICE_MODEL_NUM, szVersion, szUniquID);

            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = E_GET_REQ_KEY;
            xSendCmd.dataLen = strlen(szCode) + 1;
            xSendCmd.dataBuf[0] = (m_iActivated == 1 ? 2 : 0);
            memcpy(xSendCmd.dataBuf + 1, szCode, strlen(szCode));
            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            rOld = 0;
        }
        else if(xRecvCmd.cmdID == E_SEND_ACT_KEY)
        {
            my_printf("[test] E_SEND_ACT_KEY\n");

            char szLicense[256] = { 0 };
            memcpy(szLicense, xRecvCmd.dataBuf, xRecvCmd.dataLen);

            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = E_SEND_ACT_KEY;
            xSendCmd.dataLen = 1;

            if(m_iActivated == 0)
            {
                CustomSerialNumberInfo ci = { 0 };
                int iRet = _decodeFaceLicense(szLicense, ci);

                xSendCmd.dataBuf[0] = 1;
                if(iRet == ALL_RIGHT && ci.pHardwareID != NULL && ci.pUserData != NULL && ci.nUserDataLength == sizeof(int))
                {
//                    my_printf("Decode UID: %s\n", ci.pHardwareID);
                    iRet = ProcessActivation(ci.pHardwareID, *(int*)ci.pUserData);
                    if(iRet == 0)
                    {
                        xSendCmd.dataBuf[0] = 0;
                        m_iResult = 1;
                    }
                }
            }
            else
            {
                xSendCmd.dataBuf[0] = 0;
                m_iResult = 1;
            }

            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            break;
        }
        else
        {
            my_printf("[FuncTest] Send Failed Ack: Cmd=%x\n", xRecvCmd.cmdID);

            FUNC_TEST_UART_CMD xSendCmd = { 0 };
            xSendCmd.header = FUNC_TEST_HEADER;
            xSendCmd.cmdID = xRecvCmd.cmdID;
            xSendCmd.dataLen = 1;
            xSendCmd.dataBuf[0] = 1;
            xSendCmd.chksum = CaclFuncTestCheckSum(&xSendCmd);

            // my_mutex_lock(g_xUartMutex);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, ON);
#endif
            UART_Send((unsigned char*)&xSendCmd, xSendCmd.dataLen + 7);
#ifdef UART_EN
            GPIO_fast_setvalue(UART_EN, OFF);
#endif
            // my_mutex_unlock(g_xUartMutex);

            rOld = 0;
        }
    }
}

int checkCamPort(unsigned char* pbResult)
{
    if(pbResult == NULL)
        return 0;

    int iRet = 1;
#ifdef M24C64_WP
    GPIO_fast_setvalue(M24C64_WP, ON);
#endif

#if 0
    // for TWI0_SCK(PB06), TWI0_SDA(PB07)
    int reg_val = Get_addr_value(0x24);
    reg_val = (reg_val & ~(7<<28)) | (1 << 28); // to GPIO
    Set_addr_value(0x24, reg_val);

    reg_val = Get_addr_value(0x24);
    reg_val = (reg_val & ~(7<<24)) | (1 << 24); // to GPIO
    Set_addr_value(0x24, reg_val);
#endif
    unsigned char abPort[9][2] =
    {
        {129, 148}, // CSI_MCLK(PE01) - MIPI_MCLK(PE20)
        {142, 150}, // CSI_D6(PE14) - CSI_SDA(PE22)
        {141, 149}, // CSI_D5(PE13) - CSI_SCK(PE21)
        {128, 195}, // CSI_PCLK(PE00) - MIPI_PWDN(PG03)
        {140, 38}, // CSI_D4(PE12) - TWI0_SCK(PB06)
        {136, 39}, // CSI_D0(PE08) - TWI0_SDA(PB07)
        {139, 131}, // CSI_D3(PE11) - CSI_VSYNC(PE03)
        {137, 196}, // CSI_D1(PE09) - CSI_PWDN(PG04)
        {138, 130} // CSI_D2(PE10) - CSI_HSYNC(PE02)
    };
#if 1
    int iPairIdx = 0;
    for(iPairIdx = 0; iPairIdx < 9; iPairIdx++)
    {
        GPIO_fast_config(abPort[iPairIdx][0], OUT);
        GPIO_fast_config(abPort[iPairIdx][1], IN);

        for(int i = 0; i < 4; i++)
        {
            int iSetVal = i % 2;
            GPIO_fast_setvalue(abPort[iPairIdx][0], iSetVal);
            my_usleep(1000);
            int iGetVal = GPIO_fast_getvalue(abPort[iPairIdx][1]) == 0 ? 0 : 1;
            if(iSetVal != iGetVal)
            {
                iRet = 0;
                pbResult[iPairIdx * 2] = 0;
                pbResult[iPairIdx * 2 + 1] = 0;
                my_printf("[%s]  Pair Error   %d-%d\n", __FUNCTION__, abPort[iPairIdx][0], abPort[iPairIdx][1]);
                GPIO_fast_setvalue(abPort[iPairIdx][0], 0);
                break;
            }
        }
    }

    for(int i = 0; i < 19; i++)
    {
        for(int j = 0; j < 18; j++)
        {
            if(j == i)
                continue;
            GPIO_fast_config(*((unsigned char*)abPort + j), IN);
        }

        if(i == 18)
            GPIO_fast_config(143, OUT); // for CSI_D7(PE15)
        else
            GPIO_fast_config(*((unsigned char*)abPort + i), OUT);

        if(i == 18)
            GPIO_fast_setvalue(143, 0);
        else
            GPIO_fast_setvalue(*((unsigned char*)abPort + i), 0);

        my_printf("[%s] SEND: %d: ", __FUNCTION__, i);
        for(int j = 0; j < 18; j++)
        {
            if(j == i)
            {
                my_printf("_,");
                continue;
            }

            int iGetVal = GPIO_fast_getvalue(*((unsigned char*)abPort + j)) == 0 ? 0 : 1;
            my_printf("%d,", iGetVal);

            if(iGetVal == 0 && ((j - i) % 2 == 0 || i == 18))
            {
                iRet = 0;
                pbResult[j] = 0;
                pbResult[i] = 0;
            }
        }
        my_printf("\n");

        GPIO_fast_setvalue(*((unsigned char*)abPort + i), 1);
    }
#else
    int iPairIdx = 0;
    for(iPairIdx = 0; iPairIdx < 9; iPairIdx++)
    {
        GPIO_fast_config(abPort[iPairIdx][0], OUT);
        GPIO_fast_config(abPort[iPairIdx][1], IN);

        for(int i = 0; i < 3; i++)
        {
            int iSetVal = i % 2;
            GPIO_fast_setvalue(abPort[iPairIdx][0], iSetVal);
            my_usleep(1000);
            int iGetVal = GPIO_fast_getvalue(abPort[iPairIdx][1]) == 0 ? 0 : 1;
            if(iSetVal != iGetVal)
            {
                pbResult[iPairIdx * 2] = 0;
                pbResult[iPairIdx * 2 + 1] = 0;
                my_printf("[%s]  Pair Error   %d-%d\n", __FUNCTION__, abPort[iPairIdx][0], abPort[iPairIdx][1]);
                GPIO_fast_setvalue(abPort[iPairIdx][0], 0);
                break;
            }

            if(i == 2)
            {
                pbResult[iPairIdx * 2] = 1;
                pbResult[iPairIdx * 2 + 1] = 1;
            }
        }
    }

    pbResult[18] = 1; // CSI_D4(PE12)

    for(int i = 0; i < 1/*2*/; i++)
    {
        if(i == 1) // reverse
        {
            for(iPairIdx = 0; iPairIdx < 9; iPairIdx++)
            {
                GPIO_fast_config(abPort[iPairIdx][0], IN);
                GPIO_fast_config(abPort[iPairIdx][1], OUT);
                GPIO_fast_setvalue(abPort[iPairIdx][1], 0);
            }
        }

        for(iPairIdx = 0; iPairIdx < 9; iPairIdx++)
        {
            GPIO_fast_setvalue(abPort[iPairIdx][i], 1);
            my_usleep(1000);
            for(int j = 0; j < 9; j++)
            {
                if(j == iPairIdx)
                    continue;

                int iGetVal = GPIO_fast_getvalue(abPort[j][1 - i]) == 0 ? 0 : 1;
                if(iGetVal == 1)
                {
                    my_printf("[%s]  Link Error   %d-(%d)\n", __FUNCTION__, abPort[iPairIdx][i], abPort[j][i]);
                    pbResult[j * 2 + i] = 0;
                    pbResult[j * 2 + 1 - i] = 0;
                }
            }
            GPIO_fast_setvalue(abPort[iPairIdx][i], 0);
        }

        if(i == 0) // test CSI_D4(PE12)
        {
            GPIO_fast_config(140, OUT); // CSI_D4(PE12)
            GPIO_fast_setvalue(140, 1);
            my_usleep(1000);
            for(int j = 0; j < 9; j++)
            {
                int iGetVal = GPIO_fast_getvalue(abPort[j][1 - i]) == 0 ? 0 : 1;
                if(iGetVal == 1)
                {
                    my_printf("[%s]  Link Error   140-(%d)\n", __FUNCTION__, abPort[j][i]);
                    pbResult[j * 2 + i] = 0;
                    pbResult[j * 2 + 1 - i] = 0;
//                    pbResult[18] = 0;
                }
            }
            GPIO_fast_setvalue(140, 0);
        }
    }
#endif
#if 0
    reg_val = Get_addr_value(0x24);
    reg_val = (reg_val & ~(7<<28)) | (2 << 28); // to TWI0
    Set_addr_value(0x24, reg_val);

    reg_val = Get_addr_value(0x24);
    reg_val = (reg_val & ~(7<<24)) | (2 << 24); // to TWI0
    Set_addr_value(0x24, reg_val);
#endif

#ifdef M24C64_WP
    GPIO_fast_setvalue(M24C64_WP, OFF);
#endif

    return iRet;
}

void FuncTestProc::ThreadProc()
{
    run();
}

void* funcTest_ThreadProc(void* param)
{
    FuncTestProc* pThread = (FuncTestProc*)(param);
    pThread->ThreadProc();
    return NULL;
}