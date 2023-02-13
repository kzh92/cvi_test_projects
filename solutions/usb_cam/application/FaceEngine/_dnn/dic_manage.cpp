#include "dic_manage.h"

#include "detect.h"
#include "modeling.h"
#include "livemn.h"
#include "livemnse.h"
#include "livemnse3.h"
#include "occ.h"
#include "esn.h"
#include "feat.h"

#include "engine_inner_param.h"
#ifndef __RTK_OS__
#include <sys/mman.h>
#endif
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>

#include "DBManager.h"
#include "string.h"
#include "common_types.h"

#ifdef __RTK_OS__
#define FN_LIVEA1   FN_A1_DICT_PATH
#define FN_LIVEA2   FN_A2_DICT_PATH
#define FN_LIVEB    FN_B_DICT_PATH
#define FN_LIVEB2   FN_B2_DICT_PATH
#define FN_LIVEC    FN_C_DICT_PATH
#define FN_DETECT   FN_DETECT_DICT_PATH
#define FN_MODEL    FN_DLAMK_DICT_PATH
#define FN_FEATURE  FN_WNO_DICT_PATH
#define FN_ESN      FN_ESN_DICT_PATH
#define FN_OCC      FN_OCC_DICT_PATH
#define FN_H_1      "/mnt/hdic_1.bin"
#define FN_H_2      "/test/hdic_2.bin"
#else
#define FN_LIVEA1   FN_A1_DICT_PATH
#define FN_LIVEA2   FN_A2_DICT_PATH
#define FN_LIVEB    FN_B_DICT_PATH
#define FN_LIVEB2   FN_B2_DICT_PATH
#define FN_LIVEC    FN_C_DICT_PATH
#define FN_DETECT   FN_DETECT_DICT_PATH
#define FN_MODEL    FN_DLAMK_DICT_PATH
#define FN_FEATURE  FN_WNO_DICT_PATH
#define FN_ESN      FN_ESN_DICT_PATH
#define FN_OCC      FN_OCC_DICT_PATH
#define FN_H_1      "/test/hdic_1.bin"
#define FN_H_2      "/test/hdic_2.bin"
int g_id_detect = 0;
int g_id_model = 0;
int g_id_live_a1 = 0;
int g_id_live_a2 = 0;
int g_id_live_b = 0;
int g_id_live_b2 = 0;
int g_id_live_c = 0;
int g_id_occ = 0;
int g_id_esn = 0;
int g_id_H_2 = 0;
#endif

unsigned char*  g_dic_detect = 0;
unsigned char*  g_dic_model = 0;
unsigned char*  g_dic_live_a1 = 0;
unsigned char*  g_dic_live_a2 = 0;
unsigned char*  g_dic_live_b = 0;
unsigned char*  g_dic_live_b2 = 0;
unsigned char*  g_dic_live_c = 0;
unsigned char*  g_dic_occ = 0;
unsigned char*  g_dic_esn = 0;
unsigned char*  g_dic_feature = 0;
unsigned char*  g_dic_H_1 = 0;
unsigned char*  g_dic_H_2 = 0;

int g_nDicLoadedFlag = 0;
int g_nDicLoadedFromFileFlag = 0;

int g_nDicCheckSum_Calced_detector = 0;
int g_nDicCheckSum_Calced_modeling = 0;
int g_nDicCheckSum_Calced_spoof_A1 = 0;
int g_nDicCheckSum_Calced_spoof_A2 = 0;
int g_nDicCheckSum_Calced_spoof_B = 0;
int g_nDicCheckSum_Calced_spoof_B2 = 0;
int g_nDicCheckSum_Calced_spoof_3D = 0;
int g_nDicCheckSum_Calced_occ = 0;
int g_nDicCheckSum_Calced_esn = 0;
int g_nDicCheckSum_Calced_H_1 = 0;
int g_nDicCheckSum_Calced_H_2 = 0;

int g_nDicCheckSum_Checked_detector = 0;
int g_nDicCheckSum_Checked_modeling = 0;
int g_nDicCheckSum_Checked_spoof_A1 = 0;
int g_nDicCheckSum_Checked_spoof_A2 = 0;
int g_nDicCheckSum_Checked_spoof_B = 0;
int g_nDicCheckSum_Checked_spoof_B2 = 0;
int g_nDicCheckSum_Checked_spoof_3D = 0;
int g_nDicCheckSum_Checked_occ = 0;
int g_nDicCheckSum_Checked_esn = 0;
int g_nDicCheckSum_Checked_H_1 = 0;
int g_nDicCheckSum_Checked_H_2 = 0;

int g_nDicCheckSum_FEAT = 0;
int g_nDicCheckSum_H_1 = 0;

#define SHA_LEN 20
static unsigned char g_abEncData[SHA_LEN] = { 0 };

extern void APP_LOG(const char * format, ...);

#ifdef __RTK_OS__
#include "FaceRetrievalSystem.h"
#endif

int getBitInIndex(int nSrc, int nIndex)
{
    return ((nSrc >> nIndex) & 1);
}

int setBitInIndex(int &nSrcDst, int nIndex, int nValue)
{
    if(nValue)
    {
        nSrcDst |=  (1 << nIndex);
    }
    else
    {
        nSrcDst &= ~(1 << nIndex);
    }
    return 0;
}

void getDicInfos(int nMachineIndex, int** ppnFileIndicator, unsigned char*** pppbDicData, int* pnDicSize, char* pszFileName,
                 int* pnGTCheckSum = 0, int** ppnDicCheckSum_Calced = 0, int** ppnCheckSum_Checked = 0)
{
    int* pnFileIndicator = 0;
    unsigned char** ppbDicData = 0;
    int nDicSize = 0;
    char szFileNameTemp[255];
    int nGTCheckSum = 0;
    int* pnCheckSum_Checked = 0;
    int* pnDicCheckSum_Calced = 0;

    switch (nMachineIndex)
    {
    case MachineFlagIndex_DNN_Detect:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_detect;
#endif
        nDicSize = Detect::dnn_dic_size();
        strcpy(szFileNameTemp, FN_DETECT);
        ppbDicData = &g_dic_detect;
        nGTCheckSum = DNN_DETECT_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_detector;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_detector;
        break;
    }
    case MachineFlagIndex_DNN_Modeling:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_model;
#endif
        nDicSize = Modeling::dnn_dic_size();
        strcpy(szFileNameTemp, FN_MODEL);
        ppbDicData = &g_dic_model;
        nGTCheckSum = DNN_MODELING_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_modeling;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_modeling;
        break;
    }
    case MachineFlagIndex_DNN_Liveness_A1:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_live_a1;
#endif
        nDicSize = LiveMnSE::dnn_dic_size();
        strcpy(szFileNameTemp, FN_LIVEA1);
        ppbDicData = &g_dic_live_a1;
        nGTCheckSum = DNN_2D_LIVE_A1_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_spoof_A1;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_spoof_A1;
        break;
    }
    case MachineFlagIndex_DNN_Liveness_A2:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_live_a2;
#endif
        nDicSize = LiveMnSE::dnn_dic_size();
        strcpy(szFileNameTemp, FN_LIVEA2);
        ppbDicData = &g_dic_live_a2;
        nGTCheckSum = DNN_2D_LIVE_A2_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_spoof_A2;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_spoof_A2;
        break;
    }
    case MachineFlagIndex_DNN_Liveness_B:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_live_b;
#endif
        nDicSize = LiveMnSE::dnn_dic_size();
        strcpy(szFileNameTemp, FN_LIVEB);
        ppbDicData = &g_dic_live_b;
        nGTCheckSum = DNN_2D_LIVE_B_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_spoof_B;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_spoof_B;
        break;
    }
    case MachineFlagIndex_DNN_Liveness_B2:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_live_b2;
#endif
        nDicSize = LiveMnSE3::dnn_dic_size();
        strcpy(szFileNameTemp, FN_LIVEB2);
        ppbDicData = &g_dic_live_b2;
        nGTCheckSum = DNN_2D_LIVE_B2_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_spoof_B2;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_spoof_B2;
        break;
    }
    case MachineFlagIndex_DNN_Liveness_C:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_live_c;
#endif
        nDicSize = LiveMn::dnn_dic_size();
        strcpy(szFileNameTemp, FN_LIVEC);
        ppbDicData = &g_dic_live_c;
        nGTCheckSum = DNN_3D_LIVE_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_spoof_3D;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_spoof_3D;
        break;
    }
    case MachineFlagIndex_DNN_OCC:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_occ;
#endif
        nDicSize = Occlusion::dnn_dic_size();
        ppbDicData = &g_dic_occ;
        strcpy(szFileNameTemp, FN_OCC);
        nGTCheckSum = DNN_OCC_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_occ;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_occ;
        break;
    }
    case MachineFlagIndex_DNN_ESN:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_esn;
#endif
        nDicSize = ESN::dnn_dic_size();
        strcpy(szFileNameTemp, FN_ESN);
        ppbDicData = &g_dic_esn;
        nGTCheckSum = DNN_ESN_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_esn;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_esn;
        break;
    }
    case MachineFlagIndex_DNN_Feature:
    {
        nDicSize = Feature::dnn_dic_size();
        strcpy(szFileNameTemp, FN_FEATURE);
        ppbDicData = &g_dic_feature;
        break;
    }
    case MachineFlagIndex_H_1:
    {
        nDicSize = H_DICT_SIZE1;
        strcpy(szFileNameTemp, FN_H_1);
        ppbDicData = &g_dic_H_1;
        nGTCheckSum = DNN_H_1_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_H_1;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_H_1;
        break;
    }
    case MachineFlagIndex_H_2:
    {
#ifndef __RTK_OS__
        pnFileIndicator = &g_id_H_2;
#endif
        nDicSize = H_DICT_SIZE2;
        strcpy(szFileNameTemp, FN_H_2);
        ppbDicData = &g_dic_H_2;
        nGTCheckSum = DNN_H_2_CHECKSUM;
        pnDicCheckSum_Calced = &g_nDicCheckSum_Calced_H_2;
        pnCheckSum_Checked = &g_nDicCheckSum_Checked_H_2;
        break;
    }
    default:
        break;
    }

    if(ppnFileIndicator)
        *ppnFileIndicator = pnFileIndicator;
    if(pppbDicData)
        *pppbDicData = ppbDicData;
    if(pnDicSize)
        *pnDicSize = nDicSize;
    if(pszFileName)
        strncpy(pszFileName, szFileNameTemp, 255);
    if(pnGTCheckSum)
        *pnGTCheckSum = nGTCheckSum;
    if(ppnCheckSum_Checked)
        *ppnCheckSum_Checked = pnCheckSum_Checked;
    if(ppnDicCheckSum_Calced)
        *ppnDicCheckSum_Calced = pnDicCheckSum_Calced;

    return;
}


int loadMachineDic(int nMachineIndex)
{
    if(nMachineIndex < 0 || nMachineIndex >= MachineFlagIndex_MAX)
    {
        return 1;
    }
    if(getBitInIndex(g_nDicLoadedFlag, nMachineIndex))
    {
        return 0;
    }

    int nDicSize = 0;
    int* pFileIndicator = 0;
    char szDicFilePath[255];
    unsigned char** pDicDataBuffer = 0;
    int* pnDicCheckSum_Calced = 0;
    int nReadLength = 0;
    getDicInfos(nMachineIndex, &pFileIndicator, &pDicDataBuffer, &nDicSize, szDicFilePath, 0, &pnDicCheckSum_Calced);

    if(nMachineIndex != MachineFlagIndex_DNN_Feature && nMachineIndex != MachineFlagIndex_H_1)
    {
#ifdef __RTK_OS__
        nReadLength = fr_ReadFileData(szDicFilePath, 0, *pDicDataBuffer, nDicSize);
        if (nReadLength == nDicSize)
        {
            if(pnDicCheckSum_Calced)
            {
                *pnDicCheckSum_Calced = 0;
            }
            setBitInIndex(g_nDicLoadedFlag, nMachineIndex, 1);
        }
        else
        {
            APP_LOG("[%d] pecc 3-%d-2 %d\n", (int)Now(), nMachineIndex, nReadLength);
        }
#else
        int nDicFileLen = 0;
        unsigned char* pTempDic = (unsigned char*)getDictMem(szDicFilePath, &nDicFileLen);
        APP_LOG("[%d] pecc 3-%d:%p, %d\n",(int)Now(), nMachineIndex, pTempDic, nDicFileLen);
        if(pTempDic)
        {
            if(nDicFileLen == nDicSize)
            {
                *pDicDataBuffer = pTempDic;
                APP_LOG("[%d] pecc 3-%d-1\n", (int)Now(), nMachineIndex);
                setBitInIndex(g_nDicLoadedFlag, nMachineIndex, 1);
                //setBitInIndex(g_nDicCheckSum_Calced, nMachineIndex, 0);
                //setBitInIndex(g_nDicLoadedFromFileFlag, nMachineIndex, 1);
                if(pnDicCheckSum_Calced)
                {
                    *pnDicCheckSum_Calced = 0;
                }
//                if(nMachineIndex == MachineFlagIndex_DNN_Detect)
//                {
//                    g_nDicLoadedFromFileFlag = 1;
//                }

            }
            else
            {
                APP_LOG("[%d] pecc 3-%d-2 %d\n", (int)Now(), nMachineIndex, nDicFileLen);
            }
        }
        else
        {
            APP_LOG("[%d] pecc 3-%d-3\n", (int)Now(), nMachineIndex);
        }
#endif
    }
    else//dnn feat or H_1
    {
#ifdef __RTK_OS__
        nReadLength = fr_ReadFileData(szDicFilePath, 0, *pDicDataBuffer, nDicSize);
        if(nReadLength != nDicSize)
        {
            APP_LOG("[%d] pecc 3-%d-2 %d\n", (int)Now(), nMachineIndex, nReadLength);
        }
#else
        FILE* fp = fopen(szDicFilePath, "rb");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            nReadLength = ftell(fp);
            rewind(fp);

            if(nReadLength == nDicSize)
            {
                nReadLength = fread(*pDicDataBuffer, 1, nDicSize, fp);
                if(nReadLength != nDicSize)
                {
                    APP_LOG("[%d] pecc 3-%d-2x2 %d\n", (int)Now(), nMachineIndex, nReadLength);
                }
            }
            else
            {
                APP_LOG("[%d] pecc 3-%d-2x1 %d\n", (int)Now(), nMachineIndex, nReadLength);
            }
            fclose(fp);
        }
#endif
        if (nReadLength == nDicSize)
        {
            APP_LOG("[%d] pecc 3-%d-1\n", (int)Now(), nMachineIndex);
            unsigned char* pbData = *pDicDataBuffer;
#ifdef PROTECT_ENGINE
            for(int a = 0; a < nDicSize / SHA_LEN; a ++)
            {
                for(int b = 0; b < SHA_LEN; b ++)
                    pbData[a * SHA_LEN + b] = pbData[a * SHA_LEN + b] ^ g_abEncData[b];
            }
#endif

            int *pCheckSum = &g_nDicCheckSum_FEAT;
            if(nMachineIndex == MachineFlagIndex_H_1)
            {
                pCheckSum = &g_nDicCheckSum_H_1;
            }
            *pCheckSum = 0;
            int* pnData = (int*)pbData;
            for(int i = 0; i < nDicSize / 4; i ++)
                *pCheckSum ^= pnData[i];

            setBitInIndex(g_nDicLoadedFlag, nMachineIndex, 1);
            APP_LOG("[%d] pecc 2-%d-1 %x\n", (int)Now(), nMachineIndex, *pCheckSum);
        }
    }

    return 0;
}

int releaseMachineDic(int nMachineIndex)
{
    if(nMachineIndex < 0 || nMachineIndex >= MachineFlagIndex_MAX)
    {
        return 1;
    }
#ifndef __RTK_OS__
    if(nMachineIndex != MachineFlagIndex_DNN_Feature)
    {
        int* pFileIndicator = 0;
        int nDicSize = 0;
        unsigned char** pDicDataBuffer = 0;
        char szDicFilePath[255];
        getDicInfos(nMachineIndex, &pFileIndicator, &pDicDataBuffer, &nDicSize, szDicFilePath);

        //if (*pDicDataBuffer && ((nMachineIndex == MachineFlagIndex_DNN_Detect && g_nDicLoadedFromFileFlag) || (nMachineIndex != MachineFlagIndex_DNN_Detect)))
        if (*pDicDataBuffer)
        {
            APP_LOG("[%d] pecc 4-%d-2\n", (int)Now(), nMachineIndex);
            freeDictMem((void*)(*pDicDataBuffer), (const char*)szDicFilePath);
            //munmap(*pDicDataBuffer, nDicSize);
            *pDicDataBuffer = 0;
            //close(*pFileIndicator);
//            if(nMachineIndex == MachineFlagIndex_DNN_Detect)
//            {
//                g_nDicLoadedFromFileFlag = 0;
//            }
        }
        else
        {
            APP_LOG("[%d] pecc 4-%d-3\n", (int)Now(), nMachineIndex);
        }
    }

#endif
    setBitInIndex(g_nDicLoadedFlag, nMachineIndex, 0);

    return 0;
}

int getLoadedDicFlag(int nIndex)
{
    return getBitInIndex(g_nDicLoadedFlag, nIndex);
}
int setLoadedDicFlag(int nIndex, int nValue)
{
    setBitInIndex(g_nDicLoadedFlag, nIndex, nValue);
    return 0;
}

int getDicChecSumChecked(int nMachineIndex)
{
    if(nMachineIndex >= MachineFlagIndex_MAX || nMachineIndex < 0)
    {
        return 0;
    }

    if(nMachineIndex == MachineFlagIndex_H_1)
    {
        if(g_nDicCheckSum_H_1 == DNN_H_1_CHECKSUM)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    int nDicSize = 0;
    unsigned char** ppDicDataBuffer = 0;
    int nGTDicCheckSum = 0;
    int* pnDicCheckSum_Calced = 0;
    int* pnCheckSum_Checked = 0;

    getDicInfos(nMachineIndex, 0, &ppDicDataBuffer, &nDicSize, 0, &nGTDicCheckSum, &pnDicCheckSum_Calced, &pnCheckSum_Checked);
    if(!pnDicCheckSum_Calced || !pnCheckSum_Checked)
    {
        return 0;
    }

    if(*pnDicCheckSum_Calced)
    {
        return *pnCheckSum_Checked;
    }

    int iSum = 0;
    int i, nDicCheckSum;
    if(*ppDicDataBuffer)
    {
        int* pnData = (int*)(*ppDicDataBuffer);
        for (i = 0; i < nDicSize / 4; i ++)
            iSum ^= pnData[i];
    }
    APP_LOG("[%d] pecc 2-%d-1 %x\n", (int)Now(), nMachineIndex, iSum);

    if(iSum == nGTDicCheckSum)
    {
        APP_LOG("[%d] pecc 2-%d-2\n", (int)Now(), nMachineIndex);
        *pnCheckSum_Checked = 1;
        nDicCheckSum = 1;
    }
    else
    {
        nDicCheckSum = 0;
        APP_LOG("[%d] pecc 2-%d-3\n", (int)Now(), nMachineIndex);
    }
    *pnDicCheckSum_Calced = 1;

    return nDicCheckSum;
}


int SetKdnnFeatData(unsigned char* abData)
{
    memcpy(g_abEncData, abData, sizeof(g_abEncData));

    return 0;
}
