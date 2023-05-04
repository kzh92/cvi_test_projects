
#include "FaceRetrievalSystem.h"
#include "engineparam.h"
#include "DBManager.h"
#include "ImageProcessing.h"
#include "KDNN_EngineInterface.h"
#include "HEngine.h"
#include "gammacorrection.h"
#include "dictionary.h"
#include "appdef.h"
#include "HAlign.h"
#include "manageEnvironment.h"
#include "DBManager.h"
#include "sha1.h"
#include "engineparam.h"
//#include "armCommon.h"
#include "convert.h"
#include "HandRetrival_.h"
#include "hand_feat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __RTK_OS__
#include <shared.h>
#endif

#include <fcntl.h>
#include <stdarg.h>

#include "Pose.h"
#include "UltraFace.hpp"
#include "esn_detection.h"
#include "occ_detection.h"
#include "FaceRetrievalSystem_base.h"
#include "FaceRetrievalSystem_dnn.h"
#include "manageIRCamera.h"

#ifndef __RTK_OS__
#include "sn.cpp"
#endif // !__RTK_OS__

#include "detect.h"
#include "modeling.h"
#include "livemnse.h"
#include "livemnse3.h"
#include "occ.h"
#include "esn.h"
#include "feat.h"
#include "enn_global.h"
#include "ennq_global.h"
#include "dic_manage.h"
#include <math.h>

#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
#include "FaceRetrievalSystem_h.h"
#endif

unsigned char*   g_pbYIrImage = 0;//g_pbYIrImage[N_D_ENGINE_SIZE]
unsigned char*   g_pbDiffIrImage = 0;
unsigned char*   g_pbOffIrImage = 0;
unsigned char*   g_pbOffIrImage2 = 0;

//////////////////////////////////////////////   For DNN Memory Optimization  /////////////////////////////////////
unsigned char*  g_shared_mem = 0;
unsigned char*  g_global_memory = 0;


SFeatInfo*      g_xEnrollFeatA = 0;//SFeatInfo       g_xEnrollFeatA = {0};
SEngineResult   g_xEngineResult = {0};
SEngineParam    g_xEngineParam = {0};
int             g_nUpdateID = -1;

int             g_rAverageLedOffImage;
int             g_rAverageDiffImage;
int             g_rAverageLedOnImage;
int*            g_nHistInLEDOnImage;//g_nHistInLEDOnImage[256];
int*            g_nHistInLEDOnImage_temp;//g_nHistInLEDOnImage_temp[256];
int             g_nFirstImageFaceAvailable;

int             g_nProcessArea;
float           g_rSaturatedRate;
int             g_nBrightUpThresholdLevel = 0;

int             g_nEnrollProcessCount;

// for dnn
int*            g_nIndexList;//int g_nIndexList[N_MAX_PERSON_NUM];
int*            g_nIndexList_Admin;//int g_nIndexList_Admin[N_MAX_PERSON_NUM];
int*            g_nIndexList_Dup;//int g_nIndexList_Dup[N_MAX_PERSON_NUM];
int*            g_nDNNFeatureCount_Dup;//int g_nDNNFeatureCount_Dup[N_MAX_PERSON_NUM];
int*            g_nDNNFeatureCount;//int g_nDNNFeatureCount[N_MAX_PERSON_NUM];
int*            g_nDNNFeatureCount_Admin;//int g_nDNNFeatureCount_Admin[N_MAX_PERSON_NUM];
unsigned short**         g_prDNNEnrolledFeature_Dup;//float* g_prDNNEnrolledFeature_Dup[N_MAX_PERSON_NUM];
unsigned short**         g_prDNNEnrolledFeature;//float*          g_prDNNEnrolledFeature[N_MAX_PERSON_NUM];
unsigned short**         g_prDNNEnrolledFeature_Admin;//float* g_prDNNEnrolledFeature_Admin[N_MAX_PERSON_NUM];

int             g_nEnrollePersonCount_Dup;

int             g_fRealState = 0;
int             g_nVerifyInitCount = 0;

int             g_thread_flag_detect = 0;
int             g_thread_flag_model = 0;
int             g_thread_flag_spoofa1 = 0;
int             g_thread_flag_spoofa2 = 0;
int             g_thread_flag_spoofb = 0;
int             g_thread_flag_spoofb2 = 0;
int             g_thread_flag_spoofc = 0;
#ifdef ENGINE_FOR_DESSMAN
int             g_thread_flag_esn = 0;
int             g_thread_flag_occ = 0;
#endif // ENGINE_FOR_DESSMAN
int             g_thread_flag_feat = 0;

int             g_thread_flag_H_1 = 0;
int             g_thread_flag_H_2 = 0;

mythread_ptr g_EngineInitThrad = 0;


//Extract DNN Feature
unsigned short* g_arLastDNNFeature;//float g_arLastDNNFeature[KDNN_FEAT_SIZE];

static int		g_nContinueRealCount = 0;
static int		g_nLivenessCheckStrong_On_NoUser = 1;

int g_nNeedToCalcNextExposure = 0;
int g_nStopEngine = 0;
float g_rSecurityValue = 0;
int g_nSecurityMode = SECURITY_LEVEL_COMMON;

static Face_Process_Data* g_xFaceProcessData = 0;//Face_Process_Data g_xFaceProcessData;

//added by KSB 20180802
static int g_iDNNFeatureExtracted = 0;
void extarctDNNFeature_process();

int fr_UpdateFeat_DNN(int nUserIndex, float* prDNNFeature, unsigned char nCurInfo);
int isFaceISGoodForDNN();//added 20180718

int getEnvFromInfo(unsigned char nInfo);
int getGlassFromInfo(unsigned char nInfo);

#define ENV_COUNT					2
#define H_FEATURE_COUNT_PER_ENV		5//
#define MIN_H_FEATURE_COUNT_ALL_H	3//added by KSB 20180718

static int g_exposure = 0;
static int g_nGain = 0;
static int g_nFineGain = 0;
static int g_exposure2 = 0;
static int g_nGain2 = 0;
static int g_nFineGain2 = 0;
static int g_nMainControlCameraIndex = 0;

int g_exposure_bkup = 0;
int g_nGain_bkup = 0;
int g_nFineGain_bkup = 0;
int g_exposure2_bkup = 0;
int g_nGain2_bkup = 0;
int g_nFineGain2_bkup = 0;
int g_nMainProcessCameraIndex = 0;
int g_nNeedSmallFaceCheck = 0;

int g_nBayerYConvertedCameraIndex = -1;
int g_nEntireImageAnalysed = 0;

//int g_exposureImage = 0;
//int g_exposurePrev = 0;


static int g_exposureImage = 0;
static int g_exposurePrev = 0;

//added by KSB
static int      g_nFaceDetected = 0;
static int      g_nFaceDetectedMode = 0;

//int      g_nCurEnv = 0;
static int		g_nCurEnvForCameraControl = -1;//0:dark, 1:indoor, 2:oudoor
//static int		g_nPrevEnvForCameraControl = -1;//0:dark, 1:indoor, 2:oudoor
int		g_nCurEnvForCameraControlSettenFromOffImage = 0;//0:dark, 1:indoor, 2:oudoor

static int g_nGlassesEquiped = 0;
static int g_nGlassesEquipedPrev = 0;

int g_nSecondImageNeedReCheck = 0;
int g_nSecondImageIsRight = 0;
int g_nEnrollFeatureAdded = 0;

////////////for Arm/H Detection added by KSB 20180718/////////////////////////////////////
#define MAX_INDOOR_AVERAGE_VALUE			35
#define MAX_SEMI_OUTDOOR_AVERAGE_VALUE		70
#define VALID_DIFF_PIXEL_COUNT				100
#define H_DETECTIONFAILED_FRAMECOUNT_TO_ARM	2

#define MIN_LEDOFF_AVE_DIFF_VALID			10
#define MAX_LEDOFF_AVE_DIFF_VALID			60
#define MAX_ON_OFF_RATE_VALID				0.9f

int analyseBufferAndUpgradeCurEnvForCameraControl(unsigned char* pbLedOffBuffer, int nBufferWidth, int nBufferHeigt, int nCurExp, int nIsOffImage);

//////////////////////////////////////////////////////////////////////////

/////////////////////		Enroll Liveness Detection			////////////////////////////////////////
//added by KSB 20180718
#define ENROLL_LIVENESS_MIN_AVERAGE	40
int g_nEnrolledFrameCountUnderMin = 0;
int g_nEnrollDirectionMode = ENROLL_MULTI_DIRECTION_MODE;
int g_nEnrollDirectionMax = 0;


////////////////////////       Pose         ////////////////////////////////
float g_rAngleX;
float g_rAngleY;
float g_rAngleZ;
//Pose m_Pose;
int pose_estimate();
int isCorrectPose(int nEnrollStep);
int isGoodFaceToEnroll();//mode 0:enroll, 1:verify

float g_rFrontAngleX;
float g_rFrontAngleY;
float g_rFrontAngleZ;
float g_rFrontPan;
float g_rTotalFrontAngleX;
float g_rTotalFrontAngleY;
float g_rTotalFrontAngleZ;
float g_rTotalFrontPan;

int   g_nFrontFaceEnrolled = 0;
int   g_nFrontFacePassed = 0;

int g_nFakeFrameCountInOneDirection = 0;
int g_nFakeFrameCountInOneDirection_Prev = 0;

int g_nPassedDirectionCount = 0;


//////////////////////////////////////////        DNN Face Detection              /////////////////////////////////////////////////////////////////////
//UltraFace m_DnnFace;
unsigned char*   g_pbFaceDetectionBuffer = 0;//unsigned char   g_pbFaceDetectionBuffer[0x8000];
//float g_rLandmarkPoint[68*2];

////////////////////////////////////        Check EyeState and Occlusion        //////////////////////////////////////////
int g_nWaitingOpenEye = 0;
int g_nCheckOpenEyeEnable = 0;

float g_rExposureSettenTime = 0;

/////////////////////////////////////////////          Dic Manage            /////////////////////////////////////////////////////////
int g_nHDicCheckSum = 0;
int g_nHDicCheckSumMatched = 0;

/////////////////////////////////////////////////////       Thread     ////////////////////////////////////////////////////////
int g_nThreadCount = ENGINE_THREAD_COUNT;


#ifdef TimeProfiling
float g_rStartTime;
#endif

//extern const ARM_COMPACT_RECT_PATTERN gpx_Detector_RectPattern[3597];
//extern const ARM_COMPACT_QUAD_PATTERN gpx_Detector_QuadPattern[1500];
//extern const ARM_COMPACT_WAVE_PATTERN gpx_Detector_WavePattern[1053];
//extern const ARM_COMPACT_RECT_PATTERN gpx_Model_RectPattern[19095];
//extern const ARM_COMPACT_QUAD_PATTERN gpx_Model_QuadPattern[2850];
//extern const ARM_COMPACT_WAVE_PATTERN gpx_Model_WavePattern[1425];

#ifdef LOG_MODE
int g_nLogIndex;
int g_nSubIndex;
float g_rLastTime;
float g_rFrameCaptureTime;

int ReadSuccessSendCount1()
{
    FILE* fp = fopen("/db1/cur_success.ini", "rb");
    if(fp == NULL)
        return 0;

    int ret = 0;
    fread(&ret, sizeof(int), 1, fp);
    fclose(fp);

    if(ret < 0)
        return 0;

    return ret;
}

void IncreaseSuccessSendCount1()
{
    int runningCount = ReadSuccessSendCount1();
    runningCount ++;

    FILE* fp = fopen("/db1/cur_success.ini", "wb");
    if(fp)
    {
        fwrite(&runningCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}
#endif

void APP_LOG(const char * format, ...)
{
#ifndef __RTK_OS__
#if 1
    if(g_xEngineParam.iEnableLogFile)
    {
        FILE* fp = fopen(APP_LOG_PATH, "rb");
        if(fp)
        {
            int iFileLen = 0;
            fseek(fp, 0, SEEK_END);
            iFileLen = ftell(fp);
            fclose(fp);

            if(iFileLen > 2 * 1024 * 1024)
                remove(APP_LOG_PATH);
        }

        fp = fopen(APP_LOG_PATH, "ab");
        if(fp)
        {
            va_list args;
            va_start (args, format);
            vfprintf (fp, format, args);
            va_end (args);

            fclose(fp);
            //sync();
        }
    }
#else
    va_list args;
    va_start (args, format);
    vprintf (format, args);
    va_end (args);
#endif
#else // !__RTK_OS__
#if 1
    va_list valist;
    char str[1024];
    if(g_xEngineParam.iEnableLogFile)
    {
        va_start(valist, format);
        vsnprintf(str, 1023, format, valist);
        va_end(valist);
        fr_WriteAppLog("app_log.txt", 0, str, strlen(str));
    }
#else
    char buf[1024];
    va_list args;
    va_start (args, format);
    vsnprintf(buf, 1023, format, args);
    va_end (args);
    my_printf(buf);
#endif
#endif // !__RTK_OS__
}

#ifndef __RTK_OS__
static int g_iDevMem = 0;
static unsigned int g_iGlobalMem = 0;
int DevMemInit()
{
    int fd;
    unsigned int addr_start, addr_offset;
    unsigned int PageSize, PageMask;
    void *pc;
    int iCheckSum = 0;
    
    if(g_iDevMem == -1)
        return -1;

    if(g_iDevMem == 0)
    {
        fd = open("/dev/mem", O_RDWR);
        if(fd < 0)
        {
            perror("Unable to open /dev/mem");
            return(-1);
        }

        PageSize = sysconf(_SC_PAGESIZE);
        PageMask = (~(PageSize-1));

        addr_start  = GLOBAL_MEM_ADDR &  PageMask;
        addr_offset = GLOBAL_MEM_ADDR & ~PageMask;

        pc = (void *)mmap(0, GLOBAL_MEM_SIZE + PageSize * 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr_start);
        if(pc == MAP_FAILED)
        {
            perror("Unable to mmap file");
            return(-1);
        }

        g_iGlobalMem = (unsigned int)pc;
        g_iGlobalMem = ((unsigned int)g_iGlobalMem + addr_offset);

        close(fd);

        g_iDevMem = 1;

        int iOff = 0;
        g_dic_detect = (unsigned char*)g_iGlobalMem + iOff;
        int nDetectDicSize = Detect_dnn_dic_size();
        iCheckSum = GetIntCheckSum((int*)g_iGlobalMem, nDetectDicSize);
        if(iCheckSum != *(int*)(g_iGlobalMem + nDetectDicSize))
        {
            munmap(pc, GLOBAL_MEM_SIZE + PageSize * 2);
///			g_pbAData1 = NULL;
            g_dic_detect = NULL;
            g_iDevMem = -1;
            return -1;

        }
        setLoadedDicFlag(MachineFlagIndex_DNN_Detect, 1);
    }
    return 0;
}
#endif


int buffer_rotate90_bayerConvert(unsigned char* pbSrcLandScapeBayerBuffer, unsigned char* pbDstBuffer, int nWidth, int nHeight, int* pnFaceRectInDst, int nForcedConvert)
{
	int nDstLeft, nDstTop, nDstRight, nDstBottom;
	int nSrcLeft, nSrcTop, nSrcRight, nSrcBottom;

	int y, x;

	nDstLeft = 0;
	nDstTop = 0;
	nDstRight = nWidth;
	nDstBottom = nHeight;
	if (pnFaceRectInDst)
	{
		nDstLeft = pnFaceRectInDst[0];
		nDstTop = pnFaceRectInDst[1];
		nDstRight = pnFaceRectInDst[2];
		nDstBottom = pnFaceRectInDst[3];
	}

	unsigned char* pY_init = 0;
	if (g_xEngineParam.iCamFlip == 0)
	{
		nSrcLeft = nHeight - 1 - nDstBottom;
		nSrcTop = nDstLeft;
		nSrcRight = nHeight - 1 - nDstTop;
		nSrcBottom = nDstRight;
		pY_init = pbDstBuffer + nDstBottom * nWidth + nDstLeft;
	}
	else
	{
		nSrcLeft = nDstTop;
		nSrcTop = nWidth - 1 - nDstRight;
		nSrcRight = nDstBottom;
		nSrcBottom = nWidth - 1 - nDstLeft;
		pY_init = pbDstBuffer + nDstTop * nWidth + nDstRight;
	}

	int nSrcDelta = 0;
	int nSrcInitDelta = 0;
	if (g_xEngineParam.iCamFlip == 0)
	{
		nSrcDelta = -nWidth;
		nSrcInitDelta = 1;
	}
	else
	{
		nSrcDelta = nWidth;
		nSrcInitDelta = -1;
	}


	for (y = nSrcTop; y <= nSrcBottom; y++)
	{
		uint8_t* pY = pY_init;
        for (x = nSrcLeft; x < nSrcRight; x++)
		{
			//*pY = pbSrcLandScapeBuffer[nY1 * nHeight + nX1];
            if((*pY == 0 && !nForcedConvert) || nForcedConvert)
            {
                *pY = getYFromBAYER(pbSrcLandScapeBayerBuffer, nHeight, nWidth, x, y);// pbSrcLandScapeBuffer[nY1 * nHeight + nX1];
            }
			pY += nSrcDelta;
		}
		pY_init += nSrcInitDelta;
	}
	return 0;
}


void calcAverageValues(unsigned char* pLEDOnImage, unsigned char* pLEDOffImage, SRect rect)
{
    int nX, nY;
    int nTotalLedOffValue = 0;
    int nTotalLedOnValue = 0;

    int nProcessArea = rect.width * rect.height / 4;
    memset(g_nHistInLEDOnImage, 0, sizeof(int) * 256);

    _u8 *pLedOnInit, *pLedOffInit = 0;
    int nOffset = rect.y * g_xEngineParam.nDetectionWidth + rect.x;
    int E_IMAGE_WIDTH_2 = g_xEngineParam.nDetectionWidth << 1;
    pLedOnInit = pLEDOnImage + nOffset;
    if(pLEDOffImage)
    {
        pLedOffInit = pLEDOffImage + nOffset;
    }

    for (nY = rect.y; nY < rect.y + rect.height; nY += 2)
    {
        _u8* pLEDOnBuf = pLedOnInit;
        _u8* pLEDOffBuf = 0;
        if(pLEDOffImage)
        {
            pLEDOffBuf = pLedOffInit;
        }

        for (nX = rect.x; nX < rect.x + rect.width; nX += 2)
        {
            int bLedOn, bLedOff;
            bLedOn = *pLEDOnBuf;
            nTotalLedOnValue += bLedOn;
            g_nHistInLEDOnImage[bLedOn] ++;
            pLEDOnBuf += 2;

            if(pLEDOffImage)
            {
                bLedOff = *pLEDOffBuf;
                nTotalLedOffValue += bLedOff;
                pLEDOffBuf += 2;
            }
        }
        pLedOnInit += E_IMAGE_WIDTH_2;
        pLedOffInit += E_IMAGE_WIDTH_2;
    }

    if(nProcessArea)
    {
        g_rAverageLedOffImage = nTotalLedOffValue / nProcessArea;
        g_rAverageLedOnImage = nTotalLedOnValue / nProcessArea;
        g_nProcessArea = nProcessArea;
        float rSigmaSaturation = 0;
        int nValue;
        for(nValue = 250; nValue <= 255; nValue++)
        {
            rSigmaSaturation += g_nHistInLEDOnImage[nValue];
        }

        g_rSaturatedRate = rSigmaSaturation *  100.0f / g_nProcessArea;

        int nAvaiableSatuartionPixels = nProcessArea * SAT_THRESHOLD / 100;
        int nLevel;
        int nThresLevel = 0;
        int nSigma = 0;
        for (nLevel = 255; nLevel >= 0; nLevel--)
        {
            nSigma += g_nHistInLEDOnImage[nLevel];

            if (nSigma > nAvaiableSatuartionPixels)
            {
                nThresLevel = nLevel;
                break;
            }
        }

        nThresLevel++;
        g_nBrightUpThresholdLevel = nThresLevel;
    }
}


void calcAverageValues_rotate(unsigned char* pLEDOnImage, SRect rect)
{
    int nX, nY;
    int nTotalLedOffValue = 0;
    int nTotalLedOnValue = 0;

//    int nProcessArea = (rect.width / 2) * (rect.height / 2);
    int nProcessArea = 0;
    memset(g_nHistInLEDOnImage, 0, sizeof(int) * 256);

    int nOffset2[5][2] = {{0,0},{0,1},{0,-1},{-1,0},{1,0}};

    for (nY = rect.y; nY < rect.y + rect.height; nY += 4)
    {
        for (nX = rect.x; nX < rect.x + rect.width; nX += 4)
        {
            int nXInBayer, nYInBayer;
            int nSrcY = nY;
            int nSrcX = nX;
            nXInBayer = g_xEngineParam.nDetectionHeight - nSrcY + 1;
            nYInBayer = nSrcX;
            if (g_xEngineParam.iCamFlip == 0)
            {
                nYInBayer = nSrcX;
                nXInBayer = g_xEngineParam.nDetectionHeight - 1 - nSrcY;
            }
            else
            {
                nXInBayer = nSrcY;
                nYInBayer = g_xEngineParam.nDetectionWidth - nSrcX - 1;
            }

            int bLedOn;
            int nProcessCount = 0;
            bLedOn = 0;

            int nDirectionIndex = 0;
            for(nDirectionIndex = 0; nDirectionIndex < 5; nDirectionIndex ++)
            {
                int nOffsetY = nYInBayer + nOffset2[nDirectionIndex][1];
                int nOffsetX = nXInBayer + nOffset2[nDirectionIndex][0];

                if(nOffsetX >= 0 && nOffsetX < g_xEngineParam.nDetectionHeight &&
                   nOffsetY >= 0 && nOffsetY < g_xEngineParam.nDetectionWidth)
                {
                    bLedOn += pLEDOnImage[nOffsetY * g_xEngineParam.nDetectionHeight + nOffsetX];
                    nProcessCount ++;
                }
            }
            bLedOn = bLedOn / nProcessCount;
            nTotalLedOnValue += bLedOn;
            nProcessArea ++;
            g_nHistInLEDOnImage[bLedOn] ++;

        }
    }

    if(nProcessArea)
    {
        g_rAverageLedOffImage = nTotalLedOffValue / nProcessArea;
        g_rAverageLedOnImage = nTotalLedOnValue / nProcessArea;
        g_nProcessArea = nProcessArea;
        float rSigmaSaturation = 0;
        int nValue;
        for(nValue = 212; nValue <= 255; nValue++)
        {
            rSigmaSaturation += g_nHistInLEDOnImage[nValue];
        }

        g_rSaturatedRate = rSigmaSaturation *  100.0f / g_nProcessArea;
    }
}


//pnRect: left, top, right, bottom
int isFaceOutOfImage(int* pnRect, int nImageWidth, int nImageHeight)
{
    int nWidth = pnRect[2] - pnRect[0];
    int nHeight = pnRect[3] - pnRect[1];
    int nMaxWidth_Height = nWidth;
    if(nMaxWidth_Height < nHeight)
    {
        nMaxWidth_Height = nHeight;
    }
    float rOutRate = 0.1f;

    int nWidthThreshold = (int)((float)nMaxWidth_Height * rOutRate);
    int nUpHeightThreshold = 0;
    int nDownHeightThreshold = nImageHeight - 1 - 60;
    int nLeftThreshold = - nWidthThreshold;
    int nRightThreshold = nImageWidth - 1 + nWidthThreshold;

#if (ENGINE_LENS_TYPE == ENGINE_LENS_M277_2409)
    nUpHeightThreshold = 96;
    nDownHeightThreshold = nImageHeight - 1 - 126;
    nLeftThreshold = 84 - nWidthThreshold;
    nRightThreshold = nImageWidth - 84 + nWidthThreshold;
#endif

    if(pnRect[0] < nLeftThreshold)
    {
        return 2;
    }
    if(pnRect[2] > nRightThreshold)
    {
        return 1;
    }
    if(pnRect[1] <  nUpHeightThreshold)
    {
        return 3;
    }
    if(pnRect[3] > nDownHeightThreshold)
    {
        return 4;
    }

    return 0;
}


int isGoodFaceToEnroll()
{
    float rFaceSize = g_xFaceProcessData->rFaceRect[2];

    if (g_rAngleY > ENROLL_MIN_YAW && g_rAngleY < ENROLL_MAX_YAW && rFaceSize > FACE_MIN_WIDTH_AVAIABLE_ENROLL)
    {
        return 1;
    }
    return 0;
}

//nBadCode:0-good, 1-too small, 2-too big, 3-out of image, 4-low score
int getBestIndex(FaceInfo* face_info, int n_face_cnt, int &nBadCode)
{
    int nFACE_MIN_WIDTH_AVAIABLE_ENROLL = FACE_MIN_WIDTH_AVAIABLE_ENROLL;
    int nFACE_MAX_WIDTH_AVAIABLE = FACE_MAX_WIDTH_AVAIABLE;

    int nBestFaceSizeInGoodFace = 0;
    int nBestFaceSizeInBadFace = 0;
    int nBestFaceIndexInGoodFace = -1;
    int nBestFaceIndexInBadFace = -1;
    int nBestFaceInBadFace_BadCode = 0;

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
        if((g_xEngineParam.fEngineState == ENS_REGISTER && nFaceMaxLine < nFACE_MIN_WIDTH_AVAIABLE_ENROLL && g_nPassedDirectionCount == 0) /*||
                nFaceMaxLine < nFACE_MIN_WIDTH_AVAIABLE*/)
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
    return nBestFaceIndexInBadFace;
}


int checkFaceInCamera2Image_expand_shrink_DNN(unsigned char* pCamera2Buffer_8Byaer, int nBufferWidth, int nBufferHeight, float* prFaceRectInMainCamera, int nCameraisRight)
{
    int nFaceWidthInMainCamera = (int)(prFaceRectInMainCamera[2]);
    int nFaceCenterXInMainCamera = (int)(prFaceRectInMainCamera[0] + prFaceRectInMainCamera[2] / 2);
    int nFaceCenterYInMainCamera = (int)(prFaceRectInMainCamera[1] + prFaceRectInMainCamera[3] / 2);

    int nDstCenterX = CheckFaceBuffer_Width / 2;
    int nDstCenterY = CheckFaceBuffer_Height / 2;

    float rScaleDst2Src = (float)nFaceWidthInMainCamera / 20;
    int nX, nY;

    int nOffset = getDetectMenSize();
    if(nOffset < N_D_ENGINE_SIZE)
    {
        nOffset = N_D_ENGINE_SIZE;
    }
    //g_pbFaceDetectionBuffer = g_shared_mem + getDetectMenSize();
    unsigned char *pbDstBuffer = g_shared_mem + nOffset;

    for(nY = 0; nY < CheckFaceBuffer_Height; nY ++)
    {
        int nSrcY = nFaceCenterYInMainCamera + (int)((float)(nY - nDstCenterY) * rScaleDst2Src);

        for(nX = 0; nX < CheckFaceBuffer_Width; nX ++)
        {
            int nSrcX = nFaceCenterXInMainCamera + (int)((float)(nX - nDstCenterX) * rScaleDst2Src);

            unsigned char bDstValue = 0x80;
            if(nSrcX >= 0 && nSrcX < nBufferWidth && nSrcY >= 0 && nSrcY < nBufferHeight)
            {
                int nXInBayer, nYInBayer;
                nXInBayer = nBufferHeight - nSrcY + 1;
                nYInBayer = nSrcX;
                if ((g_xEngineParam.iCamFlip == 0 && nCameraisRight == 1) || (g_xEngineParam.iCamFlip == 1 && nCameraisRight == 0))
                {
                    nXInBayer = nSrcY;
                    nYInBayer = nBufferWidth - 1 - nSrcX;
                }
                else//((g_xEngineParam.iCamFlip == 1 && nCameraisRight == 1) || (g_xEngineParam.iCamFlip == 0 && nCameraisRight == 0))
                {
                    nXInBayer = nBufferHeight - 1 - nSrcY;
                    nYInBayer = nSrcX;
                }

                bDstValue = getYFromBAYER((void*)pCamera2Buffer_8Byaer, nBufferHeight, nBufferWidth, nXInBayer, nYInBayer);
                bDstValue = getGammaCorrection(bDstValue);
            }
            //g_pbCheckFaceBuffer[nY * CheckFaceBuffer_Width + nX] = bDstValue;

#ifdef DETECT_USE_NCNN
            prDstBuffer[nY * CheckFaceBuffer_Width + nX] = (float)((int)bDstValue - 127) / 128;
#else
            pbDstBuffer[nY * CheckFaceBuffer_Width + nX] = bDstValue;
#endif
        }
    }

	if (g_nStopEngine == 1)
	{
		APP_LOG("[%d] stop by flag_stop\n", (int)Now());
		return 0;
	}
    int nFaceCheck = checkFace(pbDstBuffer, CheckFaceBuffer_Width, CheckFaceBuffer_Height);

    return nFaceCheck;
}


int checkFaceInClr_expand_shrink_DNN(unsigned char* pClrBuffer_YUV222, int nBufferWidth, int nBufferHeight, float* prFaceRectInMainCamera)
{
    int nFaceWidthInMainCamera = (int)(prFaceRectInMainCamera[2]);
    int nFaceCenterXInMainCamera = (int)(prFaceRectInMainCamera[0] + prFaceRectInMainCamera[2] / 2);
    int nFaceCenterYInMainCamera = (int)(prFaceRectInMainCamera[1] + prFaceRectInMainCamera[3] / 2);

    int nDstCenterX = CheckFaceBufferClr_Width / 2;
    int nDstCenterY = CheckFaceBufferClr_Height / 2;

    float rScaleDst2Src = (float)nFaceWidthInMainCamera / 32;
    int nX, nY;

    int nOffset = getDetectMenSize();
//    if(nOffset < N_D_ENGINE_SIZE)
//    {
//        nOffset = N_D_ENGINE_SIZE;
//    }
    //g_pbFaceDetectionBuffer = g_shared_mem + getDetectMenSize();
    unsigned char *pbDstBuffer = g_shared_mem + nOffset;

    for(nY = 0; nY < CheckFaceBufferClr_Height; nY ++)
    {
        int nSrcY = nFaceCenterYInMainCamera + (int)((float)(nY - nDstCenterY) * rScaleDst2Src);

        for(nX = 0; nX < CheckFaceBufferClr_Width; nX ++)
        {
            int nSrcX = nFaceCenterXInMainCamera + (int)((float)(nX - nDstCenterX) * rScaleDst2Src);

            unsigned char bDstValue = 0x80;
            if(nSrcX >= 0 && nSrcX < nBufferWidth && nSrcY >= 0 && nSrcY < nBufferHeight)
            {
                int nXInBayer, nYInBayer;
                nXInBayer = nBufferHeight - nSrcY + 1;
                nYInBayer = nSrcX;
                if (g_xEngineParam.iCamFlip == 0)
                {
                    nXInBayer = nSrcY;
                    nYInBayer = nBufferWidth - 1 - nSrcX;
                }
                else//((g_xEngineParam.iCamFlip == 1 && nCameraisRight == 1) || (g_xEngineParam.iCamFlip == 0 && nCameraisRight == 0))
                {
                    nXInBayer = nBufferHeight - 1 - nSrcY;
                    nYInBayer = nSrcX;
                }

                bDstValue = pClrBuffer_YUV222[nYInBayer * (nBufferHeight * 2) + nXInBayer * 2];
            }
            //g_pbCheckFaceBuffer[nY * CheckFaceBuffer_Width + nX] = bDstValue;

#ifdef DETECT_USE_NCNN
            prDstBuffer[nY * CheckFaceBuffer_Width + nX] = (float)((int)bDstValue - 127) / 128;
#else
            pbDstBuffer[nY * CheckFaceBufferClr_Width + nX] = bDstValue;
#endif
        }
    }

    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return 0;
    }
    int nFaceCheck = checkFace(pbDstBuffer, CheckFaceBufferClr_Width, CheckFaceBufferClr_Height);

    return nFaceCheck;
}

void waitDicChecksum(int* pThread)
{
    if (*pThread != 2)
    {
        float rStartTime = Now();
        while(1)
        {
            my_usleep(1000);
            if(*pThread == 2)
                break;

            if(Now() - rStartTime > 500)
                break;

            if (g_nStopEngine == 1)
            {
                APP_LOG("[%d] stop by flag_stop\n", (int)Now());
                return;
            }
        }
    }
}

int checkFaceIsInPhoto()
{
    if(g_nMainProcessCameraIndex == -1)
    {
        return ES_FAILED;
    }

    unsigned char* pbLedOffImage = g_pbOffIrImage;
    int nExp = g_exposure_bkup;
    int nGain = g_nGain_bkup;
    int nFineGain = g_nFineGain_bkup;
    int nIsLeftCam = 1;

    if(g_nMainProcessCameraIndex == 1)
    {
        pbLedOffImage = g_pbOffIrImage2;
        nExp = g_exposure2_bkup;
        nGain = g_nGain2_bkup;
        nFineGain = g_nFineGain2_bkup;
        nIsLeftCam = 0;
    }

    int nOffImageWidth, nOffImageHeight;
    nOffImageWidth = g_xEngineParam.nDetectionWidth / LEDOFFIMAGE_REDUCE_RATE;
    nOffImageHeight = g_xEngineParam.nDetectionHeight / LEDOFFIMAGE_REDUCE_RATE;
    //get face rect average
    float rFaceRectLeftInOff, rFaceRectRightInOff, rFaceRectTopInOff, rFaceRectBottomInOff;
    int nFaceRectLeftInOff, nFaceRectRightInOff, nFaceRectTopInOff, nFaceRectBottomInOff;
    int nFaceWidth = (int)g_xFaceProcessData->rFaceRect[2];
    rFaceRectLeftInOff = g_xFaceProcessData->rFaceRect[0] + g_xFaceProcessData->rFaceRect[2] * 0.2f;
    rFaceRectRightInOff = g_xFaceProcessData->rFaceRect[0] + g_xFaceProcessData->rFaceRect[2] * 0.8f;
    rFaceRectTopInOff = g_xFaceProcessData->rFaceRect[1] + g_xFaceProcessData->rFaceRect[3] * 0.1f;
    rFaceRectBottomInOff = g_xFaceProcessData->rFaceRect[1] + g_xFaceProcessData->rFaceRect[3] * 0.9f;

    nFaceRectLeftInOff = (int)(rFaceRectLeftInOff / LEDOFFIMAGE_REDUCE_RATE);
    if (nFaceRectLeftInOff * LEDOFFIMAGE_REDUCE_RATE !=  rFaceRectLeftInOff)
    {
        nFaceRectLeftInOff ++;
    }
    nFaceRectRightInOff = (int)(rFaceRectRightInOff / LEDOFFIMAGE_REDUCE_RATE);
    nFaceRectTopInOff = (int)(rFaceRectTopInOff / LEDOFFIMAGE_REDUCE_RATE);
    if(nFaceRectTopInOff * LEDOFFIMAGE_REDUCE_RATE != rFaceRectTopInOff)
    {
        nFaceRectTopInOff ++;
    }
    nFaceRectBottomInOff = (int)(rFaceRectBottomInOff / LEDOFFIMAGE_REDUCE_RATE);
    int nProcessCount = 0;
    int nTotalLedOnValue = 0;
    int nOffImageAverVal = 0;

    int nY, nX;
    for (nY = nFaceRectTopInOff; nY <= nFaceRectBottomInOff; nY ++)
    {
        for (nX = nFaceRectLeftInOff; nX < nFaceRectRightInOff; nX ++)
        {
            int nXInBayer, nYInBayer;
            int nSrcY = nY;
            int nSrcX = nX;
            if ((g_xEngineParam.iCamFlip == 0 && nIsLeftCam == 1) || (g_xEngineParam.iCamFlip == 1 && nIsLeftCam == 0))
            {
                nYInBayer = nSrcX;
                nXInBayer = nOffImageHeight - 1 - nSrcY;
            }
            else
            {
                nXInBayer = nSrcY;
                nYInBayer = nOffImageWidth - nSrcX - 1;
            }

            nTotalLedOnValue += pbLedOffImage[nYInBayer * nOffImageHeight + nXInBayer];
            nProcessCount ++;
        }
    }
    if(nProcessCount)
    {
        nOffImageAverVal = nTotalLedOnValue / nProcessCount;
    }

    APP_LOG("\t\t***OffIm = %d\n", nOffImageAverVal);
    int nDeltaValue = g_rAverageLedOnImage - nOffImageAverVal;
    if(nDeltaValue < 20)
    {
        return ES_SUCCESS;
    }

    float rCornerValueRate = 0.1f;
    float rFaceCenterX, rFaceCenterY;
    float rImageCenterX, rImageCenterY;
    rFaceCenterX = g_xFaceProcessData->rFaceRect[0] + g_xFaceProcessData->rFaceRect[2] * 0.5f;
    rFaceCenterY = g_xFaceProcessData->rFaceRect[1] + g_xFaceProcessData->rFaceRect[3] * 0.5f;
    rImageCenterX = g_xEngineParam.nDetectionWidth / 2;
    rImageCenterY = g_xEngineParam.nDetectionHeight / 2;
    float rFaceCenter_ImageCenter_Dist = sqrt((rImageCenterX - rFaceCenterX) * (rImageCenterX - rFaceCenterX) + (rImageCenterY - rFaceCenterY) * (rImageCenterY - rFaceCenterY));
    float rImageCenter_Dist = sqrt((rImageCenterX - 120) * (rImageCenterX - 120) + (rImageCenterY - 120) * (rImageCenterY - 120));
    float rBrightnessRate = ((rFaceCenter_ImageCenter_Dist * rCornerValueRate + (rImageCenter_Dist - rFaceCenter_ImageCenter_Dist) * 1) / rImageCenter_Dist);
//    printf("rFaceCenterX =%f, rFaceCenterY = %f\n", rFaceCenterX, rFaceCenterY);
//    printf("rFaceCenter_ImageCenter_Dist = %f\n", rFaceCenter_ImageCenter_Dist);
//    printf("rBrightnessRate = %f\n", rBrightnessRate);

    float rCenterThreshold = 700.0f;
    float rCornerThreshold = 940.0f;
    if(rFaceCenterY < rImageCenterY)
    {
        rCornerThreshold = 800.0f;
    }
    float rThreshold = ((rFaceCenter_ImageCenter_Dist * rCornerThreshold + (rImageCenter_Dist - rFaceCenter_ImageCenter_Dist) * rCenterThreshold) / rImageCenter_Dist);

    float rExpValue = ((float)nExp * getGainRateFromGain_SC2355(nGain, nFineGain));
    float rDist = sqrt(rExpValue / (nDeltaValue / rBrightnessRate));
    float rCheckVal = rDist * nFaceWidth;
#if (ENGINE_LENS_TYPE == ENGINE_LENS_M277_2409)
    rCheckVal = rCheckVal /0.8203f;
#endif

    if(rCheckVal > rThreshold)
        return ES_SUCCESS;

    return ES_FAILED;

}


int check2D_3DFake()
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
    float rStartTime = Now();
    my_printf("[%d] before KdnnDetectLiveness2D_ATime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif
    unsigned char *pLiveAlignAC, *pLiveAlignB, *pLiveAlignB2;
    pLiveAlignAC = g_shared_mem;
    pLiveAlignB = pLiveAlignAC + 128*128;
    pLiveAlignB2 = pLiveAlignB + 128*128;

#ifdef TimeProfiling
    my_printf("[%d] generateAlignImageForLiveness Time = %f\r\n", (int)Now(), Now() - rStartTime);
    my_printf("[%d] generateAlignImageForLiveness Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
    rStartTime = Now();
#endif

    waitDicChecksum(&g_thread_flag_spoofa1);
    waitDicChecksum(&g_thread_flag_spoofa2);

    if (g_thread_flag_spoofa1 != 2 ||
        g_thread_flag_spoofa2 != 2)
    {
        APP_LOG("[%d] pec 27-1\n", (int)Now());
        return ES_FAILED;
    }

    float rResult = 0;
    rResult = KdnnDetectLiveness2D_A(pLiveAlignAC);
    //my_printf("KdnnDetectLiveness2D_A = %f\n", rResult);

    if(rResult < 0)
    {
        APP_LOG("[%d] pec 26-1\n", (int)Now());
        return ES_SPOOF_FACE;
    }

    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

#ifdef TimeProfiling
    float rbayerConvertTime = Now() - rStartTime;
    my_printf("[%d] KdnnDetectLiveness2D_A Time = %f\r\n", (int)Now(), rbayerConvertTime);
    my_printf("[%d] KdnnDetectLiveness2D_ATime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
    rStartTime = Now();
#endif

#if (ENGINE_USE_TWO_CAM != 0)//ENGINE_USE_TWO_CAM=0:2D, 1:common, 2:3M
    waitDicChecksum(&g_thread_flag_spoofb);
    if (g_thread_flag_spoofb != 2)
    {
        APP_LOG("[%d] pec 27-2\n", (int)Now());
        return ES_FAILED;
    }

    rResult = KdnnDetectLiveness_2D_B(pLiveAlignB);
    //my_printf("KdnnDetectLiveness2D_B = %f\n", rResult);
    if(rResult < 0)
    {
        APP_LOG("[%d] pec 26-2\n", (int)Now());
        return ES_SPOOF_FACE;
    }

    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

#ifdef TimeProfiling
    rbayerConvertTime = Now() - rStartTime;
    my_printf("[%d] KdnnDetectLiveness_B Time = %f\r\n", (int)Now(), rbayerConvertTime);
    my_printf("[%d] KdnnDetectLiveness_B Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
    rStartTime = Now();
#endif

    waitDicChecksum(&g_thread_flag_spoofb2);
    if (g_thread_flag_spoofb2 != 2)
    {
        APP_LOG("[%d] pec 27-22\n", (int)Now());
        return ES_FAILED;
    }

    rResult = KdnnDetectLiveness_2D_B2(pLiveAlignB2);
    //my_printf("KdnnDetectLiveness2D_B2 = %f\n", rResult);
    if(rResult < 0)
    {
        APP_LOG("[%d] pec 26-22\n", (int)Now());
        return ES_SPOOF_FACE;
    }
#endif
    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

#ifdef TimeProfiling
    rbayerConvertTime = Now() - rStartTime;
    my_printf("[%d] KdnnDetectLiveness_B2 Time = %f\r\n", (int)Now(), rbayerConvertTime);
    my_printf("[%d] KdnnDetectLiveness_B2 Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
    rStartTime = Now();
#endif

    waitDicChecksum(&g_thread_flag_spoofc);
    if (g_thread_flag_spoofc != 2)
    {
        APP_LOG("[%d] pec 27-3\n", (int)Now());
        return ES_FAILED;
    }
    rResult = KdnnDetectLiveness_3D(pLiveAlignAC);
    //my_printf("KdnnDetectLiveness3D = %f\n", rResult);
    if(rResult < 0)
    {
        APP_LOG("[%d] pec 26-3\n", (int)Now());
        return ES_SPOOF_FACE;
    }

#ifdef TimeProfiling
    rbayerConvertTime = Now() - rStartTime;
    my_printf("[%d] KdnnDetectLiveness_3D Time = %f\r\n", (int)Now(), rbayerConvertTime);
    my_printf("[%d] KdnnDetectLiveness_3DTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

    return ES_SUCCESS;

}

int checkEyeOpenness()
{
#ifdef ENGINE_FOR_DESSMAN
    if (!g_nCheckOpenEyeEnable)
    {
        return 1;
    }

	if (g_nStopEngine == 1)
	{
		APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return 0;
	}

    float rScore[4];
    waitDicChecksum(&g_thread_flag_esn);
    if (g_thread_flag_esn != 2)
    {
        return 1;
    }

    int nEst = get_esn_detection(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, g_xFaceProcessData->rLandmarkPoint, rScore, g_pbFaceDetectionBuffer);
	
	if (g_nStopEngine == 1)
	{
		APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return 0;
	}

    return nEst;
#else
    return 1;
#endif
}



void fr_InitEngine(int iDupCheck, int iCamFlip, int nDnnCheckSum, int nHCheckSum)
{

    if(!g_xEngineParam.fLoadDict)
    {
        //g_nDnnFeatureCheckSum = nDnnCheckSum;
        g_nHDicCheckSum = nHCheckSum;

        AllocEngineMemory();
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
        if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
        {
            fr_InitEngine_dnn();
        }
        else
        {
            fr_InitEngine_h();
        }
#else
        fr_InitEngine_dnn();
#endif
        g_xEngineParam.fLoadDict = 1;
    }

    g_xEngineParam.nDetectionWidth = E_IMAGE_HEIGHT;
    g_xEngineParam.nDetectionHeight = E_IMAGE_WIDTH;
//    g_xEngineParam.nModelWidth = E_IMAGE_HEIGHT;
//    g_xEngineParam.nModelHeight = E_IMAGE_WIDTH;
//    g_xEngineParam.nColorImageWidth = E_CLR_HEIGHT;
//    g_xEngineParam.nColorImageHeight = E_CLR_WIDTH;

    g_xEngineParam.arThresholdVals[0] = THRESHOLD1;
    g_xEngineParam.arThresholdVals[1] = THRESHOLD2;
    g_xEngineParam.arThresholdVals[2] = THRESHOLD3;

    g_xEngineParam.iDupCheck = iDupCheck;
    g_xEngineParam.iCamFlip = iCamFlip;

    g_nVerifyInitCount = 0;
    
}

void fr_InitLive()
{
    return;
}

void    fr_FreeEngine()
{
    fr_EndThread();
    //release detect
    releaseDetectEngine(Detect_Mode_Face);
    releaseModelingEngine(0);
    KdnnFreeEngine_liveness();
    KdnnFreeEngine_feat(0);

#ifdef ENGINE_FOR_DESSMAN
    deinit_occ_detection();
    deinit_esn_detection();
#endif

    if(g_global_memory)
    {
        my_free(g_global_memory);
        g_global_memory = 0;
        APP_LOG("[%d] pecc 4-10-1\n", (int)Now());
    }
}

void fr_EndThread()
{
    if(g_EngineInitThrad)
    {
        my_thread_join(&g_EngineInitThrad);
        g_EngineInitThrad = 0;
    }
}

void fr_SetDupCheck(int iDupCheck)
{
    g_xEngineParam.iDupCheck = iDupCheck;
}

void fr_SetCameraFlip(int iCameraFlip)
{
    g_xEngineParam.iCamFlip = iCameraFlip;
}

void fr_EnableLogFile(int iEnable)
{
    g_xEngineParam.iEnableLogFile = iEnable;

    APP_LOG("Log Start!\n");
}


void fr_SetEngineState(int fState, int iParam1, int iParam2, int iParam3, int iParam4, int iParam5)
{
    g_xEngineParam.fEngineState = fState;
    memset(g_xEnrollFeatA, 0, sizeof(SFeatInfo));
    memset(&g_xEngineResult, 0, sizeof(SEngineResult));
    g_nEnrollProcessCount = 0;
    g_nWaitingOpenEye = 0;

	g_xEngineParam.iDemoMode = iParam2;
    if(fState == ENS_VERIFY)
    {
        g_xEngineParam.fVerifyMode = iParam1;
#if (N_MAX_HAND_NUM)
        fr_SetEngineState_Hand(fState, 0);
#endif
        if (!g_nVerifyInitCount)
        {
#if (CHECK_FAKE_USER == 1)
			if (dbm_GetPersonCount() == 0 && g_nLivenessCheckStrong_On_NoUser)
            {
                g_nContinueRealCount = 0;
            }
            else
            {
                g_nContinueRealCount = 1;
            }
#else
            g_nContinueRealCount = 1;
#endif
        }
        else
        {
            g_nContinueRealCount = 0;
        }
        g_nVerifyInitCount++;
		APP_LOG("[%d] ss 0\n", (int)Now());

    }
    else
    {
        g_xEngineParam.iEnrollKind = iParam5;
        g_nEnrollDirectionMax = ENROLL_DIRECTION_COUNT;
        if(iParam4)
        {
            g_nEnrollDirectionMax = iParam4;
        }

        g_nUpdateID = iParam1;
        g_nEnrollDirectionMode = iParam3;
        //added by KSB 20180718
        g_nEnrolledFrameCountUnderMin = 0;
        g_nContinueRealCount = 0;
        g_nFrontFaceEnrolled = 0;
        g_nFrontFacePassed = 0;
		APP_LOG("[%d] ss 1\n", (int)Now());
        g_nFakeFrameCountInOneDirection = 0;
        g_nPassedDirectionCount = 0;

        int nNeedLoadFace = 0;
#if (N_MAX_HAND_NUM)
        int nNeedLoadHand = 0;
#endif

#if !(N_MAX_HAND_NUM)
        nNeedLoadFace = 1;
#else // !N_MAX_HAND_NUM
#if (ENROLL_FACE_HAND_MODE == ENROLL_FACE_HAND_MIX) //face hand enroll mix mode
        nNeedLoadFace = 1;
        nNeedLoadHand = 1;
        //g_xEngineParam.iEnrollKind = -1;
#else // ENROLL_FACE_HAND_MODE
        if(g_xEngineParam.iEnrollKind == EEK_Face)
        {
            nNeedLoadFace = 1;
            nNeedLoadHand = 0;
        }
        else
        {
            nNeedLoadFace = 0;
            nNeedLoadHand = 1;
        }
#endif // ENROLL_FACE_HAND_MODE
#endif // !N_MAX_HAND_NUM

        if(nNeedLoadFace)
        {
            //if(g_xEngineParam.iDupCheck)
            {
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
                if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
                {
                    fr_LoadFeatureForDuplicateCheck_dnn(g_nUpdateID);
                }
                else
                {
                    fr_LoadFeatureForDuplicateCheck_h(g_nUpdateID);
                }
#else
                fr_LoadFeatureForDuplicateCheck_dnn(g_nUpdateID);
#endif
            }
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
            if(g_nSecurityMode == SECURITY_LEVEL_TWIN)
            {
                fr_LoadFeatEngine();
            }
#endif
        }
#if (N_MAX_HAND_NUM)
        if(nNeedLoadHand)
        {
            fr_SetEngineState_Hand(fState, g_nUpdateID);
        }
#endif
    }

    g_iDNNFeatureExtracted = 0;

    //added by KSB 20180718
    g_nGlassesEquiped = 0;
    g_nGlassesEquipedPrev = 0;
#ifdef LOG_MODE
    IncreaseSuccessSendCount1();
#endif
}

int	fr_SetCheckOpenEyeEnable(int nEnable)
{
    g_nCheckOpenEyeEnable = nEnable;
    return 0;
}

int fr_calc_Off(unsigned char *pbLedOffImage)
{
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
        if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
        {
            //return fr_calc_Off_dnn(pbLedOffImage);
        }
        else
        {
            return fr_calc_Off_h(pbLedOffImage);
        }
#else//ENGINE_SECURITY_ONLY_COMMON
    //return fr_calc_Off_dnn(pbLedOffImage);
#endif
    return ES_SUCCESS;
}

int fr_PreExtractFace(unsigned char *pbClrImage, unsigned char *pbLedOnImage)
{
#ifdef LOG_MODE
    g_nSubIndex++;
    g_nLogIndex = ReadSuccessSendCount1();
#endif
    int nRet = 0;

//#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
//    if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
//    {
//        nRet = fr_PreExtractFace_dnn(pbClrImage, pbLedOnImage);
//    }
//    else
//    {
//        nRet = fr_PreExtractFace_h(pbClrImage, pbLedOnImage);
//    }
//#else//ENGINE_SECURITY_ONLY_COMMON
//    nRet = fr_PreExtractFace_dnn(pbClrImage, pbLedOnImage);
//#endif

    g_exposure_bkup = g_exposure;
    g_nGain_bkup = g_nGain;
    g_nFineGain_bkup = g_nFineGain;
    g_exposure2_bkup = g_exposure2;
    g_nGain2_bkup = g_nGain2;
    g_nFineGain2_bkup = g_nFineGain2;
    g_nMainProcessCameraIndex = -1;

    nRet = fr_PreExtractFace_dnn(pbClrImage, pbLedOnImage);
#if (ENGINE_USE_TWO_CAM != 1)//ENGINE_USE_TWO_CAM=0:2D, 1:common, 2:3M
    //if(g_nNeedToCalcNextExposure)
    {
        CalcNextExposure_inner();
    }
#endif
    return nRet;
}


int		fr_PreExtractFace2(unsigned char *pbBayerFromCamera2)
{
#if (ENGINE_USE_TWO_CAM != 1)//ENGINE_USE_TWO_CAM=0:2D, 1:common, 2:3M
    return ES_SUCCESS;
#endif

    int nRet;

#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
    {
        nRet = fr_PreExtractFace2_dnn(pbBayerFromCamera2);
    }
    else
    {
        nRet = fr_PreExtractFace2_h(pbBayerFromCamera2);
    }
#else//ENGINE_SECURITY_ONLY_COMMON
    nRet = fr_PreExtractFace2_dnn(pbBayerFromCamera2);
#endif

    CalcNextExposure_inner();
    return nRet;
}

int		fr_convertCam2(unsigned char *pbBayerFromCamera2)
{
    convert_bayer2y_rotate_cm(pbBayerFromCamera2, g_pbYIrImage, E_IMAGE_WIDTH, E_IMAGE_HEIGHT, 1 - g_xEngineParam.iCamFlip);
    return ES_SUCCESS;
}

int		fr_PreExtractFaceClr(unsigned char *pbYUV222, int nWidth, int nHeight)
{
    if(!(getFaceProcessData())->nFaceDetected)
    {
        return ES_FAILED;
    }
    float rFaceRectInClr[4];
    float rIr2RGB_Rate = 1;

#if (CLR_LENS_TYPE == CLR_LENS_M277_1450)
#if (ENGINE_LENS_TYPE == ENGINE_LENS_40143)
    rIr2RGB_Rate = 1.05f * nWidth / g_xEngineParam.nDetectionHeight;
#elif (ENGINE_LENS_TYPE == ENGINE_LENS_M277_2409)
    rIr2RGB_Rate = 1.306f * nWidth / g_xEngineParam.nDetectionHeight;
#endif
#endif

#if (CLR_LENS_TYPE == CLR_LENS_M030_1450)
    if(g_xEngineParam.iDemoMode == N_DEMO_FACTORY_MODE)
    {
#if (ENGINE_LENS_TYPE == ENGINE_LENS_40143)
        rIr2RGB_Rate = 1.05f * nWidth / g_xEngineParam.nDetectionHeight;
#elif (ENGINE_LENS_TYPE == ENGINE_LENS_M277_2409)
        rIr2RGB_Rate = 1.306f * nWidth / g_xEngineParam.nDetectionHeight;
#endif
    }
    else
    {
#if (ENGINE_LENS_TYPE == ENGINE_LENS_40143)
        rIr2RGB_Rate = 0.824f * nWidth / g_xEngineParam.nDetectionHeight;
#elif (ENGINE_LENS_TYPE == ENGINE_LENS_M277_2409)
        rIr2RGB_Rate = 1.0f * nWidth / g_xEngineParam.nDetectionHeight;
#endif
    }

#endif
    float rFaceCenterX, rFaceCenterY, rFaceWidth, rFaceHeight;
    float rClrFaceCenterX, rClrFaceCenterY, rClrFaceWidth, rClrFaceHeight;
    rFaceCenterX = g_xFaceProcessData->rFaceRect[0] + g_xFaceProcessData->rFaceRect[2] / 2;
    rFaceCenterY = g_xFaceProcessData->rFaceRect[1] + g_xFaceProcessData->rFaceRect[3] / 2;
    rFaceWidth = g_xFaceProcessData->rFaceRect[2];
    rFaceHeight = g_xFaceProcessData->rFaceRect[3];

    rClrFaceCenterX = (nHeight / 2) - (rFaceCenterX - g_xEngineParam.nDetectionWidth / 2) * rIr2RGB_Rate;
    rClrFaceCenterY = (nWidth / 2) + (rFaceCenterY - g_xEngineParam.nDetectionHeight / 2) * rIr2RGB_Rate;
    rClrFaceWidth = rFaceWidth * rIr2RGB_Rate;
    rClrFaceHeight = rFaceHeight * rIr2RGB_Rate;

    rFaceRectInClr[0] = rClrFaceCenterX - rClrFaceWidth / 2;
    rFaceRectInClr[1] = rClrFaceCenterY - rClrFaceHeight / 2;

    rFaceRectInClr[2] = rClrFaceWidth;
    rFaceRectInClr[3] = rClrFaceHeight;

    int nFaceCheck = checkFaceInClr_expand_shrink_DNN(pbYUV222, nHeight, nWidth, rFaceRectInClr);
    if(!nFaceCheck)
    {
        (getFaceProcessData())->nFaceDetected = 0;
        APP_LOG("pes 4-c\n");
        if(g_xEngineParam.fEngineState == ENS_REGISTER)
        {
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
            if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
            {
                fr_RevertEnrollStep_dnn();
            }
            else
            {

            }
#else//ENGINE_SECURITY_ONLY_COMMON
            fr_RevertEnrollStep_dnn();
#endif
        }


        return ES_PROCESS;
    }

    return ES_SUCCESS;
}

int nTempCode = -1;

int fr_ExtractFace()
{
    int nRet = ES_SUCCESS;
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
    {
        nRet = fr_ExtractFace_dnn();
    }
    else
    {
        nRet = fr_ExtractFace_h();
    }
#else//ENGINE_SECURITY_ONLY_COMMON
    nRet = fr_ExtractFace_dnn();
#endif

    if(nRet == ES_SUCCESS)
    {
        g_nNeedSmallFaceCheck = 0;
        int nLiveness = check2D_3DFake();

        if (g_nStopEngine == 1)
        {
            APP_LOG("[%d] stop by flag_stop\n", (int)Now());
            return ES_FAILED;
        }

        bool fCurRealState = 0;
        if(nLiveness == ES_SUCCESS)
        {
            g_nNeedSmallFaceCheck = 1;
            fCurRealState = 1;
        }
        else
        {
            g_nContinueRealCount = 0;
        }
        APP_LOG("[%d] pec 27 %d\n", (int)Now(), fCurRealState);
    }
    return nRet;
}

int	fr_VerifyFace()
{
    g_rExposureSettenTime = Now();

    float rStart = Now();
    float rPassTime;
    float rPassTimeThreshold = 70;

    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

    if (g_xEngineResult.fValid == 0 || g_xEngineResult.nOcclusion == 1 || g_nNeedSmallFaceCheck == 0)
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

    if (dbm_GetPersonCount() == 0 && g_xEngineParam.iDemoMode != N_DEMO_VERIFY_MODE_ON)
    {
        return ES_PROCESS;
    }

#ifdef ENGINE_FOR_DESSMAN
    if(g_nWaitingOpenEye)
    {
        int nEst = checkEyeOpenness();
        g_xEngineResult.nEyeOpenness = nEst;

        if (g_nStopEngine == 1)
        {
            APP_LOG("[%d] stop by flag_stop\n", (int)Now());
            return ES_FAILED;
        }

        return ES_PROCESS;
    }
#endif

#if (FAKE_DETECTION == 1)

    if(checkFaceIsInPhoto() == ES_FAILED && g_xEngineParam.iDemoMode != N_DEMO_FACTORY_MODE )
    {
        APP_LOG("[%d] pec 26-ps\n", (int)Now());
        g_nContinueRealCount = 0;
        return ES_PROCESS;
    }

#ifdef TimeProfiling
        float rTempStartTime = Now();
#endif

    g_nContinueRealCount++;

    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

#ifdef TimeProfiling
    my_printf("[%d] 2D 3D fake Time = %f\r\n", (int)Now(), Now() - rTempStartTime);
    my_printf("[%d] 2D 3D fake Time111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif


    int nRealFrameThreshold = USER_CONTINUE_FRAME;
    if ((dbm_GetPersonCount() == 0 || g_xEngineParam.iDemoMode == N_DEMO_VERIFY_MODE_ON) && getLivenessCheckStrong_On_NoUser())
    {
        nRealFrameThreshold = NO_USER_CONTINUE_FRAME;
    }

    APP_LOG("[%d] pec 29 %d %d\n", (int)Now(), *getContinueRealCount(), nRealFrameThreshold);

    if (g_nContinueRealCount < nRealFrameThreshold)
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
#endif//FAKE_DETECTION

#if (CHECK_FAKE_USER == 1)
    if (g_xEngineParam.iDemoMode == N_DEMO_VERIFY_MODE_ON)
    {
#ifdef ENGINE_FOR_DESSMAN

        int nEst = checkEyeOpenness();
        if(nEst)
            g_xEngineResult.nEyeOpenness = 1;
        else
            g_nWaitingOpenEye = 1;


        if (g_nStopEngine == 1)
        {
            APP_LOG("[%d] stop by flag_stop\n", (int)Now());
            return ES_FAILED;
        }

#endif
        APP_LOG("[%d] pec 31\n", (int)Now());

        return ES_SUCCESS;
    }

#endif

    if (g_nStopEngine == 1)
    {
        APP_LOG("[%d] stop by flag_stop\n", (int)Now());
        return ES_FAILED;
    }

#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
    {
        return fr_Retrieval_dnn();
    }
    else
    {
        return fr_Retrieval_h();
    }
#else//ENGINE_SECURITY_ONLY_COMMON
    return fr_Retrieval_dnn();
#endif

}

int	fr_RegisterFace(int iFaceDir)
{
    g_rExposureSettenTime = Now();
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
    {
        return fr_RegisterFace_dnn(iFaceDir);
    }
    else
    {
        return fr_RegisterFace_h(iFaceDir);
    }
#else//ENGINE_SECURITY_ONLY_COMMON
    return fr_RegisterFace_dnn(iFaceDir);
#endif
}

int	fr_GetRegisteredFeatInfo(PSFeatInfo pxFeatInfo)
{
    if(pxFeatInfo == NULL)
        return ES_FAILED;

#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
    {
        return fr_GetRegisteredFeatInfo_dnn((void*)pxFeatInfo);
    }
    else
    {
        return fr_GetRegisteredFeatInfo_h((void*)pxFeatInfo);
    }
#else//ENGINE_SECURITY_ONLY_COMMON
    return fr_GetRegisteredFeatInfo_dnn((void*)pxFeatInfo);
#endif

}


int     fr_CalcScreenValue(unsigned char *pbLedOnImage, int nMode)
{

    int nBufferWidth = g_xEngineParam.nDetectionHeight;
    int nBufferHeight = g_xEngineParam.nDetectionWidth;
    int nX, nY;
    int nCalcWindowWidth;
    if(nMode == IR_SCREEN_GETIMAGE_MODE)
    {
        nCalcWindowWidth = 40;
        int nCalcPointCount = 0;
        int nTotalValue = 0;

        for (nY = 0; nY < nBufferHeight; nY += nCalcWindowWidth)
        {
            for (nX = 240; nX < nBufferWidth - 240; nX += nCalcWindowWidth)
            {
                nTotalValue += pbLedOnImage[nY * nBufferWidth + nX];
                nCalcPointCount++;
            }
        }
        g_rAverageLedOnImage = 0;
        if(nCalcPointCount)
        {
            g_rAverageLedOnImage = nTotalValue / nCalcPointCount;
        }

        CalcNextExposure_ir_screen_inner(IR_SCREEN_GETIMAGE_MODE);
    }
    else
    {
        nCalcWindowWidth = 4;
        int nProcessArea = 0;
        memset(g_nHistInLEDOnImage, 0, sizeof(int) * 256);

        for (nY = 0; nY < nBufferHeight; nY += nCalcWindowWidth)
        {
            for (nX = 280; nX < nBufferWidth - 280; nX += nCalcWindowWidth)
            {
                int bLedOn = pbLedOnImage[nY * nBufferWidth + nX];
                g_nHistInLEDOnImage[bLedOn] ++;
                nProcessArea ++;
            }
        }

        float rSigmaSaturation = 0;
        int nValue;
        for(nValue = 250; nValue <= 255; nValue++)
        {
            rSigmaSaturation += g_nHistInLEDOnImage[nValue];
        }
        g_rSaturatedRate = rSigmaSaturation *  100.0f / nProcessArea;

        int nAvaiableSatuartionPixels = nProcessArea * 1 / 100;
        int nLevel;
        int nThresLevel = 0;
        int nSigma = 0;
        for (nLevel = 255; nLevel >= 0; nLevel--)
        {
            nSigma += g_nHistInLEDOnImage[nLevel];

            if (nSigma > nAvaiableSatuartionPixels)
            {
                nThresLevel = nLevel;
                break;
            }
        }

        nThresLevel++;
        g_nBrightUpThresholdLevel = nThresLevel;
        CalcNextExposure_ir_screen_inner(IR_SCREEN_CAMERAVIEW_MODE);
    }

    return ES_SUCCESS;
}


void extarctDNNFeature_process()
{
    waitDicChecksum(&g_thread_flag_feat);
    if(g_thread_flag_feat == 2 && getFeatEngineLoaded(0))
    {
        //float rO = Now();
		if (g_nStopEngine == 1)
		{
			APP_LOG("[%d] stop by flag_stop\n", (int)Now());
			return;
		}

        unsigned char *pLiveAlignFeat;
        pLiveAlignFeat = g_shared_mem + 128*128*2 + 88*128;
        //generateAlignImageForFeature(g_pbYIrImage, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, g_pbFaceDetectionBuffer, (getFaceProcessData())->rLandmarkPoint);

        int nRet = KdnnDetect_feat(pLiveAlignFeat, g_arLastDNNFeature, 0);
        if (nRet == KDNN_SUCCESS)
        {
            g_iDNNFeatureExtracted = 1;
        }
        else
        {
            memset(g_arLastDNNFeature, 0, sizeof(unsigned short) * KDNN_FEAT_SIZE);
        }
    }
}

int GetMaxDNNScore(unsigned short** ppDNNEnrolledFeaturte, int* pnDNNEnrolledFeatureCount, int nPersonCount, unsigned short* pVerifyFeature, float* prScore, int* pnIndex)//OK
{

    int nMaxScoreIndex = -1;
    float rMaxScore = 0;

    int nEnrollPersonIndex;
    int nEnrollFeatureIndex;

    for (nEnrollPersonIndex = 0; nEnrollPersonIndex < nPersonCount; nEnrollPersonIndex++)
    {
        unsigned short *pEnrolledFeaturte = ppDNNEnrolledFeaturte[nEnrollPersonIndex];

        for (nEnrollFeatureIndex = 0; nEnrollFeatureIndex < pnDNNEnrolledFeatureCount[nEnrollPersonIndex]; nEnrollFeatureIndex++)
        {
            float rScore = KdnnGetSimilarity(pEnrolledFeaturte + nEnrollFeatureIndex * KDNN_FEAT_SIZE, pVerifyFeature);

            if (rMaxScore < rScore)
            {
                rMaxScore = rScore;
                nMaxScoreIndex = nEnrollPersonIndex;
            }
        }
    }
    *prScore = rMaxScore;
    *pnIndex = nMaxScoreIndex;

    return 0;
}

int analyseBufferAndUpgradeCurEnvForCameraControl(unsigned char* pbLedOffBuffer, int nBufferWidth, int nBufferHeigt, int nCurExp, int nIsOffImage)
{
	int nTotalOffValue = 0;
	int nTotalPixelCount = 0;
	int nStep = 16;
	int nLedOffAverageValue = 0;

	unsigned char* pCurOffBuffer0 = pbLedOffBuffer;
	int nY, nX;

	for (nY = 0; nY < nBufferHeigt; nY += nStep)
	{
		unsigned char* pCurOffBuffer = pCurOffBuffer0;

		for (nX = 0; nX < nBufferWidth; nX += nStep)
		{
			int nCurOffValue = (int)(*pCurOffBuffer);

			nTotalOffValue += nCurOffValue;
			nTotalPixelCount++;
			pCurOffBuffer += nStep;
		}
		pCurOffBuffer0 += nStep * nBufferWidth;
	}

	nLedOffAverageValue = nTotalOffValue / nTotalPixelCount;
	int nAveValuePerExp = nLedOffAverageValue * 1000 / nCurExp;

    int nThresholdOutdoor = 150;
    int nThresholdSemiOutdoor = 50;
	if (!nIsOffImage)
	{
        nThresholdOutdoor = 150;
        nThresholdSemiOutdoor = 70;
	}

	if (nAveValuePerExp > nThresholdOutdoor)
	{
		g_nCurEnvForCameraControl = 2;
	}
	else if (nAveValuePerExp > nThresholdSemiOutdoor)
	{
		g_nCurEnvForCameraControl = 1;
	}
	else
	{
		g_nCurEnvForCameraControl = 0;
	}
#ifdef LOG_MODE
//    g_nCurEnvForCameraControl = 0;
#endif
	return 0;
}


int isFaceISGoodForDNN()//added 20180712
{
    if (g_rAverageLedOnImage < MIN_DNN_LUM)
    {
        return 0;
    }

    return 1;
}


int pose_estimate()
{
    getPose68(g_xFaceProcessData->rFaceRect[0], g_xFaceProcessData->rFaceRect[1], g_xFaceProcessData->rFaceRect[2],
            g_xFaceProcessData->rLandmarkPoint, 68, g_rAngleX, g_rAngleY, g_rAngleZ);

    return 0;
}

int isCorrectPose(int nFaceDirection)
{
    int nRet = pose_estimate();
    if(nRet)
    {
        return 0;
    }
	APP_LOG("[%d] pec 12 %d %f %f %f\n", (int)Now(), nFaceDirection, g_rAngleX, g_rAngleY, g_rAngleZ);
    if(nFaceDirection == ENROLL_DIRECTION_FRONT_CODE_0 || nFaceDirection == ENROLL_DIRECTION_FRONT_CODE_1)
    {
        if(g_rAngleX > FRONT_MIN_PITCH && g_rAngleX < FRONT_MAX_PITCH && g_rAngleY > FRONT_MIN_YAW && g_rAngleY < FRONT_MAX_YAW)
        {
            if(!g_nFrontFacePassed)
            {
                g_rTotalFrontAngleX = g_rAngleX;
                g_rTotalFrontAngleY = g_rAngleY;
                g_rTotalFrontAngleZ = g_rAngleZ;
                g_nFrontFacePassed =1;
            }
            else
            {
                g_rTotalFrontAngleX += g_rAngleX;
                g_rTotalFrontAngleY += g_rAngleY;
                g_rTotalFrontAngleZ += g_rAngleZ;
                g_nFrontFacePassed ++;
            }

            g_rFrontAngleX = g_rTotalFrontAngleX / g_nFrontFacePassed;
            g_rFrontAngleY = g_rTotalFrontAngleY / g_nFrontFacePassed;
            g_rFrontAngleZ = g_rTotalFrontAngleZ / g_nFrontFacePassed;
			APP_LOG("[%d] pec 13 %f %f %f\n", (int)Now(), g_rFrontAngleX, g_rFrontAngleY, g_rFrontAngleZ);
            g_nFrontFaceEnrolled = 1;
            return 1;
        }
    }
    else if(nFaceDirection == ENROLL_DIRECTION_UP_CODE)//up
    {
        if(g_nFrontFaceEnrolled)
        {
#if (ENROLL_ANGLE_MODE == 2)
            if((g_rFrontAngleX - g_rAngleX > PITCH_UP_DELTA) && g_rAngleX > MIN_PITCH && g_rAngleX < MAX_PITCH)
#else
            if((g_rAngleX < FRONT_MIN_PITCH_INNER || (g_rFrontAngleX - g_rAngleX > PITCH_UP_DELTA)) && g_rAngleX > MIN_PITCH && g_rAngleX < MAX_PITCH)
#endif
            {
				APP_LOG("[%d] pec 14\n", (int)Now());

                return 1;
            }
        }
        else
        {
            if(/*g_rAngleY > FRONT_MIN_YAW && g_rAngleY < FRONT_MAX_YAW &&*/ g_rAngleX < MAX_UP_PITCH &&
                    g_rAngleX > MIN_PITCH && g_rAngleX < MAX_PITCH)
            {
                return 1;
            }
        }
    }
    else if(nFaceDirection == ENROLL_DIRECTION_DOWN_CODE)//down
    {
        if(g_nFrontFaceEnrolled)
        {
#if (ENROLL_ANGLE_MODE == 2)
            if((g_rAngleX - g_rFrontAngleX > PITCH_DOWN_DELTA) && g_rAngleX > MIN_PITCH && g_rAngleX < MAX_PITCH)
#else
            if((g_rAngleX - g_rFrontAngleX > PITCH_DOWN_DELTA || g_rAngleX > FRONT_MAX_PITCH_INNER) &&
                g_rAngleX > MIN_PITCH && g_rAngleX < MAX_PITCH)
#endif
            {
				APP_LOG("[%d] pec 14-1\n", (int)Now());
                return 1;
            }
        }
        else
        {
            if(/*g_rAngleY > FRONT_MIN_YAW && g_rAngleY < FRONT_MAX_YAW && */ g_rAngleX > MIN_DOWN_PITCH &&
                    g_rAngleX > MIN_PITCH && g_rAngleX < MAX_PITCH)
            {
                return 1;
            }
        }
    }
    else if(nFaceDirection == ENROLL_DIRECTION_LEFT_CODE)//Left
    {
        if(g_nFrontFaceEnrolled)
        {
#if (ENROLL_ANGLE_MODE == 2)
            if((g_rFrontAngleY - g_rAngleY > YAW_LEFT_DELTA))
#else
            if((g_rFrontAngleY - g_rAngleY > YAW_LEFT_DELTA || g_rAngleY < FRONT_MIN_YAW_INNER)/* &&
                    g_rAngleY > MIN_YAW && g_rAngleY < MAX_YAW*/)
#endif
            {
				APP_LOG("[%d] pec 14-2\n", (int)Now());
                return 1;
            }

        }
        else
        {
            if(/*g_rAngleX > FRONT_MIN_PITCH && g_rAngleX < FRONT_MAX_PITCH &&*/ g_rAngleY < MAX_LEFT_YAW &&
                    g_rAngleY > MIN_YAW && g_rAngleY < MAX_YAW)
            {
                return 1;
            }

        }

    }
    else if(nFaceDirection == ENROLL_DIRECTION_RIGHT_CODE)//Right
    {
        if(g_nFrontFaceEnrolled)
        {
#if (ENROLL_ANGLE_MODE == 2)
            if(g_rAngleY - g_rFrontAngleY > YAW_RIGHT_DELTA)
#else
            if((g_rAngleY - g_rFrontAngleY > YAW_RIGHT_DELTA || g_rAngleY > FRONT_MAX_YAW_INNER /*|| (g_rFrontPan - g_xArmFaceProcessData.xFace.rPan) > PAN_DELTA*/) /*&&
                    g_rAngleY > MIN_YAW && g_rAngleY < MAX_YAW*/)
#endif
            {
				APP_LOG("[%d] pec 14-3\n", (int)Now());

                return 1;
            }
        }
        else
        {
            if(g_rAngleX > FRONT_MIN_PITCH && g_rAngleX < FRONT_MAX_PITCH && g_rAngleY > MIN_RIGHT_YAW &&
                    g_rAngleY > MIN_YAW && g_rAngleY < MAX_YAW)
            {
                return 1;
            }
        }
    }
	APP_LOG("[%d] pec 15\n", (int)Now());

    return 0;
}

int getEnvFromInfo(unsigned char nInfo)
{
    int nRet = 0;
    nRet = ((int)nInfo) & 1;
    return nRet;
}

#if (N_MAX_HAND_NUM)
extern int getMemSizeNeedToFeat_CheckValidHand();
#endif

#define DIC_MEM_ALIGN (64)

int AllocEngineMemory()
{
    if(g_global_memory)
    {
        return 0;
    }

    int g_global_dic_size = 0;
    int g_global_size = 0;
    int g_global_add_size = 0;

    int nNeedH_memory = 0;
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(g_nSecurityMode == SECURITY_LEVEL_TWIN)
    {
        nNeedH_memory = 1;
    }
#endif

    g_global_dic_size = ENNQ::get_blob_size(Feature::dnn_dic_size(),  DIC_MEM_ALIGN);
    //h engine
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(nNeedH_memory)
    {
        g_global_dic_size += ENNQ::get_blob_size(H_DICT_SIZE1,  DIC_MEM_ALIGN);
    }
#endif

    int g_global_tmp_size = 0;
    //calc Entire Memory size once

    g_global_tmp_size = N_D_ENGINE_SIZE * 2;
    int nMaxMemorySizeinPart = getDetectMenSize() + g_DNN_Detection_input_width * g_DNN_Detection_input_height + N_D_ENGINE_SIZE;
    if(g_global_tmp_size < nMaxMemorySizeinPart)
    {
        g_global_tmp_size = nMaxMemorySizeinPart;
    }

    nMaxMemorySizeinPart = N_D_ENGINE_SIZE;
    if(nMaxMemorySizeinPart < getDetectMenSize())
    {
        nMaxMemorySizeinPart = getDetectMenSize();
    }

    nMaxMemorySizeinPart += N_D_ENGINE_SIZE + CheckFaceBuffer_Width * CheckFaceBuffer_Height;
    nMaxMemorySizeinPart += (E_IMAGE_WIDTH / LEDOFFIMAGE_REDUCE_RATE) * (E_IMAGE_HEIGHT / LEDOFFIMAGE_REDUCE_RATE) * 2;
    if(g_global_tmp_size < nMaxMemorySizeinPart)
    {
        g_global_tmp_size = nMaxMemorySizeinPart;
    }
    nMaxMemorySizeinPart = Modeling_dnn_mem_size() + g_DNN_Modeling_input_width * g_DNN_Modeling_input_height + N_D_ENGINE_SIZE;
    if(g_global_tmp_size < nMaxMemorySizeinPart)
    {
        g_global_tmp_size = nMaxMemorySizeinPart;
    }

    nMaxMemorySizeinPart = LiveMnSE_dnn_mem_size();
//    if(nMaxMemorySizeinPart < LiveMn::dnn_mem_size())
//    {
//        nMaxMemorySizeinPart = LiveMn::dnn_mem_size();
//    }
    if(nMaxMemorySizeinPart < LiveMnSE3_dnn_mem_size())
    {
        nMaxMemorySizeinPart = LiveMnSE3_dnn_mem_size();
    }
    if(nMaxMemorySizeinPart < Feature::dnn_mem_size())
    {
        nMaxMemorySizeinPart = Feature::dnn_mem_size();
    }
    nMaxMemorySizeinPart += 128 * 128 * 3 + 88 * 128;
    if(g_global_tmp_size < nMaxMemorySizeinPart)
    {
        g_global_tmp_size = nMaxMemorySizeinPart;
    }
#if (N_MAX_HAND_NUM)
    nMaxMemorySizeinPart = HandFeat_dnn_mem_size();
    nMaxMemorySizeinPart += 128*128;
    if(g_global_tmp_size < nMaxMemorySizeinPart)
    {
        g_global_tmp_size = nMaxMemorySizeinPart;
    }
#endif

    /*
    int nDicSize[8];
    int nDicSizeCount = 6;

    nDicSize[0] = getDetectMenSize();
    nDicSize[1] = Modeling::dnn_mem_size();
    nDicSize[2] = LiveMnSE::dnn_mem_size();
    nDicSize[3] = LiveMn::dnn_mem_size();
    nDicSize[4] = Feature::dnn_mem_size();
    nDicSize[5] = LiveMnSE3::dnn_mem_size();

#ifdef ENGINE_FOR_DESSMAN
    nDicSizeCount = 8;
    nDicSize[6] = Occlusion::dnn_mem_size();
    nDicSize[7] = ESN::dnn_mem_size();
#endif

    int nDicIndex;
    for(nDicIndex = 0; nDicIndex < nDicSizeCount; nDicIndex++)
    {
        if(g_global_tmp_size < nDicSize[nDicIndex])
        {
            g_global_tmp_size = nDicSize[nDicIndex];
        }
    }
    */

    g_global_add_size =
            sizeof(int) * 256           //int           g_nHistInLEDOnImage[256];
            + sizeof(int) * 256          //int           g_nHistInLEDOnImage_temp[256];
            + sizeof(int) * N_MAX_PERSON_NUM  //int g_nIndexList[N_MAX_PERSON_NUM];
            + sizeof(int) * N_MAX_PERSON_NUM  //int g_nIndexList_Admin[N_MAX_PERSON_NUM];
            + sizeof(int) * N_MAX_PERSON_NUM  //int g_nIndexList_Dup[N_MAX_PERSON_NUM];
            + sizeof(int) * N_MAX_PERSON_NUM  //int g_nDNNFeatureCount_Dup[N_MAX_PERSON_NUM];
            + sizeof(int) * N_MAX_PERSON_NUM  //int g_nDNNFeatureCount[N_MAX_PERSON_NUM];
            + sizeof(int) * N_MAX_PERSON_NUM   //int g_nDNNFeatureCount_Admin[N_MAX_PERSON_NUM];
            + sizeof(unsigned short*) * N_MAX_PERSON_NUM    //float* g_prDNNEnrolledFeature_Dup[N_MAX_PERSON_NUM];
            + sizeof(unsigned short*) * N_MAX_PERSON_NUM    //float*          g_prDNNEnrolledFeature[N_MAX_PERSON_NUM];
            + sizeof(unsigned short*) * N_MAX_PERSON_NUM   //float* g_prDNNEnrolledFeature_Admin[N_MAX_PERSON_NUM];
            + sizeof(unsigned short) * KDNN_FEAT_SIZE      //float g_arLastDNNFeature[KDNN_FEAT_SIZE];
            + sizeof(Face_Process_Data)          //g_xFaceProcessData
            + sizeof(SFeatInfo);                //SFeatInfo       g_xEnrollFeatA

    //H engine buffer
    if(nNeedH_memory)
    {
        g_global_add_size += N_D_ENGINE_SIZE;
    }

    g_global_size = DIC_MEM_ALIGN + g_global_dic_size + g_global_tmp_size + g_global_add_size;
    g_global_memory = (unsigned char*)my_malloc(g_global_size);
    unsigned char* addr = (unsigned char*)(((size_t)(g_global_memory + (DIC_MEM_ALIGN - 1))) & (-DIC_MEM_ALIGN));

    g_dic_feature = addr;           addr += ENNQ::get_blob_size(Feature::dnn_dic_size(),    DIC_MEM_ALIGN);
    //H engine
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(nNeedH_memory)
    {
        g_dic_H_1 = addr;               addr += ENNQ::get_blob_size(H_DICT_SIZE1,    DIC_MEM_ALIGN);
    }
#endif//ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    g_shared_mem = addr;                        addr += g_global_tmp_size;
    g_nHistInLEDOnImage = (int*)addr;           addr += sizeof(int) * 256;
    g_nHistInLEDOnImage_temp = (int*)addr;      addr += sizeof(int) * 256;
    g_nIndexList = (int*)addr;                  addr += sizeof(int) * N_MAX_PERSON_NUM;
    g_nIndexList_Admin = (int*)addr;            addr += sizeof(int) * N_MAX_PERSON_NUM;//int g_nIndexList_Admin[N_MAX_PERSON_NUM];
    g_nIndexList_Dup = (int*)addr;              addr += sizeof(int) * N_MAX_PERSON_NUM;//int g_nIndexList_Dup[N_MAX_PERSON_NUM];
    g_nDNNFeatureCount_Dup = (int*)addr;        addr += sizeof(int) * N_MAX_PERSON_NUM;//int g_nDNNFeatureCount_Dup[N_MAX_PERSON_NUM];
    g_nDNNFeatureCount = (int*)addr;            addr += sizeof(int) * N_MAX_PERSON_NUM;//int g_nDNNFeatureCount[N_MAX_PERSON_NUM];
    g_nDNNFeatureCount_Admin = (int*)addr;      addr += sizeof(int) * N_MAX_PERSON_NUM;//int g_nDNNFeatureCount_Admin[N_MAX_PERSON_NUM];
    g_prDNNEnrolledFeature_Dup = (unsigned short**)addr; addr += sizeof(unsigned short*) * N_MAX_PERSON_NUM;//float* g_prDNNEnrolledFeature_Dup[N_MAX_PERSON_NUM];
    g_prDNNEnrolledFeature = (unsigned short**)addr;     addr += sizeof(unsigned short*) * N_MAX_PERSON_NUM;//float*          g_prDNNEnrolledFeature[N_MAX_PERSON_NUM];
    g_prDNNEnrolledFeature_Admin = (unsigned short**)addr; addr += sizeof(unsigned short*) * N_MAX_PERSON_NUM;//float* g_prDNNEnrolledFeature_Admin[N_MAX_PERSON_NUM];
    g_arLastDNNFeature = (unsigned short*)addr;           addr += sizeof(unsigned short) * KDNN_FEAT_SIZE;//float g_arLastDNNFeature[KDNN_FEAT_SIZE];
    g_xFaceProcessData = (Face_Process_Data*)addr;addr += sizeof(Face_Process_Data);//g_xFaceProcessData
    g_xEnrollFeatA = (SFeatInfo*)addr;             addr += sizeof(SFeatInfo);//SFeatInfo       g_xEnrollFeatA


    int nMaxOffset = 0;
    int nOffset = getDetectMenSize() + g_DNN_Detection_input_width * g_DNN_Detection_input_height;
    if(nMaxOffset < nOffset)
    {
        nMaxOffset = nOffset;
    }
    nOffset = Modeling_dnn_mem_size() + g_DNN_Modeling_input_width * g_DNN_Modeling_input_height;
    if(nMaxOffset < nOffset)
    {
        nMaxOffset = nOffset;
    }
    if(nMaxOffset < N_D_ENGINE_SIZE)
    {
        nMaxOffset = N_D_ENGINE_SIZE;
    }
    nOffset = getDetectMenSize();
    if(nOffset < N_D_ENGINE_SIZE)
    {
        nOffset = N_D_ENGINE_SIZE;
    }
    nOffset += CheckFaceBuffer_Width * CheckFaceBuffer_Height;
    g_pbOffIrImage = g_shared_mem + nOffset;
    nOffset += (E_IMAGE_WIDTH / LEDOFFIMAGE_REDUCE_RATE) * (E_IMAGE_HEIGHT / LEDOFFIMAGE_REDUCE_RATE);
    g_pbOffIrImage2 = g_shared_mem + nOffset;
    nOffset += (E_IMAGE_WIDTH / LEDOFFIMAGE_REDUCE_RATE) * (E_IMAGE_HEIGHT / LEDOFFIMAGE_REDUCE_RATE);
    if(nMaxOffset < nOffset)
    {
        nMaxOffset = nOffset;
    }
    g_pbYIrImage = g_shared_mem + nMaxOffset;


    //H engine buffer
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(nNeedH_memory)
    {
        g_pbDiffIrImage = addr; //addr += N_D_ENGINE_SIZE;
    }
#endif

    return 0;
}

int* fr_GetProcessArea()
{
    return &g_nProcessArea;
}

int* fr_GetAverageLedOnImage()
{
    return &g_rAverageLedOnImage;
}

float   fr_GetSaturatedRate()
{
    return g_rSaturatedRate;
}
int     fr_GetBrightUpThresholdLevel()
{
    return g_nBrightUpThresholdLevel;
}

int* fr_GetHistInLEDOnImage()
{
    return g_nHistInLEDOnImage;
}

SEngineResult* fr_GetEngineResult()
{
    return &g_xEngineResult;
}

SEngineParam* fr_GetEngineParam()
{
    return &g_xEngineParam;
}

unsigned char* fr_GetDiffIrImage()
{
    return g_pbYIrImage;
}

int		fr_GetCurEnvForCameraControl()
{
	return g_nCurEnvForCameraControl;
}

int* fr_GetExposure()
{
    return &g_exposure;
}

int*    fr_GetGain()
{
    return &g_nGain;
}

int*    fr_GetFineGain()
{
    return &g_nFineGain;
}

int*    fr_GetExposure2()
{
    return &g_exposure2;
}

int*    fr_GetGain2()
{
    return &g_nGain2;
}

int*    fr_GetFineGain2()
{
    return &g_nFineGain2;
}

int*    fr_GetExposure_bkup()
{
    return &g_exposure_bkup;
}
int*    fr_GetGain_bkup()
{
    return &g_nGain_bkup;
}
int*    fr_GetFineGain_bkup()
{
    return &g_nFineGain_bkup;
}
int*    fr_GetExposure2_bkup()
{
    return &g_exposure2_bkup;
}
int*    fr_GetGain2_bkup()
{
    return &g_nGain2_bkup;
}
int*    fr_GetFineGain2_bkup()
{
    return &g_nFineGain2_bkup;
}


int*    fr_MainControlCameraIndex()
{
    return &g_nMainControlCameraIndex;
}



int*    fr_GetExposurePrev()
{
    return &g_exposurePrev;
}

int*    fr_GetExposureImage()
{
    return &g_exposureImage;
}


void    fr_InitIRCamera_ExpGain()
{
    InitIRCamera_ExpGain();
}

void    fr_BackupIRCamera_ExpGain()
{
    g_exposure_bkup = g_exposure;
    g_nGain_bkup = g_nGain;
    g_nFineGain_bkup = g_nFineGain;
    g_exposure2_bkup = g_exposure2;
    g_nGain2_bkup = g_nGain2;
    g_nFineGain2_bkup = g_nFineGain2;

    *fr_GetBayerYConvertedCameraIndex() = -1;
    *fr_GetEntireImageAnalysed() = 0;
    g_nSecondImageNeedReCheck = 0;
    g_nNeedSmallFaceCheck = 0;

    *fr_GetFaceDetected() = 0;
    g_nNeedToCalcNextExposure = 0;


#ifdef TimeProfiling
    g_rStartTime = Now();
#endif
    //printf("fr_BackupIRCamera_ExpGain %d %d %d %d %d %d\n", g_exposure_bkup, g_nGain_bkup, g_nFineGain_bkup, g_exposure2_bkup, g_nGain2_bkup, g_nFineGain2_bkup);
}
int fr_GetNeedToCalcNextExposure()
{
	return g_nNeedToCalcNextExposure;
}

int		fr_SetLivenessCheckStrong_On_NoUser(int nMode)
{
	g_nLivenessCheckStrong_On_NoUser = nMode;
	return 0;
}

int     fr_GetNeedSmallFaceCheck()
{
    return g_nNeedSmallFaceCheck;
}
void fr_SetDecodedData(unsigned int iKey, unsigned char* pbData)
{
    if(pbData == NULL)
        return;

#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    if(g_nSecurityMode == SECURITY_LEVEL_TWIN)
    {
        waitDicChecksum(&g_thread_flag_H_1);
        SetDecodedData(iKey, pbData);
    }
#endif
}

void fr_SetDNNData()
{
#ifdef PROTECT_ENGINE
    char szSN[256] = { 0 };
    SHA1 psha;
    GetSN(szSN);
    psha.addBytes(szSN, strlen(szSN));
    unsigned char* pDig1 = (unsigned char*)psha.getDigest();

    SetKdnnFeatData(pDig1);
    my_free(pDig1);
#endif
}


int* fr_GetFaceDetected()
{
    return &g_nFaceDetected;
}

int* fr_GetFaceDetectedMode()
{
    return &g_nFaceDetectedMode;
}


void fr_SetStopEngineFlag(int f)
{
	g_nStopEngine = f;
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    setStopFlag_h(f);
#endif
	return;
}

void fr_SetMaxThresholdValue(float v)
{
    g_rSecurityValue = v;
}

void    fr_SetSecurityMode(int nSecurityMode)
{
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
    g_nSecurityMode = nSecurityMode;
#endif

}

Face_Process_Data* getFaceProcessData()
{
    return g_xFaceProcessData;
}

int* getDNNFeatureExtracted()
{
    return &g_iDNNFeatureExtracted;
}

unsigned short* getLastDNNFeature()
{
    return g_arLastDNNFeature;
}

int* getContinueRealCount()
{
    return &g_nContinueRealCount;
}

int getLivenessCheckStrong_On_NoUser()
{
    return g_nLivenessCheckStrong_On_NoUser;
}

int fr_getSuitableThreshold_dnn()
{
    if(g_EngineInitThrad)
    {
        my_thread_join(&g_EngineInitThrad);
        g_EngineInitThrad = 0;
    }
    if(g_thread_flag_feat != 2)
    {
        return -1;
    }
    return g_nDicCheckSum_FEAT - DNN_FEAT_CHECKSUM;
}

#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
int fr_getSuitableThreshold_h()
{
    if(g_EngineInitThrad)
    {
        my_thread_join(&g_EngineInitThrad);
        g_EngineInitThrad = 0;
    }
    if(g_thread_flag_H_1 != 2)
    {
        return -1;
    }
    return g_nDicCheckSum_H_1 - DNN_H_1_CHECKSUM;
}
#endif

int fr_getSuitableThreshold()
{
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
        if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
        {
            return fr_getSuitableThreshold_dnn();
        }
        else
        {
            return fr_getSuitableThreshold_h();
        }
#else
        return fr_getSuitableThreshold_dnn();
#endif
}

int fr_getHDicCheckSumMatched()
{
    return g_nHDicCheckSumMatched;
}
unsigned char* fr_GetInputImageBuffer1()
{
    return g_shared_mem;
}
unsigned char* fr_GetInputImageBuffer2()
{
    return g_shared_mem + N_D_ENGINE_SIZE;
}

unsigned char* fr_GetOffImageBuffer()
{
    return g_pbOffIrImage;
}

unsigned char* fr_GetOffImageBuffer2()
{
    return g_pbOffIrImage2;
}

unsigned char* fr_GetFullOffImageBuffer()
{
    return g_pbDiffIrImage;
}

void reset_FaceProcessVars()
{
    g_nBayerYConvertedCameraIndex = -1;
    g_nSecondImageNeedReCheck = 0;
}

int fr_GetSecondImageNeedReCheck()
{
    return g_nSecondImageNeedReCheck;
}
int     fr_GetSecondImageIsRight()
{
    return g_nSecondImageIsRight;
}
int*    fr_GetMainProcessCameraIndex()
{
    return &g_nMainProcessCameraIndex;
}

int* fr_GetBayerYConvertedCameraIndex()
{
    return &g_nBayerYConvertedCameraIndex;
}

int* fr_GetEntireImageAnalysed()
{
    return &g_nEntireImageAnalysed;
}

int fr_CheckFaceInSecondImage(unsigned char *pbBayerNeedReCheck)
{
    if(!g_nSecondImageNeedReCheck)
    {
        return ES_SUCCESS;
    }

    int nFaceCheck = 0;
    if(g_nSecondImageIsRight)
    {
#ifdef TimeProfiling
        my_printf("[%d] before checkFaceInCamera2 in second imageTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
        float rStartTime1 = Now();
#endif
        nFaceCheck = checkFaceInCamera2Image_expand_shrink_DNN(pbBayerNeedReCheck, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, (getFaceProcessData())->rFaceRect, 1);

#ifdef TimeProfiling
        my_printf("[%d] checkFaceInCamera2 in second imageTime  = %f\r\n", (int)Now(), Now() - rStartTime1);
        my_printf("[%d] checkFaceInCamera2 in second imageTime111 = %f\r\n", (int)Now(), Now() - g_rStartTime);
#endif

#ifdef LOG_MODE
        {
            char szImageFilePath[255];
            FILE* pFile;
            sprintf(szImageFilePath, "%s/%05d_%03d_recheck_r.bin", LOG_PATH, g_nLogIndex, g_nSubIndex);
            APP_LOG("%s\r\n", szImageFilePath);
            pFile = fopen(szImageFilePath, "wb");
            if (pFile)
            {
                fwrite(pbBayerRight, 1280 * 720, 1, pFile);
                fclose(pFile);
            }
        }
#endif

    }
    else
    {
        nFaceCheck = checkFaceInCamera2Image_expand_shrink_DNN(pbBayerNeedReCheck, g_xEngineParam.nDetectionWidth, g_xEngineParam.nDetectionHeight, (getFaceProcessData())->rFaceRect, 0);
#ifdef LOG_MODE
        {
            char szImageFilePath[255];
            FILE* pFile;
            sprintf(szImageFilePath, "%s/%05d_%03d_recheck_l.bin", LOG_PATH, g_nLogIndex, g_nSubIndex);
            APP_LOG("%s\r\n", szImageFilePath);
            pFile = fopen(szImageFilePath, "wb");
            if (pFile)
            {
                fwrite(pbBayerLeft, 1280 * 720, 1, pFile);
                fclose(pFile);
            }
        }
#endif
    }
    
    APP_LOG("pecc csF %d\n", nFaceCheck);
    if(!nFaceCheck)
    {
        //if in enroll mode, revert last enroll state
        if(g_xEngineParam.fEngineState == ENS_REGISTER)
        {
#if ENGINE_SECURITY_MODE == ENGINE_SECURITY_TWIN_COMMON
            if(g_nSecurityMode == SECURITY_LEVEL_COMMON)
            {
                fr_RevertEnrollStep_dnn();
            }
            else
            {

            }
#else//ENGINE_SECURITY_ONLY_COMMON
            fr_RevertEnrollStep_dnn();
#endif
        }

        return ES_PROCESS;
    }
    return ES_SUCCESS;
}

int		fr_Get_thread_flag_feat()
{
    return g_thread_flag_feat;
}
