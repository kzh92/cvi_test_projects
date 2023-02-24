#include "settings.h"
#include "appdef.h"
#include "i2cbase.h"
#ifndef _APP_UPGRADE
//#include "DBManager.h"
#include "shared.h"
//#include "countbase.h"
#endif // !_APP_UPGRADE
#include "uartcomm.h"
// #include <unistd.h>
// #include <stdio.h>
// #include <memory.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <stdlib.h>
#include <string.h>
// #include <time.h>


PERMANENCE_SETTINGS     g_xPS = { 0 };
COMMON_SETTINGS         g_xCS = { 0 };
#ifndef _APP_UPGRADE
ROK_LOG                 g_xROKLog = { 0 };
HEAD_INFO               g_xHD = { 0 };
HEAD_INFO2              g_xHD2 = { 0 };
SYSTEM_STATE            g_xSS = { 0 };
//TEST_RESULT             g_xTR = { 0 };
//RECOVERY_HADER          g_xRecvHdr = { 0 };
ENCRYPT_SETTINGS        g_xES = { 0 };
#endif // !_APP_UPGRADE

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
 * @brief ReadPermanenceSettings: 영구설정정보를 24C64에서 읽는다. 영구설정값의 CheckSum이 틀리면 Default설정값을 리용한다.
 */
void ReadPermanenceSettings()
{
    M24C64_GetPS((unsigned char*)&g_xPS);

//    g_iDebugEn = g_xPS.x.bDebugEn;
//    if(g_iDebugEn == 1)
//    {
//        unsigned char* pbData = (unsigned char*)&g_xPS;
//        LOG_PRINT("[I2C] Read 0x%0*x: ", 2, ADDR_PS);
//        for (int i = 0; i < sizeof(g_xPS); i++)
//        {
//            LOG_PRINT("%0*x, ", 2, pbData[i]);
//        }
//        LOG_PRINT("\n\r");
//    }

    if(g_xPS.x.bCheckSum != GetSettingsCheckSum(g_xPS.a, sizeof(g_xPS.a)))       //영구설정값의 CheckSum이 틀리면 Default설정값을 리용하게 함
        ResetPermanenceSettings();
}

/**
 * @brief UpdatePermanenceSettings: 영구설정값을 24C64에 쓰기한다. 값을 쓰기전에 CheckSum을 구해서 반영한다.
 */
void UpdatePermanenceSettings()
{
    g_xPS.x.bCheckSum = GetSettingsCheckSum(g_xPS.a, sizeof(g_xPS.a));
    M24C64_SetPS((unsigned char*)&g_xPS);
}

/**
 * @brief ResetPermanenceSettings: 영구설정값을 Default설정으로 초기화한다.
 */
void ResetPermanenceSettings()
{
    //!!!CAUTION: DO NOT reset all members,
    ///you should not reset dic checksum values.
    //memset(&g_xPS, 0, sizeof(g_xPS));

    g_xPS.x.bDebugEn = DEBUG_EN;
    g_xPS.x.bCamFlip = CAM_ROTATION_MODE;
    g_xPS.x.bSendLastMsg = SEND_LAST_MSG;
    g_xPS.x.bUvcBitrateDefault = DEFAULT_UVC_COMP_PARAM_BT_DEF;
    g_xPS.x.bUvcBitrateMax = DEFAULT_UVC_COMP_PARAM_BT_MAX;
    g_xPS.x.bUvcImageQuality = DEFAULT_UVC_COMP_PARAM_IMQ;
    g_xPS.x.bUvcRepeatFrame = DEFAULT_UVC_COMP_PARAM_RPFR;
    g_xPS.x.bEnableLogFile = 0;
    g_xPS.x.bHijackEnable = 0;
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
    g_xCS.x.bCheckSum = GetSettingsCheckSum(g_xCS.a, sizeof(g_xCS.a));
    M24C64_SetCS((unsigned char*)&g_xCS);

    //일반설정을 쓰기할때 Backup구역에 정보를 중복으로 보관한다.
    UpdateBackupSettings();
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
    COMMON_SETTINGS xBackupCS = { 0 };
    M24C64_GetBackupCS((unsigned char*)&xBackupCS);

    //Backup한 일반설정정보의 CheckSum이 틀릴때 Default설정으로 초기화한다.
    if(xBackupCS.x.bCheckSum != GetSettingsCheckSum(xBackupCS.a, sizeof(xBackupCS.a)))
        ResetCS(&xBackupCS);

    memcpy(&g_xCS, &xBackupCS, sizeof(COMMON_SETTINGS));

    UpdateCommonSettings();
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
void ResetCS(COMMON_SETTINGS* pxCS)
{
    memset(pxCS, 0, sizeof(COMMON_SETTINGS));
    pxCS->x.bPresentation = DEFAULT_PRESENATATION;
    pxCS->x.bDupCheck = ENROLL_DUPLICATION_CHECK;
    pxCS->x.bUVCDir = DEFAULT_UVC_DIR;
    pxCS->x.bLivenessMode = DEFAULT_LIVENESS_MODE;
    pxCS->x.bTwinsMode = DEFAULT_TWINS_MODE;
    pxCS->x.bSecureFlag = DEFAULT_SECURE_VALUE;
    pxCS->x.bProtoEncMode = DEFAULT_PROTO_ENC_MODE;
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
    g_xHD.x.bCheckSum = GetSettingsCheckSum(g_xHD.a, sizeof(g_xHD.a));
    M24C64_SetHD((unsigned char*)&g_xHD);
    UpdateBackupHeadInfos();
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
void ResetHeadInfoParams(HEAD_INFO* pHD)
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
    HEAD_INFO xBackupHD = { 0 };
    M24C64_GetBackupHD((unsigned char*)&xBackupHD);

    //Backup한 일반설정정보의 CheckSum이 틀릴때 Default설정으로 초기화한다.
    if(xBackupHD.x.bCheckSum != GetSettingsCheckSum(xBackupHD.a, sizeof(xBackupHD.a)))
        ResetHeadInfoParams(&xBackupHD);

    memcpy(&g_xHD, &xBackupHD, sizeof(HEAD_INFO));

    // UpdateHeadInfos();
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
    g_xHD2.x.bCheckSum = GetSettingsCheckSum(g_xHD2.a, sizeof(g_xHD2.a));
    M24C64_SetHD2((unsigned char*)&g_xHD2);
}

/**
 * @brief ResetHeadInfos: Head정보를 Default설정으로 초기화한다.
 */
void ResetHeadInfos2()
{
    memset(&g_xHD2, 0, sizeof(HEAD_INFO2));
    UpdateHeadInfos2();
}

void ReadROKLogs()
{
    M24C64_GetBootingLogs((unsigned char*)&g_xROKLog);
    if (g_xROKLog.bCheckSum != GetSettingsCheckSum((unsigned char*)&g_xROKLog, sizeof(g_xROKLog)))
        ResetROKLogs();
}

void ResetROKLogs()
{
    memset(&g_xROKLog, 0, sizeof(g_xROKLog));
    g_xROKLog.bMountPoint = DB_PART1;
    g_xROKLog.bCheckSum = GetSettingsCheckSum((unsigned char*)&g_xROKLog, sizeof(g_xROKLog));
    M24C64_SetBootingLogs((unsigned char*)&g_xROKLog);
}

void UpdateROKLogs()
{
    g_xROKLog.bCheckSum = GetSettingsCheckSum((unsigned char*)&g_xROKLog, sizeof(g_xROKLog));
    M24C64_SetBootingLogs((unsigned char*)&g_xROKLog);
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
    g_xES.x.bCheckSum = GetSettingsCheckSum(g_xES.a, sizeof(g_xES.a));
    M24C64_SetES((unsigned char*)&g_xES);
}

/**
 * @brief ResetEncryptSettings: 일반설정을 Default설정으로 초기화한다.
 */
void ResetEncryptSettings()
{
    unsigned char base_code[] = {
        0x06, 0x12, 0x07, 0x03,
        0x0D, 0x0D, 0x17, 0x04,
        0x08, 0x01, 0x00, 0x19,
        0x09, 0x02, 0x02, 0x07
    };
    memset(&g_xES, 0, sizeof(g_xES));
    //use DESMAN original code for old compatibility
    memcpy(g_xES.x.bEncKeyPos, base_code, sizeof(g_xES.x.bEncKeyPos));
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
    g_xSS.iUvcWidth = UVC_WIDTH;
    g_xSS.iUvcHeight = UVC_HEIGHT;
    g_xSS.iCurUvcWidth = UVC_WIDTH;
    g_xSS.iCurUvcHeight = UVC_HEIGHT;
    g_xSS.iCameraRotate = g_xPS.x.bCamFlip;
    g_xSS.iUsbHostMode = g_xCS.x.bUsbHost;
}

void recvReadHeader()
{
#if 0
    memset(&g_xRecvHdr, 0, sizeof(g_xRecvHdr));
    I2C_Read8(g_iM24C64, ADDR_RECV_HDR, (unsigned char*)&g_xRecvHdr, WORD_SIZE);
    if (g_xRecvHdr.bCheckSum != GetSettingsCheckSum((unsigned char*)&g_xRecvHdr, sizeof(g_xRecvHdr)))
    {
        I2C_Read8(g_iM24C64, ADDR_RECV_HDR_BAK, (unsigned char*)&g_xRecvHdr, WORD_SIZE);
        if(g_xRecvHdr.bCheckSum != GetSettingsCheckSum((unsigned char*)&g_xRecvHdr, sizeof(g_xRecvHdr)))
        {
            recvResetHeader();
        }
    }
    else
    {
        RECOVERY_HADER bak;
        I2C_Read8(g_iM24C64, ADDR_RECV_HDR_BAK, (unsigned char*)&bak, WORD_SIZE);
        if (memcmp(&bak, &g_xRecvHdr, sizeof(g_xRecvHdr))) //update backup header.
            I2C_Write8(g_iM24C64, ADDR_RECV_HDR_BAK, (unsigned char*)&g_xRecvHdr, WORD_SIZE);
    }
#endif
}

void recvUpdateHeader()
{
#if 0
    g_xRecvHdr.bCheckSum = GetSettingsCheckSum((unsigned char*)&g_xRecvHdr, sizeof(g_xRecvHdr));
    I2C_Write8(g_iM24C64, ADDR_RECV_HDR, (unsigned char*)&g_xRecvHdr, WORD_SIZE);
    I2C_Write8(g_iM24C64, ADDR_RECV_HDR_BAK, (unsigned char*)&g_xRecvHdr, WORD_SIZE);
#endif
}

void recvResetHeader()
{
#if 0
    memset(&g_xRecvHdr, 0, sizeof(g_xRecvHdr));
    recvUpdateHeader();
#endif
}

int GetZigbeeIdx(int iZigbeeMode)
{
    if(iZigbeeMode == ZIGBEE_DISABLE)
        return 0;
    else if(iZigbeeMode == ZIGBEE_OUR)
        return 1;
    else if(iZigbeeMode == WIFI_YINGHUA_JIWEI)
        return 2;
    else if(iZigbeeMode == WIFI_YINGHUA_SIGE)
        return 3;
    else if(iZigbeeMode == WIFI_GESANG_SIGE)
        return 4;

    return 0;
}

int GetZigbeeMode(int iZigbeeIdx)
{
    if(iZigbeeIdx == 0)
        return ZIGBEE_DISABLE;
    else if(iZigbeeIdx == 1)
        return ZIGBEE_OUR;
    else if(iZigbeeIdx == 2)
        return WIFI_YINGHUA_JIWEI;
    else if(iZigbeeIdx == 3)
        return WIFI_YINGHUA_SIGE;
    else if(iZigbeeIdx == 4)
        return WIFI_GESANG_SIGE;

    return ZIGBEE_DISABLE;
}

void SetMountStatus(int iSuccess)
{
    int iNeedSave = 1; // this must always be saved
    if (iSuccess != 0)
    {
        if (g_xROKLog.bMountStatus != 0xAA)
        {
            g_xROKLog.bMountStatus = 0xAA;
            iNeedSave = 1;
        }
        if (g_xROKLog.bMountRetry != 0)
        {
            g_xROKLog.bMountRetry = 0;
            iNeedSave = 1;
        }
    }
    else
    {
        if (g_xROKLog.bMountStatus != 0x55)
        {
            g_xROKLog.bMountStatus = 0x55;
            iNeedSave = 1;
        }
    }
    if (iNeedSave)
        UpdateROKLogs();
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