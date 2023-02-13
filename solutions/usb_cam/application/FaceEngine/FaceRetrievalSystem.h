
#ifndef __FACE_RETRIEVAL_SYSTEM_H__
#define __FACE_RETRIEVAL_SYSTEM_H__

#include "EngineStruct.h"

extern SEngineResult g_xEngineResult;
extern SEngineParam g_xEngineParam;

#define SECURITY_LEVEL_TWIN     0
#define SECURITY_LEVEL_COMMON   1

#define ENROLL_MULTI_DIRECTION_MODE         0
#define ENROLL_ONLY_FRONT_DIRECTION_MODE    1

#define IR_SCREEN_GETIMAGE_MODE     0
#define IR_SCREEN_CAMERAVIEW_MODE   1


//#define H_DICT_SIZE1        (0xDD2D4)

#ifdef __cplusplus
extern	"C"
{
#endif

LIBFOO_DLL_EXPORTED void    fr_InitEngine(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum);
LIBFOO_DLL_EXPORTED void    fr_FreeEngine();
LIBFOO_DLL_EXPORTED void    fr_InitLive();
LIBFOO_DLL_EXPORTED void    fr_EndThread();
LIBFOO_DLL_EXPORTED void    fr_SetDupCheck(int iDupCheck);
LIBFOO_DLL_EXPORTED void    fr_SetCameraFlip(int iCameraFlip);
LIBFOO_DLL_EXPORTED void    fr_EnableLogFile(int iEnable);

LIBFOO_DLL_EXPORTED void    fr_SetEngineState(int nState, int iParam1, int iParam2, int iParam3 = ENROLL_MULTI_DIRECTION_MODE, int iParam4 = 0);

LIBFOO_DLL_EXPORTED int     fr_PreExtractFace(unsigned char *pbClrImage, unsigned char *pbLedOnImage);
LIBFOO_DLL_EXPORTED int		fr_PreExtractFace2(unsigned char *pbBayerFromCamera2);
LIBFOO_DLL_EXPORTED int		fr_ExtractFace();
LIBFOO_DLL_EXPORTED int     fr_calc_Off(unsigned char *pbLedOffImage);

LIBFOO_DLL_EXPORTED int     fr_VerifyFace();
LIBFOO_DLL_EXPORTED int     fr_RetrievalH();

LIBFOO_DLL_EXPORTED int     fr_RegisterFace(int iFaceDir);
LIBFOO_DLL_EXPORTED int     fr_GetRegisteredFeatInfo(PSFeatInfo pxFeatInfo);

LIBFOO_DLL_EXPORTED int*    fr_GetProcessArea();
LIBFOO_DLL_EXPORTED int*    fr_GetAverageLedOnImage();
LIBFOO_DLL_EXPORTED float   fr_GetSaturatedRate();
LIBFOO_DLL_EXPORTED int     fr_GetBrightUpThresholdLevel();
LIBFOO_DLL_EXPORTED int*    fr_GetHistInLEDOnImage();
LIBFOO_DLL_EXPORTED SEngineResult*      fr_GetEngineResult();
LIBFOO_DLL_EXPORTED SEngineParam*       fr_GetEngineParam();
LIBFOO_DLL_EXPORTED unsigned char*      fr_GetLastHFeat();
LIBFOO_DLL_EXPORTED unsigned char*      fr_GetDiffIrImage();
LIBFOO_DLL_EXPORTED void    fr_SetDecodedData(unsigned int iKey, unsigned char* pbData);
LIBFOO_DLL_EXPORTED int		fr_GetCurEnvForCameraControl();
LIBFOO_DLL_EXPORTED int*    fr_GetExposure();
LIBFOO_DLL_EXPORTED int*    fr_GetGain();
LIBFOO_DLL_EXPORTED int*    fr_GetExposure2();
LIBFOO_DLL_EXPORTED int*    fr_GetGain2();
LIBFOO_DLL_EXPORTED int*    fr_MainControlCameraIndex();

LIBFOO_DLL_EXPORTED int		fr_GetNeedToCalcNextExposure();
LIBFOO_DLL_EXPORTED int		fr_SetLivenessCheckStrong_On_NoUser(int nMode);
LIBFOO_DLL_EXPORTED int     fr_SetCheckOpenEyeEnable(int nEnable);

LIBFOO_DLL_EXPORTED int     fr_CalcScreenValue(unsigned char *pbLedOnImage, int nMode);

LIBFOO_DLL_EXPORTED void    fr_SetDNNData();
LIBFOO_DLL_EXPORTED int*    fr_GetFaceDetected();
LIBFOO_DLL_EXPORTED void    fr_SetStopEngineFlag(int);
LIBFOO_DLL_EXPORTED void    fr_SetMaxThresholdValue(float);
LIBFOO_DLL_EXPORTED int     fr_getSuitableThreshold();
LIBFOO_DLL_EXPORTED int     fr_getHDicCheckSumMatched();

LIBFOO_DLL_EXPORTED void    fr_SetSecurityMode(int nSecurityMode);
LIBFOO_DLL_EXPORTED void    fr_InitIRCamera_ExpGain();

LIBFOO_DLL_EXPORTED int     fr_GetSecondImageNeedReCheck();
LIBFOO_DLL_EXPORTED int     fr_CheckFaceInSecondImage(unsigned char *pbBayerLeft, unsigned char *pbBayerRight);

#ifdef __RTK_OS__
LIBFOO_DLL_EXPORTED int 	fr_ReadFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
LIBFOO_DLL_EXPORTED int 	fr_WriteFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
#endif // __RTK_OS__


#ifdef __cplusplus
}
#endif

#endif//__FACE_RETRIEVAL_SYSTEM_H__

