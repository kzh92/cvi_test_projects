#include "countbase.h"
#include "settings.h"
#include "DBManager.h"
#include "appdef.h"

#include <stdio.h>

#if 0
#define DB_DIR  "/db"

int g_nFaceFailedCount = -1;
DATETIME_32 g_tLastFaceFailedTime = {0};
int g_nPasscodeFailedCount = -1;
DATETIME_32 g_tLastPassscodeFailedTime = {0};
int g_nHandFailed5Count = -1;
DATETIME_32 g_tHandFaceFailedTime = {0};
int g_nRequestCount = -1;
DATETIME_32 g_tRequestTime = {0};

int ReadSystemRunningCount()
{
    FILE* fp = fopen("/db/booting_count.ini", "rb");
    if(fp == NULL)
        return 0;

    int ret = 0;
    fread(&ret, sizeof(int), 1, fp);
    fclose(fp);

    if(ret < 0)
        ret = 1;

    return ret;
}

void IncreaseSystemRunningCount()
{
    int runningCount = ReadSystemRunningCount();
    runningCount ++;

    FILE* fp = fopen("/db/booting_count.ini", "wb");
    if(fp)
    {
        fwrite(&runningCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int ReadSystemRunningCount1()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/booting_count1.ini", DB_DIR);

    FILE* fp = fopen(szPath, "rb");
    if(fp == NULL)
        return 0;

    int ret = 0;
    fread(&ret, sizeof(int), 1, fp);
    fclose(fp);

    return ret;
}

void IncreaseSystemRunningCount1()
{
    int runningCount = ReadSystemRunningCount1();
    runningCount ++;

    char szPath[256] = { 0 };
    sprintf(szPath, "%s/booting_count1.ini", DB_DIR);

    FILE* fp = fopen(szPath, "wb");
    if(fp)
    {
        fwrite(&runningCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int ReadClrCamErrorCount()
{
    FILE* fp = fopen("/db/error_clr.ini", "rb");
    if(fp == NULL)
        return 0;

    int ret = 0;
    fread(&ret, sizeof(int), 1, fp);
    fclose(fp);

    if(ret < 0)
        return 0;

    return ret;
}

void IncClrCamErrorCount()
{
    int runningCount = ReadClrCamErrorCount();
    runningCount ++;

    FILE* fp = fopen("/db/error_clr.ini", "wb");
    if(fp)
    {
        fwrite(&runningCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}
int ReadIRCamErrorCount()
{
    FILE* fp = fopen("/db/error_ir.ini", "rb");
    if(fp == NULL)
        return 0;

    int ret = 0;
    fread(&ret, sizeof(int), 1, fp);
    fclose(fp);

    if(ret < 0)
        return 0;

    return ret;
}

void IncIRCamErrorCount()
{
    int runningCount = ReadIRCamErrorCount();
    runningCount ++;

    FILE* fp = fopen("/db/error_ir.ini", "wb");
    if(fp)
    {
        fwrite(&runningCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
//        sync();
    }
}

int ReadSuccessSendCount()
{
    FILE* fp = fopen("/db/cur_success.ini", "rb");
    if(fp == NULL)
        return 0;

    int ret = 0;
    fread(&ret, sizeof(int), 1, fp);
    fclose(fp);

    if(ret < 0)
        return 0;

    return ret;
}

void IncreaseSuccessSendCount()
{
    int runningCount = ReadSuccessSendCount();
    runningCount ++;

    FILE* fp = fopen("/db/cur_success.ini", "wb");
    if(fp)
    {
        fwrite(&runningCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int ReadFailedSendCount()
{
    FILE* fp = fopen("/db/cur_failed.ini", "rb");
    if(fp == NULL)
        return 0;

    int ret = 0;
    fread(&ret, sizeof(int), 1, fp);
    fclose(fp);

    if(ret < 0)
        return 0;

    return ret;
}

void IncreaseFailedSendCount()
{
    int runningCount = ReadFailedSendCount();
    runningCount ++;

    FILE* fp = fopen("/db/cur_failed.ini", "wb");
    if(fp)
    {
        fwrite(&runningCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}


void IncreaseFaceFailedCount()
{
    DATETIME_32 lastTime = GetLastFaceFailedTime();
    DATETIME_32 curTime = dbm_GetCurDateTime();

    int diffSec = dbm_GetDiffSec(curTime, lastTime);
    if(diffSec < 0 || diffSec > 5 * 60)
        ResetFaceFailedCount();

    SetLastFaceFailedTime();

    if(g_nFaceFailedCount < 0)
        ReadFaceFailedCount();

    g_nFaceFailedCount ++;

    char szPath[256] = { 0 };
    sprintf(szPath, "%s/face_failed.ini", DB_DIR);

    FILE* fp = fopen(szPath, "wb");
    if(fp)
    {
        fwrite(&g_nFaceFailedCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int ReadFaceFailedCount()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/face_failed.ini", DB_DIR);

    if(g_nFaceFailedCount < 0)
    {
        FILE* fp = fopen(szPath, "rb");
        if(fp)
        {
            fread(&g_nFaceFailedCount, sizeof(int), 1, fp);
            fclose(fp);
        }

        if(g_nFaceFailedCount < 0)
            g_nFaceFailedCount = 0;
    }

    return g_nFaceFailedCount;
}

void ResetFaceFailedCount()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/face_failed.ini", DB_DIR);

    g_nFaceFailedCount = 0;

    FILE* fp = fopen(szPath, "wb");
    if(fp)
    {
        fwrite(&g_nFaceFailedCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

void SetLastFaceFailedTime()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/last_face_failed.ini", DB_DIR);

    g_tLastFaceFailedTime = dbm_GetCurDateTime();

    FILE* fp = fopen(szPath, "wb");
    if(fp)
    {
        fwrite(&g_tLastFaceFailedTime, sizeof(g_tLastFaceFailedTime), 1 ,fp);
        fflush(fp);
        fclose(fp);
    }
}

DATETIME_32 GetLastFaceFailedTime()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/last_face_failed.ini", DB_DIR);

    FILE* fp = fopen(szPath, "rb");
    if(fp)
    {
        fread(&g_tLastFaceFailedTime, sizeof(g_tLastFaceFailedTime), 1, fp);
        fclose(fp);
    }

    return g_tLastFaceFailedTime;
}


void IncreaseHandFailedCount()
{
    DATETIME_32 xLastTime = GetLastHandFailedTime();
    DATETIME_32 xCurTime = dbm_GetCurDateTime();

    int diffSec = dbm_GetDiffSec(xCurTime, xLastTime);
    if(diffSec < 0 || diffSec > 5 * 60)
        ResetHandFailedCount();

    SetLastHandFailedTime();

    if(g_nHandFailed5Count < 0)
        ReadHandFailedCount();

    g_nHandFailed5Count ++;

    FILE* fp = fopen("/db/hand_failed.ini", "wb");
    if(fp)
    {
        fwrite(&g_nHandFailed5Count, sizeof(g_nHandFailed5Count), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int ReadHandFailedCount()
{
    if(g_nHandFailed5Count < 0)
    {
        FILE* fp = fopen("/db/hand_failed.ini", "rb");
        if(fp)
        {
            fread(&g_nHandFailed5Count, sizeof(int), 1, fp);
            fclose(fp);
        }

        if(g_nHandFailed5Count < 0)
            g_nHandFailed5Count = 0;
    }

    return g_nHandFailed5Count;
}

void ResetHandFailedCount()
{
    g_nHandFailed5Count = 0;
    FILE* fp = fopen("/db/hand_failed.ini", "wb");
    if(fp)
    {
        fwrite(&g_nHandFailed5Count, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

void SetLastHandFailedTime()
{
    g_tHandFaceFailedTime = dbm_GetCurDateTime();
    FILE* fp = fopen("/db/last_hand_failed.ini", "wb");
    if(fp)
    {
        fwrite(&g_tHandFaceFailedTime, sizeof(g_tHandFaceFailedTime), 1 ,fp);
        fflush(fp);
        fclose(fp);
    }
}

/**
 * @brief GetLastHandFailedTime
 * @return
 */
DATETIME_32 GetLastHandFailedTime()
{
    FILE* fp = fopen("/db/last_hand_failed.ini", "rb");
    if(fp)
    {
        fread(&g_tHandFaceFailedTime, sizeof(g_tHandFaceFailedTime), 1, fp);
        fclose(fp);
    }

    return g_tHandFaceFailedTime;
}


void IncreasePasscodeFailedCount()
{
    DATETIME_32 lastTime = GetLastPasscodeFailedTime();
    DATETIME_32 curTime = dbm_GetCurDateTime();

    int diffSec = dbm_GetDiffSec(curTime, lastTime);
    if(diffSec < 0 || diffSec > 5 * 60)
        ResetPasscodeFailedCount();

    SetLastPasscodeFailedTime();

    if(g_nPasscodeFailedCount < 0)
        ReadPasscodeFailedCount();

    g_nPasscodeFailedCount ++;

    char szPath[256] = { 0 };
    sprintf(szPath, "%s/passcode_failed.ini", DB_DIR);

    FILE* fp = fopen(szPath, "wb");
    if(fp)
    {
        fwrite(&g_nPasscodeFailedCount, sizeof(g_nPasscodeFailedCount), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int ReadPasscodeFailedCount()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/passcode_failed.ini", DB_DIR);

    if(g_nPasscodeFailedCount < 0)
    {
        FILE* fp = fopen(szPath, "rb");
        if(fp)
        {
            fread(&g_nPasscodeFailedCount, sizeof(g_nPasscodeFailedCount), 1, fp);
            fclose(fp);
        }

        if(g_nPasscodeFailedCount < 0)
            g_nPasscodeFailedCount = 0;
    }

    return g_nPasscodeFailedCount;
}

void ResetPasscodeFailedCount()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/passcode_failed.ini", DB_DIR);

    g_nPasscodeFailedCount = 0;

    FILE* fp = fopen(szPath, "wb");
    if(fp)
    {
        fwrite(&g_nPasscodeFailedCount, sizeof(g_nPasscodeFailedCount), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

void SetLastPasscodeFailedTime()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/last_passcode_failed.ini", DB_DIR);

    g_tLastPassscodeFailedTime = dbm_GetCurDateTime();

    FILE* fp = fopen(szPath, "wb");
    if(fp)
    {
        fwrite(&g_tLastPassscodeFailedTime, sizeof(g_tLastPassscodeFailedTime), 1 ,fp);
        fflush(fp);
        fclose(fp);
    }
}

DATETIME_32 GetLastPasscodeFailedTime()
{
    char szPath[256] = { 0 };
    sprintf(szPath, "%s/last_passcode_failed.ini", DB_DIR);

    FILE* fp = fopen(szPath, "rb");
    if(fp)
    {
        fread(&g_tLastPassscodeFailedTime, sizeof(g_tLastPassscodeFailedTime), 1, fp);
        fclose(fp);
    }

    return g_tLastPassscodeFailedTime;
}

int CheckPasscodeError()
{
    int iRemainedTime = 0;
    if(ReadPasscodeFailedCount() >= N_MAX_PASS_FAILED_COUNT)
    {
        DATETIME_32 xLastTime = GetLastPasscodeFailedTime();
        DATETIME_32 xCurTime = dbm_GetCurDateTime();

        int rDiffSec = dbm_GetDiffSec(xCurTime, xLastTime);
        if(rDiffSec < 0 || rDiffSec > PASSCODE_NO_ENTER_TIMEOUT)
            ResetPasscodeFailedCount();
        else
            iRemainedTime = PASSCODE_NO_ENTER_TIMEOUT - rDiffSec;
    }

    return iRemainedTime;
}

void IncreaseRequestCount()
{
    DATETIME_32 lastTime = GetLastRequestTime();
    DATETIME_32 curTime = dbm_GetCurDateTime();

    int diffSec = dbm_GetDiffSec(curTime, lastTime);
    if(diffSec < 0 || diffSec > 5 * 60)
        ResetRequestCount();

    SetLastRequestTime();

    if(g_nRequestCount < 0)
        ReadRequestCount();

    g_nRequestCount ++;

    FILE* fp = fopen("/db/request.ini", "wb");
    if(fp)
    {
        fwrite(&g_nRequestCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

int ReadRequestCount()
{
    if(g_nRequestCount < 0)
    {
        FILE* fp = fopen("/db/request.ini", "rb");
        if(fp)
        {
            fread(&g_nRequestCount, sizeof(int), 1, fp);
            fclose(fp);
        }

        if(g_nRequestCount < 0)
            g_nRequestCount = 0;
    }

    return g_nRequestCount;
}

void ResetRequestCount()
{
    g_nRequestCount = 0;
    FILE* fp = fopen("/db/request.ini", "wb");
    if(fp)
    {
        fwrite(&g_nRequestCount, sizeof(int), 1, fp);
        fflush(fp);
        fclose(fp);
    }
}

void SetLastRequestTime()
{
    FILE* fp = fopen("/db/last_request.ini", "wb");
    if(fp)
    {
        DATETIME_32 curTime = dbm_GetCurDateTime();
        fwrite(&curTime, sizeof(curTime), 1 ,fp);
        fflush(fp);
        fclose(fp);
    }
}

DATETIME_32 GetLastRequestTime()
{
    DATETIME_32 lastTime = { 0 };
    FILE* fp = fopen("/db/last_request.ini", "rb");
    if(fp)
    {
        fread(&lastTime, sizeof(lastTime), 1, fp);
        fclose(fp);
    }

    return lastTime;
}
#endif

int ReadSystemRunningCount()
{
    return g_xHD2.x.iBootingCount;
}

void IncreaseSystemRunningCount()
{
    // g_xHD2.x.iBootingCount ++;
    // UpdateHeadInfos2();
}

int ReadSystemRunningCount1()
{
    return g_xHD2.x.iBootingCount1;
}

void IncreaseSystemRunningCount1()
{
    // g_xHD2.x.iBootingCount1 ++;
    // UpdateHeadInfos2();
}
