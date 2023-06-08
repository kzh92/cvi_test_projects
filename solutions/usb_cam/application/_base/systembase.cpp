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
//     int iRetryFlag = 0;
// fec_begin:
    dbfs_set_cur_part(DB_PART1);
    int iCurPart = dbfs_get_cur_part();
    //unsigned char fRestoreDB = 0;
//    int fRecover = 0;

    while(iCurPart <= DB_PART_BACKUP)
    {
        int iCheckOk = 0;
        dbug_printf("cur_part=%d,\n", iCurPart);

        iRet = FaceEngine::Create(ENROLL_DUPLICATION_CHECK, g_xSS.iCameraRotate, g_xPS.x.iChecksumDNN, g_xPS.x.iCheckSumH);
        my_printf("1.p = %d, u = %d, %0.3f\n", dbm_GetPersonCount(), g_xCS.x.bUserCount, Now());

        if(iRet != ES_FILE_ERROR && dbm_GetPersonCount() == g_xCS.x.bUserCount)
        {
            if(argc != 1)
            {
                iCheckOk = 1;
                goto exit2;
            }

            if(IsModifyUser())
            {
                int iCheck = 0;

//                my_printf("[DB]: CheckBackupDBInfo: %d\n", iCheck);
                if(iCheck == 0)     //ok
                {
                    iCheckOk = 1;
                    goto exit2;
                }
            }
            else
            {
                iCheckOk = 1;
                goto exit2;
            }
        }
exit2:
#if (N_MAX_HAND_NUM == 0)
        if (iCheckOk)
            goto exit1;
#else // N_MAX_HAND_NUM == 0
        if (iCheckOk || iCurPart == DB_PART_BACKUP)
        {
            iRet = FaceEngine::CreateHand(ENROLL_DUPLICATION_CHECK, g_xSS.iCameraRotate, g_xPS.x.iChecksumDNN, g_xPS.x.iCheckSumH);
            my_printf("2.h = %d, u = %d, %d.\n", dbm_GetHandCount(), g_xCS.x.bHandCount, iCheckOk);

            if(iRet != ES_FILE_ERROR && dbm_GetHandCount() == g_xCS.x.bHandCount)
            {
                if(argc != 1)
                    goto exit1;

                if(IsModifyUser())
                {
                    // int iCheck = 0;

                    // mount_backup_db(MS_RDONLY | MS_NOATIME);
                    // mount_backup_db();
                    // iCheck = dbm_CheckHandBackupDBInfos();
                    // umount_backup_db();

    //                printf("[DB]: CheckBackupDBInfo: %d\n", iCheck);
                    // if(iCheck == 0)     //ok
                    //     goto exit1;
                    goto exit1;
                }
                else
                    goto exit1;
            }
        }
#endif // N_MAX_HAND_NUM == 0

        my_printf("[Init] create file error!\n");

        // if(iRet != ES_FILE_ERROR && dbm_GetPersonCount() != g_xCS.x.bUserCount)
        // {
        //     MY_ALL_SETTINGS xCsBak;
        //     M24C64_GetBackupCS((unsigned char*)&xCsBak);
        //     if (xCsBak.x.bCheckSum == GetSettingsCheckSum((unsigned char*)&xCsBak, sizeof(xCsBak)) &&
        //             dbm_GetPersonCount() == xCsBak.x.bUserCount)
        //     {
        //         my_printf("[%s] UpdateUserCount %d\n", __FUNCTION__, dbm_GetPersonCount());
        //         UpdateUserCount();
        //         goto exit1;
        //     }
        // }

#if (N_MAX_HAND_NUM)
        // if(iRet != ES_FILE_ERROR && dbm_GetHandCount() != g_xCS.x.bHandCount)
        // {
        //     MY_ALL_SETTINGS xCsBak;
        //     M24C64_GetBackupCS((unsigned char*)&xCsBak);
        //     if (xCsBak.x.bCheckSum == GetSettingsCheckSum((unsigned char*)&xCsBak, sizeof(xCsBak)) &&
        //             dbm_GetPersonCount() == xCsBak.x.bHandCount)
        //     {
        //         printf("[%s] uhc %d\n", __FUNCTION__, dbm_GetHandCount());
        //         UpdateUserCount();
        //         goto exit1;
        //     }
        // }
#endif // N_MAX_HAND_NUM

        if (iCurPart >= DB_PART_BACKUP)
            break;

        iCurPart++;
        dbfs_set_cur_part(DB_PART_BACKUP);
    }

    if(dbm_GetPersonCount() == 0 && g_xSS.iWakeupByFunc == 0 && !IsModifyUser())
    {
        //contine to boot.
    }

    UpdateUserCount();
exit1:

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
