
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
#include "ComboRetrievalSystem.h"

int g_nComboProcessMode = -1;//if 0:Face, 1:Hand
int     fr_PreExtractCombo(unsigned char *pbLedOnImage, int nProcessMode)
{
#if (N_MAX_HAND_NUM)
    g_nComboProcessMode = -1;

    if(g_xEngineParam.fEngineState == ENS_REGISTER)
    {
#if (ENROLL_FACE_HAND_MODE == ENROLL_FACE_HAND_MIX) //face hand enroll mix mode
        if(g_xEngineParam.iEnrollKind != EEK_Hand)
        {
            g_nComboProcessMode = -1;
        }
        else
        {
            g_nComboProcessMode = 1;
        }

#else//face hand enroll seperate mode
        if(g_xEngineParam.iEnrollKind == EEK_Face)
            g_nComboProcessMode = 0;
        else
            g_nComboProcessMode = 1;
#endif
    }
    else
    {
        g_nComboProcessMode = nProcessMode;
    }
    int nRet = ES_SUCCESS;
    if(g_nComboProcessMode == 0)
    {
        nRet = fr_PreExtractFace(0, pbLedOnImage);
    }
    else if(g_nComboProcessMode == 1)
    {
        reset_FaceProcessVars();
        nRet = fr_PreExtractHand(pbLedOnImage);

    }
    else
    {
        nRet = fr_PreExtractHand(pbLedOnImage);
        if(*fr_GetHandDetected())
        {
            g_nComboProcessMode = 1;
        }
        else
        {
            nRet = fr_PreExtractFace(0, pbLedOnImage);
            if(*fr_GetFaceDetected())
            {
                g_nComboProcessMode = 0;
            }
        }
    }
#else
    g_nComboProcessMode = 0;
    int nRet = fr_PreExtractFace(0, pbLedOnImage);
#endif // N_MAX_HAND_NUM


    return nRet;
}

int fr_PreExtractCombo2(unsigned char *pbBayerFromCamera2)
{
    int nRet = ES_FAILED;
    if(g_nComboProcessMode == 0)
    {
        nRet = fr_PreExtractFace2(pbBayerFromCamera2);
    }
    return nRet;
}

int     fr_ExtractCombo()
{
    int nRet = ES_FAILED;
    if(g_nComboProcessMode == 0)
    {
        nRet = fr_ExtractFace();
    }
#if (N_MAX_HAND_NUM)
    else if(g_nComboProcessMode == 1)
    {
        nRet = fr_ExtractHand();
    }
#endif // N_MAX_HAND_NUM
    return nRet;
}

SEngineResult* fr_GetEngineResultCombo(int* pnProcessMode)
{
    SEngineResult* nRet = fr_GetEngineResult();
    *pnProcessMode = 0;

#if (N_MAX_HAND_NUM)
    if(g_nComboProcessMode == 1)
    {
        nRet = fr_GetEngineResult_Hand();
        *pnProcessMode = 1;
    }
#endif // N_MAX_HAND_NUM
    return nRet;
}

int     fr_RegisterCombo(int iFaceDir, int* pnProcessMode)
{
    int nRet = ES_PROCESS;
    //my_printf("g_nComboProcessMode %d\n", g_nComboProcessMode);
#if (N_MAX_HAND_NUM)
    if(g_nComboProcessMode == 1)
    {
        *pnProcessMode = 1;
        nRet = fr_RegisterHand();
        if(nRet == ES_ENEXT || nRet == ES_SUCCESS)
        {
            g_xEngineParam.iEnrollKind = EEK_Hand;
        }
    }
    else
#endif // N_MAX_HAND_NUM
    {
        *pnProcessMode = 0;
        nRet = fr_RegisterFace(iFaceDir);
    }
    return nRet;
}

int     fr_VerifyCombo(int* pnProcessMode)
{
    int nRet = ES_PROCESS;
#if (N_MAX_HAND_NUM)
    if(g_nComboProcessMode == 1)
    {
        *pnProcessMode = 1;
        nRet = fr_VerifyHand_();
    }
    else
#endif // N_MAX_HAND_NUM
    {
        *pnProcessMode = 0;
        nRet = fr_VerifyFace();
    }
    return nRet;
}
