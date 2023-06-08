#include "fm_main.h"
//#include "camerasurface.h"
#include "settings.h"
#include "engineparam.h"
#include "FaceRetrievalSystem.h"
#include "shared.h"
#include "faceengine.h"
#include "uartcomm.h"
#include "drv_gpio.h"
#include "common_types.h"
#include "i2cbase.h"
#include "msg.h"
#include "senselocktask.h"
#include "facemoduletask.h"
#include "camerasurface.h"
#include "mount_fs.h"
#include "systembase.h"
#include "countbase.h"
#include "upgradebase.h"
#include "watchtask.h"
#include "common_types.h"
#include "facerecogtask.h"
#include "DBManager.h"
#include "KeyGenAPI.h"
#include "KeyGenBase.h"
#include "b64.h"
// #include "st_base.h"
#include "functestproc.h"
#include "FaceRetrievalSystem_base.h"
#include "sha1.h"
#include "desinterface.h"
#include "sn.h"
#include "check_firmware.h"
#include "upgrade_firmware.h"
#include "vdbtask.h"
#include "uvc_func.h"

#include <fcntl.h>
#include <string.h>

//using namespace std;

#define RET_CONTINUE -1
#define RET_POWEROFF 0
#define RET_REBOOT 1

#define RET_CANCEL 0
#define RET_OK 1

//shared
WatchTask  g_WatchTask;
WatchTask*  g_pWatchTask = &g_WatchTask;
FaceModuleTask g_FMTask;
FaceModuleTask* g_pFMTask = &g_FMTask;
SenseLockTask g_SenseTask;
SenseLockTask* g_pSenseTask = &g_SenseTask;

#if (USE_WIFI_MODULE)
MySpiThread g_MySpiThread;
MySpiThread* g_pMySpiThread = &g_MySpiThread;
#endif // USE_WIFI_MODULE

#if (USE_VDBTASK)
extern FaceRecogTask* g_pFaceRecogTask;
VDBTask g_VDBTask;
VDBTask* g_pVDBTask = &g_VDBTask;
extern int             g_iJpgDataLen;
#endif // USE_VDBTASK
extern mymutex_ptr g_FlashReadWriteLock;
extern mymutex_ptr g_MyPrintfLock;

unsigned char raw_msg_enroll_middle[] = {0x13, 0x00, 0x23, 0x00, 0x54, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x0F};
unsigned char raw_msg_enroll_right[] = {0x13, 0x00, 0x23, 0x00, 0x54, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x12};
unsigned char raw_msg_enroll_left[] = {0x13, 0x00, 0x23, 0x00, 0x54, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x06, 0x0C};
unsigned char raw_msg_enroll_down[] = {0x13, 0x00, 0x23, 0x00, 0x54, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x06, 0x08};
unsigned char raw_msg_enroll_up[] = {0x13, 0x00, 0x23, 0x00, 0x54, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x06, 0x20};
unsigned char raw_msg_verify[] = {0x12, 0x00, 0x02, 0x00, 0x08, 0x18};
#if (USE_AUTO_50_REPLY)
unsigned char raw_msg_init_enc[] = {0x50, 0x00, 0x05, 0x30, 0x31, 0x32, 0x33, 0x00, 0x55};
#endif

int GotoMain();
int MsgProcFM(MSG* pMsg);
int MsgProcSense(MSG* pMsg);
int MsgProcError(MSG* pMsg);
void EndIns();
void ResetFaceRegisterStates();

static int ProcessSenseFace(int iCmd);

void UART_Create();
void UART_Release();
int ProcessActivation(char* pbUID, int iUniqueID);
extern int fr_ReadAppLog(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
extern int fr_GetAppLogLen();
extern "C" void drv_reboot(void);
extern "C" int MEDIA_VIDEO_Deinit();

extern float g_rAppStartTime;

int g_iEnrollInit = 0;

mythread_ptr   g_thdInsmod = 0;
mymutex_ptr    g_xVDBMutex;
extern int g_iUniqueID;
extern unsigned char g_abKey[16];

void SystemReboot(void)
{
    drv_reboot();
}

void ResetPersonDB(int flag)
{
    SetModifyUser(1);

    int iBackupState = mount_backup_db(0);
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
    {
        end_restore_dbPart();
        iBackupState = 0;
    }

    if (flag == SM_DEL_ALL_TYPE_DEFAULT)
    {
        dbm_SetEmptyPersonDB(&iBackupState);
#if (N_MAX_HAND_NUM)
        dbm_SetEmptyHandDB(&iBackupState);
#endif // N_MAX_HAND_NUM
    }
    else if (flag == SM_DEL_ALL_TYPE_FACE)
    {
        dbm_SetEmptyPersonDB(&iBackupState);
    }
#if (N_MAX_HAND_NUM)
    else if (flag == SM_DEL_ALL_TYPE_HAND)
    {
        dbm_SetEmptyHandDB(&iBackupState);
    }
#endif // N_MAX_HAND_NUM

    umount_backup_db();
    UpdateUserCount();

    ResetFaceRegisterStates();
}

//reset all status flags when poweroff module.
void ResetFMStates()
{
    //reset register state
    FaceEngine::UnregisterFace(-1);
    g_xSS.iRegisterDir = 0;
    g_xSS.iRegisterID = 0;
#if (N_MAX_HAND_NUM)
    g_xSS.iRegisterHand = 0;
#endif

    g_xSS.iRegsterAuth = 0;

    g_xSS.iDemoMode = N_DEMO_VERIFY_MODE_OFF;
    g_xSS.iStartOta = 0;
    g_xSS.iAutoUserAdd = 0;
}

//reset all face register status flags when poweroff module.
void ResetFaceRegisterStates()
{
    //reset register state
    FaceEngine::UnregisterFace(-1);
    g_xSS.iRegisterDir = 0;
    g_xSS.iRegisterID = 0;
#if (N_MAX_HAND_NUM)
    g_xSS.iRegisterHand = 0;
#endif

    g_xSS.iRegsterAuth = 0;
    g_xSS.iAutoUserAdd = 0;
}

void DriverInit()
{
    g_pWatchTask->Init();
    g_pSenseTask->Init();
    g_pFMTask->Init();
    //gpio init
    GPIO_fast_init();
#ifdef IR_LED
    GPIO_fast_config(IR_LED, OUT);
    GPIO_fast_setvalue(IR_LED, OFF);
#endif

// #ifdef AUDIO_EN
//     GPIO_fast_config(AUDIO_EN, OUT);
//     GPIO_fast_setvalue(AUDIO_EN, OFF);
// #endif

// #ifdef M24C64_WP
//     GPIO_fast_config(M24C64_WP, OUT);
//     GPIO_fast_setvalue(M24C64_WP, OFF);
// #endif

// #ifdef PSENSE_DET
//     GPIO_fast_config(PSENSE_DET, IN);
// #endif // PSENSE_DET

// #ifdef GPIO_USBSense
//     GPIO_fast_config(GPIO_USBSense, IN);
// #endif

// #if (USE_WIFI_MODULE)
// #ifdef UART_EN
//     GPIO_fast_config(UART_EN, OUT);
//     GPIO_fast_setvalue(UART_EN, OFF);
// #endif
// #else // USE_WIFI_MODULE
// #ifdef UART_EN
//     GPIO_fast_config(UART_EN, OUT);
//     GPIO_fast_setvalue(UART_EN, ON);
// #endif
// #endif // USE_WIFI_MODULE

// #ifdef IOCtl
//     GPIO_fast_config(IOCtl, OUT);
//     GPIO_fast_setvalue(IOCtl, OFF);
// #endif

// #ifdef AUDIO_EN
//     GPIO_fast_config(AUDIO_EN, OUT);
//     GPIO_fast_setvalue(AUDIO_EN, OFF);
// #endif

// #ifdef SPI_CS
//     GPIO_fast_config(SPI_CS, OUT);
//     GPIO_fast_setvalue(SPI_CS, 1);
// #endif

    //CreateSharedMem();

    M24C64_Open();
    UART_Init();

    // ST_Base_Init();
}

void DriverRelease()
{
    // ST_Base_Exit();
    UART_Quit();
    M24C64_Close();

    GPIO_fast_deinit();
    g_pFMTask->Deinit();
    g_pWatchTask->Deinit();
    g_pSenseTask->Deinit();
    end_restore_dbPart();
}

void ReleaseAll()
{
    g_pWatchTask->Stop();

    UART_Release();
    message_queue_destroy(&g_worker);
}

void StartVDB()
{
#if (USE_VDBTASK)
    my_mutex_lock(g_xVDBMutex);
#ifdef MID_VIDEO_ON
    g_xSS.iVDBCmd = MID_VIDEO_ON;
#endif
    if(g_xSS.iVDBStart == 0)
    {
        g_xSS.iVDBStart = 1;
        if(g_pVDBTask == NULL)
            g_pVDBTask = &g_VDBTask;

        g_pVDBTask->Start();
    }
    else
        g_xSS.iVDBStart = 1;
    my_mutex_unlock(g_xVDBMutex);
#endif // USE_VDBTASK
}

void StopVDB()
{
#if (USE_VDBTASK)
    my_mutex_lock(g_xVDBMutex);
    if(g_xSS.iVDBStart)
    {
        g_pVDBTask->Stop();
        g_xSS.iVDBStart = 0;
    }
    my_mutex_unlock(g_xVDBMutex);
#endif // USE_VDBTASK
}

int processGlobalMsg();

/**
 * @brief GotoMain
 * @param iForceManager
 * @return
 *     caution: return value must be zero(power off) or RET_REBOOT
       be careful when you modify return value.
 */
int GotoMain()
{
    int iRet = 0;
    // int bOldUsbHostMode = g_xSS.iUsbHostMode;
    // if(g_xSS.iUsbHostMode)
    // {
    //     my_printf("[USB] insmod usb drivers\n");
    //     my_system("./insmod_usb.sh");
    // }
    ///////////////////////
    g_xSS.rMainLoopTime = Now();
    g_xSS.iMState = MS_STANDBY;
    // g_xSS.iTimeoutTimer = g_pWatchTask->AddTimer(1);
    g_xSS.bUVCRunning = 0;

#if DB_TEST
    g_xROKLog.x.bKernelFlag = 0xAA;//kzh
    UpdateROKLogs();
#endif

#if (USE_VDBTASK)
    StartVDB();
#endif // USE_VDBTASK
#if (USE_WIFI_MODULE)
    g_pMySpiThread->Init();
    g_pMySpiThread->Start();
#endif // USE_WIFI_MODULE
#ifndef NOTHREAD_MUL
    iRet = processGlobalMsg();
#else // ! NOTHREAD_MUL

    g_pSenseTask->SendReady();
    my_printf("Send Ready2: %f\n", Now());

    iRet = g_pSenseTask->doProcess();
#endif // ! NOTHREAD_MUL

    fr_EndThread();
#if (USE_VDBTASK)
    if(g_pVDBTask)
       g_pVDBTask->Stop();
#endif // USE_VDBTASK

    StopCamSurface();

    // if (g_xSS.iUsbHostMode == 1 && bOldUsbHostMode == 1 && g_xSS.iStartOta == 0)
    // {
    //     iRet = RET_POWEROFF;
    //     my_usleep(1000*1000);
    //     ShowUsbUpgradeFail();
    // }

    return iRet;
}

int processGlobalMsg()
{
    int iRet = 0;
    MSG* pMsg = NULL;
    while(1)
    {
        pMsg = (MSG*)message_queue_tryread(&g_worker);
        if (pMsg == NULL)
        {
#ifdef NOTHREAD_MUL
            break;
#else // NOTHREAD_MUL
            my_usleep(5000);
            continue;
#endif // NOTHREAD_MUL
        }
        if(pMsg->type == MSG_SENSE)
        {
            s_msg* pSenseMsg = (s_msg*)pMsg->data1;
            if(g_xSS.iMState == MS_OTA)
            {
                if (pMsg->data2 == OTA_RECV_DONE_OK || pMsg->data2 == OTA_RECV_DONE_STOP)
                {
                    if (g_xCS.x.bCheckFirmware == 0)
                    {
                        s_msg* reply_msg = SenseLockTask::Get_Note_OtaDone(0);
                        g_pSenseTask->Send_Msg(reply_msg);
                        my_usleep(10 * 1000);
                    }
                    pMsg = NULL;
                    iRet = 0;
                    break;
                }
                else if (pMsg->data2 != OTA_RECV_PCK)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Note_OtaDone(1);
                    g_pSenseTask->Send_Msg(reply_msg);
                    my_usleep(10 * 1000);
                    pMsg = NULL;
                    iRet = 0;
                    g_xSS.iStartOta = 1;
                    break;
                }
            }
            else if(pSenseMsg->mid == MID_START_OTA && g_xSS.iStartOta)
            {
                g_xSS.iMState = MS_OTA;
                SenseLockTask::m_encMode = SenseLockTask::EM_NOENCRYPT;

                s_msg* msg = SenseLockTask::Get_Reply(MID_START_OTA, MR_SUCCESS);
                g_pSenseTask->Send_Msg(msg);
                pMsg = NULL;
                iRet = 0;
                break;
            }
            if (USE_NEW_RST_PROTO == 0 || pSenseMsg == NULL || g_xSS.rResetFlagTime <= pMsg->time_ms)
            {
                iRet = MsgProcSense(pMsg);
                if(iRet == 0 || iRet == RET_REBOOT)
                    break;
                iRet = 0;
            }
        }
        else if(pMsg->type == MSG_ERROR)
        {
            if(!MsgProcError(pMsg))
                break;
        }
        else if(pMsg->type == MSG_FM)
        {
            if(!MsgProcFM(pMsg))
                break;
        }
#if (USE_VDBTASK)
        else if(pMsg->type == MSG_VDB_TASK)
        {            
            if(pMsg->data1 == VDB_CAPTURED_IMAGE /*&& pMsg->data3 == g_pVDBTask->GetCounter()*/)
            {
                dbug_printf("capture image result %ld\n", pMsg->data2);
                if(g_iJpgDataLen > 0)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply_GetSavedImage(MR_SUCCESS, g_iJpgDataLen);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
                else
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GETSAVEDIMAGE, MR_FAILED4_UNKNOWNREASON);
                    g_pSenseTask->Send_Msg(reply_msg);
                }

                my_mutex_lock(g_xVDBMutex);
                if(g_xSS.iVDBStart == 2)
                {
                    g_pVDBTask->Stop();
                    g_xSS.iVDBStart = 0;
                }
                my_mutex_unlock(g_xVDBMutex);
            }
            else if(pMsg->data1 == VDB_CAMERA_ERROR /*&& pMsg->data3 == g_pVDBTask->GetCounter()*/)
            {
                if(g_xSS.iVDBCmd == MID_GETSAVEDIMAGE)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply_CamError(MID_GETSAVEDIMAGE, MR_FAILED4_CAMERA, g_xSS.iCamError);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
#ifdef MID_VIDEO_ON
                else if(g_xSS.iVDBCmd == MID_VIDEO_ON)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply_CamError(MID_VIDEO_ON, MR_FAILED4_CAMERA, g_xSS.iCamError);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
#endif // MID_VIDEO_ON
            }
#ifdef MID_VIDEO_ON
            else if(pMsg->data1 == VDB_STARTED && pMsg->data3 == g_pVDBTask->GetCounter())
            {
                if(g_xSS.iVDBCmd == MID_VIDEO_ON)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_VIDEO_ON, MR_SUCCESS);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
            }
#endif // MID_VIDEO_ON
        }
#endif // USE_VDBTASK

        message_queue_message_free(&g_worker, (void*)pMsg);
    }

    if(pMsg != NULL)
        message_queue_message_free(&g_worker, (void*)pMsg);

    return iRet;
}

int GotoActivation()
{
    MSG* pMsg = NULL;
    while(1)
    {
        pMsg = (MSG*)message_queue_tryread(&g_worker);
        if (pMsg == NULL)
        {
            my_usleep(5000);
            continue;
        }
        if(pMsg->type == MSG_FM)
        {
            if(!MsgProcFM(pMsg))
                break;
        }

        message_queue_message_free(&g_worker, (void*)pMsg);
    }

    if(pMsg != NULL)
        message_queue_message_free(&g_worker, (void*)pMsg);

    return 0;
}

void UART_Create()
{
    UART_SetBaudrate(UART_Baudrate(DEFAULT_UART0_BAUDRATE));
    //g_pSenseTask = new SenseLockTask();
    g_pSenseTask = &g_SenseTask;
    g_pSenseTask->Start();
}

void UART_Release()
{
    if(g_pFMTask != NULL)
    {
        g_pFMTask->Stop();
        g_pFMTask->Wait();
        // delete g_pFMTask;
        // g_pFMTask = NULL;
    }

    if(g_pSenseTask != NULL)
    {
        g_pSenseTask->Stop();
        g_pSenseTask->Wait();
        // delete g_pSenseTask;
        // g_pSenseTask = NULL;
    }
}

void* ProcessInsmod(void *param)
{
    StartFirstCam();

    // my_thread_exit(NULL);
    return NULL;
}

void EndIns()
{
    if(g_thdInsmod)
    {
        my_thread_join(&g_thdInsmod);
        g_thdInsmod = 0;
    }

    if(g_iMipiCamInited == -1)
    {
        GPIO_fast_setvalue(IR_LED, OFF);
        g_iFirstCamFlag = 0;
        dbug_printf("camera error\n");
    }
}

void EngineMapInit()
{

}

void EngineMapRelease()
{

}

int Upgrade_Firmware(void)
{
    MEDIA_VIDEO_Deinit();
#ifndef UPGRADE_MODE
    unsigned int nWaitCnt = 0;
#endif
    my_printf("[upgrader] start %f\n", Now());

    GPIO_fast_init();
    M24C64_Open();

#ifdef IR_LED
    GPIO_fast_config(IR_LED, OUT);
    GPIO_fast_setvalue(IR_LED, OFF);
#endif

#ifdef M24C64_WP
    GPIO_fast_config(M24C64_WP, OUT);
    GPIO_fast_setvalue(M24C64_WP, OFF);
#endif

#ifdef PSENSE_DET
    GPIO_fast_config(PSENSE_DET, IN);
#endif // PSENSE_DET

#ifdef GPIO_USBSense
    GPIO_fast_config(GPIO_USBSense, IN);
#endif

    UART_Init();

    UART_SetBaudrate(UART_Baudrate(DEFAULT_UART0_BAUDRATE));

    message_queue_init(&g_worker, sizeof(MSG), MAX_MSG_NUM);
#ifndef UPGRADE_MODE
    g_pSenseTask = &g_SenseTask;
    g_pSenseTask->Start();

    my_usleep(10 * 1000);

    g_pSenseTask->SetActive(1);

    nWaitCnt = 0;
    while(++nWaitCnt < 20 && g_xSS.bUVCRunning != 0)
    {
        my_usleep(100 * 1000);
    }

    g_xSS.iMState = MS_OTA;
    SenseLockTask::m_encMode = SenseLockTask::EM_NOENCRYPT;
    s_msg* msg = SenseLockTask::Get_Reply(MID_START_OTA, MR_SUCCESS);
    g_pSenseTask->Send_Msg(msg);
#ifndef NOTHREAD_MUL
    processGlobalMsg();
#else // ! NOTHREAD_MUL
    g_pSenseTask->doProcess();
#endif

    g_xSS.iStartOta = 0;

#else//UPGRADE_MODE

    resetUpgradeInfo();
#endif//!UPGRADE_MODE

    my_printf("[upgrader] end %f\n", Now());
    my_usleep(10 * 1000);

    //end darkhorse

    UART_Quit();
    M24C64_Close();
#ifndef UPGRADE_MODE
    g_pSenseTask->Stop();
    g_pSenseTask->Wait();
#endif
    GPIO_fast_deinit();
    return 1;
}
static int main1(int argc, char** argv);
int main0(int argc, char** argv)
{
    my_printf("Main: %0.3f\n", Now());
//    process_seg_fault();

    if (argc != 1 && argc != 2)
        return 0;

    int iRet = 0;
    g_rAppStartTime = Now();

    DriverInit();

    message_queue_init(&g_worker, sizeof(MSG), MAX_MSG_NUM);

    if (CreateSharedLCD())
    {        
        my_printf("Cannot create shared mem.\n");
        ReleaseAll();
        DriverRelease();

        return RET_POWEROFF;
    }

    ResetSystemState(APP_MAIN);

    g_pWatchTask->Start(0);
    if (argc == 1)
    {
        fr_InitLive();
        fr_InitEngine_Hand();
        fr_InitIRCamera_ExpGain();
        //my_thread_create_ext(&g_thdInsmod, 0, ProcessInsmod, NULL, (char*)"insmod1", 8192, 0/*MYTHREAD_PRIORITY_MEDIUM*/);
        ProcessInsmod(NULL);
#if (USE_VDBTASK)
        StartClrCam();
#endif // USE_VDBTASK
        //EngineMapInit();
    }

    int iUpgradeFlag = g_xCS.x.bUpgradeFlag;
    int iUpgradeBaudrate = g_xCS.x.bUpgradeBaudrate;
    if(argc == 2 && !strcmp(argv[1], "-at"))
    {
        ResetMyAllSettings();

        EndIns();
        StopCamSurface();
#if (USE_VDBTASK)
        StopClrCam();
#endif // USE_VDBTASK

        UART_SetBaudrate(UART_Baudrate(DEFAULT_UART0_BAUDRATE));
        // UART_Create();

        g_xSS.iNoActivated = 1;

        g_pFMTask->Start();

        //send uart baudrate info ?
        g_pFMTask->SendCmd(FM_CMD_STATUS, DEFAULT_UART0_BAUDRATE, 0, STATUS_NO_ACTIVATED);

        GotoActivation();

        ReleaseAll();
        DriverRelease();
#ifndef LIB_TEST
        EngineMapRelease();
#endif
        if(g_xSS.iActivated)
        {
            my_usleep(5 * 1000 * 1000);
            return RET_REBOOT;
        }

        return RET_POWEROFF;
    }

    UART_Create();

    if(argc == 1)
    {
        int iIsFirst = 0;
        int iOldUsbHost = g_xCS.x.bUsbHost || g_xCS.x.bOtaMode;
        iIsFirst = rootfs_is_first();
        if(iIsFirst == 1)
        {
            if(iUpgradeFlag == 0)
            {
                my_printf("%s:%d\n", __FILE__, __LINE__);
                fr_InitAppLog();
            }

            rootfs_set_first_flag();
        }

        ///partition를 map하고 공장설정, 일반설정정보를 읽는다.
        if (try_mount_dbfs() == 1)
        {
            umount_dbfs();
            ReleaseAll();
            DriverRelease();
#ifndef LIB_TEST
            EngineMapRelease();
#endif

            return RET_POWEROFF;
        }

        if (iIsFirst)
        {
#ifndef __RTK_OS__
            ResetTmp(iUpgradeFlag);
#endif // ! __RTK_OS__

            EndIns();
            StopCamSurface();

            if(iUpgradeFlag == 1)
            {
                g_xCS.x.bUpgradeFlag = 0;
                g_xCS.x.bUsbHost = 0;
                g_xCS.x.bOtaMode = 0;
                UpdateCommonSettings();

                //send otag done success!
                unsigned char abOtaCmd[] = {0xEF, 0xAA, 0x01, 0x00, 0x02, 0x03, 0x00, 0x00};
                //abOtaCmd[7] = SenseLockTask::Get_CheckSum(abOtaCmd + 2, sizeof(abOtaCmd) - 3);

                if (BR_IS_VALID(iUpgradeBaudrate))
                {
                    UART_SetBaudrate(UART_Baudrate(iUpgradeBaudrate));
                    usleep(10 * 1000);
                }
                else
                {
                    UART_SetBaudrate(UART_Baudrate(DEFAULT_UART0_BAUDRATE));
                }

                ///Ota Done을 받지 못하는 문제가 있어서 100ms지연을 줌
                my_usleep(100 * 1000);
                UART_Send(abOtaCmd, sizeof(abOtaCmd));
                my_usleep(10 * 1000);
                my_printf("Send Ota Finished! ok, %d\n", iOldUsbHost);
                if (iOldUsbHost == 1)
                {
                    EndIns();
                    StopCamSurface();
                    //notify finishing of upgrading.
                    GPIO_fast_setvalue(IR_LED, 1);
                    for (int i = 0; i < 20; i ++)
                    {
                        if (i % 2 == 0)
                        {
                            GPIO_fast_setvalue(IR_LED, 1);
                            my_usleep(100*1000);
                        }
                        else
                        {
                            GPIO_fast_setvalue(IR_LED, 0);
                            my_usleep(100*1000);
                        }
                    }
                    GPIO_fast_setvalue(IR_LED, 1);
                    //delay in order to delay 2 secs.
                }
            }
            else
            {
                if(g_pSenseTask)
                {
                    g_pSenseTask->Stop();
                    g_pSenseTask->Wait();
                    // delete g_pSenseTask;
                    // g_pSenseTask = NULL;
                }

                //g_pFMTask = new FaceModuleTask;
                g_pFMTask->Start();

                g_pFMTask->SendCmd(FM_CMD_STATUS, 0, 0, STATUS_ACTIVATE_SUCCESSED);

                my_usleep(100 * 1000);
            }

            //g_xROKLog.x.bKernelCounter = 0;
            //g_xROKLog.x.bKernelFlag = 0xAA;
            //UpdateROKLogs();
            ReleaseAll();
            DriverRelease();
            dbug_printf("First initialize finished!\n");
            if (iOldUsbHost == 1)
                return RET_POWEROFF;
            else
                return RET_REBOOT;
        }
    }

    // if (g_xCS.x.bUsbHost != 0)
    // {
    //     g_xCS.x.bUsbHost = 0;
    //     UpdateCommonSettings();
    // }

    iRet = main1(argc, argv);

    FaceEngine::Release();

    if(iRet == RET_POWEROFF)
    {
        //EndBattLog();
        IncreaseSystemRunningCount1();

        umount_dbfs();
    }
#if (DB_TEST == 1)
    umount("/db1");
#endif

    if(g_xSS.iStartOta)
    {
        my_printf("exec upgrader!\n");
        //execl(UPGRADE_PATH, NULL);//kkk
    }

#if (FM_PROTOCOL == FM_DESMAN)
    if(g_xSS.iFuncTestFlag != 1)
    {
        if(g_xSS.pLastMsg)
        {
            if(g_xSS.pLastMsg->mid == MID_DELALL)
            {
//                sync(); //MID_DELALL호출부분에서 sync를 하였으므로 다시 호출하지 않음
                my_usleep(200 * 1000);
            }
            if(g_xSS.iRestoreRootfs == 2)
            {
                my_sync();
                my_usleep(200 * 1000);
            }

            g_pSenseTask->Send_Msg(g_xSS.pLastMsg);
        }
        else
        {
            if(g_xSS.iMidPowerdown == 0)
                g_xSS.iMidPowerdown = MID_POWERDOWN;

            g_pSenseTask->Send_Msg(SenseLockTask::Get_Reply(g_xSS.iMidPowerdown, MR_SUCCESS));
        }
    }
#endif

    ReleaseAll();
    DriverRelease();
#ifndef LIB_TEST
    EngineMapRelease();
#endif
#if 0
    if (get_kernel_flag_action())
    {
        M24C64_Open();
        ReadROKLogs();
        if(g_xSS.iRestoreRootfs == 1)
        {
            //체계복귀가 끝나지 않았으면 다음번기동에 두번째파티션으로 기동하게 함
            g_xROKLog.x.bKernelCounter = MAX_ROOTFS_FAIL_COUNT;
            g_xROKLog.x.bKernelFlag = 0x55;
        }
        else
            g_xROKLog.x.bKernelFlag = 0xAA;//kzh
        UpdateROKLogs();
        M24C64_Close();
    }
#endif
    return iRet;
}

static int main1(int argc, char** argv)
{
    int iRet = RET_POWEROFF;
    g_rAppStartTime = Now();
    ///////////////////////////////////////체계정보를 초기화한다.//////////////////////////////////////
    //g_xSS.bPresentation = g_xCS.x.bPresentation;
    if(argc == 1)
        IncreaseSystemRunningCount();

    //StartBattLog();
    //////////////////////////////////////////////엔진초기화를 진행한다./////////////////////////////////////////////////

    // if(g_xPS.x.bEnableLogFile)
    //     mount_db1();

    // fr_EnableLogFile(g_xPS.x.bEnableLogFile);

#if(USE_TWIN_ENGINE)
    fr_SetDNNData();
#endif

    if (face_engine_create(argc))
    {
        set_kernel_flag_action(0);
        return RET_POWEROFF;
    }
#ifndef __RTK_OS__
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
    {
        if (face_engine_create_again())
        {
            set_kernel_flag_action(0);
            return RET_POWEROFF;
        }
    }
#else // !__RTK_OS__
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
    {
        my_printf("@@@ Restore DBpartition!\n");
        start_restore_dbPart();
    }
#endif // !__RTK_OS__

#if (FM_PROTOCOL == FM_EASEN)
    g_pFMTask->SendCmd(FM_CMD_STATUS, 0, 0, STATUS_READY);
#elif (FM_PROTOCOL == FM_DESMAN)
    SenseLockTask::Set_KeyPos(g_xES.x.bEncKeyPos);
#if (ROOTFS_BAK)
    start_restore_roofs();
#endif
#ifndef NOTHREAD_MUL
    g_pSenseTask->SendReady();
    my_printf("SR1: %0.1f\n", Now());
#endif // ! NOTHREAD_MUL
#endif // FM_PROTOCOL

    g_xSS.rAppStartTime = Now();

    //mark as success.
    //my_printf("mount mark success.\n");
    SetMountStatus(1);
    SetModifyUser(0);

#if (USE_AUTO_50_REPLY)
    s_msg* psMsg = NULL;
    psMsg = (s_msg*)my_malloc(sizeof(raw_msg_init_enc));
    memcpy(psMsg, raw_msg_init_enc, sizeof(raw_msg_init_enc));
    SendGlobalMsg(MSG_SENSE, (long)psMsg, 0, 0);
#endif // USE_AUTO_50_REPLY

#ifdef USE_TWIN_ENGINE
#ifdef PROTECT_ENGINE
    ///STM으로부터 읽은 사전자료를 설정한다.
    unsigned char abDict[160] = { 0 };
    MainSTM_GetDict(abDict, sizeof(abDict));
    fr_SetDecodedData(0, abDict);
#endif // PROTECT_ENGINE
#endif // USE_TWIN_ENGINE

    //caution: iRet must be zero(power off) or RET_REBOOT
    //be careful when you modify return value.
    iRet = GotoMain();

    return iRet;
}

int fmMain()
{
    int ret = 0;
    g_FlashReadWriteLock = my_mutex_init();
    g_MyPrintfLock = my_mutex_init();
#if USE_VDBTASK
    g_xVDBMutex = my_mutex_init();
#endif

    ReadMyAllSettings();
#ifdef UPGRADE_MODE
    ret = Upgrade_Firmware();
    if (ret)
        SystemReboot();
#else//!UPGRADE_MODE
    const char *argv[2]={"app", "-at"};
    do {
        ret = main0(rootfs_is_activated(), (char**)argv);
        if (g_xSS.iStartOta)
        {
            g_xSS.iStartOta = 0;
            ret = Upgrade_Firmware();
            if (ret)
                SystemReboot();
        }
        else if (ret == RET_REBOOT)
        {
            SystemReboot();
        }
    } while(ret != 0);
#endif//UPGRADE_MODE
    return 0;
}

int MsgProcSense(MSG* pMsg)
{
    int iRet = -1;
    if(pMsg->type != MSG_SENSE)
        return iRet;

    if(pMsg->data1 == 0)
    {
        if (pMsg->data2 == OTA_USB_START)
        {
#if (USE_VDBTASK)
            if (g_pVDBTask->IsStreaming() == 0)
            {
                if (pMsg->data3 == 1)
                {
                    g_xCS.x.bUsbHost = 1;
                    UpdateCommonSettings();
                    my_printf("***** USB upgrade mode...\n");
                }
                else
                {
                    my_printf("restart for restore rootfs.\n");
                }
                return RET_REBOOT;
            }
            else
                return -1;
#else // USE_VDBTASK
            if (pMsg->data3 == 1)
            {
                g_xCS.x.bUsbHost = 1;
                UpdateCommonSettings();
                my_printf("***** USB upgrade mode...\n");
            }
            else
            {
                my_printf("restart for restore rootfs.\n");
            }
            return RET_REBOOT;
#endif // USE_VDBTASK
        }
        else if (pMsg->data2 == OTA_USB_DETECTED)
        {
            if (pMsg->data3 == 1)
            {
                my_printf("***** USB detected.\n");
                fr_WriteUSBScanEnableState();
                SystemReboot();
                g_xSS.iStartOta = 1;
                g_xCS.x.bOtaMode = 1;
                UpdateCommonSettings();
            }
            else
            {
                //usb not found
            }
            return 0;
        }
        else if (pMsg->data2 == SENSE_READY_DETECTED)
        {
            ResetFMStates();

            UART_Release();
            UART_Quit();
            UART_Init();
            UART_Create();
            my_usleep(100*1000);
            s_msg* msg = SenseLockTask::Get_Note(NID_READY);
            g_pSenseTask->SetActive(1);
            g_pSenseTask->Send_Msg(msg);
            my_printf("Send Ready, running, %0.3f\n", Now());
            return -1;
        }
        else
        {
#ifndef NOTHREAD_MUL
            return iRet;
#else
            g_xSS.iOtaError = pMsg->data2;
            return 0;
#endif
        }
    }

    s_msg* pSenseMsg = (s_msg*)pMsg->data1;
#if 0
    if(pSenseMsg->mid != MID_OTA_HEADER && pSenseMsg->mid != MID_INIT_ENCRYPTION && SenseLockTask::m_encMode == SenseLockTask::EM_NOENCRYPT)
    {
        dbug_printf("MR_FAILED4_NO_ENCRYPT\n");
        s_msg* msg = SenseLockTask::Get_Reply(pSenseMsg->mid, MR_FAILED4_NO_ENCRYPT);
        g_pSenseTask->Send_Msg(msg);
        my_free(pSenseMsg);
        g_xSS.iMState = MS_STANDBY;
        return -1;
    }
#endif

    g_xSS.iMState = MS_BUSY;
    g_xSS.iRunningCmd = pSenseMsg->mid;
    if(pSenseMsg->mid == MID_ENROLL)
    {
        dbug_printf("MID_ENROLL\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_enroll_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_ENROLL, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        s_msg_enroll_data d;
        memcpy(&d, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
        if (!IS_FACE_DIR_VALID(d.face_direction))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_ENROLL, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        if(d.face_direction == FACE_DIRECTION_UNDEFINE)
            d.face_direction = FACE_DIRECTION_MIDDLE;
        if (d.timeout == 0)
            d.timeout = SF_DEF_FACE_ENROLL_TIMEOUT;

        SF_ENROLL_NOR2ITG(&d, &g_xSS.msg_enroll_itg_data);
        g_xSS.msg_enroll_itg_data.enroll_type = FACE_ENROLL_TYPE_INTERACTIVE;
        g_xSS.msg_enroll_itg_data.enable_duplicate = FACE_ENROLL_DCM_NFACE_NAME;

        g_xSS.iEnrollMutiDirMode = 1;
        g_xSS.iEnrollFaceDupCheck = ENROLL_DUPLICATION_CHECK;
#if (USE_SANJIANG3_MODE)
        if (SenseLockTask::m_encMode == SenseLockTask::EM_XOR && g_xSS.iProtoMode == 1)
            g_xSS.iEnrollFaceDupCheck = 0;
#endif // USE_SANJIANG3_MODE
        g_xSS.iEnrollDupCheckMode = FACE_ENROLL_DCM_NFACE_NAME;

        iRet = ProcessSenseFace(FaceRecogTask::E_REGISTER);
//        dbug_printf("Enroll Ret = %d\n", iRet);
    }
    else if(pSenseMsg->mid == MID_ENROLL_SINGLE)
    {
        dbug_printf("MID_ENROLL_SINGLE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_enroll_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_ENROLL_SINGLE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }
        s_msg_enroll_data d;
        memcpy(&d, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
#if (N_MAX_HAND_NUM)
        g_xSS.iRegisterHand = 0;
        if (d.face_direction == FACE_DIRECTION_HAND)
            g_xSS.iRegisterHand = 1;
#endif // N_MAX_HAND_NUM
        //ignores face direction
        d.face_direction = FACE_DIRECTION_UNDEFINE;
        if (d.timeout == 0)
            d.timeout = SF_DEF_FACE_ENROLL_TIMEOUT;

        SF_ENROLL_NOR2ITG(&d, &g_xSS.msg_enroll_itg_data);
        g_xSS.msg_enroll_itg_data.enroll_type = FACE_ENROLL_TYPE_SINGLE;
        g_xSS.msg_enroll_itg_data.enable_duplicate = FACE_ENROLL_DCM_NFACE_NAME;

        g_xSS.iEnrollMutiDirMode = 0;
#if (USE_FUSHI_PROTO)
        if (g_xSS.iProtocolHeader == FUSHI_HEAD1)
            g_xSS.iEnrollFaceDupCheck = 1;
        else
#endif // USE_FUSHI_PROTO
            g_xSS.iEnrollFaceDupCheck = 0;

#if (N_MAX_HAND_NUM)
#if (USE_SANJIANG3_MODE)
        if (g_xSS.iRegisterHand)
        {
            g_xSS.iEnrollFaceDupCheck = ENROLL_HAND_DUP_CHECK;
            if (SenseLockTask::m_encMode == SenseLockTask::EM_XOR && g_xSS.iProtoMode == 1)
                g_xSS.iEnrollFaceDupCheck = 0;
        }
#else // USE_SANJIANG3_MODE
        if (g_xSS.iRegisterHand)
            g_xSS.iEnrollFaceDupCheck = ENROLL_HAND_DUP_CHECK;
#endif // USE_SANJIANG3_MODE
#endif // N_MAX_HAND_NUM

        g_xSS.iEnrollDupCheckMode = FACE_ENROLL_DCM_NFACE_NAME;

        iRet = ProcessSenseFace(FaceRecogTask::E_REGISTER);
//        dbug_printf("Enroll Ret = %d\n", iRet);
    }
#if (USE_ENROLL_ITG)
    else if(pSenseMsg->mid == MID_ENROLL_ITG)
    {
        dbug_printf("MID_ENROLL_ITG\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_enroll_itg_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_ENROLL_ITG, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        memcpy(&g_xSS.msg_enroll_itg_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
        if (!IS_FACE_ENROLL_TYPE_VALID(g_xSS.msg_enroll_itg_data.enroll_type) ||
                !IS_FACE_ENROLL_DCM_VALID(g_xSS.msg_enroll_itg_data.enable_duplicate) ||
                !IS_FACE_DIR_VALID(g_xSS.msg_enroll_itg_data.face_direction))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_ENROLL_ITG, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        dbug_printf("data=");
        for (int i = 0; i < SenseLockTask::Get_MsgLen(pSenseMsg); i++)
            dbug_printf("%02x ", pSenseMsg->data[i]);
        dbug_printf("\n%d, %d, \n", SenseLockTask::Get_MsgLen(pSenseMsg),
                    g_xSS.msg_enroll_itg_data.timeout);

        if (g_xSS.msg_enroll_itg_data.timeout == 0)
            g_xSS.msg_enroll_itg_data.timeout = SF_DEF_FACE_ENROLL_TIMEOUT;

        g_xSS.iEnrollMutiDirMode = (g_xSS.msg_enroll_itg_data.enroll_type == FACE_ENROLL_TYPE_INTERACTIVE);
        if (!g_xSS.iEnrollMutiDirMode)
            g_xSS.msg_enroll_itg_data.face_direction = FACE_DIRECTION_UNDEFINE;
        else if(g_xSS.msg_enroll_itg_data.face_direction == FACE_DIRECTION_UNDEFINE)
                g_xSS.msg_enroll_itg_data.face_direction = FACE_DIRECTION_MIDDLE;
        g_xSS.iEnrollDupCheckMode = g_xSS.msg_enroll_itg_data.enable_duplicate;
        g_xSS.iEnrollFaceDupCheck = (g_xSS.iEnrollDupCheckMode == FACE_ENROLL_DCM_NFACE_NAME);

        iRet = ProcessSenseFace(FaceRecogTask::E_REGISTER);
//        dbug_printf("Enroll Ret = %d\n", iRet);
    }
#endif // ENROLL_ITG
    else if(pSenseMsg->mid == MID_VERIFY)
    {
#if (USE_FUSHI_PROTO)
        g_xSS.bVerifying = 1;
#endif
        dbug_printf("MID_VERIFY %f\n", Now());

        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_verify_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        ResetFaceRegisterStates();

        memcpy(&g_xSS.msg_verify_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
        iRet = ProcessSenseFace(FaceRecogTask::E_VERIFY);
    }
    else if(pSenseMsg->mid == MID_FACERESET)
    {
        dbug_printf("MID_FACERESET\n");
        ResetFaceRegisterStates();

        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_FACERESET, MR_SUCCESS);
        g_pSenseTask->Send_Msg(reply_msg);
    }
    else if(pSenseMsg->mid == MID_DELUSER)
    {
        dbug_printf("MID_DELUSER\n");
        memset(&g_xSS.msg_deluser_data, 0, sizeof(g_xSS.msg_deluser_data));
        if(SenseLockTask::Get_MsgLen(pSenseMsg) > 0)
        {
            int len = __min(SenseLockTask::Get_MsgLen(pSenseMsg), (int)sizeof(g_xSS.msg_deluser_data));
            memcpy(&g_xSS.msg_deluser_data, pSenseMsg->data, len);
            if (g_xSS.msg_deluser_data.user_type >= SM_DEL_USER_TYPE_END)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(pSenseMsg->mid, MR_FAILED4_INVALIDPARAM);
                g_pSenseTask->Send_Msg(reply_msg);

                free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return -1;
            }
        }

        int iUserID = TO_SHORT(g_xSS.msg_deluser_data.user_id_heb, g_xSS.msg_deluser_data.user_id_leb);
        PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iUserID - 1);
#if (N_MAX_HAND_NUM)
        if (iUserID > N_MAX_PERSON_NUM)
            pxMetaInfo = dbm_GetHandMetaInfoByID(iUserID - N_MAX_PERSON_NUM - 1);
        else if (g_xSS.msg_deluser_data.user_type == SM_DEL_USER_TYPE_HAND)
            pxMetaInfo = dbm_GetHandMetaInfoByID(iUserID - 1);
#endif // N_MAX_HAND_NUM
        if(pxMetaInfo == NULL)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_DELUSER, MR_FAILED4_UNKNOWNUSER);
            if(g_xSS.iSendLastMsgMode)
            {
                g_xSS.pLastMsg = reply_msg;
                my_free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return 0;
            }
            else
                g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            SetModifyUser(1);

            int iBackupState = mount_backup_db(0);
#if (N_MAX_HAND_NUM)
            if (iUserID > N_MAX_PERSON_NUM)
                dbm_RemoveHandByID(iUserID - N_MAX_PERSON_NUM - 1, &iBackupState);
            else if (g_xSS.msg_deluser_data.user_type == SM_DEL_USER_TYPE_HAND)
                dbm_RemoveHandByID(iUserID - 1, &iBackupState);
            else
#endif // N_MAX_HAND_NUM
            {
                dbm_RemovePersonByID(iUserID - 1, &iBackupState);
            }
            umount_backup_db();

            UpdateUserCount();
            ResetFaceRegisterStates();

            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_DELUSER, MR_SUCCESS);
            if(g_xSS.iSendLastMsgMode)
            {
                g_xSS.pLastMsg = reply_msg;
                free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return 0;
            }
            else
                g_pSenseTask->Send_Msg(reply_msg);
        }
    }
    else if(pSenseMsg->mid == MID_DELALL)
    {
        dbug_printf("MID_DELALL\n");

        s_msg_del_all_data d;
        memset(&d, 0, sizeof(d));
        if(SenseLockTask::Get_MsgLen(pSenseMsg) > 0)
        {
            int len = __min(SenseLockTask::Get_MsgLen(pSenseMsg), (int)sizeof(d));
            memcpy(&d, pSenseMsg->data, len);
            if (d.type >= SM_DEL_ALL_TYPE_END)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(pSenseMsg->mid, MR_FAILED4_INVALIDPARAM);
                g_pSenseTask->Send_Msg(reply_msg);

                free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return -1;
            }
        }
#if (DESMAN_ENC_MODE == 0)
        //disable hijack function
        g_xPS.x.bHijackEnable = 0;
        UpdatePermanenceSettings();
#endif
        {
            ResetPersonDB(d.type);

            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_DELALL, MR_SUCCESS);
            g_pSenseTask->Send_Msg(reply_msg);
        }
    }
    else if(pSenseMsg->mid == MID_GETUSERINFO)
    {
        dbug_printf("MID_GETUSERINFO\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_getuserinfo_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GETUSERINFO, MR_FAILED4_INVALIDPARAM);
            if(g_xSS.iSendLastMsgMode)
            {
                g_xSS.pLastMsg = reply_msg;
                my_free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return 0;
            }
            else
            {
                g_pSenseTask->Send_Msg(reply_msg);
                my_free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return -1;
            }
        }

        memcpy(&g_xSS.msg_getuserinfo_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));

        int iUserID = TO_SHORT(g_xSS.msg_getuserinfo_data.user_id_heb, g_xSS.msg_getuserinfo_data.user_id_leb);
        PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iUserID - 1);
        if(pxMetaInfo == NULL)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GETUSERINFO, MR_FAILED4_UNKNOWNUSER);
            if(g_xSS.iSendLastMsgMode)
            {
                g_xSS.pLastMsg = reply_msg;
                my_free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return 0;
            }
            else
                g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply_GetUserInfo(MR_SUCCESS, iUserID);
            if(g_xSS.iSendLastMsgMode)
            {
                g_xSS.pLastMsg = reply_msg;
                my_free(pSenseMsg);
                g_xSS.iMState = MS_STANDBY;
                return 0;
            }
            else
                g_pSenseTask->Send_Msg(reply_msg);
        }
    }
    else if(pSenseMsg->mid == MID_GET_ALL_USERID)
    {
        dbug_printf("MID_GET_ALL_USERID\n");
        int failed_code = MR_SUCCESS;
        int fmt = SM_USERID_DATA_FMT_DEFAULT;
        if(SenseLockTask::Get_MsgLen(pSenseMsg) == sizeof(s_msg_get_all_userid_data))
        {
            s_msg_get_all_userid_data d;
            memcpy(&d, pSenseMsg->data, sizeof(d));
            fmt = d.fmt;
#if (N_MAX_PERSON_NUM >= 0xFF)
            if (fmt != SM_USERID_DATA_FMT_BIT_EXT)
            {
                failed_code = MR_FAILED4_INVALIDPARAM;
            }
#endif // N_MAX_PERSON_NUM
        }
#if (N_MAX_PERSON_NUM >= 0xFF)
        else
        {
            failed_code = MR_FAILED4_INVALIDPARAM;
        }
#endif // N_MAX_PERSON_NUM

        s_msg* reply_msg = NULL;
        if (failed_code == MR_SUCCESS)
        {
            reply_msg = SenseLockTask::Get_Reply_GetAllUserID(MR_SUCCESS, fmt);
        }
        else
            reply_msg = SenseLockTask::Get_Reply(pSenseMsg->mid, failed_code);
        g_pSenseTask->Send_Msg(reply_msg);
    }
#if (USE_FUSHI_PROTO)
    else if(pSenseMsg->mid == MID_GET_FREE_USERID)
    {
        dbug_printf("MID_GET_FREE_USERID\n");
        s_msg* reply_msg = SenseLockTask::Get_Reply_GetFreeUserID(MR_SUCCESS);
        if(g_xSS.iSendLastMsgMode)
        {
            g_xSS.pLastMsg = reply_msg;
            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return 0;
        }
        else
            g_pSenseTask->Send_Msg(reply_msg);
    }
#endif // USE_FUSHI_PROTO
    else if(pSenseMsg->mid == MID_GET_VERSION)
    {
        dbug_printf("MID_GET_VERSION\n");
        s_msg* reply_msg = SenseLockTask::Get_Reply_GetVersion(MR_SUCCESS);
        if(g_xSS.iSendLastMsgMode)
        {
            g_xSS.pLastMsg = reply_msg;
            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return 0;
        }
        else
            g_pSenseTask->Send_Msg(reply_msg);
    }
    else if(pSenseMsg->mid == MID_GET_UID)
    {
        dbug_printf("MID_GET_UID\n");
        s_msg* reply_msg = SenseLockTask::Get_Reply_GetUID(MR_SUCCESS);
        g_pSenseTask->Send_Msg(reply_msg);
    }
    else if(pSenseMsg->mid == MID_INIT_ENCRYPTION)
    {
        dbug_printf("MID_INIT_ENCRYPTION\n");
        int iMode = 1;
        int failed_code = MR_SUCCESS;
        int copy_len = 0;
        GET_SENSE_MSG_LEN(SenseLockTask::Get_MsgLen(pSenseMsg),
                          g_xSS.msg_init_encryption_data,
                          copy_len);
        memset(&g_xSS.msg_init_encryption_data, 0, sizeof(g_xSS.msg_init_encryption_data));
        if (copy_len >= 4)
        {
            memcpy(&g_xSS.msg_init_encryption_data, pSenseMsg->data, copy_len);

            if (copy_len > 4)
                iMode = g_xSS.msg_init_encryption_data.mode;
            else
                iMode = SenseLockTask::EM_AES;
#if (DEFAULT_PROTO_ENC_MODE == PROTO_EM_ENCRYPT_AES_DEFAULT)
            if (g_xSS.msg_init_encryption_data.mode == 0)
                iMode = SenseLockTask::EM_AES;
#endif // DEFAULT_PROTO_ENC_MODE

            char* strEncKey = NULL;
#if (USE_SANJIANG3_MODE)
            unsigned char sj_seed[4] = {0x30, 0x31, 0x32, 0x33};
            unsigned char sj_seed_lanheng[4] = {0x3C, 0x56, 0xB3, 0x85};
            strEncKey = (char*)PROTO_EM_XOR1_KEY_SANJIANG;
            g_xSS.iProtoMode = 1;
            if (memcmp(g_xSS.msg_init_encryption_data.seed, sj_seed, sizeof(sj_seed)))
            {
                if (memcmp(g_xSS.msg_init_encryption_data.seed, sj_seed_lanheng, sizeof(sj_seed_lanheng)) == 0)
                {
                    //lanheng mode
                    strEncKey = (char*)PROTO_EM_XOR1_KEY_LANHENG;
                    g_xSS.iProtoMode = 0;
                }
                else
                {
                    //qixin mode, force no encryption
                    iMode = SenseLockTask::EM_NOENCRYPT;
                }
            }
#elif (USE_AES_NOENC_MODE)
            unsigned char sj_seed[4] = {0x30, 0x31, 0x32, 0x33};
            if (memcmp(g_xSS.msg_init_encryption_data.seed, sj_seed, sizeof(sj_seed)) == 0)
            {
                //qixin mode, force no encryption
                iMode = SenseLockTask::EM_NOENCRYPT;
            }
#endif // USE_AES_NOENC_MODE

            if (SenseLockTask::Set_Key((unsigned char*)g_xSS.msg_init_encryption_data.seed, 4, iMode, strEncKey))
            {
                failed_code = MR_FAILED4_INVALIDPARAM;
            }
        }
        else
            failed_code = MR_FAILED4_INVALIDPARAM;

        if (failed_code != MR_SUCCESS)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_INIT_ENCRYPTION, failed_code);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply_Init_Encryption_Data(MR_SUCCESS);
            g_pSenseTask->Send_Msg(reply_msg);
        }
    }
    else if(pSenseMsg->mid == MID_SET_ENC_KEY)
    {
        dbug_printf("MID_SET_ENC_KEY\n");
        int failed_code = MR_SUCCESS;
        int copy_len = 0;
        s_msg_enc_key_number_data data;
        GET_SENSE_MSG_LEN(SenseLockTask::Get_MsgLen(pSenseMsg),
                          data,
                          copy_len);
        if (copy_len == sizeof(data))
        {
            memcpy(&data, pSenseMsg->data, copy_len);

            if (SenseLockTask::Set_KeyPos(data.enc_key_number))
            {
                failed_code = MR_FAILED4_INVALIDPARAM;
            }
            else
            {
                memcpy(g_xES.x.bEncKeyPos, data.enc_key_number, sizeof(g_xES.x.bEncKeyPos));
                UpdateEncryptSettings();
            }
        }
        else
            failed_code = MR_FAILED4_INVALIDPARAM;

        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_SET_ENC_KEY, failed_code);
        g_pSenseTask->Send_Msg(reply_msg);
    }
    else if(pSenseMsg->mid == MID_SET_RELEASE_ENC_KEY)
    {
        dbug_printf("MID_SET_RELEASE_ENC_KEY\n");
        int failed_code = MR_SUCCESS;
        int copy_len = 0;
        s_msg_enc_key_number_data data;
        GET_SENSE_MSG_LEN(SenseLockTask::Get_MsgLen(pSenseMsg),
                          data,
                          copy_len);
        if (copy_len == sizeof(data))
        {
            memcpy(&data, pSenseMsg->data, copy_len);

#if (DEFAULT_PROTO_ENC_MODE == PROTO_EM_ENCRYPT_AES_DEFAULT)
            if (g_xES.x.bEncKeyPos[0] == 0xff) // never set before
            {
                if (SenseLockTask::Set_KeyPos(data.enc_key_number))
                {
                    failed_code = MR_FAILED4_INVALIDPARAM;
                }
                else
                {
                    memcpy(g_xES.x.bEncKeyPos, data.enc_key_number, sizeof(g_xES.x.bEncKeyPos));
                    UpdateEncryptSettings();
                }
            }
            else // already set
                failed_code = MR_REJECTED;
#else // DEFAULT_PROTO_ENC_MODE
            if (SenseLockTask::Set_KeyPos(data.enc_key_number))
            {
                failed_code = MR_FAILED4_INVALIDPARAM;
            }
            else
            {
                memcpy(g_xES.x.bEncKeyPos, data.enc_key_number, sizeof(g_xES.x.bEncKeyPos));
                UpdateEncryptSettings();
            }
#endif // DEFAULT_PROTO_ENC_MODE
        }
        else
            failed_code = MR_FAILED4_INVALIDPARAM;

        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_SET_RELEASE_ENC_KEY, failed_code);
        g_pSenseTask->Send_Msg(reply_msg);
    }
    else if(pSenseMsg->mid == MID_RESET)
    {
        dbug_printf("MID_RESET\n");
        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_RESET, MR_SUCCESS);
        g_pSenseTask->Send_Msg(reply_msg);
    }
    else if(pSenseMsg->mid == MID_SNAPIMAGE)
    {
        dbug_printf("MID_SNAPIMAGE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_snap_image_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_SNAPIMAGE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        memcpy(&g_xSS.msg_snap_image_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));

        int iImageCount = g_xSS.msg_snap_image_data.image_counts;
        int iStartNum = g_xSS.msg_snap_image_data.start_number;
        int iInvalidParam = 0;
        if(iImageCount > SI_MAX_IMAGE_COUNT || iImageCount <= 0)
            iInvalidParam = 1;

        if(iStartNum <= 0 || iStartNum > SI_MAX_IMAGE_COUNT)
            iInvalidParam = 1;

        if(iInvalidParam == 1)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_SNAPIMAGE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            iRet = ProcessSenseFace(FaceRecogTask::E_GET_IMAGE);
            ResetFaceRegisterStates();
        }
    }
    else if(pSenseMsg->mid == MID_GETSAVEDIMAGE)
    {
        dbug_printf("MID_GETSAVEDIMAGE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_get_saved_image_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GETSAVEDIMAGE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        memcpy(&g_xSS.msg_get_saved_image_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));

        mount_db1();

        int iImgNum = g_xSS.msg_get_saved_image_data.image_number;

        if(iImgNum > 0)
        {
			int iImgLen = 0;
            if (iImgNum > 0 && iImgNum <= SI_MAX_IMAGE_COUNT)
                iImgLen = g_xSS.iSnapImageLen[iImgNum - 1];
            dbug_printf("iImgLen = %d, %d\n", iImgNum, g_xSS.iSnapImageLen[iImgNum - 1]);
            if(iImgLen > 0)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply_GetSavedImage(MR_SUCCESS, iImgLen);
                g_pSenseTask->Send_Msg(reply_msg);
            }
            else
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GETSAVEDIMAGE, MR_FAILED4_INVALIDPARAM);
                g_pSenseTask->Send_Msg(reply_msg);
            }
        }
#if (USE_VDBTASK)
        else
        {
            int iSuccessCode = saveUvcScene();
            //int iSuccessCode = MR_FAILED4_NOCAMERA;
            if (iSuccessCode != MR_SUCCESS)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GETSAVEDIMAGE, iSuccessCode);
                g_pSenseTask->Send_Msg(reply_msg);
            }
            else
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply_GetSavedImage(MR_SUCCESS, g_iJpgDataLen);
                g_pSenseTask->Send_Msg(reply_msg);
            }
        }
#else //USE_VDBTASK
        else
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GETSAVEDIMAGE, MR_FAILED4_NOCAMERA);
            g_pSenseTask->Send_Msg(reply_msg);
        }
#endif //USE_VDBTASK
    }
    else if(pSenseMsg->mid == MID_GET_LOGFILE)
    {
        dbug_printf("MID_GET_LOGFILE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_get_saved_image_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GET_LOGFILE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        int iImgLen = 0;
        iImgLen = fr_GetAppLogLen();

        memcpy(&g_xSS.msg_get_saved_image_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));

        if(iImgLen > 0)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply_GetLogFile(MR_SUCCESS, iImgLen);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_GET_LOGFILE, MR_FAILED4_UNKNOWNREASON);
            g_pSenseTask->Send_Msg(reply_msg);
        }
    }
    else if(pSenseMsg->mid == MID_UPLOADIMAGE)
    {
        dbug_printf("MID_UPLOADIMAGE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_upload_image_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UPLOADIMAGE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        memcpy(&g_xSS.msg_upload_image_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));

        mount_db1();

        int iImageOffset = (g_xSS.msg_upload_image_data.upload_image_offset[0] << 24) |
                (g_xSS.msg_upload_image_data.upload_image_offset[1] << 16) |
                (g_xSS.msg_upload_image_data.upload_image_offset[2] << 8) |
                (g_xSS.msg_upload_image_data.upload_image_offset[3]);

        int iImageSize = (g_xSS.msg_upload_image_data.upload_image_size[0] << 24) |
                (g_xSS.msg_upload_image_data.upload_image_size[1] << 16) |
                (g_xSS.msg_upload_image_data.upload_image_size[2] << 8) |
                (g_xSS.msg_upload_image_data.upload_image_size[3]);

        int iImgNum = g_xSS.msg_get_saved_image_data.image_number;
        if(iImageSize <= 0 || iImageSize > MAX_IMAGE_SIZE || iImageOffset < 0 || iImgNum < 0 || iImgNum > SI_MAX_IMAGE_COUNT)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UPLOADIMAGE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            if(iImgNum > 0)
            {
	            unsigned char* pbImageData = g_xSS.bSnapImageData + SI_MAX_IMAGE_SIZE * (iImgNum - 1) + iImageOffset;
	            if (g_xSS.bSnapImageData != NULL && pbImageData >= g_xSS.bSnapImageData && pbImageData + iImageSize < g_xSS.bSnapImageData + SI_MAX_IMAGE_COUNT * SI_MAX_IMAGE_SIZE)
	            {
	                s_msg* reply_msg = SenseLockTask::Get_Image(pbImageData, iImageSize);
	                g_pSenseTask->Send_Msg(reply_msg);
	            }
	            else
	            {
	                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UPLOADIMAGE, MR_FAILED4_UNKNOWNREASON);
	                g_pSenseTask->Send_Msg(reply_msg);
	            }
	        }
#if (USE_VDBTASK)
            else
            {
                if(g_iJpgDataLen > 0 && iImageOffset + iImageSize <= g_iJpgDataLen)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Image(g_abJpgData + iImageOffset, iImageSize);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
                else
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UPLOADIMAGE, MR_FAILED4_INVALIDPARAM);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
            }
#endif //USE_VDBTASK
        }
    }
    else if(pSenseMsg->mid == MID_UPLOAD_LOGFILE)
    {
        dbug_printf("MID_UPLOAD_LOGFILE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_upload_image_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UPLOAD_LOGFILE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        memcpy(&g_xSS.msg_upload_image_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));

        int iImageOffset = (g_xSS.msg_upload_image_data.upload_image_offset[0] << 24) |
                (g_xSS.msg_upload_image_data.upload_image_offset[1] << 16) |
                (g_xSS.msg_upload_image_data.upload_image_offset[2] << 8) |
                (g_xSS.msg_upload_image_data.upload_image_offset[3]);

        int iImageSize = (g_xSS.msg_upload_image_data.upload_image_size[0] << 24) |
                (g_xSS.msg_upload_image_data.upload_image_size[1] << 16) |
                (g_xSS.msg_upload_image_data.upload_image_size[2] << 8) |
                (g_xSS.msg_upload_image_data.upload_image_size[3]);

        if(iImageSize <= 0 || iImageSize > MAX_IMAGE_SIZE || iImageOffset < 0)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UPLOAD_LOGFILE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {

            unsigned char* pbImageData = (unsigned char*)my_malloc(iImageSize + 4);

            fr_ReadAppLog("app_log.txt", 0, pbImageData, iImageSize + 4);
            s_msg* reply_msg = SenseLockTask::Get_Image(pbImageData + 4, iImageSize);// 4B: Header
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pbImageData);
        }
    }
    else if(pSenseMsg->mid == MID_POWERDOWN)
    {
        dbug_printf("MID_POWERDOWN\n");
        iRet = 0;
        ResetFMStates();
        g_pSenseTask->Send_Msg(SenseLockTask::Get_Reply(MID_POWERDOWN, MR_SUCCESS));
        my_usleep(50 * 1000);
    }
    else if(pSenseMsg->mid == MID_POWERDOWN_ED)
    {
        dbug_printf("MID_POWERDOWN, ED\n");
        ResetFMStates();
        g_pSenseTask->Send_Msg(SenseLockTask::Get_Reply(MID_POWERDOWN_ED, MR_SUCCESS));
    }
    else if(pSenseMsg->mid == MID_DEMOMODE)
    {
        dbug_printf("MID_DEMOMODE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_demomode_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_DEMOMODE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            memcpy(&g_xSS.msg_demomode_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            g_xSS.iDemoMode = g_xSS.msg_demomode_data.enable;
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_DEMOMODE, MR_SUCCESS);
            g_pSenseTask->Send_Msg(reply_msg);
        }
    }
#if (USE_VDBTASK)
    else if(pSenseMsg->mid == MID_UVC_DIR)
    {
        dbug_printf("MID_UVC_DIR\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_uvc_dir_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UVC_DIR, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            memcpy(&g_xSS.msg_uvc_dir_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            if(g_xSS.msg_uvc_dir_data.rotate > 1)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UVC_DIR, MR_FAILED4_INVALIDPARAM);
                g_pSenseTask->Send_Msg(reply_msg);
            }
            else
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UVC_DIR, MR_SUCCESS);
                g_pSenseTask->Send_Msg(reply_msg);

                g_xCS.x.bUVCDir = g_xSS.msg_uvc_dir_data.rotate;
                UpdateCommonSettings();
            }
        }
    }
    else if(pSenseMsg->mid == MID_UVC_SET_COMPRESS_PARAM)
    {
        dbug_printf("MID_UVC_SET_COMPRESS_PARAM\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_uvc_set_compressparam_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UVC_SET_COMPRESS_PARAM, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            s_msg_uvc_set_compressparam_data cd;
            memcpy(&cd, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            if(cd.image_quality < UVC_COMP_PARAM_IMQ_MIN || cd.image_quality > UVC_COMP_PARAM_IMQ_MAX)
                cd.image_quality = UVC_COMP_PARAM_IMQ_MAX;
            if(cd.repeat_frame < UVC_COMP_PARAM_RPFR_MIN || cd.repeat_frame > UVC_COMP_PARAM_RPFR_MAX)
                cd.repeat_frame = UVC_COMP_PARAM_RPFR_MIN;
            if((cd.bitrate_max < UVC_COMP_PARAM_BT_MIN || cd.bitrate_max > UVC_COMP_PARAM_BT_MAX) ||
                    cd.bitrate_default < UVC_COMP_PARAM_BT_MIN || cd.bitrate_default > UVC_COMP_PARAM_BT_MAX ||
                    cd.bitrate_max <= cd.bitrate_default)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UVC_SET_COMPRESS_PARAM, MR_FAILED4_INVALIDPARAM);
                g_pSenseTask->Send_Msg(reply_msg);
            }
            else
            {
                g_xPS.x.bUvcBitrateDefault = cd.bitrate_default;
                g_xPS.x.bUvcBitrateMax = cd.bitrate_max;
                g_xPS.x.bUvcImageQuality = cd.image_quality;
                g_xPS.x.bUvcRepeatFrame = cd.repeat_frame;
                UpdatePermanenceSettings();

                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_UVC_SET_COMPRESS_PARAM, MR_SUCCESS);
                g_pSenseTask->Send_Msg(reply_msg);
            }
        }
    }
#endif //USE_VDBTASK
    else if(pSenseMsg->mid == MID_FACTORY_TEST)
    {
        dbug_printf("MID_FACTORY_TEST\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_factorytest_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_FACTORY_TEST, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            s_msg_factorytest_data fdata;

            memcpy(&fdata, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            if (fdata.enable == 1)
            {
                g_xSS.iDemoMode = N_DEMO_FACTORY_MODE;
                g_xSS.iSendLastMsgMode = 0;
#if (USE_WHITE_LED)
                gpio_whiteled_on(1);
#endif
            }
            if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
            {
                //use default camera direction in factory test mode.
                g_xSS.iCameraRotate = CAM_RM_DEFAULT;
                ResetFaceRegisterStates();
#if 1
                if (dbm_GetUserCount() == 0)
                {
                    s_msg* psMsg = NULL;
                    psMsg = (s_msg*)my_malloc(sizeof(raw_msg_enroll_middle));
                    memcpy(psMsg, raw_msg_enroll_middle, sizeof(raw_msg_enroll_middle));
                    g_xSS.iAutoUserAdd = 1;
                    my_printf("*** auto add step 1\n");
                    SendGlobalMsg(MSG_SENSE, (long)psMsg, 0, 0);
                }
                else
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_FACTORY_TEST, MR_SUCCESS);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
#endif
            }
        }
    }
#if (DESMAN_ENC_MODE == 0)
    else if(pSenseMsg->mid == MID_HIJACK)
    {
        dbug_printf("MID_HIJACK\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_hijack_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_HIJACK, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_HIJACK, MR_SUCCESS);
            s_msg_hijack_data hdata;

            memcpy(&hdata, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            g_xPS.x.bHijackEnable = (hdata.enable == 0 ? 0: 1);
            UpdatePermanenceSettings();

            g_pSenseTask->Send_Msg(reply_msg);
        }
    }
#endif // DESMAN_ENC_MODE
#if (USE_TWIN_ENGINE)
    else if(pSenseMsg->mid == MID_SET_THRESHOLD_LEVEL)
    {
        dbug_printf("MID_SET_THRESHOLD_LEVEL\n");
        int iOldValue = g_xPS.x.bTwinsMode;
        int iUpdate = 0;
        int iSuccCode = MR_SUCCESS;
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_algo_threshold_level))
        {
            iSuccCode = MR_FAILED4_INVALIDPARAM;
        }
        else
        {
            s_msg_algo_threshold_level hdata;

            memcpy(&hdata, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            if (hdata.verify_threshold_level == S_VERIFY_LEVEL_HIGH && iOldValue != S_VERIFY_LEVEL_HIGH)
            {
                iUpdate = 1;
            }
            else if (hdata.verify_threshold_level != S_VERIFY_LEVEL_HIGH && iOldValue == S_VERIFY_LEVEL_HIGH)
            {
                iUpdate = 1;
            }
            if (iUpdate)
            {
                //delete all users
                {
                    ResetPersonDB(SM_DEL_ALL_TYPE_DEFAULT);
                    //save config
                    g_xPS.x.bTwinsMode = hdata.verify_threshold_level;
                    UpdatePermanenceSettings();
                }
            }
        }
        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_SET_THRESHOLD_LEVEL, iSuccCode);
        g_pSenseTask->Send_Msg(reply_msg);
    }
#endif // USE_TWIN_ENGINE
    else if(pSenseMsg->mid == MID_ENABLE_LOGFILE)
    {
        dbug_printf("MID_ENABLE_LOGFILE\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_demomode_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_ENABLE_LOGFILE, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_ENABLE_LOGFILE, MR_SUCCESS);

            memcpy(&g_xSS.msg_send_msg_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            g_xPS.x.bEnableLogFile = g_xSS.msg_send_msg_data.enable;
            UpdatePermanenceSettings();

            if(g_xPS.x.bEnableLogFile)
                mount_db1();

            fr_EnableLogFile(g_xPS.x.bEnableLogFile);
            g_pSenseTask->Send_Msg(reply_msg);
        }
    }
    else if(pSenseMsg->mid == MID_CAMERA_FLIP)
    {
        dbug_printf("MID_CAMERA_FLIP\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(s_msg_demomode_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_CAMERA_FLIP, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);
        }
        else
        {
            memcpy(&g_xSS.msg_send_msg_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
            if (g_xSS.msg_send_msg_data.enable != 0 && g_xSS.msg_send_msg_data.enable != 1)
            {
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_CAMERA_FLIP, MR_FAILED4_INVALIDPARAM);
                g_pSenseTask->Send_Msg(reply_msg);
            }
            else
            {
                g_xSS.iCameraRotate = g_xSS.msg_send_msg_data.enable;
                if (g_xPS.x.bCamFlip != g_xSS.msg_send_msg_data.enable)
                {
                    g_xPS.x.bCamFlip = g_xSS.msg_send_msg_data.enable;
                    UpdatePermanenceSettings();
                }
                s_msg* reply_msg = SenseLockTask::Get_Reply(MID_CAMERA_FLIP, MR_SUCCESS);
                g_pSenseTask->Send_Msg(reply_msg);
            }
        }
    }
    else if(pSenseMsg->mid == MID_START_OTA)
    {
        dbug_printf("MID_START_OTA\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < (int)sizeof(g_xSS.msg_startota_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_START_OTA, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }
        else if(g_xSS.iUsbHostMode != 0)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_START_OTA, MR_REJECTED);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        my_free(pSenseMsg);
        g_xSS.iStartOta = 1;
        return 0;
#if 0
        memcpy(&g_xSS.msg_startota_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));
        SenseLockTask::EncMode = 0;

        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_START_OTA, MR_SUCCESS);
        g_pSenseTask->Send_Msg(reply_msg);
#endif
    }
#if 0
    else if(pSenseMsg->mid == MID_OTA_HEADER)
    {
        dbug_printf("MID_OTA_HEADER\n");
        if(SenseLockTask::Get_MsgLen(pSenseMsg) < sizeof(g_xSS.msg_otaheader_data))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_OTA_HEADER, MR_FAILED4_INVALIDPARAM);
            g_pSenseTask->Send_Msg(reply_msg);

            my_free(pSenseMsg);
            g_xSS.iMState = MS_STANDBY;
            return -1;
        }

        memcpy(&g_xSS.msg_otaheader_data, pSenseMsg->data, SenseLockTask::Get_MsgLen(pSenseMsg));

        int iFileSize = TO_INT(g_xSS.msg_otaheader_data.fsize_b);
        int iPckCount = TO_INT(g_xSS.msg_otaheader_data.num_pkt);
        int iPckSize = TO_SHORT(g_xSS.msg_otaheader_data.pkt_size[0], g_xSS.msg_otaheader_data.pkt_size[1]);

        g_xSS.pbOtaData = (unsigned char*)my_malloc(iFileSize);
        g_xSS.piOtaPckIdx = (int*)my_malloc(iPckCount * sizeof(int));
        memset(g_xSS.piOtaPckIdx, 0, iPckCount * sizeof(int));
        g_xSS.iMState = MS_OTA;
        g_xSS.rOtaTime = Now();

        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_OTA_HEADER, MR_SUCCESS);
        g_pSenseTask->Send_Msg(reply_msg);

        my_free(pSenseMsg);
        return -1;
    }
#endif
    else
    {
        my_printf("reject %d\n", pSenseMsg->mid);
        s_msg* reply_msg = SenseLockTask::Get_Reply(pSenseMsg->mid, MR_REJECTED);
        g_pSenseTask->Send_Msg(reply_msg);
    }


    my_free(pSenseMsg);
    g_xSS.iMState = MS_STANDBY;

    return iRet;
}

int ProcessSenseFace(int iCmd)
{
    ClearSenseResetFlag();

    ///기동하면서 켠 카메라에서 첫 프레임을 받은 상태이라면
    if((g_iFirstCamFlag & LEFT_IR_CAM_RECVED) || (g_iFirstCamFlag & RIGHT_IR_CAM_RECVED))
    {
        ///첫 프레임을 받은 시간으로부터 500ms미만이라면
        if(g_rFirstCamTime != 0 && Now() - g_rFirstCamTime > 500)
        {
            g_iFirstCamFlag = 0;
        }
    }

    if(iCmd == FaceRecogTask::E_REGISTER)
    {
#if (USE_WAEL_PROTO == 1)
        my_usleep(20 * 1000);
#endif
        int iUserID = dbm_GetNewUserID();
#if (N_MAX_HAND_NUM)
        if (g_xSS.iRegisterHand)
        {
            iUserID = dbm_GetNewHandUserID();
        }
#endif // N_MAX_HAND_NUM
        if(iUserID == -1)
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply(g_xSS.iRunningCmd, MR_FAILED4_MAXUSER);
            g_pSenseTask->Send_Msg(reply_msg);
#if (USE_VDBTASK && IR_LED_ONOFF_MODE == 1)
            if (VDBTask::CaptureCam != TC_MIPI_CAM)
                camera_set_irled(0, 0);
#endif // USE_VDBTASK
            g_xSS.rFaceEngineTime = 0;
            return -1;
        }
        else if(g_xSS.iEnrollMutiDirMode && (g_xSS.iRegisterDir & g_xSS.msg_enroll_itg_data.face_direction))
        {
            s_msg* reply_msg = SenseLockTask::Get_Reply_Enroll(MR_SUCCESS, -1, g_xSS.iRegisterDir, g_xSS.iRunningCmd);
            g_pSenseTask->Send_Msg(reply_msg);
#if (USE_VDBTASK && IR_LED_ONOFF_MODE == 1)
            if (VDBTask::CaptureCam != TC_MIPI_CAM)
                camera_set_irled(0, 0);
#endif // USE_VDBTASK
            g_xSS.rFaceEngineTime = 0;
            return -1;
        }
        else
        {
            if (g_xSS.iRunningCmd == MID_ENROLL_ITG && g_xSS.iEnrollDupCheckMode == FACE_ENROLL_DCM_FACE_NNAME)
            {
                int iDupError = 0;
                for (int k = 0; k < dbm_GetPersonCount(); k++)
                {
                    dbm_GetPersonMetaInfoByID(k);
                    PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByIndex(k);
                    if(pxMetaInfo == NULL)
                        continue;
                    if (strcmp(pxMetaInfo->szName, (const char*)g_xSS.msg_enroll_itg_data.user_name) == 0)
                    {
                        iDupError = 1;
                        break;
                    }
                }
                if (iDupError)
                {
                    s_msg* reply_msg = SenseLockTask::Get_Reply_Enroll(MR_FAILED4_FACEENROLLED, -1, g_xSS.iRegisterDir, g_xSS.iRunningCmd);
                    g_pSenseTask->Send_Msg(reply_msg);
                    g_xSS.rFaceEngineTime = 0;
                    return -1;
                }
            }
            g_iFirstCamFlag = 0;
            g_xSS.iRegisterID = iUserID + 1;
            g_xSS.iRegsterAuth = g_xSS.msg_enroll_itg_data.admin;

            if(g_iEnrollInit == 0)
            {
                FaceEngine::UnregisterFace(-1, g_xSS.iEnrollMutiDirMode);
                g_xSS.iRegisterDir = 0;
                g_iEnrollInit = 1;
            }
            else if (!g_xSS.iEnrollMutiDirMode)
            {
                FaceEngine::UnregisterFace(-1, g_xSS.iEnrollMutiDirMode);
                g_xSS.iRegisterDir = 0;
            }
        }
    }
    else if(iCmd == FaceRecogTask::E_VERIFY)
    {
#if (USE_WAEL_PROTO == 1)
        my_usleep(20 * 1000);
#endif
#if (!USE_DEMOMODE2)
        if(dbm_GetPersonCount() == 0 && g_xSS.iDemoMode == N_DEMO_VERIFY_MODE_OFF)
        {
#if (DESMAN_ENC_MODE == 0)
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_FAILED4_UNKNOWNUSER);
            g_pSenseTask->Send_Msg(reply_msg);
#else // DESMAN_ENC_MODE == 0
            s_msg* reply_msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_ABORTED);
            g_pSenseTask->Send_Msg(reply_msg);
#endif // DESMAN_ENC_MODE == 0
#if (USE_VDBTASK && IR_LED_ONOFF_MODE == 1)
            if (VDBTask::CaptureCam != TC_MIPI_CAM)
                camera_set_irled(0, 0);
#endif
            g_xSS.rFaceEngineTime = 0;
            return -1;
        }
#endif // !USE_DEMOMODE2
    }

    ///기동하면서 켠 카메라에서 첫 프레임을 받은 상태가 아니라면
    if(!(g_iFirstCamFlag & LEFT_IR_CAM_RECVED) || g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
    {
        if (g_iMipiCamInited == -1)
            StartCamSurface(0);
        else
            StartCamSurface(1);
    }

#if (USE_VDBTASK)
    if(!(g_xSS.iCamError & CAM_ERROR_DVP2 || g_xSS.iCamError & CAM_ERROR_DVP1))
        StartClrCam();
#endif // USE_VDBTASK

    g_xSS.iResetFlag = 0;
    unsigned int iflag_NoStopCam = 0;
    FaceRecogTask* pFaceTask = getFaceInstance();
    feFaceStart(iCmd);
#if (USE_VDBTASK)
    g_pFaceRecogTask = pFaceTask;
#endif // USE_VDBTASK

    MSG* pMsg = NULL;
    while(1)
    {
        pMsg = (MSG*)message_queue_read(&g_queue_face);
        if(pMsg->type == MSG_RECOG_FACE && pMsg->data3 == pFaceTask->GetCounter())
        {
            if(pMsg->data1 == FACE_TASK_FINISHED)
            {
                if(g_xSS.iResetFlag == 1)
                {
//                    my_printf("Reset Flag!\n");
                    break;
                }

                int iResult = pFaceTask->GetResult();
                int iEyeOpened = pFaceTask->GetEyeOpened();
                if(iResult == FACE_RESULT_SUCCESS)
                {
                    dbug_printf("FACE_RESULT_SUCCESS\n");

                    int iID = -1;
                    int iFindIdx = pFaceTask->GetRecogIndex();
                    PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByIndex(iFindIdx);
                    if(pxMetaInfo)
                    {
                        iID = pxMetaInfo->iID;
                    }

                    if(g_xSS.iDemoMode == N_DEMO_VERIFY_MODE_ON)
                    {
                        my_usleep(20*1000); //delay 20ms for host
                        iID = -2;
                    }
#if (USE_VDBTASK)
                    if (g_xSS.iDemoMode == N_DEMO_FACTORY_MODE)
                    {
                        float _tmpNow = Now();
                        while(Now() - _tmpNow < 5000)
                        {
                            if (g_xSS.iCamError & CAM_ERROR_CLR_CHECKED)
                                break;
                            my_usleep(10*1000);
                        }
                        my_usleep(50*1000);
                        if (!(g_xSS.iCamError & CAM_ERROR_CLR_CHECKED))
                            g_xSS.iCamError |= CAM_ERROR_DVP2;
                        g_xSS.iCamError &= ~CAM_ERROR_CLR_CHECKED;
                    }
#endif // USE_VDBTASK
                    s_msg* msg = NULL;
                    if (g_xSS.iDemoMode != N_DEMO_FACTORY_MODE || g_xSS.iCamError == 0)
                    {
                        msg = SenseLockTask::Get_Reply_Verify(MR_SUCCESS, iID + 1,
                                                                     iEyeOpened ? ST_FACE_MODULE_STATUS_UNLOCK_OK : ST_FACE_MODULE_STATUS_UNLOCK_WITH_EYES_CLOSE);
                    }
                    else
                    {
                        msg = SenseLockTask::Get_Reply_CamError(MID_VERIFY, MR_FAILED4_CAMERA, g_xSS.iCamError);
                    }
#if (DEFAULT_SECURE_MODE == 1)
                    unsigned char _tmp_buf_cs[sizeof(g_xCS)];
                    unsigned char _tmp_buf_hd2[sizeof(g_xHD2)];
                    memcpy(_tmp_buf_cs, &g_xCS, sizeof(g_xCS));
                    memcpy(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2));
                    fr_resetThresholdState(&g_xHD, &g_xCS, &g_xHD2);
                    if (memcmp(_tmp_buf_cs, &g_xCS, sizeof(g_xCS)))
                        UpdateCommonSettings();
                    if (memcmp(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2)))
                        UpdateHeadInfos2();
#endif // DEFAULT_SECURE_MODE

                    ResetFaceRegisterStates();
                    if(g_xSS.iSendLastMsgMode)
                        g_xSS.pLastMsg = msg;
                    else
                        g_pSenseTask->Send_Msg(msg);

                }
                else if(iResult == FACE_RESULT_FAILED || iResult == HAND_RESULT_FAILED)
                {
                    dbug_printf("FACE_RESULT_FAILED\n");

#if (DEFAULT_SECURE_MODE == 1)
                    unsigned char _tmp_buf_cs[sizeof(g_xCS)];
                    unsigned char _tmp_buf_hd2[sizeof(g_xHD2)];
                    memcpy(_tmp_buf_cs, &g_xCS, sizeof(g_xCS));
                    memcpy(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2));
                    fr_resetThresholdState(&g_xHD, &g_xCS, &g_xHD2);
                    if (memcmp(_tmp_buf_cs, &g_xCS, sizeof(g_xCS)))
                        UpdateCommonSettings();
                    if (memcmp(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2)))
                        UpdateHeadInfos2();
#endif // DEFAULT_SECURE_MODE

                    s_msg* msg = NULL;
#if (DESMAN_ENC_MODE == 0)
                    if (g_xSS.iDemoMode == N_DEMO_VERIFY_MODE_ON)
                        msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_FAILED4_LIVENESSCHECK);
                    else
#endif
                    {
                        msg = SenseLockTask::Get_Reply(MID_VERIFY, iResult == FACE_RESULT_FAILED ? MR_FAILED4_UNKNOWNUSER : MR_FAILED4_UNKNOWN_HANDUSER);
                    }
                    if(g_xSS.iSendLastMsgMode)
                        g_xSS.pLastMsg = msg;
                    else
                        g_pSenseTask->Send_Msg(msg);
#if (USE_VDBTASK)
                    if(g_iCapturedClrFlag == CLR_CAPTURE_END)
                        g_iCapturedClrFlag = CLR_CAPTURE_OK;
                    else
                        g_iCapturedClrFlag = IR_CAPTURE_OK;
#endif // USE_VDBTASK
                }
                else if(iResult == FACE_RESULT_ENROLLED_FACE || (iResult == FACE_RESULT_DUPLICATED_FACE && g_xSS.iEnrollFaceDupCheck == 0))
                {
                    dbug_printf(iResult == FACE_RESULT_ENROLLED_FACE ? "Face Enrolled!\n" : "Face Duplicated!\n");
                    int iSuccessCode = MR_SUCCESS;
                    SMetaInfo *pxMetaInfo = (SMetaInfo *)my_malloc(sizeof(SMetaInfo));
                    SFeatInfo *pxFeatInfo = (SFeatInfo *)my_malloc(sizeof(SFeatInfo));
                    memset(pxMetaInfo, 0, sizeof(SMetaInfo));
                    memset(pxFeatInfo, 0, sizeof(SFeatInfo));
                    pxMetaInfo->iID = g_xSS.iRegisterID - 1;
                    strncpy(pxMetaInfo->szName, (char*)g_xSS.msg_enroll_itg_data.user_name, N_MAX_NAME_LEN - 1);
                    pxMetaInfo->fPrivilege = g_xSS.iRegsterAuth;

                    dbug_printf("Endroll Face: ID=%d, Privilege=%d, Name: %s, Len=%ld\n", 
                        pxMetaInfo->iID, pxMetaInfo->fPrivilege, pxMetaInfo->szName, strlen(pxMetaInfo->szName));
                    if (pxFeatInfo == NULL)
                    {
                        //fail
                    }

                    FaceEngine::GetRegisteredFeatInfo(pxFeatInfo);

                    SetModifyUser(1);

                    int iBackupState = mount_backup_db(0);
                    if (iBackupState < 0)
                    {
                        iSuccessCode = MR_FAILED4_WRITE_FILE;
                        SetModifyUser(0);
                    }
                    else
                    {
                        int ret = FaceEngine::SavePerson(pxMetaInfo, pxFeatInfo, &iBackupState);
                        umount_backup_db();
                        if (ret != ES_SUCCESS)
                        {
                            iSuccessCode = MR_FAILED4_WRITE_FILE;
                            SetModifyUser(0);
                        }
                        else
                        {
                            UpdateUserCount();
                        }
                    }
                    if (pxMetaInfo != NULL)
                        my_free(pxMetaInfo);
                    if (pxFeatInfo != NULL)
                        my_free(pxFeatInfo);

                    g_xSS.iRegisterDir |= g_xSS.msg_enroll_itg_data.face_direction;
                    s_msg* msg = SenseLockTask::Get_Reply_Enroll(iSuccessCode, g_xSS.iRegisterID, g_xSS.iRegisterDir, g_xSS.iRunningCmd);
                    if(g_xSS.iSendLastMsgMode)
                        g_xSS.pLastMsg = msg;
                    else
                    {
                        g_pSenseTask->Send_Msg(msg);
#if (USE_VDBTASK)
                        if(g_iCapturedClrFlag == CLR_CAPTURE_END)
                            g_iCapturedClrFlag = CLR_CAPTURE_OK;
                        else
                            g_iCapturedClrFlag = IR_CAPTURE_OK;
#endif //USE_VDBTASK
                    }
                    
                    if (g_xSS.iAutoUserAdd == 5)
                    {
                        my_printf("*** auto add step 6\n");
                        s_msg* reply_msg = SenseLockTask::Get_Reply(MID_FACTORY_TEST, MR_SUCCESS);
                        g_pSenseTask->Send_Msg(reply_msg);
                    }
                    ResetFaceRegisterStates();
                }
                else if(iResult == FACE_RESULT_DUPLICATED_FACE)
                {
                    dbug_printf("Face Duplicated!\n");
                    int iFaceDir = g_xSS.msg_enroll_itg_data.face_direction;
                    if(iFaceDir == 0)
                        iFaceDir = 1;

#if (ENROLL_DUPLICATION_CHECK != EDC_ENABLE_NO_SKIP)
                    g_xSS.iRegisterDir |= iFaceDir;
#endif // ENROLL_DUPLICATION_CHECK

                    s_msg* msg = SenseLockTask::Get_Reply_Enroll(MR_FAILED4_FACEENROLLED, -1, g_xSS.iRegisterDir, g_xSS.iRunningCmd);

#if (ENROLL_DUPLICATION_CHECK == EDC_ENABLE_NO_SKIP)
                    ResetFaceRegisterStates();
#endif // ENROLL_DUPLICATION_CHECK

                    g_pSenseTask->Send_Msg(msg);
                }
                else if(iResult == FACE_RESULT_SPOOF_FACE)
                {
                    dbug_printf("Face Spoof!\n");

                    s_msg* msg = SenseLockTask::Get_Reply(g_xSS.iRunningCmd, MR_FAILED4_TIMEOUT);
                    g_pSenseTask->Send_Msg(msg);

                    ResetFaceRegisterStates();
#if (USE_VDBTASK)
                    if(g_iCapturedClrFlag == CLR_CAPTURE_END)
                        g_iCapturedClrFlag = CLR_CAPTURE_OK;
                    else
                        g_iCapturedClrFlag = IR_CAPTURE_OK;
#endif // USE_VDBTASK
                }
                else if(iResult == FACE_RESULT_TIMEOUT)
                {
                    dbug_printf("FACE_RESULT_TIMEOUT\n");
#if (DEFAULT_SECURE_MODE == 1)
                    unsigned char _tmp_buf_cs[sizeof(g_xCS)];
                    unsigned char _tmp_buf_hd2[sizeof(g_xHD2)];
                    memcpy(_tmp_buf_cs, &g_xCS, sizeof(g_xCS));
                    memcpy(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2));
                    fr_resetThresholdState(&g_xHD, &g_xCS, &g_xHD2);
                    if (memcmp(_tmp_buf_cs, &g_xCS, sizeof(g_xCS)))
                        UpdateCommonSettings();
                    if (memcmp(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2)))
                        UpdateHeadInfos2();
#endif // DEFAULT_SECURE_MODE
                    if(iCmd == FaceRecogTask::E_REGISTER)
                    {
                        s_msg* msg = SenseLockTask::Get_Reply(g_xSS.iRunningCmd, MR_FAILED4_TIMEOUT);
                        if(g_xSS.iSendLastMsgMode)
                            g_xSS.pLastMsg = msg;
                        else
                            g_pSenseTask->Send_Msg(msg);
                    }
                    else if(iCmd == FaceRecogTask::E_GET_IMAGE)
                    {
                    }
                    else if(iCmd == FaceRecogTask::E_VERIFY)
                    {
                        s_msg* msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_FAILED4_TIMEOUT);
                        if(g_xSS.iSendLastMsgMode)
                            g_xSS.pLastMsg = msg;
                        else
                            g_pSenseTask->Send_Msg(msg);
                    }
                    else if(iCmd == FaceRecogTask::E_EYE_CHECK)
                    {
                        s_msg* msg = SenseLockTask::Get_Note_EyeState(g_xSS.note_data_eye.eye_state);
                        g_pSenseTask->Send_Msg(msg);
                    }
                }
                else if(iResult == FACE_RESULT_ENROLLED_NEXT || iResult == HAND_RESULT_ENROLLED_NEXT)
                {
                    dbug_printf("FACE_RESULT_ENROLLED_NEXT\n");
                    int iFaceDir = g_xSS.msg_enroll_itg_data.face_direction;
                    if(iFaceDir == 0)
                        iFaceDir = 1;

                    g_xSS.iRegisterDir |= iFaceDir;

                    if (g_xSS.iAutoUserAdd == 1)
                    {
                        //enroll right
                        my_printf("*** auto add step 2\n");
                        s_msg* psMsg = NULL;
                        psMsg = (s_msg*)my_malloc(sizeof(raw_msg_enroll_right));
                        memcpy(psMsg, raw_msg_enroll_right, sizeof(raw_msg_enroll_right));
                        g_xSS.iAutoUserAdd = 2;
                        SendGlobalMsg(MSG_SENSE, (long)psMsg, 0, 0);
                    }
                    else if (g_xSS.iAutoUserAdd == 2)
                    {
                        //enroll left
                        my_printf("*** auto add step 3\n");
                        s_msg* psMsg = NULL;
                        psMsg = (s_msg*)my_malloc(sizeof(raw_msg_enroll_left));
                        memcpy(psMsg, raw_msg_enroll_left, sizeof(raw_msg_enroll_left));
                        g_xSS.iAutoUserAdd = 3;
                        SendGlobalMsg(MSG_SENSE, (long)psMsg, 0, 0);
                    }
                    else if (g_xSS.iAutoUserAdd == 3)
                    {
                        //enroll down
                        my_printf("*** auto add step 4\n");
                        s_msg* psMsg = NULL;
                        psMsg = (s_msg*)my_malloc(sizeof(raw_msg_enroll_down));
                        memcpy(psMsg, raw_msg_enroll_down, sizeof(raw_msg_enroll_down));
                        g_xSS.iAutoUserAdd = 4;
                        SendGlobalMsg(MSG_SENSE, (long)psMsg, 0, 0);
                    }
                    else if (g_xSS.iAutoUserAdd == 4)
                    {
                        //enroll up
                        my_printf("*** auto add step 5\n");
                        s_msg* psMsg = NULL;
                        psMsg = (s_msg*)my_malloc(sizeof(raw_msg_enroll_up));
                        memcpy(psMsg, raw_msg_enroll_up, sizeof(raw_msg_enroll_up));
                        g_xSS.iAutoUserAdd = 5;
                        SendGlobalMsg(MSG_SENSE, (long)psMsg, 0, 0);
                    }
                    else
                    {
                        s_msg* msg = SenseLockTask::Get_Reply_Enroll(MR_SUCCESS, -1, g_xSS.iRegisterDir, g_xSS.iRunningCmd);
                        g_pSenseTask->Send_Msg(msg);
                        iflag_NoStopCam = 1;
                    }
                }
                else if(iResult == FACE_RESULT_FAILED_CAMERA)
                {
                    ResetFaceRegisterStates();
                    if(iCmd == FaceRecogTask::E_REGISTER)
                    {
                        s_msg* msg = SenseLockTask::Get_Reply_CamError(g_xSS.iRunningCmd, MR_FAILED4_CAMERA, g_xSS.iCamError);
                        if(g_xSS.iSendLastMsgMode)
                            g_xSS.pLastMsg = msg;
                        else
                            g_pSenseTask->Send_Msg(msg);
                    }
                    else if(iCmd == FaceRecogTask::E_GET_IMAGE)
                    {
                        s_msg* msg = SenseLockTask::Get_Reply_CamError(MID_SNAPIMAGE, MR_FAILED4_CAMERA, g_xSS.iCamError);
                        if(g_xSS.iSendLastMsgMode)
                            g_xSS.pLastMsg = msg;
                        else
                            g_pSenseTask->Send_Msg(msg);
                    }
                    else if(iCmd == FaceRecogTask::E_VERIFY)
                    {
                        s_msg* msg = SenseLockTask::Get_Reply_CamError(MID_VERIFY, MR_FAILED4_CAMERA, g_xSS.iCamError);
                        if(g_xSS.iSendLastMsgMode)
                            g_xSS.pLastMsg = msg;
                        else
                            g_pSenseTask->Send_Msg(msg);
                    }
                }
                else if(iResult == FACE_RESULT_CAPTURED_FACE)
                {
                    s_msg* msg = SenseLockTask::Get_Reply(MID_SNAPIMAGE, MR_SUCCESS);
                    g_pSenseTask->Send_Msg(msg);
                }
                else if(iResult == FACE_RESULT_CAPTURED_FACE_FAILED)
                {
                    s_msg* msg = SenseLockTask::Get_Reply(MID_SNAPIMAGE, MR_FAILED4_UNKNOWNREASON);
                    g_pSenseTask->Send_Msg(msg);
                }
#if (N_MAX_HAND_NUM)
                else if(iResult == HAND_RESULT_SUCCESS)
                {
                    dbug_printf("HAND_RESULT_SUCCESS\n");

                    int iID = -1;
                    int iFindIdx = pFaceTask->GetRecogIndex();
                    PSMetaInfo pxMetaInfo = dbm_GetHandMetaInfoByIndex(iFindIdx);
                    dbug_printf("iFindIdx = %d, %p\n", iFindIdx, pxMetaInfo);
                    if(pxMetaInfo)
                    {
                        iID = pxMetaInfo->iID;
                    }

                    if(g_xSS.iDemoMode == N_DEMO_VERIFY_MODE_ON)
                        iID = -2 - N_MAX_PERSON_NUM;

                    s_msg* msg = NULL;
                    msg = SenseLockTask::Get_Reply_Verify(MR_SUCCESS, iID + N_MAX_PERSON_NUM + 1, ST_FACE_MODULE_STATUS_UNLOCK_HAND_OK);

#if (DEFAULT_SECURE_MODE == 1)
                    unsigned char _tmp_buf_cs[sizeof(g_xCS)];
                    unsigned char _tmp_buf_hd2[sizeof(g_xHD2)];
                    memcpy(_tmp_buf_cs, &g_xCS, sizeof(g_xCS));
                    memcpy(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2));
                    fr_resetThresholdState(&g_xHD, &g_xCS, &g_xHD2);
                    if (memcmp(_tmp_buf_cs, &g_xCS, sizeof(g_xCS)))
                        UpdateCommonSettings();
                    if (memcmp(_tmp_buf_hd2, &g_xHD2, sizeof(g_xHD2)))
                        UpdateHeadInfos2();
#endif // DEFAULT_SECURE_MODE

                    if (msg != NULL)
                        g_pSenseTask->Send_Msg(msg);
                }
                else if(iResult == HAND_RESULT_ENROLLED || (iResult == HAND_RESULT_ENROLL_DUPLICATED && g_xSS.iEnrollFaceDupCheck == 0))
                {
                    dbug_printf(iResult == HAND_RESULT_ENROLLED ? "Hand Enrolled!\n" : "Hand Duplicated!\n");
                    SMetaInfo xMetaInfo;
                    SHandFeatInfo xFeatInfo;
                    int iSuccessCode = MR_SUCCESS;
                    CLEAR(xMetaInfo);
                    CLEAR(xFeatInfo);
                    //get new user id because of mixed enroll mode
                    int iUserID = dbm_GetNewHandUserID();
                    if (iUserID >= 0)
                    {
                        g_xSS.iRegisterID = iUserID + 1;
                        xMetaInfo.iID = g_xSS.iRegisterID - 1;
                        strncpy(xMetaInfo.szName, (char*)g_xSS.msg_enroll_itg_data.user_name, N_MAX_NAME_LEN - 1);
                        xMetaInfo.fPrivilege = g_xSS.iRegsterAuth;

                        dbug_printf("Endroll Hand: ID=%d, Privilege=%d, Name: %s, Len=%ld\n", 
                            xMetaInfo.iID, xMetaInfo.fPrivilege, xMetaInfo.szName, strlen(xMetaInfo.szName));

                        FaceEngine::GetRegisteredFeatInfo_Hand(&xFeatInfo);

                        SetModifyUser(1);

                        int iBackupState = mount_backup_db(0);
                        if (iBackupState < 0)
                        {
                            iSuccessCode = MR_FAILED4_WRITE_FILE;
                            SetModifyUser(0);
                        }
                        else
                        {
                            int ret = FaceEngine::SaveHand(&xMetaInfo, &xFeatInfo, &iBackupState);
                            umount_backup_db();
                            if (ret != ES_SUCCESS)
                            {
                                iSuccessCode = MR_FAILED4_WRITE_FILE;
                                SetModifyUser(0);
                            }
                            else
                            {
                                UpdateUserCount();
                            }
                        }
                    }
                    else
                    {
                        iSuccessCode = MR_FAILED4_MAXUSER;
                    }

                    s_msg* msg = NULL;
                    if (iSuccessCode == MR_SUCCESS)
                    {
                        msg = SenseLockTask::Get_Reply_Enroll(iSuccessCode, g_xSS.iRegisterID + N_MAX_PERSON_NUM, g_xSS.iRegisterDir, g_xSS.iRunningCmd);
#if (USE_SANJIANG3_MODE && ENROLL_FACE_HAND_MODE == ENROLL_FACE_HAND_MIX && N_MAX_HAND_NUM)
                        if (SenseLockTask::m_encMode == SenseLockTask::EM_XOR && g_xSS.iProtoMode == 1)
                        {
                            free(msg);
                            msg = SenseLockTask::Get_Reply_Enroll(iSuccessCode, g_xSS.iRegisterID, g_xSS.iRegisterDir, g_xSS.iRunningCmd);
                        }
#endif // USE_SANJIANG3_MODE
                    }
                    else
                        msg = SenseLockTask::Get_Reply(g_xSS.iRunningCmd, iSuccessCode);
                    if(g_xSS.iSendLastMsgMode)
                        g_xSS.pLastMsg = msg;
                    else
                    {
                        g_pSenseTask->Send_Msg(msg);
                    }

                    ResetFaceRegisterStates();
                }
                else if(iResult == HAND_RESULT_ENROLL_DUPLICATED)
                {
                    s_msg* msg = NULL;
                    msg = SenseLockTask::Get_Reply_Enroll(MR_FAILED4_HANDENROLLED, -1, 0, g_xSS.iRunningCmd);
                    g_pSenseTask->Send_Msg(msg);

                    ResetFaceRegisterStates();
                }
#endif // N_MAX_HAND_NUM
#if (USE_DEMOMODE2)
                else if(iResult == FACE_RESULT_DETECTED || iResult == HAND_RESULT_DETECTED)
                {
                    ResetFaceRegisterStates();
                    s_msg* reply_msg = SenseLockTask::Get_Reply(MID_VERIFY,
                                                                iResult == FACE_RESULT_DETECTED ? MR_FAILED4_UNKNOWNUSER : MR_FAILED4_UNKNOWN_HANDUSER);
                    g_pSenseTask->Send_Msg(reply_msg);
                }
#endif // USE_DEMOMODE2
                break;
            }
            else if(pMsg->data1 == FACE_TASK_DETECTED)
            {
                if(iCmd == FaceRecogTask::E_REGISTER)
                {
#if (USE_FUSHI_PROTO)
                    if (g_xSS.iProtocolHeader == FUSHI_HEAD1)
                    {
                        g_xSS.iFaceState = g_xSS.note_data_face.state;
                    }
                    else
#endif // USE_FUSHI_PROTO
                    {
                        s_msg* msg = SenseLockTask::Get_Note_FaceState(NID_FACE_STATE);
                        g_pSenseTask->Send_Msg(msg);
                    }
                }
                else if(iCmd == FaceRecogTask::E_VERIFY)
                {
#if (USE_FUSHI_PROTO)
                    if (g_xSS.iProtocolHeader == FUSHI_HEAD1)
                    {
                        if (g_xSS.rVerifyStartTime > 0 && (Now() - g_xSS.rVerifyStartTime) >= FUSHI_VERIFY_ACK_TIMEOUT)
                        {
                            if (g_xSS.iFaceState == FACE_STATE_NOFACE)
                            {
                                s_msg* msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_FAILED4_TIMEOUT);
                                g_pSenseTask->Send_Msg(msg);
                            }
                            else if (g_xSS.iFaceState == FACE_STATE_NORMAL)
                            {
                                s_msg* msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_FAILED4_UNKNOWNUSER);
                                g_pSenseTask->Send_Msg(msg);
                            }
                            else
                            {
                                s_msg* msg = SenseLockTask::Get_Reply(MID_VERIFY, MR_FAILED4_TIMEOUT);
                                g_pSenseTask->Send_Msg(msg);
                            }
                            g_xSS.rVerifyStartTime = 0;
                            g_xSS.rLastVerifyAckSendTime = Now();
                        }
                        g_xSS.iFaceState = g_xSS.note_data_face.state;
                    }
                    else
#endif // USE_FUSHI_PROTO
                    {
                        s_msg* msg = SenseLockTask::Get_Note_FaceState(NID_FACE_STATE);
                        g_pSenseTask->Send_Msg(msg);
                    }
                }
            }
        }

        message_queue_message_free(&g_queue_face, (void*)pMsg);
    }

    if(pMsg != NULL)
        message_queue_message_free(&g_queue_face, (void*)pMsg);

    feFaceStop();
#if (USE_VDBTASK)
    g_pFaceRecogTask = NULL;
#endif // USE_VDBTASK

#if (USE_VDBTASK)
#if (NO_ENCRYPT_FRM3_4 == 0)
    my_mutex_lock(g_xVDBMutex);
    if(g_xSS.iVDBStart == 0)
    {
        if (iflag_NoStopCam == 0)
            StopCamSurface();
        StopClrCam();
    }
    else if(VDBTask::CaptureCam != TC_MIPI_CAM && iflag_NoStopCam == 0)
        StopCamSurface();
    my_mutex_unlock(g_xVDBMutex);
#else //NO_ENCRYPT_FRM3_4
#if (IR_LED_ONOFF_MODE == 1)
    if (VDBTask::CaptureCam != TC_MIPI_CAM)
    {
        camera_set_irled(0, 0);
        GPIO_fast_setvalue(IR_LED, OFF);
    }
#endif
#endif //NO_ENCRYPT_FRM3_4
#else //USE_VDBTASK
    // if (iflag_NoStopCam == 0)
    //     StopCamSurface();
    GPIO_fast_setvalue(IR_LED, OFF);
#endif //USE_VDBTASK

    g_iFirstCamFlag = 0;
    g_rFirstCamTime = 0;
    if (iflag_NoStopCam)
        iflag_NoStopCam = 0;

    g_xSS.rFaceEngineTime = 0;
    if(iCmd == FaceRecogTask::E_VERIFY)
    {
#if (USE_VDBTASK)
        // if(g_xSS.msg_verify_data.pd_rightaway == 1 && g_pVDBTask->IsStreaming() != 1)
        //     return 0;
#else // USE_VDBTASK
        if(g_xSS.msg_verify_data.pd_rightaway == 1)
        {
            return 0;
        }
#endif // USE_VDBTASK
    }

    if(g_xSS.pLastMsg)
    {
        return 0;
    }
    return -1;
}

int MsgProcFM(MSG* pMsg)
{
    if(pMsg->type != MSG_FM)
        return -1;

    dbug_printf("msg: %ld, %f\n", pMsg->data1, Now());
    if(pMsg->data1 == FM_CMD_ACTIVATE)
    {
        char* pbActData = (char*)pMsg->data2;
        if(pbActData)
        {
//            SetLed(BLED);
            CustomSerialNumberInfo ci = { 0 };
            int iRet = _decodeFaceLicense(pbActData, ci);
            if(iRet == ALL_RIGHT && ci.pHardwareID != NULL && ci.pUserData != NULL && ci.nUserDataLength == sizeof(int))
            {
                int iRet = ProcessActivation(ci.pHardwareID, *(int*)ci.pUserData);
                if(iRet < 0)
                {
                    my_free(ci.pHardwareID);

                    dbug_printf("activation failed! %d\n", iRet);

                    my_free(pbActData);

                    g_pFMTask->SendCmd(FM_CMD_STATUS, 0, 0, STATUS_ACTIVATE_FAILED);

//                    SetLed(RLED);
                    return -1;
                }

                dbug_printf("activation success!\n");
                my_free(ci.pHardwareID);
                my_free(pbActData);

                g_xSS.iActivated = 1;
                rootfs_set_activated(1, 1);
//                g_pFMTask->SendCmd(FM_CMD_STATUS, 0, 0, STATUS_ACTIVATE_SUCCESSED);
//                SetLed(GLED);
                return 0;
            }

            my_free(pbActData);

            g_pFMTask->SendCmd(FM_CMD_STATUS, 0, 0, STATUS_ACTIVATE_FAILED);
//            SetLed(RLED);
        }
    }
#if (USE_VDBTASK)
    else if(pMsg->data1 == FM_CMD_START_VDB)
    {
        if(g_xSS.iVDBStart == 0)
        {
            g_xSS.iVDBStart = 1;
            if(g_pVDBTask == NULL)
                g_pVDBTask = &g_VDBTask;

            g_pVDBTask->Start();
        }
        else
            g_xSS.iVDBStart = 1;
    }
#endif // USE_VDBTASK
#if (USE_VDBTASK)
    else if(pMsg->data1 == FM_CMD_STOP_VDB)
    {
        if(g_xSS.iVDBStart)
        {
            g_pVDBTask->Stop();
            g_xSS.iVDBStart = 0;
        }
    }
#endif // USE_VDBTASK
    else if(pMsg->data1 == FM_CMD_FINISH || (pMsg->data1 == SENSE_BASE_MSG + MID_POWERDOWN))
        return 0;
    else if(pMsg->data1 == FM_CMD_START_OTA)
    {
        g_xSS.iStartOta = 1;
        return 0;
    }
#if 1
    else if(pMsg->data1 == FM_CMD_DEV_TEST_START)
    {
        g_xSS.iFuncTestFlag = 1;
        UART_Release();

        if(g_xSS.iNoActivated == 0)
            StopCamSurface();

        FuncTestProc pFuncTestProc;
        FuncTestProc* g_pFuncTestProc = &pFuncTestProc;
        g_pFuncTestProc->SetActivated(g_xSS.iNoActivated == 1 ? 0 : 1);
        g_pFuncTestProc->Start();
        g_pFuncTestProc->Wait();
//        pFuncTestProc->GetResult();

        // delete pFuncTestProc;
        return 0;
    }
#endif
    else
        return -1;
    return -1;
}

int MsgProcError(MSG* pMsg)
{
    if(pMsg->type != MSG_ERROR)
        return -1;

    if(pMsg->data1 == ERROR_CAMERA_TCMIPI || pMsg->data1 == ERROR_CAMERA_DVP)
    {
#if (FM_PROTOCOL == FM_EASEN)
        g_pFMTask->SendCmd(FM_CMD_STATUS, 0, 0, STATUS_CAM_ERROR);
#endif
    }
    return -1;
}

void calculate_xor(unsigned int *msg, int msg_len, unsigned int*key,  int key_len)
{
    int i = 0, j = 0;
    for (i = 0; i < msg_len; i++) {
        msg[i] ^= key[j];
        if (++j >= key_len)
            j = 0;
    }
}

int MarkActivationFailed(int iError)
{
    dbug_printf("[%s] %d\n", __func__, iError);
    if (g_xPS.x.bActMark != 0xAA)
    {
        g_xPS.x.bActMark = 0xAA;
        UpdatePermanenceSettings();
    }
    return 0;
}

int _set_activation_mark()
{
    if (g_xPS.x.bActMark != 0x55)
    {
        g_xPS.x.bActMark = 0x55;
        UpdatePermanenceSettings();
    }
    return 1;
}

int _get_activation_mark()
{
    return (g_xPS.x.bActMark == 0x55 || g_xPS.x.bActMark == 0xAA);
}

int ProcessActivation(char* pbUID, int iUniqueID)
{
    if(pbUID == NULL)
        return -1;

    if(iUniqueID != g_iUniqueID)
    {
        my_printf("Unique ID Failed!  %x, %x, %s\n", iUniqueID, g_iUniqueID, pbUID);
        return -1;
    }

    unsigned char* b64_dec_data;
    b64_dec_data = base64_decode(pbUID, strlen(pbUID));
    if(b64_dec_data == NULL)
    {
        my_printf("UID Mark Size!\n");
        return -1;
    }

    int iMark = *(int*)b64_dec_data;
    if(iMark != UNIQUIE_MARK)
    {
        my_printf("UID Mark Error!\n");
        my_free(b64_dec_data);
        return -1;
    }

    unsigned int aiV3S_ID[4] = { 0 };

    memcpy(aiV3S_ID, b64_dec_data + sizeof(int) * 1, sizeof(aiV3S_ID));
    my_free(b64_dec_data);

    if(_get_activation_mark())
    {
        my_printf("*** mark detected1\n");
        return -6;
    }
    else
    {
        _set_activation_mark();
        my_printf("*** mark start\n");
        _get_activation_mark();
    }

    ///////////////////////////proca///////////////////////////////////
#ifdef __ENCRYPT_ENGINE__
    {
        int iResult = 1;
        const char* szDictNames[] = {
            FN_WNO_DICT_PATH,
            NULL};
        const unsigned long szDictLength[] = {
            FN_WNO_DICT_SIZE,
            0 };

        for(int i = 0; szDictNames[i] != NULL; i ++)
        {
            unsigned char* pbData = (unsigned char*)my_malloc(szDictLength[i]);
            if (pbData == NULL)
            {
                my_printf("failed to malloc, %d\n", i);
                break;
            }
            fr_ReadFileData(szDictNames[i], 0, pbData, szDictLength[i]);

            dbug_printf("proca: %s\n", szDictNames[i]);

            //write with encrypt
            rootfs_set_activated(1, 0);
            fr_WriteFileData(szDictNames[i], 0, pbData, szDictLength[i]);
            rootfs_set_activated(0, 0);
            my_free(pbData);
        }

        if(iResult == 0)
        {
            MarkActivationFailed(-2);
            return -2;
        }
    }
#endif // __ENCRYPT_DICT__

    return 0;
}

