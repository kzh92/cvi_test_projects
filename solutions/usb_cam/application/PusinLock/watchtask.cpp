#include "watchtask.h"
#include "unistd.h"
#include "drv_gpio.h"
#include "appdef.h"
#include "shared.h"
#include "msg.h"
#include "i2cbase.h"
#include "mount_fs.h"
#include "DBManager.h"
#include "senselocktask.h"
#include "upgradebase.h"
#include "cam_base.h"
#if (USE_PRINT_TEMP)
#include "cvi_tempsen.h"
#endif
#include <string.h>

extern SenseLockTask* g_pSenseTask;

void* watchTask_ThreadProc1(void* param);

WatchTask::WatchTask()
{
    Init();
}

WatchTask::~WatchTask()
{

}

void WatchTask::Init()
{
    m_iIDCounter = 0;
    m_iRunning = 0;
    m_iTimerCount = 0;
    m_xTimerMutex = my_mutex_init();
}

void WatchTask::Deinit()
{
    my_mutex_destroy(m_xTimerMutex);
    m_xTimerMutex = NULL;
}

void WatchTask::Start(int iBattScan)
{
    m_iRunning = 1;
    m_iTimerCount = 0;
    m_iBattScan = iBattScan;

    // m_aiTimerIDs = (int*)my_malloc(sizeof(int) * MAX_TIMER_COUNT);
    // memset(m_aiTimerIDs, 0, sizeof(int) * MAX_TIMER_COUNT);
    // m_aiTimerCounter = (int*)my_malloc(sizeof(int) * MAX_TIMER_COUNT);
    // memset(m_aiTimerCounter, 0, sizeof(int) * MAX_TIMER_COUNT);
    // m_aiTimerMsec = (float*)my_malloc(sizeof(float) * MAX_TIMER_COUNT);
    // memset(m_aiTimerMsec, 0, sizeof(float) * MAX_TIMER_COUNT);
    // m_arTimerTick = (float*)my_malloc(sizeof(float) * MAX_TIMER_COUNT);
    // memset(m_arTimerTick, 0, sizeof(float) * MAX_TIMER_COUNT);

#ifndef NOTHREAD_MUL
    if (my_thread_create_ext(&m_thread, NULL, watchTask_ThreadProc1, this, (char*)"wdt", 8192, MYTHREAD_PRIORITY_HIGH))
        my_printf("[WatchTask]create thread error.\n");
#endif // NOTHREAD_MUL
}

void WatchTask::Stop()
{
    m_iRunning = 0;
    Thread::Wait();

    // if (m_aiTimerIDs)
    // {
    //     my_free(m_aiTimerIDs);
    //     m_aiTimerIDs = NULL;
    // }
    // if (m_aiTimerCounter)
    // {
    //     my_free(m_aiTimerCounter);
    //     m_aiTimerCounter = NULL;
    // }
    // if (m_aiTimerMsec)
    // {
    //     my_free(m_aiTimerMsec);
    //     m_aiTimerMsec = NULL;
    // }
    // if (m_arTimerTick)
    // {
    //     my_free(m_arTimerTick);
    //     m_arTimerTick = NULL;
    // }
}

int WatchTask::AddTimer(float iMsec)
{
    // if(m_iTimerCount > MAX_TIMER_COUNT)
    //     return -1;

    // my_mutex_lock(m_xTimerMutex);
    // m_iIDCounter ++;
    // m_aiTimerIDs[m_iTimerCount] = m_iIDCounter;
    // m_aiTimerCounter[m_iTimerCount] = 0;
    // m_aiTimerMsec[m_iTimerCount] = iMsec;
    // m_arTimerTick[m_iTimerCount] = Now();
    // m_iTimerCount ++;
    // my_mutex_unlock(m_xTimerMutex);

    return m_iIDCounter;
}

void WatchTask::RemoveTimer(int iTimerID)
{
    // my_mutex_lock(m_xTimerMutex);

    // int iExist = -1;
    // for(int i = 0; i < m_iTimerCount; i ++)
    // {
    //     if(m_aiTimerIDs[i] == iTimerID)
    //     {
    //         iExist = i;
    //         break;
    //     }
    // }

    // if(iExist < 0)
    // {
    //     my_mutex_unlock(m_xTimerMutex);
    //     return;
    // }

    // for(int i = iExist; i < m_iTimerCount - 1; i ++)
    // {
    //     m_aiTimerIDs[i] = m_aiTimerIDs[i + 1];
    //     m_aiTimerCounter[i] = m_aiTimerCounter[i + 1];
    //     m_aiTimerMsec[i] = m_aiTimerMsec[i + 1];
    //     m_arTimerTick[i] = m_arTimerTick[i + 1];
    // }
    // m_iTimerCount --;
    // my_mutex_unlock(m_xTimerMutex);
}

void WatchTask::ResetTimer(int iTimerID)
{
    // my_mutex_lock(m_xTimerMutex);
    // int iExist = -1;
    // for(int i = 0; i < m_iTimerCount; i ++)
    // {
    //     if(m_aiTimerIDs[i] == iTimerID)
    //     {
    //         iExist = i;
    //         break;
    //     }
    // }

    // if(iExist < 0)
    // {
    //     my_mutex_unlock(m_xTimerMutex);
    //     return;
    // }

    // m_aiTimerCounter[iExist] ++;
    // m_arTimerTick[iExist] = Now();
    // my_mutex_unlock(m_xTimerMutex);
}

int WatchTask::GetCounter(int iTimerID)
{
    // my_mutex_lock(m_xTimerMutex);
    // int iExist = -1;
    // for(int i = 0; i < m_iTimerCount; i ++)
    // {
    //     if(m_aiTimerIDs[i] == iTimerID)
    //     {
    //         iExist = i;
    //         break;
    //     }
    // }

    // if(iExist < 0)
    // {
    //     my_mutex_unlock(m_xTimerMutex);
    //     return -1;
    // }

    // int iRet = 0;
    // iRet = m_aiTimerCounter[iExist];
    // my_mutex_unlock(m_xTimerMutex);

    // return iRet;
    return 0;
}

void WatchTask::ScanBattery(int iSendFlag)
{
}

#if (USE_WATCHDOG)
#include <drv/wdt.h>
extern "C" csi_wdt_t g_wdt;
#endif // USE_WATCHDOG

void WatchTask::run()
{
    int iROKCounter = 0;
    int iClrDarkCounter = 0;
    // float rOldTime = Now();
    my_printf("=========== WatchTask start\n");
#ifdef PSENSE_DET
    int iPowerOnFlag = 0;
    iPowerOnFlag = GPIO_fast_getvalue(PSENSE_DET);
#endif // PSENSE_DET
#if (USE_PRINT_TEMP)
    cvi_tempsen_t tps;
    cvi_tempsen_init(&tps);
    unsigned int temp = 0;
#endif
    while(m_iRunning)
    {
        {
            if(g_xSS.iCurClrGain <= (0xf80 - NEW_CLR_IR_SWITCH_THR))
                iClrDarkCounter = 0;
            else
            {
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
                if (camera_get_actIR() == MIPI_CAM_S2RIGHT)
#endif
                iClrDarkCounter++;
            }
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
            if(iClrDarkCounter)
#else
            if(iClrDarkCounter > 10)
#endif
            {
                iClrDarkCounter = 0;
#if (USE_WHITE_LED == 0)
                g_xSS.iUvcSensor = (DEFAULT_SNR4UVC + 1) % 2;
                uvc_set_reinit_flag();
#else // USE_WHITE_LED
#if (USE_3M_MODE)
                if (USE_3M_MODE == 1 || g_xSS.bUVCRunning)
                    gpio_whiteled_on(ON);
#endif
#endif // USE_WHITE_LED
            }
        }
#ifdef PSENSE_DET
        int bFlag = GPIO_fast_getvalue(PSENSE_DET);
        if (bFlag == 0 && iPowerOnFlag != 0)
        {
            SendGlobalMsg(MSG_SENSE, 0, SENSE_READY_DETECTED, 0);
        }
        iPowerOnFlag = (bFlag != 0);
#endif // PSENSE_DET

        if((iROKCounter % 10) == 0)
        {
            // printf("[ROK] %d\n", (int)Now());
#if (USE_PRINT_TEMP)
            temp = cvi_tempsen_read_temp_mC(&tps, 500);
            if (temp > 0 && temp < 1000000)
                my_printf("******* temper(%08d): %u\n", (int)Now(), temp);
#endif
            if (g_xSS.iUsbHostMode == 0)
            {
                if (g_xSS.rLastSenseCmdTime == 0 &&
                         g_xSS.rAppStartTime != 0 &&
                         Now() - g_xSS.rAppStartTime >= USB_UPGRADE_TIMEOUT * 1000)
                {
#ifdef PSENSE_DET
                    if (bFlag)
#endif
                    {
                        //SendGlobalMsg(MSG_SENSE, 0, OTA_USB_START, 1);
                        g_xSS.iUsbHostMode = 1;
                    }
                }
            }
            else
            {
                if (g_xSS.iStartOta == 0 && g_xSS.rAppStartTime != 0 && Now() - g_xSS.rAppStartTime >= USB_UPGRADE_TIMEOUT * 1000)
                {
#if (USE_USB_CHECKFIRM_MODE)
                    if (!g_xSS.bUVCRunning)
                        g_xSS.bCheckFirmware = 1;
#endif
                    //power off module
                    // SendGlobalMsg(MSG_SENSE, 0, OTA_USB_DETECTED, 1);
                    // my_printf("ota: send OTA_USB_DETECTED\n");
                }
                else if (g_xSS.rAppStartTime != 0 && Now() - g_xSS.rAppStartTime >= USB_DETECT_TIMEOUT * 1000)
                {
#if 0
                    //try mount home
                    if ((iROKCounter % 20) == 0)
                        my_system("mount /dev/sda /home");

                    my_system("ls /home");
                    FILE* _fp = fopen(UPDATE_FIRM_ZIMG_PATH, "rb");
                    if (_fp != NULL)
                    {
                        fclose(_fp);
                        //do upgrade
                        SendGlobalMsg(MSG_SENSE, 0, OTA_USB_DETECTED, 1);
                    }
                    else
                    {
                        my_printf("ota: not found USB data.\n");
                    }
#endif
                }
            }
#if (USE_WHITE_LED)
            if (g_xSS.iFtIRLed && g_xSS.iFtWhiteLed < WLED_TEST_TIMEOUT)
            {
                g_xSS.iFtWhiteLed++;
                if (g_xSS.iFtWhiteLed == WLED_TEST_TIMEOUT)
                    gpio_whiteled_on(0);
            }
#endif
        }

        for(int i = 0; i < 10; i ++)
        {
            my_usleep(10 * 1000);
            if(!m_iRunning)
            {
                my_printf("m_iRunning end\n");
                break;
            }
        }
#if (USE_WATCHDOG)
        csi_wdt_feed(&g_wdt);
#endif
        iROKCounter ++;
    }
    my_printf("=========== WatchTask end\n");
}

void WatchTask::ThreadProc()
{
    run();
}

void* watchTask_ThreadProc1(void* param)
{
    WatchTask* pThread = (WatchTask*)(param);
    pThread->ThreadProc();
    return NULL;
}
