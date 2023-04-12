#ifndef SENSE_LOCK_TASK_H
#define SENSE_LOCK_TASK_H

#include "senselockmessage.h"
#include "mutex.h"
#include "thread.h"

//#include <pthread.h>

#define TO_SHORT(a, b)  (((a) << 8) | (b))
#define TO_INT(a) ((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3])

#define HIGH_BYTE(a)    (((a) >> 8) & 0xFF)
#define LOW_BYTE(a)     ((a) & 0xFF)
#define ENC_KEY_SIZE    16
#define SENSE_BASE_MSG  (1000)
#define MAX_OTA_FSIZE   (20 * 1024 * 1024)
#define MAX_OTA_PCK_SIZE    (4000)

#if (NOTE_INTERVAL_MS)
#define NOTE2REPLY_MS       (NOTE_INTERVAL_MS + 5)
#define DELAY4REPLY(t) \
    do { \
        float _rTime = Now(); \
        if (_rTime - t < NOTE2REPLY_MS) \
            my_usleep((NOTE2REPLY_MS - (_rTime - t)) * 1000); \
    }while(0)
#endif // NOTE_INTERVAL_MS

#define GET_SENSE_MSG_LEN(packet_len, data, ret_len) \
    do { \
        ret_len = sizeof(data); \
        memset(&(data), 0, ret_len); \
        if ((packet_len) < ret_len) \
            ret_len = packet_len; \
    } while(0)

class SenseLockTask : public Thread
{
public:
    SenseLockTask();
    ~SenseLockTask();
    void ThreadProc();
    void Init();
    void Deinit();

    int     Start();
    int     Stop();
    void    Wait();
    int     doProcess();

    typedef enum {
        EM_NOENCRYPT,
        EM_AES,
        EM_XOR,
        EM_END
    } EEncMode;
    static EEncMode         m_encMode;
    static char             EncKey[ENC_KEY_SIZE + 1];
    static unsigned char    m_encKeyPos[ENC_KEY_SIZE];
    static mymutex_ptr            CommMutex;

#if 0
    int     SendCmd(int iType, int iP1, int iP2, int iP3);
    int     SendData(int iType, unsigned char* pbData, int iLen);

    static void             SendAck(int iType, int iSeqNum, int iQ1, int iQ2, int iQ3);
    static void             SendData(int iType, int iSeqNum, int iAck, int iDataLen, unsigned char* pbData);
    static unsigned char    GetCheckSum(FM_CMD* pxCmd);
protected:
    void    run();

    FM_CMD              CommCmd(FM_CMD xCmd);
    FM_CMD              CommData(FM_CMD xCmd, unsigned char* pbData, int iLen);
    FM_CMD              ParseCmd(unsigned char* pbData, int iLen);
    unsigned char       GenSeq(int iFlag, unsigned char bSeqNum);


    int                 RecvCmd(FM_CMD* pxCmd);
    int                 RecvData(unsigned char* pbData, int iLen);
#endif

    static s_msg*       Get_Reply_Init_Encryption_Data(int iResult);
    static s_msg*       Get_Reply_PowerDown();
    static s_msg*       Get_Reply_Enroll(int iResult, int iUserID, int iFaceDirection, int iCmd = -1);
    static s_msg*       Get_Reply_Verify(int iResult, int iUserID, int iUnlockState);
    static s_msg*       Get_Reply_GetUserInfo(int iResult, int iUserID);
    static s_msg*       Get_Reply_GetAllUserID(int iResult, int iFmt);
    static s_msg*       Get_Reply_GetVersion(int iResult);
    static s_msg*       Get_Reply_GetUID(int iResult);
    static s_msg*       Get_Reply_GetStatus(int iResult, int iStatus);
    static s_msg*       Get_Reply_GetSavedImage(int iResult, int iImgLen);
    static s_msg*       Get_Reply_GetLogFile(int iResult, int iImgLen);
    static s_msg*       Get_Reply_GetOtaStatus(int iStatus, int iPID);
    static s_msg*       Get_Reply(int iMID, int iResult, unsigned char* pParam = NULL, int paramLen = 0);
    static s_msg*       Get_Reply_CamError(int iMID, int iResult, int iCamError);

    static s_msg*       Get_Note(int iNID);
    static s_msg*       Get_Note_FaceState(int iNID);
    static s_msg*       Get_Note_OtaDone(int iOk);
    static s_msg*       Get_Note_EyeState(int iEyeState);

    static s_msg*       Get_Image(unsigned char* pbImage, int iImgLen);

    static int          Get_MsgLen(s_msg* msg);
    static int          Set_Key(unsigned char* pbSeed, int iSize, int iMode, char* strEncKey);
    static int          Set_KeyPos(unsigned char* pbKeyPos);
    static unsigned char    Get_CheckSum(unsigned char* pbData, int iLen);
    static int          Encrypt_Msg(unsigned char* pbSrc, int iSrcLen, unsigned char** pbOut, int* pOutLen = NULL);
    static int          Decrypt_Msg(unsigned char* pbSrc, int iSrcLen, unsigned char** pbOut, int* pOutLen = NULL);
    static int          SetBaudrate(int index);

    void                Send_Msg(s_msg* msg);
    void                SetActive(int a) {m_iActive = a;}

protected:
    static int          Encrypt_Msg_Xor(unsigned char* pbSrc, int iSrcLen, unsigned char* pbOut);
    void    run();

protected:
    int                 m_iStep;
    int                 m_iEnd;

#if 0
    static unsigned char    m_bSeqNum;
#endif

    int                 m_iMutex;
    int                 m_iComm;

    int                 m_iActive;
#if 0
    pthread_mutex_t     m_mutex;
    pthread_cond_t      m_cond;
    FM_CMD              m_xSendCmd;
    int                 m_iSendType;
    unsigned char*      m_pSendData;
    int                 m_iSendLen;
#endif
};

#endif // FACE_MOUDLE_PROC_H
