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

enum E_Sound_Type
{
    Sound_None,
    Sound_Left,
    Sound_Top
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


enum
{
    KEY_ANY = 2000,
    KEY_BASE = 1000,
    KEY_MULTI = 1000,
    KEY_1 = 1001,
    KEY_2 = 1002,
    KEY_3 = 1003,
    KEY_4 = 1004,
    KEY_5 = 1005,
    KEY_6 = 1006,
    KEY_7 = 1007,
    KEY_8 = 1008,
    KEY_9 = 1009,
    KEY_0 = 1010,
    KEY_BACK = 1011,
    KEY_CONFIRM = 1012,
    KEY_FUNC = 1013,
    KEY_FUNC_CLICK = 1030,
    KEY_FUNC_DBL_CLICK = 1031,
    KEY_FUNC_LONG_CLICK = 1032
};

enum
{
    E_TEST_FAILED,
    E_DEVICE_TEST_START,
    E_PRESENTATION_TEST_START,
    E_PRESENTATION_TEST_STOP,
};

#define N_DEMO_VERIFY_MODE_ON       0x01
#define N_DEMO_VERIFY_MODE_OFF      0x00
#define N_DEMO_FACTORY_MODE         0x10

#pragma pack(push, 1)

///영구(공장)설정
/// !!!CAUTION: You MUST modify ResetPermanenceSettings() function
/// when you add new member to this structure.
typedef struct _tagPERMANENCE_SETTINGS
{
    // [14]
    unsigned char   bIsFirst;         //bDebugEn;

    unsigned char   bCamFlip:1;         //bCamFlip:1
    unsigned char   bReserved02:1;          // bSendLastMsg:1;
    unsigned char   bEnableLogFile:1;
    unsigned char   bHijackEnable:1;
    unsigned char   bReserved1:4;       //bTwinsMode before, 0: twins, 1: non-twins

    unsigned char   bUvcImageQuality; // s_msg_uvc_set_compressparam_data
    unsigned char   bUvcBitrateMax:4; // s_msg_uvc_set_compressparam_data
    unsigned char   bUvcBitrateDefault:4; // s_msg_uvc_set_compressparam_data

    unsigned char   bUvcRepeatFrame; // s_msg_uvc_set_compressparam_data

    int             iChecksumDNN;
    int             iCheckSumH;
    unsigned char   bReserved4[2];
    unsigned char   bCheckSum;
} PERMANENCE_SETTINGS__;

typedef union
{
    PERMANENCE_SETTINGS__       x;
    unsigned char               a[16];
} PERMANENCE_SETTINGS;

///
typedef struct _tagHEAD_INFO
{
    // [0-2]
    unsigned char   bReserved0;             //bModifyUser before

    // [3]
    unsigned char   bReserved1_0:1;         //bMountFlag, V3S에서 파티션복귀에 리용하는 옵션, 다음번기동에 리용
    unsigned char   bReserved1:7;           //예약

    //[13-14]
    unsigned char   bReserved3;             //bIsResetFlag before, 24c64 reset flag, 15 is true
    unsigned char   bChipID[8];             //chip id
    unsigned char   bReserved2[4];          //예약
    unsigned char   bCheckSum;              //[15]: 0xFF - (([0] + ...  + [14]) & 0xFF)
} HEAD_INFO__;

typedef union
{
    HEAD_INFO__     x;
    unsigned char   a[16];
} HEAD_INFO;

typedef struct _tagHEAD_INFO2
{
    unsigned int   iBootingCount;             //
    unsigned int   iBootingCount1;             //
    unsigned int   iIRFailedoCount;             //
    unsigned char   bModifyUser;
    unsigned char   bMountFlag:1;
    unsigned char   bReserved1:7;
    unsigned char   bIsResetFlag;
    unsigned char   bCheckSum;              //[15]: 0xFF - (([0] + ...  + [14]) & 0xFF)
} HEAD_INFO2__;

typedef union
{
    HEAD_INFO2__     x;
    unsigned char   a[16];
} HEAD_INFO2;

typedef struct _tagRECOVERY_HEADER
{
    unsigned short iRfsRecoverCount; //rootfs recovery count
    unsigned short iUserDbRecoverCount; //dbfs recovery count
    unsigned short iLastRcvLogIdx; //index number in Recovery Log Data
    unsigned int iRfsChecksum0;
    unsigned int iRfsChecksum2;
    unsigned char bReserved;
    unsigned char bCheckSum;
} RECOVERY_HADER;

///일반체계설정
typedef struct _tagCOMMON_SETTINGS
{
    //[0]
#if (N_MAX_PERSON_NUM < 0xFF)
    unsigned char   bUserCount;             //등록된 사용자수
    unsigned char   bHandCount;             //baud rate before
#else // N_MAX_PERSON_NUM
    unsigned short  bUserCount;             //등록된 사용자수
#endif // N_MAX_PERSON_NUM
    unsigned char   bPresentation:1;        //연시방식(0: Off, 1: On)
    unsigned char   bReserved01:1; // bDupCheck:1;
    unsigned char   bUpgradeFlag:1;
    unsigned char   bUsbHost:1;// 0: USB device mode, 1: USB host mode
    unsigned char   bUVCDir:2;// 0: not rotate 90, 1: rotate 90
    unsigned char   bCheckFirmware:1;
    unsigned char   bOtaMode:1;// 0: OTA UART mode, 1: OTA USB mode

    unsigned char   bReserved04:2; // bLivenessMode:2;
    unsigned char   bReserved05:2; // bTwinsMode:2; //0: twins, 1: normal
    unsigned char   bUpgradeBaudrate:3;
    unsigned char   bReserved2:1;
    unsigned char   bSecureFlag;

    unsigned char   bReserved06:4; //bProtoEncMode:4; //0: default,
    unsigned char   bReserved3:4;
    
    unsigned char   bReserved4[9];

    //[15]
    unsigned char   bCheckSum;            //[15]: 0xFF - (([0] + ...  + [14]) & 0xFF)
} COMMON_SETTINGS__;

typedef union
{
    COMMON_SETTINGS__   x;
    unsigned char       a[16];
} COMMON_SETTINGS;

typedef struct _tagROK_LOG
{
    unsigned short  i7Error_No_Secure;      //보안통신을 못하고 7초만에 꺼준 오유
    unsigned short  i7Error_Secure;         //보안통신을 진행한다음에 7초만에 꺼준 오유
    unsigned short  i60Error;               //60초동안 건반사건이 들어오지 않을때 꺼준 오유
    unsigned char   iDBformatCount;
    unsigned char   aReserved[3];

    unsigned char   bKernelFlag;
    unsigned char   bKernelCounter;

    unsigned char   bMountStatus;            //0x55: start, 0xAA: ok
    unsigned char   bMountPoint:3;             //DB_PART1, DB_PART2, ...
    unsigned char   bMountRetry:5;

    unsigned char   bMountCounter;
    unsigned char   bCheckSum;
} ROK_LOG;

#ifndef _APP_UPGRADE

typedef struct _tagFACTORY_SETTINGS
{
    unsigned char bBattTest;
    unsigned char bShowBatt;
    unsigned char bTestState;
    unsigned char bAutoTest;

    int           iCamOffX;
    int           iCamOffY;

    int           iMotorControl;
} FACTORY_SETTINGS;

typedef struct _tagTEST_RESULT
{
    int             fStandbyOk;
    unsigned int    dwStandByCur1;
    unsigned int    dwStandByCur2;

    int             fMiniVolOk;

    int             fMainMenuOk;
    unsigned int    dwMainMenuCur;

    int             fClrCamOk;
    unsigned int    dwClrCamCur;

    int             fIRCamOk;
    unsigned int    dwIRCamCur;
} TEST_RESULT;

typedef struct _tagENCRYPT_SETTINGS
{
    //[0]
    unsigned char   bEncKeyPos[16];
    unsigned char   bReserved2[15];

    //[31]
    unsigned char   bCheckSum;            //[15]: 0xFF - (([0] + ...  + [14]) & 0xFF)
} ENCRPYT_SETTINGS__;

typedef union
{
    ENCRPYT_SETTINGS__   x;
    unsigned char       a[32];
} ENCRYPT_SETTINGS;

typedef struct _tagSYSTEM_STATE
{
    //System
    int             iAppType;
    int             iFirstFlag;
    int             iVerifyMode;
    int             iWakeupByFunc;  //안내건을 눌러서 기동한 기발
    int             idbPartNoRestore;
    int             idbRestoreStop;

    DATETIME_32     xSetDate;

    int             iRunningCamSurface;
    int             iRunningDvpCam;
    int             iShowIrCamera;
    float           rIRCamTime;
    float           rCLRCamTime;

    //Face Recog Task
    SRect           xFaceRect;
    float           rFaceEngineTime;

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

    unsigned char*              pbOtaData;
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
    int                         iProtoMode; // 0: no encryption, 1:sanjiang mode

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

    s_msg_verify_data   msg_verify_data;
    s_msg_enroll_itg    msg_enroll_itg_data;
    s_msg_deluser_data  msg_deluser_data;
    s_msg_getuserinfo_data  msg_getuserinfo_data;
    s_msg_snap_image_data   msg_snap_image_data;
    s_msg_get_saved_image_data  msg_get_saved_image_data;
    s_msg_upload_image_data     msg_upload_image_data;
    s_msg_startota_data         msg_startota_data;
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

void RestoreBackupSettings();
void UpdateBackupSettings();
void ResetCS(COMMON_SETTINGS* pxCS);

int ReadEncryptSettings();
void UpdateEncryptSettings();
void ResetEncryptSettings();

void ReadHeadInfos();
void UpdateHeadInfos();
void ResetHeadInfos();
void ResetHeadInfoParams(HEAD_INFO*);
void RestoreBackupHeadInfos();
void UpdateBackupHeadInfos();

void ReadROKLogs();
void ResetROKLogs();
void UpdateROKLogs();

void recvReadHeader();
void recvUpdateHeader();
void recvResetHeader();

void ResetSystemState(int iAppType);
int SetLogo(unsigned char* pbData, int iSize);

int     GetZigbeeIdx(int iZigbeeMode);
int     GetZigbeeMode(int iZigbeeIdx);

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

extern PERMANENCE_SETTINGS g_xPS;
extern COMMON_SETTINGS g_xCS;
#ifndef _APP_UPGRADE
extern ROK_LOG g_xROKLog;
extern HEAD_INFO g_xHD;
extern HEAD_INFO2 g_xHD2;
extern SYSTEM_STATE g_xSS;
extern ENCRYPT_SETTINGS g_xES;
#endif // _APP_UPGRADE

#endif // SETTINGS_H
