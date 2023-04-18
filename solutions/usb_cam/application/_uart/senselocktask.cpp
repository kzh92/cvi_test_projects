
#include "senselocktask.h"
#include "settings.h"
#include "DBManager.h"
#include "appdef.h"
#include "uartcomm.h"
#include "msg.h"
#include "i2cbase.h"
#include "faceengine.h"
#include "shared.h"
#include "sha1.h"
#include "facemoduledef.h"
#include "EngineDef.h"
#include "systembase.h"
#include "drv_gpio.h"
#include "FaceRetrievalSystem.h"
#include "senselockmessage.h"
#include "aes1.h"
#include "aescrypt.h"
#include "upgradebase.h"
#include "facemoduletask.h"
#include "common_types.h"

#include <string.h>


static message_queue g_queue_send;
//static pthread_t g_thread_send = 0;
#ifndef NOTHREAD_MUL
static mythread_ptr g_thread_send = NULL;
#endif

#define REH_MAGIC_LEN   8
#define REH_MAX_SLOTS   64
#define REH_KEY_LEN     32
#define REH_MAGIC       "ESUSB1"
#define REH_CF_MAGIC    "EASEN_CHECK_FIRMWARE"
 
#define PAGE_SIZE       0x1000

typedef struct _tagUpInfo
{
    unsigned int offset;
    unsigned int size;
    unsigned int reserved[14];
} stUpInfo;
 
typedef struct _tagREHSlot
{
    unsigned int offset;
    unsigned int size;
} stREHSlot;
 
typedef struct _tagUsbUpgradeEasenHeader
{
    char magic[REH_MAGIC_LEN];
    unsigned int slot_count;
    stREHSlot slots[REH_MAX_SLOTS];
    unsigned char key[REH_KEY_LEN];
} stUsbUpgradeEasenHeader;

SenseLockTask::EEncMode SenseLockTask::m_encMode = SenseLockTask::EM_NOENCRYPT;
char SenseLockTask::EncKey[ENC_KEY_SIZE + 1] = { 0 };
unsigned char SenseLockTask::m_encKeyPos[ENC_KEY_SIZE] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
};

mymutex_ptr SenseLockTask::CommMutex = my_mutex_init();

extern void StartVDB();
extern void StopVDB();
void* senseLockTask_ThreadProc1(void*);
void* senseSendThread_ThreadProc1(void*);
extern int processGlobalMsg();

#define MODE921600  1
#define APB2_30MHZ  (0x02000013)
#define APB2_24MHZ  (0x01000000)

#if (USE_UVC_PAUSE_MODE)
float PauseUVC_Time = 0;
#endif

void _decryptBuf(unsigned char *buf, unsigned long bufLen, unsigned char *key)
{
    for (unsigned long i = 0; i < bufLen; i++)
    {
        buf[i] = buf[i] ^ (key[i % REH_KEY_LEN] + i / REH_KEY_LEN);
    }
}

void encryptUpgrader(unsigned char *buf, unsigned long bufLen)
{
    unsigned int anUID[4] = { 0 };
    GetSSDID(anUID);
    for (unsigned long i = 0; i < bufLen; i++)
    {
        buf[i] = buf[i] ^ anUID[i % 4];
    }
}
//unsigned char SenseLockTask::m_bSeqNum = 0;

SenseLockTask::SenseLockTask()
{
    Init();
}

SenseLockTask::~SenseLockTask()
{
}

void SenseLockTask::Init()
{
#if 0
    m_mutex = PTHREAD_MUTEX_INITIALIZER;
    m_cond = PTHREAD_COND_INITIALIZER;
#endif
    m_thread = NULL;
}

void SenseLockTask::Deinit()
{
}

int SenseLockTask::Start()
{
    if (m_thread == NULL)
    {
        m_iStep = 0;
        m_iEnd = 1;
    //    m_iSendType = 0;
        m_iComm = 0;
        m_iActive = 0;
        message_queue_init(&g_queue_send, sizeof(MSG), MAX_MSG_NUM);
#ifndef NOTHREAD_MUL
        if(my_thread_create_ext(&m_thread, NULL, senseLockTask_ThreadProc1, this, (char*)"stask", 4096, 0 /*MYTHREAD_PRIORITY_MEDIUM*/))
            my_printf("[LockTask]create thread error.\n");
#else // ! NOTHREAD_MUL
#endif // !NOTHREAD_MUL
    }

    return 1;
}

int SenseLockTask::Stop()
{
    m_iStep = m_iEnd;
    return 1;
}

void SenseLockTask::Wait()
{
#ifndef __RTK_OS__
    if(m_thread != 0)
    {
        pthread_join(m_thread, NULL);
        m_thread = 0;
    }
#else // !__RTK_OS__
    if (m_thread != NULL)
    {
        my_thread_join(&m_thread);
        m_thread = NULL;
    }
#endif // !__RTK_OS__
}

void* senseSendThread_ThreadProc1(void*)
{
    s_msg* msg = NULL;
    MSG* pMsg = NULL;
#if (NOTE_INTERVAL_MS)
    MSG* pMsgNext = NULL;
    float rLastSendTime = 0;
    int iLastSendMsgID = -1;
    pMsg = (MSG*)message_queue_read(&g_queue_send);
#endif // NOTE_INTERVAL_MS
#ifndef NOTHREAD_MUL
    dbug_printf("send thread start\n");
#endif // ! NOTHREAD_MUL
    while(1)
    {
#if (!NOTE_INTERVAL_MS)
        pMsg = (MSG*)message_queue_tryread(&g_queue_send);
#endif // NOTE_INTERVAL_MS
        if (pMsg == NULL)
        {
#ifdef NOTHREAD_MUL
            break;
#else // NOTHREAD_MUL
            my_usleep(1000);
            continue;
#endif // NOTHREAD_MUL
        }

        msg = (s_msg*)pMsg->data1;

        if(msg == NULL)
        {
            message_queue_message_free(&g_queue_send, (void*)pMsg);
            break;
        }

        int iPckLen= 0;
        unsigned char* pbPck = NULL;
        if (msg->mid == MID_NOTE)
        {
            s_msg_note_data *note_data = (s_msg_note_data*)(msg->data);
            if (note_data->nid == NID_READY)
            {
                SenseLockTask::m_encMode = SenseLockTask::EM_NOENCRYPT;
            }
        }
        if(SenseLockTask::m_encMode == SenseLockTask::EM_NOENCRYPT)
        {
            iPckLen = SenseLockTask::Get_MsgLen(msg) + 6;       //head len(2) + msg id(1) + msg size(2) + parity(1);
            pbPck = (unsigned char*)my_malloc(iPckLen);

            pbPck[0] = SENSE_HEAD1;
            pbPck[1] = SENSE_HEAD2;
            memcpy(pbPck + 2, msg, SenseLockTask::Get_MsgLen(msg) + 3);

            pbPck[iPckLen - 1] = SenseLockTask::Get_CheckSum((unsigned char*)msg, SenseLockTask::Get_MsgLen(msg) + 3);
        }
        else if(SenseLockTask::m_encMode == SenseLockTask::EM_AES)
        {
            int iOutLen = 0;
            unsigned char* pbEnc = NULL;

            int iSrcLen = ((TO_SHORT(msg->size_heb, msg->size_leb) + 3) + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE * AES_BLOCK_SIZE;
            unsigned char* pbOrg = (unsigned char*)my_malloc(iSrcLen);
            memset(pbOrg, 0, iSrcLen);
            memcpy(pbOrg, msg, TO_SHORT(msg->size_heb, msg->size_leb) + 3);

            LOG_PRINT("[Sense] Org = %d: ", iSrcLen);
            for (int i = 0;i < iSrcLen; i++)
                LOG_PRINT("0x%02x,", pbOrg[i]);
            LOG_PRINT("\n");

            SenseLockTask::Encrypt_Msg((unsigned char*)pbOrg, iSrcLen, &pbEnc, &iOutLen);
            if(pbEnc != NULL && iOutLen > 0)
            {
                iPckLen = iOutLen + 5;
                pbPck = (unsigned char*)my_malloc(iPckLen);

                pbPck[0] = SENSE_HEAD1;
                pbPck[1] = SENSE_HEAD2;
                pbPck[2] = HIGH_BYTE(iOutLen);
                pbPck[3] = LOW_BYTE(iOutLen);
                memcpy(pbPck + 4, pbEnc, iOutLen);

                pbPck[iPckLen - 1] = SenseLockTask::Get_CheckSum(pbEnc, iOutLen);
                my_free(pbEnc);
            }

            my_free(pbOrg);
        }
        else if(SenseLockTask::m_encMode == SenseLockTask::EM_XOR)
        {
            iPckLen = SenseLockTask::Get_MsgLen(msg) + 8;       //head len(2) + msg size(2) + parity(1);
            pbPck = (unsigned char*)my_malloc(iPckLen);

            pbPck[0] = SENSE_HEAD1;
            pbPck[1] = SENSE_HEAD2;
            pbPck[2] = HIGH_BYTE(SenseLockTask::Get_MsgLen(msg) + 3);
            pbPck[3] = LOW_BYTE(SenseLockTask::Get_MsgLen(msg) + 3);
            unsigned char* pbDst = pbPck + 4;
            SenseLockTask::Encrypt_Msg((unsigned char*)msg, SenseLockTask::Get_MsgLen(msg) + 3, &pbDst);

            pbPck[iPckLen - 1] = SenseLockTask::Get_CheckSum(pbPck + 4, SenseLockTask::Get_MsgLen(msg) + 3);
        }

#if (NOTE_INTERVAL_MS)
        //interval time must be 100ms between note and note, note and reply.
        if (iLastSendMsgID == MID_NOTE && (msg->mid == MID_NOTE || msg->mid == MID_REPLY))
        {
            float rCurTime = Now();
            int iInterval = msg->mid == MID_NOTE ? NOTE_INTERVAL_MS : NOTE2REPLY_MS;
            if (rLastSendTime != 0 && rCurTime - rLastSendTime < iInterval)
                my_usleep((iInterval - (rCurTime - rLastSendTime))*1000);
        }
        iLastSendMsgID = msg->mid;
        rLastSendTime = Now();
#endif // NOTE_INTERVAL_MS

        if (DEFAULT_UART0_BAUDRATE != Baud_Rate_115200)
        {
            if (msg->mid == MID_NOTE && msg->data[0] == NID_READY)
            {
                unsigned char readyPacket[8] = {0xEF, 0xAA, 0x01, 0x00, 0x02, 0x00, 0x05/*baudrate*/, 0x06};
                readyPacket[6] = DEFAULT_UART0_BAUDRATE + 1;
                readyPacket[7] = SenseLockTask::Get_CheckSum(readyPacket + 2, sizeof(readyPacket) - 3);
                my_mutex_lock(SenseLockTask::CommMutex);
                UART_SetBaudrate(UART_Baudrate(Baud_Rate_115200));
                my_usleep(5 * 1000);
                UART_Send((unsigned char*)readyPacket, sizeof(readyPacket));
                my_usleep(5 * 1000);
                UART_SetBaudrate(UART_Baudrate(DEFAULT_UART0_BAUDRATE));
                my_usleep(5 * 1000);
                my_mutex_unlock(SenseLockTask::CommMutex);
            }
        }

        my_mutex_lock(SenseLockTask::CommMutex);
        UART_Send((unsigned char*)pbPck, iPckLen);
        my_usleep(UART_SendTimePredict(iPckLen) * 1000);
        my_mutex_unlock(SenseLockTask::CommMutex);

#if (NOTE_INTERVAL_MS)
        pMsgNext = (MSG*)message_queue_tryread(&g_queue_send);
        while(pMsgNext == NULL)
        {
            if (/*g_xSS.rFaceEngineTime != 0 && */msg->mid == MID_NOTE && msg->data[0] == NID_FACE_STATE)
            {
                if (Now() - rLastSendTime >= NOTE_INTERVAL_MS)
                {
                    iLastSendMsgID = msg->mid;
                    rLastSendTime = Now();

                    my_mutex_lock(SenseLockTask::CommMutex);
                    UART_Send((unsigned char*)pbPck, iPckLen);
                    my_usleep(5 * 1000);
                    my_mutex_unlock(SenseLockTask::CommMutex);
                }
            }
            my_usleep(5*1000);
            pMsgNext = (MSG*)message_queue_tryread(&g_queue_send);
        }
#endif // NOTE_INTERVAL_MS

        my_free(msg);
        my_free(pbPck);
        message_queue_message_free(&g_queue_send, (void*)pMsg);

#if (NOTE_INTERVAL_MS)
        pMsg = pMsgNext;
#endif // NOTE_INTERVAL_MS
    }
#ifndef NOTHREAD_MUL
    dbug_printf("send thread end\n");
#endif // ! NOTHREAD_MUL
    return NULL;
}

int SenseLockTask::doProcess()
{
#ifdef NOTHREAD_MUL
    run();
#endif // NOTHREAD_MUL
    return 0;
}

void SenseLockTask::run()
{
    int iCurCmd = -1, iLastCmd = -1;
#ifdef NOTHREAD_MUL
    int iMsgSent = 0;
#endif // NOTHREAD_MUL
    stUsbUpgradeEasenHeader header;
    //static pthread_t g_thread_send = 0;

#ifndef NOTHREAD_MUL
    dbug_printf("SenseLockTask::run start\n");
    //message_queue_init(&g_queue_send, sizeof(MSG), MAX_MSG_NUM);

    if(my_thread_create_ext(&g_thread_send, NULL, senseSendThread_ThreadProc1, NULL, (char*)"sstask", 8192, 0/*MYTHREAD_PRIORITY_MEDIUM*/))
        my_printf("[SendThreadTask]create send thread error.\n");
#endif // NOTHREAD_MUL

    while(m_iStep < m_iEnd)
    {
        int iRecvHead = 0;
        int iRecvEasenHead = 0;
        unsigned char abMsgHead[10] = { 0 };
        FM_CMD xRecvEasenCmd = {0};

#ifdef NOTHREAD_MUL
        if (iMsgSent)
        {
            processGlobalMsg();
            iMsgSent = 0;
        }
        if (g_xSS.iStartOta)
            break;
#endif // NOTHREAD_MUL
        while(m_iStep < m_iEnd)
        {
            int iSleepTime = 1;
            int iRet = UART_Recv(abMsgHead, 1);
            if(iRet > 0 && abMsgHead[0] == SENSE_HEAD1)
            {
                iRet = UART_RecvDataForWait(abMsgHead + 1, 1, 10, 0);
                if(iRet > 0 && abMsgHead[1] == SENSE_HEAD2)
                {
                    iRecvHead = 1;
                    break;
                }
                else
                    iSleepTime = 0;
            }
            else if(iRet > 0 && abMsgHead[0] == FM_HEADER)
            {
                iRet = UART_RecvDataForWait((unsigned char*)&xRecvEasenCmd + 1, sizeof(FM_CMD) - 1, 100, 0);
                if((iRet == sizeof(FM_CMD) - 1) && (xRecvEasenCmd.bChk == FaceModuleTask::GetCheckSum(&xRecvEasenCmd)))
                {
                    iRecvEasenHead = 1;
                    break;
                }
                else
                {
                    iSleepTime = 0;
                    if(iRet != sizeof(FM_CMD) - 1)
                        my_printf("[FM] Incomplete Easen Packet\n");
                    else if(xRecvEasenCmd.bChk == FaceModuleTask::GetCheckSum(&xRecvEasenCmd))
                        my_printf("[FM] RecvCmd CheckSum Error: %x, %x\n", xRecvEasenCmd.bChk, FaceModuleTask::GetCheckSum(&xRecvEasenCmd));
                }
            }
            else if(iRet > 0)
            {
                iSleepTime = 0;
            }
#if (USE_UVC_PAUSE_MODE)
            if (g_xSS.iUVCpause > 0)
            {
                if (Now() - PauseUVC_Time > UVC_PAUSE_LIMIT_TIME)
                    g_xSS.iUVCpause = 0;
                else if (Now() - PauseUVC_Time > 0)
                    g_xSS.iUVCpause = 2;
#if (FRM_PRODUCT_TYPE >= FRM_PT_DEFAULT_3_3)
                else
                {
                    if (g_xSS.iUVCpause == 2 && Now() - PauseUVC_Time + 1800 > 0)
                        g_xSS.iUVCpause = 1;
                }
#endif // FRM_PRODUCT_TYPE
            }
#endif // USE_UVC_PAUSE_MODE
#ifndef NOTHREAD_MUL
            if ((g_xSS.iMState != MS_OTA || g_xSS.bCheckFirmware == 1) && iSleepTime > 0)
                my_usleep(iSleepTime);//
#else // ! NOTHREAD_MUL
            //my_usleep(1);
#endif // ! NOTHREAD_MUL
        }

        if(iRecvEasenHead == 1)
        {
            if(xRecvEasenCmd.bType == FM_CMD_DEV_TEST_START)
            {
                SendGlobalMsg(MSG_FM, xRecvEasenCmd.bType, xRecvEasenCmd.bP3, 0);
#ifdef NOTHREAD_MUL
                iMsgSent = 1;
#endif // NOTHREAD_MUL
                FaceModuleTask::SendAck(xRecvEasenCmd.bType, FaceModuleTask::GenSeq(1, xRecvEasenCmd.bSeqNum), 0, 0, FM_ACK_SUCCESS);
            }

            continue;
        }

        if(iRecvHead == 0)
            continue;

        my_mutex_lock(SenseLockTask::CommMutex);

        int iMsgLen = 0;
        unsigned char* pMsg = NULL;
        if(SenseLockTask::m_encMode == EM_NOENCRYPT)
        {
            unsigned char abMsgHead[3] = { 0 };
            int iRet = UART_RecvDataForWait(abMsgHead, 3, 100, 0);
            if(iRet <= 0)
            {
                my_mutex_unlock(SenseLockTask::CommMutex);
                my_printf("[Sense] Msg Recv Failed1!\n");
                continue;
            }

            iMsgLen = TO_SHORT(abMsgHead[1], abMsgHead[2]);
            pMsg = (unsigned char*)my_malloc(iMsgLen + 4);
            if (pMsg == NULL)
            {
                my_printf("malloc fail, pMsg1, %d.\n", iMsgLen + 4);
                continue;
            }
            memcpy(pMsg, abMsgHead, sizeof(abMsgHead));

            iRet = UART_RecvDataForWait(pMsg + sizeof(abMsgHead), iMsgLen + 1, 100, 0);
            if(iRet <= 0)
            {
                my_mutex_unlock(SenseLockTask::CommMutex);
                my_printf("[Sense] Msg Recv Failed2!\n");
                my_free(pMsg);
                continue;
            }

            if(pMsg[iMsgLen + 3] != Get_CheckSum(pMsg, iMsgLen + 3))
            {
                my_mutex_unlock(SenseLockTask::CommMutex);
                my_printf("[Sense] CheckSum Failed1: %x, %x\n", pMsg[iMsgLen + 3], Get_CheckSum(pMsg, iMsgLen + 3));
                my_free(pMsg);
                continue;
            }
        }
        else
        {
            unsigned char abPckHead[2] = { 0 };
            int iRet = UART_RecvDataForWait(abPckHead, 2, 100, 0);
            if(iRet <= 0)
            {
                my_mutex_unlock(SenseLockTask::CommMutex);
                my_printf("[Sense] Msg Recv Failed3!\n");
                continue;
            }

            int iPckLen = TO_SHORT(abPckHead[0], abPckHead[1]);
            unsigned char* pPck = (unsigned char*)my_malloc(iPckLen + 3);
            memcpy(pPck, abPckHead, sizeof(abPckHead));

            iRet = UART_RecvDataForWait(pPck + sizeof(abPckHead), iPckLen + 1, 100, 0);
            if(iRet <= 0)
            {
                my_mutex_unlock(SenseLockTask::CommMutex);
                my_printf("[Sense] Msg Recv Failed2!\n");
                continue;
            }

            if(pPck[iPckLen + 2] != Get_CheckSum(pPck + 2, iPckLen))
            {
                my_mutex_unlock(SenseLockTask::CommMutex);
                my_printf("[Sense] CheckSum Failed2: %x, %x\n", pPck[iPckLen + 2], Get_CheckSum(pPck + 2, iPckLen));
                continue;
            }

            if(SenseLockTask::m_encMode == EM_AES)
            {
                SenseLockTask::Decrypt_Msg(pPck + 2, iPckLen, &pMsg, &iMsgLen);
                if(pMsg == NULL || iMsgLen == 0)
                {
                    my_mutex_unlock(SenseLockTask::CommMutex);
                    my_printf("[Sense] Msg Decrypt Failed!  %d\n", iPckLen);
                    continue;
                }

                LOG_PRINT("[Sense] Decode = %d: ", iMsgLen);
                for (int i = 0;i < iMsgLen; i++)
                    LOG_PRINT("0x%02x,", pMsg[i]);
                LOG_PRINT("\n");

                my_free(pPck);
            }
            else if(SenseLockTask::m_encMode == EM_XOR)
            {
                iMsgLen = iPckLen;
                pMsg = (unsigned char*)my_malloc(iPckLen);
                SenseLockTask::Decrypt_Msg(pPck + 2, iPckLen, &pMsg);
                my_free(pPck);
            }
        }

        my_mutex_unlock(SenseLockTask::CommMutex);
        if(pMsg == NULL)
            continue;

        s_msg* msg = (s_msg*)pMsg;

        if (g_xSS.iUsbHostMode == 0)
            g_xSS.rLastSenseCmdTime = Now();

        if(m_iActive == 0)
        {
            my_printf("Recv MID (0x%x) in inactive state..... %f\n", msg->mid, Now());
            my_free(msg);
            continue;
        }

        iCurCmd = msg->mid;
        
        if(g_xSS.iMState == MS_OTA)
        {
            if(msg->mid == MID_STOP_OTA)
            {
                my_printf("MID_STOP_OTA\n");
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_STOP_OTA, MR_SUCCESS);
                Send_Msg(reply_msg);

                SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_DONE_STOP, 0);
#ifdef NOTHREAD_MUL
                iMsgSent = 1;
#endif // NOTHREAD_MUL
            }
            else if(msg->mid == MID_GET_OTA_STATUS)
            {
                my_printf("MID_GET_OTA_STATUS\n");
                int iPckCount = TO_INT(g_xSS.msg_otaheader_data.num_pkt);
                int iNextPID = 0;
                for(int i = 0; i < iPckCount; i ++)
                {
                    if(g_xSS.piOtaPckIdx[i] == 0)
                    {
                        iNextPID = i;
                        break;
                    }
                }

                s_msg* reply_msg = SenseLockTask::Get_Reply_GetOtaStatus(g_xSS.iMState, iNextPID);
                Send_Msg(reply_msg);
            }
            else if(msg->mid == MID_CONFIG_BAUDRATE)
            {
                s_msg_config_baudrate* msg_config_baudrate = (s_msg_config_baudrate*)msg->data;

                g_xCS.x.bUpgradeBaudrate = msg_config_baudrate->baudrate_index;
                UpdateCommonSettings();

                my_printf("MID_CONFIG_BAUDRATE: %d\n", msg_config_baudrate->baudrate_index);
#if 1
                if (BR_IS_VALID(msg_config_baudrate->baudrate_index))
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_CONFIG_BAUDRATE, MR_SUCCESS);
                    Send_Msg(reply_msg);

                    my_usleep(50 * 1000);
                    UART_SetBaudrate(UART_Baudrate(msg_config_baudrate->baudrate_index));
                }
                else
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_CONFIG_BAUDRATE, MR_FAILED4_INVALIDPARAM);
                    Send_Msg(reply_msg);
                }
#endif
            }
            else if(msg->mid == MID_OTA_HEADER)
            {
                my_printf("MID_OTA_HEADER\n");
                if(SenseLockTask::Get_MsgLen(msg) < (int)sizeof(g_xSS.msg_otaheader_data))
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_OTA_HEADER, MR_FAILED4_INVALIDPARAM);
                    Send_Msg(reply_msg);

                    SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_PACKET_ERROR, 0);
#ifdef NOTHREAD_MUL
                    iMsgSent = 1;
#endif // NOTHREAD_MUL

                    my_free(msg);
                    continue;
                }

                memcpy(&g_xSS.msg_otaheader_data, msg->data, SenseLockTask::Get_MsgLen(msg));

                int iFileSize = TO_INT(g_xSS.msg_otaheader_data.fsize_b);
                int iPckCount = TO_INT(g_xSS.msg_otaheader_data.num_pkt);
                int iPckSize = TO_SHORT(g_xSS.msg_otaheader_data.pkt_size[0], g_xSS.msg_otaheader_data.pkt_size[1]);

                if(iFileSize < 0 || iFileSize > MAX_OTA_FSIZE || iPckSize <= 0 || iPckSize > MAX_OTA_PCK_SIZE)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_OTA_HEADER, MR_FAILED4_INVALIDPARAM);
                    Send_Msg(reply_msg);

                    SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_PACKET_ERROR, 0);
#ifdef NOTHREAD_MUL
                    iMsgSent = 1;
#endif // NOTHREAD_MUL

                    my_free(msg);
                    continue;
                }
                g_xSS.pbOtaData = (unsigned char*)my_malloc(iFileSize);
                g_xSS.piOtaPckIdx = (int*)my_malloc(iPckCount * sizeof(int));
                memset(g_xSS.piOtaPckIdx, 0, iPckCount * sizeof(int));

                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_OTA_HEADER, MR_SUCCESS);
                Send_Msg(reply_msg);

                SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_PCK, 0);
#ifdef NOTHREAD_MUL
                iMsgSent = 1;
#endif // NOTHREAD_MUL
            }
            if(msg->mid == MID_OTA_PACKET)
            {
                s_msg_otapacket_data* msg_otapacket_data = (s_msg_otapacket_data*)msg->data;
                int iFSize = TO_INT(g_xSS.msg_otaheader_data.fsize_b);
                int iPckCount = TO_INT(g_xSS.msg_otaheader_data.num_pkt);
                int iPckSize = TO_SHORT(g_xSS.msg_otaheader_data.pkt_size[0], g_xSS.msg_otaheader_data.pkt_size[1]);
                int iPID = TO_SHORT(msg_otapacket_data->pid[0], msg_otapacket_data->pid[1]);
                int iPSize = TO_SHORT(msg_otapacket_data->psize[0], msg_otapacket_data->psize[1]);

                if(iPID * iPckSize + iPSize <= iFSize && iPID * iPckSize >= 0 && iPID >= 0 && iPID < iPckCount)
                {
                    memcpy(g_xSS.pbOtaData + iPID * iPckSize, msg_otapacket_data->data, iPSize);

                    //my_printf("msg: MID_OTA_PACKET: %d, %d, ===%d, %d\n", iPID, iPSize, g_xSS.iOtaOff, g_xSS.iOtaSize);
                    my_printf("%d, %d, %d, %d\n", iPID, iPckSize, iPID * iPckSize + iPSize, iFSize);

                    s_msg* reply_msg = Get_Reply(MID_OTA_PACKET, MR_SUCCESS);
                    Send_Msg(reply_msg);

                    g_xSS.piOtaPckIdx[iPID] = 1;
                    SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_PCK, 0);
#ifdef NOTHREAD_MUL
                    iMsgSent = 1;
#endif // NOTHREAD_MUL
                }
                else
                {
                    my_printf("=============+Error Pcket!\n");
                    SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_PACKET_ERROR, 0);
#ifdef NOTHREAD_MUL
                    iMsgSent = 1;
#endif // NOTHREAD_MUL

                    my_free(msg);
                    continue;
                }

#if 1
                int iValid = 1;
                for(int i = 0; i < iPckCount; i ++)
                {
                    if(g_xSS.piOtaPckIdx[i] == 0)
                    {
                        iValid = 0;
                        break;
                    }
                }

                if(iValid)
                {
                    my_printf("[upgrade] 0x%x receive OK\n", iFSize);

                    int erase_size = 0;
                    unsigned int i;
                    unsigned char bFileCheckSum = 0;
                    unsigned int iFileSize = iFSize;
                    iFileSize--; // omitting checksum.
                    for (i = 0 ; i < iFileSize ; i++)
                        bFileCheckSum = bFileCheckSum ^ g_xSS.pbOtaData[i];
                    if (bFileCheckSum != g_xSS.pbOtaData[iFileSize])
                    {
                        my_printf("XXXXXXXXXXXXX CheckSum Error: %x, %x\n", g_xSS.pbOtaData[iFileSize], bFileCheckSum);
                        SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_CHECKSUM_ERROR, 0);
#ifdef NOTHREAD_MUL
                        iMsgSent = 1;
#endif // NOTHREAD_MUL
                        my_free(msg);
                        continue;
                    }

                    stUpInfo upgrader;
                    my_flash_read(UPGRADER_INFO_ADDR, sizeof(stUpInfo), &upgrader,sizeof(stUpInfo));

                    memcpy((unsigned int *)&header, (unsigned int *)g_xSS.pbOtaData, sizeof(stUsbUpgradeEasenHeader));
                    _decryptBuf((unsigned char *)&header, sizeof(header) - REH_KEY_LEN, header.key);
                    if (!strcmp(header.magic, REH_MAGIC))
                    {
                        int offset = sizeof(header);
                        for (i = 0; i < header.slot_count; i++)
                        {
                            if (header.slots[i].size <= 0)
                                continue;
                            _decryptBuf((unsigned char *)(g_xSS.pbOtaData + offset), header.slots[i].size, header.key);
                            if (i == 0 && header.slots[i].size >= strlen(REH_CF_MAGIC))
                            {
                                if (strncmp((const char*)(g_xSS.pbOtaData + offset), REH_CF_MAGIC, strlen(REH_CF_MAGIC)) == 0)
                                {
                                    g_xCS.x.bCheckFirmware = 1;
                                    UpdateCommonSettings();
                                    my_printf("*** check firmware\n");
                                    break;
                                }
                            }

                            erase_size = ((header.slots[i].size - 1)/PAGE_SIZE + 1) * PAGE_SIZE;
                            my_printf("[upgrade] Erase : start = 0x%x, end = 0x%x\n", header.slots[i].offset, header.slots[i].offset + erase_size);

                            if(i == 0)//for Upgrader rtk
                            {
                                encryptUpgrader((unsigned char *)(g_xSS.pbOtaData + offset), header.slots[i].size);
                                upgrader.offset = header.slots[i].offset;
                                upgrader.size = erase_size;
                            }

                            if (my_flash_write(header.slots[i].offset, g_xSS.pbOtaData + offset, header.slots[i].size) < header.slots[i].size)
                            {
                                my_printf("[upgrade] failed to write\n");
                                break;
                            }

                            offset += header.slots[i].size;
                        }

                        if (i == header.slot_count)//receive success
                        {
                            my_flash_erase(UPGRADER_INFO_ADDR, PAGE_SIZE);
                            my_flash_write_pages(UPGRADER_INFO_ADDR, &upgrader, sizeof(stUpInfo));
                        }

                        SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_DONE_OK, 0);
#ifdef NOTHREAD_MUL
                        iMsgSent = 1;
#endif // NOTHREAD_MUL

                        my_free(msg);
                        continue;
                    }
                    else
                    {
                        my_printf("[upgrade] Magic Fail %s\n", header.magic);
                        SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_PACKET_ERROR, 0);
#ifdef NOTHREAD_MUL
                        iMsgSent = 1;
#endif // NOTHREAD_MUL

                        my_free(msg);
                        continue;
                    }
#if 0//darkhorse
                    int iFileSize = iFSize;
                    iFileSize--; // omitting checksum.
                    unsigned int iPieceSize = 5120;
                    unsigned char* tmp_buf = NULL;
                    unsigned int i = 0;
                    int iTotal = iFileSize / iPieceSize;

                    unsigned char checkSum = 0;
                    unsigned char bFileCheckSum = 0;
                    for(; i < (unsigned int)iTotal; i++)
                    {
                        tmp_buf = g_xSS.pbOtaData + i * iPieceSize;
                        for(unsigned int j = 0; j < iPieceSize; j++)
                        {
                            tmp_buf[j] = tmp_buf[j] ^ (((i * iPieceSize + j) ^ (iFileSize - (i * iPieceSize + j))) % 128);
                            checkSum ^= tmp_buf[j];
                        }
                    }

                    unsigned int iRemain = iFileSize % iPieceSize;
                    if(iRemain != 0)
                    {
                        tmp_buf = g_xSS.pbOtaData + i * iPieceSize;
                        for(unsigned int j = 0; j < iRemain; j++)
                        {
                            tmp_buf[j] = tmp_buf[j] ^ (((iTotal * iPieceSize + j) ^ (iFileSize - (iTotal * iPieceSize + j))) % 128);
                            checkSum ^= tmp_buf[j];
                        }
                    }

                    bFileCheckSum = g_xSS.pbOtaData[iFileSize];

                    if(checkSum != bFileCheckSum)
                    {
                        my_printf("XXXXXXXXXXXXX CheckSum Error: %x, %x\n", checkSum, bFileCheckSum);

                        SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_CHECKSUM_ERROR, 0);
                        iMsgSent = 1;

                        my_free(msg);
                        continue;
                    }
                    else
                    {
                        mount_tmp();
                        FILE* fp = fopen(UPDATE_FIRM_ZIP_PATH, "wb");
                        if(fp)
                        {
                            fwrite(g_xSS.pbOtaData, iFSize, 1, fp);
                            fclose(fp);
                        }
                        umount_tmp();

                        SendGlobalMsg(MSG_SENSE, 0, OTA_RECV_DONE_OK, 0);
                        iMsgSent = 1;

                        my_free(msg);
                        continue;
                    }
#endif//darkhorse
                }
#endif
            }

            my_free(msg);
        }
        else if(msg->mid == MID_GETSTATUS)
        {
            my_printf("MID_GETSTATUS\n");
            if (g_xSS.iUsbHostMode == 0)
            {
                s_msg* reply_msg = Get_Reply_GetStatus(MR_SUCCESS, g_xSS.iMState);
                Send_Msg(reply_msg);
                my_free(msg);
            }
            else
            {
                s_msg* reply_msg = Get_Reply_GetStatus(MR_SUCCESS, MS_OTA);
                Send_Msg(reply_msg);
                my_free(msg);
            }
        }
        else if(msg->mid == MID_RESET || msg->mid == MID_FACERESET || msg->mid == MID_DELALL || msg->mid == MID_GET_UID)
        {
            my_printf(msg->mid == MID_RESET ? "MID_RESET_recv\n" : "MID_FACERESET_recv\n");
            MarkSenseResetFlag();
            SendGlobalMsg(MSG_SENSE, (long)msg, 0, 0);
#ifdef NOTHREAD_MUL
            iMsgSent = 1;
#endif // NOTHREAD_MUL
        }
        else if(msg->mid == MID_GET_UID)
        {
            dbug_printf("MID_GET_UID\n");
            s_msg* reply_msg = SenseLockTask::Get_Reply_GetUID(MR_SUCCESS);
            Send_Msg(reply_msg);
#ifdef NOTHREAD_MUL
            iMsgSent = 1;
#endif // NOTHREAD_MUL
        }
        else if(msg->mid == MID_ENROLL)
        {
            g_xSS.iUVCpause = 0;
            if(g_xSS.rFaceEngineTime == 0)
            {
                //my_printf("******* got ENROLL, sent\n");
                SendGlobalMsg(MSG_SENSE, (long)msg, 0, 0);
#ifdef NOTHREAD_MUL
                iMsgSent = 1;
#endif // NOTHREAD_MUL
            }
            else if(iLastCmd == MID_VERIFY)
            {
                MarkSenseResetFlag();
                SendGlobalMsg(MSG_SENSE, (long)msg, 0, 0);
            }
            else
            {
                //my_printf("******* got ENROLL, skip\n");
            }
        }
        else if(msg->mid == MID_VERIFY)
        {
            g_xSS.iUVCpause = 0;
            if(g_xSS.rFaceEngineTime == 0)
            {
                //my_printf("******* got ENROLL, sent\n");
                SendGlobalMsg(MSG_SENSE, (long)msg, 0, 0);
#ifdef NOTHREAD_MUL
                iMsgSent = 1;
#endif // NOTHREAD_MUL
            }
            else
            {
                //my_printf("******* got ENROLL, skip\n");
            }
        }
        else if(msg->mid == MID_POWERDOWN || msg->mid == MID_POWERDOWN_ED)
        {
            MarkSenseResetFlag();
            SendGlobalMsg(MSG_SENSE, (long)msg, 0, 0);
#ifdef NOTHREAD_MUL
            iMsgSent = 1;
#endif // NOTHREAD_MUL
        }
#if 0
        else if(msg->mid == MID_VIDEO_ON)
        {
            if(g_xSS.iVDBStart > 0)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_VIDEO_ON, MR_SUCCESS);
                Send_Msg(reply_msg);
            }

            StartVDB();
        }
        else if(msg->mid == MID_VIDEO_OFF)
        {
            StopVDB();

            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_VIDEO_OFF, MR_SUCCESS);
            Send_Msg(reply_msg);
        }
#endif
        else
        {
            SendGlobalMsg(MSG_SENSE, (long)msg, 0, 0);
#ifdef NOTHREAD_MUL
            iMsgSent = 1;
#endif // NOTHREAD_MUL
        }
        iLastCmd = iCurCmd;
    }
#ifndef NOTHREAD_MUL
    MSG* pxMsg = (MSG*)message_queue_message_alloc(&g_queue_send);
    memset(pxMsg, 0, sizeof(MSG));
    message_queue_write(&g_queue_send, pxMsg);

    my_thread_join(&g_thread_send);
    message_queue_destroy(&g_queue_send);
    g_thread_send = NULL;
#endif // ! NOTHREAD_MUL
}

s_msg* SenseLockTask::Get_Reply_Init_Encryption_Data(int iResult)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_init_encryption_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    if (msg == NULL)
        return NULL;
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_INIT_ENCRYPTION;
    msg_reply_data->result = iResult;

    int szLen = 64;
    char* szSN = (char*)my_malloc(szLen);
    memset(szSN, 0, szLen);
    GetSerialNumber(szSN);

    s_msg_reply_init_encryption_data* msg_reply_init_encryption_data =
            (s_msg_reply_init_encryption_data*)(msg_reply_data->data);

    memcpy(msg_reply_init_encryption_data->device_id, szSN, strlen(szSN));
    my_free(szSN);

    return msg;
}

s_msg* SenseLockTask::Get_Reply_PowerDown()
{
    int iMsgDataLen = sizeof(s_msg_reply_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_POWERDOWN;
    msg_reply_data->result = MR_SUCCESS;

    return msg;
}

s_msg* SenseLockTask::Get_Reply_Enroll(int iResult, int iUserID, int iFaceDirection, int iCmd)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_enroll_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    if (iCmd == -1)
        msg_reply_data->mid = MID_ENROLL;
    else
        msg_reply_data->mid = iCmd;
    msg_reply_data->result = iResult;

    s_msg_reply_enroll_data* msg_reply_enroll_data =
            (s_msg_reply_enroll_data*)(msg_reply_data->data);

    msg_reply_enroll_data->user_id_heb = HIGH_BYTE(iUserID);
    msg_reply_enroll_data->user_id_leb = LOW_BYTE(iUserID);
    msg_reply_enroll_data->face_direction = iFaceDirection;
#if (USE_UVC_PAUSE_MODE)
    if (iUserID < 0)
    {
#if (FRM_PRODUCT_TYPE >= FRM_PT_DEFAULT_3_3)
        PauseUVC_Time = Now() + 2000;
        g_xSS.iUVCpause = 2;
#else // FRM_PRODUCT_TYPE
        PauseUVC_Time = Now();
        g_xSS.iUVCpause = 2;
#endif // FRM_PRODUCT_TYPE
    }
    else
    	g_xSS.iUVCpause = 0;
#endif//USE_UVC_PAUSE_MODE
    return msg;
}

s_msg* SenseLockTask::Get_Reply_Verify(int iResult, int iUserID, int iUnlockState)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_verify_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_VERIFY;
    msg_reply_data->result = iResult;

    s_msg_reply_verify_data* msg_reply_verify_data =
            (s_msg_reply_verify_data*)(msg_reply_data->data);

    msg_reply_verify_data->user_id_heb = HIGH_BYTE(iUserID);
    msg_reply_verify_data->user_id_leb = LOW_BYTE(iUserID);

#if (USE_SANJIANG3_MODE && ENROLL_FACE_HAND_MODE == ENROLL_FACE_HAND_MIX && N_MAX_HAND_NUM)
        if (SenseLockTask::m_encMode == SenseLockTask::EM_XOR && g_xSS.iProtoMode == 1 && iUserID > N_MAX_PERSON_NUM)
        {
            msg_reply_verify_data->user_id_heb = HIGH_BYTE(iUserID - N_MAX_PERSON_NUM);
            msg_reply_verify_data->user_id_leb = LOW_BYTE(iUserID - N_MAX_PERSON_NUM);
        }
#endif // USE_SANJIANG3_MODE

    int iID = iUserID - 1;
    PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iID);
#if (N_MAX_HAND_NUM)
        if (iUserID > N_MAX_PERSON_NUM)
            pxMetaInfo = dbm_GetHandMetaInfoByID(iID - N_MAX_PERSON_NUM);
#endif // N_MAX_HAND_NUM
    if(pxMetaInfo)
    {
        strcpy((char*)msg_reply_verify_data->user_name, pxMetaInfo->szName);
        msg_reply_verify_data->admin = pxMetaInfo->fPrivilege;
    }
    msg_reply_verify_data->unlockStatus = iUnlockState;

    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetUserInfo(int iResult, int iUserID)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_getuserinfo_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_GETUSERINFO;
    msg_reply_data->result = iResult;

    s_msg_reply_getuserinfo_data* msg_reply_getuserinfo_data =
            (s_msg_reply_getuserinfo_data*)(msg_reply_data->data);

    msg_reply_getuserinfo_data->user_id_heb = HIGH_BYTE(iUserID);
    msg_reply_getuserinfo_data->user_id_leb = LOW_BYTE(iUserID);

    int iID = iUserID - 1;
    PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iID);
#if (N_MAX_HAND_NUM)
    if (iUserID > N_MAX_PERSON_NUM)
        pxMetaInfo = dbm_GetHandMetaInfoByID(iID - N_MAX_PERSON_NUM);
#endif // N_MAX_HAND_NUM
    if(pxMetaInfo)
    {
        strcpy((char*)msg_reply_getuserinfo_data->user_name, pxMetaInfo->szName);
        msg_reply_getuserinfo_data->admin = pxMetaInfo->fPrivilege;
    }

    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetAllUserID(int iResult, int iFmt)
{
    s_msg* msg = NULL;
    {
        if (iFmt == SM_USERID_DATA_FMT_DEFAULT)
        {
            int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_all_userid_data);
            int iMsgLen = sizeof(s_msg) + iMsgDataLen;
            msg = (s_msg*)my_malloc(iMsgLen);
            memset(msg, 0, iMsgLen);

            msg->mid = MID_REPLY;
            msg->size_heb = HIGH_BYTE(iMsgDataLen);
            msg->size_leb = LOW_BYTE(iMsgDataLen);

            s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
            msg_reply_data->mid = MID_GET_ALL_USERID;
            msg_reply_data->result = iResult;

            s_msg_reply_all_userid_data* msg_reply_all_userid_data =
                    (s_msg_reply_all_userid_data*)(msg_reply_data->data);

            msg_reply_all_userid_data->user_counts = dbm_GetPersonCount();
            for(int i = 0; i < dbm_GetPersonCount(); i ++)
            {
                PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByIndex(i);
                if(pxMetaInfo == NULL)
                    continue;

                msg_reply_all_userid_data->users_id[i * 2] = HIGH_BYTE(pxMetaInfo->iID + 1);
                msg_reply_all_userid_data->users_id[i * 2 + 1] = LOW_BYTE(pxMetaInfo->iID + 1);
            }
        }
        else if (iFmt == SM_USERID_DATA_FMT_BIT1)
        {
            int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_all_userid_data_fmt1);
            int iMsgLen = sizeof(s_msg) + iMsgDataLen;
            msg = (s_msg*)my_malloc(iMsgLen);
            memset(msg, 0, iMsgLen);

            msg->mid = MID_REPLY;
            msg->size_heb = HIGH_BYTE(iMsgDataLen);
            msg->size_leb = LOW_BYTE(iMsgDataLen);

            s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
            msg_reply_data->mid = MID_GET_ALL_USERID;
            msg_reply_data->result = iResult;

            s_msg_reply_all_userid_data_fmt1* msg_reply_all_userid_data =
                    (s_msg_reply_all_userid_data_fmt1*)(msg_reply_data->data);

            msg_reply_all_userid_data->user_counts = dbm_GetPersonCount();
            for(int i = 0; i < dbm_GetPersonCount(); i ++)
            {
                PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByIndex(i);
                if(pxMetaInfo == NULL)
                    continue;

                int idx = pxMetaInfo->iID / 8;
                int shift = pxMetaInfo->iID % 8;
                msg_reply_all_userid_data->users_id[idx] |= 1 << shift;
            }
        }
        else if (iFmt == SM_USERID_DATA_FMT_BIT_EXT)
        {
            int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_all_userid_data_fmt_ext);
            int iMsgLen = sizeof(s_msg) + iMsgDataLen;
            msg = (s_msg*)my_malloc(iMsgLen);
            memset(msg, 0, iMsgLen);

            msg->mid = MID_REPLY;
            msg->size_heb = HIGH_BYTE(iMsgDataLen);
            msg->size_leb = LOW_BYTE(iMsgDataLen);

            s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
            msg_reply_data->mid = MID_GET_ALL_USERID;
            msg_reply_data->result = iResult;

            s_msg_reply_all_userid_data_fmt_ext* msg_reply_all_userid_data =
                    (s_msg_reply_all_userid_data_fmt_ext*)(msg_reply_data->data);

            msg_reply_all_userid_data->magic = 0xFF;
            msg_reply_all_userid_data->max_user_counts = N_MAX_PERSON_NUM;
            msg_reply_all_userid_data->user_counts = dbm_GetPersonCount();
            for(int i = 0; i < dbm_GetPersonCount(); i ++)
            {
                PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByIndex(i);
                if(pxMetaInfo == NULL)
                    continue;

                int idx = pxMetaInfo->iID / 8;
                int shift = pxMetaInfo->iID % 8;
                msg_reply_all_userid_data->users_id[idx] |= 1 << shift;
            }
        }
#if (N_MAX_HAND_NUM)
#ifndef _NO_ENGINE_
        else if (iFmt == SM_USERID_DATA_FMT_HAND1)
        {
            int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_all_userid_data_fmt1);
            int iMsgLen = sizeof(s_msg) + iMsgDataLen;
            msg = (s_msg*)my_malloc(iMsgLen);
            memset(msg, 0, iMsgLen);

            msg->mid = MID_REPLY;
            msg->size_heb = HIGH_BYTE(iMsgDataLen);
            msg->size_leb = LOW_BYTE(iMsgDataLen);

            s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
            msg_reply_data->mid = MID_GET_ALL_USERID;
            msg_reply_data->result = iResult;

            s_msg_reply_all_userid_data_fmt1* msg_reply_all_userid_data =
                    (s_msg_reply_all_userid_data_fmt1*)(msg_reply_data->data);

            msg_reply_all_userid_data->user_counts = dbm_GetHandCount();
            for(int i = 0; i < msg_reply_all_userid_data->user_counts; i ++)
            {
                PSMetaInfo pxMetaInfo = dbm_GetHandMetaInfoByIndex(i);
                if(pxMetaInfo == NULL)
                    continue;

                int idx = pxMetaInfo->iID / 8;
                int shift = pxMetaInfo->iID % 8;
                msg_reply_all_userid_data->users_id[idx] |= 1 << shift;
            }
        }
#endif // !_NO_ENGINE_
#endif // N_MAX_HAND_NUM

    }

    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetVersion(int iResult)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_version_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_GET_VERSION;
    msg_reply_data->result = iResult;

    s_msg_reply_version_data* msg_reply_version_data =
            (s_msg_reply_version_data*)(msg_reply_data->data);
#ifdef DEVICE_TYPE_NUM
    strcpy((char*)msg_reply_version_data->version_info, DEVICE_TYPE_NUM);
    strcpy((char*)msg_reply_version_data->version_info + 8, DEVICE_FIRMWARE_VERSION);
#else // DEVICE_TYPE_NUM
    strcpy((char*)msg_reply_version_data->version_info, DEVICE_FIRMWARE_VERSION);
#if (DESMAN_ENC_MODE != 0)
    //for qixin only, version length must be less or equal than 12.
    strcpy((char*)msg_reply_version_data->version_info + 12, DEVICE_FIRMWARE_VERSION);
#endif // DESMAN_ENC_MODE
#endif // DEVICE_TYPE_NUM
    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetUID(int iResult)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_uid_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_GET_UID;
    msg_reply_data->result = iResult;

    s_msg_reply_uid_data* msg_reply_uid_data =
            (s_msg_reply_uid_data*)(msg_reply_data->data);

    char szSN[UID_INFO_BUFFER_SIZE] = {0};
    GetSerialNumber(szSN);
    memcpy((char*)msg_reply_uid_data->uid_info, szSN, UID_INFO_BUFFER_SIZE);
    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetStatus(int iResult, int iStatus)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_getstatus_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_GETSTATUS;
    msg_reply_data->result = iResult;

    s_msg_reply_getstatus_data* msg_reply_getstatus_data =
            (s_msg_reply_getstatus_data*)(msg_reply_data->data);

    msg_reply_getstatus_data->status = iStatus;

    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetSavedImage(int iResult, int iImgLen)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_get_saved_image_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_GETSAVEDIMAGE;
    msg_reply_data->result = iResult;

    s_msg_reply_get_saved_image_data* msg_reply_get_saved_image_data =
            (s_msg_reply_get_saved_image_data*)(msg_reply_data->data);

    msg_reply_get_saved_image_data->image_size[0] = (iImgLen >> 24) & 0xFF;
    msg_reply_get_saved_image_data->image_size[1] = (iImgLen >> 16) & 0xFF;
    msg_reply_get_saved_image_data->image_size[2] = (iImgLen >> 8) & 0xFF;
    msg_reply_get_saved_image_data->image_size[3] = (iImgLen) & 0xFF;

    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetLogFile(int iResult, int iImgLen)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_get_saved_image_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_GET_LOGFILE;
    msg_reply_data->result = iResult;

    s_msg_reply_get_saved_image_data* msg_reply_get_saved_image_data =
            (s_msg_reply_get_saved_image_data*)(msg_reply_data->data);

    msg_reply_get_saved_image_data->image_size[0] = (iImgLen >> 24) & 0xFF;
    msg_reply_get_saved_image_data->image_size[1] = (iImgLen >> 16) & 0xFF;
    msg_reply_get_saved_image_data->image_size[2] = (iImgLen >> 8) & 0xFF;
    msg_reply_get_saved_image_data->image_size[3] = (iImgLen) & 0xFF;

    return msg;
}

s_msg* SenseLockTask::Get_Reply_GetOtaStatus(int iStatus, int iPID)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + sizeof(s_msg_reply_getotastatus_data);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = MID_GET_OTA_STATUS;
    msg_reply_data->result = MR_SUCCESS;

    s_msg_reply_getotastatus_data* msg_reply_getotastatus_data =
            (s_msg_reply_getotastatus_data*)(msg_reply_data->data);

    msg_reply_getotastatus_data->ota_status = iStatus;
    msg_reply_getotastatus_data->next_pid_e[0] = (iPID >> 8) & 0xFF;
    msg_reply_getotastatus_data->next_pid_e[1] = (iPID) & 0xFF;

    return msg;
}


s_msg* SenseLockTask::Get_Reply(int iMID, int iResult, unsigned char* pParam, int paramLen)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + paramLen;
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    if (msg == NULL)
    {
        my_printf("[%s] failed to malloc msg.\n", __func__);
        return NULL;
    }
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = iMID;
    msg_reply_data->result = iResult;
    if (pParam != NULL && paramLen > 0)
        memcpy(msg_reply_data->data, pParam, paramLen);

    return msg;
}

s_msg* SenseLockTask::Get_Reply_CamError(int iMID, int iResult, int iCamError)
{
    int iMsgDataLen = sizeof(s_msg_reply_data) + 1;
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_REPLY;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_reply_data* msg_reply_data = (s_msg_reply_data*)(msg->data);
    msg_reply_data->mid = iMID;
    msg_reply_data->result = iResult;

    unsigned char* pbData = (unsigned char*)msg_reply_data->data;
    pbData[0] = iCamError;

    return msg;
}

s_msg* SenseLockTask::Get_Note(int iNID)
{
    s_msg* msg = NULL;
    int iMsgDataLen = sizeof(s_msg_note_data);
#if (USE_READY0_PROTO == 0)
    if (iNID == NID_READY)
        iMsgDataLen += 1;
#endif // USE_READY0_PROTO
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_NOTE;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_note_data* msg_note_data = (s_msg_note_data*)(msg->data);

    if (iNID == NID_READY)
        msg_note_data->nid = DEVICE_NID_READY_VER;
    else
        msg_note_data->nid = iNID;
#if (USE_READY0_PROTO == 0)
    if (iNID == NID_READY)
    {
#if (DEFAULT_UART0_BAUDRATE + 1 > 0xf)
#error "DEFAULT_UART0_BAUDRATE + 1 must less than 0xf"
#endif
        if (DEFAULT_UART0_BAUDRATE == Baud_Rate_115200)
            msg_note_data->data[0] = 1;
        else
            msg_note_data->data[0] = DEFAULT_UART0_BAUDRATE + 1;
    }
#if (DEFAULT_PROTO_ENC_MODE > 0xf)
#error "DEFAULT_PROTO_ENC_MODE must be less than 0xf"
#endif
    msg_note_data->data[0] |= (DEFAULT_PROTO_ENC_MODE < PROTO_EM_ENCRYPT_AES_DEFAULT) ? 0 : (DEFAULT_PROTO_ENC_MODE << 4);
#endif // USE_READY0_PROTO


#ifdef USE_UVC_PAUSE_MODE
    PauseUVC_Time = Now() - (UVC_PAUSE_LIMIT_TIME - 200);
    g_xSS.iUVCpause = 2;
#endif // USE_UVC_PAUSE_MODE
    return msg;
}

s_msg* SenseLockTask::Get_Note_FaceState(int iNID)
{
    int iMsgDataLen = sizeof(s_msg_note_data) + sizeof(s_note_data_face);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_NOTE;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_note_data* msg_note_data = (s_msg_note_data*)((unsigned char*)msg + (sizeof(s_msg)));
    msg_note_data->nid = iNID;

    s_note_data_face* note_data_face =
            (s_note_data_face*)(msg_note_data->data);

    memcpy(note_data_face, &g_xSS.note_data_face, sizeof(s_note_data_face));

    return msg;
}

s_msg* SenseLockTask::Get_Note_OtaDone(int iOk)
{
    int iMsgDataLen = sizeof(s_msg_note_data) + 1;
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_NOTE;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_note_data* msg_note_data = (s_msg_note_data*)(msg->data);
    msg_note_data->nid = NID_OTA_DONE;
    msg_note_data->data[0] = iOk;

    return msg;
}

s_msg* SenseLockTask::Get_Note_EyeState(int iEyeState)
{
    int iMsgDataLen = sizeof(s_msg_note_data) + sizeof(s_note_data_eye);
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_NOTE;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    s_msg_note_data* msg_note_data = (s_msg_note_data*)((unsigned char*)msg + (sizeof(s_msg)));
    msg_note_data->nid = NID_EYE_STATE;

    s_note_data_eye* note_data_eye =
            (s_note_data_eye*)(msg_note_data->data);

    note_data_eye->eye_state = iEyeState;
    return msg;
}

s_msg* SenseLockTask::Get_Image(unsigned char* pbImage, int iImgLen)
{
    int iMsgDataLen = iImgLen;
    int iMsgLen = sizeof(s_msg) + iMsgDataLen;
    s_msg* msg = (s_msg*)my_malloc(iMsgLen);
    memset(msg, 0, iMsgLen);

    msg->mid = MID_IMAGE;
    msg->size_heb = HIGH_BYTE(iMsgDataLen);
    msg->size_leb = LOW_BYTE(iMsgDataLen);

    unsigned char* upload_image_data =
            (unsigned char*)(msg->data);

    memcpy(upload_image_data, pbImage, iImgLen);
    return msg;
}

void SenseLockTask::Send_Msg(s_msg* msg)
{
    if(msg == NULL)
        return;

    MSG* pxMsg = (MSG*)message_queue_message_alloc(&g_queue_send);
    if (pxMsg == NULL)
    {
        my_printf("[%s] failed to malloc msg.\n", __func__);
        return;
    }
    memset(pxMsg, 0, sizeof(MSG));
    pxMsg->data1 = (long)msg;

    message_queue_write(&g_queue_send, pxMsg);
#ifdef NOTHREAD_MUL
    senseSendThread_ThreadProc1(NULL);
#endif // NOTHREAD_MUL
}

int SenseLockTask::Set_KeyPos(unsigned char* pbKeyPos)
{
    if (pbKeyPos == NULL)
        return -1;
    if (pbKeyPos[0] == 0xff)
    {
        //key pos is not set
#if (DESMAN_ENC_MODE == 0)
        unsigned char dessman_base_code[] = {
            0x06, 0x12, 0x07, 0x03,
            0x0D, 0x0D, 0x17, 0x04,
            0x08, 0x01, 0x00, 0x19,
            0x09, 0x02, 0x02, 0x07
        };
        memcpy(m_encKeyPos, dessman_base_code, ENC_KEY_SIZE);
#endif // DESMAN_ENC_MODE
#if (DEFAULT_PROTO_ENC_KEY_ORD == DEFAULT_PROTO_ENC_KEY_1)
        unsigned char dessman_base_code[] = {
            0x00, 0x01, 0x02, 0x03,
            0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B,
            0x0C, 0x0D, 0x0E, 0x0F
        };
        memcpy(m_encKeyPos, dessman_base_code, ENC_KEY_SIZE);
#endif // DEFAULT_PROTO_ENC_KEY_ORD
    }
    else
    {
        for(int i = 0; i < ENC_KEY_SIZE; i ++)
        {
            if (pbKeyPos[i] >= 32) //must not exceed 32
                return -1;
        }
        memcpy(m_encKeyPos, pbKeyPos, ENC_KEY_SIZE);
    }
    return 0;
}

int SenseLockTask::Set_Key(unsigned char* pbSeed, int iSize, int iMode, char* strEncKey)
{
    if (iSize != 4)
    {
        my_printf("Incorrect input seed size");
        return -1;
    }

    if (iMode < EM_NOENCRYPT || iMode >= EM_END)
    {
        //invalid encryption mode
        return -2;
    }

    if (m_encKeyPos[0] != 0xff)
    {
        char md5str[256] = { 0 };
        unsigned char abMd5[16] = { 0 };
        md5((uint8_t*)pbSeed, iSize, abMd5);
        for(unsigned int i = 0; i < sizeof(abMd5); i ++)
        {
            char szTmp[10] = { 0 };
            sprintf(szTmp, "%02x", abMd5[i]);
            strcat(md5str, szTmp);
        }

        // specical code for DSM
#if (DESMAN_ENC_MODE == 1)
        snprintf(SenseLockTask::EncKey, ENC_KEY_SIZE + 1, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                 md5str[0], md5str[1], md5str[2], md5str[3],
                md5str[4], md5str[5], md5str[6], md5str[7],
                md5str[8], md5str[9], md5str[10], md5str[11],
                md5str[12], md5str[13], md5str[14], md5str[15]);
#else // DESMAN_ENC_MODE
        memset(SenseLockTask::EncKey, 0, sizeof(SenseLockTask::EncKey));
        for (int i = 0; i < ENC_KEY_SIZE; i++)
            SenseLockTask::EncKey[i] = md5str[m_encKeyPos[i]];
#endif // DESMAN_ENC_MODE
        switch(DEFAULT_PROTO_ENC_MODE)
        {
        case PROTO_EM_NOENCRYPT:
            SenseLockTask::m_encMode = SenseLockTask::EM_NOENCRYPT;
            break;
        case PROTO_EM_ENCRYPT:
            SenseLockTask::m_encMode = (EEncMode)iMode;
            break;
        case PROTO_EM_ENCRYPT_XOR_LANHENG:
            SenseLockTask::m_encMode = (EEncMode)iMode;
            if (strEncKey == NULL)
                strcpy(SenseLockTask::EncKey, PROTO_EM_XOR1_KEY_LANHENG);
            else
                strcpy(SenseLockTask::EncKey, strEncKey);
            break;
        case PROTO_EM_ENCRYPT_AES_XLAN:
            SenseLockTask::m_encMode = (EEncMode)iMode;
            if (iMode == SenseLockTask::EM_XOR)
                strcpy(SenseLockTask::EncKey, DEFAULT_PROTO_EM_XOR1_KEY);
            break;
        default:
            SenseLockTask::m_encMode = (EEncMode)iMode;
            break;
        }
    }

    return 0;
}


unsigned char SenseLockTask::Get_CheckSum(unsigned char* pbData, int iLen)
{
    unsigned char bCheckSum = 0;
    for(int i = 0; i < iLen; i ++)
        bCheckSum ^= pbData[i];

    return bCheckSum;
}

int SenseLockTask::Encrypt_Msg(unsigned char* pbSrc, int iSrcLen, unsigned char** pbOut, int* pOutLen)
{
    if(pbSrc == NULL || pbOut == NULL || iSrcLen <= 0)
        return -1;
    if(m_encMode == EM_AES)
    {
        if (pOutLen == NULL)
            return -2;
        AES_Encrypt((unsigned char*)EncKey, (unsigned char*)pbSrc, iSrcLen, pbOut, pOutLen);
    }
    else if(m_encMode == EM_XOR)
    {
        Encrypt_Msg_Xor(pbSrc, iSrcLen, *pbOut);
    }
    return 0;
}

int SenseLockTask::Decrypt_Msg(unsigned char* pbSrc, int iSrcLen, unsigned char** pbOut, int* pOutLen)
{
    if(pbSrc == NULL || pbOut == NULL || iSrcLen <= 0)
        return -1;
    if(m_encMode == EM_AES)
    {
        if (pOutLen == NULL)
            return -2;
        AES_Decrypt((unsigned char*)EncKey, (unsigned char*)pbSrc, iSrcLen, pbOut, pOutLen);
    }
    else if(m_encMode == EM_XOR)
    {
        Encrypt_Msg_Xor(pbSrc, iSrcLen, *pbOut);
    }
    return 0;
}

int SenseLockTask::Encrypt_Msg_Xor(unsigned char* pbSrc, int iSrcLen, unsigned char* pbOut)
{
    int iKeyLen = ENC_KEY_SIZE;
    for(int i = 0; i < iSrcLen; i ++)
    {
        pbOut[i] = ~(EncKey[i % iKeyLen] ^ (pbSrc[i]));
    }
    return 0;
}

int SenseLockTask::Get_MsgLen(s_msg* msg)
{
    if(msg == NULL)
        return 0;

    return TO_SHORT(msg->size_heb, msg->size_leb);
}

void SenseLockTask::ThreadProc()
{
    run();
}

void* senseLockTask_ThreadProc1(void* param)
{
    SenseLockTask* pThread = (SenseLockTask*)(param);
    pThread->ThreadProc();
    return NULL;
}
