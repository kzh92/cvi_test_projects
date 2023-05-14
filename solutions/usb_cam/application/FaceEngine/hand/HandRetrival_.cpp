#include "HandRetrival_.h"
#include "dic_manage.h"
#include "UltraFace.hpp"
#include "modeling_interface.h"
#include "appdef.h"
#include "common_types.h"
//#include "modeling.h"
#include "modeling_interface.h"
#include <memory.h>
#include "FaceRetrievalSystem_base.h"
#include "manageIRCamera.h"
#include "KDNN_handValid.h"
//#include "hand_feat.h"
//#include "ennq_global.h"
#include "settings.h" //N_DEMO_FACTORY_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include <vfs.h>
#include <sys/fcntl.h>


#if (N_MAX_HAND_NUM)

SEngineResult g_xEngineResult_Hand;
Hand_Process_Data g_xHandProcessData;
extern SEngineParam g_xEngineParam;

mythread_ptr g_EngineInitThrad_Hand = 0;
unsigned char*   g_pbYIrImage_Hand = 0;
unsigned char*  g_shared_mem_Hand = 0;
int g_nNeedToCalcNextExposure_Hand = 0;

int g_thread_flag_detect_h = 0;
int g_thread_flag_model_h = 0;
int g_thread_flag_feat_h = 0;
int g_thread_flag_checkValid_h = 0;


static int      g_nHandDetected = 0;

int             g_rAverageLedOnImage_Hand;
int             g_nHistInLEDOnImage_Hand[256];//g_nHistInLEDOnImage[256];
float           g_rSaturatedRate_Hand;
int             g_nBrightUpThresholdLevel_Hand = 0;

SHandFeatInfo g_xEnrollFeat_Hand;

extern void APP_LOG(const char * format, ...);
extern void waitDicChecksum(int* pThread);

int fr_UpdateFeat_Hand(int nUserIndex, unsigned short* prDNNFeature);

int* g_nIndexList_Hand = 0;
int* g_nIndexList_Admin_Hand = 0;
int* g_nIndexList_Dup_Hand = 0;
int* g_nDNNFeatureCount_Dup_Hand = 0;
int* g_nDNNFeatureCount_Hand = 0;
int* g_nDNNFeatureCount_Admin_Hand = 0;
unsigned short** g_prDNNEnrolledFeature_Dup_Hand = 0;
unsigned short** g_prDNNEnrolledFeature_Hand = 0;
unsigned short** g_prDNNEnrolledFeature_Admin_Hand = 0;
int g_nEnrollePersonCount_Dup_Hand = 0;

int g_nEnrollProcessCount_Hand = 0;
int g_iDNNFeatureExtracted_Hand = 0;

unsigned char*  g_global_memory_hand = 0;
unsigned short* g_arLastDNNFeature_Hand;//float g_arLastDNNFeature[KDNN_FEAT_SIZE];

unsigned char*  g_pAlignBackupBuffer = 0;
float*          g_prBackupCheckScore = 0;

int g_nContinueRealCount_Hand = 0;
#define USER_CONTINUE_FRAME_HAND	1

#define HAND_ENROLL_FEAT_COUNT 3
#define DNN_ENROLL_CHECK_THRESHOLD_HAND  74.0f
#define DNN_VERIFY_THRESHOLD_HAND        81.0f
#define DNN_UPDATE_THRESHOLD_HAND        82.5f
#define DNN_UPDATE_THRESHOLD_MAX_HAND    92.0f

#define HAND_ALIGN_MIN_AVERAGE          75.0f


#define HAND_ENROLL_MAX_WAIT_FRAMECOUNT     8
#define HAND_ENROLL_MAX_BACKUP_FRAMECOUNT   3
int g_nBackupedFrameCount = 0;
int g_nRegiterTotalFrameCount = 0;
int g_nDuplicated = 0;

#define DIC_MEM_ALIGN_ (64)
int AllocEngineMemory_Hand()
{
    if(g_global_memory_hand)
    {
        return 0;
    }
    int g_global_add_size =
            128 * 128 * HAND_ENROLL_MAX_BACKUP_FRAMECOUNT
            + sizeof(float) * HAND_ENROLL_MAX_BACKUP_FRAMECOUNT
            + sizeof(int) * N_MAX_HAND_NUM  //int g_nIndexList_Hand[N_MAX_HAND_NUM];
            + sizeof(int) * N_MAX_HAND_NUM  //int g_nIndexList_Admin_Hand[N_MAX_HAND_NUM];
            + sizeof(int) * N_MAX_HAND_NUM  //int g_nIndexList_Dup_Hand[N_MAX_HAND_NUM];
            + sizeof(int) * N_MAX_HAND_NUM  //int g_nDNNFeatureCount_Dup_Hand[N_MAX_HAND_NUM];
            + sizeof(int) * N_MAX_HAND_NUM  //int g_nDNNFeatureCount_Hand[N_MAX_HAND_NUM];
            + sizeof(int) * N_MAX_HAND_NUM   //int g_nDNNFeatureCount_Admin_Hand[N_MAX_HAND_NUM];
            + sizeof(unsigned short*) * N_MAX_HAND_NUM    //float* g_prDNNEnrolledFeature_Dup_Hand[N_MAX_HAND_NUM];
            + sizeof(unsigned short*) * N_MAX_HAND_NUM    //float* g_prDNNEnrolledFeature_Hand[N_MAX_HAND_NUM];
            + sizeof(unsigned short*) * N_MAX_HAND_NUM   //float* g_prDNNEnrolledFeature_Admin_Hand[N_MAX_HAND_NUM];
            + sizeof(unsigned short) * KDNN_FEAT_SIZE;      //float g_arLastDNNFeature_Hand[KDNN_FEAT_SIZE];

    //int g_global_dic_size = ENNQ::get_blob_size(HandFeat_dnn_dic_size(),  DIC_MEM_ALIGN_);
    int g_global_size = DIC_MEM_ALIGN_ + g_global_add_size;
    g_global_memory_hand = (unsigned char*)my_malloc(g_global_size);
    unsigned char* addr = (unsigned char*)(((size_t)(g_global_memory_hand + (DIC_MEM_ALIGN_ - 1))) & (-DIC_MEM_ALIGN_));

    //g_dic_feature_hand = addr;           addr += ENNQ::get_blob_size(HandFeat_dnn_dic_size(),    DIC_MEM_ALIGN_);
    //printf("g_dic_feature_hand = 0x%x\n", (int)g_dic_feature_hand);

    g_pAlignBackupBuffer = addr;                     addr += 128 * 128 * HAND_ENROLL_MAX_BACKUP_FRAMECOUNT;
    g_prBackupCheckScore = (float*)addr;                     addr += sizeof(float) * HAND_ENROLL_MAX_BACKUP_FRAMECOUNT;
    g_nIndexList_Hand = (int*)addr;                  addr += sizeof(int) * N_MAX_HAND_NUM;//int g_nIndexList_Hand[N_MAX_HAND_NUM];
    g_nIndexList_Admin_Hand = (int*)addr;            addr += sizeof(int) * N_MAX_HAND_NUM;//int g_nIndexList_Admin_Hand[N_MAX_HAND_NUM];
    g_nIndexList_Dup_Hand = (int*)addr;              addr += sizeof(int) * N_MAX_HAND_NUM;//int g_nIndexList_Dup_Hand[N_MAX_HAND_NUM];
    g_nDNNFeatureCount_Dup_Hand = (int*)addr;        addr += sizeof(int) * N_MAX_HAND_NUM;//int g_nDNNFeatureCount_Dup_Hand[N_MAX_HAND_NUM];
    g_nDNNFeatureCount_Hand = (int*)addr;            addr += sizeof(int) * N_MAX_HAND_NUM;//int g_nDNNFeatureCount_Hand[N_MAX_HAND_NUM];
    g_nDNNFeatureCount_Admin_Hand = (int*)addr;      addr += sizeof(int) * N_MAX_HAND_NUM;//int g_nDNNFeatureCount_Admin_Hand[N_MAX_HAND_NUM];
    g_prDNNEnrolledFeature_Dup_Hand = (unsigned short**)addr; addr += sizeof(unsigned short*) * N_MAX_HAND_NUM;//float* g_prDNNEnrolledFeature_Dup_Hand[N_MAX_HAND_NUM];
    g_prDNNEnrolledFeature_Hand = (unsigned short**)addr;     addr += sizeof(unsigned short*) * N_MAX_HAND_NUM;//float* g_prDNNEnrolledFeature_Hand[N_MAX_HAND_NUM];
    g_prDNNEnrolledFeature_Admin_Hand = (unsigned short**)addr; addr += sizeof(unsigned short*) * N_MAX_HAND_NUM;//float* g_prDNNEnrolledFeature_Admin_Hand[N_MAX_HAND_NUM];
    g_arLastDNNFeature_Hand = (unsigned short*)addr;           addr += sizeof(unsigned short) * KDNN_FEAT_SIZE;//float g_arLastDNNFeature_Hand[KDNN_FEAT_SIZE];

    return 0;
}

int getMemSizeNeedToFeat_CheckValidHand()
{
    int nRet = 128 * 128 * (HAND_ENROLL_MAX_BACKUP_FRAMECOUNT);
    nRet += sizeof(float) * HAND_ENROLL_MAX_BACKUP_FRAMECOUNT;
//    int nRet = 128 * 128 * 1;
    return nRet;
}


void*   EngineLoadAndCheckFunc_Hand(void*)
{
    AllocEngineMemory_Hand();
    // g_thread_flag_detect_h = 1;
    // loadMachineDic(MachineFlagIndex_DNN_Detect_Hand);
    // createDetectEngine(g_shared_mem_Hand, Detect_Mode_Hand);
    // getDicChecSumChecked(MachineFlagIndex_DNN_Detect_Hand);
    // g_thread_flag_detect_h = 2;

    // g_thread_flag_model_h = 1;
    // loadMachineDic(MachineFlagIndex_DNN_Modeling_Hand);
    // createModelingEngine(g_shared_mem_Hand, 1);
    // getDicChecSumChecked(MachineFlagIndex_DNN_Modeling_Hand);
    // g_thread_flag_model_h = 2;


    // unsigned char* pHand_Feat_Mem = g_shared_mem_Hand + 128 * 128;
    // g_thread_flag_checkValid_h = 1;
    // loadMachineDic(MachineFlagIndex_DNN_CheckValid_Hand);
    // KdnnCreateCheckValid_Hand(pHand_Feat_Mem);
    // getDicChecSumChecked(MachineFlagIndex_DNN_CheckValid_Hand);
    // g_thread_flag_checkValid_h = 2;


    // g_thread_flag_feat_h = 1;
    // loadMachineDic(MachineFlagIndex_DNN_Feature_Hand);
    // KdnnCreateEngine_feat(pHand_Feat_Mem, 1);
    // g_thread_flag_feat_h = 2;


    return NULL;
}

int createHandEngine_()
{
    my_printf("createHandEngine_\n");
#ifdef __RTK_OS__
    my_thread_create_ext(&g_EngineInitThrad_Hand, 0, EngineLoadAndCheckFunc_Hand, NULL, (char*)"EngineInitThread", 16 * 1024, MYTHREAD_PRIORITY_MEDIUM);
#else
    if(g_EngineInitThrad_Hand == 0)
    {
        pthread_create(&g_EngineInitThrad_Hand, 0, EngineLoadAndCheckFunc_Hand, NULL);
    }
#endif

    return ES_SUCCESS;
}

struct node1
{
    float rScore;
    int nIndex;
};

int compare_float(const void * a, const void * b)
{
    return ((node1*)a)->rScore < ((node1*)b)->rScore;
}

int refineEnrolledFeat(int nRemainFeatCount)
{
    int nOrgFeatCount = g_xEnrollFeat_Hand.nDNNFeatCount;
    if(nOrgFeatCount < nRemainFeatCount || nOrgFeatCount <= 2)
    {
        return 0;
    }

    node1* pNodeScores = (node1*)g_shared_mem_Hand;
    int nIndex1, nIndex2;

    for(nIndex1 = 0; nIndex1 < nOrgFeatCount; nIndex1 ++)
    {
        pNodeScores[nIndex1].nIndex = nIndex1;
        pNodeScores[nIndex1].rScore = 0;
    }
    for(nIndex1 = 0; nIndex1 < nOrgFeatCount - 1; nIndex1 ++)
    {
        for(nIndex2 = nIndex1 + 1; nIndex2 < nOrgFeatCount; nIndex2 ++)
        {
            float rScore = KdnnGetSimilarity((unsigned short*)g_xEnrollFeat_Hand.arFeatBuffer + nIndex1 * KDNN_FEAT_SIZE,
                                             (unsigned short*)g_xEnrollFeat_Hand.arFeatBuffer + nIndex2 * KDNN_FEAT_SIZE);

            pNodeScores[nIndex1].rScore += rScore;
            pNodeScores[nIndex2].rScore += rScore;
        }
    }

    qsort(pNodeScores, nOrgFeatCount, sizeof(node1), compare_float);
    unsigned short* pTempFeat = (unsigned short*)(g_shared_mem_Hand + sizeof(node1) * nOrgFeatCount);
    for(nIndex1 = 0; nIndex1 < nRemainFeatCount; nIndex1 ++)
    {
        memcpy(pTempFeat + nIndex1 * KDNN_FEAT_SIZE, (unsigned short*)g_xEnrollFeat_Hand.arFeatBuffer + pNodeScores[nIndex1].nIndex * KDNN_FEAT_SIZE,
               sizeof(unsigned short) * KDNN_FEAT_SIZE);
    }

    memcpy(g_xEnrollFeat_Hand.arFeatBuffer, pTempFeat, sizeof(unsigned short) * KDNN_FEAT_SIZE * nRemainFeatCount);
    g_xEnrollFeat_Hand.nDNNFeatCount = nRemainFeatCount;
    return 0;
}



void    fr_FreeEngine_Hand()
{
    releaseDetectEngine(Detect_Mode_Hand);
    releaseModelingEngine(1);
    KdnnFreeEngine_feat(1);
    KdnnFreeCheckValid_Hand();

    if(g_global_memory_hand)
    {
        my_free(g_global_memory_hand);
        g_global_memory_hand = 0;
        APP_LOG("[%d] pecc 4-10-1-h\n", (int)Now());
    }
}


int setHandShareMem_(unsigned char* pbShareMem, unsigned char* pbShareYmem)
{
    //printf("Set Shared Mem\n");
    g_shared_mem_Hand = pbShareMem;
//    int nMaxOffset = 0;
//    int nOffset = getDetectMenSize() + g_DNN_Detection_hand_input_width * g_DNN_Detection_hand_input_height;
//    if(nMaxOffset < nOffset)
//    {
//        nMaxOffset = nOffset;
//    }
//    nOffset = Modeling_dnn_mem_size() + g_DNN_Modeling_input_width * g_DNN_Modeling_input_height;
//    if(nMaxOffset < nOffset)
//    {
//        nMaxOffset = nOffset;
//    }
//    if(nMaxOffset < N_D_ENGINE_SIZE)
//    {
//        nMaxOffset = N_D_ENGINE_SIZE;
//    }

    g_pbYIrImage_Hand = pbShareYmem;
    return ES_SUCCESS;
}

void calcAverageValues_Hand(unsigned char* pLEDOnImage, unsigned char* pLEDOffImage, SRect rect)
{
    int nX, nY;
    // int nTotalLedOffValue = 0;
    int nTotalLedOnValue = 0;

    int nProcessArea = 0;
    memset(g_nHistInLEDOnImage_Hand, 0, sizeof(int) * 256);

    _u8 *pLedOnInit/*, *pLedOffInit = 0*/;
    int nOffset = rect.y * g_xEngineParam.nDetectionWidth + rect.x;
    int E_IMAGE_WIDTH_2 = g_xEngineParam.nDetectionWidth << 1;
    pLedOnInit = pLEDOnImage + nOffset;
    if(pLEDOffImage)
    {
        // pLedOffInit = pLEDOffImage + nOffset;
    }

    for (nY = rect.y; nY < rect.y + rect.height; nY += 2)
    {
        _u8* pLEDOnBuf = pLedOnInit;
        // _u8* pLEDOffBuf = 0;
        if(pLEDOffImage)
        {
            // pLEDOffBuf = pLedOffInit;
        }

        for (nX = rect.x; nX < rect.x + rect.width; nX += 2)
        {
            int bLedOn/*, bLedOff*/;
            bLedOn = *pLEDOnBuf;
            nTotalLedOnValue += bLedOn;
            g_nHistInLEDOnImage_Hand[bLedOn] ++;
            pLEDOnBuf += 2;
            nProcessArea ++;
        }
        pLedOnInit += E_IMAGE_WIDTH_2;
    }

    if(nProcessArea)
    {
        g_rAverageLedOnImage_Hand = nTotalLedOnValue / nProcessArea;
        float rSigmaSaturation = 0;
        int nValue;
        for(nValue = 250; nValue <= 255; nValue++)
        {
            rSigmaSaturation += g_nHistInLEDOnImage_Hand[nValue];
        }

        g_rSaturatedRate_Hand = rSigmaSaturation *  100.0f / nProcessArea;

        int nAvaiableSatuartionPixels = nProcessArea * SAT_THRESHOLD / 100;
        int nLevel;
        int nThresLevel = 0;
        int nSigma = 0;
        for (nLevel = 255; nLevel >= 0; nLevel--)
        {
            nSigma += g_nHistInLEDOnImage_Hand[nLevel];

            if (nSigma > nAvaiableSatuartionPixels)
            {
                nThresLevel = nLevel;
                break;
            }
        }

        nThresLevel++;
        g_nBrightUpThresholdLevel_Hand = nThresLevel;
    }
}


extern int isFaceOutOfImage(int* pnRect, int nImageWidth, int nImageHeight);

//nBadCode:0-good, 1-too small, 2-too big, 3-out of image, 4-low score
int getBestIndex_Hand(FaceInfo* face_info, int n_face_cnt, int &nBadCode, int &nOutImageCode)
{
    int nFACE_MIN_WIDTH_AVAIABLE_ENROLL = HAND_MIN_WIDTH_AVAIABLE_ENROLL;
    int nFACE_MIN_WIDTH_AVAIABLE = HAND_MIN_WIDTH_AVAIABLE;
    int nFACE_MAX_WIDTH_AVAIABLE = HAND_MAX_WIDTH_AVAIABLE;

    int nBestFaceSizeInGoodFace = 0;
    int nBestFaceSizeInBadFace = 0;
    int nBestFaceIndexInGoodFace = -1;
    int nBestFaceIndexInBadFace = -1;
    int nBestFaceInBadFace_BadCode = 0;
    int nBestFaceInBadFace_OutImageCode = 0;

    int nIndex;
    for(nIndex = 0; nIndex < n_face_cnt; nIndex ++)
    {
        FaceInfo face = face_info[nIndex];

        //float rScore;
        int nFaceRects[4];
        nFaceRects[0] = (int)(face.x1);
        nFaceRects[1] = (int)(face.y1);
        nFaceRects[2] = (int)(face.x2);
        nFaceRects[3] = (int)(face.y2);

        int nFaceWidth = nFaceRects[2] - nFaceRects[0];
        int nFaceHeight = nFaceRects[3] - nFaceRects[1];
        int nFaceMaxLine = nFaceWidth > nFaceHeight ? nFaceWidth : nFaceHeight;

        int nIsBad = 0;
        int nCurBadCode = 0;
        int nCurOutImageBadCode = 0;
        if((g_xEngineParam.fEngineState == ENS_REGISTER && nFaceMaxLine < nFACE_MIN_WIDTH_AVAIABLE_ENROLL) || nFaceMaxLine < nFACE_MIN_WIDTH_AVAIABLE)
        {
            nIsBad = 1;
            nCurBadCode = 1;
        }
        else if(nFaceMaxLine > nFACE_MAX_WIDTH_AVAIABLE)
        {
            nIsBad = 1;
            nCurBadCode = 2;
        }
        else
        {
            int nRet = isFaceOutOfImage(nFaceRects, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight);
            if(nRet)
            {
                nIsBad = 1;
                nCurBadCode = 3;
                nCurOutImageBadCode = nRet;
            }
            else if(face.score < 0.9)
            {
                nIsBad = 1;
                nCurBadCode = 4;
            }
        }
        if(nIsBad)
        {
            if(nFaceMaxLine > nBestFaceSizeInBadFace)
            {
                nBestFaceIndexInBadFace = nIndex;
                nBestFaceSizeInBadFace = nFaceMaxLine;
                nBestFaceInBadFace_BadCode = nCurBadCode;
                nBestFaceInBadFace_OutImageCode = nCurOutImageBadCode;
            }
        }
        else
        {
            if(nFaceMaxLine > nBestFaceSizeInGoodFace)
            {
                nBestFaceSizeInGoodFace = nFaceMaxLine;
                nBestFaceIndexInGoodFace = nIndex;
            }
        }
    }

    if(nBestFaceIndexInGoodFace != -1)
    {
        nBadCode = 0;
        return nBestFaceIndexInGoodFace;
    }
    nBadCode = nBestFaceInBadFace_BadCode;
    nOutImageCode = nBestFaceInBadFace_OutImageCode;
    return nBestFaceIndexInBadFace;
}

//int nTempIndex = 0;
void extarctDNNFeature_process_Hand()
{
    waitDicChecksum(&g_thread_flag_feat_h);
    if(g_thread_flag_feat_h == 2 && getFeatEngineLoaded(1)  && getDicChecSumChecked(MachineFlagIndex_DNN_Feature_Hand))
    {
        if (g_nStopEngine == 1)
        {
            APP_LOG("[%d] stop by flag_stop\n", (int)Now());
            return;
        }

        unsigned char *pLiveAlignFeat;
        pLiveAlignFeat = g_shared_mem_Hand;
        //generateAlignImageForFeature(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, g_pbFaceDetectionBuffer, (getFaceProcessData())->rLandmarkPoint);

        int nRet = KdnnDetect_feat(pLiveAlignFeat, g_arLastDNNFeature_Hand, 1);
        
#if 0
    {
        char szImageFilePath[255];
        sprintf(szImageFilePath, "/mnt/sd/handFeatAlign_buf_%d.bin", nTempIndex);
        int fd1 = aos_open(szImageFilePath, O_CREAT | O_RDWR);
        if(fd1 >= 0)
        {

            aos_write(fd1, pLiveAlignFeat, 128 * 128);
            aos_sync(fd1);
            aos_close(fd1);
            APP_LOG("%s saved\n", szImageFilePath);
        }
        else
        {
            APP_LOG("%s not saved\n", szImageFilePath);
        }
        
        sprintf(szImageFilePath, "/mnt/sd/handFeat_%d.bin", nTempIndex);
        fd1 = aos_open(szImageFilePath, O_CREAT | O_RDWR);
        if(fd1 >= 0)
        {

            aos_write(fd1, g_arLastDNNFeature_Hand, sizeof(unsigned short) * KDNN_FEAT_SIZE);
            aos_sync(fd1);
            aos_close(fd1);
            APP_LOG("%s saved\n", szImageFilePath);
        }
        else
        {
            APP_LOG("%s not saved\n", szImageFilePath);
        }

        nTempIndex ++;
    }
#endif
        if(g_nStopEngine)
        {
            return;
        }
        if (nRet == KDNN_SUCCESS)
        {
            g_iDNNFeatureExtracted_Hand = 1;
        }
        else
        {
            memset(g_arLastDNNFeature_Hand, 0, sizeof(unsigned short) * KDNN_FEAT_SIZE);
        }
    }
}


void backup_process_for_feat(unsigned char* pAlignedBuf, float rScore)
{
    int nNeedUpdateIndex = -1;
    int nIndex;
    int nUnitBufferSize = 128 * 128;
    unsigned char* pBackup_Buffer = g_pAlignBackupBuffer;
    float* pBackup_Score = g_prBackupCheckScore;
    if(!g_nBackupedFrameCount)
    {
        nNeedUpdateIndex = 0;
    }
    else
    {
        for(nIndex = 0; nIndex < g_nBackupedFrameCount; nIndex ++)
        {
            if(rScore > pBackup_Score[nIndex])
            {
                nNeedUpdateIndex = nIndex;
                break;
            }
        }
    }

    if(nNeedUpdateIndex != -1)
    {
        //shift exist buffer and score
        for(nIndex = g_nBackupedFrameCount - 1; nIndex >= nNeedUpdateIndex; nIndex --)
        {
            if((nIndex + 1) < HAND_ENROLL_MAX_BACKUP_FRAMECOUNT)
            {
                pBackup_Score[nIndex + 1] = pBackup_Score[nIndex];
                memcpy(pBackup_Buffer + nUnitBufferSize * (nIndex + 1), pBackup_Buffer + nUnitBufferSize * nIndex, nUnitBufferSize);
            }
        }
        pBackup_Score[nNeedUpdateIndex] = rScore;
        memcpy(pBackup_Buffer + nUnitBufferSize * nNeedUpdateIndex, pAlignedBuf, nUnitBufferSize);
        g_nBackupedFrameCount ++;
        if(g_nBackupedFrameCount > HAND_ENROLL_MAX_BACKUP_FRAMECOUNT)
        {
            g_nBackupedFrameCount = HAND_ENROLL_MAX_BACKUP_FRAMECOUNT;
        }
    }

    return;
}



int checkValid_Hand(float *prScore = 0)
{
    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

#ifdef TimeProfiling
    my_printf("[%d] before KdnnCheckValid_HandTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    waitDicChecksum(&g_thread_flag_checkValid_h);

#ifdef TimeProfiling
    float rStartTime = Now();
    my_printf("[%d] after waitDicChecksum(checkValid_h)Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    if (g_thread_flag_checkValid_h != 2)
    {
        APP_LOG("[%d] pec 27-1-h\n", (int)Now());
        return ES_FAILED;
    }

    unsigned char *pAlign_CheckValidFeat;
    pAlign_CheckValidFeat = g_shared_mem_Hand;

    float rResult = 0;
    rResult = KdnnCheckValid_Hand(pAlign_CheckValidFeat);

#ifdef TimeProfiling
    my_printf("[%d] KdnnCheckValid_Hand Time = %f\r\n", (int)Now(), Now() - rStartTime);
    my_printf("[%d] KdnnCheckValid_Hand Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    if(prScore)
    {
        *prScore = rResult;
    }
    //temp code
    //rResult = 1;
    if(rResult < 0)
    {
        APP_LOG("[%d] pec 26-h\n", (int)Now());
        return ES_SPOOF_FACE;
    }

    return ES_SUCCESS;
}



int nTempIndex1 = 0;


int fr_PreExtractHand(unsigned char *pbLedOnImage)
{
    if (pbLedOnImage == NULL)
        return ES_FAILED;
    APP_LOG("[%d] process h start\n", (int)Now());

    if (g_xEngineParam.fEngineState == ENS_REGISTER && g_nEnrollProcessCount_Hand == HAND_ENROLL_FEAT_COUNT)
    {
        return ES_SUCCESS;
    }
    g_nNeedToCalcNextExposure_Hand = 0;
    memset(&g_xEngineResult_Hand, 0, sizeof(SEngineResult));
    memset(&g_xHandProcessData, 0, sizeof(Hand_Process_Data));

    if(*fr_GetBayerYConvertedCameraIndex() != 0)
    {
        convert_bayer2y_rotate_cm_riscv(pbLedOnImage, g_pbYIrImage_Hand, E_IMAGE_WIDTH, E_IMAGE_HEIGHT, g_xEngineParam.iCamFlip);
        *fr_GetBayerYConvertedCameraIndex() = 0;
        //printf("Camera 0 Bayer->Y converted\n");
    }
    g_nHandDetected = 0;

    waitDicChecksum(&g_thread_flag_detect_h);
    if (g_thread_flag_detect_h != 2)
    {
        return ES_FAILED;
    }

    unsigned char*   pbHandDetectionBuffer;
    pbHandDetectionBuffer = g_shared_mem_Hand + getDetectMenSize();
    FaceInfo *face_info = (FaceInfo *)pbHandDetectionBuffer;
    int n_face_cnt = 0;
    float rHandRects[4];
    int aLeft, aTop, aWidth, aHeight;

#ifdef TimeProfiling
    float rStartTime = Now();
#endif

    detect(g_pbYIrImage_Hand, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, face_info, MAX_FACE_NUM, &n_face_cnt, pbHandDetectionBuffer, 1);
#ifdef TimeProfiling
    my_printf("[%d] detect_HandTime = %f\r\n", (int)Now(), Now() - rStartTime);
    my_printf("[%d] detect_HandTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif
    /*
    {
        nTempIndex1 ++;
        char szPath[255];
        sprintf(szPath, "/home/%d.bin", nTempIndex1);
        FILE* pFile = fopen(szPath, "wb");
        if(pFile)
        {
            fwrite(g_pbYIrImage_Hand, 900 * 1600, 1, pFile);
            printf("%s saved\n", szPath);
            fclose(pFile);
        }
    }
    */
    IF_FLAG_STOP1(ES_FAILED);

    int nFaceNum = 0;
    nFaceNum = n_face_cnt;
    int nHandDetectSuccess = -1;
    if(nFaceNum)
    {
        IF_FLAG_STOP1(ES_FAILED);
        int nBadCode = 0;
        int nBestFaceIndex;
        int nOutImageCode = 0;

        nBestFaceIndex = getBestIndex_Hand(face_info, n_face_cnt, nBadCode, nOutImageCode);

        FaceInfo face = face_info[nBestFaceIndex];
        rHandRects[0] = face.x1;
        rHandRects[1] = face.y1;
        rHandRects[2] = face.x2;
        rHandRects[3] = face.y2;
        //my_printf("nFaceRects = %f %f %f %f\n", face.x1, face.y1, face.x2, face.y2);

        nHandDetectSuccess = 0;
        g_nHandDetected = 1;

        APP_LOG("[%d] pec 1-0 h %d %d %d %d\n", (int)Now(), (int)rHandRects[0], (int)rHandRects[1], (int)rHandRects[2], (int)rHandRects[3]);
        int nFaceWidth = (float)(rHandRects[2] - rHandRects[0]);
        int nFaceHeight = (float)(rHandRects[3] - rHandRects[1]);
        int nFaceMaxLine = nFaceWidth > nFaceHeight ? nFaceWidth : nFaceHeight;

        if(nBadCode)
        {
            nHandDetectSuccess = -1;
            if(nBadCode == 1)//face too small
            {
                APP_LOG("[%d] pec 2-1-h %d\n", (int)Now(), nFaceMaxLine);
                g_xEngineResult_Hand.nFaceNearFar = 2;
            }
            else if(nBadCode == 2)//face too big
            {
                APP_LOG("[%d] pec 2-2-h %d\n", (int)Now(), nFaceMaxLine);
                g_xEngineResult_Hand.nFaceNearFar = 1;
            }
            else if(nBadCode == 3)//face is out of image
            {
                APP_LOG("[%d] pec 2-3-h %d\n", (int)Now(), nOutImageCode);
                g_xEngineResult_Hand.nFacePosition = nOutImageCode;
            }
            else//low score
            {
                APP_LOG("[%d] pec 2-4-h %f\n", (int)Now(), face.score);
            }
        }
        IF_FLAG_STOP1(ES_FAILED);
    }
    else
    {
        APP_LOG("[%d] pec 1-h\n", (int)Now());
        nHandDetectSuccess = -1;
//            my_printf("Face is not detected\r\n");
    }

    if(g_nHandDetected)
    {
        // int nMIN_USER_LUM, nMAX_USER_LUM;
        // nMIN_USER_LUM = MIN_USER_LUM_HAND;
        // nMAX_USER_LUM = MAX_USER_LUM_HAND;

        refineRect(rHandRects, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight);
        aLeft = (int)rHandRects[0];
        aTop = (int)rHandRects[1];
        aWidth = (int)(rHandRects[2] - rHandRects[0]);
        aHeight = (int)(rHandRects[3] - rHandRects[1]);

        SRect rectAvg;
        rectAvg.x = aLeft + aWidth * 0.2f;
        rectAvg.y = aTop + aHeight * 0.2f;
        rectAvg.width = aWidth * 0.6f;
        rectAvg.height = aHeight * 0.6f;

        calcAverageValues_Hand(g_pbYIrImage_Hand, 0, rectAvg);

        APP_LOG("[%d] pes 1-h %d %f\n", (int)Now(), (int)g_rAverageLedOnImage_Hand, g_rSaturatedRate_Hand);

#ifdef TimeProfiling
            my_printf("[%d] calcAverageValues_HandTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif
    }
    else
    {
        //if hand detection is not done
        if(*fr_GetEntireImageAnalysed() == 0)
        {
            APP_LOG("pes 2-h o\n");
            int nExpValue = (int)((float)(*fr_GetExposure_bkup()) * getGainRateFromGain_SC2355(*fr_GetGain_bkup(), *fr_GetFineGain_bkup()));
            analyseBufferAndUpgradeCurEnvForCameraControl(g_pbYIrImage_Hand, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, nExpValue, 0);
            *fr_GetEntireImageAnalysed() = 1;
        }
    }
    if(!nHandDetectSuccess)
    {
        if (g_rAverageLedOnImage_Hand > MIN_DNN_LUM_Hand && (g_rSaturatedRate_Hand < SAT_THRESHOLD))
        {
            g_xHandProcessData.rHandRect[0] = rHandRects[0];
            g_xHandProcessData.rHandRect[1] = rHandRects[1];
            g_xHandProcessData.rHandRect[2] = rHandRects[2] - rHandRects[0];
            g_xHandProcessData.rHandRect[3] = rHandRects[3] - rHandRects[1];
            g_xHandProcessData.nHandDetected = 1;
        }
    }

    CalcNextExposure_inner_hand();
    return ES_SUCCESS;
}

int    fr_GetNeedCamera1_HandDetect()
{
    if(!g_nHandDetected)
        return ES_FAILED;

    if (g_rAverageLedOnImage_Hand < MIN_DNN_LUM_Hand || (g_rSaturatedRate_Hand > SAT_THRESHOLD))
        return ES_FAILED;

    if(*fr_GetExposure_bkup() != *fr_GetExposure2_bkup() || *fr_GetGain_bkup() != *fr_GetGain2_bkup() || *fr_GetFineGain_bkup() != *fr_GetFineGain2_bkup())
        return ES_FAILED;

    if(g_xEngineResult_Hand.nFacePosition != 1)
        return ES_FAILED;

    return ES_SUCCESS;
}

int fr_PreExtractHand2(unsigned char *pbLedOnImage)
{
    if (pbLedOnImage == NULL)
        return ES_FAILED;
    APP_LOG("[%d] process h2 start\n", (int)Now());

    memset(&g_xEngineResult_Hand, 0, sizeof(SEngineResult));
    memset(&g_xHandProcessData, 0, sizeof(Hand_Process_Data));


    if(*fr_GetBayerYConvertedCameraIndex() != 1)
    {
        convert_bayer2y_rotate_cm(pbLedOnImage, g_pbYIrImage_Hand, E_IMAGE_WIDTH, E_IMAGE_HEIGHT, 1 - g_xEngineParam.iCamFlip);
        *fr_GetBayerYConvertedCameraIndex() = 1;
        //printf("Camera 1 Bayer->Y converted\n");
    }
    /*
    {
        char szPath[255];
        sprintf(szPath, "/home/%d_r.bin", nTempIndex1);
        FILE* pFile = fopen(szPath, "wb");
        if(pFile)
        {
            fwrite(g_pbYIrImage_Hand, 900 * 1600, 1, pFile);
            printf("%s saved\n", szPath);
            fclose(pFile);
        }
    }
    */

    int nTempHandDetected = 0;

    unsigned char*   pbHandDetectionBuffer;
    pbHandDetectionBuffer = g_shared_mem_Hand + getDetectMenSize();
    FaceInfo *face_info = (FaceInfo *)pbHandDetectionBuffer;
    int n_face_cnt = 0;
    float rHandRects[4];
    int aLeft, aTop, aWidth, aHeight;
    detect(g_pbYIrImage_Hand, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, face_info, MAX_FACE_NUM, &n_face_cnt, pbHandDetectionBuffer, 1);

    IF_FLAG_STOP1(ES_FAILED);

    int nFaceNum = 0;
    nFaceNum = n_face_cnt;
    int nHandDetectSuccess = -1;
    if(nFaceNum)
    {
        IF_FLAG_STOP1(ES_FAILED);
        int nBadCode = 0;
        int nBestFaceIndex;
        int nOutImageCode = 0;

        nBestFaceIndex = getBestIndex_Hand(face_info, n_face_cnt, nBadCode, nOutImageCode);

        FaceInfo face = face_info[nBestFaceIndex];
        rHandRects[0] = face.x1;
        rHandRects[1] = face.y1;
        rHandRects[2] = face.x2;
        rHandRects[3] = face.y2;
        //my_printf("nFaceRects = %f %f %f %f\n", face.x1, face.y1, face.x2, face.y2);

        nHandDetectSuccess = 0;
        nTempHandDetected = 1;

        APP_LOG("[%d] pec 1-0 1 %d %d %d %d\n", (int)Now(), (int)rHandRects[0], (int)rHandRects[1], (int)rHandRects[2], (int)rHandRects[3]);
        int nFaceWidth = (float)(rHandRects[2] - rHandRects[0]);
        int nFaceHeight = (float)(rHandRects[3] - rHandRects[1]);
        int nFaceMaxLine = nFaceWidth > nFaceHeight ? nFaceWidth : nFaceHeight;

        if(nBadCode)
        {
            nHandDetectSuccess = -1;
            if(nBadCode == 1)//face too small
            {
                APP_LOG("[%d] pec 2-1-h %d\n", (int)Now(), nFaceMaxLine);
                g_xEngineResult_Hand.nFaceNearFar = 2;
            }
            else if(nBadCode == 2)//face too big
            {
                APP_LOG("[%d] pec 2-2-h %d\n", (int)Now(), nFaceMaxLine);
                g_xEngineResult_Hand.nFaceNearFar = 1;
            }
            else if(nBadCode == 3)//face is out of image
            {
                APP_LOG("[%d] pec 2-3-h %d\n", (int)Now(), nOutImageCode);
                g_xEngineResult_Hand.nFacePosition = nOutImageCode;
            }
            else//low score
            {
                APP_LOG("[%d] pec 2-4-1 %f\n", (int)Now(), face.score);
            }
        }
        IF_FLAG_STOP1(ES_FAILED);
    }
    else
    {
        APP_LOG("[%d] pec 1-h\n", (int)Now());
        nHandDetectSuccess = -1;
//            my_printf("Face is not detected\r\n");
    }

    if(nTempHandDetected)
    {
        g_nHandDetected = 1;
        // int nMIN_USER_LUM, nMAX_USER_LUM;
        // nMIN_USER_LUM = MIN_USER_LUM_HAND;
        // nMAX_USER_LUM = MAX_USER_LUM_HAND;

        refineRect(rHandRects, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight);
        aLeft = (int)rHandRects[0];
        aTop = (int)rHandRects[1];
        aWidth = (int)(rHandRects[2] - rHandRects[0]);
        aHeight = (int)(rHandRects[3] - rHandRects[1]);

        SRect rectAvg;
        rectAvg.x = aLeft + aWidth * 0.2f;
        rectAvg.y = aTop + aHeight * 0.2f;
        rectAvg.width = aWidth * 0.6f;
        rectAvg.height = aHeight * 0.6f;

        calcAverageValues_Hand(g_pbYIrImage_Hand, 0, rectAvg);

        APP_LOG("[%d] pes 1 %d %f\n", (int)Now(), (int)g_rAverageLedOnImage_Hand, g_rSaturatedRate_Hand);

#ifdef TimeProfiling
            my_printf("[%d] calcAverageValues_rotateTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif
        APP_LOG("pes %d Satu %f\n", g_rAverageLedOnImage_Hand, g_rSaturatedRate_Hand);
    }

    if(!nHandDetectSuccess)
    {
        if (g_rAverageLedOnImage_Hand > MIN_DNN_LUM_Hand && (g_rSaturatedRate_Hand < SAT_THRESHOLD))
        {
            g_xHandProcessData.rHandRect[0] = rHandRects[0];
            g_xHandProcessData.rHandRect[1] = rHandRects[1];
            g_xHandProcessData.rHandRect[2] = rHandRects[2] - rHandRects[0];
            g_xHandProcessData.rHandRect[3] = rHandRects[3] - rHandRects[1];
            g_xHandProcessData.nHandDetected = 1;
        }
    }

    return ES_SUCCESS;
}

int		fr_ExtractHand()
{
    if (g_xEngineParam.fEngineState == ENS_REGISTER && g_nEnrollProcessCount_Hand == HAND_ENROLL_FEAT_COUNT)
    {
        return ES_SUCCESS;
    }


    if (!g_xHandProcessData.nHandDetected)
    {
       APP_LOG("[%d] pec 3-h %d %d\n", (int)Now(), g_xEngineResult_Hand.nFaceNearFar, g_xEngineResult_Hand.nFacePosition);

       return ES_FAILED;
    }

    waitDicChecksum(&g_thread_flag_model_h);
    if (g_thread_flag_model_h != 2)
    {
        return ES_FAILED;
    }
#ifdef TimeProfiling
    float rStartTime1 = Now();
#endif

    unsigned char* pAlignBufferForModeling = g_shared_mem_Hand;
    int nRet = getHandModelPoint(g_pbYIrImage_Hand, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, pAlignBufferForModeling, g_xHandProcessData.rHandRect, g_xHandProcessData.rLandmarkPoint);

#ifdef TimeProfiling
    my_printf("[%d] getFaceModelPointTime = %f\r\n", (int)Now(), Now() - rStartTime1);
    my_printf("[%d] getFaceModelPointTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    if(nRet)
    {
        return ES_FAILED;
    }
    IF_FLAG_STOP1(ES_FAILED);

    unsigned char *pHandAlign_Feat_CheckValid = g_shared_mem_Hand;
    float rAverageValue = 0;
    //feat & check valid
    handAlign(g_pbYIrImage_Hand, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, pHandAlign_Feat_CheckValid, g_xHandProcessData.rLandmarkPoint,
               128, 128, -24, -23, -1, -3, &rAverageValue);

    APP_LOG("[%d] pes 1-a %f\n", (int)Now(), rAverageValue);
    if(rAverageValue < HAND_ALIGN_MIN_AVERAGE && (g_rAverageLedOnImage_Hand < MIN_USER_LUM_HAND || g_rAverageLedOnImage_Hand > MAX_USER_LUM_HAND))
    {
        return ES_FAILED;
    }
    g_xHandProcessData.nHandModelExtracted =1;
    g_xEngineResult_Hand.fValid = 1;
    g_xEngineResult_Hand.xFaceRect.x = (float)(g_xHandProcessData.rHandRect[0]);
    g_xEngineResult_Hand.xFaceRect.y = (float)(g_xHandProcessData.rHandRect[1]);
    g_xEngineResult_Hand.xFaceRect.width = (float)(g_xHandProcessData.rHandRect[2]);
    g_xEngineResult_Hand.xFaceRect.height = (float)(g_xHandProcessData.rHandRect[3]);


//    {
//        nTempIndex1 ++;
//        char szPath[255];
//        sprintf(szPath, "/home/%d.bin", nTempIndex1);
//        FILE* pFile = fopen(szPath, "wb");
//        if(pFile)
//        {
//            fwrite(g_pbYIrImage_Hand, 900 * 1600, 1, pFile);
//            printf("%s saved\n", szPath);
//            fclose(pFile);
//        }
//        printf("Face rect %d %d %d %d\n", g_xEngineResult_Hand.xFaceRect.x, g_xEngineResult_Hand.xFaceRect.y, g_xEngineResult_Hand.xFaceRect.width, g_xEngineResult_Hand.xFaceRect.height);
//        int nIndex;
//        for(nIndex = 0; nIndex < 7; nIndex ++)
//        {
//            printf("%f %f\n", g_xHandProcessData.rLandmarkPoint[nIndex * 2], g_xHandProcessData.rLandmarkPoint[nIndex * 2 + 1]);
//        }

//        sprintf(szPath, "/home/%d_Align.bin", nTempIndex1);
//        pFile = fopen(szPath, "wb");
//        if(pFile)
//        {
//            fwrite(pHandAlign, 136 * 136, 1, pFile);
//            printf("%s saved\n", szPath);
//            fclose(pFile);
//        }
//    }

    //printf("fr_ExtractHand processed\n");
    return ES_SUCCESS;
}

void    fr_SetEngineState_Hand(int fState, int nUpdateID)
{
    g_nContinueRealCount_Hand = 0;
    if(fState == ENS_VERIFY)
    {

    }
    else
    {
        memset(&g_xEnrollFeat_Hand, 0, sizeof(SHandFeatInfo));
        fr_LoadFeatureForDuplicateCheck_Hand(nUpdateID);
        g_nEnrollProcessCount_Hand = 0;
        g_nBackupedFrameCount = 0;
        g_nRegiterTotalFrameCount = 0;
        g_nDuplicated = 0;

//        FILE *pFile = fopen("/home/feat_hand.bin", "wb");
//        if(pFile)
//        {
//            fwrite(g_dic_feature_hand, HandFeat_dnn_dic_size(), 1, pFile);
//            fclose(pFile);
//            sync();
//        }

    }
}



void    fr_LoadFeatureForDuplicateCheck_Hand(int nUpdateID)
{
    int nPersonCount = dbm_GetHandCount();
    g_nEnrollePersonCount_Dup_Hand = 0;

    for (int i = 0; i < nPersonCount; i++)
    {
        PSMetaInfo pxMetaInfo = dbm_GetHandMetaInfoByIndex(i);
        SHandFeatInfo* pxFeatInfo = dbm_GetHandFeatInfoByIndex(i);
        if(pxMetaInfo == NULL || pxFeatInfo == NULL)
            continue;

        if(pxMetaInfo->iID == nUpdateID)
            continue;

        g_nIndexList_Dup_Hand[g_nEnrollePersonCount_Dup_Hand] = i;
        g_nDNNFeatureCount_Dup_Hand[g_nEnrollePersonCount_Dup_Hand] = pxFeatInfo->nDNNFeatCount;
        g_prDNNEnrolledFeature_Dup_Hand[g_nEnrollePersonCount_Dup_Hand] = (unsigned short*)pxFeatInfo->arFeatBuffer;
        g_nEnrollePersonCount_Dup_Hand ++;
    }
}


int     fr_GetRegisteredFeatInfo_Hand(SHandFeatInfo* pFeatInfo)
{
    //printf("fr_GetRegisteredFeatInfo_Hand\n");
    SHandFeatInfo* pxFeatInfo = (SHandFeatInfo*)pFeatInfo;
    if (g_xEnrollFeat_Hand.nDNNFeatCount > 0)
    {
        //LOG_PRINT("dfa = %d\n", g_xEnrollFeatA->nDNNFeatCount);
        memcpy(pxFeatInfo->arFeatBuffer, g_xEnrollFeat_Hand.arFeatBuffer, g_xEnrollFeat_Hand.nDNNFeatCount * KDNN_FEAT_SIZE * sizeof(unsigned short));
        //memcpy(pxFeatInfo->ab_Info, g_xEnrollFeatA->ab_Info, g_xEnrollFeatA->nDNNFeatCount * sizeof(unsigned char));
        pxFeatInfo->nDNNFeatCount = g_xEnrollFeat_Hand.nDNNFeatCount;
    }
    return 0;
}

int     fr_VerifyHand_()
{
    float rStart = Now();
    float rPassTime;
    float rPassTimeThreshold = 70;

    if (g_xEngineResult_Hand.fValid == 0)
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

//    if(checkValid_Hand() != ES_SUCCESS)
//    {
//        APP_LOG("[%d] pec 18-h %d\n", (int)Now(), g_nContinueRealCount_Hand);
//        return ES_PROCESS;
//    }

    PSMetaInfo pUserInfo;
    SHandFeatInfo* pFeatInfo;

    int nUserCount = dbm_GetHandCount();
//    printf("fr_VerifyHand_ nUserCount %d\n", nUserCount);
    int nPersonCount = 0;
    int nAdminPersonCount = 0;

    for (int i = 0; i < nUserCount; i++)
    {
        pUserInfo = dbm_GetHandMetaInfoByIndex(i);
        pFeatInfo = dbm_GetHandFeatInfoByIndex(i);

        if(pUserInfo->fPrivilege == 1)//admin
        {
            g_nIndexList_Admin_Hand[nAdminPersonCount] = i;
            g_nDNNFeatureCount_Admin_Hand[nAdminPersonCount] = pFeatInfo->nDNNFeatCount;
            g_prDNNEnrolledFeature_Admin_Hand[nAdminPersonCount] = (unsigned short*)(pFeatInfo->arFeatBuffer);
            nAdminPersonCount ++;
        }
        else
        {
            g_nIndexList_Hand[nPersonCount] = i;
            g_nDNNFeatureCount_Hand[nPersonCount] = pFeatInfo->nDNNFeatCount;
            g_prDNNEnrolledFeature_Hand[nPersonCount] = (unsigned short*)(pFeatInfo->arFeatBuffer);
            nPersonCount ++;
        }
    }

    IF_FLAG_STOP1(ES_FAILED);

    g_xEngineResult_Hand.nFineUserIndex = -1;

    float rDNNScore;
    int nDNNMaxScoreIndex;//[0]: indoor_indoor, [1]:indoor_outdoor, [2]:outdoor_outdoor

#ifdef TimeProfiling
    float rStartTime = Now();
    my_printf("[%d] before extarctDNNFeature_process_Hand Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    //added by KSB 20180718
    g_iDNNFeatureExtracted_Hand = 0;
    extarctDNNFeature_process_Hand();
#ifdef TimeProfiling
    my_printf("[%d] extarctDNNFeature_process_Hand Time = %f\r\n", (int)Now(), Now() - rStartTime);
    my_printf("[%d] extarctDNNFeature_process_Hand Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    IF_FLAG_STOP1(ES_FAILED);

    if (g_iDNNFeatureExtracted_Hand)
    {
        int nEntireDNNMaxScoreIndex = -1;

        if(nAdminPersonCount)
        {
            GetMaxDNNScore(g_prDNNEnrolledFeature_Admin_Hand, g_nDNNFeatureCount_Admin_Hand, nAdminPersonCount, g_arLastDNNFeature_Hand, &rDNNScore, &nDNNMaxScoreIndex);
            APP_LOG("[%d] pec 32-h %f\n", (int)Now(), rDNNScore);

            if (rDNNScore > DNN_VERIFY_THRESHOLD)
            {
                nEntireDNNMaxScoreIndex = g_nIndexList_Admin_Hand[nDNNMaxScoreIndex];
            }
        }
        IF_FLAG_STOP1(ES_FAILED);

        if(nEntireDNNMaxScoreIndex == -1)
        {
            GetMaxDNNScore(g_prDNNEnrolledFeature_Hand, g_nDNNFeatureCount_Hand, nPersonCount, g_arLastDNNFeature_Hand, &rDNNScore, &nDNNMaxScoreIndex);
            APP_LOG("[%d] pec 33-h %f\n", (int)Now(), rDNNScore);
            if (rDNNScore > DNN_VERIFY_THRESHOLD)
            {
                nEntireDNNMaxScoreIndex = g_nIndexList_Hand[nDNNMaxScoreIndex];
            }
        }
        IF_FLAG_STOP1(ES_FAILED);

#ifdef TimeProfiling
    my_printf("[%d] after Matching DNNFeature 111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

        if (nEntireDNNMaxScoreIndex != -1)
        {
            g_xEngineResult_Hand.nFineUserIndex = nEntireDNNMaxScoreIndex;
//            printf("HandVerify Index = %d\n", g_xEngineResult_Hand.nFineUserIndex);

            if ((rDNNScore > DNN_UPDATE_THRESHOLD_HAND && rDNNScore < DNN_UPDATE_THRESHOLD_MAX_HAND) || (g_xEngineParam.iDemoMode == N_DEMO_FACTORY_MODE))
            {
                //pose_estimate();
                //if(isGoodFaceToEnroll())

                {
                    fr_UpdateFeat_Hand(nEntireDNNMaxScoreIndex, g_arLastDNNFeature_Hand);
                }

                return ES_UPDATE;
            }
            return ES_SUCCESS;
        }
    }
    //LOGE("fr_RetrievalH DNN failed");
    g_xEngineResult_Hand.nFineUserIndex = -2;

    return ES_PROCESS;
}

int     fr_RegisterHand()
{
    float rStart = Now();
    float rPassTime;
    float rPassTimeThreshold = 70;
    // my_printf("fr_RegisterHand g_nPassedDirectionCount = %d\n", g_nPassedDirectionCount);
    // my_printf("g_nEnrollProcessCount_Hand %d\n", g_nEnrollProcessCount_Hand);
    // my_printf("g_xEngineResult_Hand.fValid %d\n", g_xEngineResult_Hand.fValid);
    if (g_nEnrollProcessCount_Hand == HAND_ENROLL_FEAT_COUNT)
    {
        g_nPassedDirectionCount ++;
        if (g_nPassedDirectionCount >= g_nEnrollDirectionMax)
        {
            return ES_SUCCESS;
        }
        else
        {
            return ES_ENEXT;
        }
    }

    if (g_xEngineResult_Hand.fValid == 0)
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
    g_nRegiterTotalFrameCount ++;
    // int nValidCheck = 1;
    APP_LOG("[%d] pec 17-h %d\n", (int)Now(), g_nRegiterTotalFrameCount);
    float rCheckScore = 0;
    if(checkValid_Hand(&rCheckScore) != ES_SUCCESS)
    {
        unsigned char *pAlign_CheckValidFeat;
        pAlign_CheckValidFeat = g_shared_mem_Hand;
        backup_process_for_feat(pAlign_CheckValidFeat, rCheckScore);
        APP_LOG("[%d] pec 17-h-b\n", (int)Now());
    }
    else
    {
        // int nEnrollFeatureAdded = 0;
        g_iDNNFeatureExtracted_Hand = 0;
        extarctDNNFeature_process_Hand();
        IF_FLAG_STOP1(ES_FAILED);
        if(!g_iDNNFeatureExtracted_Hand)
        {
    //        my_printf("extract DNN Feature falied\r\n");
            APP_LOG("[%d] pec 18-1-h\n", (int)Now());
            return ES_PROCESS;
        }
        if(g_nEnrollProcessCount_Hand != HAND_ENROLL_FEAT_COUNT)
        {
            if(g_nEnrollProcessCount_Hand == 0)
            {
                memcpy((unsigned short*)(g_xEnrollFeat_Hand.arFeatBuffer) + g_xEnrollFeat_Hand.nDNNFeatCount * KDNN_FEAT_SIZE, g_arLastDNNFeature_Hand, sizeof(unsigned short)* KDNN_FEAT_SIZE);
                g_xEnrollFeat_Hand.nDNNFeatCount = g_xEnrollFeat_Hand.nDNNFeatCount + 1;
                APP_LOG("[%d] pec 19-h\n", (int)Now());
                // nEnrollFeatureAdded = 1;
            }
            else
            {
//                int pnDNNFeatureCount[2];
//                unsigned short* prDNNEnrolledFeature[2];

//                pnDNNFeatureCount[0] = g_xEnrollFeat_Hand.nDNNFeatCount;
//                prDNNEnrolledFeature[0] = (unsigned short*)g_xEnrollFeat_Hand.arFeatBuffer;

//                float rDNNScore;
//                int nDNNMaxScoreIndex;
//                GetMaxDNNScore(prDNNEnrolledFeature, pnDNNFeatureCount, 1, g_arLastDNNFeature_Hand, &rDNNScore, &nDNNMaxScoreIndex);
//                APP_LOG("[%d] pec 20-h %f\n", (int)Now(), rDNNScore);

//                if(rDNNScore < DNN_ENROLL_CHECK_THRESHOLD_HAND)
//                {
//                    APP_LOG("[%d] pec 21-h\n", (int)Now());
//                    return ES_PROCESS;
//                }

                memcpy((unsigned short*)(g_xEnrollFeat_Hand.arFeatBuffer) + g_xEnrollFeat_Hand.nDNNFeatCount * KDNN_FEAT_SIZE, g_arLastDNNFeature_Hand, sizeof(unsigned short)* KDNN_FEAT_SIZE);
                g_xEnrollFeat_Hand.nDNNFeatCount = g_xEnrollFeat_Hand.nDNNFeatCount + 1;
                // nEnrollFeatureAdded = 1;
            }
            g_nEnrollProcessCount_Hand ++;
        }

        if(g_xEngineParam.iDupCheck && g_nEnrollePersonCount_Dup_Hand)
        {
            float rDNNScore;
            int nDNNMaxScoreIndex;
            GetMaxDNNScore(g_prDNNEnrolledFeature_Dup_Hand, g_nDNNFeatureCount_Dup_Hand, g_nEnrollePersonCount_Dup_Hand, g_arLastDNNFeature_Hand, &rDNNScore, &nDNNMaxScoreIndex);
            if (rDNNScore > DNN_VERIFY_THRESHOLD_HAND)
            {
                APP_LOG("[%d] pec 22-h\n", (int)Now());
                g_nDuplicated = 1;
            }
        }
        APP_LOG("[%d] pec 23-h %d\n", (int)Now(), g_nEnrollProcessCount_Hand);
    }

    if(g_nRegiterTotalFrameCount >= HAND_ENROLL_MAX_WAIT_FRAMECOUNT && g_nEnrollProcessCount_Hand != HAND_ENROLL_FEAT_COUNT)
    {
        //refill feat
        int nIndex;
        for(nIndex = 0; nIndex < HAND_ENROLL_FEAT_COUNT - g_nEnrollProcessCount_Hand; nIndex ++)
        {
            g_iDNNFeatureExtracted_Hand = 0;
            //restore backup buffer
            memcpy(g_shared_mem_Hand, g_pAlignBackupBuffer + nIndex * 128 * 128, 128 * 128);
            extarctDNNFeature_process_Hand();
            if(g_iDNNFeatureExtracted_Hand)
            {
                memcpy((unsigned short*)(g_xEnrollFeat_Hand.arFeatBuffer) + g_xEnrollFeat_Hand.nDNNFeatCount * KDNN_FEAT_SIZE, g_arLastDNNFeature_Hand, sizeof(unsigned short)* KDNN_FEAT_SIZE);
                g_xEnrollFeat_Hand.nDNNFeatCount = g_xEnrollFeat_Hand.nDNNFeatCount + 1;
                if(g_xEngineParam.iDupCheck && g_nEnrollePersonCount_Dup_Hand)
                {
                    float rDNNScore;
                    int nDNNMaxScoreIndex;
                    GetMaxDNNScore(g_prDNNEnrolledFeature_Dup_Hand, g_nDNNFeatureCount_Dup_Hand, g_nEnrollePersonCount_Dup_Hand, g_arLastDNNFeature_Hand, &rDNNScore, &nDNNMaxScoreIndex);
                    if (rDNNScore > DNN_VERIFY_THRESHOLD_HAND)
                    {
                        APP_LOG("[%d] pec 22-h\n", (int)Now());
                        g_nDuplicated = 1;
                    }
                }
            }
        }
        APP_LOG("[%d] pec 23-h-f\n", (int)Now());
        g_nEnrollProcessCount_Hand = HAND_ENROLL_FEAT_COUNT;
    }


    IF_FLAG_STOP1(ES_FAILED);

    if (g_nEnrollProcessCount_Hand == HAND_ENROLL_FEAT_COUNT)
    {
        refineEnrolledFeat(2);
        g_nPassedDirectionCount ++;
        if (g_nPassedDirectionCount == g_nEnrollDirectionMax)
        {
            if (g_nDuplicated && (g_nEnrollDirectionMax == 1))
            {
                return ES_DUPLICATED;
            }

            return ES_SUCCESS;
        }
        else
        {
            if (g_nDuplicated)
            {
                return ES_DUPLICATED;
            }

            return ES_ENEXT;
        }
    }

    return ES_PROCESS;
}


int*    fr_GetHandDetected()
{
    return &g_nHandDetected;
}
int* fr_GetAverageLedOnImage_Hand()
{
    return &g_rAverageLedOnImage_Hand;
}

float   fr_GetSaturatedRate_Hand()
{
    return g_rSaturatedRate_Hand;
}
int     fr_GetBrightUpThresholdLevel_Hand()
{
    return g_nBrightUpThresholdLevel_Hand;
}


SEngineResult* fr_GetEngineResult_Hand()
{
    return &g_xEngineResult_Hand;
}


int update_DNN_EnrollFeature_sizeChanged_Hand(unsigned short* prEnrollFeatureToUpdate, int* pnEnrolledFeatureCount, unsigned short* pVerifyFeatureToUpdate)
{
    int nIndexToUpdate = -1;
    // int nFeatureCountPerEnv[ENV_COUNT] = { 0 };

    if (*pnEnrolledFeatureCount < TOTAL_ENROLL_MAX_DNNFEATURE_HAND_COUNT)
    {
        nIndexToUpdate = *pnEnrolledFeatureCount;
        memcpy(prEnrollFeatureToUpdate + nIndexToUpdate * KDNN_FEAT_SIZE, pVerifyFeatureToUpdate, KDNN_FEAT_SIZE * sizeof(unsigned short));
        *pnEnrolledFeatureCount = *pnEnrolledFeatureCount + 1;
        return nIndexToUpdate;
    }
    else
    {

        int nEnrollFeatureIndex;
        float rMaxScore = 0;
        for (nEnrollFeatureIndex = 0; nEnrollFeatureIndex < *pnEnrolledFeatureCount; nEnrollFeatureIndex++)
        {
            float rScore = KdnnGetSimilarity(prEnrollFeatureToUpdate + nEnrollFeatureIndex * KDNN_FEAT_SIZE, pVerifyFeatureToUpdate);
            if (rScore > rMaxScore)
            {
                rMaxScore = rScore;
                nIndexToUpdate = nEnrollFeatureIndex;
            }
        }

        if (nIndexToUpdate == -1)
        {
            return -1;
        }

        memcpy(prEnrollFeatureToUpdate + nIndexToUpdate * KDNN_FEAT_SIZE, pVerifyFeatureToUpdate, KDNN_FEAT_SIZE * sizeof(unsigned short));
    }
    return nIndexToUpdate;
}

int fr_UpdateFeat_Hand(int nUserIndex, unsigned short* prDNNFeature)
{
#ifdef DB_BAK3
    //skip feature update in case of using backup partition.
    if (is_backup_partition())
        return ES_FAILED;
#endif //DB_BAK3

    //must be correct to N person
    if (nUserIndex < 0)
        return ES_FAILED;

    SHandFeatInfo* pxFeatInfo = dbm_GetHandFeatInfoByIndex(nUserIndex);
    if (pxFeatInfo == NULL)
        return ES_FAILED;

    int nRet2 = -1;

    nRet2 = update_DNN_EnrollFeature_sizeChanged_Hand((unsigned short*)pxFeatInfo->arFeatBuffer, &pxFeatInfo->nDNNFeatCount, prDNNFeature);

    if(nRet2 != -1)
        return dbm_UpdateHandFeatInfo(nUserIndex, pxFeatInfo, NULL);

    return 0;
}



#endif
