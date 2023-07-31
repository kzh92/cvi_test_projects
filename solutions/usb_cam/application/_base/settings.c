#include "settings.h"
#include "appdef.h"
#include "i2cbase.h"
#ifndef _APP_UPGRADE
//#include "DBManager.h"
#include "shared.h"
//#include "countbase.h"
#endif // !_APP_UPGRADE
#include "uartcomm.h"
#include <string.h>

MY_ALL_SETTINGS         g_xAS = {0};
SYSTEM_STATE            g_xSS = { 0 };

mymutex_ptr g_uvcWindowLocker = 0;

/**
 * @brief GetSettingsCheckSum: 설정값의 CheckSum을 구현다.
 * @param pbData
 * @return
 */
unsigned char GetSettingsCheckSum(unsigned char* pbData, int iSize)
{
    int iCheckSum = 0;
    for(int i = 0; i < iSize - 1; i ++)
        iCheckSum += pbData[i];

    iCheckSum = 0xFF - (iCheckSum & 0xFF);
    return (unsigned char)iCheckSum;
}

/**
 * @brief RestoreBackupSettings: Backup한 일반설정정보로부터 일반설정정보를 복귀한다.
 */
void RestoreMyAllBackupSettings()
{
    MY_ALL_SETTINGS xBackupCS;
    my_memset(&xBackupCS, 0, sizeof(xBackupCS));
    int ret = my_settings_read(ADDR_MY_ALL_SS_BAK, &xBackupCS, sizeof(xBackupCS));
    if (ret < sizeof(xBackupCS))
    {
        my_printf("[%s]read fail.\n", __func__);
        return;
    }

    //Backup한 일반설정정보의 CheckSum이 틀릴때 Default설정으로 초기화한다.
    if(xBackupCS.x.bCheckSum != GetSettingsCheckSum(xBackupCS.a, sizeof(xBackupCS)))
    {
        ResetMyAllSettings(&xBackupCS);
    }

    memcpy(&g_xAS, &xBackupCS, sizeof(MY_ALL_SETTINGS));

    UpdateMyAllSettings();
}

/**
 * @brief ReadCommonSettings: 읽반설정을 24C64에서 읽는다.
 */
int ReadMyAllSettings()
{
    int ret = my_settings_read(ADDR_MY_ALL_SS, &g_xAS, sizeof(g_xAS));
    if (ret < sizeof(g_xAS))
        return -1;

    //읽반설정의 CheckSum이 틀릴때 Backup한 CS자료로부터 불러들인다.
    if(g_xAS.x.bCheckSum != GetSettingsCheckSum(g_xAS.a, sizeof(g_xAS)))
        RestoreMyAllBackupSettings();

    return 0;
}

/**
 * @brief UpdateBackupSettings: Backup구역에 일반설정정보를 쓰기한다.
 */
void UpdateMyAllBackupSettings()
{
    my_settings_write(ADDR_MY_ALL_SS_BAK, &g_xAS, sizeof(g_xAS));
}

/**
 * @brief UpdateCommonSettings: 일반설정을 24C64에 쓰기한다.
 */
void UpdateMyAllSettings()
{
    g_xAS.x.bCheckSum = GetSettingsCheckSum(g_xAS.a, sizeof(g_xAS));
    my_settings_write(ADDR_MY_ALL_SS, &g_xAS, sizeof(g_xAS));

    UpdateMyAllBackupSettings();
}

/**
 * @brief ResetCommonSettings: 일반설정을 Default설정으로 초기화한다.
 */
void ResetMyAllSettings()
{
    MY_ALL_SETTINGS xTemp = g_xAS;
    my_memset(&g_xAS, 0, sizeof(g_xAS) - 1);
    ResetCS(&g_xCS);
    ResetPermanenceSettings();
    ResetEncryptSettings();
    if (memcmp(&xTemp, &g_xAS, sizeof(g_xAS) - 1))
        UpdateMyAllSettings();
}

/**
 * @brief ReadPermanenceSettings: 영구설정정보를 24C64에서 읽는다. 영구설정값의 CheckSum이 틀리면 Default설정값을 리용한다.
 */
void ReadPermanenceSettings()
{
    M24C64_GetPS((unsigned char*)&g_xPS);

    if(g_xPS.x.bCheckSum != GetSettingsCheckSum(g_xPS.a, sizeof(g_xPS.a)))       //영구설정값의 CheckSum이 틀리면 Default설정값을 리용하게 함
//        ResetPermanenceSettings();
        memset(&g_xPS, 0, sizeof(g_xPS));
}

/**
 * @brief UpdatePermanenceSettings: 영구설정값을 24C64에 쓰기한다. 값을 쓰기전에 CheckSum을 구해서 반영한다.
 */
void UpdatePermanenceSettings()
{
    UpdateMyAllSettings();
}

/**
 * @brief ResetPermanenceSettings: 영구설정값을 Default설정으로 초기화한다.
 */
void ResetPermanenceSettings()
{
    //!!!CAUTION: DO NOT reset all members,
    ///you should not reset dic checksum values.
    //memset(&g_xPS, 0, sizeof(g_xPS));

    g_xPS.x.bCamFlip = CAM_ROTATION_MODE;
    g_xPS.x.bEnableLogFile = 0;
    g_xPS.x.bHijackEnable = 0;
#if (USE_TWIN_ENGINE)
    g_xPS.x.bTwinsMode = S_VERIFY_LEVEL_DEFAULT;
#endif
    UpdatePermanenceSettings();
}

/**
 * @brief ReadCommonSettings: 읽반설정을 24C64에서 읽는다.
 */
int ReadCommonSettings()
{
    int iRet = M24C64_GetCS((unsigned char*)&g_xCS);
    if(iRet < 0)
        return iRet;

    //읽반설정의 CheckSum이 틀릴때 Backup한 CS자료로부터 불러들인다.
    if(g_xCS.x.bCheckSum != GetSettingsCheckSum(g_xCS.a, sizeof(g_xCS.a)))
        RestoreBackupSettings();

    return 0;
}

/**
 * @brief UpdateCommonSettings: 일반설정을 24C64에 쓰기한다.
 */
void UpdateCommonSettings()
{
    UpdateMyAllSettings();
}

/**
 * @brief ResetCommonSettings: 일반설정을 Default설정으로 초기화한다.
 */
void ResetCommonSettings()
{
    ResetCS(&g_xCS);
    UpdateCommonSettings();
}

/**
 * @brief RestoreBackupSettings: Backup한 일반설정정보로부터 일반설정정보를 복귀한다.
 */
void RestoreBackupSettings()
{
}

/**
 * @brief UpdateBackupSettings: Backup구역에 일반설정정보를 쓰기한다.
 */
void UpdateBackupSettings()
{
    g_xCS.x.bCheckSum = GetSettingsCheckSum(g_xCS.a, sizeof(g_xCS.a));
    M24C64_SetBackupCS((unsigned char*)&g_xCS);
}

/**
 * @brief ResetCS: 일반설정정보를 Default설정으로 만든다.
 * @param pxCS
 */
void ResetCS(MY_ALL_SETTINGS* pxCS)
{
    pxCS->x.bPresentation = DEFAULT_PRESENATATION;
    pxCS->x.bSecureFlag = DEFAULT_SECURE_VALUE;
}

#ifndef _APP_UPGRADE

/**
 * @brief ReadHeadInfos: 24C64의 머리부정보를 읽는다.
 */
void ReadHeadInfos()
{
    int ret = M24C64_GetHD((unsigned char*)&g_xHD);
    if (ret < 0)
        return;

    //머리부정보의 CheckSum이 틀리면 Head정보를 초기화한다.
    if(g_xHD.x.bCheckSum != GetSettingsCheckSum(g_xHD.a, sizeof(g_xHD.a)))
    {
        RestoreBackupHeadInfos();
    }
}

/**
 * @brief UpdateHeadInfos: Head정보를 24C64에 쓰기한다.
 */
void UpdateHeadInfos()
{
    UpdateMyAllSettings();
}

/**
 * @brief ResetHeadInfos: Head정보를 Default설정으로 초기화한다.
 */
void ResetHeadInfos()
{
    ResetHeadInfoParams(&g_xHD);
    // UpdateHeadInfos();
}

/**
 * @brief ResetHeadInfoParams: Head정보를 Default설정으로 초기화한다.
 */
void ResetHeadInfoParams(MY_ALL_SETTINGS* pHD)
{
    //!!!CAUTION: DO NOT reset all members,
    ///you should not reset cpu id values.
    //memset(&g_xHD, 0, sizeof(HEAD_INFO));
}

/**
 * @brief RestoreBackupSettings: Backup한 일반설정정보로부터 일반설정정보를 복귀한다.
 */
void RestoreBackupHeadInfos()
{
}

/**
 * @brief UpdateBackupSettings: Backup구역에 일반설정정보를 쓰기한다.
 */
void UpdateBackupHeadInfos()
{
    g_xHD.x.bCheckSum = GetSettingsCheckSum(g_xHD.a, sizeof(g_xHD.a));
    M24C64_SetBackupHD((unsigned char*)&g_xHD);
}

void ReadHeadInfos2()
{
    M24C64_GetHD2((unsigned char*)&g_xHD2);

    //머리부정보의 CheckSum이 틀리면 Head정보를 초기화한다.
    if(g_xHD2.x.bCheckSum != GetSettingsCheckSum(g_xHD2.a, sizeof(g_xHD2.a)))
        ResetHeadInfos2();
}

void UpdateHeadInfos2()
{
    UpdateMyAllSettings();
}

/**
 * @brief ResetHeadInfos: Head정보를 Default설정으로 초기화한다.
 */
void ResetHeadInfos2()
{
}

void ReadROKLogs()
{
}

void ResetROKLogs()
{
    memset(&g_xROKLog, 0, sizeof(g_xROKLog));
    g_xROKLog.x.bMountPoint = DB_PART1;
}

void UpdateROKLogs()
{
    UpdateMyAllSettings();
}

/**
 * @brief ReadEncryptSettings:
 */
int ReadEncryptSettings()
{
    int iRet = M24C64_GetES((unsigned char*)&g_xES);
    if(iRet < 0)
        return iRet;

    //읽반설정의 CheckSum이 틀릴때 Backup한 CS자료로부터 불러들인다.
    if(g_xES.x.bCheckSum != GetSettingsCheckSum(g_xES.a, sizeof(g_xES.a)))
        ResetEncryptSettings();

    return 0;
}

/**
 * @brief UpdateEncryptSettings: 일반설정을 24C64에 쓰기한다.
 */
void UpdateEncryptSettings()
{
    UpdateMyAllSettings();
}

/**
 * @brief ResetEncryptSettings: 일반설정을 Default설정으로 초기화한다.
 */
void ResetEncryptSettings()
{
    // set values that indicates unset.
    memset(&g_xES.x.bEncKeyPos, 0xff, sizeof(g_xES.x.bEncKeyPos));
    UpdateEncryptSettings();
}

/**
 * @brief ResetSystemState: 체계상태를 초기화한다.
 * @param iAppType: 0: 인식App, 1: 설정App
 */
void ResetSystemState(int iAppType)
{
    memset(&g_xSS, 0, sizeof(g_xSS));

    g_xSS.iAppType = iAppType;
    g_xSS.iVerifyFailType = 0xFF;
    g_xSS.iCameraRotate = g_xPS.x.bCamFlip;
    g_xSS.iUsbHostMode = g_xCS.x.bUsbHost;
    g_xSS.iSendLastMsgMode = SEND_LAST_MSG;
    g_xSS.iCapWidth = CAPTURE_WIDTH;
    g_xSS.iCapHeight = CAPTURE_HEIGHT;
    g_xSS.iUvcDirect = DEFAULT_UVC_DIR;
    g_xSS.iRegisterMixMode = ENROLL_FACE_HAND_MODE;
}

void SetMountStatus(int iSuccess)
{
/*
    int iNeedSave = 1; // this must always be saved
    if (iSuccess != 0)
    {
        if (g_xROKLog.x.bMountStatus != 0xAA)
        {
            g_xROKLog.x.bMountStatus = 0xAA;
            iNeedSave = 1;
        }
        if (g_xROKLog.x.bMountRetry != 0)
        {
            g_xROKLog.x.bMountRetry = 0;
            iNeedSave = 1;
        }
    }
    else
    {
        if (g_xROKLog.x.bMountStatus != 0x55)
        {
            g_xROKLog.x.bMountStatus = 0x55;
            iNeedSave = 1;
        }
    }
    if (iNeedSave)
        UpdateROKLogs();
*/
}

void SetModifyUser(int iModify)
{
    if(iModify)
    {
        if (g_xHD2.x.bModifyUser != 0x55)
        {
            g_xHD2.x.bModifyUser = 0x55;
            UpdateHeadInfos2();
        }
    }
    else
    {
        if (g_xHD2.x.bModifyUser != 0xAA)
        {
            g_xHD2.x.bModifyUser = 0xAA;
            UpdateHeadInfos2();
        }
    }
}


int IsModifyUser()
{
    return (g_xHD2.x.bModifyUser == 0x55) ? 1 : 0;
}

#endif // !_APP_UPGRADE

int GetIntCheckSum(int* piData, int iSize)
{
    int iCheckSum = 0;
    for(int i = 0; i < iSize / (int)sizeof(int); i ++)
        iCheckSum = iCheckSum + piData[i];

    iCheckSum = ~iCheckSum;

    return iCheckSum;
}

int setUvcWindow(int width, int height)
{
    int ret = 0;
    dbug_printf("[%s] start %d, %d\n", __func__, width, height);
#ifndef _APP_UPGRADE
    if (width < UVC_MIN_WIDTH || height < UVC_MIN_HEIGHT || width > UVC_MAX_WIDTH || height > UVC_MAX_HEIGHT)
        return 0;
    lockUvcWindow();
    ret = (g_xSS.iUvcWidth != width) || (g_xSS.iUvcHeight != height);
    g_xSS.iUvcWidth = width;
    g_xSS.iUvcHeight = height;
    unlockUvcWindow();
#endif // _APP_UPGRADE
    return ret;
}

void lockUvcWindow()
{
    if (g_uvcWindowLocker == 0)
        g_uvcWindowLocker = my_mutex_init();
    my_mutex_lock(g_uvcWindowLocker);
}

void unlockUvcWindow()
{
    if (g_uvcWindowLocker == 0)
        g_uvcWindowLocker = my_mutex_init();
    my_mutex_unlock(g_uvcWindowLocker);
}

extern void    fr_SetStopEngineFlag(int);

void ClearSenseResetFlag()
{
#ifndef _APP_UPGRADE
    g_xSS.iResetFlag = 0;
#ifndef _NO_ENGINE_
    fr_SetStopEngineFlag(0);
#endif
#endif
}

void MarkSenseResetFlag()
{
#ifndef _APP_UPGRADE
    g_xSS.iResetFlag = 1;
    g_xSS.rResetFlagTime = Now();
    g_xSS.iDemoMode = N_DEMO_VERIFY_MODE_OFF;
#ifndef _NO_ENGINE_
    fr_SetStopEngineFlag(1);
#endif
#endif //! _APP_UPGRADE
}
