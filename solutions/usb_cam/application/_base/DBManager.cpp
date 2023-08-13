

#include "DBManager.h"
#include "appdef.h"
#include "FaceRetrievalSystem.h"
#include "sha1.h"
#include "common_types.h"

#include <string.h>
// #include <time.h>
#include <stdio.h>
// #include <errno.h>
// #include <math.h>
// #include <unistd.h>
// #include <ctype.h>
#include <fcntl.h>
// #include <stdarg.h>
// #include <malloc.h>
#include <stdlib.h>
// #include <dirent.h>
// #include <sys/stat.h>
// #include <sys/mman.h>
// #include <sys/ioctl.h>
// #include <sys/mount.h>

int             g_iDebugEn = DEBUG_EN;
char*           g_szDataPath = NULL;

#ifdef MMAP_MODE
DB_INFO *g_xDB = NULL;
DB_INFO *g_xDBreal = NULL;
#else // MMAP_MODE
#ifndef __RTK_OS__
DB_INFO         g_xDB = { 0 };
#else // ! __RTK_OS__
DB_INFO*        g_xDBreal = NULL;
#endif // ! __RTK_OS__
#endif // MMAP_MODE
int* g_aiDBValid = NULL;

#if (N_MAX_HAND_NUM)
DB_HAND_INFO    g_xHandDB;
//int g_aiHandValid[N_MAX_HAND_NUM] = { 0 };
void dbm_SetEmptyHandDB(int* piBlkNum);
#define g_xHandDBPtr (&g_xHandDB)
#endif

int             g_iTmpLogCount = 0;
SLogInfo*       g_pxTmpLogs = NULL;

int             g_anLogMaxNum[LOG_FILTER_NUM] = {N_MAX_FACE_LOG_NUM};
const char*     g_aszLogPrefix[LOG_FILTER_NUM] = {PREFIX_USER_LOG_DB_NAME};
int             g_anLogCount[LOG_FILTER_NUM] = { 0 };
int             g_anLogStartIndex[LOG_FILTER_NUM] = { 0 };

#define DB_FLUSH_FLAG_DATA          1
#define DB_FLUSH_FLAG_BIT           2
#define DB_FLUSH_FLAG_BACKUP        4
#define DB_FLUSH_FLAG_BACKUP_BIT    8
#define DB_FLUSH_FLAG_ALL (DB_FLUSH_FLAG_DATA | DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP | DB_FLUSH_FLAG_BACKUP_BIT)
int dbm_FlushUserDB(int nUserID = -1, int nFlushData = (DB_FLUSH_FLAG_DATA | DB_FLUSH_FLAG_BIT), int nIsHand = 0);

#ifdef __RTK_OS__

#ifdef MMAP_MODE
unsigned int g_iUserdbAddr = 0;
#endif

#define g_xDB (getDBPtr())

DB_INFO* getDBPtr()
{
    return g_xDBreal;
}

void initDBPtr()
{
#ifdef MMAP_MODE
    g_iUserdbAddr = USERDB_START_ADDR;
#endif // MMAP_MODE
    if (g_xDBreal == NULL)
        g_xDBreal = (DB_INFO*)my_malloc(sizeof(DB_INFO));
}

void deinitDBPtr()
{
    if (g_xDBreal)
    {
        my_free(g_xDBreal);
        g_xDBreal = NULL;
    }
}

#endif // __RTK_OS__

UserPosInfo CreateNewUserPosInfo()
{
    UserPosInfo xPosInfo = { 0 };
    return xPosInfo;
}

int m_fd = -1;
void dbm_Init(char* szDataPath)
{
#ifdef __RTK_OS__
    initDBPtr();
#endif
    if (g_szDataPath == NULL)
        g_szDataPath = (char*)my_malloc(256);
    *g_szDataPath = 0;
    if (szDataPath != NULL)
    {
        strcpy(g_szDataPath, szDataPath);
    }
    if (g_aiDBValid == NULL)
    {
        g_aiDBValid = (int*)my_malloc(sizeof(int) * N_MAX_PERSON_NUM);
        memset(g_aiDBValid, 0, sizeof(int) * N_MAX_PERSON_NUM);
    }
}

int dbm_Free()
{
#ifdef __RTK_OS__
#ifdef MMAP_MODE
    if(m_fd >= 0)
    {
        int FILESIZE = sizeof(DB_INFO);
        if (g_xDB != NULL)
        {
            my_msync(g_xDB, FILESIZE);
            my_munmap(g_xDB, FILESIZE);
        }

        m_fd = -1;
    }
#endif // MMAP_MODE
    deinitDBPtr();
#endif // __RTK_OS__
    if (g_szDataPath != NULL)
    {
        my_free(g_szDataPath);
        g_szDataPath = NULL;
    }
    if (g_aiDBValid != NULL)
    {
        my_free(g_aiDBValid);
        g_aiDBValid = NULL;
    }

    return 0;
}

int	dbm_LoadPersonDB()
{
    int iCheckSumErr = 0;
//    int fUserFileState = 0;

    memset(getDBPtr(), 0, sizeof(DB_INFO));
    int FILESIZE = sizeof(DB_INFO);
    int real_file_len;

    LOG_PRINT("[%s] start, %d, %ld, %ld\n", __func__, FILESIZE, sizeof(DB_UNIT), sizeof(g_xDB->aiValid));

    my_userdb_open(dbfs_get_cur_part());

    real_file_len = my_userdb_read(0, g_xDB, FILESIZE);

    if (real_file_len != FILESIZE)
    {
        my_printf("file size not match: %d, %d\n", real_file_len, FILESIZE);
        dbm_Free();
        return ES_FILE_ERROR;
    }
    m_fd = 1;

    memset(g_aiDBValid, 0, sizeof(int) * N_MAX_PERSON_NUM);

    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(g_xDB->aiValid[i] != 1)
            continue;

        g_aiDBValid[i] = 1;
        int iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[i].xM, sizeof(SMetaInfo));
        if(iCheckSum != g_xDB->ax[i].iMetaCheckSum)
        {
            my_printf("checksum1 error : %d, %08x<>%08x\n", i, iCheckSum, g_xDB->ax[i].iMetaCheckSum);
            iCheckSumErr = 1;
            g_aiDBValid[i] = 0;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
                g_xDB->aiValid[i] = 0;
        }
        else
        {
            iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[i].xF, sizeof(SFeatInfo));
            if(iCheckSum != g_xDB->ax[i].iFeatCheckSum)
            {
                my_printf("checksum2 error : %d, %08x<>%08x\n", i, iCheckSum, g_xDB->ax[i].iFeatCheckSum);

                iCheckSumErr = 1;
                g_aiDBValid[i] = 0;

                if (dbfs_get_cur_part() != DB_PART_BACKUP)
                    g_xDB->aiValid[i] = 0;
            }
        }
    }

    if(iCheckSumErr)
    {
        my_printf("checksum error\n");
        return ES_FILE_ERROR;
    }

    return ES_SUCCESS;
}

void dbm_SetEmptyPersonDB(int* piBlkNum)
{
    if (g_xDB != NULL/* && g_xDB != MAP_FAILED*/)
    {
        memset(g_xDB->aiValid, 0, sizeof(g_xDB->aiValid));
        memset(g_aiDBValid, 0, sizeof(g_aiDBValid[0]) * N_MAX_PERSON_NUM);
        dbm_FlushUserDB(-1, DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP_BIT);
    }
}

//////////////////////////////////////////////////////////////////////////
int dbm_AddPerson(PSMetaInfo pxUserInfo, PSFeatInfo pxFeatInfo, int* piBlkNum)
{
    dbug_printf("[%s] start\n", __func__);
    if(pxUserInfo == NULL || pxFeatInfo == NULL)
        return ES_FAILED;

    if(pxUserInfo->iID < 0 || pxUserInfo->iID >= N_MAX_PERSON_NUM)
        return ES_FAILED;

    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    dbug_printf("[%s] ID=%d\n", __func__, pxUserInfo->iID);

    g_xDB->aiValid[pxUserInfo->iID] = 1;
    g_aiDBValid[pxUserInfo->iID] = 1;

    g_xDB->ax[pxUserInfo->iID].xM = *pxUserInfo;
    g_xDB->ax[pxUserInfo->iID].xF = *pxFeatInfo;

    int iCheckSum = GetIntCheckSum((int*)pxUserInfo, sizeof(SMetaInfo));
    g_xDB->ax[pxUserInfo->iID].iMetaCheckSum = iCheckSum;
    LOG_PRINT("[%s] ms=%08x\n", __func__, g_xDB->ax[pxUserInfo->iID].iMetaCheckSum);

    iCheckSum = GetIntCheckSum((int*)pxFeatInfo, sizeof(SFeatInfo));
    g_xDB->ax[pxUserInfo->iID].iFeatCheckSum = iCheckSum;
    LOG_PRINT("[%s] fs=%08x\n", __func__, g_xDB->ax[pxUserInfo->iID].iFeatCheckSum);

    if (dbm_FlushUserDB(pxUserInfo->iID, DB_FLUSH_FLAG_ALL))
        return ES_FAILED;

    return ES_SUCCESS;
}

int	dbm_UpdatePerson(int nIndex, PSMetaInfo pxUserInfo, PSFeatInfo pxFeatInfo, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;
    int iID = dbm_GetIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;
    if (dbm_UpdatePersonMetaInfo(nIndex, pxUserInfo, piBlkNum) != ES_SUCCESS)
        return ES_FAILED;
    if (dbm_UpdatePersonFeatInfo(nIndex, pxFeatInfo, piBlkNum, -1) != ES_SUCCESS)
        return ES_FAILED;
    if (dbm_FlushUserDB(iID, DB_FLUSH_FLAG_BACKUP) != ES_SUCCESS)
        return ES_FAILED;
    return ES_SUCCESS;
}

int dbm_UpdatePersonMetaInfo(int nIndex, PSMetaInfo pxUserInfo, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if(pxUserInfo == NULL)
        return ES_FAILED;

    int iID = dbm_GetIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;

    int iCheckSum = GetIntCheckSum((int*)pxUserInfo, sizeof(SMetaInfo));
    g_xDB->ax[iID].iMetaCheckSum = iCheckSum;
    g_xDB->ax[iID].xM = *pxUserInfo;

    if (dbm_FlushUserDB(iID, DB_FLUSH_FLAG_DATA) != ES_SUCCESS)
        return ES_FAILED;

    return ES_SUCCESS;
}

int	dbm_UpdatePersonFeatInfo(int nIndex, PSFeatInfo pxFeatInfo, int* piBlkNum, int iUpdateFeatIndex)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if(pxFeatInfo == NULL)
        return ES_FAILED;

    int iID = dbm_GetIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;

    int iCheckSum = GetIntCheckSum((int*)pxFeatInfo, sizeof(SFeatInfo));
    g_xDB->ax[iID].iFeatCheckSum = iCheckSum;
    g_xDB->ax[iID].xF = *pxFeatInfo;

    if (dbm_FlushUserDB(iID, DB_FLUSH_FLAG_DATA) != ES_SUCCESS)
        return ES_FAILED;

    return ES_SUCCESS;
}

int dbm_LoadPersonDBValid(int fd)
{
    int iCheckSumErr = 0;
    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(g_xDB->aiValid[i] == 0)
            continue;

        g_aiDBValid[i] = 1;
        int iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[i].xM, sizeof(SMetaInfo));
        if(iCheckSum != g_xDB->ax[i].iMetaCheckSum)
        {
//            printf("checksum1 error : %d\n", i);
            iCheckSumErr = 1;
            g_aiDBValid[i] = 0;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xDB->aiValid[i] = 0;
            }
        }

        iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[i].xF, sizeof(SFeatInfo));
        if(iCheckSum != g_xDB->ax[i].iFeatCheckSum)
        {
//            printf("checksum2 error : %d\n", i);

            iCheckSumErr = 1;
            g_aiDBValid[i] = 0;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xDB->aiValid[i] = 0;
            }
        }
    }
    return iCheckSumErr;
}

int dbm_LoadHandDBValid(int fd)
{
    int iCheckSumErr = 0;
    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if(g_xHandDB.aiHandValid[i] == 0)
            continue;

        int iCheckSum = GetIntCheckSum((int*)&g_xHandDB.ax[i].xHM, sizeof(SMetaInfo));
        if(iCheckSum != g_xHandDB.ax[i].iHMetaCheckSum)
        {
//            printf("checksum1 error : %d\n", i);
            iCheckSumErr = 1;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xHandDB.aiHandValid[i] = 0;
            }
        }

        iCheckSum = GetIntCheckSum((int*)&g_xHandDB.ax[i].xHF, sizeof(SHandFeatInfo));
        if(iCheckSum != g_xHandDB.ax[i].iHFCheckSum)
        {
//            printf("checksum2 error : %d\n", i);

            iCheckSumErr = 1;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xHandDB.aiHandValid[i] = 0;
            }
        }
    }
    return iCheckSumErr;
}

int dbm_UpdatePersonBin(int iFlag, unsigned char* pData, int iLen, int iUserID)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if (!pData || iLen <=0)
        return ES_FAILED;

    dbug_printf("[%s] idx=%d, len=%d, %d\n", __func__, iFlag, iLen, (int)sizeof(DB_UNIT));

    if (iFlag == 2) // face only one
    {
        int iID = -1;
        if (iUserID > 0)
        {
            if (iUserID > N_MAX_PERSON_NUM)
            {
                return ES_INVALID;
            }
            // if (dbm_GetPersonMetaInfoByID(iUserID - 1))
            // {
            //     return ES_INVALID;
            // }
            iID = iUserID - 1;
        }
        if (iID < 0)
            iID = dbm_GetNewUserID();
        if (iID < 0)
            return ES_FULL;
        if (iLen !=(int)sizeof(DB_UNIT))
            return ES_INVALID;

        dbug_printf("new ID = %d\n", iID);
        memcpy(&g_xDB->ax[iID], pData, sizeof(g_xDB->ax[iID]));
        g_xDB->aiValid[iID] = 1;
        g_aiDBValid[iID] = 1;

        g_xDB->ax[iID].xM.iID = iID;
        int iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[iID].xM, sizeof(SMetaInfo));
        g_xDB->ax[iID].iMetaCheckSum = iCheckSum;

        iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[iID].xF, sizeof(SFeatInfo));
        g_xDB->ax[iID].iFeatCheckSum = iCheckSum;

        dbm_FlushUserDB(iID, DB_FLUSH_FLAG_ALL);
        return ES_SUCCESS;
    }
    else if (iFlag == 3) //hand only one
    {
        int iID = -1;
        if (iUserID > 0)
        {
            if (iUserID <= N_MAX_PERSON_NUM || iUserID > N_MAX_PERSON_NUM + N_MAX_HAND_NUM)
            {
                return ES_INVALID;
            }
            iUserID -= N_MAX_PERSON_NUM;
            // if (dbm_GetHandMetaInfoByID(iUserID - 1))
            // {
            //     return ES_INVALID;
            // }
            iID = iUserID - 1;
        }
        if (iID < 0)
            iID = dbm_GetNewHandUserID();
        if (iID < 0)
            return ES_FULL;
        if (iLen != (int)sizeof(DB_HAND_UNIT))
            return ES_INVALID;

        dbug_printf("new ID = %d\n", iID);
        memcpy(&g_xHandDB.ax[iID], pData, sizeof(g_xHandDB.ax[iID]));
        g_xHandDB.aiHandValid[iID] = 1;

        g_xHandDB.ax[iID].xHM.iID = iID;
        int iCheckSum = GetIntCheckSum((int*)&g_xHandDB.ax[iID].xHM, sizeof(SMetaInfo));
        g_xHandDB.ax[iID].iHMetaCheckSum = iCheckSum;

        iCheckSum = GetIntCheckSum((int*)&g_xHandDB.ax[iID].xHF, sizeof(SHandFeatInfo));
        g_xHandDB.ax[iID].iHFCheckSum = iCheckSum;

        dbm_FlushUserDB(iID, DB_FLUSH_FLAG_ALL, 1);
        return ES_SUCCESS;
    }
    else if (iFlag == 1)
    {
        //update whole data
#if (N_MAX_HAND_NUM)
        if (iLen != sizeof(DB_INFO) + sizeof(DB_HAND_INFO))
            return ES_INVALID;
#else
        if (iLen != sizeof(DB_INFO))
            return ES_INVALID;
#endif
        memcpy(g_xDB, pData, sizeof(*g_xDB));
        dbm_LoadPersonDBValid(-1);

        dbm_FlushUserDB(-1, DB_FLUSH_FLAG_ALL);

#if (N_MAX_HAND_NUM)
        memcpy(g_xHandDBPtr, pData + sizeof(DB_INFO), sizeof(DB_HAND_INFO));
        dbm_LoadHandDBValid(-1);

        dbm_FlushUserDB(-1, DB_FLUSH_FLAG_ALL, 1);
#endif
        return ES_SUCCESS;
    }
    else
    {
        return ES_FAILED;
    }
}

unsigned char* dbm_GetPersonBin()
{
    return (unsigned char*)g_xDB;
}

unsigned char* dbm_GetHandBin()
{
    return (unsigned char*)&g_xHandDB;
}

int	dbm_GetPersonCount()
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return 0;
#endif

    int iCount = 0;
    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(g_aiDBValid[i] == 1)
            iCount ++;
    }
    return iCount;
}

int	dbm_GetTotalUserCount()
{
    int ret = 0;
    ret = dbm_GetPersonCount();
#if (N_MAX_HAND_NUM)
    ret += dbm_GetHandCount();
#endif
    return ret;
}

int dbm_GetIDOfIndex(int iIndex)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return -1;
#endif
    int iIdx = -1;
    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(g_aiDBValid[i] == 0)
            continue;

        iIdx ++;
        if(iIdx == iIndex)
            return i;
    }

    return -1;
}

int dbm_GetIndexOfID(int iID)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return -1;
#endif

    int iIdx = -1;
    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(g_aiDBValid[i] == 0)
            continue;

        iIdx ++;
        if(i == iID)
            return iIdx;
    }

    return -1;
}


int dbm_IsValidOfID(int iID)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return 0;
#endif

    return g_aiDBValid[iID];
}

int dbm_GetNewUserID()
{
    for(int i = N_USER_PERM_BEGIN_ID; i < N_USER_PERM_BEGIN_ID + N_MAX_PERSON_NUM; i ++)
    {
        if(g_aiDBValid[i] == 0)
            return i;
    }

    return -1;
}

int dbm_GetUserCount(int iUserRole)
{
#ifdef MMAP_MODE
    if(!g_xDB)
        return 0;
#endif

    int iUserCount = 0;
    for(int i = N_USER_PERM_BEGIN_ID; i < N_USER_PERM_BEGIN_ID + N_MAX_PERSON_NUM; i ++)
    {
        if(g_aiDBValid[i] == 0)
            continue;
#ifdef MMAP_MODE
        if(g_xDB->ax[i].xM.fPrivilege == iUserRole || (iUserRole == -1))
            iUserCount ++;        
#else
        if(g_xDBreal->ax[i].xM.fPrivilege == iUserRole || (iUserRole == -1))
            iUserCount ++;
#endif
    }

    return iUserCount;
}

int dbm_GetUserInfoByIndex(int nIndex, PSMetaInfo pxUserInfo, PSFeatInfo pxFeatInfo)
{
    int iIdx = -1;
    for(int i = N_USER_PERM_BEGIN_ID; i < N_USER_PERM_BEGIN_ID + N_MAX_PERSON_NUM; i ++)
    {
        if(g_aiDBValid[i] == 0)
            continue;

        iIdx ++;
        if(iIdx == nIndex)
        {
            memcpy(pxUserInfo, &g_xDB->ax[i].xM, sizeof(SMetaInfo));
            if(pxFeatInfo != NULL)
                memcpy(pxFeatInfo, &g_xDB->ax[i].xF, sizeof(SFeatInfo));
            return ES_SUCCESS;
        }
    }

    return ES_FAILED;
}


PSMetaInfo	dbm_GetPersonMetaInfoByID(int nID)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return NULL;
#endif
	if(nID < 0 || nID >= N_MAX_PERSON_NUM)
        return NULL;
        
    if(g_aiDBValid[nID] == 0)
        return NULL;
    return &g_xDB->ax[nID].xM;
}

PSFeatInfo	dbm_GetPersonFeatInfoByID(int nID)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return NULL;
#endif

    if(nID < 0 || nID >= N_MAX_PERSON_NUM)
        return NULL;
        
    if(g_aiDBValid[nID] == 0)
        return NULL;
    return &g_xDB->ax[nID].xF;
}

PSMetaInfo dbm_GetPersonMetaInfoByIndex(int nPos)
{
    int iID = dbm_GetIDOfIndex(nPos);
    if(iID < 0)
        return NULL;
    return &g_xDB->ax[iID].xM;
}

PSFeatInfo dbm_GetPersonFeatInfoByIndex(int nPos)
{
    int iID = dbm_GetIDOfIndex(nPos);
    if(iID < 0)
        return NULL;

    return &g_xDB->ax[iID].xF;
}

int	dbm_RemovePersonByID(int nID, int* piBlkNum)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return ES_FAILED;
#endif

    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if(g_aiDBValid[nID] == 0)
        return ES_FAILED;
    g_xDB->aiValid[nID] = 0;
    g_aiDBValid[nID] = 0;
    dbm_FlushUserDB(nID, DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP_BIT);
    return ES_SUCCESS;
}

int dbm_RemovePersonByIndex(int nIndex, int* piBlkNum)
{
    int iID = dbm_GetIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;

    return dbm_RemovePersonByID(iID, piBlkNum);
}

int dbm_RemovePersonByPrivilege(int iPrivilege, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    for (int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        PSMetaInfo m = dbm_GetPersonMetaInfoByID(i);
        if (m && m->fPrivilege == iPrivilege)
        {
            g_xDB->aiValid[i] = 0;
            g_aiDBValid[i] = 0;
        }
    }

    dbm_FlushUserDB(-1, DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP_BIT);

    return ES_SUCCESS;
}

int dbm_FlushUserDB(int nUserID, int nFlushData, int nIsHand)
{
    LOG_PRINT("[%s] start(%d), uid=%d, %0.1f\n", __func__, nIsHand, nUserID, Now());

    if ((dbfs_get_cur_part() == DB_PART1 || nUserID == -1) && nIsHand == 0)
    {
        if (nUserID == -1)
        {
            if (nFlushData & DB_FLUSH_FLAG_DATA)
            {
                my_userdb_write(0, g_xDB, sizeof(DB_INFO));
                my_backupdb_write(0, g_xDB, sizeof(DB_INFO));
            }
            else
            {
                my_backupdb_write(0, g_xDB->aiValid, sizeof(g_xDB->aiValid));
                my_userdb_write(0, g_xDB->aiValid, sizeof(g_xDB->aiValid));
            }
        }
        else
        {
            //backup
            if (nFlushData & DB_FLUSH_FLAG_BACKUP)
            {
                my_backupdb_write(sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), 
                    ((char*)g_xDB) + sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), sizeof(DB_UNIT));
            }
            if (nFlushData & DB_FLUSH_FLAG_BACKUP_BIT)
            {
                my_backupdb_write(0, g_xDB->aiValid, sizeof(g_xDB->aiValid));
            }
            if (nFlushData & DB_FLUSH_FLAG_DATA)
            {
                my_userdb_write(sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), 
                    ((char*)g_xDB) + sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), sizeof(DB_UNIT));
            }
            if (nFlushData & DB_FLUSH_FLAG_BIT)
            {
                my_userdb_write(0, g_xDB->aiValid, sizeof(g_xDB->aiValid));
            }
        }
        LOG_PRINT("[%s] write end, %0.3f\n", __func__, Now());
    }
#if (N_MAX_HAND_NUM)
    else if ((dbfs_get_cur_part() == DB_PART1 || nUserID == -1) && nIsHand == 1)
    {
        int FILESIZE = sizeof(DB_INFO);
        if (nUserID == -1)
        {
            if (nFlushData & DB_FLUSH_FLAG_DATA)
            {
                my_userdb_write(FILESIZE, g_xHandDBPtr, sizeof(DB_HAND_INFO));
                my_backupdb_write(FILESIZE, g_xHandDBPtr, sizeof(DB_HAND_INFO));
            }
            else
            {
                my_userdb_write(FILESIZE, g_xHandDBPtr->aiHandValid, sizeof(g_xHandDBPtr->aiHandValid));
                my_backupdb_write(FILESIZE, g_xHandDBPtr->aiHandValid, sizeof(g_xHandDBPtr->aiHandValid));
            }
        }
        else
        {
            //backup
            if (nFlushData & DB_FLUSH_FLAG_BACKUP)
            {
                my_backupdb_write(FILESIZE + sizeof(DB_HAND_INFO::aiHandValid) + nUserID * sizeof(DB_HAND_UNIT), 
                    ((char*)g_xHandDBPtr) + sizeof(DB_HAND_INFO::aiHandValid) + nUserID * sizeof(DB_HAND_UNIT), sizeof(DB_HAND_UNIT));
            }
            if (nFlushData & DB_FLUSH_FLAG_BACKUP_BIT)
            {
                my_backupdb_write(FILESIZE, g_xHandDBPtr->aiHandValid, sizeof(g_xHandDBPtr->aiHandValid));
            }
            if (nFlushData & DB_FLUSH_FLAG_DATA)
            {
                my_userdb_write(FILESIZE + sizeof(DB_HAND_INFO::aiHandValid) + nUserID * sizeof(DB_HAND_UNIT), 
                    ((char*)g_xHandDBPtr) + sizeof(DB_HAND_INFO::aiHandValid) + nUserID * sizeof(DB_HAND_UNIT), sizeof(DB_HAND_UNIT));
            }
            if (nFlushData & DB_FLUSH_FLAG_BIT)
            {
                my_userdb_write(FILESIZE, g_xHandDBPtr->aiHandValid, sizeof(g_xHandDBPtr->aiHandValid));
            }
        }
        LOG_PRINT("[%s] write end, %0.3f\n", __func__, Now());
    }
#endif // N_MAX_HAND_NUM
    else
        return 1;

    return 0;
}

int CheckFileSystem(int iIndex)
{
    return 1;
}

int* GetDebugEn()
{
    return &g_iDebugEn;
}

#ifndef BUF_SIZE        /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024
#endif

int CopyFile(char* szDstPath, char* szSrcPath)
{
    myfdesc_ptr inputFd, outputFd;
    int openFlags;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE];

    /* Open input and output files */

    inputFd = my_open(szSrcPath, O_RDONLY, 0);
    if (!is_myfdesc_ptr_valid(inputFd))
        return -1;

    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
             S_IROTH | S_IWOTH;      /* rw-rw-rw- */
    outputFd = my_open(szDstPath, openFlags, filePerms);
    if (!is_myfdesc_ptr_valid(outputFd))
        return -1;

    /* Transfer data until we encounter end of input or an error */

    while ((numRead = my_read(inputFd, buf, BUF_SIZE)) > 0)
    {
        if (my_write(outputFd, buf, numRead) != numRead)
        {
            my_printf("couldn't write whole buffer\n");

            my_close(inputFd);
            my_close(outputFd);
            return -1;
        }
    }

    my_close(inputFd);

    my_fsync(outputFd);
    my_close(outputFd);
    return 0;
}

int fileopen_and_copy(char * dest, char * src)
{
    return CopyFile(dest, src);
}

int file_ext_cmp(const char* filename, const char*ext)
{
    int i;
    int len1 = strlen(filename), len2 = strlen(ext);
    if (len1 < len2)
        return 1;
    for (i = len2 - 1; i >= 0; i --)
        if (ext[i] != filename[len1 - len2 + i])
            break;
    if (i >= 0)
        return 1;
    return 0;
}

int str_starts(const char* str, const char* str1)
{
    if (strlen(str) < strlen(str1))
        return 1;

    return strncmp(str, str1, strlen(str1));
}

int CheckBackupDB()
{
    int FILESIZE = sizeof(DB_INFO);
    int real_file_len = 0;
    myfdesc_ptr fd;
    fd = my_open(BACKUP_USERINFO_DAT, O_RDONLY, 0);
    if (!is_myfdesc_ptr_valid(fd))
    {
        my_create_empty_file(BACKUP_USERINFO_DAT, FILESIZE);
        //reopen file
        fd = my_open(BACKUP_USERINFO_DAT, O_RDONLY, 0);
    }
    if (is_myfdesc_ptr_valid(fd))
    {
        real_file_len = my_seek(fd, 0, SEEK_END);
        my_close(fd);
    }
    if(real_file_len != FILESIZE)
    {
        dbug_printf("[%s] file length mismatch %d, %d\n", __func__, real_file_len, FILESIZE);
        my_create_empty_file(BACKUP_USERINFO_DAT, FILESIZE);
    }
    return 0;
}

int CheckBackupDBInfos()
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return -1;
#endif
    int FILESIZE = sizeof(DB_INFO);
    myfdesc_ptr fd;
    int real_file_len;
    fd = my_open(BACKUP_USERINFO_DAT, O_RDONLY, 0);
    if (!is_myfdesc_ptr_valid(fd))
        return -3;

    real_file_len = my_seek(fd, 0, SEEK_END);

    if(real_file_len != FILESIZE)
    {
        my_close(fd);
        return -2;
    }

    my_seek(fd, 0, SEEK_SET);

    int aiValid[N_MAX_PERSON_NUM];

    my_read(fd, aiValid, sizeof(aiValid));
#ifdef MMAP_MODE
    if(memcmp(g_xDB->aiValid, aiValid, sizeof(aiValid)))
#else
    if(memcmp(g_xDBreal->aiValid, aiValid, sizeof(aiValid)))
#endif
    {
        my_close(fd);
        return -4;
    }

    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(aiValid[i] == 0)
            continue;

        DB_UNIT xDBUnit;
        memset(&xDBUnit, 0, sizeof(xDBUnit));
        my_seek(fd, sizeof(aiValid) + i * sizeof(DB_UNIT), SEEK_SET);
        my_read(fd, &xDBUnit, sizeof(xDBUnit));

        if(xDBUnit.iMetaCheckSum != GetIntCheckSum((int*)&xDBUnit.xM, sizeof(xDBUnit.xM)))
        {
            my_close(fd);
            return -5;
        }

        if(xDBUnit.iFeatCheckSum != GetIntCheckSum((int*)&xDBUnit.xF, sizeof(xDBUnit.xF)))
        {
            my_close(fd);
            return -6;
        }
    }

    my_close(fd);
    return 0;
}

int is_backup_partition()
{
#ifdef ENGINE_DEV_SSD
    if (g_pxSharedLCD && g_pxSharedLCD->iMountPoints >= DB_PART_BACKUP)
        return 1;
#endif
#ifdef ENGINE_DEV_V3S
    if (g_pxSharedLCD && g_pxSharedLCD->iMapPoints >= MAP_PT_BACKUP)
        return 1;
#endif
    return 0;
}

#if (N_MAX_HAND_NUM)

int dbm_AddHand(PSMetaInfo pxUserInfo, SHandFeatInfo* pxFeatInfo, int* piBlkNum)
{
    if(pxUserInfo == NULL || pxFeatInfo == NULL)
        return ES_FAILED;

    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    g_xHandDB.aiHandValid[pxUserInfo->iID] = 1;

    g_xHandDB.ax[pxUserInfo->iID].xHM = *pxUserInfo;
    g_xHandDB.ax[pxUserInfo->iID].xHF = *pxFeatInfo;

    int iCheckSum = GetIntCheckSum((int*)pxUserInfo, sizeof(SMetaInfo));
    g_xHandDB.ax[pxUserInfo->iID].iHMetaCheckSum = iCheckSum;

    iCheckSum = GetIntCheckSum((int*)pxFeatInfo, sizeof(*pxFeatInfo));
    g_xHandDB.ax[pxUserInfo->iID].iHFCheckSum = iCheckSum;

    if (dbm_FlushUserDB(pxUserInfo->iID, DB_FLUSH_FLAG_ALL, 1))
        return ES_FAILED;

    return ES_SUCCESS;
}

int	dbm_UpdateHand(int nIndex, PSMetaInfo pxUserInfo, SHandFeatInfo* pxFeatInfo, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    dbm_UpdateHandMetaInfo(nIndex, pxUserInfo, piBlkNum);
    return dbm_UpdateHandFeatInfo(nIndex, pxFeatInfo, piBlkNum);
}

int dbm_UpdateHandMetaInfo(int nIndex, PSMetaInfo pxMetaInfo, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if(pxMetaInfo == NULL)
        return ES_FAILED;

    int iID = dbm_GetHandIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;

    int iCheckSum = GetIntCheckSum((int*)pxMetaInfo, sizeof(SMetaInfo));
    g_xHandDB.ax[iID].iHMetaCheckSum = iCheckSum;
    g_xHandDB.ax[iID].xHM = *pxMetaInfo;
    if (dbm_FlushUserDB(iID, DB_FLUSH_FLAG_DATA, 1) != ES_SUCCESS)
        return ES_FAILED;

    return ES_SUCCESS;
}

int	dbm_UpdateHandFeatInfo(int nIndex, SHandFeatInfo *pxFeatInfo, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if(pxFeatInfo == NULL)
        return ES_FAILED;

    int iID = dbm_GetHandIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;

    int iCheckSum = GetIntCheckSum((int*)pxFeatInfo, sizeof(SHandFeatInfo));
    g_xHandDB.ax[iID].iHFCheckSum = iCheckSum;
    g_xHandDB.ax[iID].xHF = *pxFeatInfo;

    if (dbm_FlushUserDB(iID, DB_FLUSH_FLAG_DATA, 1) != ES_SUCCESS)
        return ES_FAILED;

    return ES_SUCCESS;
}

int dbm_GetHandIDOfIndex(int iIndex)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return -1;
#endif
    int iIdx = -1;
    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if(g_xHandDB.aiHandValid[i] != 1)
            continue;

        iIdx ++;
        if(iIdx == iIndex)
            return i;
    }

    return -1;
}

int dbm_GetHandIndexOfID(int iID)
{
#ifdef MMAP_MODE
    if(g_xDB == NULL)
        return -1;
#endif

    int iIdx = -1;
    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if(g_xHandDB.aiHandValid[i] != 1)
            continue;

        iIdx ++;
        if(i == iID)
            return iIdx;
    }

    return -1;
}

PSMetaInfo dbm_GetHandMetaInfoByIndex(int nPos)
{
    int iID = dbm_GetHandIDOfIndex(nPos);
    if(iID < 0)
        return NULL;
    return &g_xHandDB.ax[iID].xHM;
}

PSMetaInfo dbm_GetHandMetaInfoByID(int iID)
{
    if(iID < 0 || iID >= N_MAX_HAND_NUM)
        return NULL;
    return g_xHandDB.aiHandValid[iID] != 1 ? NULL: &g_xHandDB.ax[iID].xHM;
}

SHandFeatInfo*	dbm_GetHandFeatInfoByID(int nID)
{
    if(nID < 0 || nID >= N_MAX_HAND_NUM)
        return NULL;

    if(g_xHandDB.aiHandValid[nID] != 1)
        return NULL;
    return &g_xHandDB.ax[nID].xHF;
}

SHandFeatInfo*  dbm_GetHandFeatInfoByIndex(int nPos)
{
    int iID = dbm_GetHandIDOfIndex(nPos);
    if(iID < 0)
        return NULL;

    return &g_xHandDB.ax[iID].xHF;
}

int dbm_GetNewHandUserID()
{
    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if(g_xHandDB.aiHandValid[i] != 1)
            return i;
    }

    return -1;
}

int dbm_GetHandUserCount(int iUserRole)
{
#ifdef MMAP_MODE
    if(!g_xDB)
        return 0;
#endif

    int iUserCount = 0;
    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if(g_xHandDB.aiHandValid[i] != 1)
            continue;
        if(g_xHandDB.ax[i].xHM.fPrivilege == iUserRole || (iUserRole == -1))
            iUserCount ++;
    }

    return iUserCount;
}

int	dbm_LoadHandDB()
{
    int iCheckSumErr = 0;

    my_memset(&g_xHandDB, 0, sizeof(g_xHandDB));
    int FILESIZE = sizeof(g_xHandDB);

    int nReadNum = my_userdb_read(sizeof(DB_INFO), &g_xHandDB, FILESIZE);

    if (nReadNum != sizeof(g_xHandDB))
    {
        my_printf("[%s] failed to read user info %d\n", __FUNCTION__, nReadNum);
        dbm_Free();

        return ES_FILE_ERROR;
    }

    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if (g_xHandDB.aiHandValid[i] == 1)
        {
            int iCheckSum;

            iCheckSum = GetIntCheckSum((int*)&g_xHandDB.ax[i].xHM, sizeof(SMetaInfo));
            if(iCheckSum != g_xHandDB.ax[i].iHMetaCheckSum)
            {
                my_printf("meta checksum error %d\n", i);
                iCheckSumErr = 1;
                g_xHandDB.aiHandValid[i] = 0;
                // lseek(fd, offsetof(DB_HAND_INFO, aiHandValid) + i * sizeof(g_xHandDB.aiHandValid[0]), SEEK_SET);
                // write(fd, 0, sizeof(g_xHandDB.aiHandValid[0]));
            }
            else
            {
                iCheckSum = GetIntCheckSum((int*)&g_xHandDB.ax[i].xHF, sizeof(SHandFeatInfo));
                if(iCheckSum != g_xHandDB.ax[i].iHFCheckSum)
                {
                    my_printf("hf checksum error %d\n", i);
                    iCheckSumErr = 1;
                    g_xHandDB.aiHandValid[i] = 0;
                    // lseek(fd, offsetof(DB_HAND_INFO, aiHandValid) + i * sizeof(g_xHandDB.aiHandValid[0]), SEEK_SET);
                    // write(fd, 0, sizeof(g_xHandDB.aiHandValid[0]));
                }
                else
                {
                    //temp code to recheck
                    //setHandUserFeat(i, g_xHandDB.ax[i].xHF.abFeat);
                }
            }
        }
    }

    if(iCheckSumErr)
    {
        my_printf("h.checksum error\n");
        return ES_FILE_ERROR;
    }

    return ES_SUCCESS;
}

void dbm_SetEmptyHandDB(int* piBlkNum)
{
    my_memset(&g_xHandDB, 0, sizeof(DB_HAND_INFO));
    dbm_FlushUserDB(-1, DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP_BIT, 1);
}

int	dbm_GetHandCount()
{
    int iCount = 0;
    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if(g_xHandDB.aiHandValid[i] == 1)
            iCount ++;
    }
    return iCount;
}

int	dbm_RemoveHandByID(int nID, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if (nID < 0 || nID >= N_MAX_HAND_NUM || g_xHandDB.aiHandValid[nID] != 1)
        return ES_FAILED;

    g_xHandDB.aiHandValid[nID] = 0;

    dbm_FlushUserDB(nID, DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP_BIT, 1);

    return ES_SUCCESS;
}

int dbm_CheckHandBackupDB()
{
#if 0
    int FILESIZE = sizeof(DB_HAND_INFO);
    struct stat filecheck;
    if(stat (BACKUP_HANDINFO_DAT, &filecheck) != 0 || filecheck.st_size != FILESIZE)
    {
        int fd = open(BACKUP_HANDINFO_DAT, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
        if(fd < 0)
            return -1;

        int result = lseek(fd, FILESIZE-1, SEEK_SET);
        if(result == -1)
        {
            close(fd);
            return -2;
        }

        result = write(fd, "", 1);
        if (result != 1)
        {
            close(fd);
            return -3;
        }

        fsync(fd);
        close(fd);
    }
#endif
    return 0;
}

int dbm_CheckHandBackupDBInfos()
{
#if 0
    int FILESIZE = sizeof(DB_HAND_INFO);
    struct stat filecheck;
    if(stat (BACKUP_HANDINFO_DAT, &filecheck) != 0 || filecheck.st_size != FILESIZE)
    {
        if(stat (BACKUP_HANDINFO_DAT, &filecheck) != 0)
            return 0;

        return -2;
    }

    int aiValid[N_MAX_HAND_NUM];
    FILE* fp = fopen(BACKUP_HANDINFO_DAT, "rb");
    if(fp == NULL)
        return -3;

    fread(aiValid, sizeof(aiValid), 1, fp);
    if(memcmp(g_xHandDB.aiHandValid, aiValid, sizeof(aiValid)))
    {
        fclose(fp);
        return -4;
    }

    for(int i = 0; i < N_MAX_HAND_NUM; i ++)
    {
        if(aiValid[i] == 0)
            continue;

        DB_HAND_UNIT xDBUnit;
        memset(&xDBUnit, 0, sizeof(xDBUnit));
        fseek(fp, sizeof(aiValid) + i * sizeof(DB_HAND_UNIT), SEEK_SET);
        fread(&xDBUnit, sizeof(xDBUnit), 1, fp);

        if(xDBUnit.iHMetaCheckSum != GetIntCheckSum((int*)&xDBUnit.xHM, sizeof(xDBUnit.xHM)))
        {
            fclose(fp);
            return -5;
        }

        if(xDBUnit.iHFCheckSum != GetIntCheckSum((int*)&xDBUnit.xHF, sizeof(xDBUnit.xHF)))
        {
            fclose(fp);
            return -6;
        }
    }

    fclose(fp);
#endif
    return 0;
}

#endif // N_MAX_HAND_NUM
