#include "watchtask.h"
#include "unistd.h"
#include "drv_gpio.h"
#include "appdef.h"
#include "shared.h"
#include "msg.h"
//#include "playthread.h"
#include "i2cbase.h"
#include "mount_fs.h"
//#include "lcdtask.h"
#include "DBManager.h"
//#include "uarttask.h"
//#include "mainbackproc.h"
#include "senselocktask.h"
#include "upgradebase.h"

// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <time.h>
#include <string.h>
// #include <math.h>
// #include <fcntl.h>

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
    if (my_thread_create_ext(&m_thread, NULL, watchTask_ThreadProc1, this, (char*)"wdt", 8192, MYTHREAD_PRIORITY_MEDIUM))
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

#include <drv/wdt.h>
extern "C" csi_wdt_t g_wdt;

void WatchTask::run()
{
    int iROKCounter = 0;
    float rOldTime = Now();
    my_printf("=========== WatchTask start\n");
#ifdef PSENSE_DET
    int iPowerOnFlag = 0;
    iPowerOnFlag = YAOYANG_MODE ? (!GPIO_fast_getvalue(PSENSE_DET)) : GPIO_fast_getvalue(PSENSE_DET);
#endif // PSENSE_DET

    while(m_iRunning)
    {
        if (Now() - rOldTime > 300)
        {
            csi_wdt_feed(&g_wdt);
            dbug_printf("[%d]ROK\n", (int)Now());
            rOldTime = Now();
            my_usleep(20*1000);
        }
        float rNow = Now();
        my_mutex_lock(m_xTimerMutex);
        for(int i = 0; i < m_iTimerCount; i ++)
        {
            if(rNow - m_arTimerTick[i] > m_aiTimerMsec[i] * 1000)
            {
                m_arTimerTick[i] = rNow;
//                SendGlobalMsg(MSG_WATCH, WATCH_TYPE_TIMER, m_aiTimerIDs[i], m_aiTimerCounter[i]);
            }
        }
        my_mutex_unlock(m_xTimerMutex);
#ifdef PSENSE_DET
        int bFlag = YAOYANG_MODE ? !GPIO_fast_getvalue(PSENSE_DET): GPIO_fast_getvalue(PSENSE_DET);
        if (bFlag == 0 && iPowerOnFlag != 0)
        {
            SendGlobalMsg(MSG_SENSE, 0, SENSE_READY_DETECTED, 0);
        }
        iPowerOnFlag = (bFlag != 0);
#endif // PSENSE_DET

        if((iROKCounter % 10) == 0)
        {
            if (g_xSS.iUsbHostMode == 0)
            {
#ifdef GPIO_USBSense
                int iFlagUSB = GPIO_fast_getvalue(GPIO_USBSense);
#endif
                if (g_xSS.rLastSenseCmdTime == 0 &&
                         g_xSS.rAppStartTime != 0 &&
                         Now() - g_xSS.rAppStartTime >= USB_UPGRADE_TIMEOUT * 1000)
                {
#if defined(PSENSE_DET) && defined(GPIO_USBSense)
                    if (iFlagUSB == 0 && bFlag == 1)
#elif defined(PSENSE_DET)
                    if (bFlag == 1)
#endif
                    {
#if (ROOTFS_BAK)
                    if (get_cur_rootfs_part() == RFS_PART0)
                        SendGlobalMsg(MSG_SENSE, 0, OTA_USB_START, 1);
                    else
                        SendGlobalMsg(MSG_SENSE, 0, OTA_USB_START, 0);
#else // ROOTFS_BAK
                        //SendGlobalMsg(MSG_SENSE, 0, OTA_USB_START, 1);
                        g_xSS.iUsbHostMode = 1;
#endif
                    }
                }
            }
            else
            {
                if (g_xSS.iStartOta == 0 && g_xSS.rAppStartTime != 0 && Now() - g_xSS.rAppStartTime >= USB_UPGRADE_TIMEOUT * 1000)
                {
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
            // PrintFreeMem();
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
