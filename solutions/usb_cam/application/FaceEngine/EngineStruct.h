#ifndef ENGINE_STRUCT_H
#define ENGINE_STRUCT_H

#include "EngineDef.h"
#include "Htypes.h"
#include "engine_inner_param.h"

#ifdef __RTK_OS__
#pragma pack(push, 1)
#endif


typedef struct _tagRect
{
    int x;
    int y;
    int width;
    int height;
} SRect;

typedef struct _tagDATETIME_32
{
    unsigned int iSec:6;        //[0, 59]
    unsigned int iMin:6;        //[0, 59]
    unsigned int iHour:5;       //[0,23]
    unsigned int iDay:5;        //[1, 31]
    unsigned int iMon:4;        //[0,11]
    unsigned int iYear:6;       //2000 +
} DATETIME_32__;

typedef union _taguDATETIME_32
{
    unsigned int    i;
    DATETIME_32__   x;
} DATETIME_32;

typedef struct _tagMetaInfo
{
    int             iID;                                        //4byte
    int             fPrivilege;                                 //4byte
    char            szName[N_MAX_NAME_LEN];                     //32byte

    DATETIME_32     xStartTime;                                 //4byte
    DATETIME_32     xEndTime;                                   //4byte
#if 0
    int             iImageLen;                                  //4byte
    unsigned char   abFaceImage[N_MAX_JPG_FACE_IMAGE_SIZE];     //2000byte
#endif
} SMetaInfo, *PSMetaInfo;                                       //2120

typedef struct _tagFeatInfo
{
//    int             nFeatCount;                             //4
//    unsigned char   abFeatArray[TOTAL_ENROLL_MAX_FEATURE_COUNT*UNIT_ENROLL_FEATURE_SIZE];
//    unsigned char	ab_H_Info[TOTAL_ENROLL_MAX_FEATURE_COUNT];

//    int             nDNNFeatCount;						//4
//    float           arDNNFeatArray[TOTAL_ENROLL_MAX_DNNFEATURE_COUNT*KDNN_FEAT_SIZE];       //10240
//    unsigned char	ab_DNN_Info[TOTAL_ENROLL_MAX_DNNFEATURE_COUNT];										//added by KSB 20180613
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    int             nDNNFeatCount;						//4
    int             nFeatCount;                             //4
    unsigned char   arFeatBuffer[TOTAL_ENROLL_MAX_FEATURE_COUNT * UNIT_ENROLL_FEATURE_SIZE + KDNN_FEAT_SIZE * sizeof(unsigned short) * 4];       //10240
    unsigned char	ab_Info[TOTAL_ENROLL_MAX_FEATURE_COUNT];										//added by KSB 20180613
#else//ENGINE_SECURITY_ONLY_COMMON
    int             nDNNFeatCount;						//4
    unsigned char   arFeatBuffer[TOTAL_ENROLL_MAX_DNNFEATURE_COUNT*KDNN_FEAT_SIZE * sizeof(unsigned short)];       //10240
    unsigned char	ab_Info[TOTAL_ENROLL_MAX_DNNFEATURE_COUNT];										//added by KSB 20180613
#endif

} SFeatInfo, *PSFeatInfo;                                   //69164

typedef struct _tagUserPosInfo
{
    int             nBlk;
    int             nPos;
} UserPosInfo;

typedef struct _tagSLogInfo
{
    int             iID;                                    //1
    int             fVerifyMode;        //4 인증방식: 0 -> verify user, 1 -> verify manager
    int             fSucc;              //4 성공기발: 0 -> failed, 1 -> success, 2 -> open lock, 3 -> request open

    DATETIME_32     xTime;              //4 기록날자
    int             iImageValid;                            //4
#if 0
    int             iImageLen;          //4 얼굴기록Len
    unsigned char   abFaceImg[N_MAX_JPG_FACE_IMAGE_SIZE];   //2000
#endif
} SLogInfo;     //2064

typedef struct _tagSEngineResult
{
    int             fValid;
    SRect           xFaceRect;

	int				nFineUserIndex;
    int             nMaxScore;
    int             nEyeOpenness;//if 0:close eye, 1:open eye
    int             nOcclusion;//if 1:occlusion exist, 0:no occlusion
    int             nFacePosition;//0:center, 1:left, 2:right, 3:Top, 4:Bottom
    int             nFaceNearFar;//0:not far, 1:near, 2:far
} SEngineResult;

typedef struct _tagSEngineParam
{
    int             fLoadDict;
    int             fEngineState;
    int             fVerifyMode;
    int             iDemoMode;
    int             iEnrollKind;
    float           arThresholdVals[5];

    int             nDetectionWidth;
    int             nDetectionHeight;
    int             nOffsetX;
    int             nOffsetY;
    int             iDupCheck;
    int             iCamFlip;
    int             iEnableLogFile;
} SEngineParam;

#ifdef __RTK_OS__
#pragma pack(pop)
#endif

#endif //_DatabaseDataType_H_

