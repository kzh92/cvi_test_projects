#include "systembase.h"
#include "common_types.h"
#include "i2cbase.h"
#include "settings.h"
#include "shared.h"
#include "mount_fs.h"
#include "engineparam.h"
#include "DBManager.h"
//#include "lcdtask.h"
#include "faceengine.h"
//#include "mainbackproc.h"
//#include "soundbase.h"
#include "camerasurface.h"
//#include "my_lang.h"
#include "upgradebase.h"
#include "countbase.h"
#include "uartcomm.h"
#include "drv_gpio.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/mount.h>
#include <string.h>

void OpenMoto(int iPlay, int iTesting);
void CloseMoto(int iPlay, int iTesting);

int face_engine_create(int argc)
{
    //engine create

    int iRet = 0;
    int iRetryFlag = 0;
fec_begin:
    dbfs_set_cur_part(DB_PART1);
    int iCurPart = dbfs_get_cur_part();
    //unsigned char fRestoreDB = 0;
//    int fRecover = 0;

    while(iCurPart <= DB_PART_BACKUP)
    {
        my_printf("cur_part=%d, %0.3f\n", iCurPart, Now());
        iRet = FaceEngine::Create(g_xCS.x.bDupCheck, g_xPS.x.bCamFlip, g_xPS.x.iChecksumDNN, g_xPS.x.iCheckSumH);
        my_printf("1.p = %d, u = %d, %0.3f\n", dbm_GetPersonCount(), g_xCS.x.bUserCount, Now());

        if(iRet != ES_FILE_ERROR && dbm_GetPersonCount() == g_xCS.x.bUserCount)
        {
            if(argc != 1)
                goto exit1;

            if(IsModifyUser())
            {
                int iCheck = 0;

#if 0 //kkk
                mount_backup_db(MS_RDONLY | MS_NOATIME);
                iCheck = CheckBackupDBInfos();
                umount_backup_db();
#endif

                my_printf("[DB]: CheckBackupDBInfo: %d\n", iCheck);
                if(iCheck == 0)     //ok
                    goto exit1;
            }
            else
                goto exit1;
        }

        my_printf("[Init] create file error!\n");

        if(iRet != ES_FILE_ERROR && dbm_GetPersonCount() != g_xCS.x.bUserCount)
        {
            COMMON_SETTINGS xCsBak;
            M24C64_GetBackupCS((unsigned char*)&xCsBak);
            if (xCsBak.x.bCheckSum == GetSettingsCheckSum((unsigned char*)&xCsBak, sizeof(xCsBak)) &&
                    dbm_GetPersonCount() == xCsBak.x.bUserCount)
            {
                my_printf("[%s] UpdateUserCount %d\n", __FUNCTION__, dbm_GetPersonCount());
                UpdateUserCount();
                goto exit1;
            }
        }

#if 0 //kkk
        if (iCurPart >= DB_PART_BACKUP)
            break;

        FaceEngine::Release();

        //fRecover = 1;

        if (fRestoreDB == 0 && restore_dbfs() == 0)
        {
            fRestoreDB = 1;
        }
//        else
//        {
//            if (fRestoreDB == 0)
//                my_printf("[Main]02 restore db failed.\n");
//            else
//                my_printf("[Main]02 restore db done before.\n");
//        }

        iRet = FaceEngine::Create(g_xCS.x.bDupCheck, g_xPS.x.bCamFlip, g_xPS.x.iChecksumDNN, g_xPS.x.iCheckSumH);

        my_printf("2.p = %d, u = %d.\n", dbm_GetPersonCount(), g_xCS.x.bUserCount);

        if(iRet != ES_FILE_ERROR && dbm_GetPersonCount() == g_xCS.x.bUserCount)
            goto exit1;

        FaceEngine::Release();

#if (DB_TYPE == TYPE_EXT4)
        if (!do_make_ext4(dbfs_get_cur_part_name()))
        {
            if (mount_dbfs(dbfs_get_cur_part()) == 0)
            {
                if (restore_dbfs() == 0)
                {
                    iRet = FaceEngine::Create("/db", 1, NULL, g_xEP.fUpdate, g_xEP.arThreshold, g_xEP.iEngineType == 2, g_xEP.fUpdateA, g_xEP.iMotionOffset, (g_xEP.iRemoveNoise << 2) | (g_xEP.iRemoveGlass << 1) | g_xEP.iMotionFlag,
                                              g_xEP.iMinUserLum, g_xEP.iMaxUserLum, g_xEP.iSatThreshold, g_xCS.x.bDupCheck);

                    my_printf("[Main]02-1 PersonCount = %d, nUserCount = %d.\n", dbm_GetPersonCount(), g_xCS.x.bUserCount);
                }
                else
                    my_printf("[Main]02-1 restore db failed.\n");

                //my_printf("Time02-1=%0.3f,\n", Now());

                if(iRet != ES_FILE_ERROR && dbm_GetPersonCount() == g_xCS.x.bUserCount)
                    goto exit1;
            }
        }
#endif // DB_TYPE == TYPE_EXT4
        iCurPart++;
        fRestoreDB = 0;

        FaceEngine::Release();
        umount_dbfs();

        if (iCurPart < DB_PART_BACKUP)
        {
            if (mount_dbfs(iCurPart) == 1)
            {
                return 1;//poweroff
            }
            if (restore_dbfs() == 0)
                fRestoreDB = 1;
        }
        else
        {
            if (mount_backupdbfs())
            {
                return 1; //poweroff
            }
        }
#endif
        //iCurPart = dbfs_get_cur_part();
        if (iRetryFlag == 1)
            break;
        iCurPart++;
        dbfs_set_cur_part(DB_PART_BACKUP);
    }

    if(dbm_GetPersonCount() == 0 && g_xSS.iWakeupByFunc == 0 && !IsModifyUser())
    {
        if(iRetryFlag == 0)
        {
            iRetryFlag = 1;
            goto fec_begin;
        }
    }

    UpdateUserCount();
exit1:

    return 0;
}

#ifndef __RTK_OS__
int face_engine_create_again()
{
    umount_dbfs();
    try_mount_dbfs();

    int iRet;
    int iCurPart = dbfs_get_cur_part();

    while(iCurPart <= DB_PART_BACKUP)
    {
        my_printf("cur_part=%d,\n", iCurPart);

        iRet = FaceEngine::Create(g_xCS.x.bDupCheck, g_xPS.x.bCamFlip, g_xPS.x.iChecksumDNN, g_xPS.x.iCheckSumH);

        my_printf("[Main]01 PersonCount = %d, nUserCount = %d.\n", dbm_GetPersonCount(), g_xCS.x.bUserCount);

        if(iRet != ES_FILE_ERROR && dbm_GetPersonCount() == g_xCS.x.bUserCount)
            goto exit1;

        if (iCurPart >= DB_PART_BACKUP)
            break;

        FaceEngine::Release();

        iCurPart++;

        umount_dbfs();

        if (iCurPart < DB_PART_BACKUP)
        {
            if (mount_dbfs(iCurPart) == 1)
            {
                return 1;//poweroff
            }
        }
        else
        {
            if (mount_backupdbfs())
            {
                return 1; //poweroff
            }
        }
        iCurPart = dbfs_get_cur_part();
    }

exit1:

    return 0;
}
#endif // !__RTK_OS__

int RTCInit(int argc)
{
    //print cur date
    DATETIME_32 xKey = { 0 };
#if 0
    DATETIME_32 xNow = { 0 };
    if(argc == 1)
    {
        xNow.i = MainSTM_GetRTC();
        xKey = xNow;
        SetCurDateTime(xNow, 0);
    }
    else
    {
        xNow = dbm_GetCurDateTime();
        xKey.i = MainSTM_GetRTC();
    }

    if(xNow.x.iYear + 2000 < 2015)
    {
        xNow.x.iYear = 2015 - 2000;
        xNow.x.iMon = 1 - 1;
        xNow.x.iDay = 1;
        xNow.x.iHour = 0;
        xNow.x.iMin = 0;
        xNow.x.iSec = 0;

        SetCurDateTime(xNow, 1);
    }

    srand (time(NULL));
#endif

    return xKey.i;
}

int RTCInit()
{
    DATETIME_32 xSetDate;
    xSetDate.x.iYear = 2015 - 2000;
    xSetDate.x.iMon = 1 - 1;
    xSetDate.x.iDay = 1;
    xSetDate.x.iHour = 0;
    xSetDate.x.iMin = 0;
    xSetDate.x.iSec = 0;

    SetCurDateTime(xSetDate, 1);

    return 0;
}

void ResetTmp(int iUpgradeFlag)
{
    do_reset_tmp();
    if(iUpgradeFlag == 0)
    {
        my_system("rm -rf /db/*");

        mount_backup_db(0);
        my_system("rm -rf /backup/*");
        umount_backup_db();
    }
}

// void StartBattLog()
// {
//     return;
// }

// void EndBattLog()
// {
//     return;
// }



void MotorCmd()
{
#if 0
    if(g_xSS.iMotorCmd == MAIN_STM_CMD_OPEN)
    {
        MainSTM_Motor(g_xSS.iMotorCmd, g_xPS.x.bSemiMotorPolarity, g_xCS.x.bLockType, g_xSS.bSound, g_xCS.x.bLang,
                                   g_xZD.xWifiControl_Motor.iID, g_xZD.xWifiControl_Motor.iData, g_xSS.iMotorAck);
    }
    else if(g_xSS.iMotorCmd == MAIN_STM_CMD_CLOSE)
    {
        MainSTM_Motor(g_xSS.iMotorCmd, g_xPS.x.bSemiMotorPolarity, g_xCS.x.bLockType, g_xSS.bSound, g_xCS.x.bLang,
                                   g_xZD.xWifiControl_Motor.iID, g_xZD.xWifiControl_Motor.iData, g_xSS.iMotorAck);
    }
    else if(g_xSS.iMotorCmd == MAIN_STM_CMD_OPEN_CLOSE)
    {
        MainSTM_Motor(g_xSS.iMotorCmd, g_xPS.x.bSemiMotorPolarity, g_xCS.x.bLockType, g_xSS.bSound, g_xCS.x.bLang,
                                   g_xZD.xWifiControl_Motor.iID, g_xZD.xWifiControl_Motor.iData, g_xSS.iMotorAck);
    }
#endif
}

// static unsigned short Little16ToBig16(unsigned short sVal)
// {
//     return (sVal & 0xFF) << 8 | ((sVal >> 8) & 0xFF);
// }

void LedConfig()
{
#if 0
    GPIO_fast_config(RLED, OUT);
    GPIO_fast_config(GLED, OUT);
    GPIO_fast_config(BLED, OUT);
    SetLed(0);
#endif
}

void SetLed(int iLedPort)
{
#if 0
    GPIO_fast_setvalue(GLED, ON);
    GPIO_fast_setvalue(BLED, ON);
    GPIO_fast_setvalue(RLED, ON);

    if(iLedPort)
        GPIO_fast_setvalue(iLedPort, OFF);
#endif
}


//void SetStatus(int iStatus)
//{
//    g_xSS.iSystemState = iStatus;
//}
