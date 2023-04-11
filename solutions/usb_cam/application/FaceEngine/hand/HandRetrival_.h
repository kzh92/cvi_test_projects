#ifndef _HANDRETRIVAL__H_
#define _HANDRETRIVAL__H_

//#include "hand.h"
#include "EngineStruct.h"

#if (N_MAX_HAND_NUM)

#define TOTAL_ENROLL_MAX_DNNFEATURE_HAND_COUNT  8

enum
{
    EH_SUCCESS    = 0,
    EH_NEXT       = 1,
    EH_RESTART    = 2,

    EH_FAILED     = 3,
    EH_DUPLICATED = 4,
};


struct  Hand_Process_Data
{
    float rHandRect[4];//left top width height
    float rLandmarkPoint[7*2];
    int nHandDetected;
    int nHandModelExtracted;
};

typedef struct _tagHAND_FEAT
{
    int             nDNNFeatCount;						//4
    unsigned char   arFeatBuffer[TOTAL_ENROLL_MAX_DNNFEATURE_HAND_COUNT * KDNN_FEAT_SIZE * sizeof(unsigned short)];       //10240
} HAND_FEAT;

typedef HAND_FEAT SHandFeatInfo;

extern SEngineResult g_xEngineResult_Hand;


#ifndef LIBFOO_DLL_EXPORTED
#define LIBFOO_DLL_EXPORTED __attribute__((__visibility__("default")))
#endif

#ifdef __cplusplus
extern	"C"
{
#endif

LIBFOO_DLL_EXPORTED int     createHandEngine_();
LIBFOO_DLL_EXPORTED void    fr_FreeEngine_Hand();
LIBFOO_DLL_EXPORTED int     setHandShareMem_(unsigned char* pbShareMem, unsigned char* pbShareYmem);
LIBFOO_DLL_EXPORTED int     fr_PreExtractHand(unsigned char *pbLedOnImage);
LIBFOO_DLL_EXPORTED int     fr_PreExtractHand2(unsigned char *pbLedOnImage);

LIBFOO_DLL_EXPORTED int		fr_ExtractHand();
LIBFOO_DLL_EXPORTED SEngineResult* fr_GetEngineResult_Hand();
LIBFOO_DLL_EXPORTED int     fr_VerifyHand_();
LIBFOO_DLL_EXPORTED int     fr_RegisterHand();

LIBFOO_DLL_EXPORTED int*    fr_GetHandDetected();
LIBFOO_DLL_EXPORTED int     fr_GetNeedCamera1_HandDetect();
LIBFOO_DLL_EXPORTED int     fr_GetRegisteredFeatInfo_Hand(SHandFeatInfo* pFeatInfo);

void    fr_SetEngineState_Hand(int fState, int nUpdateID);
void    fr_LoadFeatureForDuplicateCheck_Hand(int nUpdateID);


int*    fr_GetAverageLedOnImage_Hand();
float   fr_GetSaturatedRate_Hand();
int     fr_GetBrightUpThresholdLevel_Hand();


#ifdef __cplusplus
}
#endif

#endif // N_MAX_HAND_NUM

#endif//_HANDRETRIVAL_H_
