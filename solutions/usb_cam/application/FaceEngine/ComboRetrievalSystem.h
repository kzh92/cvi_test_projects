
#ifndef __COMBO_RETRIEVAL_SYSTEM_H__
#define __COMBO_RETRIEVAL_SYSTEM_H__

#include "EngineStruct.h"

#ifdef __cplusplus
extern	"C"
{
#endif

LIBFOO_DLL_EXPORTED int     fr_PreExtractCombo(unsigned char *pbLedOnImage, int nProcessMode);
LIBFOO_DLL_EXPORTED int		fr_PreExtractCombo2(unsigned char *pbBayerFromCamera2);
LIBFOO_DLL_EXPORTED int     fr_ExtractCombo();

LIBFOO_DLL_EXPORTED SEngineResult*      fr_GetEngineResultCombo(int* pnProcessMode);

LIBFOO_DLL_EXPORTED int     fr_RegisterCombo(int iFaceDir, int* pnProcessMode);
LIBFOO_DLL_EXPORTED int     fr_VerifyCombo(int* pnProcessMode);

/*
LIBFOO_DLL_EXPORTED void    fr_InitEngine(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum);
LIBFOO_DLL_EXPORTED void    fr_FreeEngine();
LIBFOO_DLL_EXPORTED void    fr_InitLive();
LIBFOO_DLL_EXPORTED void    fr_EndThread();
LIBFOO_DLL_EXPORTED void    fr_SetDupCheck(int iDupCheck);
LIBFOO_DLL_EXPORTED void    fr_SetCameraFlip(int iCameraFlip);
LIBFOO_DLL_EXPORTED void    fr_EnableLogFile(int iEnable);

LIBFOO_DLL_EXPORTED void    fr_SetEngineState(int nState, int iParam1, int iParam2, int iParam3 = ENROLL_MULTI_DIRECTION_MODE, int iParam4 = 0, int iParam5 = EEK_Face);

LIBFOO_DLL_EXPORTED int     fr_PreExtractFace(unsigned char *pbClrImage, unsigned char *pbLedOnImage);
LIBFOO_DLL_EXPORTED int		fr_PreExtractFace2(unsigned char *pbBayerFromCamera2);
LIBFOO_DLL_EXPORTED int		fr_PreExtractFaceClr(unsigned char *pbYUV222,int nWidth, int nHeight);
LIBFOO_DLL_EXPORTED int		fr_convertCam2(unsigned char *pbBayerFromCamera2);


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
LIBFOO_DLL_EXPORTED int*    fr_GetFineGain();
LIBFOO_DLL_EXPORTED int*    fr_GetExposure2();
LIBFOO_DLL_EXPORTED int*    fr_GetGain2();
LIBFOO_DLL_EXPORTED int*    fr_GetFineGain2();
LIBFOO_DLL_EXPORTED int*    fr_MainControlCameraIndex();
LIBFOO_DLL_EXPORTED int*    fr_GetMainProcessCameraIndex();
LIBFOO_DLL_EXPORTED int     fr_GetNeedSmallFaceCheck();

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
LIBFOO_DLL_EXPORTED void    fr_BackupIRCamera_ExpGain();

LIBFOO_DLL_EXPORTED int     fr_GetSecondImageNeedReCheck();
LIBFOO_DLL_EXPORTED int     fr_GetSecondImageIsRight();
LIBFOO_DLL_EXPORTED int     fr_CheckFaceInSecondImage(unsigned char *pbBayerNeedReCheck);
LIBFOO_DLL_EXPORTED unsigned char* fr_GetInputImageBuffer1();
LIBFOO_DLL_EXPORTED unsigned char* fr_GetInputImageBuffer2();
LIBFOO_DLL_EXPORTED unsigned char* fr_GetOffImageBuffer();
LIBFOO_DLL_EXPORTED unsigned char* fr_GetOffImageBuffer2();

LIBFOO_DLL_EXPORTED int*    fr_GetFaceDetectedMode();
*/

#ifdef __cplusplus
}
#endif

#endif//__FACE_RETRIEVAL_SYSTEM_H__

