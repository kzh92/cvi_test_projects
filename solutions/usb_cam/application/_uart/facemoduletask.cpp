
#include "facemoduletask.h"
#include "settings.h"
#include "DBManager.h"
#include "appdef.h"
#include "uartcomm.h"
#include "msg.h"
//#include "lcdtask.h"
#include "i2cbase.h"
#include "faceengine.h"
//#include "fptask.h"
#include "mount_fs.h"
#include "shared.h"
#include "sha1.h"
//#include "facemoduleproc.h"
#include "facemoduledef.h"
#include "EngineDef.h"
#include "systembase.h"
#include "drv_gpio.h"
#include "FaceRetrievalSystem.h"
#include "senselockmessage.h"
#include "upgradebase.h"
#include "senselocktask.h"

// #include <memory.h>
// #include <stdio.h>
// #include <termios.h>
// #include <unistd.h>
// #include <my_malloc.h>
// #include <stdlib.h>
#include <string.h>

void* faceModuleTask_ThreadProc1(void* param);
extern int _get_activation_mark();

unsigned char FaceModuleTask::m_bSeqNum = 0;

FaceModuleTask::FaceModuleTask()
{
    // m_mutex = PTHREAD_MUTEX_INITIALIZER;
    // m_cond = PTHREAD_COND_INITIALIZER;
    m_thread = NULL;
}

FaceModuleTask::~FaceModuleTask()
{
}

void FaceModuleTask::Init()
{
    m_thread = NULL;
    m_xCommMutex = my_mutex_init();
}

void FaceModuleTask::Deinit()
{
    my_mutex_destroy(m_xCommMutex);
    m_xCommMutex = NULL;
}

int FaceModuleTask::Start()
{
    if (m_thread == NULL)
    {
        m_iStep = 0;
        m_iEnd = 1;
        m_iSendType = 0;
        m_iComm = 0;

        if(my_thread_create_ext(&m_thread, NULL, faceModuleTask_ThreadProc1, this, (char*)"factk", 16384, 0))
            my_printf("[FMTask]create thread error.\n");
    }
    return 1;
}

int FaceModuleTask::Stop()
{
    m_iStep = m_iEnd;
    return 1;
}

int FaceModuleTask::SendCmd(int iType, int iP1, int iP2, int iP3)
{
    my_mutex_lock(m_xCommMutex);

    memset(&m_xSendCmd, 0, sizeof(FM_CMD));

    m_xSendCmd.bP1 = iP1;
    m_xSendCmd.bP2 = iP2;
    m_xSendCmd.bP3 = iP3;
    m_xSendCmd.bType = iType;

    m_iComm = 0;
    m_iSendType = E_SEND_TYPE_CMD;
    my_mutex_unlock(m_xCommMutex);

    return 0;
}

int FaceModuleTask::SendData(int iType, unsigned char* pbData, int iLen)
{
    my_mutex_lock(m_xCommMutex);

    memset(&m_xSendCmd, 0, sizeof(FM_CMD));

    m_xSendCmd.bP1 = HIGH_BYTE(iLen);
    m_xSendCmd.bP2 = LOW_BYTE(iLen);
    m_xSendCmd.bP3 = 0;
    m_xSendCmd.bType = iType;

    m_pSendData = pbData;
    m_iSendLen = iLen;

    m_iComm = 0;
    m_iSendType = E_SEND_TYPE_DATA;
    my_mutex_unlock(m_xCommMutex);

    return 0;
}

int FaceModuleTask::RecvCmd(FM_CMD* pxCmd)
{
    int iRet = 0;
    if(pxCmd == NULL)
        return 0;

    my_mutex_lock(m_xCommMutex);
    m_iComm = 1;
    my_mutex_unlock(m_xCommMutex);

    for(int i = 0; i < 1000; i ++)
    {
        if(m_iStep == m_iEnd)
            return 0;

        iRet = UART_Recv((unsigned char*)pxCmd, 1);
        if(iRet > 0)
        {
            if(pxCmd->bHeader == FM_HEADER)
            {
                iRet = UART_RecvDataForWait((unsigned char*)pxCmd + 1, sizeof(FM_CMD) - 1, 200, 10);
                if(iRet != sizeof(FM_CMD) - 1)
                    return 0;

                if(pxCmd->bChk != GetCheckSum(pxCmd))
                {
                    my_printf("[FM] RecvCmd CheckSum Error: %x, %x\n", pxCmd->bChk, GetCheckSum(pxCmd));
                    return 0;
                }
                return 1;
            }
        }

//        my_usleep(1000);
    }

    return 0;
}

int FaceModuleTask::RecvData(unsigned char* pbData, int iLen)
{
    int iRet = 0;
    if(pbData == NULL || iLen <= 0)
        return 0;

    my_mutex_lock(m_xCommMutex);
    m_iComm = 1;
    my_mutex_unlock(m_xCommMutex);

    //for(int i = 0; i < 1000; i ++)
    {
        //if(m_iComm == 0)
        //    return 0;

        iRet = UART_RecvDataForWait((unsigned char*)pbData, 1, 1000, 0);
        if(iRet > 0)
        {
            if(pbData[0] == FM_HEADER)
            {
                iRet = UART_RecvDataForWait((unsigned char*)pbData, iLen + 1, 1000, 0);
                dbug_printf("[%s] iRet=%d\n", __func__, iRet);
                if(iRet < iLen)
                    return 0;

                unsigned char bSrcChk = pbData[iLen];
                // iRet = UART_Recv((unsigned char*)&bSrcChk, 1);
                // dbug_printf("[%s] iRet2=%d\n", __func__, iRet);
                // if(iRet != 1)
                //     return 0;

                unsigned char bChk = 0;
                for(int i = 0; i < iLen; i ++)
                    bChk = bChk ^ pbData[i];

                if(bChk != bSrcChk)
                {
                    my_printf("[FM] RecvData CheckSum Error: %x, %x\n", bChk, bSrcChk);
                    return 0;
                }
                return 1;
            }
        }

        // my_usleep(1000);
    }

    return 0;
}


void FaceModuleTask::run()
{
    dbug_printf("FaceModuleTask::run start\n");
    while(m_iStep < m_iEnd)
    {
        my_mutex_lock(m_xCommMutex);
        if(m_iSendType == E_SEND_TYPE_CMD)
        {
            CommCmd(m_xSendCmd);
            m_iSendType = 0;
            my_mutex_unlock(m_xCommMutex);
            continue;
        }
        else if(m_iSendType == E_SEND_TYPE_DATA)
        {
            CommData(m_xSendCmd, m_pSendData, m_iSendLen);
            m_iSendType = 0;
            m_pSendData = NULL;
            m_iSendLen = 0;
            my_mutex_unlock(m_xCommMutex);
            continue;
        }
        my_mutex_unlock(m_xCommMutex);

        FM_CMD xRecvCmd;
        memset(&xRecvCmd, 0, sizeof(xRecvCmd));
        int iRet = RecvCmd(&xRecvCmd);
        if(iRet == 0)
            continue;

        if(xRecvCmd.bSeqNum & 0x80)
        {
            my_printf("[FM] RecvCmd Not Ack: %x\n", xRecvCmd.bSeqNum);
            continue;
        }

        if(xRecvCmd.bType == FM_CMD_DEV_TEST_START)
        {
            SendGlobalMsg(MSG_FM, xRecvCmd.bType, xRecvCmd.bP3, 0);
            SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
        }
        else if(g_xSS.iNoActivated == 1)
        {
            if (xRecvCmd.bType == FM_CMD_GET_HWID)
            {
                char szSerial[128] = { 0 };
                GetSerialNumber(szSerial);

                char szVersion[128] = { 0 };
                strcpy(szVersion, DEVICE_FIRMWARE_VERSION_INNER);

                // stm version is null
                strcat(szVersion, ",");

                char szUniquID[128] = { 0 };
                GetUniquID(szUniquID);

                char szCode[1024] = { 0 };
                snprintf(szCode, 1023, "%s\n%s\n%s\n%s", szSerial, DEVICE_MODEL_NUM, szVersion, szUniquID);
                dbug_printf("HWID:%s\n", szCode);

                int iDataLen = strlen(szCode);
                SendData(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), FM_ACK_SUCCESS, iDataLen, (unsigned char*)szCode);
            }
            else if (xRecvCmd.bType == FM_CMD_ACTIVATE)
            {
                dbug_printf("======================activate!\n");
                int iLen = TO_SHORT(xRecvCmd.bP1, xRecvCmd.bP2);
                dbug_printf("act packlen=%d\n", iLen);
                if(iLen > 0)
                {
                    unsigned char* pbActData = (unsigned char*)my_malloc(iLen + 1);
                    memset(pbActData, 0, iLen + 1);

                    if(RecvData(pbActData, iLen) == 1)
                    {
                        dbug_printf("Recv Activation Data!\n");
                        if(_get_activation_mark())
                        {
                            my_printf("*** mark detected2\n");
                            SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                        }
                        else
                        {
                            SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);

                            SendGlobalMsg(MSG_FM, xRecvCmd.bType, (long)pbActData, 0);
                        }
                    }
                    else
                    {
                        dbug_printf("Recv Activation Data Failed!\n");
                        SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                        my_free(pbActData);
                    }
                }
                else
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
            }
            else
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_NO_ACTIVATED);
            }

            continue;
        }
        else
        {
            if(xRecvCmd.bType == FM_CMD_START_OTA)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, 0, 0);
            }
            else if(xRecvCmd.bType == FM_CMD_REGISTER_USER)
            {
                int iUserID = TO_SHORT(xRecvCmd.bP1, xRecvCmd.bP2);
                int iUserRole = xRecvCmd.bP3;
                int iAckFull = 0;

                if(iUserID == 0)
                {
                    if(dbm_GetNewUserID() == -1)
                        iAckFull = 1;

                    iUserID = dbm_GetNewUserID() + 1;
                }

                if(iAckFull == 1)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FULL);
                }
                else if(iUserID < 0 || iUserID > N_MAX_PERSON_NUM)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ID_INVALID);
                }
                else if(iUserRole < EUSER_ROLE_MIN || iUserRole > EUSER_ROLE_MAX)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ROLE_INVALID);
                }
//                else if(dbm_GetUserCount() >= N_MAX_PERSON_NUM)
//                {
//                    my_printf("SendAck: FM_ACK_FULL\n");
//                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FULL);
//                }
                else
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                    SendGlobalMsg(MSG_FM, xRecvCmd.bType, iUserID, iUserRole);
                }
            }
            else if(xRecvCmd.bType == FM_CMD_IDENTIFY)
            {
                int iUserRole = xRecvCmd.bP3;
                if(iUserRole < 0 || iUserRole > EUSER_ROLE_MAX)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ROLE_INVALID);
                }
                else
                {
                    SendGlobalMsg(MSG_FM, xRecvCmd.bType, xRecvCmd.bP3, 0);
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                }
            }
            else if(xRecvCmd.bType == FM_CMD_VERIFY)
            {
                int iUserID = TO_SHORT(xRecvCmd.bP1, xRecvCmd.bP2);
                PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iUserID - 1);
                if(pxMetaInfo == NULL)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_NO_USER);
                }
                else
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                    SendGlobalMsg(MSG_FM, xRecvCmd.bType, iUserID, 0);
                }
            }
#if (NO_ENCRYPT_FRM3_4 == 0)
            else if(xRecvCmd.bType == FM_CMD_START_VDB)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, 0, 0);
            }
            else if(xRecvCmd.bType == FM_CMD_STOP_VDB)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, 0, 0);
            }
#endif // NO_ENCRYPT_FRM3_4 == 0
            else if(xRecvCmd.bType == FM_CMD_STOP_PROCESS)
            {
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, xRecvCmd.bP3, 0);
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
            }
            else if(xRecvCmd.bType == FM_CMD_DEL_USER)
            {
                int iUserID = TO_SHORT(xRecvCmd.bP1, xRecvCmd.bP2);
                PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iUserID - 1);
                if(pxMetaInfo == NULL)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_NO_USER);
                }
                else
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                    SendGlobalMsg(MSG_FM, xRecvCmd.bType, iUserID, 0);
                }
            }
            else if(xRecvCmd.bType == FM_CMD_DEL_ALL_USER)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, xRecvCmd.bP1, 0);
            }
            else if(xRecvCmd.bType == FM_CMD_FACTORY_RESET)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, 0, 0);
            }
            else if(xRecvCmd.bType == FM_CMD_GET_USER_COUNT)
            {
                int iUserRole = xRecvCmd.bP3;
                if(iUserRole < 0 || iUserRole > EUSER_ROLE_MAX)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ROLE_INVALID);
                }
                else
                {
                    if(iUserRole < EUSER_ROLE1)
                        iUserRole = -1;

                    int iUserCount = dbm_GetUserCount(iUserRole);
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), HIGH_BYTE(iUserCount), LOW_BYTE(iUserCount), FM_ACK_SUCCESS);
                }
            }
            else if(xRecvCmd.bType == FM_CMD_GET_USER_ROLE)
            {
                int iUserID = TO_SHORT(xRecvCmd.bP1, xRecvCmd.bP2);
                PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iUserID - 1);
                my_printf("GetUserRole: ID=%d, %p\n", iUserID, pxMetaInfo);
                if(pxMetaInfo == NULL)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_NO_USER);
                }
                else
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), pxMetaInfo->fPrivilege, 0, FM_ACK_SUCCESS);
                }
            }
            else if(xRecvCmd.bType == FM_CMD_GET_NEW_ID)
            {
                int iUserID = dbm_GetNewUserID() + 1;
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), HIGH_BYTE(iUserID), LOW_BYTE(iUserID), FM_ACK_SUCCESS);
            }
            else if(xRecvCmd.bType == FM_CMD_GET_IMG)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, xRecvCmd.bP3, 0);
            }
            else if(xRecvCmd.bType == FM_CMD_GET_INFO)
            {
                char szSN[256] = {0};
                GetSerialNumber(szSN);

                char szMsg[1024] = { 0 };
                sprintf(szMsg, "Version:%s\r\n"
                               "Model:%s\r\n"
                               "SN:%s\r\n"
                               "MaxUser:%d\r\n",
                        DEVICE_FIRMWARE_VERSION_INNER, DEVICE_MODEL_NUM, szSN, N_MAX_PERSON_NUM);

                int iDataLen = strlen(szMsg);
                SendData(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), FM_ACK_SUCCESS, iDataLen, (unsigned char*)szMsg);
            }
#if 0
            else if(xRecvCmd.bType == FM_CMD_GET_USER_FEATS)
            {
                int iUserID = TO_SHORT(xRecvCmd.bP1, xRecvCmd.bP2);
                PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iUserID - 1);
                PSFeatInfo pxFeatInfo = dbm_GetPersonFeatInfoByID(iUserID - 1);

                LOG_PRINT("GetFeats: ID=%d, %x\n", iUserID, pxMetaInfo);
                if(iUserID < 0 || iUserID > N_MAX_PERSON_NUM)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ID_INVALID);
                }
                else if(pxMetaInfo == NULL || pxFeatInfo == NULL)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_NO_USER);
                }
                else
                {
                    int iFeatLen = (sizeof(KDNN_FEAT_SIZE) * KDNN_FEAT_SIZE * 3) + (UNIT_ENROLL_FEATURE_SIZE) + 4 + 32;     //4: header len, 32: reserved
                    unsigned char* pbFeats = (unsigned char*)my_malloc(iFeatLen);
                    memset(pbFeats, 0, iFeatLen);

                    int iOff = 0;
                    memcpy(pbFeats + iOff, FEAT_VER, 4);
                    iOff += 4;

                    memcpy(pbFeats + iOff, pxFeatInfo->arDNNFeatArray, sizeof(float) * KDNN_FEAT_SIZE * 3);
                    iOff += sizeof(float) * KDNN_FEAT_SIZE * 3;

//                    memcpy(pbFeats + iOff, pxFeatInfo->abFeatArray, UNIT_ENROLL_FEATURE_SIZE);
//                    iOff += UNIT_ENROLL_FEATURE_SIZE;

                    int iCheckSum = 0;
                    for(int i = 0; i < (iFeatLen - 4) / 4; i += 4)
                        iCheckSum = iCheckSum ^  *(int*)(pbFeats + i);

                    memcpy(pbFeats +  iFeatLen - 4, &iCheckSum, sizeof(iCheckSum));

                    SendData(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), FM_ACK_SUCCESS, iFeatLen, pbFeats);

                    my_free(pbFeats);
                }
            }
            else if(xRecvCmd.bType == FM_CMD_SET_USER_FEATS)
            {
                int iLen = TO_SHORT(xRecvCmd.bP1, xRecvCmd.bP2);
                if(iLen > 0)
                {
                    unsigned char* pbFeatData = (unsigned char*)my_malloc(iLen + 1);
                    memset(pbFeatData, 0, iLen + 1);

                    if(RecvData(pbFeatData, iLen) == 1)
                    {
                        int iFeatLen = (sizeof(KDNN_FEAT_SIZE) * KDNN_FEAT_SIZE * 3) + (UNIT_ENROLL_FEATURE_SIZE) + 4 + 32;     //4: header len, 32: reserved
                        my_printf("iLen = %d, iFeatLen = %d\n", iLen, iFeatLen);
                        if(iLen < 3 + iFeatLen)
                            SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                        else
                        {
                            int iUserCount = dbm_GetPersonCount();
                            int iUserID = TO_SHORT(pbFeatData[0], pbFeatData[1]);
                            int iUserRole = pbFeatData[2];
                            if(iUserID < 0 || iUserID > N_MAX_PERSON_NUM)
                            {
                                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ID_INVALID);
                            }
                            else if(iUserRole < EUSER_ROLE_MIN || iUserRole > EUSER_ROLE_MAX)
                            {
                                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ROLE_INVALID);
                            }
                            else if(iUserCount == N_MAX_PERSON_NUM && iUserID == 0)
                            {
                                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FULL);
                            }
                            else if(memcmp(pbFeatData + 3, FEAT_VER, 4))
                            {
                                LOG_PRINT("[SetUserFeats]: invalid heaer!\n");
                                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                            }
                            else
                            {
                                int iCheckSum = 0;
                                for(int i = 0; i < (iFeatLen - 4) / 4; i += 4)
                                    iCheckSum = iCheckSum ^  *(int*)(pbFeatData + 3 + i);

                                if(iCheckSum != *(int*)(pbFeatData + 3 + iFeatLen - 4))
                                {
                                    LOG_PRINT("[SetUserFeats]: checksum error!\n");
                                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                                }
                                else
                                {
                                    LOG_PRINT("Recv Feat Data!\n");
                                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);

                                    SendGlobalMsg(MSG_FM, xRecvCmd.bType, (long)pbFeatData, iLen);
                                }
                            }
                        }
                    }
                    else
                    {
                        my_printf("Recv Feat Data Failed!\n");
                        SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                        my_free(pbFeatData);
                    }
                }
                else
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
            }
#endif
            else if(xRecvCmd.bType == FM_CMD_SET_BAUDRATE)
            {
                int iBaudRate = xRecvCmd.bP2;
                int iFlag = xRecvCmd.bP3;
                my_printf("FM_CMD_SET_BAUDRATE: %d, %d\n", iBaudRate, iFlag);

                if(iBaudRate <= 0 || iBaudRate > Baud_Rate_115200)
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                else
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, iBaudRate, FM_ACK_SUCCESS);

                    if(iFlag != 0)
                    {
                        //g_xUartMutex.Lock();
                        UART_SetBaudrate(UART_Baudrate(iBaudRate));
                        my_printf("UART_SetBaudrate1: %x\n", UART_Baudrate(iBaudRate));
                        //g_xUartMutex.Unlock();
                    }
                    else
                    {
                        //g_xUartMutex.Lock();
                        UART_SetBaudrate(UART_Baudrate(iBaudRate));
                        my_printf("UART_SetBaudrate: %x\n", UART_Baudrate(iBaudRate));
                        //g_xUartMutex.Unlock();
                    }
                }
            }
            else if(xRecvCmd.bType == FM_CMD_CAMERA_FLIP)
            {
#if 0
                if(xRecvCmd.bP3 == 0)
                {
                    g_xPS.x.bCamFlip = xRecvCmd.bP2;
                    UpdatePermanenceSettings();

                    g_xSS.iCameraRotate = g_xPS.x.bCamFlip;
                    fr_SetCameraFlip(g_xSS.iCameraRotate);

                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, g_xPS.x.bCamFlip, FM_ACK_SUCCESS);
                }
                else
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, g_xPS.x.bCamFlip, FM_ACK_SUCCESS);
                }
#endif
            }
            else if(xRecvCmd.bType == FM_CMD_SET_LED)
            {
                if(xRecvCmd.bP3 > E_LED_END)
                {
                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
                }
                else
                {
//                    if(xRecvCmd.bP3 == E_LED_OFF)
//                        SetLed(0);
//                    else if(xRecvCmd.bP3 == E_LED_R)
//                        SetLed(RLED);
//                    else if(xRecvCmd.bP3 == E_LED_G)
//                        SetLed(GLED);
//                    else if(xRecvCmd.bP3 == E_LED_B)
//                        SetLed(BLED);

                    SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                }
            }
            else if(xRecvCmd.bType == FM_CMD_FINISH)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
                SendGlobalMsg(MSG_FM, xRecvCmd.bType, 0, 0);
            }
            else if (xRecvCmd.bType == FM_CMD_GET_HWID)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ALREADY_ACTIVATED);
            }
            else if (xRecvCmd.bType == FM_CMD_ACTIVATE)
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_ALREADY_ACTIVATED);
            }
            else
            {
                SendAck(xRecvCmd.bType, GenSeq(1, xRecvCmd.bSeqNum), 0, 0, FM_ACK_FAIL);
            }
        }
    }
    dbug_printf("FaceModuleTask::run end.\n");
}

unsigned char FaceModuleTask::GetCheckSum(FM_CMD* pxCmd)
{
    if(pxCmd == NULL)
        return 0;

    unsigned char bChk = 0;
    unsigned char* pbData = (unsigned char*)pxCmd;
    for(unsigned int i = 1; i < sizeof(FM_CMD) - 1; i ++)
        bChk = bChk ^ pbData[i];

    return bChk;
}


void FaceModuleTask::SendAck(int iType, int iSeqNum, int iQ1, int iQ2, int iQ3)
{
    FM_CMD xSendCmd;
    memset(&xSendCmd, 0, sizeof(xSendCmd));

    xSendCmd.bP1 = iQ1;
    xSendCmd.bP2 = iQ2;
    xSendCmd.bP3 = iQ3;

    xSendCmd.bHeader = FM_HEADER;
    xSendCmd.bType = iType;
    xSendCmd.bSeqNum = iSeqNum;

    xSendCmd.bChk = GetCheckSum(&xSendCmd);

    UART_Send((unsigned char*)&xSendCmd, sizeof(xSendCmd));
}

void FaceModuleTask::SendData(int iType, int iSeqNum, int iAck, int iDataLen, unsigned char* pbData)
{
    FM_CMD xSendCmd;
    memset(&xSendCmd, 0, sizeof(xSendCmd));

    xSendCmd.bP1 = HIGH_BYTE(iDataLen);
    xSendCmd.bP2 = LOW_BYTE(iDataLen);
    xSendCmd.bP3 = iAck;

    xSendCmd.bHeader = FM_HEADER;
    xSendCmd.bType = iType;
    xSendCmd.bSeqNum = iSeqNum;

    xSendCmd.bChk = GetCheckSum(&xSendCmd);

    unsigned char* pbSendData = NULL;
    if(iDataLen > 0)
    {
        pbSendData = (unsigned char*)my_malloc(iDataLen + 2);

        memset(pbSendData, 0, iDataLen + 2);
        pbSendData[0] = FM_HEADER;
        memcpy(pbSendData + 1, pbData, iDataLen);

        unsigned char bChk = 0;
        for(int i = 0; i < iDataLen; i ++)
            bChk = bChk ^ pbData[i];
        pbSendData[iDataLen + 1] = bChk;
    }

    UART_Send((unsigned char*)&xSendCmd, sizeof(xSendCmd));
    if(iDataLen > 0)
        UART_Send(pbSendData, iDataLen + 2);

    if(pbSendData)
        my_free(pbSendData);
}

FM_CMD FaceModuleTask::CommCmd(FM_CMD xSendCmd)
{
    int iRet = 0;
    FM_CMD xCmd = { 0 };
    FM_CMD xRecvCmd = { 0 };

    m_bSeqNum ++;
    for(int i = 0; i < 3; i ++)
    {        
        xSendCmd.bHeader = FM_HEADER;
        xSendCmd.bSeqNum = GenSeq(0, m_bSeqNum);
        xSendCmd.bChk = GetCheckSum(&xSendCmd);

        UART_Send((unsigned char*)&xSendCmd, sizeof(xSendCmd));

        for(int j = 0; j < 200; j ++)
        {
            if(m_iStep == m_iEnd)
                break;

            memset(&xRecvCmd, 0, sizeof(xRecvCmd));
            iRet = UART_Recv((unsigned char*)&xRecvCmd, 1);
            if(iRet > 0)
            {
                if(xRecvCmd.bHeader == FM_HEADER)
                    break;
            }
            // my_usleep(1000);
        }

        if(xRecvCmd.bHeader == FM_HEADER)
        {
            iRet = UART_RecvDataForWait((unsigned char*)&xRecvCmd + 1, sizeof(FM_CMD) - 1, 200, 1);
            if(iRet != sizeof(FM_CMD) - 1)
                continue;

            xCmd = xRecvCmd;

//            my_printf("CommCmd: %d, %x, %x, %x, %x, %f\n", i, xCmd.bType, xSendCmd.bType, xCmd.bSeqNum, GenSeq(1, xSendCmd.bSeqNum), Now());
            if(xCmd.bType == xSendCmd.bType && xCmd.bSeqNum == GenSeq(1, xSendCmd.bSeqNum))
            {
                break;
            }
        }
    }

    return xCmd;
}

FM_CMD FaceModuleTask::CommData(FM_CMD xSendCmd, unsigned char* pbData, int iLen)
{
    int iRet = 0;
    FM_CMD xCmd = { 0 };
    FM_CMD xRecvCmd = { 0 };

    m_bSeqNum ++;
    for(int i = 0; i < 3; i ++)
    {
        xSendCmd.bHeader = FM_HEADER;
        xSendCmd.bSeqNum = GenSeq(0, m_bSeqNum);
        xSendCmd.bChk = GetCheckSum(&xSendCmd);

        UART_Send((unsigned char*)&xSendCmd, sizeof(xSendCmd));

        for(int j = 0; j < 200; j ++)
        {
            if(m_iComm == 0 || m_iStep == m_iEnd)
                break;

            memset(&xRecvCmd, 0, sizeof(xRecvCmd));
            iRet = UART_Recv((unsigned char*)&xRecvCmd, 1);
            if(iRet > 0)
            {
                if(xRecvCmd.bHeader == FM_HEADER)
                    break;
            }
            // my_usleep(1000);
        }

        if(xRecvCmd.bHeader == FM_HEADER)
        {
            iRet = UART_Recv((unsigned char*)&xRecvCmd + 1, sizeof(FM_CMD) - 1);
            if(iRet != sizeof(FM_CMD) - 1)
                continue;

            xCmd = xRecvCmd;

            my_printf("CommData: %d, %x, %x, %x, %x, %f\n", i, xCmd.bType, xSendCmd.bType, xCmd.bSeqNum, GenSeq(1, xSendCmd.bSeqNum), Now());
            if(xCmd.bType == xSendCmd.bType && xCmd.bSeqNum == GenSeq(1, xSendCmd.bSeqNum))
            {
                if(iLen > 0 && pbData)
                {
                    unsigned char* pbSendData = (unsigned char*)my_malloc(iLen + 2);
                    memset(pbSendData, 0, iLen + 2);

                    pbSendData[0] = FM_HEADER;
                    memcpy(pbSendData + 1, pbData, iLen);
                    unsigned char bChk = 0;
                    for(int j = 1; j < iLen + 1; j ++)
                        bChk = bChk ^ pbSendData[j];
                    pbSendData[iLen + 1] = bChk;

                    UART_Send(pbSendData, iLen + 2);

                    my_free(pbSendData);
                }

                break;
            }
        }
    }

    return xSendCmd;
}


FM_CMD FaceModuleTask::ParseCmd(unsigned char* pbData, int iLen)
{
    FM_CMD xCmd = { 0 };
    if(pbData == NULL || iLen <= 0)
        return xCmd;

    for(int i = 0; i < iLen; i ++)
    {
        if(pbData[i] != 0)
        {
            pbData = pbData + i;
            iLen -= i;
            break;
        }
    }

    if(iLen < (int)sizeof(FM_CMD))
        return xCmd;

    memcpy(&xCmd, pbData, sizeof(FM_CMD));

    if(xCmd.bHeader != FM_HEADER)
    {
        memset(&xCmd, 0, sizeof(xCmd));
        return xCmd;
    }

    if(xCmd.bChk != GetCheckSum(&xCmd))
    {
        memset(&xCmd, 0, sizeof(xCmd));
        return xCmd;
    }

    return xCmd;
}

unsigned char FaceModuleTask::GenSeq(int iFlag, unsigned char bSeqNum)
{
    if(iFlag > 0)
        return bSeqNum | (0x80);
    else
        return bSeqNum & (0x7F);
}


unsigned char FaceModuleTask::Get_SenseCheckSum(unsigned char* pbData, int iLen)
{
    unsigned char bCheckSum = 0;
    for(int i = 0; i < iLen; i ++)
        bCheckSum ^= pbData[i];

    return bCheckSum;
}

int FaceModuleTask::Get_MsgLen(s_msg* msg)
{
    if(msg == NULL)
        return 0;

    return TO_SHORT(msg->size_heb, msg->size_leb);
}

void FaceModuleTask::ThreadProc()
{
    run();
}

void* faceModuleTask_ThreadProc1(void* param)
{
    FaceModuleTask* pThread = (FaceModuleTask*)(param);
    pThread->ThreadProc();
    return NULL;
}