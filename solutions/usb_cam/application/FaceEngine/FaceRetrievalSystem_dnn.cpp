#include "FaceRetrievalSystem_dnn.h"
#include "FaceRetrievalSystem_base.h"
//#include "detect.h"
//#include "modeling.h"
//#include "livemnse.h"
//#include "occ.h"
//#include "esn.h"
//#include "feat.h"
#include "dic_manage.h"
#include "settings.h"
#include "manageIRCamera.h"
#include "KDNN_handValid.h"
#include <vfs.h>



//extern int g_exposureImage;
//extern int g_exposurePrev;

extern int g_nSecondImageNeedReCheck;
extern int g_nSecondImageIsRight;

static int g_nGlassesEquiped = 0;
//static int g_nGlassesEquipedPrev = 0;
static unsigned char g_nCurInfo = 0;
// static int      g_nCurEnv = 0;
//static int		g_nPrevEnvForCameraControl = -1;//0:dark, 1:indoor, 2:oudoor

extern int* g_nIndexList;
extern int* g_nIndexList_Admin;
extern int* g_nIndexList_Dup;
extern int* g_nDNNFeatureCount_Dup;
extern int* g_nDNNFeatureCount;
extern int* g_nDNNFeatureCount_Admin;
extern unsigned short** g_prDNNEnrolledFeature_Dup;
extern unsigned short** g_prDNNEnrolledFeature;
extern unsigned short** g_prDNNEnrolledFeature_Admin;

extern int g_nEnrollePersonCount_Dup;

#define DNN_FEATURE_COUNT_PER_ENV	4


int update_DNN_EnrollFeature_sizeChanged_withEnv(unsigned short* prEnrollFeatureToUpdate, unsigned char* pbInfo, int* pnEnrolledFeatureCount, unsigned short* pVerifyFeatureToUpdate, unsigned char nVerifyInfo)
{
    int nIndexToUpdate = -1;
    int nFeatureCountPerEnv[ENV_COUNT] = { 0 };

    int nVerifyEnv = getEnvFromInfo(nVerifyInfo);

    if (*pnEnrolledFeatureCount < TOTAL_ENROLL_MAX_DNNFEATURE_COUNT)
    {
        nIndexToUpdate = *pnEnrolledFeatureCount;
        memcpy(prEnrollFeatureToUpdate + nIndexToUpdate * KDNN_FEAT_SIZE, pVerifyFeatureToUpdate, KDNN_FEAT_SIZE * sizeof(unsigned short));
        pbInfo[nIndexToUpdate] = nVerifyInfo;
        *pnEnrolledFeatureCount = *pnEnrolledFeatureCount + 1;
        return nIndexToUpdate;
    }
    else
    {

        int nEnrollFeatureIndex;
        for (nEnrollFeatureIndex = 0; nEnrollFeatureIndex < *pnEnrolledFeatureCount; nEnrollFeatureIndex++)
        {
            int nCurEnv = getEnvFromInfo(pbInfo[nEnrollFeatureIndex]);
            nFeatureCountPerEnv[nCurEnv] ++;
        }

        int nMaxFeatureEnv = -1;
        int nMaxFeatureCountInEnv = 0;
        int nEnvIndex;
        for (nEnvIndex = 0; nEnvIndex < ENV_COUNT; nEnvIndex++)
        {
            if (nFeatureCountPerEnv[nEnvIndex] > nMaxFeatureCountInEnv)
            {
                nMaxFeatureCountInEnv = nFeatureCountPerEnv[nEnvIndex];
                nMaxFeatureEnv = nEnvIndex;
            }
        }


        //0: replace in nVerifyEnv
        //1: replace in nMaxEnv
        int nReplacingMode = 0;

        if (nFeatureCountPerEnv[nVerifyEnv] > DNN_FEATURE_COUNT_PER_ENV)
        {
            nReplacingMode = 0;
        }
        else
        {
            nReplacingMode = 1;
        }

        float rMaxScore = 0;
        for (nEnrollFeatureIndex = 0; nEnrollFeatureIndex < *pnEnrolledFeatureCount; nEnrollFeatureIndex++)
        {
            int nEnrolledEnv = getEnvFromInfo(pbInfo[nEnrollFeatureIndex]);

            if (nReplacingMode == 0)//replace in nVerifyEnv
            {
                if (nEnrolledEnv == nVerifyEnv)
                {
                    float rScore = KdnnGetSimilarity(prEnrollFeatureToUpdate + nEnrollFeatureIndex * KDNN_FEAT_SIZE, pVerifyFeatureToUpdate);
                    if (rScore > rMaxScore)
                    {
                        rMaxScore = rScore;
                        nIndexToUpdate = nEnrollFeatureIndex;
                    }
                }
            }
            else//replace in nMaxEnv
            {
                if (nEnrolledEnv == nMaxFeatureEnv)
                {
                    float rScore = KdnnGetSimilarity(prEnrollFeatureToUpdate + nEnrollFeatureIndex * KDNN_FEAT_SIZE, pVerifyFeatureToUpdate);
                    if (rScore > rMaxScore)
                    {
                        rMaxScore = rScore;
                        nIndexToUpdate = nEnrollFeatureIndex;
                    }
                }
            }
        }

        if (nIndexToUpdate == -1)
        {
            return -1;
        }

        memcpy(prEnrollFeatureToUpdate + nIndexToUpdate * KDNN_FEAT_SIZE, pVerifyFeatureToUpdate, KDNN_FEAT_SIZE * sizeof(unsigned short));
        pbInfo[nIndexToUpdate] = nVerifyInfo;
    }
    return nIndexToUpdate;
}

int fr_UpdateFeat_DNN(int nUserIndex, unsigned short* prDNNFeature, unsigned char nCurInfo)
{
#ifdef DB_BAK3
    //skip feature update in case of using backup partition.
    if (is_backup_partition())
        return ES_FAILED;
#endif //DB_BAK3

    //must be correct to N person
    if (nUserIndex < 0)
        return ES_FAILED;

    PSFeatInfo pxFeatInfo = dbm_GetPersonFeatInfoByIndex(nUserIndex);
    if (pxFeatInfo == NULL)
        return ES_FAILED;

    int nRet2 = -1;

    nRet2 = update_DNN_EnrollFeature_sizeChanged_withEnv((unsigned short*)pxFeatInfo->arFeatBuffer, pxFeatInfo->ab_Info, &pxFeatInfo->nDNNFeatCount, prDNNFeature, nCurInfo);

    if(nRet2 != -1)
        return dbm_UpdatePersonFeatInfo(nUserIndex, pxFeatInfo, NULL, -1);

    return 0;
}

void*   EngineLoadAndCheckFunc(void*)
{
    g_nThreadCount = 1;

    APP_LOG("[%s] start %0.1f\n", __func__, Now());
    allocGlobalCVDicBuffer(0);
    g_thread_flag_detect = 1;
//#ifndef __RTK_OS__
//#ifdef ENGINE_USE_DevMemInit
//    if(DevMemInit() == -1)
//#endif
//#endif
    {
        loadMachineDic(MachineFlagIndex_DNN_Detect);
    }
    createDetectEngine(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Detect);
    g_thread_flag_detect = 2;
    
    g_thread_flag_model = 1;
    loadMachineDic(MachineFlagIndex_DNN_Modeling);
    createModelingEngine(g_shared_mem, 0);
    getDicChecSumChecked(MachineFlagIndex_DNN_Modeling);
    g_thread_flag_model = 2;
    
    
#ifdef ENGINE_FOR_DESSMAN
    g_thread_flag_occ = 1;
    loadMachineDic(MachineFlagIndex_DNN_OCC);
    init_occ_detection(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_OCC);
    g_thread_flag_occ = 2;
#endif

    //Live Feature Mem
    unsigned char* pLive_Feat_Mem = g_shared_mem + (128 * 128 * 3 + 88 * 128);
    g_thread_flag_spoofa1 = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_A1);
    KdnnCreateLivenessEngine_2DA1(pLive_Feat_Mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_A1);
    g_thread_flag_spoofa1 = 2;

    g_thread_flag_spoofa2 = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_A2);
    KdnnCreateLivenessEngine_2DA2(pLive_Feat_Mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_A2);
    g_thread_flag_spoofa2 = 2;

#if (ENGINE_USE_TWO_CAM != 0)//ENGINE_USE_TWO_CAM=0:2D, 1:common, 2:3M
    g_thread_flag_spoofb = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_B);
    KdnnCreateLivenessEngine_2DB(pLive_Feat_Mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B);
    g_thread_flag_spoofb = 2;

    g_thread_flag_spoofb2 = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_B2);
    KdnnCreateLivenessEngine_2DB2(pLive_Feat_Mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B2);
    g_thread_flag_spoofb2 = 2;
#endif

    g_thread_flag_spoofc = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_C);
    KdnnCreateLivenessEngine_3D(pLive_Feat_Mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_C);
    g_thread_flag_spoofc = 2;

    g_nThreadCount = 2;
    g_thread_flag_feat = 1;
    loadMachineDic(MachineFlagIndex_DNN_Feature);
    KdnnCreateEngine_feat(pLive_Feat_Mem, 0);
    g_thread_flag_feat = 2;
/*
#ifdef ENGINE_FOR_DESSMAN
    g_thread_flag_esn = 1;
    loadMachineDic(MachineFlagIndex_DNN_ESN);
	init_esn_detection(pLive_Feat_Mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_ESN);
    g_thread_flag_esn = 2;
#endif
*/

#if (N_MAX_HAND_NUM)
    g_thread_flag_detect_h = 1;
    loadMachineDic(MachineFlagIndex_DNN_Detect_Hand);
    createDetectEngine(g_shared_mem, Detect_Mode_Hand);
    getDicChecSumChecked(MachineFlagIndex_DNN_Detect_Hand);
    g_thread_flag_detect_h = 2;

    // g_thread_flag_model_h = 1;
    // loadMachineDic(MachineFlagIndex_DNN_Modeling_Hand);
    // createModelingEngine(g_shared_mem, 1);
    // getDicChecSumChecked(MachineFlagIndex_DNN_Modeling_Hand);
    // g_thread_flag_model_h = 2;


    // unsigned char* pHand_Feat_Mem = g_shared_mem + 128 * 128;
    // g_thread_flag_checkValid_h = 1;
    // loadMachineDic(MachineFlagIndex_DNN_CheckValid_Hand);
    // KdnnCreateCheckValid_Hand(pHand_Feat_Mem);
    // getDicChecSumChecked(MachineFlagIndex_DNN_CheckValid_Hand);
    // g_thread_flag_checkValid_h = 2;


    // g_thread_flag_feat_h = 1;
    // loadMachineDic(MachineFlagIndex_DNN_Feature_Hand);
    // KdnnCreateEngine_feat(pHand_Feat_Mem, 1);
    // g_thread_flag_feat_h = 2;
#endif // N_MAX_HAND_NUM
    releaseGlobalCVDicBuffer();
    APP_LOG("[%s] end %0.1f\n", __func__, Now());
    return NULL;
}

void fr_InitEngine_dnn()
{
#ifdef __RTK_OS__
    my_thread_create_ext(&g_EngineInitThrad, 0, EngineLoadAndCheckFunc, NULL, (char*)"EngineInitThread", 16 * 1024, MYTHREAD_PRIORITY_MEDIUM);
#else
    if(g_EngineInitThrad == 0)
    {
        pthread_create (&g_EngineInitThrad, 0, EngineLoadAndCheckFunc, NULL);
    }
#endif
}

int fr_PreExtractFace_dnn(unsigned char *pbClrImage, unsigned char *pbLedOnImage)
{
    fr_SetStopEngineFlag(0);
    if (pbLedOnImage == NULL)
        return ES_FAILED;
    APP_LOG("[%d] process start\n", (int)Now());

//check Exposure setting time
    int exposure = *fr_GetExposure();
    float rFrameCaptureTime = Now() - g_rExposureSettenTime;
    if(*fr_GetExposure() == INIT_EXP)
    {
        *fr_GetExposureImage() = exposure;
    }
    else
    {
        if(rFrameCaptureTime < 70)
        {
            *fr_GetExposureImage() = *fr_GetExposurePrev();
        }
        else
        {
            *fr_GetExposureImage() = exposure;
        }
    }
    *fr_GetExposurePrev() = exposure;

#ifdef LOG_MODE
    my_printf("Frame Capture Time = %f\r\n", rFrameCaptureTime);
#endif


    //APP_LOG("Exposure Left:%d %d, Right:%d %d\n", *fr_GetExposure(), *fr_GetGain(), *fr_GetExposure2(), *fr_GetGain2());

    IF_FLAG_STOP1(ES_FAILED);

//    g_nNeedToCalcNextExposure = 0;
    memset(&g_xEngineResult, 0, sizeof(SEngineResult));
    memset(getFaceProcessData(), 0, sizeof(Face_Process_Data));
#ifdef TimeProfiling
    setTimeProfilingInfo(0);
#endif
    //my_printf("g_xEngineParam.iCamFlip %d\n", g_xEngineParam.iCamFlip);    
    if(*fr_GetBayerYConvertedCameraIndex() != 0)
    {
        //convert_bayer2y_rotate_cm(pbLedOnImage, g_pbYIrImage, E_IMAGE_WIDTH, E_IMAGE_HEIGHT, g_xEngineParam.iCamFlip);
        convert_bayer2y_rotate_cm_riscv(pbLedOnImage, g_pbYIrImage, E_IMAGE_WIDTH, E_IMAGE_HEIGHT, g_xEngineParam.iCamFlip);
        *fr_GetBayerYConvertedCameraIndex() = 0;
        //printf("Camera 0 Bayer->Y converted\n");
    }

#ifdef LOG_MODE
    {
        char szImageFilePath[255];
        FILE* pFile;
        smy_printf(szImageFilePath, "%s/%05d_%03d.bin", LOG_PATH, g_nLogIndex, g_nSubIndex);
        APP_LOG("%s\n", szImageFilePath);
        pFile = fopen(szImageFilePath, "wb");
        if (pFile)
        {
            fwrite(pbLedOnImage, 1280 * 720, 1, pFile);
            fclose(pFile);
            sync();
        }
        else
        {
            my_printf("%s not created\n", szImageFilePath);
        }
        APP_LOG("[Camera control] g_exposureImage %03d\n", g_exposureImage);
    }
#endif

#ifdef TimeProfiling
    setTimeProfilingInfo(1);
#endif

#if 0
    {
        char szImageFilePath[255];
        sprintf(szImageFilePath, "/mnt/sd/g_pbYIrImage.bin");
        int fd1 = aos_open(szImageFilePath, O_CREAT | O_RDWR);
        if(fd1 >= 0)
        {

            aos_write(fd1, g_pbYIrImage, E_IMAGE_WIDTH * E_IMAGE_HEIGHT);
            aos_sync(fd1);
            aos_close(fd1);
            APP_LOG("%s saved\n", szImageFilePath);
        }
        else
        {
            APP_LOG("%s not saved\n", szImageFilePath);
        }
    }
#endif



    IF_FLAG_STOP1(ES_FAILED);

    // int nMotionX, nMotionY;
    // nMotionX = 0;
    // nMotionY = 0;
    //LOGE("AAA");
    int aLeft, aTop, aWidth, aHeight;

    //memcpy(g_pbYIrImage, pbLedOnImage, N_C_ENGINE_SIZE);
    g_xEngineResult.fValid = 0;
    *fr_GetFaceDetected() = 0;
    g_nSecondImageNeedReCheck = 0;
    int nFaceDetectSuccess = -1;
    float rFaceRects[4];
    // float rDetectTime;

    g_nGlassesEquiped = 0;

    //g_nPrevEnvForCameraControl = g_nCurEnvForCameraControl;

    //if(nNeedToDNNDetection)
    {

        IF_FLAG_STOP1(ES_FAILED);


        waitDicChecksum(&g_thread_flag_detect);
        if (g_thread_flag_detect != 2)
        {
            return ES_FAILED;
        }
        //my_printf("[%d] waitDicChecksum(&g_thread_flag_detect)\n", (int)Now());

//#ifdef TimeProfiling
//        float rStartTime1 = Now();
//#endif
        g_pbFaceDetectionBuffer = g_shared_mem + getDetectMenSize();
        FaceInfo *face_info = (FaceInfo *)g_pbFaceDetectionBuffer;
        int n_face_cnt = 0;
        detect(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, face_info, MAX_FACE_NUM, &n_face_cnt, g_pbFaceDetectionBuffer, 0);

        IF_FLAG_STOP1(ES_FAILED);

        int nFaceNum = 0;
        nFaceNum = n_face_cnt;

#ifdef TimeProfiling
        setTimeProfilingInfo(2);
#endif

        if(nFaceNum)
        {
            IF_FLAG_STOP1(ES_FAILED);
            int nBadCode = 0;
            int nBestFaceIndex;
            nBestFaceIndex = getBestIndex(face_info, n_face_cnt, nBadCode);
            FaceInfo face = face_info[nBestFaceIndex];
            rFaceRects[0] = face.x1;
            rFaceRects[1] = face.y1;
            rFaceRects[2] = face.x2;
            rFaceRects[3] = face.y2;
            //my_printf("nFaceRects = %f %f %f %f\n", face.x1, face.y1, face.x2, face.y2);

            nFaceDetectSuccess = 0;
            *fr_GetFaceDetected() = 1;

            APP_LOG("[%d] pec 1-0 %d %d %d %d\n", (int)Now(), (int)rFaceRects[0], (int)rFaceRects[1], (int)rFaceRects[2], (int)rFaceRects[3]);
            int nFaceWidth = (float)(rFaceRects[2] - rFaceRects[0]);
            int nFaceHeight = (float)(rFaceRects[3] - rFaceRects[1]);
            int nFaceMaxLine = nFaceWidth > nFaceHeight ? nFaceWidth : nFaceHeight;

            if(nBadCode)
            {
                nFaceDetectSuccess = -1;
                if(nBadCode == 1)//face too small
                {
                    APP_LOG("[%d] pec 2-1 %d\n", (int)Now(), nFaceMaxLine);
                    g_xEngineResult.nFaceNearFar = 2;
                }
                else if(nBadCode == 2)//face too big
                {
                    APP_LOG("[%d] pec 2-2 %d\n", (int)Now(), nFaceMaxLine);
                    g_xEngineResult.nFaceNearFar = 1;
                }
                else if(nBadCode == 3)//face is out of image
                {
                    APP_LOG("[%d] pec 2-3\n", (int)Now());
                }
                else//low score
                {
                    APP_LOG("[%d] pec 2-4 %f\n", (int)Now(), face.score);
                }
            }
            IF_FLAG_STOP1(ES_FAILED);
        }
        else
        {
            APP_LOG("[%d] pec 1\n", (int)Now());
            nFaceDetectSuccess = -1;
//            my_printf("Face is not detected\r\n");
        }
    }

    IF_FLAG_STOP1(ES_FAILED);

    if(*fr_GetFaceDetected())
    {
        refineRect(rFaceRects, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight);
        aLeft = (int)rFaceRects[0];
        aTop = (int)rFaceRects[1];
        aWidth = (int)(rFaceRects[2] - rFaceRects[0]);
        aHeight = (int)(rFaceRects[3] - rFaceRects[1]);

        SRect rectAvg;
        rectAvg.x = aLeft + aWidth * 0.2f;
        rectAvg.y = aTop + aHeight * 0.1f;
        rectAvg.width = aWidth * 0.6f;
        rectAvg.height = aHeight * 0.8f;
        calcAverageValues(g_pbYIrImage, 0, rectAvg);

        APP_LOG("[%d] pes 1 %d %f\n", (int)Now(), (int)g_rAverageLedOnImage, g_rSaturatedRate);
 
#ifdef TimeProfiling
        setTimeProfilingInfo(3);
#endif

//        g_rAverageDiffImage = g_rAverageLedOnImage;

//#ifdef LOG_MODE
//        APPl_LOG("g_rAverageLedOnImage = %d\n", g_rAverageLedOnImage);
//#endif

//        if (g_rAverageLedOnImage < nMIN_USER_LUM || g_rAverageLedOnImage > nMAX_USER_LUM)
//        {
//            g_nNeedToCalcNextExposure = 1;
//        }
//        if (g_rSaturatedRate > SAT_THRESHOLD)
//        {
//            g_nNeedToCalcNextExposure = 1;
//        }
//        if(g_nNeedToCalcNextExposure)
        {
            *fr_GetExposure() = *fr_GetExposureImage();
            *fr_MainControlCameraIndex() = 0;
        }
        APP_LOG("pes %d Satu %f\n", g_rAverageLedOnImage, g_rSaturatedRate);

    }
    APP_LOG("pes 11 %d %d\n", *fr_GetFaceDetected(), nFaceDetectSuccess);
    IF_FLAG_STOP1(ES_FAILED);
    if (!nFaceDetectSuccess)
    {
        int nMIN_DNN_LUM = MIN_DNN_LUM;

        if (g_rAverageLedOnImage > nMIN_DNN_LUM && (g_rSaturatedRate < SAT_THRESHOLD))
        {
            (getFaceProcessData())->rFaceRect[0] = rFaceRects[0];
            (getFaceProcessData())->rFaceRect[1] = rFaceRects[1];
            (getFaceProcessData())->rFaceRect[2] = rFaceRects[2] - rFaceRects[0];
            (getFaceProcessData())->rFaceRect[3] = rFaceRects[3] - rFaceRects[1];
            (getFaceProcessData())->nFaceDetected = 1;

            *fr_GetMainProcessCameraIndex() = 0;
        }

        //g_nLastDetectionSuccess = 1;
#ifdef TimeProfiling
        setTimeProfilingInfo(4);
#endif
        APP_LOG("[%d] pec 0\n", (int)Now());
        return ES_SUCCESS;
    }
    else
    {
        //g_nLastDetectionSuccess = 0;
        if (!g_nCurEnvForCameraControlSettenFromOffImage && (*fr_GetEntireImageAnalysed() == 0))
        {
            APP_LOG("pes 2 o\n");
            int nExpValue = (int)((float)(*fr_GetExposureImage()) * getGainRateFromGain_SC2355(*fr_GetGain(), *fr_GetFineGain()));
            analyseBufferAndUpgradeCurEnvForCameraControl(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, nExpValue, 0);
            *fr_GetEntireImageAnalysed() = 0;
        }

#ifdef LOG_MODE
//        my_printf("No Face Reply time = %f\r\n", Now()- g_rLastTime);
//        g_rLastTime = Now();
#endif

        //if (g_nCurEnvForCameraControl > 0 || (g_nCurEnvForCameraControl == 0 && nPrevCurEnvForCameraControl > 0))
        {
            g_nNeedToCalcNextExposure = 1;
            return ES_SUCCESS;
        }

        return ES_FAILED;
    }
}

int		fr_PreExtractFace2_dnn(unsigned char *pbBayerFromCamera2)
{
    if (pbBayerFromCamera2 == NULL)
        return ES_FAILED;

    //check first camera face is good.
    int nFirstImageFaceAvailable = 1;
    int nSecondImageFaceAvailable = 1;

#ifdef LOG_MODE
    {
        char szImageFilePath[255];
        FILE* pFile;
        smy_printf(szImageFilePath, "%s/%05d_%03d_r.bin", LOG_PATH, g_nLogIndex, g_nSubIndex);
        APP_LOG("%s\n", szImageFilePath);
        pFile = fopen(szImageFilePath, "wb");
        if (pFile)
        {
            fwrite(pbBayerFromCamera2, 1280 * 720, 1, pFile);
            fclose(pFile);
            sync();
        }
        else
        {
            my_printf("%s not created\n", szImageFilePath);
        }
    }
#endif

    int rAverageLedOnImage_temp = g_rAverageLedOnImage;//save left camera values
    float rSaturatedRate_temp = g_rSaturatedRate;
    memcpy(g_nHistInLEDOnImage_temp, g_nHistInLEDOnImage, sizeof(int) * 256);
    int nFaceDetectedinLeft = *fr_GetFaceDetected();
    int nRightCameraNeedToChceck = 1;

    //check current exp state
    int nLRExpGainEqual = 1;
    if(*fr_GetExposure() != *fr_GetExposure2() || *fr_GetGain() != *fr_GetGain2())
    {
        nLRExpGainEqual = 0;
    }

    if (nFaceDetectedinLeft && g_rAverageLedOnImage < MIN_DNN_LUM && *fr_GetExposure() > *fr_GetExposure2())
    {
        nRightCameraNeedToChceck = 0;
    }

    if(!(getFaceProcessData())->nFaceDetected)
    {
        nFirstImageFaceAvailable = 0;
    }

    APP_LOG("pes 3 = %d %d\n", nFirstImageFaceAvailable, nLRExpGainEqual);
    if(nLRExpGainEqual)
    {
        if(nFirstImageFaceAvailable)
        {
#ifdef TimeProfiling
//        float rStartTime1 = Now();
#endif
            int nFaceCheck = checkFaceInCamera2Image_expand_shrink_DNN(pbBayerFromCamera2, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, (getFaceProcessData())->rFaceRect, 1);
#ifdef TimeProfiling
            setTimeProfilingInfo(5);             
#endif
            //nFaceCheck = 1;
            if(!nFaceCheck)
            {
                (getFaceProcessData())->nFaceDetected = 0;
                APP_LOG("pes 4\n");
            }
            IF_FLAG_STOP1(ES_FAILED);
        }
        else
        {
            (getFaceProcessData())->nFaceDetected = 0;
        }

        IF_FLAG_STOP1(ES_FAILED);
    }
    else
    {
        int nFaceDetectedinRight = 0;
        if(!nFirstImageFaceAvailable  && nRightCameraNeedToChceck)
        {
            int nFaceDetectSuccess = 0;
            float rFaceRects[4];
#ifdef TimeProfiling
        float rStartTime1 = Now();
#endif
            if(*fr_GetBayerYConvertedCameraIndex() != 1)
            {
                convert_bayer2y_rotate_cm_riscv(pbBayerFromCamera2, g_pbYIrImage, E_IMAGE_WIDTH, E_IMAGE_HEIGHT, 1 - g_xEngineParam.iCamFlip);
                *fr_GetBayerYConvertedCameraIndex() = 1;
                //printf("Camera 1 Bayer->Y converted\n");

            }
            IF_FLAG_STOP1(ES_FAILED);

#ifdef TimeProfiling
            my_printf("[%d] convert_bayer2y_rotateRightTime = %f, Time111 = %f\r\n", (int)Now(), Now() - rStartTime1, Now() - g_rStartTime);
            rStartTime1 = Now();
#endif
            g_pbFaceDetectionBuffer = g_shared_mem + getDetectMenSize();
            FaceInfo *face_info = (FaceInfo *)g_pbFaceDetectionBuffer;
            int n_face_cnt = 0;
            detect(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, face_info, MAX_FACE_NUM, &n_face_cnt, g_pbFaceDetectionBuffer);

            IF_FLAG_STOP1(ES_FAILED);

#ifdef TimeProfiling
            my_printf("[%d] detectRightTime = %f, Time111 = %f\r\n", (int)Now(), Now() - rStartTime1, Now() - g_rStartTime);
            rStartTime1 = Now();
#endif

            int nFaceNum = 0;
            nFaceNum = n_face_cnt;
            if(nFaceNum)
            {
                IF_FLAG_STOP1(ES_FAILED);
                int nBadCode = 0;
                int nBestFaceIndex;
                nBestFaceIndex = getBestIndex(face_info, n_face_cnt, nBadCode);
                FaceInfo face = face_info[nBestFaceIndex];
                rFaceRects[0] = face.x1;
                rFaceRects[1] = face.y1;
                rFaceRects[2] = face.x2;
                rFaceRects[3] = face.y2;

                nFaceDetectSuccess = 0;
                nFaceDetectedinRight = 1;

                APP_LOG("[%d] pec 1-0r %d %d %d %d\n", (int)Now(), (int)rFaceRects[0], (int)rFaceRects[1], (int)rFaceRects[2], (int)rFaceRects[3]);
                int nFaceWidth = (int)(rFaceRects[2] - rFaceRects[0]);
                int nFaceHeight = (int)(rFaceRects[3] - rFaceRects[1]);
                int nFaceMaxLine = nFaceWidth > nFaceHeight ? nFaceWidth : nFaceHeight;

                if(nBadCode)
                {
                    nFaceDetectSuccess = -1;
                    if(nBadCode == 1)//face too small
                    {
                        APP_LOG("[%d] pec 2r-1 %d\n", (int)Now(), nFaceMaxLine);
                        g_xEngineResult.nFaceNearFar = 2;
                    }
                    else if(nBadCode == 2)//face too big
                    {
                        APP_LOG("[%d] pec 2r-2 %d\n", (int)Now(), nFaceMaxLine);
                        g_xEngineResult.nFaceNearFar = 1;
                    }
                    else if(nBadCode == 3)//face is out of image
                    {
                        APP_LOG("[%d] pec 2r-3\n", (int)Now());
                    }
                    else//low score
                    {
                        APP_LOG("[%d] pec 2r-4 %f\n", (int)Now(), face.score);
                    }
                }
                IF_FLAG_STOP1(ES_FAILED);
            }
            else
            {
                APP_LOG("[%d] pec 1r\n", (int)Now());
                nFaceDetectSuccess = -1;
    //            my_printf("Face is not detected\r\n");
            }

            if(nFaceDetectedinRight)
            {
                int aLeft, aTop, aWidth, aHeight;
                refineRect(rFaceRects, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight);

                aLeft = (int)rFaceRects[0];
                aTop = (int)rFaceRects[1];
                aWidth = (int)(rFaceRects[2] - rFaceRects[0]);
                aHeight = (int)(rFaceRects[3] - rFaceRects[1]);

                SRect rectAvg;
                rectAvg.x = aLeft + aWidth * 0.2f;
                rectAvg.y = aTop + aHeight * 0.1f;
                rectAvg.width = aWidth * 0.6f;
                rectAvg.height = aHeight * 0.8f;
                calcAverageValues(g_pbYIrImage, 0, rectAvg);
                IF_FLAG_STOP1(ES_FAILED);

#ifdef TimeProfiling
        my_printf("[%d] calcAverageValuesRightTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif
                APP_LOG("pes r %d %f\n", g_rAverageLedOnImage, g_rSaturatedRate);
            }
            APP_LOG("pes 2r %d  %d\n", nFaceDetectedinRight, nFaceDetectSuccess);
            if(nFaceDetectSuccess)
            {
                nSecondImageFaceAvailable = 0;
            }
            else
            {
                if (g_rAverageLedOnImage < MIN_DNN_LUM || (g_rSaturatedRate > SAT_THRESHOLD))
                {
                    nSecondImageFaceAvailable = 0;
                }
            }

            if(nSecondImageFaceAvailable)
            {
                (getFaceProcessData())->rFaceRect[0] = rFaceRects[0];
                (getFaceProcessData())->rFaceRect[1] = rFaceRects[1];
                (getFaceProcessData())->rFaceRect[2] = rFaceRects[2] - rFaceRects[0];
                (getFaceProcessData())->rFaceRect[3] = rFaceRects[3] - rFaceRects[1];
                (getFaceProcessData())->nFaceDetected = 1;

                *fr_GetMainProcessCameraIndex() = 1;
                //g_nLastDetectionSuccess = 1;
            }

            if(!nFaceDetectedinLeft)
            {
                g_nSecondImageNeedReCheck = 1;
                g_nSecondImageIsRight = 0;
            }
            else
            {
                g_nSecondImageNeedReCheck = 0;
            }
        }
        else if(nFirstImageFaceAvailable)//first camera face is valid
        {
#ifdef TimeProfiling
//        float rStartTime1 = Now();
#endif
            int nFaceCheck = checkFaceInCamera2Image_expand_shrink_DNN(pbBayerFromCamera2, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, (getFaceProcessData())->rFaceRect, 1);
#ifdef TimeProfiling
        // my_printf("[%d] checkFaceInCamera2ImageTime = %f\r\n", (int)Now(), Now() - rStartTime1);
        // my_printf("[%d] checkFaceInCamera2ImageTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
            setTimeProfilingInfo(5);
#endif
            //temp code
            //nFaceCheck = 1;
            if(!nFaceCheck)
            {
                g_nSecondImageNeedReCheck = 1;
                g_nSecondImageIsRight = 1;
                APP_LOG("pes 5\n");
            }
            else
            {
                g_nSecondImageNeedReCheck = 0;
                APP_LOG("pes 51\n");
            }
            IF_FLAG_STOP1(ES_FAILED);
        }

        //camera control decide
        if(!nFaceDetectedinLeft && !nFaceDetectedinRight)
        {
            //g_nNeedToCalcNextExposure = 1;
            *fr_GetFaceDetected() = 0;
        }
        if(nFaceDetectedinLeft && !nFaceDetectedinRight)
        {
            g_rAverageLedOnImage = rAverageLedOnImage_temp;//restore left camera values
            g_rSaturatedRate = rSaturatedRate_temp;
            memcpy(g_nHistInLEDOnImage, g_nHistInLEDOnImage_temp, sizeof(int) * 256);
            g_rAverageDiffImage = g_rAverageLedOnImage;
            //g_nNeedToCalcNextExposure = 1;
            *fr_GetFaceDetected() = 1;
            //*fr_GetExposure() = g_exposureImage;//nedd to decide
            *fr_MainControlCameraIndex() = 0;
            APP_LOG("pes 6\n");

        }
        if(!nFaceDetectedinLeft && nFaceDetectedinRight)
        {
            g_rAverageDiffImage = g_rAverageLedOnImage;
            //g_nNeedToCalcNextExposure = 1;
            *fr_GetFaceDetected() = 1;
            //*fr_GetExposure() = g_exposureImage;//nedd to decide
            *fr_MainControlCameraIndex() = 1;
            APP_LOG("pes 7\n");

        }
        if(nFaceDetectedinLeft && nFaceDetectedinRight)
        {
            int nGoodCameraIndex = 0;
            if(rAverageLedOnImage_temp < g_rAverageLedOnImage)
            {
                nGoodCameraIndex = 1;
            }
            if(rSaturatedRate_temp < g_rSaturatedRate)
            {
                nGoodCameraIndex = 0;
            }
            else
            {
                nGoodCameraIndex = 1;
            }
            if(nGoodCameraIndex == 0)
            {
                g_rAverageLedOnImage = rAverageLedOnImage_temp;//restore left camera values
                g_rSaturatedRate = rSaturatedRate_temp;
                memcpy(g_nHistInLEDOnImage, g_nHistInLEDOnImage_temp, sizeof(int) * 256);
            }
            //g_nNeedToCalcNextExposure = 1;
            *fr_GetFaceDetected() = 1;
            //*fr_GetExposure() = g_exposureImage;//nedd to decide
            *fr_MainControlCameraIndex() = nGoodCameraIndex;
            APP_LOG("pes 10-1 %d\n", nGoodCameraIndex);

        }
    }

    APP_LOG("pes 11 %d\n", g_nSecondImageNeedReCheck);
    APP_LOG("pes 12  %d\n", g_nSecondImageIsRight);
    APP_LOG("pes 13  %d\n", g_nNeedToCalcNextExposure);

    return ES_SUCCESS;

}

int fr_ExtractFace_dnn()
{
#ifdef TimeProfiling
//    my_printf("[%d] fr_ExtractFace_dnn start Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    IF_FLAG_STOP1(ES_FAILED);
    if (!(getFaceProcessData())->nFaceDetected)
    {
       APP_LOG("[%d] pec 3 %d %d\n", (int)Now(), g_xEngineResult.nFaceNearFar, g_xEngineResult.nFacePosition);

    //        my_printf("Face far = %d\r\n", g_xEngineResult.nFaceNearFar);
    //        my_printf("Face position = %d\r\n", g_xEngineResult.nFacePosition);
       return ES_FAILED;
    }

#ifdef TimeProfiling
    //float rStartTime1 = Now();
#endif


    IF_FLAG_STOP1(ES_FAILED);
    {
        waitDicChecksum(&g_thread_flag_model);
        if (g_thread_flag_model != 2)
        {
            return ES_FAILED;
        }
        
        unsigned char* pAlignBufferForModeling = g_shared_mem;
        int nRet = getFaceModelPoint(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, pAlignBufferForModeling, (getFaceProcessData())->rFaceRect, (getFaceProcessData())->rLandmarkPoint);
#ifdef TimeProfiling
        // my_printf("[%d] getFaceModelPointTime = %f Time111 = %f\r\n", (int)Now(), Now() - rStartTime1, Now() - g_rStartTime);
        // rStartTime1 = Now();
        setTimeProfilingInfo(6);             
#endif
        
        if(nRet)
        {
            return ES_FAILED;
        }
        IF_FLAG_STOP1(ES_FAILED);

        //memcpy((getFaceProcessData())->rLandmarkPoint, g_rLandmarkPoint, sizeof(float) * 68 * 2);
        (getFaceProcessData())->nFaceModelExtracted =1;

#ifdef TimeProfiling
        //rStartTime1 = Now();
#endif

#ifdef ENGINE_FOR_DESSMAN
        //occlusion
        IF_FLAG_STOP1(ES_FAILED);

        if((g_xEngineParam.fEngineState == ENS_REGISTER && g_nPassedDirectionCount == 0) || g_xEngineParam.fEngineState == ENS_VERIFY )
        {
            waitDicChecksum(&g_thread_flag_occ);
            if(g_thread_flag_occ != 2)
                return ES_FAILED;

            float score[2];
            int nOccu = get_occ_detection(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, (getFaceProcessData())->rLandmarkPoint, score, g_pbFaceDetectionBuffer);

            IF_FLAG_STOP1(ES_FAILED);

            g_xEngineResult.nOcclusion = nOccu == 1 ? 0 : 1;
        }

//        my_printf("g_xEngineResult.nOcclusion = %d\r\n", g_xEngineResult.nOcclusion);
        if(g_xEngineResult.nOcclusion)
        {
            APP_LOG("[%d] pec 5\n", (int)Now());
            return ES_FAILED;
        }
#ifdef TimeProfiling
        float rOcclusTime = Now() - rStartTime1;
        my_printf("[%d] get_occ_detectionTime = %f\r\n", (int)Now(), rOcclusTime );
        my_printf("[%d] get_occ_detectionTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

#endif

        g_xEngineResult.fValid = 1;

        g_xEngineResult.xFaceRect.x = (float)((getFaceProcessData())->rFaceRect[0]);
        g_xEngineResult.xFaceRect.y = (float)((getFaceProcessData())->rFaceRect[1]);
        g_xEngineResult.xFaceRect.width = (float)((getFaceProcessData())->rFaceRect[2]);
        g_xEngineResult.xFaceRect.height = (float)((getFaceProcessData())->rFaceRect[3]);

        //LOG_PRINT("========face rect = %d, %d, %d, %d\n", g_xEngineResult.xFaceRect.x, g_xEngineResult.xFaceRect.y, g_xEngineResult.xFaceRect.width, g_xEngineResult.xFaceRect.height);
#ifdef TimeProfiling
        setTimeProfilingInfo(7);
#endif

#if 0
        ExtractFaceImage();
#endif
    }

    APP_LOG("[%d] pec 00\n", (int)Now());
#ifdef TimeProfiling
    //rStartTime1 = Now();
#endif
    unsigned char *pLiveAlignAC, *pLiveAlignB, *pLiveAlignB2, *pLiveAlignFeat;
    pLiveAlignAC = g_shared_mem;
    pLiveAlignB = pLiveAlignAC + 128*128;
    pLiveAlignB2 = pLiveAlignB + 128*128;
    pLiveAlignFeat = pLiveAlignB2 + 88*128;
    generateAlignImageForLiveness(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, pLiveAlignAC, pLiveAlignB, pLiveAlignB2, (getFaceProcessData())->rLandmarkPoint);
    generateAlignImageForFeature(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, pLiveAlignFeat, (getFaceProcessData())->rLandmarkPoint);
#ifdef TimeProfiling
    setTimeProfilingInfo(8);
#endif

    return ES_SUCCESS;
}

int	fr_RegisterFace_dnn(int iFaceDir)
{
    g_nEnrollFeatureAdded = 0;
    IF_FLAG_STOP1(ES_FAILED);

    float rStart = Now();
    float rPassTime;
    float rPassTimeThreshold = 70;

    if (g_xEngineResult.fValid == 0 /*|| fr_GetNeedSmallFaceCheck() == 0*/)
    {
        if (g_nNeedToCalcNextExposure)
        {
           rPassTime = Now() - rStart;
           if (rPassTime < rPassTimeThreshold)
           {
               my_usleep((rPassTimeThreshold - rPassTime) * 1000);
           }
        }
        return ES_PROCESS;
    }
    APP_LOG("[%d] pdc 2 %d\n", (int)Now(), g_nPassedDirectionCount);

    if(g_nEnrollDirectionMode == ENROLL_ONLY_FRONT_DIRECTION_MODE)
    {
        iFaceDir = ENROLL_DIRECTION_FRONT_CODE_0;
    }

    if(g_nPassedDirectionCount == g_nEnrollDirectionMax)
    {
        if (g_nNeedToCalcNextExposure)
        {
           rPassTime = Now() - rStart;
           if (rPassTime < rPassTimeThreshold)
           {
               my_usleep((rPassTimeThreshold - rPassTime) * 1000);
           }
        }
        APP_LOG("[%d] pec 9\n", (int)Now());
        return ES_FAILED;
    }


    if(!isCorrectPose(iFaceDir) && g_xEngineParam.iDemoMode != N_DEMO_FACTORY_MODE)
    {
        if (g_nNeedToCalcNextExposure)
        {
           rPassTime = Now() - rStart;
           if (rPassTime < rPassTimeThreshold)
           {
               my_usleep((rPassTimeThreshold - rPassTime) * 1000);
           }
        }
        return ES_DIRECTION_ERROR;
    }

    IF_FLAG_STOP1(ES_FAILED);

    int nNeedToSkip = 0;

#if (FAKE_DETECTION == 1 && ENROLL_FAKE == 1)
    //float rO = Now();

#ifdef TimeProfiling
        float rTempStartTime = Now();
#endif

    IF_FLAG_STOP1(ES_FAILED);

    if(((fr_GetNeedSmallFaceCheck() == 1 && checkFaceIsInPhoto() != ES_SUCCESS) || fr_GetNeedSmallFaceCheck() == 0) && g_xEngineParam.iDemoMode != N_DEMO_FACTORY_MODE)
    {
        *getContinueRealCount() = 0;
        if(fr_GetNeedSmallFaceCheck() == 1 && checkFaceIsInPhoto() != ES_SUCCESS)
        {
            APP_LOG("[%d] pec 26-ps\n", (int)Now());
        }
        APP_LOG("[%d] pec 26-ps\n", (int)Now());
    }
    else
    {
        *getContinueRealCount() = *getContinueRealCount() + 1;
    }


#ifdef TimeProfiling
    my_printf("[%d] 2D 3D fake Time = %f\r\n", (int)Now(), Now() - rTempStartTime);
    my_printf("[%d] 2D 3D fake Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    APP_LOG("[%d] pec 17 %d\n", (int)Now(), *getContinueRealCount());

    if(*getContinueRealCount() < USER_CONTINUE_FRAME)
    {
        g_nFakeFrameCountInOneDirection ++;
        APP_LOG("[%d] pec 18 %d\n", (int)Now(), g_nFakeFrameCountInOneDirection);

        if(g_nFakeFrameCountInOneDirection > MAX_FakeFrameCountInOneDirection && g_nPassedDirectionCount != 0)
        {
            nNeedToSkip = 1;
        }
        else
        {
            if (g_nNeedToCalcNextExposure)
            {
               rPassTime = Now() - rStart;
               if (rPassTime < rPassTimeThreshold)
               {
                   my_usleep((rPassTimeThreshold - rPassTime) * 1000);
               }
            }
            return ES_PROCESS;
        }
    }
#endif

    IF_FLAG_STOP1(ES_FAILED);

    *getDNNFeatureExtracted() = 0;
    extarctDNNFeature_process();
    unsigned short* arLastDNNFeature = getLastDNNFeature();

    IF_FLAG_STOP1(ES_FAILED);

    if(!(*getDNNFeatureExtracted()))
    {
//        my_printf("extract DNN Feature falied\r\n");
        if (g_nNeedToCalcNextExposure)
        {
           rPassTime = Now() - rStart;
           if (rPassTime < rPassTimeThreshold)
           {
               my_usleep((rPassTimeThreshold - rPassTime) * 1000);
           }
        }
        return ES_PROCESS;
    }

    if(g_nPassedDirectionCount == 0)
    {
        memcpy((unsigned short*)(g_xEnrollFeatA->arFeatBuffer) + g_xEnrollFeatA->nDNNFeatCount * KDNN_FEAT_SIZE, arLastDNNFeature, sizeof(unsigned short)* KDNN_FEAT_SIZE);
        g_xEnrollFeatA->ab_Info[g_xEnrollFeatA->nDNNFeatCount] = g_nCurInfo;
        g_xEnrollFeatA->nDNNFeatCount = g_xEnrollFeatA->nDNNFeatCount + 1;
        APP_LOG("[%d] pec 19\n", (int)Now());
        g_nEnrollFeatureAdded = 1;

    }
    else
    {
        int pnDNNFeatureCount[2];
        unsigned short* prDNNEnrolledFeature[2];

        pnDNNFeatureCount[0] = g_xEnrollFeatA->nDNNFeatCount;
        prDNNEnrolledFeature[0] = (unsigned short*)g_xEnrollFeatA->arFeatBuffer;

        float rDNNScore;
        int nDNNMaxScoreIndex;
        GetMaxDNNScore(prDNNEnrolledFeature, pnDNNFeatureCount, 1, arLastDNNFeature, &rDNNScore, &nDNNMaxScoreIndex);
        APP_LOG("[%d] pec 20 %f\n", (int)Now(), rDNNScore);

        if(rDNNScore < DNN_ENROLL_CHECK_THRESHOLD)
        {
            APP_LOG("[%d] pec 21\n", (int)Now());

            return ES_PROCESS;
        }

        if(!nNeedToSkip && isGoodFaceToEnroll())
        {
//            my_printf("In Enrolling DNN Feature added.\r\n");

            memcpy((unsigned short*)(g_xEnrollFeatA->arFeatBuffer) + g_xEnrollFeatA->nDNNFeatCount * KDNN_FEAT_SIZE, arLastDNNFeature, sizeof(unsigned short)* KDNN_FEAT_SIZE);
            g_xEnrollFeatA->ab_Info[g_xEnrollFeatA->nDNNFeatCount] = g_nCurInfo;
            g_xEnrollFeatA->nDNNFeatCount = g_xEnrollFeatA->nDNNFeatCount + 1;
            g_nEnrollFeatureAdded = 1;
        }
    }

    IF_FLAG_STOP1(ES_FAILED);

//    if(g_nPassedDirectionCount == 0)
//    {
//        memcpy(g_pbRegisteredFaceImage, g_pbLastFaceImage, sizeof(g_pbRegisteredFaceImage));
//    }

    g_nPassedDirectionCount ++;
    g_nFakeFrameCountInOneDirection_Prev = g_nFakeFrameCountInOneDirection;

    int nDuplicated = 0;
    if(g_xEngineParam.iDupCheck && g_nEnrollePersonCount_Dup)
    {
        float rDNNScore;
        int nDNNMaxScoreIndex;
        GetMaxDNNScore(g_prDNNEnrolledFeature_Dup, g_nDNNFeatureCount_Dup, g_nEnrollePersonCount_Dup, arLastDNNFeature, &rDNNScore, &nDNNMaxScoreIndex);
        if (rDNNScore > DNN_VERIFY_THRESHOLD)
        {
            APP_LOG("[%d] pec 22\n", (int)Now());
            nDuplicated = 1;
        }
    }

    if (g_nPassedDirectionCount == g_nEnrollDirectionMax)
    {
        if (nDuplicated && (g_nEnrollDirectionMax == 1))
        {
            APP_LOG("[%d] pec 25\n", (int)Now());
            return ES_DUPLICATED;
        }
        return ES_SUCCESS;
    }

    g_nFakeFrameCountInOneDirection = 0;

    if (nDuplicated)
    {
        APP_LOG("[%d] pec 25\n", (int)Now());

        return ES_DUPLICATED;
    }
    return ES_ENEXT;
}

int     fr_RevertEnrollStep_dnn()
{
    g_nPassedDirectionCount--;
    if(g_nPassedDirectionCount < 0)
    {
        g_nPassedDirectionCount = 0;
    }
    if(g_nEnrollFeatureAdded)
    {
        g_xEnrollFeatA->nDNNFeatCount--;
        if(g_xEnrollFeatA->nDNNFeatCount < 0)
        {
            g_xEnrollFeatA->nDNNFeatCount = 0;
        }
    }
    g_nFakeFrameCountInOneDirection = g_nFakeFrameCountInOneDirection_Prev;
    return ES_SUCCESS;
}




int fr_Retrieval_dnn()
{
    if(!isFaceISGoodForDNN())
        return ES_PROCESS;

//    int nFineUserIndex = -1;

    PSMetaInfo pUserInfo;
    PSFeatInfo pFeatInfo;

    int nUserCount = dbm_GetPersonCount();
    int nPersonCount = 0;
    int nAdminPersonCount = 0;

    //DATETIME_32 xNow = dbm_GetCurDateTime();
    //my_printf("--------------------------------\n");
    //float rTime = Now();
    for (int i = 0; i < nUserCount; i++)
    {
        pUserInfo = dbm_GetPersonMetaInfoByIndex(i);
        pFeatInfo = dbm_GetPersonFeatInfoByIndex(i);

//        if(g_xEngineParam.iVerifyID != -1)
//        {
//            if(pUserInfo->iID != g_xEngineParam.iVerifyID)
//                continue;
//        }
//        else
//        {
//            LOG_PRINT("***** %d, Auth=%d, %d\n", i, pUserInfo->fPrivilege, g_xEngineParam.fVerifyMode);
//            //관리자인증방식에서 손님은 인식을 진행하지 않는다.
//            if(pUserInfo->fPrivilege != g_xEngineParam.fVerifyMode && g_xEngineParam.fVerifyMode != 0)
//                continue;
//            if(pUserInfo->xEndTime.i != 0)
//            {
//                //사용기간밖에 있는 사용자는 인증에 참가하지 않는다.
//                if(!(pUserInfo->xStartTime.i <= xNow.i && xNow.i <= pUserInfo->xEndTime.i))
//                    continue;
//            }
//        }

        if(pUserInfo->fPrivilege == 1)//admin
        {
            g_nIndexList_Admin[nAdminPersonCount] = i;
            g_nDNNFeatureCount_Admin[nAdminPersonCount] = pFeatInfo->nDNNFeatCount;
            g_prDNNEnrolledFeature_Admin[nAdminPersonCount] = (unsigned short*)(pFeatInfo->arFeatBuffer);
            nAdminPersonCount ++;
        }
        else
        {
            g_nIndexList[nPersonCount] = i;
            g_nDNNFeatureCount[nPersonCount] = pFeatInfo->nDNNFeatCount;
            g_prDNNEnrolledFeature[nPersonCount] = (unsigned short*)(pFeatInfo->arFeatBuffer);
            nPersonCount ++;
        }
    }

    IF_FLAG_STOP1(ES_FAILED);

    g_xEngineResult.nFineUserIndex = -1;

    float rDNNScore;
    int nDNNMaxScoreIndex;//[0]: indoor_indoor, [1]:indoor_outdoor, [2]:outdoor_outdoor

    //added by KSB 20180718
    *getDNNFeatureExtracted() = 0;
#ifdef TimeProfiling
    // float rTempStartTime = Now();
    // my_printf("[%d] pre extarctDNNFeature_process111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
    setTimeProfilingInfo(13);
#endif
    extarctDNNFeature_process();
    unsigned short* arLastDNNFeature = getLastDNNFeature();
    // {
    //     int nIndex;
    //     my_printf("Face Feature\n");
    //     for(nIndex = 0; nIndex < 256; nIndex ++)
    //     {
    //         my_printf("%f\n", arLastDNNFeature[nIndex]);
    //     }
    // }
    IF_FLAG_STOP1(ES_FAILED);

#ifdef TimeProfiling
    // my_printf("[%d] extarctDNNFeature_process Time = %f\r\n", (int)Now(), Now() - rTempStartTime);
    // my_printf("[%d] after extarctDNNFeature_process111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
    setTimeProfilingInfo(14);
#endif

    if (*getDNNFeatureExtracted())
    {
        int nEntireDNNMaxScoreIndex = -1;

        if(nAdminPersonCount)
        {
            GetMaxDNNScore(g_prDNNEnrolledFeature_Admin, g_nDNNFeatureCount_Admin, nAdminPersonCount, arLastDNNFeature, &rDNNScore, &nDNNMaxScoreIndex);
            APP_LOG("[%d] pec 32 %f\n", (int)Now(), rDNNScore);

            if (rDNNScore > DNN_VERIFY_THRESHOLD)
            {
                nEntireDNNMaxScoreIndex = g_nIndexList_Admin[nDNNMaxScoreIndex];
            }
        }
        IF_FLAG_STOP1(ES_FAILED);

        if(nEntireDNNMaxScoreIndex == -1)
        {
            GetMaxDNNScore(g_prDNNEnrolledFeature, g_nDNNFeatureCount, nPersonCount, arLastDNNFeature, &rDNNScore, &nDNNMaxScoreIndex);
            APP_LOG("[%d] pec 33 %f\n", (int)Now(), rDNNScore);
            if (rDNNScore > DNN_VERIFY_THRESHOLD)
            {
                nEntireDNNMaxScoreIndex = g_nIndexList[nDNNMaxScoreIndex];
            }
        }
        IF_FLAG_STOP1(ES_FAILED);

#ifdef TimeProfiling
    my_printf("[%d] after Matching DNNFeature 111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

        if (nEntireDNNMaxScoreIndex != -1)
        {

#ifdef ENGINE_FOR_DESSMAN
#ifdef TimeProfiling
            float rTempStartTime = Now();
#endif
            int nEst = checkEyeOpenness();
            if(nEst)
                g_xEngineResult.nEyeOpenness = 1;
            else
                g_nWaitingOpenEye = 1;
            APP_LOG("[%d] pec 34 %d\n", (int)Now(), nEst);

            IF_FLAG_STOP1(ES_FAILED);

#ifdef TimeProfiling
            my_printf("[%d] checkEyeOpenness Time = %f\r\n", (int)Now(), Now() - rTempStartTime);
            my_printf("[%d] after checkEyeOpenness111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif
#endif

            g_xEngineResult.nFineUserIndex = nEntireDNNMaxScoreIndex;
            if ((rDNNScore > DNN_UPDATE_THRESHOLD && rDNNScore < DNN_UPDATE_THRESHOLD_MAX) || (g_xEngineParam.iDemoMode == N_DEMO_FACTORY_MODE))
            {
                pose_estimate();
                if(isGoodFaceToEnroll())
                {
                    fr_UpdateFeat_DNN(nEntireDNNMaxScoreIndex, arLastDNNFeature,  g_nCurInfo);
                }
//#ifdef TimeProfiling
//    my_printf("[%d] after update DNNFeature 111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
//#endif

                return ES_UPDATE;
            }

#ifdef TimeProfiling
            setTimeProfilingInfo(15);
#endif            
            
            return ES_SUCCESS;
        }
    }
    //LOGE("fr_RetrievalH DNN failed");
    g_xEngineResult.nFineUserIndex = -2;

#ifdef TimeProfiling
    setTimeProfilingInfo(15);
#endif

    return ES_PROCESS;
}

int fr_calc_Off_dnn(unsigned char *pbLedOffImage)
{
    if (!g_nCurEnvForCameraControlSettenFromOffImage)
    {
        APP_LOG("pes 15 f\n");
        analyseBufferAndUpgradeCurEnvForCameraControl(pbLedOffImage, g_xEngineParam.nDetectionHeight, g_xEngineParam.nDetectionWidth, *fr_GetExposurePrev(), 1);
        g_nCurEnvForCameraControlSettenFromOffImage = 1;
    }
    return 0;
}

void    fr_LoadFeatureForDuplicateCheck_dnn(int nUpdateID)
{
    int nPersonCount = dbm_GetPersonCount();
    g_nEnrollePersonCount_Dup = 0;

    for (int i = 0; i < nPersonCount; i++)
    {
        PSMetaInfo pxMetaInfo = dbm_GetPersonMetaInfoByIndex(i);
        PSFeatInfo pxFeatInfo = dbm_GetPersonFeatInfoByIndex(i);
        if(pxMetaInfo == NULL || pxFeatInfo == NULL)
            continue;

        if(pxMetaInfo->iID == nUpdateID)
            continue;

        g_nIndexList_Dup[g_nEnrollePersonCount_Dup] = i;
        g_nDNNFeatureCount_Dup[g_nEnrollePersonCount_Dup] = pxFeatInfo->nDNNFeatCount;
        g_prDNNEnrolledFeature_Dup[g_nEnrollePersonCount_Dup] = (unsigned short*)pxFeatInfo->arFeatBuffer;

        g_nEnrollePersonCount_Dup ++;
    }

}

int     fr_GetRegisteredFeatInfo_dnn(void* pFeatInfo)
{
    PSFeatInfo pxFeatInfo = (PSFeatInfo)pFeatInfo;
    if (g_xEnrollFeatA->nDNNFeatCount > 0)
    {
        //LOG_PRINT("dfa = %d\n", g_xEnrollFeatA->nDNNFeatCount);
        memcpy(pxFeatInfo->arFeatBuffer, g_xEnrollFeatA->arFeatBuffer, g_xEnrollFeatA->nDNNFeatCount * KDNN_FEAT_SIZE * sizeof(unsigned short));
        memcpy(pxFeatInfo->ab_Info, g_xEnrollFeatA->ab_Info, g_xEnrollFeatA->nDNNFeatCount * sizeof(unsigned char));
        pxFeatInfo->nDNNFeatCount = g_xEnrollFeatA->nDNNFeatCount;
    }
    return 0;
}
