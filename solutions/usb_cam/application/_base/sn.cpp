#include "sn.h"
#include "settings.h"
#include "FaceRetrievalSystem.h"
#include "common_types.h"
#include "shared.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#ifndef __RTK_OS__
#include <sys/mman.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#endif // !__RTK_OS__

#define SUNXI_SID_BASE		(0x01c23800)

#ifdef FE_ENGINE_LIB

int fr_setSuitableThreshold(void* param, void* param1, void* param2)
{
    return seCheckThreshold(param, param1, param2);
}

int fr_resetThresholdState(void* param, void* param1, void* param2)
{
    COMMON_SETTINGS* pCS = (COMMON_SETTINGS*)param1;
    HEAD_INFO2 *pHD2 = (HEAD_INFO2*)param2;
    unsigned int sum = (unsigned int)fr_getSuitableThreshold();
    unsigned int iNewResetFlag = sum / DEFAULT_SECURE_STEP1;
    if (iNewResetFlag > 0)
        iNewResetFlag = DEFAULT_SECURE_STEP1 - 1;
    else
        iNewResetFlag = sum % DEFAULT_SECURE_STEP1;
    dbug_printf("#xx# 01, %d, %d\n", iNewResetFlag, sum);
    iNewResetFlag = (iNewResetFlag + (DEFAULT_SECURE_STEP1 - 1)) / DEFAULT_SECURE_STEP1 * DEFAULT_SECURE_STEP1;
    dbug_printf("#xx# 02, %d, %d\n", iNewResetFlag, pHD2->x.bIsResetFlag);
    if (pHD2->x.bIsResetFlag == 0 && iNewResetFlag != 0)
    {
        pHD2->x.bIsResetFlag = iNewResetFlag;
    }
    else if (pHD2->x.bIsResetFlag != DEFAULT_SECURE_STEP2 && iNewResetFlag == 0)
    {
        if (pCS->x.bSecureFlag != DEFAULT_SECURE_VALUE)
            pCS->x.bSecureFlag = DEFAULT_SECURE_VALUE;
        pHD2->x.bIsResetFlag = iNewResetFlag;
    }
    {
        int iOldFlag = pCS->x.bSecureFlag;
        //set invalid secure flag
        //condition is
        //(g_xHD2.x.bIsResetFlag == 15) AND
        //(ReadSystemRunningCount() % N_SECURE_CHK_CNT == 0) AND
        //(g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
        //then secure flag will be set different from inital value.
        int iNewFlag = DEFAULT_SECURE_FALSE_VAL * ((iNewResetFlag & 0xF) / DEFAULT_SECURE_STEP1);
        dbug_printf("#xx# 03, %d, %d\n", iNewFlag, iOldFlag);
        if (iOldFlag != iNewFlag && iNewFlag != 0)
        {
            pCS->x.bSecureFlag = iNewFlag;
        }
    }
    return 0;
}

int seCheckThreshold(void* param, void* param1, void* param2)
{
    HEAD_INFO *pHD = (HEAD_INFO*)param;
    COMMON_SETTINGS* pCS = (COMMON_SETTINGS*)param1;
    HEAD_INFO2* pHD2 = (HEAD_INFO2*)param2;
    unsigned char abChipID[8];
    unsigned int aiV3S_ID[4] = { 0 };
    unsigned int sum = 0;
    GetSSDID(aiV3S_ID);
#if 0
    //kkk test
    dbug_printf("aaa1: ");
    for (int j = 0; j < 16; j ++)
    {
        dbug_printf("%02x ", ((unsigned char*)aiV3S_ID)[j]);
    }
    dbug_printf("\n");
#endif
    memcpy(abChipID, aiV3S_ID, 8);
    for (int j = 0; j < 8; j ++)
    {
        abChipID[j] = ~(abChipID[j] ^ ((unsigned char*)aiV3S_ID)[j + 8]);
    }
#if 0
    //kkk test
    dbug_printf("aaa1-2: ");
    for (int j = 0; j < 8; j ++)
    {
        dbug_printf("%02x ", ((unsigned char*)abChipID)[j]);
    }
    dbug_printf("\n");
    dbug_printf("aaa1-3: ");
    for (int j = 0; j < 8; j ++)
    {
        dbug_printf("%02x ", ((unsigned char*)pHD->x.bChipID)[j]);
    }
    dbug_printf("\n");
#endif
    for (int j = 0; j < 8; j ++)
    {
        sum = sum + abs(pHD->x.bChipID[j] - abChipID[j]);
    }

    unsigned int iNewResetFlag = sum / DEFAULT_SECURE_STEP2;
    if (iNewResetFlag > 0)
        iNewResetFlag = DEFAULT_SECURE_STEP2 - 1;
    else
        iNewResetFlag = sum % DEFAULT_SECURE_STEP2;
    dbug_printf("--- 01, %d\n", iNewResetFlag);
    iNewResetFlag = (iNewResetFlag + (DEFAULT_SECURE_STEP2 - 1)) / DEFAULT_SECURE_STEP2 * DEFAULT_SECURE_STEP2;
    dbug_printf("--- 02, %d\n", iNewResetFlag);
    if (pHD2->x.bIsResetFlag == 0 && iNewResetFlag != 0)
    {
        pHD2->x.bIsResetFlag = iNewResetFlag;
    }
    else if (pHD2->x.bIsResetFlag != DEFAULT_SECURE_STEP1 && iNewResetFlag == 0)
    {
        if (pCS->x.bSecureFlag != DEFAULT_SECURE_VALUE)
            pCS->x.bSecureFlag = DEFAULT_SECURE_VALUE;
        pHD2->x.bIsResetFlag = iNewResetFlag;
    }
    {
        int iOldFlag = pCS->x.bSecureFlag;
        //set invalid secure flag
        //condition is
        //(g_xHD2.x.bIsResetFlag == 15) AND
        //(ReadSystemRunningCount() % N_SECURE_CHK_CNT == 0) AND
        //(g_xSS.iDemoMode != N_DEMO_FACTORY_MODE)
        //then secure flag will be set different from inital value.
        int iNewFlag = DEFAULT_SECURE_FALSE_VAL * ((pHD2->x.bIsResetFlag & 0xF) / DEFAULT_SECURE_STEP1) * (pHD2->x.iBootingCount % N_SECURE_CHK_CNT == 0);
        dbug_printf("xyz2, %d, %d\n", iNewFlag, pHD2->x.iBootingCount);
        if (iOldFlag != iNewFlag && iNewFlag != 0)
        {
            pCS->x.bSecureFlag = iNewFlag;
        }
    }
    return sum;
}

#endif // FE_ENGINE_LIB
