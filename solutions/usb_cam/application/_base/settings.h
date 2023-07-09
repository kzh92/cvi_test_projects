#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdio.h>
#include <stdint.h>
#include "appdef.h"
#ifndef _APP_UPGRADE
#include "EngineStruct.h"
#include "senselockmessage.h"
#endif

#define VAL_ON  1
#define VAL_OFF 0

#define LCD_SHARED_NONE         0
#define LCD_SHARED_STEP1        1
#define LCD_SHARED_STEP2        2
#define LCD_SHARED_EXIT         3

enum
{
    VIEW_READY,
    VIEW_REGISTER_FACE,
    VIEW_IDENTIFY,
    VIEW_GET_IMAGE
};

enum
{
    APP_MAIN,
    APP_SETTINGS
};

enum
{
    ERROR_NONE = 0,
    ERROR_CAMERA_TCMIPI = 1,
    ERROR_CAMERA_DVP = 2,
    ERROR_THREAD = 3,
    ERROR_LOOP = 4,
    ERROR_I2C = 5,
};

#define N_DEMO_VERIFY_MODE_ON       0x01
#define N_DEMO_VERIFY_MODE_OFF      0x00
#define N_DEMO_FACTORY_MODE         0x10

#pragma pack(push, 1)

typedef struct _tagMY_ALL_SETTINGS
{
    // permanance settings
    unsigned char   bIsFirst;         //bDebugEn;

    unsigned char   bCamFlip:1;         //bCamFlip:1
    unsigned char   bReserved00:1;          // bSendLastMsg:1;
    unsigned char   bEnableLogFile:1;
    unsigned char   bHijackEnable:1;
    unsigned char   bReserved01:4;       //bTwinsMode before, 0: twins, 1: non-twins

    unsigned char   bUvcImageQuality; // s_msg_uvc_set_compressparam_data
    unsigned char   bUvcBitrateMax:4; // s_msg_uvc_set_compressparam_data
    unsigned char   bUvcBitrateDefault:4; // s_msg_uvc_set_compressparam_data

    unsigned char   bUvcRepeatFrame; // s_msg_uvc_set_compressparam_data

    int             iChecksumDNN;
    int             iCheckSumH;
    unsigned char   bIsActivated;
    unsigned char   bActMark;

    //head infos
    unsigned char   bChipID[8];             //chip id
    //head infos 2
    unsigned int   iBootingCount;             //
    unsigned int   iBootingCount1;             //
    unsigned int   iIRFailedoCount;             //
    unsigned char   bModifyUser;
    unsigned char   bMountFlag:1;
    unsigned char   bReserved02:7;
    unsigned char   bIsResetFlag;

    //common settings
    //[0]
    unsigned short  bUserCount;             //등록된 사용자수
    unsigned char   bHandCount;             //등록된 사용자수
    unsigned char   bPresentation:1;        //연시방식(0: Off, 1: On)
    unsigned char   bReserved03:1; // bDupCheck:1;
    unsigned char   bUpgradeFlag:1;
    unsigned char   bUsbHost:1;// 0: USB device mode, 1: USB host mode
    unsigned char   bUVCDir:2;// 0: not rotate 90, 1: rotate 90
    unsigned char   bCheckFirmware:1;
    unsigned char   bOtaMode:1;// 0: OTA UART mode, 1: OTA USB mode

    unsigned char   bReserved04:2; // bLivenessMode:2;
    unsigned char   bReserved05:2; // bTwinsMode:2; //0: twins, 1: normal
    unsigned char   bUpgradeBaudrate:3;
    unsigned char   bReserved06:1;
    unsigned char   bSecureFlag;

    //rok log
    // unsigned short  i7Error_No_Secure;      //보안통신을 못하고 7초만에 꺼준 오유
    // unsigned short  i7Error_Secure;         //보안통신을 진행한다음에 7초만에 꺼준 오유
    // unsigned short  i60Error;               //60초동안 건반사건이 들어오지 않을때 꺼준 오유
    unsigned char   iDBformatCount;

    unsigned char   bKernelFlag;
    unsigned char   bKernelCounter;

    unsigned char   bMountStatus;            //0x55: start, 0xAA: ok
    unsigned char   bMountPoint:3;             //DB_PART1, DB_PART2, ...
    unsigned char   bMountRetry:5;
    unsigned char   bMountCounter;

    //encrypt settings
    unsigned char   bEncKeyPos[16];
    unsigned char   bReserved07[15];

    unsigned char   bCheckSum;
} MY_ALL_SETTINGS__;

typedef union
{
    MY_ALL_SETTINGS__       x;
    unsigned char               a[0];
} MY_ALL_SETTINGS;

#ifndef _APP_UPGRADE

typedef struct _tagSYSTEM_STATE
{
    //System
    int             iAppType;
    int             iFirstFlag;
    int             iVerifyMode;
    int             iWakeupByFunc;  //안내건을 눌러서 기동한 기발
    int             idbPartNoRestore;
    int             idbRestoreStop;

    int             iRunningCamSurface;
    int             iRunningDvpCam;
    int             iShowIrCamera;
    float           rIRCamTime;
    float           rCLRCamTime;

    //Face Recog Task
    SRect           xFaceRect;
    float           rFaceEngineTime;
    int             iEFIFlag;
    int             iEFIImageType;
    int             iSnapImageFace;

    float           rCardEngineTime;
    float           rFPEngineTime;
    float           rMainLoopTime;

    int             bPresentation;
    int             bSound;
    int             iCurLogo;
    int             iShowInnerVersion;

    float           rClrProcessTime;        //얼굴인식쓰레드가 죽었을때부터 동작한 카메라쓰레드시간
    float           rIRProcessTime;         //얼굴인식쓰레드가 죽었을때부터 동작한 카메라쓰레드시간

    int             iWaitingCacnel;
    int             iFrontClose;
    int             iWifiConfigState;

    int             iNoSoundPlayFlag;
    int             iManagerSound;

    int             iVerifyFailType;

    int             iRegisterID;
    int             iRegsterAuth;
    int             iRegisterDir;
    int             iRegisterHand;

    int             iVerifyRole;

    int             iUVCpause;//1:ReadyStep 2:PauseStep
    int             iTimeoutTimer;

    int             iVerifyFailedCount;
    int             iFaceImage;

    int             iNoActivated;
    int             iActivated;

    int             iVDBCmd;
    int             iVDBStart;
    int             iGetImage;

#if (USE_WIFI_MODULE)
    int             iJiweiStart;
    int             iRecogQRMode;
    int             iRecogQRTimeout;
    char            strQRContents[256];

    int             iSwitchToIR;
#endif // USE_WIFI_MODULE

    int             iStartOta;
    int             iUsbHostMode;
    int             iAutoUserAdd;
    int             iCameraRotate;
    int                         iUpgradeLen;
    int                         iUpgradeImgLen;

    unsigned char*              pbOtaData;
    int                         iDBUpdateFlag;
    int                         iDBgetFlag;
    int                         iDBgetIndex;
    int*                        piOtaPckIdx;
    int                         iOtaError;

    int                         iResetFlag;
    float                       rResetFlagTime;
    int                         iMState;
    int                         iDemoMode;

    int                         iFuncTestFlag;
    s_msg*                      pLastMsg;

    int                         iRestoreRootfs;
    int                         iCamError;
    float                       rLastSenseCmdTime;
    float                       rAppStartTime;
    int                         iEnrollMutiDirMode;
    int                         iEnrollFaceDupCheck;
    int                         iEnrollDupCheckMode;
    int                         iRunningCmd;
    int                         iUvcWidth;
    int                         iUvcHeight;
    int                         iCurUvcWidth;
    int                         iCurUvcHeight;
    int                         iUvcDirect;
    int                         iCurClrGain;
    int                         iProtoMode; // 0: no encryption, 1:sanjiang mode
    int                         iCapWidth;
    int                         iCapHeight;

    int                         iSendLastMsgMode;
    unsigned char               iMidPowerdown;
#if (USE_FUSHI_PROTO)
    unsigned char               iProtocolHeader;
    int16_t                     iFaceState;
    float                       rVerifyStartTime;
    float                       rLastVerifyAckSendTime;
    unsigned char               bVerifying;
#endif // USE_FUSHI_PROTO
    int                         bCheckFirmware;
    int                         bUVCRunning;
    unsigned char               *bSnapImageData;
    unsigned int                iSnapImageLen[SI_MAX_IMAGE_COUNT];
    unsigned char               iOcclusionFlag;

    s_msg_verify_data   msg_verify_data;
    s_msg_enroll_itg    msg_enroll_itg_data;
    s_msg_deluser_data  msg_deluser_data;
    s_msg_getuserinfo_data  msg_getuserinfo_data;
    s_msg_snap_image_data   msg_snap_image_data;
    s_msg_get_saved_image_data  msg_get_saved_image_data;
    s_msg_upload_image_data     msg_upload_image_data;
    s_msg_otaheader_data        msg_otaheader_data;
    s_msg_init_encryption_data  msg_init_encryption_data;
    s_msg_demomode_data  msg_demomode_data;
    s_msg_demomode_data  msg_send_msg_data;
    s_msg_uvc_dir_data  msg_uvc_dir_data;
    s_note_data_face    note_data_face;
    s_note_data_eye     note_data_eye;

} SYSTEM_STATE;

#endif // !_APP_UPGRADE

#pragma pack(pop)

#ifdef __cplusplus
extern  "C"
{
#endif // __cplusplus

void ReadHeadInfos2();
void UpdateHeadInfos2();
void ResetHeadInfos2();

unsigned char GetSettingsCheckSum(unsigned char* pbData, int iSize);
void ReadPermanenceSettings();
void UpdatePermanenceSettings();
void ResetPermanenceSettings();

int ReadCommonSettings();
void UpdateCommonSettings();
void ResetCommonSettings();

int ReadMyAllSettings();
void UpdateMyAllSettings();
void ResetMyAllSettings();

void RestoreBackupSettings();
void UpdateBackupSettings();
void ResetCS(MY_ALL_SETTINGS* pxCS);

int ReadEncryptSettings();
void UpdateEncryptSettings();
void ResetEncryptSettings();

void ReadHeadInfos();
void UpdateHeadInfos();
void ResetHeadInfos();
void ResetHeadInfoParams(MY_ALL_SETTINGS*);
void RestoreBackupHeadInfos();
void UpdateBackupHeadInfos();

void ReadROKLogs();
void ResetROKLogs();
void UpdateROKLogs();

void ResetSystemState(int iAppType);

void    SetModifyUser(int iModify);
void    SetMountStatus(int iSuccess);
int     IsModifyUser();
int     setUvcWindow(int width, int height);
void    lockUvcWindow();
void    unlockUvcWindow();
void    ClearSenseResetFlag();
void    MarkSenseResetFlag();

#ifdef __cplusplus
}
#endif // __cplusplus

extern MY_ALL_SETTINGS g_xAS;
#define g_xCS           g_xAS
#ifndef _APP_UPGRADE
#define g_xPS           g_xAS
#define g_xROKLog       g_xAS
#define g_xHD           g_xAS
#define g_xHD2          g_xAS
#define g_xES           g_xAS
#endif // _APP_UPGRADE
extern SYSTEM_STATE g_xSS;

#endif // SETTINGS_H
