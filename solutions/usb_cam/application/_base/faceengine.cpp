#include "faceengine.h"
#include "FaceRetrievalSystem.h"
#include "DBManager.h"
#include "shared.h"
#include "common_types.h"
#include "facerecogtask.h"
#include "upgradebase.h"
#include "mount_fs.h"
#include "jpgloader.h"
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

int FaceEngine::ResetAll(int flag)
{
    int ret = ES_SUCCESS;
    SetModifyUser(1);

    int iBackupState = mount_backup_db(0);
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
    {
        end_restore_dbPart();
        iBackupState = 0;
    }

    if (flag == SM_DEL_ALL_TYPE_DEFAULT)
    {
        dbm_SetEmptyPersonDB(&iBackupState);
#if (N_MAX_HAND_NUM)
        dbm_SetEmptyHandDB(&iBackupState);
#endif // N_MAX_HAND_NUM
    }
    else if (flag == SM_DEL_ALL_TYPE_FACE)
    {
        dbm_SetEmptyPersonDB(&iBackupState);
    }
#if (N_MAX_HAND_NUM)
    else if (flag == SM_DEL_ALL_TYPE_HAND)
    {
        dbm_SetEmptyHandDB(&iBackupState);
    }
#endif // N_MAX_HAND_NUM
    else if(flag == SM_DEL_ALL_TYPE_NORMAL_FACE_USERS)
    {
        ret = dbm_RemovePersonByPrivilege(0, &iBackupState);
    }

    umount_backup_db();
    UpdateUserCount();

    return ret;
}

//returns 0 on success, otherwise returns error code
int FaceEngine::RemoveUser(int iUserID, int iType, int iSkipUpdateCount)
{
    PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByID(iUserID - 1);
#if (N_MAX_HAND_NUM)
    if (iUserID > N_MAX_PERSON_NUM)
        pxMetaInfo = dbm_GetHandMetaInfoByID(iUserID - N_MAX_PERSON_NUM - 1);
    else if (iType == SM_DEL_USER_TYPE_HAND)
        pxMetaInfo = dbm_GetHandMetaInfoByID(iUserID - 1);
#endif // N_MAX_HAND_NUM
    if(pxMetaInfo == NULL)
    {
        return MR_FAILED4_UNKNOWNUSER;
    }
    else
    {
        SetModifyUser(1);

        int iBackupState = mount_backup_db(0);
#if (N_MAX_HAND_NUM)
        if (iUserID > N_MAX_PERSON_NUM)
            dbm_RemoveHandByID(iUserID - N_MAX_PERSON_NUM - 1, &iBackupState);
        else if (iType == SM_DEL_USER_TYPE_HAND)
            dbm_RemoveHandByID(iUserID - 1, &iBackupState);
        else
#endif // N_MAX_HAND_NUM
        {
            dbm_RemovePersonByID(iUserID - 1, &iBackupState);
        }
        umount_backup_db();

        if (!iSkipUpdateCount)
            UpdateUserCount();

        return MR_SUCCESS;
    }
}

int FaceEngine::RemoveUserRange(int iUserBeginID, int iUserEndID)
{
    if (iUserBeginID > iUserEndID || iUserBeginID < 1 || iUserEndID > (N_MAX_PERSON_NUM + N_MAX_HAND_NUM))
        return MR_FAILED4_INVALIDPARAM;
    for (int i = iUserBeginID; i <= iUserEndID; i ++)
    {
        int ret = RemoveUser(i, 0, 1);
        if (ret != MR_SUCCESS && ret != MR_FAILED4_UNKNOWNUSER)
        {
            UpdateUserCount();
            return ret;
        }
    }
    UpdateUserCount();
    return MR_SUCCESS;
}

void FaceEngine::VerifyInit(int fAdminMode, int iUserID)
{
    fr_SetCameraFlip(g_xSS.iCameraRotate);
    fr_SetDupCheck(g_xSS.iEnrollFaceDupCheck);
    fr_SetLivenessCheckStrong_On_NoUser(DEFAULT_LIVENESS_MODE);
    fr_SetCheckOpenEyeEnable(g_xPS.x.bHijackEnable);
    fr_SetEngineState(ENS_VERIFY, fAdminMode, iUserID);
}

void FaceEngine::UnregisterFace(int nUpdateID, int isMultiDirectionMode)
{
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
            fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_ONLY_FRONT_DIRECTION_MODE, 1,
                              g_xSS.iRegisterMixMode == ENROLL_FACE_HAND_MIX ? EEK_UNKNOWNED : EEK_Face);
        }
    }
    else
    {
#if (USE_FUSHI_PROTO)
        if (g_xSS.iProtocolHeader == FUSHI_HEAD1)
            fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_MULTI_DIRECTION_MODE, 4);
        else
#endif // USE FUSHI_PROTO
        {
            fr_SetEngineState(ENS_REGISTER, nUpdateID, g_xSS.iDemoMode, ENROLL_MULTI_DIRECTION_MODE, 5,
                              g_xSS.iRegisterMixMode == ENROLL_FACE_HAND_MIX ? EEK_UNKNOWNED : EEK_Face);
        }
    }
}

int FaceEngine::ExtractFace(unsigned char* pbRgbData, unsigned char* pbLedOnData, float* prResultArray)
{
    if (prResultArray == NULL)
        return ES_FAILED;

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
    int nProcessMode = 0;
    int nRet = fr_RegisterCombo(iFaceDir, &nProcessMode);
    prResultArray[0] = (float)(nRet);
    prResultArray[1] = nProcessMode;
}

void FaceEngine::RegisterImage(float* prResultArray, unsigned char* pbClrBuffer, int width, int height)
{
#if (USE_RENT_ENGINE)
    int nRet = fr_EnrollClrImage(pbClrBuffer, width, height);
    prResultArray[0] = (float)(nRet);
#endif // USE_RENT_ENGINE
}

void FaceEngine::RegisterFeat(float* prResultArray, unsigned char* pbFeat, int length)
{
#if (USE_RENT_ENGINE)
    int nRet = fr_EnrollClrFeat(pbFeat, length);
    prResultArray[0] = (float)nRet;
#endif // USE_RENT_ENGINE
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

int FaceEngine::DecodeRegisterFileData(unsigned char** pBuffer, int file_len, int * puser_count, uint16_t** puser_ids)
{
    int ret = -(MR_FAILED4_INVALIDPARAM);
#if (USE_RENT_ENGINE || USE_DB_UPDATE_MODE)
    s_feat_data_v3* fd = (s_feat_data_v3*)*pBuffer;
    s_feat_data_v2* fd_v2 = (s_feat_data_v2*)*pBuffer;
    int reg_user_id = 0;
    if (*puser_count > 0)
    {
        reg_user_id = *puser_count;
        *puser_count = 0;
    }
#endif
#if (USE_RENT_ENGINE)
    if (memcmp(fd->m_header.m_magic, ENROLL_FACE_IMG_MAGIC3, sizeof(ENROLL_FACE_IMG_MAGIC3)) == 0)
    {
        dbug_printf("decode multi feat\n");
        //decode multi feat data
        if (fd->m_header.m_size != file_len)
        {
            dbug_printf("size mismatch %d, %d\n", fd->m_header.m_size, file_len);
            return -(MR_FAILED4_INVALIDPARAM);
        }
        printf("fcount=%d\n", fd->m_header.m_count);
        if (fd->m_header.m_count)
        {
            int n_reg_user_count = 0;
            if (puser_ids)
            {
                *puser_ids = (uint16_t*)malloc(sizeof(uint16_t)*fd->m_header.m_count);
                if (*puser_ids == NULL)
                {
                    return -(MR_FAILED4_NOMEMORY);
                }
                else
                {
                    memset(*puser_ids, 0, sizeof(uint16_t)*fd->m_header.m_count);
                }
            }
            else
                return -(MR_FAILED4_NOMEMORY);
            FaceEngine::UnregisterFace(-1, 0);
            for (int i = 0; i < fd->m_header.m_count; i++)
            {
                if (memcmp(fd->m_feat->m_magic, ENROLL_FACE_IMG_MAGIC2, sizeof(ENROLL_FACE_IMG_MAGIC2)) == 0)
                {
                    float arEngineResult[10] = { 0 };
                    xor_encrypt(fd->m_feat[i].feat_data, sizeof(fd->m_feat[i].feat_data),
                                (unsigned char*)ENROLL_FACE_IMG_MAGIC2, sizeof(ENROLL_FACE_IMG_MAGIC2));
                    FaceEngine::RegisterFeat(arEngineResult, fd->m_feat[i].feat_data, sizeof(fd->m_feat[i].feat_data));
                    if (arEngineResult[0] != ES_SUCCESS)
                    {
                        dbug_printf("reg fail %d, get feat\n", i);
                        return -(MR_FAILED4_UNKNOWNREASON);
                    }

                    int iUserID = -1;
                    if (i == 0 && reg_user_id > 0)
                    {
                        if (reg_user_id > N_MAX_PERSON_NUM)
                        {
                            return -(MR_FAILED4_INVALIDPARAM);
                        }
                        if (dbm_GetPersonMetaInfoByID(reg_user_id - 1))
                        {
                            return -(MR_FAILED4_INVALIDPARAM);
                        }
                        iUserID = reg_user_id - 1;
                    }
                    if (iUserID < 0)
                        iUserID = dbm_GetNewUserID();
                    if(iUserID == -1)
                    {
                        dbug_printf("reg fail %d, max user\n", i);
                        return -(MR_FAILED4_MAXUSER);
                    }

                    SMetaInfo xMetaInfo;
                    SFeatInfo xFeatInfo;
                    memset(&xMetaInfo, 0, sizeof(xMetaInfo));
                    memset(&xFeatInfo, 0, sizeof(xFeatInfo));
                    xMetaInfo.iID = iUserID;

                    FaceEngine::GetRegisteredFeatInfo(&xFeatInfo);

                    SetModifyUser(1);

                    int iBackupState = mount_backup_db(0);
                    if (iBackupState < 0)
                    {
                        dbug_printf("reg fail %d, write fail\n", i);
                        SetModifyUser(0);
                        return -(MR_FAILED4_WRITE_FILE);
                    }
                    else
                    {
                        int ret = FaceEngine::SavePerson(&xMetaInfo, &xFeatInfo, &iBackupState);
                        umount_backup_db();
                        if (ret != ES_SUCCESS)
                        {
                            dbug_printf("reg fail %d, write fail\n", i);
                            SetModifyUser(0);
                            return -(MR_FAILED4_WRITE_FILE);
                        }
                        else
                        {
                            UpdateUserCount();
                            (*puser_ids)[n_reg_user_count++] = iUserID + 1;
                            *puser_count = n_reg_user_count;
                        }
                    }

                    dbug_printf("reg ok fidx=%d, %d\n", i, iUserID);
                }
                else
                {
                    dbug_printf("header mismatch fidx=%d\n", i);
                }
            }
            return 1;
        }
    }
    else if (memcmp(fd_v2->m_magic, ENROLL_FACE_IMG_MAGIC2, sizeof(ENROLL_FACE_IMG_MAGIC2)) == 0)
    {
        float arEngineResult[10] = { 0 };
        int n_reg_user_count = 0;
        if (puser_ids)
        {
            *puser_ids = (uint16_t*)malloc(sizeof(uint16_t));
            if (*puser_ids == NULL)
            {
                return -(MR_FAILED4_NOMEMORY);
            }
            else
            {
                memset(*puser_ids, 0, sizeof(uint16_t));
            }
        }
        else
            return -(MR_FAILED4_NOMEMORY);
        FaceEngine::UnregisterFace(-1, 0);
        xor_encrypt(fd_v2->feat_data, sizeof(fd_v2->feat_data),
                    (unsigned char*)ENROLL_FACE_IMG_MAGIC2, sizeof(ENROLL_FACE_IMG_MAGIC2));
        FaceEngine::RegisterFeat(arEngineResult, fd_v2->feat_data, sizeof(fd_v2->feat_data));
        if (arEngineResult[0] != ES_SUCCESS)
        {
            dbug_printf("reg fail, get feat\n");
            return -(MR_FAILED4_UNKNOWNREASON);
        }

        int iUserID = -1;
        if (reg_user_id > 0)
        {
            if (reg_user_id > N_MAX_PERSON_NUM)
            {
                return -(MR_FAILED4_INVALIDPARAM);
            }
            if (dbm_GetPersonMetaInfoByID(reg_user_id - 1))
            {
                return -(MR_FAILED4_INVALIDPARAM);
            }
            iUserID = reg_user_id - 1;
        }

        if (iUserID < 0)
            iUserID = dbm_GetNewUserID();
        if(iUserID == -1)
        {
            dbug_printf("reg fail, max user\n");
            return -(MR_FAILED4_MAXUSER);
        }

        SMetaInfo xMetaInfo;
        SFeatInfo xFeatInfo;
        memset(&xMetaInfo, 0, sizeof(xMetaInfo));
        memset(&xFeatInfo, 0, sizeof(xFeatInfo));
        xMetaInfo.iID = iUserID;

        FaceEngine::GetRegisteredFeatInfo(&xFeatInfo);

        SetModifyUser(1);

        int iBackupState = mount_backup_db(0);
        if (iBackupState < 0)
        {
            dbug_printf("reg fail, write fail\n");
            SetModifyUser(0);
            return -(MR_FAILED4_WRITE_FILE);
        }
        else
        {
            int ret = FaceEngine::SavePerson(&xMetaInfo, &xFeatInfo, &iBackupState);
            umount_backup_db();
            if (ret != ES_SUCCESS)
            {
                dbug_printf("reg fail, write fail\n");
                SetModifyUser(0);
                return -(MR_FAILED4_WRITE_FILE);
            }
            else
            {
                UpdateUserCount();
                (*puser_ids)[n_reg_user_count++] = iUserID + 1;
                *puser_count = n_reg_user_count;
            }
        }

        dbug_printf("reg ok %d\n", iUserID);
        return 1;
    } else
#endif // USE_RENT_ENGINE
#if (USE_DB_UPDATE_MODE)    
    if(memcmp(fd->m_header.m_magic, DB_UPDATE_MAGIC, strlen(DB_UPDATE_MAGIC)) == 0)
    {
        mount_backup_db(0);
        if (FaceEngine::UpdateDbBin(*pBuffer, file_len, 1, reg_user_id))
        {
            SetModifyUser(1);
            UpdateUserCount();
            ret = MR_SUCCESS;
        }
        umount_backup_db();
    }
    else if(memcmp(fd->m_header.m_magic, DB_UPDATE_MAGIC_2, strlen(DB_UPDATE_MAGIC_2)) == 0)
    {
        mount_backup_db(0);
        if (FaceEngine::UpdateDbBin(*pBuffer, file_len, 2, reg_user_id))
        {
            SetModifyUser(1);
            UpdateUserCount();
            ret = MR_SUCCESS;
        }
        umount_backup_db();
    }
    else if(memcmp(fd->m_header.m_magic, DB_UPDATE_MAGIC_3, strlen(DB_UPDATE_MAGIC_3)) == 0)
    {
        mount_backup_db(0);
        if (FaceEngine::UpdateDbBin(*pBuffer, file_len, 3, reg_user_id))
        {
            SetModifyUser(1);
            UpdateUserCount();
            ret = MR_SUCCESS;
        }
        umount_backup_db();
    }
#endif // USE_DB_UPDATE_MODE
#if (USE_RENT_ENGINE)
    else
    {
        dbug_printf("decode jpeg data\n");
        njInit();
        if (njDecode(*pBuffer, file_len) == 0)
        {
            unsigned char* pbImgData = njGetImage();
            int w = njGetWidth();
            int h = njGetHeight();
            printf("nj %dx%d\n", w, h);
            if (w > 0 && h > 0 && w * h <= 1280*720)
            {
                int image_len = w * h * 3 + UF_HEADER_MAGIC_SIZE + 4 + 4;
                free(*pBuffer);
                *pBuffer = (unsigned char*)my_malloc(image_len);
                if (*pBuffer)
                {
                    memcpy(*pBuffer, ENROLL_FACE_IMG_MAGIC, sizeof(ENROLL_FACE_IMG_MAGIC));
                    memcpy(*pBuffer + UF_HEADER_MAGIC_SIZE, &w, 4);
                    memcpy(*pBuffer + UF_HEADER_MAGIC_SIZE + 4, &h, 4);
                    memcpy(*pBuffer + UF_HEADER_MAGIC_SIZE + 4 + 4, pbImgData, w * h * 3);
                    njDone();
                    return image_len;
                }
                else
                    ret = -(MR_FAILED4_NOMEMORY);
            }
            if (pbImgData)
                my_free(pbImgData);
        }
        else
        {
            free(*pBuffer);
            *pBuffer = NULL;
        }

        njDone();
    }
#endif // USE_RENT_ENGINE
    return ret;
}

int FaceEngine::UpdateDbBin(unsigned char* pBuffer, int iLen, int iUpdateFlag, int uid)
{
#if (USE_DB_UPDATE_MODE)
    unsigned char check_buf[MAGIC_LEN_UPDATE_DB] = {0};
    //decoding data
    memcpy(check_buf, pBuffer, MAGIC_LEN_UPDATE_DB);

    for (int i = 0 ; i < iLen - MAGIC_LEN_UPDATE_DB; i++)
        pBuffer[i + MAGIC_LEN_UPDATE_DB] = pBuffer[i + MAGIC_LEN_UPDATE_DB] ^ ((i + check_buf[i % MAGIC_LEN_UPDATE_DB]) & 0xff);

    if (dbm_UpdatePersonBin(iUpdateFlag, pBuffer + MAGIC_LEN_UPDATE_DB, iLen - MAGIC_LEN_UPDATE_DB, uid) == ES_SUCCESS)
    {
        return 1;
    }
#endif // USE_DB_UPDATE_MODE
    return 0;
}

int FaceEngine::GetPersonDbBin(unsigned char* pBuffer, int iLen, int iUpdateFlag, int iID, int iOffset)
{
#if (USE_DB_UPDATE_MODE)
    if (iUpdateFlag)
    {
        if (iOffset)
            iOffset -= MAGIC_LEN_UPDATE_DB;
        int real_offset = 0;
        if (iUpdateFlag == 1)
            real_offset = iOffset;
        else if (iUpdateFlag == 2)
        {
            if (iID < N_MAX_PERSON_NUM)
                real_offset = iOffset + sizeof(DB_INFO::aiValid) + iID * sizeof(DB_UNIT);
#if (N_MAX_HAND_NUM)
            else
                real_offset = iOffset + sizeof(DB_HAND_INFO::aiHandValid) + (iID - N_MAX_PERSON_NUM) * sizeof(DB_HAND_UNIT);
#endif
        }
        if (iOffset == 0)
        {
            if (iUpdateFlag == 1)
                memcpy(pBuffer, DB_UPDATE_MAGIC, MAGIC_LEN_UPDATE_DB);
            else if (iUpdateFlag == 2)
            {
                if (iID < N_MAX_PERSON_NUM)
                    memcpy(pBuffer, DB_UPDATE_MAGIC_2, MAGIC_LEN_UPDATE_DB);
#if (N_MAX_HAND_NUM)
                else
                    memcpy(pBuffer, DB_UPDATE_MAGIC_3, MAGIC_LEN_UPDATE_DB);
#endif
            }
            if (iID < N_MAX_PERSON_NUM)
                memcpy(pBuffer + MAGIC_LEN_UPDATE_DB,
                    dbm_GetPersonBin() + real_offset, 
                    iLen - MAGIC_LEN_UPDATE_DB);
#if (N_MAX_HAND_NUM)
            else
                memcpy(pBuffer + MAGIC_LEN_UPDATE_DB,
                    dbm_GetHandBin() + real_offset, 
                    iLen - MAGIC_LEN_UPDATE_DB);
#endif
        }
        else
        {
            if (iUpdateFlag == 1)
            {
                if (real_offset < (int)sizeof(DB_INFO))
                    memcpy(pBuffer,
                            dbm_GetPersonBin() + real_offset, 
                            iLen);
#if (N_MAX_HAND_NUM)
                if (real_offset + iLen > (int)sizeof(DB_INFO))
                {
                    if (real_offset < (int)sizeof(DB_INFO))
                    {
                        int lenHandData = iLen - (sizeof(DB_INFO) - real_offset);
                        memcpy(pBuffer + sizeof(DB_INFO) - real_offset,
                            dbm_GetHandBin(), 
                            lenHandData);
                    }
                    else
                    {
                        memcpy(pBuffer,
                            dbm_GetHandBin() + real_offset - sizeof(DB_INFO), 
                            iLen);
                    }
                }
#endif
            }
            else
            {
                if (iID < N_MAX_PERSON_NUM)
                    memcpy(pBuffer,
                        dbm_GetPersonBin() + real_offset, 
                        iLen);
#if (N_MAX_HAND_NUM)
                else
                    memcpy(pBuffer,
                        dbm_GetHandBin() + real_offset, 
                        iLen);
#endif
            }
        }

        //encoding data
        unsigned char check_buf[MAGIC_LEN_UPDATE_DB] = {0};
        if (iUpdateFlag == 1)
            memcpy(check_buf, DB_UPDATE_MAGIC, MAGIC_LEN_UPDATE_DB);
        else if (iUpdateFlag == 2)
        {
            if (iID < N_MAX_PERSON_NUM)
                memcpy(check_buf, DB_UPDATE_MAGIC_2, MAGIC_LEN_UPDATE_DB);
#if (N_MAX_HAND_NUM)
            else
                memcpy(check_buf, DB_UPDATE_MAGIC_3, MAGIC_LEN_UPDATE_DB);
#endif
        }
        for (int i = 0 ; i < iLen; i++)
        {
            int real_pos = i + iOffset;
            if (iOffset == 0)
                real_pos -= MAGIC_LEN_UPDATE_DB;
            if (real_pos < 0)
                continue;
            pBuffer[i] = pBuffer[i] ^ ((real_pos + check_buf[real_pos % MAGIC_LEN_UPDATE_DB]) & 0xff);
        }
    }
#endif // USE_DB_UPDATE_MODE
    return 0;
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
