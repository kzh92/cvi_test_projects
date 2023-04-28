#include "faceengine.h"
#include "FaceRetrievalSystem.h"
#include "DBManager.h"
#include "shared.h"
#include "common_types.h"
#include "facerecogtask.h"
#if (N_MAX_HAND_NUM)
#include "hand/HandRetrival_.h"
#endif // N_MAX_HAND_NUM
#include "ComboRetrievalSystem.h"

#include <string.h>
//#include <malloc.h>
//#include <stdlib.h>
//#include <sys/mman.h>
//#include <fcntl.h>
FaceRecogTask xFaceTask;

int FaceEngine::Create(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum)
{
    int ret = ES_SUCCESS;
    fr_SetMaxThresholdValue(g_xCS.x.bSecureFlag);
#if (USE_TWIN_ENGINE)
    fr_SetSecurityMode(g_xPS.x.bTwinsMode == S_VERIFY_LEVEL_HIGH ? 0 : 1);
#else
    fr_SetSecurityMode(DEFAULT_TWINS_MODE);
#endif
    fr_InitEngine(iDupCheck, iCamFlip, nDnnCheckSum, nHCheckSum);
    fr_SetCheckOpenEyeEnable(DESMAN_ENC_MODE == 0);
    dbm_Init(NULL);

    ret = dbm_LoadPersonDB();
    return ret;
}

int FaceEngine::Release()
{
    dbm_Free();
    return 0;
}

int FaceEngine::ResetAll()
{
    dbm_SetEmptyPersonDB(NULL);
    return ES_SUCCESS;
}

void FaceEngine::VerifyInit(int fAdminMode, int iUserID)
{
    return; //test
    fr_SetCameraFlip(g_xSS.iCameraRotate);
    fr_SetDupCheck(g_xSS.iEnrollFaceDupCheck);
    fr_SetLivenessCheckStrong_On_NoUser(DEFAULT_LIVENESS_MODE);
    fr_SetCheckOpenEyeEnable(g_xPS.x.bHijackEnable);
    fr_SetEngineState(ENS_VERIFY, fAdminMode, iUserID);
}

void FaceEngine::UnregisterFace(int nUpdateID, int isMultiDirectionMode)
{
    return; //test
    fr_SetCameraFlip(g_xSS.iCameraRotate);
    fr_SetDupCheck(g_xSS.iEnrollFaceDupCheck);
    fr_SetLivenessCheckStrong_On_NoUser(DEFAULT_LIVENESS_MODE);
    fr_SetCheckOpenEyeEnable(g_xPS.x.bHijackEnable);
    if (!isMultiDirectionMode)
    {
#if (N_MAX_HAND_NUM)
        if (g_xSS.iRegisterHand)
            fr_SetEngineState(ENS_REGISTER, g_xSS.iRegisterID, g_xSS.iDemoMode, ENROLL_ONLY_FRONT_DIRECTION_MODE, 1, EEK_Hand);
        else
#endif // N_MAX_HAND_NUM
        {
            fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_ONLY_FRONT_DIRECTION_MODE, 1);
        }
    }
    else
    {
#if (USE_FUSHI_PROTO)
        if (g_xSS.iProtocolHeader == FUSHI_HEAD1)
            fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_MULTI_DIRECTION_MODE, 4);
        else
#endif // USE FUSHI_PROTO
            fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_MULTI_DIRECTION_MODE, 5);

    }
}

int FaceEngine::ExtractFace(unsigned char* pbRgbData, unsigned char* pbLedOnData, float* prResultArray)
{
    return ES_FAILED; //test
    if (pbLedOnData == NULL)
    {
        prResultArray[0] = ES_FAILED;
        return ES_FAILED;
    }

    SEngineResult* pxEngineResult =  fr_GetEngineResult();
    SEngineParam* pxEngineParam = fr_GetEngineParam();

    int nRet = fr_PreExtractFace(pbRgbData, pbLedOnData);
    prResultArray[0] = (float)nRet;
    if (pxEngineResult->fValid == 1)
    {
        prResultArray[1] = 1;
        prResultArray[2] = ((float)pxEngineResult->xFaceRect.x + pxEngineParam->nOffsetX) / 2;
        prResultArray[3] = ((float)pxEngineResult->xFaceRect.y + pxEngineParam->nOffsetY) / 2;
        prResultArray[4] = ((float)pxEngineResult->xFaceRect.width) / 2;
        prResultArray[5] = ((float)pxEngineResult->xFaceRect.height) / 2;
    }
    else
        memset(prResultArray + 1, 0, sizeof(int) * 5);

    return nRet;
}

int  FaceEngine::VerifyFace(float* prResultArray)
{
    return ES_PROCESS; //test
    SEngineResult* pxEngineResult = NULL;
    int nProcessMode = 0;
    int nRet = fr_VerifyCombo(&nProcessMode);

    pxEngineResult =  fr_GetEngineResultCombo(&nProcessMode);

    prResultArray[0] = (float)nRet;
    prResultArray[1] = nProcessMode; // 0: face, 1: hand
    prResultArray[4] = pxEngineResult->nMaxScore;
#if (DESMAN_ENC_MODE == 0)
    prResultArray[6] = pxEngineResult->nEyeOpenness;
#else // DESMAN_ENC_MODE
    prResultArray[6] = 1;
#endif // DESMAN_ENC_MODE
    if(nRet == ES_SUCCESS || nRet == ES_UPDATE)
        return pxEngineResult->nFineUserIndex;

    return -1;
}

void FaceEngine::RegisterFace(float* prResultArray, int iFaceDir)
{
    return; //test
    int nProcessMode = 0;
    int nRet = fr_RegisterCombo(iFaceDir, &nProcessMode);
    prResultArray[0] = (float)(nRet);
    prResultArray[1] = nProcessMode;
}

void FaceEngine::InitCalibOffset()
{
}

int FaceEngine::AutoCameraAdjust(unsigned char* pbClrData, unsigned char* pbRedOnData, float* prResultArray)
{
    return 0;
}


int FaceEngine::GetRegisteredFaceImage(unsigned char* /*pbJpgData*/, int* /*pnJpgLen*/)
{
    return 0;
}

int FaceEngine::GetLastFaceData(unsigned char* /*pbFaceData*/)
{
    return 0;
}

int  FaceEngine::GetLastFaceImage(unsigned char* /*pbJpgData*/, int* /*pnJpgLen*/)
{
    return 0;
}

int  FaceEngine::SetLastFaceScene(unsigned char* /*pbRgbData*/)
{
    return 0;
}

void FaceEngine::GetRegisteredFeatInfo(PSFeatInfo pxFeatInfo)
{
    fr_GetRegisteredFeatInfo(pxFeatInfo);
}

int FaceEngine::SavePerson(PSMetaInfo pxUserInfo, PSFeatInfo pxFeatInfo, int* piBlkNum)
{
    if(pxUserInfo == NULL || pxFeatInfo == NULL)
        return ES_FAILED;

    int iFindIdx = dbm_GetIndexOfID(pxUserInfo->iID);
    if(iFindIdx < 0)
    {
        return dbm_AddPerson(pxUserInfo, pxFeatInfo, piBlkNum);
    }
    else
    {
        return dbm_UpdatePerson(iFindIdx, pxUserInfo, pxFeatInfo, piBlkNum);
    }
}

FaceRecogTask* getFaceInstance()
{
    return &xFaceTask;
}

int g_feTaskRunning = 1;
int g_feTaskCmd = FE_TASK_CMD_NONE;
void* g_feTaskCmdArgs[FE_TASK_MAX_CMD_ARGS];
mymutex_ptr g_feTaskMutex = NULL;
typedef void* (fn_customFunc)(void* data);

extern "C" void* feTaskFunc(void* data)
{
#if 0
    g_feTaskMutex = my_mutex_init();
    int cmd;
    void* args[FE_TASK_MAX_CMD_ARGS];
    while(g_feTaskRunning)
    {
        my_mutex_lock(g_feTaskMutex);
        cmd = g_feTaskCmd;
        memcpy(args, g_feTaskCmdArgs, sizeof(void*) * FE_TASK_MAX_CMD_ARGS);
        g_feTaskCmd = FE_TASK_CMD_NONE;
        my_mutex_unlock(g_feTaskMutex);
        if (cmd == FE_TASK_CMD_NONE)
        {
            my_usleep(10);
            continue;
        }
        if (cmd == FE_TASK_CMD_CUSTOM_FUNC)
        {
            fn_customFunc* cf = (fn_customFunc*)args[0];
            cf(args[1]);
        }
        else if (cmd == FE_TASK_CMD_INIT_FEGN)
        {
            int *fargs = (int*)args[0];
            fr_InitEngine(fargs[0]/*iDupCheck*/, 
                        fargs[1]/*iCamFlip*/, 
                        fargs[2]/*nDnnCheckSum*/, 
                        fargs[3]/*nHCheckSum*/);
        }
        else if (cmd == FE_TASK_CMD_DO_FACE_TASK_START)
        {
            xFaceTask.Start((int)args[0]);
        }
        else if (cmd == FE_TASK_CMD_DO_FACE_TASK_STOP)
        {
            xFaceTask.Stop();
        }
    }
#endif
    return NULL;
}

void feTaskCustomFunction_arg1(void* p_fn, void* arg0)
{
    my_mutex_lock(g_feTaskMutex);
    g_feTaskCmd = FE_TASK_CMD_CUSTOM_FUNC;
    g_feTaskCmdArgs[0] = (void*)p_fn;
    g_feTaskCmdArgs[1] = arg0;
    my_mutex_unlock(g_feTaskMutex);
}

void fe_InitEngine(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum)
{
    int* fargs;
    fargs = (int*)my_malloc(sizeof(int) * 4);
    if (fargs == NULL)
    {
        my_printf("[%s] failed to malloc.\n", __func__);
        return;
    }
    fargs[0] = iDupCheck;
    fargs[1] = iCamFlip;
    fargs[2] = nDnnCheckSum;
    fargs[3] = nHCheckSum;
    my_mutex_lock(g_feTaskMutex);
    g_feTaskCmd = FE_TASK_CMD_INIT_FEGN;
    g_feTaskCmdArgs[0] = (void*)fargs;
    my_mutex_unlock(g_feTaskMutex);
}

#if (N_MAX_HAND_NUM)
void FaceEngine::GetRegisteredFeatInfo_Hand(SHandFeatInfo* pxFeatInfo)
{
    fr_GetRegisteredFeatInfo_Hand(pxFeatInfo);
}

int FaceEngine::SaveHand(PSMetaInfo pxUserInfo, SHandFeatInfo* pxFeatInfo, int* piBlkNum)
{
    if(pxUserInfo == NULL || pxFeatInfo == NULL)
        return ES_FAILED;

    int iFindIdx = dbm_GetHandIndexOfID(pxUserInfo->iID);
    if(iFindIdx < 0)
    {
        return dbm_AddHand(pxUserInfo, pxFeatInfo, piBlkNum);
    }
    else
    {
        return dbm_UpdateHand(iFindIdx, pxUserInfo, pxFeatInfo, piBlkNum);
    }
}

int FaceEngine::CreateHand(int /*iDupCheck*/, int /*iCamFlip*/, int /*nDnnCheckSum*/, int /*nHCheckSum*/)
{
    setHandShareMem_(fr_GetInputImageBuffer1(), fr_GetDiffIrImage());
    createHandEngine_();

    int ret2 = dbm_LoadHandDB();
    return ret2;
}

#endif // N_MAX_HAND_NUM

void feFaceStart(int iCmd)
{
#if 0 //kkk
#if (USE_SMP_CORE1 == 0)
    xFaceTask.Start(iCmd);
#else // USE_SMP_CORE1
    my_mutex_lock(g_feTaskMutex);
    g_feTaskCmd = FE_TASK_CMD_DO_FACE_TASK_START;
    g_feTaskCmdArgs[0] = (void*)iCmd;
    my_mutex_unlock(g_feTaskMutex);
#endif // USE_SMP_CORE1
#endif
    xFaceTask.Start(iCmd);
}

void feFaceStop()
{
#if 0 //kkk
#if (USE_SMP_CORE1 == 0)
    xFaceTask.Stop();
#else // USE_SMP_CORE1
    my_mutex_lock(g_feTaskMutex);
    g_feTaskCmd = FE_TASK_CMD_DO_FACE_TASK_STOP;
    my_mutex_unlock(g_feTaskMutex);
#endif // USE_SMP_CORE1
#endif
    xFaceTask.Stop();
}
