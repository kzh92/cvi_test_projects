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

    static s_msg*       Get_Reply_Init_Encryption_Data(int iResult);
    static s_msg*       Get_Reply_PowerDown();
    static s_msg*       Get_Reply_Enroll(s_msg* pSenseMsg, int iResult, int iUserID, int iFaceDirection, int iCmd = -1, unsigned char* pbExtData = NULL, int iExtDataLen = 0);
    static s_msg*       Get_Reply_Verify(s_msg* pSenseMsg, int iResult, int iUserID, int iUnlockState, int iAuthType = 0);
    static s_msg*       Get_Reply_GetUserInfo(s_msg* pSenseMsg, int iResult, int iUserID);
    static s_msg*       Get_Reply_GetAllUserID(s_msg* pSenseMsg, int iResult, int iFmt);
    static s_msg*       Get_Reply_MxGetAllUserID(s_msg* pSenseMsg, int iResult, int iFmt);
    static s_msg*       Get_Reply_TransFilePacket(s_msg* pSenseMsg, int iResult, int iUserCount, uint16_t* pUserIds);
    static s_msg*       Get_Reply_GetVersion(s_msg* pSenseMsg, int iResult);
    static s_msg*       Get_Reply_GetUID(s_msg* pSenseMsg, int iResult);
    static s_msg*       Get_Reply_GetSN(s_msg* pSenseMsg, int iResult);
    static s_msg*       Get_Reply_GetStatus(int iResult, int iStatus);
    static s_msg*       Get_Reply_GetSavedImage(s_msg* pSenseMsg, int iResult, int iImgLen);
    static s_msg*       Get_Reply_GetLogFile(s_msg* pSenseMsg, int iResult, int iImgLen, int iCmd = MID_GET_LOGFILE);
    static s_msg*       Get_Reply_GetOtaStatus(int iStatus, int iPID);
    static s_msg*       Get_Reply(s_msg* pSenseMsg, int iMID, int iResult, unsigned char* pParam = NULL, int paramLen = 0);
    static s_msg*       Get_Reply_CamError(s_msg* pSenseMsg, int iMID, int iResult, int iCamError);

    static s_msg*       Get_Note(int iNID);
    static s_msg*       Get_Note_FaceState(s_msg* pSenseMsg, int iNID);
    static s_msg*       Get_Note_OtaDone(int iOk);
    static s_msg*       Get_Note_EyeState(s_msg* pSenseMsg, int iEyeState);

    static s_msg*       Get_Image(unsigned char* pbImage, int iImgLen);

    static int          Get_MsgLen(s_msg* msg);
    static int          Get_DataLen(s_msg* msg);
    static int          Set_Key(unsigned char* pbSeed, int iSize, int iMode, char* strEncKey);
    static int          Set_KeyPos(unsigned char* pbKeyPos);
    static unsigned char    Get_CheckSum(unsigned char* pbData, int iLen);
    static int          Encrypt_Msg(unsigned char* pbSrc, int iSrcLen, unsigned char** pbOut, int* pOutLen = NULL);
    static int          Decrypt_Msg(unsigned char* pbSrc, int iSrcLen, unsigned char** pbOut, int* pOutLen = NULL);
    static int          SetBaudrate(int index);

    void                Send_Msg(s_msg* msg);
    void                SetActive(int a) {m_rActive = a ? Now() : 0;}
    int                 SendReady();

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

    float               m_rActive;
    float               m_rRecvCmdTime;
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
