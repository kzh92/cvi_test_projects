#ifndef FACEENGINE_H
#define FACEENGINE_H

#include <stdint.h>
#include "DBManager.h"
#if (N_MAX_HAND_NUM)
#include "hand/HandRetrival_.h"
#endif

#define FE_TASK_MAX_CMD_ARGS    2
typedef enum _tagFE_TASK_CMD_TYPE{
    FE_TASK_CMD_NONE,
    FE_TASK_CMD_CUSTOM_FUNC,
    FE_TASK_CMD_INIT_FEGN,
    FE_TASK_CMD_DO_FACE_TASK_START,
    FE_TASK_CMD_DO_FACE_TASK_STOP,
} FE_TASK_CMD_TYPE;

class FaceRecogTask;

void feTaskCustomFunction_arg1(void* p_fn, void* arg0);
void fe_InitEngine(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum);
void feFaceStart(int iCmd);
void feFaceStop();
FaceRecogTask* getFaceInstance();

class FaceEngine
{
public:
    static int Create(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum);
    static int ResetAll(int flag);
    static int RemoveUser(int iUserID, int iType, int iSkipUpdateCount = 0);
    static int RemoveUserRange(int iUserBeginID, int iUserEndID);
    static int Release();

	//engine process
    static void	VerifyInit(int fAdminiMode, int iUserID = -1);
    static void	UnregisterFace(int nUpdateID = -1, int isMultiDirectionMode = 1);

    static int	ExtractFace(unsigned char* pbRgbData, unsigned char* pbLedOnData, float* prResultArray);
    static int	VerifyFace(float* prResultArray);
    static int  RegisterFace(float* prResultArray, int iFaceDir);
    static void RegisterImage(float* prResultArray, unsigned char* pbClrBuffer, int width, int height);
    static void RegisterFeat(float* prResultArray, unsigned char* pbFeat, int length, int isIRImage = 0);

    static void InitCalibOffset();
    static int  AutoCameraAdjust(unsigned char* pbClrData, unsigned char* pbRedOnData, float* pnResultArray);

    static int	GetRegisteredFaceImage(unsigned char* pbJpgData, int* pnJpgLen);
    static int  GetLastFaceData(unsigned char* pbFaceData);
    static int  GetLastFaceImage(unsigned char* pbJpgData, int* pnJpgLen);
    static int  SetLastFaceScene(unsigned char* pbRgbData);
    static void GetRegisteredFeatInfo(PSFeatInfo pxFeatInfo);
    static int GetIRFeatInfo(void**);
    static int SavePerson(PSMetaInfo pxUserInfo, PSFeatInfo pxFeatInfo, int* piBlkNum);
#if (N_MAX_HAND_NUM)
    static int CreateHand(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum);
    static void GetRegisteredFeatInfo_Hand(SHandFeatInfo*  pxFeatInfo);
    static int SaveHand(PSMetaInfo pxUserInfo, SHandFeatInfo* pxFeatInfo, int* piBlkNum);
#endif // N_MAX_HAND_NUM
    static int  DecodeRegisterFileData(unsigned char** pBuffer, int file_len, int * puser_count, uint16_t** puser_ids);
    static int  UpdateDbBin(unsigned char* pBuffer, int iLen, int iUpdateFlag, int uid);
    static int  GetPersonDbBin(unsigned char* pBuffer, int iLen, int iUpdateFlag, int iID, int iOffset);
    static int SetLivenessLevel();
private:
    static void* m_irFeatBuffer;
};

#endif // FACEENGINE_H
