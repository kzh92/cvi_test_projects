#include "faceengine.h"
#include "FaceRetrievalSystem.h"
#include "DBManager.h"
#include "shared.h"
#include "common_types.h"
#include "facerecogtask.h"

#include <string.h>
//#include <malloc.h>
//#include <stdlib.h>
//#include <sys/mman.h>
//#include <fcntl.h>
FaceRecogTask xFaceTask;

int FaceEngine::Create(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum)
{
    int ret = ES_SUCCESS;
    fr_SetSecurityMode(g_xCS.x.bTwinsMode);
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
    fr_SetCameraFlip(g_xSS.iCameraRotate);
    fr_SetDupCheck(g_xSS.iEnrollFaceDupCheck);
    fr_SetLivenessCheckStrong_On_NoUser(g_xCS.x.bLivenessMode);
    fr_SetCheckOpenEyeEnable(g_xPS.x.bHijackEnable);
    fr_SetEngineState(ENS_VERIFY, fAdminMode, iUserID);
}

void FaceEngine::UnregisterFace(int nUpdateID, int isMultiDirectionMode)
{
    fr_SetCameraFlip(g_xSS.iCameraRotate);
    fr_SetDupCheck(g_xSS.iEnrollFaceDupCheck);
    fr_SetLivenessCheckStrong_On_NoUser(g_xCS.x.bLivenessMode);
    fr_SetCheckOpenEyeEnable(g_xPS.x.bHijackEnable);
    if (!isMultiDirectionMode)
        fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_ONLY_FRONT_DIRECTION_MODE, 1);
    else
        fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_MULTI_DIRECTION_MODE, 5);
}

int FaceEngine::ExtractFace(unsigned char* pbRgbData, unsigned char* pbLedOnData, float* prResultArray)
{
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
    int nRet = fr_VerifyFace();
//    nRet = ES_PROCESS;

    SEngineResult* pxEngineResult =  fr_GetEngineResult();

    prResultArray[0] = (float)nRet;
    // prResultArray[1] = (float)(pxEngineResult->fUpdateFlagA + (pxEngineResult->fUpdateFlagB << 1));
    // prResultArray[2] = pxEngineResult->fSuccessA;
    // prResultArray[3] = pxEngineResult->fSuccessB;
    prResultArray[4] = pxEngineResult->nMaxScore;
    // prResultArray[5] = pxEngineResult->arDistVals[4];
    prResultArray[6] = DESMAN_ENC_MODE==0 ? pxEngineResult->nEyeOpenness : 1;

    if(nRet == ES_SUCCESS || nRet == ES_UPDATE)
        return pxEngineResult->nFineUserIndex;

    return -1;
}

void FaceEngine::RegisterFace(float* prResultArray, int iFaceDir)
{
    int nRet = fr_RegisterFace(iFaceDir);
    prResultArray[0] = (float)(nRet);
}

void FaceEngine::InitCalibOffset()
{
    // fr_InitCalcOffsetState();
}

int FaceEngine::AutoCameraAdjust(unsigned char* pbClrData, unsigned char* pbRedOnData, float* prResultArray)
{
    // int anOffset[2] = { 0 };
    // int nRet = fr_AutoCameraAdjust(pbClrData, pbRedOnData, anOffset);
    // prResultArray[0] = (float)nRet;

    // SEngineResult* pxEngineResult =  fr_GetEngineResult();

    // if (pxEngineResult->fValid == 1)
    // {
    //     prResultArray[1] = 1;
    //     prResultArray[2] = ((float)pxEngineResult->xFaceRect.x) / 2;
    //     prResultArray[3] = ((float)pxEngineResult->xFaceRect.y) / 2;
    //     prResultArray[4] = ((float)pxEngineResult->xFaceRect.width) / 2;
    //     prResultArray[5] = ((float)pxEngineResult->xFaceRect.height) / 2;
    //     prResultArray[6] = ((float)anOffset[0]) / 2;
    //     prResultArray[7] = ((float)anOffset[1]) / 2;
    // }
    // else
    //     memset(prResultArray + 1, 0, sizeof(int) * 5);

    // return nRet;
    return 0;
}


int FaceEngine::GetRegisteredFaceImage(unsigned char* /*pbJpgData*/, int* /*pnJpgLen*/)
{
    //return fr_GetRegisteredFace(pbJpgData, pnJpgLen);
    return 0;
}

int FaceEngine::GetLastFaceData(unsigned char* /*pbFaceData*/)
{
    //return fr_GetLastFaceData(pbFaceData);
    return 0;
}

int  FaceEngine::GetLastFaceImage(unsigned char* /*pbJpgData*/, int* /*pnJpgLen*/)
{
    // return fr_GetLastFace(pbJpgData, pnJpgLen);
    return 0;
}

int  FaceEngine::SetLastFaceScene(unsigned char* /*pbRgbData*/)
{
    // return fr_SetLastScene(pbRgbData);
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