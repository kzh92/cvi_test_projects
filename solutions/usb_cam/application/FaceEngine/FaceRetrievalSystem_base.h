
#ifndef __FACE_RETRIEVAL_SYSTEM_BASE_H__
#define __FACE_RETRIEVAL_SYSTEM_BASE_H__

#include "EngineStruct.h"
#include "FaceRetrievalSystem.h"
#include "engineparam.h"
#include "DBManager.h"
#include "ImageProcessing.h"
#include "KDNN_EngineInterface.h"
#include "HEngine.h"
#include "gammacorrection.h"
//#include "dictionary.h"
#include "appdef.h"
#include "HAlign.h"
#include "manageEnvironment.h"
#include "DBManager.h"
#include "sha1.h"
#include "common_types.h"
#include "engineparam.h"
//#include "armCommon.h"
#include "convert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>

#include "Pose.h"
#include "UltraFace.hpp"
#include "esn_detection.h"
#include "occ_detection.h"
#include "convertBayer2Y_cm_riscv.h"
#include "modeling_interface.h"

struct  Face_Process_Data
{
    float rFaceRect[4];//left top width height
    float rLandmarkPoint[68*2];
    int nFaceDetected;
    int nFaceModelExtracted;
};


////////////////////////////////////////////      Enroll Direction Manage      ////////////////////////////////////////////////////
#ifdef ENGINE_FOR_DESSMAN
#define FRONT_MIN_PITCH         -18
#define FRONT_MIN_PITCH_INNER   -18
#define PITCH_UP_DELTA          4
#define PITCH_DOWN_DELTA        4

#undef ENROLL_ANGLE_MODE
#define ENROLL_ANGLE_MODE       0
#else//ENGINE_FOR_COMMON
/*
#define FRONT_MIN_PITCH         -24
#define FRONT_MIN_PITCH_INNER   -9
#define PITCH_UP_DELTA          2
#define PITCH_DOWN_DELTA        2
#endif

#define FRONT_MAX_PITCH         20
#define FRONT_MAX_PITCH_INNER   12
#define FRONT_MIN_YAW           -16
#define FRONT_MAX_YAW           16
#define FRONT_MIN_YAW_INNER		-12
#define FRONT_MAX_YAW_INNER		12

#define YAW_LEFT_DELTA        4
#define YAW_RIGHT_DELTA       4

#define MAX_PITCH           30
#define MIN_PITCH           -30
#define MAX_YAW             35
#define MIN_YAW             -35
#define MAX_UP_PITCH        -5
#define MIN_DOWN_PITCH      3
#define MAX_LEFT_YAW      -4
#define MIN_RIGHT_YAW      4
#define PAN_DELTA              10

#define ENROLL_MIN_YAW      -15
#define ENROLL_MAX_YAW      15
*/
//for vfl_1.10
#define FRONT_MIN_PITCH         -34
#define FRONT_MIN_PITCH_INNER   -10
#define PITCH_UP_DELTA          2
#define PITCH_DOWN_DELTA        2
#endif

#define FRONT_MAX_PITCH         23
#define FRONT_MAX_PITCH_INNER   13
#define FRONT_MIN_YAW           -17
#define FRONT_MAX_YAW           17
#define FRONT_MIN_YAW_INNER		-13
#define FRONT_MAX_YAW_INNER		13

#define YAW_LEFT_DELTA        4
#define YAW_RIGHT_DELTA       4

#define MAX_PITCH           35
#define MIN_PITCH           -41
#define MAX_YAW             41
#define MIN_YAW             -41
#define MAX_UP_PITCH        -5
#define MIN_DOWN_PITCH      3
#define MAX_LEFT_YAW      -4
#define MIN_RIGHT_YAW      4

#define ENROLL_MIN_YAW      -18
#define ENROLL_MAX_YAW      18

#if (ENROLL_ANGLE_MODE == 1)
#undef  FRONT_MIN_PITCH_INNER
#define FRONT_MIN_PITCH_INNER   -25
#undef  FRONT_MAX_PITCH_INNER
#define FRONT_MAX_PITCH_INNER   15
#undef  FRONT_MIN_YAW_INNER
#define FRONT_MIN_YAW_INNER		-17
#undef  FRONT_MAX_YAW_INNER
#define FRONT_MAX_YAW_INNER		17

#undef  PITCH_UP_DELTA
#define PITCH_UP_DELTA          5
#undef  PITCH_DOWN_DELTA
#define PITCH_DOWN_DELTA        7
#undef  YAW_LEFT_DELTA
#define YAW_LEFT_DELTA          7
#undef  YAW_RIGHT_DELTA
#define YAW_RIGHT_DELTA         7
#endif


#if (ENROLL_ANGLE_MODE == 2)
#undef FRONT_MIN_PITCH
#define FRONT_MIN_PITCH         -20
#undef  FRONT_MAX_PITCH
#define FRONT_MAX_PITCH         15
#undef  FRONT_MIN_YAW
#define FRONT_MIN_YAW           -13
#undef  FRONT_MAX_YAW
#define FRONT_MAX_YAW           13

#undef  PITCH_UP_DELTA
#define PITCH_UP_DELTA          5
#undef  PITCH_DOWN_DELTA
#define PITCH_DOWN_DELTA        7
#undef  YAW_LEFT_DELTA
#define YAW_LEFT_DELTA          7
#undef  YAW_RIGHT_DELTA
#define YAW_RIGHT_DELTA         7
#endif


#define ENROLL_DIRECTION_FRONT_CODE_0      0x00
#define ENROLL_DIRECTION_FRONT_CODE_1      0x01
#define ENROLL_DIRECTION_UP_CODE           0x10
#define ENROLL_DIRECTION_DOWN_CODE         0x08
#define ENROLL_DIRECTION_LEFT_CODE         0x04
#define ENROLL_DIRECTION_RIGHT_CODE        0x02

#define ENROLL_DIRECTION_COUNT              5
#define MAX_FakeFrameCountInOneDirection    0


#define			NO_USER_CONTINUE_FRAME	3
#define			USER_CONTINUE_FRAME	2

extern unsigned char*   g_pbYIrImage;
extern unsigned char*   g_pbDiffIrImage;
extern unsigned char*   g_pbOffIrImage;
extern unsigned char*   g_pbOffIrImage2;

//////////////////////////////////////////////   For DNN Memory Optimization  /////////////////////////////////////
extern unsigned char*  g_shared_mem;
extern unsigned char*  g_global_memory;

extern SFeatInfo* g_xEnrollFeatA;

extern mythread_ptr g_EngineInitThrad;

void FreeDetectionDict();
int LoadModelingDict();
void FreeModelingDict();
int DevMemInit();

int getBestIndex(FaceInfo* face_info, int n_face_cnt, int &nBadCode);

void CreateShrinkImage_normalize_FixRate_rotate(float* prDstImage, unsigned char* pbDstImage, int nDstWidth, int nDstHeight, float *prShrinkScale, unsigned char* pbSrcImage, int nSrcWidth, int nSrcHeight, int iCamFlip, float rMean, float rNorm);
void CreateShrinkImage_normalize_FixRate(float* prDstImage, unsigned char* pbDstImage, int nDstWidth, int nDstHeight, float *prShrinkScale, unsigned char* pbSrcImage, int nSrcWidth, int nSrcHeight, float rMean, float rNorm);
int isFaceOutOfImage(int* pnRect, int nImageWidth, int nImageHeight);
void calcAverageValues_rotate(unsigned char* pLEDOnImage, SRect rect);
void calcAverageValues(unsigned char* pLEDOnImage, unsigned char* pLEDOffImage, SRect rect);
int analyseBufferAndUpgradeCurEnvForCameraControl(unsigned char* pbLedOffBuffer, int nBufferWidth, int nBufferHeigt, int nCurExp, int nIsOffImage);
int buffer_rotate90_bayerConvert(unsigned char* pbSrcLandScapeBayerBuffer, unsigned char* pbDstBuffer, int nWidth, int nHeight, int* pnFaceRectInDst, int nForcedConvert = 0);
int checkFaceInCamera2Image_expand_shrink_DNN(unsigned char* pCamera2Buffer_8Byaer, int nBufferWidth, int nBufferHeight, float* prFaceRectInMainCamera, int nCameraisRight);
int isCorrectPose(int nEnrollStep);
int check2D_3DFake();
void extarctDNNFeature_process();
int* getDNNFeatureExtracted();
unsigned short* getLastDNNFeature();
int* getContinueRealCount();
int getLivenessCheckStrong_On_NoUser();
int pose_estimate();
void waitDicChecksum(int* pThread);

int getEnvFromInfo(unsigned char nInfo);
int*    fr_GetExposurePrev();
int*    fr_GetExposureImage();
int*    fr_GetMainProcessCameraIndex();



int GetMaxDNNScore(unsigned short** ppDNNEnrolledFeaturte, int* pnDNNEnrolledFeatureCount, int nPersonCount, unsigned short* pVerifyFeature, float* prScore, int* pnIndex);
int isGoodFaceToEnroll();//mode 0:enroll, 1:verify
int isFaceISGoodForDNN();//added 20180712
int checkEyeOpenness();

int AllocEngineMemory();
float getGainRateFromGain_SC2355(int nCoarseGain, int nFineGain);
int checkFaceIsInPhoto();
int* fr_GetBayerYConvertedCameraIndex();
int* fr_GetEntireImageAnalysed();
void reset_FaceProcessVars();
Face_Process_Data* getFaceProcessData();

//////////////////////////////////////////        DNN Face Detection              /////////////////////////////////////////////////////////////////////
//extern UltraFace m_DnnFace;
#define CheckFaceBuffer_Width       80
#define CheckFaceBuffer_Height      80

#define CheckFaceBufferClr_Width       56
#define CheckFaceBufferClr_Height      56

extern unsigned char*   g_pbFaceDetectionBuffer;
//extern  float g_rLandmarkPoint[68*2];

#if (ENGINE_LENS_TYPE == ENGINE_LENS_40143)
#define FACE_MIN_WIDTH_AVAIABLE         160
#define FACE_MIN_WIDTH_AVAIABLE_ENROLL  200
#define FACE_MAX_WIDTH_AVAIABLE         600

#define HAND_MIN_WIDTH_AVAIABLE         400
#define HAND_MIN_WIDTH_AVAIABLE_ENROLL  400
#define HAND_MAX_WIDTH_AVAIABLE         800

#elif (ENGINE_LENS_TYPE == ENGINE_LENS_M277_2409)
#define FACE_MIN_WIDTH_AVAIABLE         130
#define FACE_MIN_WIDTH_AVAIABLE_ENROLL  160
#define FACE_MAX_WIDTH_AVAIABLE         492

#define HAND_MIN_WIDTH_AVAIABLE         400
#define HAND_MIN_WIDTH_AVAIABLE_ENROLL  400
#define HAND_MAX_WIDTH_AVAIABLE         800
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////		Enroll Liveness Detection			////////////////////////////////////////
#define ENROLL_LIVENESS_MIN_AVERAGE	40
extern int g_nEnrolledFrameCountUnderMin;
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////        Check EyeState and Occlusion        //////////////////////////////////////////
extern int g_nWaitingOpenEye;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////         Thread          ////////////////////////////////////////////////
extern int g_nThreadCount;

#define ENV_COUNT					2

extern int g_nStopEngine;
extern int g_nNeedToCalcNextExposure;
extern int g_nNeedDelayForCameraControl;
extern int g_nPassedDirectionCount;

extern int g_rAverageLedOffImage;
extern int g_rAverageDiffImage;
extern int g_rAverageLedOnImage;
extern int g_rAverageLedOnImage_Camera1;
extern float g_rFaceRect_Camera1[4];
extern int* g_nHistInLEDOnImage;
extern int* g_nHistInLEDOnImage_temp;
extern int g_nFirstImageFaceAvailable;


extern int g_nProcessArea;
extern float g_rSaturatedRate;
extern int nTempCode;

extern int g_fRealState;
extern int g_nFakeFrameCountInOneDirection;
extern int g_nFakeFrameCountInOneDirection_Prev;
extern int g_nPassedDirectionCount;
extern int g_nEnrollDirectionMode;
extern int g_nEnrollDirectionMax;
extern float g_rExposureSettenTime;
extern int g_nCurEnvForCameraControlSettenFromOffImage;
extern int g_nEnrollFeatureAdded;
//extern int g_iDicCashed;

extern int                 g_thread_flag_detect;
extern int                 g_thread_flag_detect_h;
extern int                 g_thread_flag_model;
extern int                 g_thread_flag_spoofa1;
extern int                 g_thread_flag_spoofa2;
extern int                 g_thread_flag_spoofb;
extern int                  g_thread_flag_spoofb2;
extern int                 g_thread_flag_spoofc;
#ifdef ENGINE_FOR_DESSMAN
extern int                 g_thread_flag_esn;
extern int                 g_thread_flag_occ;
#endif // ENGINE_FOR_DESSMAN
extern int                  g_thread_flag_feat;
extern int                  g_thread_flag_H_1;
extern int                  g_thread_flag_H_2;


int*    fr_GetExposure_bkup();
int*    fr_GetGain_bkup();
int*    fr_GetFineGain_bkup();
int*    fr_GetExposure2_bkup();
int*    fr_GetGain2_bkup();
int*    fr_GetFineGain2_bkup();


void APP_LOG(const char * format, ...);
#define IF_FLAG_STOP1(ret) \
    do { \
        if (g_nStopEngine) \
        { \
            APP_LOG("[%d] stop by flag_stop\n", (int)Now()); \
            return (ret); \
        } \
    } while(0)

#define IF_FLAG_STOP1_NOPARAM() \
    do { \
        if (g_nStopEngine) \
        { \
            APP_LOG("[%d] stop by flag_stop\n", (int)Now()); \
            return; \
        } \
    } while(0)

//#define TimeProfiling
#ifdef TimeProfiling
extern float g_rStartTime;
extern float g_rStartTimeCur;
void showTotalTimeProfileInfo();
void setTimeProfilingInfo(int nIndex);
void initTimeProfiling();
#endif

//#define LOG_MODE
#ifdef LOG_MODE
extern int g_nLogIndex;
extern int g_nSubIndex;
extern float g_rLastTime;
extern float g_rFrameCaptureTime;
#define LOG_PATH	"/db1"
int ReadSuccessSendCount1();
void IncreaseSuccessSendCount1();
#endif

#endif//__FACE_RETRIEVAL_SYSTEM_BASE_H__

