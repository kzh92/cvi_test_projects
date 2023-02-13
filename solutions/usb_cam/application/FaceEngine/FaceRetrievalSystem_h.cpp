#include "FaceRetrievalSystem_h.h"
#include "FaceRetrievalSystem_base.h"
#include "dic_manage.h"
//#include "sn.h"

#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
extern int g_exposureImage;
extern int g_exposurePrev;
static unsigned char g_nCurInfo = 0;
static int      g_nCurEnv = 0;
extern int g_nEnrollProcessCount;

extern int g_nEnrollePersonCount_Dup;
extern int* g_nIndexList;
extern int* g_nIndexList_Admin;
extern int* g_nIndexList_Dup;
extern int* g_nDNNFeatureCount_Dup;
extern int* g_nDNNFeatureCount;
extern int* g_nDNNFeatureCount_Admin;
extern float** g_prDNNEnrolledFeature_Dup;
extern float** g_prDNNEnrolledFeature;
extern float** g_prDNNEnrolledFeature_Admin;

#define MIN_LEDOFF_AVE_DIFF_VALID			10
#define MAX_LEDOFF_AVE_DIFF_VALID			60
#define MAX_ON_OFF_RATE_VALID				0.9f

#define H_FEATURE_COUNT_PER_ENV	4

FaceDetetResult g_xFDResult;
HModelResult    g_xFMResult;

mythread_ptr g_FeatEngineLoadThrad;

void get_640_360Image(unsigned char* pSrc, unsigned char* pDes, SRect rect);
void getFace320_240RectFromDNNFace(int* pn320_240Rect, int* pSrc_720P);
int update_H_EnrollFeature_sizeChanged_withEnv(_u8* pEnrollFeatureToUpdate, unsigned char* pbEnv, int* pnEnrolledFeatureCount, _u8* pVerifyFeatureToUpdate, unsigned char nVerifyInfo, int nForceUpdate = 0);
int updateEnrollFeatureByEnrollFeature_(_u8* pEnrollFeatureToUpdate, int* pnEnrolledFeatureCount, _u8* pEnrolledFeatureByUpdate);

void*   EngineLoadAndCheckFunc_h(void*)
{
    g_nThreadCount = 1;

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
    createModelingEngine(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Modeling);
    g_thread_flag_model = 2;

    g_thread_flag_H_1 = 1;
    g_thread_flag_H_2 = 1;
    loadMachineDic(MachineFlagIndex_H_1);
    loadMachineDic(MachineFlagIndex_H_2);
    LoadHEngine1(g_dic_H_1, g_dic_H_2);
    g_thread_flag_H_1 = 2;

#ifdef ENGINE_FOR_DESSMAN
    g_thread_flag_occ = 1;
    loadMachineDic(MachineFlagIndex_DNN_OCC);
    init_occ_detection(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_OCC);
    g_thread_flag_occ = 2;
#endif

    g_thread_flag_spoofa1 = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_A1);
    KdnnCreateLivenessEngine_2DA1(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_A1);
    g_thread_flag_spoofa1 = 2;

    g_thread_flag_spoofa2 = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_A2);
    KdnnCreateLivenessEngine_2DA2(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_A2);
    g_thread_flag_spoofa2 = 2;

    g_thread_flag_spoofb = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_B);
    KdnnCreateLivenessEngine_2DB(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B);
    g_thread_flag_spoofb = 2;

    g_thread_flag_spoofb2 = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_B2);
    KdnnCreateLivenessEngine_2DB2(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_B2);
    g_thread_flag_spoofb2 = 2;

    g_thread_flag_spoofc = 1;
    loadMachineDic(MachineFlagIndex_DNN_Liveness_C);
    KdnnCreateLivenessEngine_3D(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_Liveness_C);
    g_thread_flag_spoofc = 2;

    getDicChecSumChecked(MachineFlagIndex_H_2);
    g_thread_flag_H_2 = 2;

    g_nThreadCount = 2;

#ifdef ENGINE_FOR_DESSMAN
    g_thread_flag_esn = 1;
    loadMachineDic(MachineFlagIndex_DNN_ESN);
    init_esn_detection(g_shared_mem);
    getDicChecSumChecked(MachineFlagIndex_DNN_ESN);
    g_thread_flag_esn = 2;
#endif
    return NULL;
}

void*   FeatEngineLoadAndCheckFunc(void*)
{
    g_thread_flag_feat = 1;
    loadMachineDic(MachineFlagIndex_DNN_Feature);
    KdnnCreateEngine_feat(g_shared_mem);
    g_thread_flag_feat = 2;
    return NULL;
}



void    fr_InitEngine_h()
{
#ifdef __RTK_OS__
    my_thread_create_ext(&g_EngineInitThrad, 0, EngineLoadAndCheckFunc_h, NULL, (char*)"EngineInitThread_h", 16 * 1024, MYTHREAD_PRIORITY_MEDIUM);
#else
    if(g_EngineInitThrad == 0)
    {
        pthread_create (&g_EngineInitThrad, 0, EngineLoadAndCheckFunc_h, NULL);
    }
#endif
}

void    fr_LoadFeatEngine()
{
#ifndef __RTK_OS__
    if(g_FeatEngineLoadThrad == 0)
    {
        pthread_create (&g_FeatEngineLoadThrad, 0, FeatEngineLoadAndCheckFunc, NULL);
    }
#endif
}



int diffImageIsValid(int nLedOnAve, int nLedOffAve)
{
#ifdef TIME_PROFILING
    return 1;
#endif // TIME_PROFILING

    if (nLedOffAve < MIN_LEDOFF_AVE_DIFF_VALID)
    {
        return 0;
    }

    if (nLedOffAve > MAX_LEDOFF_AVE_DIFF_VALID)
    {
        float rRate = (float)nLedOffAve / nLedOnAve;
        if (rRate > MAX_ON_OFF_RATE_VALID)
        {
            return 0;
        }
    }

    return 1;
}

int     fr_ExtractFace_h(unsigned char *pbBayerFromCamera2)
{
    int nFirstImageFaceAvailable = 1;
    if (g_rAverageLedOnImage < MIN_DNN_LUM || (g_rSaturatedRate > SAT_THRESHOLD))
    {
        (getFaceProcessData())->nFaceDetected = 0;
    }

    if(!(getFaceProcessData())->nFaceDetected)
    {
        nFirstImageFaceAvailable = 0;
    }

    if(nFirstImageFaceAvailable)
    {
        int nFaceCheck = checkFaceInCamera2Image_expand_shrink_DNN(pbBayerFromCamera2, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, (getFaceProcessData())->nFaceRect, 1);
        if(!nFaceCheck)
        {
            (getFaceProcessData())->nFaceDetected = 0;
            APP_LOG("[%d] pec 8\n", (int)Now());
        }
        APP_LOG("[%d] pec 8-1 %d\n", (int)Now(), nFaceCheck);

    }

    IF_FLAG_STOP1(ES_FAILED);

    g_rExposureSettenTime = Now();

    float rStart = Now();
    float rPassTime;
#ifdef ENGINE_IS_DUALCAMERA
    float rPassTimeThreshold = 30;
#else
    float rPassTimeThreshold = 70;
#endif

    if(nTempCode == -1)
    {
        nTempCode = 0;
        rPassTimeThreshold = 0;
    }

    if (!(getFaceProcessData())->nFaceDetected)
    {
       if (g_nNeedToCalcNextExposure)
       {
           rPassTime = Now() - rStart;
           if (rPassTime < rPassTimeThreshold)
           {
               my_usleep((rPassTimeThreshold - rPassTime) * 1000);
           }
       }
       APP_LOG("[%d] pec 3 %d %d\n", (int)Now(), g_xEngineResult.nFaceNearFar, g_xEngineResult.nFacePosition);

    //        my_printf("Face far = %d\r\n", g_xEngineResult.nFaceNearFar);
    //        my_printf("Face position = %d\r\n", g_xEngineResult.nFacePosition);
       return ES_FAILED;
    }

#ifdef TimeProfiling
    my_printf("[%d] before modeling Time111 = %f\r\n", (int)Now(),Now() - g_rStartTime);
    float rStartTime1 = Now();
#endif


    IF_FLAG_STOP1(ES_FAILED);

    {
        waitDicChecksum(&g_thread_flag_model);
        if (g_thread_flag_model != 2)
        {
            return ES_FAILED;
        }

        int nRet = getFaceModelPoint(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, g_pbFaceDetectionBuffer, (getFaceProcessData())->nFaceRect, (getFaceProcessData())->rLandmarkPoint);
#ifdef TimeProfiling
        my_printf("[%d] getFaceModelPointTime = %f\r\n", (int)Now(), Now() - rStartTime1);
        my_printf("[%d] getFaceModelPointTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
        //rStartTime1 = Now();
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

        if (diffImageIsValid(g_rAverageLedOnImage, g_rAverageLedOffImage))
        {
            unsigned char* g_pbLEDOff_640_360 = g_shared_mem;
            unsigned char* g_pbLEDOn_640_360 = g_shared_mem + 0x38400;
            g_rAverageDiffImage = g_rAverageLedOnImage - g_rAverageLedOffImage;
            int nMotionX, nMotionY;
            int aLeft, aTop, aWidth, aHeight;
            aLeft = (getFaceProcessData())->nFaceRect[0];
            aTop = (getFaceProcessData())->nFaceRect[1];
            aWidth = (getFaceProcessData())->nFaceRect[2];
            aHeight = (getFaceProcessData())->nFaceRect[3];

            SRect rectAvg;
            rectAvg.x = aLeft - MOTION_OFFSET * 2;
            rectAvg.y = aTop - MOTION_OFFSET * 2;
            rectAvg.width = aWidth + MOTION_OFFSET * 4;
            rectAvg.height = aHeight + MOTION_OFFSET * 4;
            get_640_360Image(g_pbDiffIrImage, g_pbLEDOff_640_360, rectAvg);
            get_640_360Image(g_pbYIrImage, g_pbLEDOn_640_360, rectAvg);

            GetFaceMotion_Fast(g_pbLEDOff_640_360, g_pbLEDOn_640_360, (g_xEngineParam.nDetectionHeight >> 1),
                            (g_xEngineParam.nDetectionWidth >> 1), (aLeft >> 1), (aTop >> 1), (aHeight >> 1), (aWidth >> 1), &nMotionX,
                &nMotionY);

            IF_FLAG_STOP1(ES_FAILED);

            nMotionX = nMotionX << 1;
            nMotionY = nMotionY << 1;
            float rValueUpScale = 1;
            if(g_rAverageLedOnImage < 10)
            {
                g_rAverageLedOnImage = 10;
            }
            if(g_rAverageDiffImage < MIN_USER_LUM)
            {
                rValueUpScale =  (float)(MIN_USER_LUM + g_rAverageLedOffImage) / g_rAverageLedOnImage;
            }
            //rStartTime = Now();
            float rAlpha = rValueUpScale;
            float rBeta = -1.0f;

            if (g_rAverageDiffImage < MIN_USER_LUM)
            {
                int nMidValue;
                float rMidValueRate;
                rMidValueRate = (((float)MIN_USER_LUM + g_rAverageLedOffImage) / g_rAverageLedOnImage - 1) * 0.7f;
                nMidValue = (int)((float)g_rAverageLedOffImage * rMidValueRate);

                rAlpha = ((float)MIN_USER_LUM - nMidValue) / g_rAverageDiffImage;
                rBeta = (float)nMidValue / g_rAverageLedOffImage - rAlpha;
            }

            int nCalcDiffRects[4];
            nCalcDiffRects[0] = aLeft - aWidth * 0.2f;
            nCalcDiffRects[2] = aLeft + aWidth * 1.2f;
            nCalcDiffRects[1] = aTop - aHeight * 0.2f;
            nCalcDiffRects[3] = aTop + aHeight * 1.2f;

            refineRect(nCalcDiffRects, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight);

            unsigned char* g_pbTempOffImage = g_shared_mem;
            memcpy(g_pbTempOffImage, g_pbDiffIrImage, N_D_ENGINE_SIZE);
            CalcDiffImage(g_pbDiffIrImage, g_pbYIrImage, g_pbTempOffImage, nMotionX, nMotionY, nCalcDiffRects[0], nCalcDiffRects[1], nCalcDiffRects[2], nCalcDiffRects[3], rAlpha, rBeta);
            IF_FLAG_STOP1(ES_FAILED);
        }
        else
        {
            memcpy(g_pbDiffIrImage, g_pbYIrImage, g_xEngineParam.nDetectionWidth * g_xEngineParam.nDetectionHeight);
        }

        //convert detection result to HDetection
        int nFaceRect_2[4];
        getFace320_240RectFromDNNFace(nFaceRect_2, (getFaceProcessData())->nFaceRect);
        generateHDetectionFromRect(&g_xFDResult, nFaceRect_2);

        HImage img;
        APP_LOG("[%d] pdc 1\n", (int)Now());
        img.width = g_xEngineParam.nDetectionWidth;
        img.height = g_xEngineParam.nDetectionHeight;
        img.data = g_pbDiffIrImage;

        waitDicChecksum(&g_thread_flag_H_1);
        if (g_thread_flag_H_1 != 2 || !getDicChecSumChecked(MachineFlagIndex_H_1))
        {
            return ES_FAILED;
        }

        FaceDetect(&img, &g_xFDResult, 2);
        APP_LOG("[%d] pdc 1-1\n", (int)Now());

        IF_FLAG_STOP1(ES_FAILED);
        unsigned char bRet;
        int nModelingSuccess = FaceModeling(img, &g_xFDResult, &g_xFMResult, &bRet);
        APP_LOG("[%d] pdc 1-2\n", (int)Now());
        IF_FLAG_STOP1(ES_FAILED);

        if (nModelingSuccess)
        {
            APP_LOG("[%d] pec 6\n", (int)Now());

            float rLeftEyeX, rLeftEyeY, rRightEyeX, rRightEyeY;
            float rSigmaX, rSigmaY;
            int nIndex;

            rSigmaX = 0;
            rSigmaY = 0;
            for(nIndex = 36; nIndex <= 41; nIndex ++)
            {
               rSigmaX += (getFaceProcessData())->rLandmarkPoint[nIndex * 2];
               rSigmaY += (getFaceProcessData())->rLandmarkPoint[nIndex * 2 + 1];
            }

            rLeftEyeX = rSigmaX / 6;
            rLeftEyeY = rSigmaY / 6;

            rSigmaX = 0;
            rSigmaY = 0;
            for(nIndex = 42; nIndex <= 47; nIndex ++)
            {
               rSigmaX += (getFaceProcessData())->rLandmarkPoint[nIndex * 2];
               rSigmaY += (getFaceProcessData())->rLandmarkPoint[nIndex * 2 + 1];
            }
            rRightEyeX = rSigmaX / 6;
            rRightEyeY = rSigmaY / 6;

            g_xFMResult.eyeLX = rLeftEyeX;
            g_xFMResult.eyeLY = rLeftEyeY;
            g_xFMResult.eyeRX = rRightEyeX;
            g_xFMResult.eyeRY = rRightEyeY;
            g_xFMResult.modelSuccessFlag__ = 1;
        }

        g_xEngineResult.fValid = 1;

        g_xEngineResult.xFaceRect.x = (getFaceProcessData())->nFaceRect[0];
        g_xEngineResult.xFaceRect.y = (getFaceProcessData())->nFaceRect[1];
        g_xEngineResult.xFaceRect.width = (getFaceProcessData())->nFaceRect[2];
        g_xEngineResult.xFaceRect.height = (getFaceProcessData())->nFaceRect[3];

        //LOG_PRINT("========face rect = %d, %d, %d, %d\n", g_xEngineResult.xFaceRect.x, g_xEngineResult.xFaceRect.y, g_xEngineResult.xFaceRect.width, g_xEngineResult.xFaceRect.height);
#ifdef TimeProfiling
        my_printf("[%d] fr_ExtractFace111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

#if 0
        ExtractFaceImage();
#endif
    }

    if (g_nNeedToCalcNextExposure)
    {
        rPassTime = Now() - rStart;
        if (rPassTime < rPassTimeThreshold)
        {
            my_usleep((rPassTimeThreshold - rPassTime) * 1000);
        }
    }
    APP_LOG("[%d] pec 00\n", (int)Now());

    return ES_SUCCESS;
}


int fr_calc_Off_h(unsigned char *pbLedOffImage)
{
    if (!g_nCurEnvForCameraControlSettenFromOffImage)
    {
        analyseBufferAndUpgradeCurEnvForCameraControl(pbLedOffImage, g_xEngineParam.nDetectionHeight, g_xEngineParam.nDetectionWidth, g_exposurePrev, 1);
        g_nCurEnvForCameraControlSettenFromOffImage = 1;
    }
    IF_FLAG_STOP1(0);

    g_nCurEnv = 0;
    if(*fr_GetFaceDetected() == 1)
    {
        int aLeft, aTop, aWidth, aHeight;
        aLeft = (getFaceProcessData())->nFaceRect[0];
        aTop = (getFaceProcessData())->nFaceRect[1];
        aWidth = (getFaceProcessData())->nFaceRect[2];
        aHeight = (getFaceProcessData())->nFaceRect[3];

        {
            int nX, nY;
            int nTotalLedOffValue = 0;

            int nProcessArea = 0;

            for (nY = aTop; nY < aTop + aHeight; nY += 2)
            {
                for (nX = aLeft; nX < aLeft + aWidth; nX += 2)
                {
                    int nY1, nX1;

                    if(g_xEngineParam.iCamFlip == 0)
                    {
                        nY1 = nX;
                        nX1 = g_xEngineParam.nDetectionHeight - nY;
                    }
                    else
                    {
                        nY1 = g_xEngineParam.nDetectionWidth - nX;
                        nX1 = nY;
                    }

                    nTotalLedOffValue += pbLedOffImage[nY1 * g_xEngineParam.nDetectionHeight + nX1];
                    nProcessArea ++;
                }
            }

            if(nProcessArea)
            {
                g_rAverageLedOffImage = nTotalLedOffValue / nProcessArea;
            }
        }
        float rEnvValue =  (float)g_rAverageLedOffImage * 100 / (*fr_GetExposure());

        IF_FLAG_STOP1(0);

        if(diffImageIsValid(g_rAverageLedOnImage, g_rAverageLedOffImage))
        {
            convert_bayer2y_rotate((void*)pbLedOffImage, (void*)g_pbDiffIrImage, 1280, 720, g_xEngineParam.iCamFlip);
            IF_FLAG_STOP1(0);
        }

        if(rEnvValue  > 12)
            g_nCurEnv = 1;
    }

    g_nCurInfo = g_nCurEnv;
    return 0;
}


void get_640_360Image(unsigned char* pSrc, unsigned char* pDes, SRect rect)
{
    memset(pDes, 0, 0x38400);

    int nX, nY;

    _u8 *pSrcInit, *pLedOffInit ;
    _u8 *pDesInit;
    int nOffset = rect.y * g_xEngineParam.nDetectionWidth + rect.x;
    int E_IMAGE_WIDTH_2 = (g_xEngineParam.nDetectionWidth << 1);
    pLedOffInit = pSrc + nOffset;
    pDesInit = pDes + ((rect.y >> 1) * (g_xEngineParam.nDetectionWidth >> 1)) + (rect.x >> 1);

    for (nY = rect.y; nY < rect.y + rect.height; nY += 2)
    {
        _u8* pLEDOffBuf = pLedOffInit;
        _u8* pLEDOff320_240Buf = pDesInit;
        for (nX = rect.x; nX < rect.x + rect.width; nX += 2)
        {
            int bLedOn, bLedOff;
            bLedOff = *pLEDOffBuf;

            *pLEDOff320_240Buf = bLedOff;

            pLEDOffBuf += 2;
            pLEDOff320_240Buf ++;
        }
        pLedOffInit += E_IMAGE_WIDTH_2;
        pDesInit += (g_xEngineParam.nDetectionWidth >> 1);
    }
}

//pn320_240Rect: [0]:x, [1]:y, [2]:width, [3]:height
void getFace320_240RectFromDNNFace(int* pn320_240Rect, int* pSrc_720P)
{
    float rArmCenterX, rArmCenterY, rArmWidth;
    float rCalcedCenterX, rCalcedCenterY, rCalcedWidth;
    float rA, rC, rK;
    {

        float rArmCenterX, rArmCenterY, rArmWidth;
        float rCalcedCenterX, rCalcedCenterY, rCalcedWidth, rCalcedHeight;
        float rA, rC, rK1, rK2;

        rA = 0.016731f;
        rC = 0.015456f;
        rK1 = 0.955173f;
        rK2 = 0.955173f;

        rArmWidth = pSrc_720P[2];
        rArmCenterX = pSrc_720P[0] + (pSrc_720P[2]) / 2;
        rArmCenterY = pSrc_720P[1] + (pSrc_720P[3]) / 2;

        rCalcedCenterX = rArmCenterX + rA * rArmWidth;
        rCalcedCenterY = rArmCenterY + rC * rArmWidth;
        rCalcedWidth = rK1 * rArmWidth;
        rCalcedHeight = rK2 * rArmWidth;

        pn320_240Rect[0] = (int)((rCalcedCenterX - rCalcedWidth * 0.5f));
        pn320_240Rect[1] = (int)((rCalcedCenterY - rCalcedHeight * 0.5f));
        pn320_240Rect[2] = (int)((rCalcedCenterX + rCalcedWidth * 0.5f));
        pn320_240Rect[3] = (int)((rCalcedCenterY + rCalcedHeight * 0.5f));

    }

    if (pn320_240Rect[0] < 0)
    {
        pn320_240Rect[0] = 0;
    }

    if (pn320_240Rect[1] < 0)
    {
        pn320_240Rect[1] = 0;
    }

    if (pn320_240Rect[2] >= g_xEngineParam.nDetectionWidth)
    {
        pn320_240Rect[2] = g_xEngineParam.nDetectionWidth - 1;
    }

    if (pn320_240Rect[3] >= g_xEngineParam.nDetectionHeight)
    {
        pn320_240Rect[3] = g_xEngineParam.nDetectionHeight - 1;
    }

    pn320_240Rect[2] -= pn320_240Rect[0];
    pn320_240Rect[3] -= pn320_240Rect[1];

    int nIndex;
    for (nIndex = 0; nIndex < 4; nIndex++)
    {
        pn320_240Rect[nIndex] = pn320_240Rect[nIndex] / 2;
    }

    return;
}


int fr_UpdateFeat_H(int nUserIndex, unsigned char* pVerifyFeature)
{
#ifdef DB_BAK3
    //skip feature update in case of using backup partition.
    if (is_backup_partition())
        return ES_FAILED;
#endif //DB_BAK3

    if (nUserIndex < 0 || pVerifyFeature == NULL)
        return ES_FAILED;

    PSFeatInfo pxFeatInfo = dbm_GetPersonFeatInfoByIndex(nUserIndex);
    if (pxFeatInfo == NULL)
        return ES_FAILED;

    //updateEnrollFeature(pxFeatInfo->abFeatArray, pVerifyFeature);
    int iRet = update_H_EnrollFeature_sizeChanged_withEnv(pxFeatInfo->arFeatBuffer, pxFeatInfo->ab_Info, &pxFeatInfo->nFeatCount, pVerifyFeature, g_nCurInfo, 0);
    if(iRet != -1)
        return dbm_UpdatePersonFeatInfo(nUserIndex, pxFeatInfo, NULL);

    return ES_SUCCESS;
}


extern _s32 sub_E02FBEF4(_u8* pFea1, _u8* pFea2);
int update_H_EnrollFeature_sizeChanged_withEnv(_u8* pEnrollFeatureToUpdate, unsigned char* pbInfo, int* pnEnrolledFeatureCount, _u8* pVerifyFeatureToUpdate, unsigned char nVerifyInfo, int nForceUpdate)
{
    int nDarkFaceCount = 0;//count of face that average value is lower than nLowThreshold(40)
    int nLowThreshold = 40;
    int nFeatureCountPerEnv[ENV_COUNT] = { 0 };
    _u8 nVar_1B1C[0x1B04];

    memcpy(nVar_1B1C, pVerifyFeatureToUpdate, 0x1B00);//BL              memcpy_E041E628
    nVar_1B1C[0x1B00] = pVerifyFeatureToUpdate[0x5100];
    nVar_1B1C[0x1B01] = pVerifyFeatureToUpdate[0x5101];
    nVar_1B1C[0x1B02] = pVerifyFeatureToUpdate[0x5102];
    nVar_1B1C[0x1B03] = pVerifyFeatureToUpdate[0x5103];

    int nVerifyEnv = getEnvFromInfo(nVerifyInfo);

    int nIndexToUpdate = -1;
    if (*pnEnrolledFeatureCount < TOTAL_ENROLL_MAX_FEATURE_COUNT)
    {
        nIndexToUpdate = *pnEnrolledFeatureCount;
        memcpy(pEnrollFeatureToUpdate + nIndexToUpdate * UNIT_ENROLL_FEATURE_SIZE, nVar_1B1C, 0x1B04);
        pbInfo[nIndexToUpdate] = nVerifyInfo;
        *pnEnrolledFeatureCount = *pnEnrolledFeatureCount + 1;
        return nIndexToUpdate;
    }
    else
    {

        int nMaxEnrolledFeatureCount = TOTAL_ENROLL_MAX_FEATURE_COUNT;

        _s32 n1B00, n1B01;
        n1B00 = (((_s32)nVar_1B1C[0x1B00]) << 24) >> 24;
        //n1B01 = nVar_1B1C[0x1B01];

        int nEnrollFeatureIndex;
        for (nEnrollFeatureIndex = 0; nEnrollFeatureIndex < *pnEnrolledFeatureCount; nEnrollFeatureIndex++)
        {
            _s32 nCur1B02;
            nCur1B02 = *(pEnrollFeatureToUpdate + nEnrollFeatureIndex * UNIT_ENROLL_FEATURE_SIZE + 0x1B02);
            if (nCur1B02 < nLowThreshold)
            {
                nDarkFaceCount++;
            }

            int nCurEnv = getEnvFromInfo(pbInfo[nEnrollFeatureIndex]);
            nFeatureCountPerEnv[nCurEnv] ++;
        }

        //find Max Env Index
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

        //0:replace in dark face
        //1: replace in nVerifyEnv
        //2: replace in Max face
        int nReplacingMode = 0;

        if (nFeatureCountPerEnv[nVerifyEnv] > H_FEATURE_COUNT_PER_ENV)
        {
            nReplacingMode = 1;
            //replacing in nVerifyEnv mode
            if (nVerifyEnv == 0 && nVar_1B1C[0x1B02] < nLowThreshold && nDarkFaceCount > 4)
            {
                nReplacingMode = 0;
            }
        }
        else
        {
            //replacing in Max Count Env mode
            nReplacingMode = 2;
        }

        _s32 nMinnDelta0x1B00 = 0xFF;
        for (nEnrollFeatureIndex = 0; nEnrollFeatureIndex < *pnEnrolledFeatureCount; nEnrollFeatureIndex++)
        {
            _s32 nCur1B00, nCur1B01, nCur1B02;
            nCur1B00 = *(pEnrollFeatureToUpdate + nEnrollFeatureIndex * UNIT_ENROLL_FEATURE_SIZE + 0x1B00);
            nCur1B00 = (nCur1B00 << 24) >> 24;
            //nCur1B01 = *(pEnrollFeatureToUpdate + nEnrollFeatureIndex * UNIT_ENROLL_FEATURE_SIZE + 0x1B01);
            nCur1B02 = *(pEnrollFeatureToUpdate + nEnrollFeatureIndex * UNIT_ENROLL_FEATURE_SIZE + 0x1B02);

            _s32 nDelta0x1B00;
            nDelta0x1B00 = nCur1B00 - n1B00;
            nDelta0x1B00 = (nDelta0x1B00 ^ (nDelta0x1B00 >> 31)) - (nDelta0x1B00 >> 31);

            int nEnrolledEnv = getEnvFromInfo(pbInfo[nEnrollFeatureIndex]);

            if (nReplacingMode == 0)//replace in dark face
            {
                if (nDelta0x1B00 < nMinnDelta0x1B00 && nEnrolledEnv == 0 && nCur1B02 < nLowThreshold)
                {
                    nIndexToUpdate = nEnrollFeatureIndex;
                    nMinnDelta0x1B00 = nDelta0x1B00;
                }
            }
            else if (nReplacingMode == 1)// replace in nVerifyEnv
            {
                if (nDelta0x1B00 < nMinnDelta0x1B00 && nEnrolledEnv == nVerifyEnv)
                {
                    nIndexToUpdate = nEnrollFeatureIndex;
                    nMinnDelta0x1B00 = nDelta0x1B00;
                }
            }
            else//2: replace in Max face
            {
                if (nDelta0x1B00 < nMinnDelta0x1B00 && nEnrolledEnv == nMaxFeatureEnv)
                {
                    nIndexToUpdate = nEnrollFeatureIndex;
                    nMinnDelta0x1B00 = nDelta0x1B00;
                }
            }
        }

        int nRet;
        nRet = sub_E02FBEF4(pEnrollFeatureToUpdate + UNIT_ENROLL_FEATURE_SIZE * nIndexToUpdate, pVerifyFeatureToUpdate);
        if (nRet <= 0x230 && nForceUpdate == 0)
        {
            return -1;
        }
        else
        {
            memcpy(pEnrollFeatureToUpdate + nIndexToUpdate * UNIT_ENROLL_FEATURE_SIZE, nVar_1B1C, 0x1B04);
            pbInfo[nIndexToUpdate] = nVerifyInfo;
        }
    }

    return nIndexToUpdate;
}


int     fr_RegisterFace_h(int iFaceDir)
{
    IF_FLAG_STOP1(ES_PROCESS);

    if (g_xEngineResult.fValid == 0)
        return ES_PROCESS;

    APP_LOG("[%d] pdc 2 %d\n", (int)Now(), g_nPassedDirectionCount);

    if(g_nEnrollDirectionMode == ENROLL_ONLY_FRONT_DIRECTION_MODE)
    {
        iFaceDir = ENROLL_DIRECTION_FRONT_CODE_0;
    }

    if(g_nPassedDirectionCount == g_nEnrollDirectionMax)
    {
        APP_LOG("[%d] pec 9\n", (int)Now());
        return ES_FAILED;
    }

    if(g_rSaturatedRate > SAT_THRESHOLD)
    {
        APP_LOG("[%d] pec 10\n", (int)Now());
        return ES_FAILED;
    }


#if (FAKE_DETECTION == 1 && ENROLL_FAKE == 1)
    if (g_nEnrolledFrameCountUnderMin && g_rAverageLedOnImage < ENROLL_LIVENESS_MIN_AVERAGE)
    {
        APP_LOG("[%d] pec 11\n", (int)Now());
        return ES_FAILED;
    }
#endif

    if (!isCorrectPose(iFaceDir) && g_xEngineParam.iDemoMode != N_DEMO_FACTORY_MODE)
    {
        return ES_DIRECTION_ERROR;
    }

    IF_FLAG_STOP1(ES_PROCESS);

    bool fCurRealState = 0;
    bool fComboRealState = 0;

#if (FAKE_DETECTION == 1 && ENROLL_FAKE == 1)
    //float rO = Now();

    int nLiveness = check2D_3DFake();
    IF_FLAG_STOP1(ES_PROCESS);

    if (nLiveness == ES_SUCCESS)
    {
        fCurRealState = 1;
    }
    else
    {
        APP_LOG("[%d] pec 16\n", (int)Now());
    }

    fComboRealState = fCurRealState && g_fRealState;
    g_fRealState = fCurRealState;

    APP_LOG("[%d] pec 17 %d %d\n", (int)Now(), fCurRealState, g_fRealState);

    int nNeedToSkip = 0;

    if(!fComboRealState)
    {
        g_nFakeFrameCountInOneDirection ++;
        APP_LOG("[%d] pec 18 %d\n", (int)Now(), g_nFakeFrameCountInOneDirection);

        if(g_nFakeFrameCountInOneDirection > MAX_FakeFrameCountInOneDirection && g_nPassedDirectionCount != 0)
        {
            nNeedToSkip = 1;
        }
        else
        {
            return ES_PROCESS;
        }
    }
#endif


    IF_FLAG_STOP1(ES_PROCESS);

//check other person
    *getDNNFeatureExtracted() = 0;
    extarctDNNFeature_process();
    float* arLastDNNFeature = getLastDNNFeature();
    IF_FLAG_STOP1(ES_PROCESS);

    if(!(*getDNNFeatureExtracted()))
    {
        return ES_PROCESS;
    }

    if(g_nPassedDirectionCount == 0)
    {
        float* prDNNFeat = (float*)(g_xEnrollFeatA->arFeatBuffer + TOTAL_ENROLL_MAX_FEATURE_COUNT * UNIT_ENROLL_FEATURE_SIZE);
        memcpy(prDNNFeat + g_xEnrollFeatA->nDNNFeatCount * KDNN_FEAT_SIZE, arLastDNNFeature, sizeof(float)* KDNN_FEAT_SIZE);
        g_xEnrollFeatA->nDNNFeatCount = g_xEnrollFeatA->nDNNFeatCount + 1;
    }
    else
    {
        int pnDNNFeatureCount[2];
        float* prDNNEnrolledFeature[2];

        float* prDNNFeat = (float*)(g_xEnrollFeatA->arFeatBuffer + TOTAL_ENROLL_MAX_FEATURE_COUNT * UNIT_ENROLL_FEATURE_SIZE);
        pnDNNFeatureCount[0] = g_xEnrollFeatA->nDNNFeatCount;
        prDNNEnrolledFeature[0] = prDNNFeat;

        float rDNNScore;
        int nDNNMaxScoreIndex;
        GetMaxDNNScore(prDNNEnrolledFeature, pnDNNFeatureCount, 1, arLastDNNFeature, &rDNNScore, &nDNNMaxScoreIndex);
        APP_LOG("[%d] pec 20 %f\n", (int)Now(), rDNNScore);

        if(rDNNScore < DNN_ENROLL_CHECK_THRESHOLD)
        {
            APP_LOG("[%d] pec 21\n", (int)Now());
            return ES_PROCESS;
        }
        if(g_xEnrollFeatA->nDNNFeatCount < 4)
        {
            float* prDNNFeat = (float*)(g_xEnrollFeatA->arFeatBuffer + TOTAL_ENROLL_MAX_FEATURE_COUNT * UNIT_ENROLL_FEATURE_SIZE);
            memcpy(prDNNFeat + g_xEnrollFeatA->nDNNFeatCount * KDNN_FEAT_SIZE, arLastDNNFeature, sizeof(float)* KDNN_FEAT_SIZE);
            g_xEnrollFeatA->nDNNFeatCount = g_xEnrollFeatA->nDNNFeatCount + 1;
        }
    }
    IF_FLAG_STOP1(ES_PROCESS);

    if (!nNeedToSkip && isGoodFaceToEnroll())
    {
        waitDicChecksum(&g_thread_flag_H_2);
        if(g_thread_flag_H_2 != 2 || !getDicChecSumChecked(MachineFlagIndex_H_2))
        {
            return ES_PROCESS;
        }

        HImage img;
        img.width = g_xEngineParam.nDetectionWidth;
        img.height = g_xEngineParam.nDetectionHeight;
        img.data = g_pbDiffIrImage;
        unsigned char* g_pbEnrollUnitFeature = g_shared_mem;
        enroll_feature_extract(img, &g_xFMResult, g_pbEnrollUnitFeature);

        IF_FLAG_STOP1(ES_PROCESS);

        memcpy((unsigned char*)(g_xEnrollFeatA->arFeatBuffer) + g_nEnrollProcessCount * UNIT_ENROLL_FEATURE_SIZE, g_pbEnrollUnitFeature, UNIT_ENROLL_FEATURE_SIZE);

        APP_LOG("[%d] pec 22\n", (int)Now());
#if (FAKE_DETECTION == 1 && ENROLL_FAKE == 1)
        if (g_rAverageLedOnImage < ENROLL_LIVENESS_MIN_AVERAGE)
        {
            g_nEnrolledFrameCountUnderMin ++;
        }
#endif
        g_nEnrollProcessCount ++;
    }

    g_nPassedDirectionCount ++;


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

    IF_FLAG_STOP1(ES_PROCESS);

    if (g_nPassedDirectionCount == g_nEnrollDirectionMax)
    {
        g_xEnrollFeatA->nFeatCount = g_nEnrollProcessCount;

        //added by KSB 20160614
        //env value check
        int nFeatureIndex;
        for (nFeatureIndex = 0; nFeatureIndex < g_xEnrollFeatA->nFeatCount; nFeatureIndex++)
        {
            g_xEnrollFeatA->ab_Info[nFeatureIndex] = g_nCurInfo;
        }
        APP_LOG("[%d] pec 23\n", (int)Now());

        if (nDuplicated && (g_nEnrollDirectionMax == 1))
        {
            return ES_DUPLICATED;
        }

        return ES_SUCCESS;
    }

    g_nFakeFrameCountInOneDirection = 0;
    APP_LOG("[%d] pec 24\n", (int)Now());

    if (nDuplicated)
    {
        APP_LOG("[%d] pec 25\n", (int)Now());
        return ES_DUPLICATED;
    }

    return ES_ENEXT;
}

void    fr_LoadFeatureForDuplicateCheck_h(int nUpdateID)
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
        float* prDNNFeat = (float*)(pxFeatInfo->arFeatBuffer + TOTAL_ENROLL_MAX_FEATURE_COUNT * UNIT_ENROLL_FEATURE_SIZE);
        g_prDNNEnrolledFeature_Dup[g_nEnrollePersonCount_Dup] = prDNNFeat;

        g_nEnrollePersonCount_Dup ++;
    }
}


int fr_Retrieval_h()
{
    IF_FLAG_STOP1(ES_PROCESS);
    waitDicChecksum(&g_thread_flag_H_2);
    if(g_thread_flag_H_2 != 2 || !getDicChecSumChecked(MachineFlagIndex_H_2))
    {
        return ES_PROCESS;
    }

    unsigned char *g_abLast_H_Feat = g_shared_mem;
    HImage img;
    img.width = g_xEngineParam.nDetectionWidth;
    img.height = g_xEngineParam.nDetectionHeight;
    img.data = g_pbDiffIrImage;
    verify_feature_extract(img, &g_xFMResult, g_abLast_H_Feat);

    IF_FLAG_STOP1(ES_PROCESS);

    int nFineUserIndex = -1;
    int nGlobalFineUserIndex = -1;

    PSMetaInfo pUserInfo;
    PSFeatInfo pFeatInfo;

    //float rStartTime = Now();
    int nMaxScore = 0;

    int nUserCount = dbm_GetPersonCount();
    int nPersonCount = 0;
    int nAdminPersonCount = 0;

    DATETIME_32 xNow = dbm_GetCurDateTime();

    unsigned char**  g_pEnrolledFeature = (unsigned char**)g_prDNNEnrolledFeature;
    unsigned char**  g_pEnrolledFeature_Admin = (unsigned char**)g_prDNNEnrolledFeature_Admin;
    int*             g_nFeatureCount = g_nDNNFeatureCount;
    int*             g_nFeatureCount_Admin = g_nDNNFeatureCount_Admin;

    float rTime = Now();
    for (int i = 0; i < nUserCount; i++)
    {
        pUserInfo = dbm_GetPersonMetaInfoByIndex(i);
        pFeatInfo = dbm_GetPersonFeatInfoByIndex(i);

        if (pUserInfo->fPrivilege == 1)//admin
        {
            g_pEnrolledFeature_Admin[nAdminPersonCount] = pFeatInfo->arFeatBuffer;
            g_nFeatureCount_Admin[nAdminPersonCount] = pFeatInfo->nFeatCount;
            g_nIndexList_Admin[nAdminPersonCount] = i;

            nAdminPersonCount++;
        }
        else
        {
            g_pEnrolledFeature[nPersonCount] = pFeatInfo->arFeatBuffer;
            g_nFeatureCount[nPersonCount] = pFeatInfo->nFeatCount;
            g_nIndexList[nPersonCount] = i;

            nPersonCount++;
        }


        LOG_PRINT("us %d : fh = %d\n", i, pFeatInfo->nFeatCount);
    }

    IF_FLAG_STOP1(ES_PROCESS);

    if (nAdminPersonCount)
    {
        nMaxScore = calcSimilaryScore_changable_Size(g_pEnrolledFeature_Admin, g_nFeatureCount_Admin, nAdminPersonCount, g_abLast_H_Feat, &nFineUserIndex);
        if (nMaxScore > THRESHOLD1)
        {
            nGlobalFineUserIndex = g_nIndexList_Admin[nFineUserIndex];
        }
        APP_LOG("[%d] pec 32-1 %d\n", (int)Now(), nMaxScore);

    }

    IF_FLAG_STOP1(ES_PROCESS);

    if (nGlobalFineUserIndex == -1)
    {
        nMaxScore = calcSimilaryScore_changable_Size(g_pEnrolledFeature, g_nFeatureCount, nPersonCount, g_abLast_H_Feat, &nFineUserIndex);
        if (nMaxScore > THRESHOLD1)
        {
            nGlobalFineUserIndex = g_nIndexList[nFineUserIndex];
        }
        APP_LOG("[%d] pec 32-2 %d\n", (int)Now(), nMaxScore);

    }
    LOG_PRINT("ms = %d, mt = %f\n", nMaxScore, Now() - rTime);
    IF_FLAG_STOP1(ES_PROCESS);

    g_xEngineResult.nMaxScore = nMaxScore;
    APP_LOG("[%d] pec 32-3 %d\n", (int)Now(), nMaxScore);

    if (nGlobalFineUserIndex != -1)
    {
        g_xEngineResult.nFineUserIndex = nGlobalFineUserIndex;

#ifdef ENGINE_FOR_DESSMAN
        if(getLoadedDicFlag(MachineFlagIndex_DNN_ESN))
        {
            int nEst = checkEyeOpenness();
            if(nEst)
                g_xEngineResult.nEyeOpenness = 1;
            else
                g_nWaitingOpenEye = 1;
            APP_LOG("[%d] pec 34 %d\n", (int)Now(), nEst);

        }
        else
        {
            g_xEngineResult.nEyeOpenness = 1;
        }
#endif

        IF_FLAG_STOP1(ES_PROCESS);

        if((nMaxScore > THRESHOLD2 && g_rSaturatedRate < SAT_THRESHOLD) || (g_xEngineParam.iDemoMode == N_DEMO_FACTORY_MODE))
        {
            rTime = Now();
            fr_UpdateFeat_H(nGlobalFineUserIndex, g_abLast_H_Feat);
            LOG_PRINT("uft = %f\n", Now() - rTime);

            return ES_UPDATE;

        }
        APP_LOG("[%d] pec 32\n", (int)Now());

        return ES_SUCCESS;
    }

    return ES_PROCESS;
}


int fr_GetRegisteredFeatInfo_h(void* pFeatInfo)
{
    PSFeatInfo pxFeatInfo = (PSFeatInfo)pFeatInfo;

    if(pxFeatInfo == NULL)
        return ES_FAILED;

    int nNewFeatureIndex;

    for(nNewFeatureIndex = 0; nNewFeatureIndex < g_xEnrollFeatA->nFeatCount; nNewFeatureIndex ++)
    {
        int nUpdateIndex = updateEnrollFeatureByEnrollFeature_(pxFeatInfo->arFeatBuffer, &pxFeatInfo->nFeatCount, g_xEnrollFeatA->arFeatBuffer + nNewFeatureIndex * UNIT_ENROLL_FEATURE_SIZE);
        if (nUpdateIndex != -1)
        {
            pxFeatInfo->ab_Info[nUpdateIndex] = g_xEnrollFeatA->ab_Info[nNewFeatureIndex];
        }
    }

    if (g_xEnrollFeatA->nDNNFeatCount > 0)
    {
        LOG_PRINT("dfa = %d\n", g_xEnrollFeatA->nDNNFeatCount);
        float* prDNNFeat = (float*)(g_xEnrollFeatA->arFeatBuffer + TOTAL_ENROLL_MAX_FEATURE_COUNT * UNIT_ENROLL_FEATURE_SIZE);
        float* prFeatInfoFeat = (float*)(pxFeatInfo->arFeatBuffer + TOTAL_ENROLL_MAX_FEATURE_COUNT * UNIT_ENROLL_FEATURE_SIZE);
        memcpy(prFeatInfoFeat , prDNNFeat, g_xEnrollFeatA->nDNNFeatCount * KDNN_FEAT_SIZE * sizeof(float));
        pxFeatInfo->nDNNFeatCount = g_xEnrollFeatA->nDNNFeatCount;
    }

    return ES_SUCCESS;
}

extern _s32 sub_E02FBEF4(_u8* pFea1, _u8* pFea2);
int updateEnrollFeatureByEnrollFeature_(_u8* pEnrollFeatureToUpdate, int* pnEnrolledFeatureCount, _u8* pEnrolledFeatureByUpdate)
{
    int nMaxEnrolledFeatureCount = TOTAL_ENROLL_MAX_FEATURE_COUNT;
    //int nMaxEnrolledFeatureCount = 20;

    _s32 n1B00, n1B01;
    n1B00 = (((_s32)pEnrolledFeatureByUpdate[0x1B00]) << 24) >> 24;
    //n1B01 = pEnrolledFeatureByUpdate[0x1B01];

    int nIndexToUpdate = -1;

    if(*pnEnrolledFeatureCount < nMaxEnrolledFeatureCount)
    {
        memcpy(pEnrollFeatureToUpdate + *pnEnrolledFeatureCount * UNIT_ENROLL_FEATURE_SIZE, pEnrolledFeatureByUpdate, 0x1B04);
        nIndexToUpdate = *pnEnrolledFeatureCount;
        *pnEnrolledFeatureCount = *pnEnrolledFeatureCount + 1;
        return nIndexToUpdate;
    }
    else
    {
        if(pEnrolledFeatureByUpdate[0x1B01] == 0)
        {
            nIndexToUpdate = 1;
        }
        else
        {
            nIndexToUpdate = 3;
        }
        int nEnrollFeatureIndex;

        _s32 nMinValue = 0xFF;
        for(nEnrollFeatureIndex = 0; nEnrollFeatureIndex < *pnEnrolledFeatureCount; nEnrollFeatureIndex ++)
        {
            _s32 nCur1B00, nCur1B01;
            nCur1B00 = *(pEnrollFeatureToUpdate + nEnrollFeatureIndex * UNIT_ENROLL_FEATURE_SIZE + 0x1B00);
            nCur1B00 = (nCur1B00 << 24) >> 24;

            //nCur1B01 = *(pEnrollFeatureToUpdate + nEnrollFeatureIndex * UNIT_ENROLL_FEATURE_SIZE + 0x1B01);

            _s32 nDelta0x1B00;
            nDelta0x1B00 = nCur1B00 - n1B00;
            nDelta0x1B00 = (nDelta0x1B00 ^ (nDelta0x1B00 >> 31)) - (nDelta0x1B00 >> 31);

            if(nDelta0x1B00 < nMinValue)
            {
                nIndexToUpdate = nEnrollFeatureIndex;
                nMinValue = nDelta0x1B00;
            }
        }
        if (nIndexToUpdate != -1)
        {
            memcpy(pEnrollFeatureToUpdate + nIndexToUpdate * UNIT_ENROLL_FEATURE_SIZE, pEnrolledFeatureByUpdate, 0x1B04);
        }
    }

    return nIndexToUpdate;
}

#endif//ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
