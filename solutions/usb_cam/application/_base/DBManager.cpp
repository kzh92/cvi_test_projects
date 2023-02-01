

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
//SMetaInfo       g_xTmpMetaInfo = { 0 };

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
int dbm_FlushUserDB(int nUserID = -1, int nFlushData = (DB_FLUSH_FLAG_DATA | DB_FLUSH_FLAG_BIT));

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

static int dbfs_get_cur_part()
{
    //my_printf("[%s][%s]\n", __FILE__, __func__);
    CreateSharedLCD();

    if(g_pxSharedLCD == NULL)
        return -1;

    return g_pxSharedLCD->iMountPoints;
}

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

#ifndef __RTK_OS__
#ifdef MMAP_MODE
    if(!g_xDB)
    {
        int FILESIZE = sizeof(DB_INFO);

        struct stat filecheck;

        if(stat (DB_USERINFO_DAT, &filecheck) != 0 || filecheck.st_size != FILESIZE)
        {
            if(stat (DB_USERINFO_DAT, &filecheck) == 0 && filecheck.st_size != FILESIZE)
            {
                my_printf("#######  MMap FileSize Error!!! Src: %x, Err: %x\n", FILESIZE, filecheck.st_size);
                dbm_Free();

                return ES_FILE_ERROR;
            }

            m_fd = open(DB_USERINFO_DAT, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

            // Stretch the file size to the size of the (mmapped) array of ints
            int result = lseek(m_fd, FILESIZE-1, SEEK_SET);
            if (result == -1)
            {
                my_printf("Error calling lseek() to 'stretch' the file\n");
                dbm_Free();

                return ES_FILE_ERROR;
            }

            result = write(m_fd, "", 1);
            if (result != 1)
            {
                my_printf("Error writing last byte of the file\n");
                dbm_Free();

                return ES_FILE_ERROR;
            }
        }
        else
        {
            if (dbfs_get_cur_part() < DB_PART_BACKUP)
                m_fd = open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
            else
                m_fd = open(DB_USERINFO_DAT, O_RDONLY, (mode_t)0600);
        }

        if (m_fd < 0)
        {
            my_printf("[%s]failed to open db file(%s).\n", __FUNCTION__, strerror(errno));
            return ES_FILE_ERROR;
        }

        my_printf("Dbm_Init fd: %d\n", m_fd);

        if (dbfs_get_cur_part() < DB_PART_BACKUP)
            g_xDB = (DB_INFO*)mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
        else
            g_xDB = (DB_INFO*)mmap(NULL, FILESIZE, PROT_READ, MAP_SHARED, m_fd, 0);

        LOG_PRINT("MMap Result %x\n", (unsigned int)g_xDB);

        if(g_xDB == MAP_FAILED)
        {
            my_printf("MMap failed(%s).\n", strerror(errno));
            dbm_Free();

            return ES_FILE_ERROR;
        }

        memset(g_aiDBValid, 0, sizeof(int) * N_MAX_PERSON_NUM);

        for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
        {
            if(g_xDB->aiValid[i] == 0)
                continue;

            g_aiDBValid[i] = 1;
            int iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[i].xM, sizeof(SMetaInfo));
            if(iCheckSum != g_xDB->ax[i].iMetaCheckSum)
            {
                my_printf("checksum1 error : %d\n", i);
                iCheckSumErr = 1;
                g_aiDBValid[i] = 0;

                if (dbfs_get_cur_part() != DB_PART_BACKUP)
                    g_xDB->aiValid[i] = 0;
            }

            iCheckSum = GetIntCheckSum((int*)&g_xDB->ax[i].xF, sizeof(SFeatInfo));
            if(iCheckSum != g_xDB->ax[i].iFeatCheckSum)
            {
                my_printf("checksum2 error : %d\n", i);

                iCheckSumErr = 1;
                g_aiDBValid[i] = 0;

                if (dbfs_get_cur_part() != DB_PART_BACKUP)
                    g_xDB->aiValid[i] = 0;
            }
        }
    }
#else // MMAP_MODE
    memset(&g_xDB, 0, sizeof(g_xDB));
    int FILESIZE = sizeof(DB_INFO);

    struct stat filecheck;
    int fd = -1;

    if(stat (DB_USERINFO_DAT, &filecheck) != 0 || filecheck.st_size != FILESIZE)
    {
        if(stat (DB_USERINFO_DAT, &filecheck) == 0 && filecheck.st_size != FILESIZE)
        {
            my_printf("#######  MMap FileSize Error!!! Src: %x, Err: %x\n", FILESIZE, filecheck.st_size);
            dbm_Free();

            return ES_FILE_ERROR;
        }

        fd = open(DB_USERINFO_DAT, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

        // Stretch the file size to the size of the (mmapped) array of ints
        int result = lseek(fd, FILESIZE-1, SEEK_SET);
        if (result == -1)
        {
            my_printf("Error calling lseek() to 'stretch' the file\n");
            close(fd);
            dbm_Free();

            return ES_FILE_ERROR;
        }

        result = write(fd, "", 1);
        if (result != 1)
        {
            my_printf("Error writing last byte of the file\n");
            close(fd);
            dbm_Free();

            return ES_FILE_ERROR;
        }
    }
    else
    {
        if (dbfs_get_cur_part() < DB_PART_BACKUP)
            fd = open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
        else
            fd = open(DB_USERINFO_DAT, O_RDONLY, (mode_t)0600);
    }

    if (fd < 0)
    {
        my_printf("[%s]failed to open db file(%s).\n", __FUNCTION__, strerror(errno));
        return ES_FILE_ERROR;
    }

    my_printf("Dbm_Init fd: %d\n", fd);

    lseek(fd, 0, SEEK_SET);
    int nReadNum = read(fd, &g_xDB, sizeof(DB_INFO));
    if (nReadNum != sizeof(DB_INFO))
    {
        my_printf("[%s] failed to read user info %d\n", __FUNCTION__, nReadNum);
        dbm_Free();

        return ES_FILE_ERROR;
    }

    memset(g_aiDBValid, 0, sizeof(int) * N_MAX_PERSON_NUM);

    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(g_xDBreal->aiValid[i] == 0)
            continue;

        g_aiDBValid[i] = 1;
        int iCheckSum = GetIntCheckSum((int*)&g_xDBreal->ax[i].xM, sizeof(SMetaInfo));
        if(iCheckSum != g_xDBreal->ax[i].iMetaCheckSum)
        {
            my_printf("checksum1 error : %d\n", i);
            iCheckSumErr = 1;
            g_aiDBValid[i] = 0;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xDBreal->aiValid[i] = 0;
                lseek(fd, g_xDBreal->ax[i].xM.iID * sizeof(int), SEEK_SET);
                write(fd, 0, sizeof(int));
            }
        }

        iCheckSum = GetIntCheckSum((int*)&g_xDBreal->ax[i].xF, sizeof(SFeatInfo));
        if(iCheckSum != g_xDBreal->ax[i].iFeatCheckSum)
        {
            my_printf("checksum2 error : %d\n", i);

            iCheckSumErr = 1;
            g_aiDBValid[i] = 0;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xDBreal->aiValid[i] = 0;
                lseek(fd, g_xDBreal->ax[i].xM.iID * sizeof(int), SEEK_SET);
                write(fd, 0, sizeof(int));
            }
        }
    }

    if(fd >= 0)
    {
        fsync(fd);
        close(fd);
    }
#endif
#else // !__RTK_OS__
#ifdef MMAP_MODE
    memset(getDBPtr(), 0, sizeof(DB_INFO));
    int FILESIZE = sizeof(DB_INFO);
    int real_file_len;

    LOG_PRINT("[%s] start, %d, %d, %d\n", __func__, FILESIZE, sizeof(DB_UNIT), sizeof(g_xDB->aiValid));

    if (dbfs_get_cur_part() == DB_PART1)
        g_iUserdbAddr = USERDB_START_ADDR;
    else
        g_iUserdbAddr = USERDB_START_ADDR + USERDB_SIZE;

    real_file_len = my_flash_read(g_iUserdbAddr, FILESIZE, g_xDB, FILESIZE);

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
        if(g_xDB->aiValid[i] == 0)
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
#else // MMAP_MODE
    memset(getDBPtr(), 0, sizeof(DB_INFO));
    int FILESIZE = sizeof(DB_INFO);
    int real_file_len;
    myfdesc_ptr fd = NULL;

    LOG_PRINT("[%s] start\n", __func__);

    fd = my_open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
    if (!is_myfdesc_ptr_valid(fd))
    {
        my_create_empty_file(DB_USERINFO_DAT, FILESIZE);
        //reopen file
        fd = my_open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
    }
    if (fd == NULL)
    {
        my_printf("[%s]failed to open db file.\n", __FUNCTION__);
        dbm_Free();
        return ES_FILE_ERROR;
    }

    //get real file size
    real_file_len = my_seek(fd, 0, SEEK_END);
    if (real_file_len != FILESIZE)
    {
        my_printf("file size not match: %d, %d\n", real_file_len, FILESIZE);
        dbm_Free();
        return ES_FILE_ERROR;
    }

    LOG_PRINT("[%s] fd: %p\n", __func__, fd);

    my_seek(fd, 0, SEEK_SET);
    int nReadNum = my_read(fd, g_xDBreal, sizeof(DB_INFO));
    if (nReadNum != sizeof(DB_INFO))
    {
        my_printf("[%s] failed to read user info %d\n", __FUNCTION__, nReadNum);
        dbm_Free();

        return ES_FILE_ERROR;
    }

    memset(g_aiDBValid, 0, sizeof(int) * N_MAX_PERSON_NUM);

    for(int i = 0; i < N_MAX_PERSON_NUM; i ++)
    {
        if(g_xDBreal->aiValid[i] == 0)
            continue;
        LOG_PRINT("[%s] u%d, v%d\n", __func__, i, g_xDBreal->aiValid[i]);

        g_aiDBValid[i] = 1;
        int iCheckSum = GetIntCheckSum((int*)&g_xDBreal->ax[i].xM, sizeof(SMetaInfo));
        if(iCheckSum != g_xDBreal->ax[i].iMetaCheckSum)
        {
            // my_printf("checksum1 error : %d\n", i);
            iCheckSumErr = 1;
            g_aiDBValid[i] = 0;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xDBreal->aiValid[i] = 0;
                my_seek(fd, g_xDBreal->ax[i].xM.iID * sizeof(int), SEEK_SET);
                my_write(fd, 0, sizeof(int));
            }
        }

        iCheckSum = GetIntCheckSum((int*)&g_xDBreal->ax[i].xF, sizeof(SFeatInfo));
        if(iCheckSum != g_xDBreal->ax[i].iFeatCheckSum)
        {
            // my_printf("checksum2 error : %d\n", i);

            iCheckSumErr = 1;
            g_aiDBValid[i] = 0;

            if (dbfs_get_cur_part() != DB_PART_BACKUP)
            {
                g_xDBreal->aiValid[i] = 0;
                my_seek(fd, g_xDBreal->ax[i].xM.iID * sizeof(int), SEEK_SET);
                my_write(fd, 0, sizeof(int));
            }
        }
    }

    if(fd != NULL)
    {
        my_close(fd);
    }
#endif // MMAP_MODE
#endif // !__RTK_OS__
    if(iCheckSumErr)
    {
        my_printf("checksum error\n");
        return ES_FILE_ERROR;
    }


    return ES_SUCCESS;
}

void dbm_SetEmptyPersonDB(int* piBlkNum)
{
    // if (dbfs_get_cur_part() == DB_PART_BACKUP)
    //     return;

#ifdef MMAP_MODE
    if (g_xDB != NULL/* && g_xDB != MAP_FAILED*/)
    {
        memset(g_xDB, 0, sizeof(DB_INFO));
        memset(g_aiDBValid, 0, sizeof(int) * N_MAX_PERSON_NUM);
        dbm_FlushUserDB(0, DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP_BIT);
    }

    if(piBlkNum && *piBlkNum == 0)
    {
        //kkk test
        // CheckBackupDB();
        // FILE* fp = fopen(BACKUP_USERINFO_DAT, "r+b");
        // if(fp)
        // {
        //     fwrite(g_xDB, sizeof(DB_INFO), 1, fp);
        //     fsync(fileno(fp));
        //     fclose(fp);
        // }
    }
#else // MMAP_MODE
    myfdesc_ptr fd = NULL;
    fd = my_open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
    if(!is_myfdesc_ptr_valid(fd))
    {
        my_printf("[%s]failed to open db file.\n", __FUNCTION__);
        return;
    }

    memset(g_xDBreal, 0, sizeof(DB_INFO));
    memset(g_aiDBValid, 0, sizeof(int) * N_MAX_PERSON_NUM);

    my_seek(fd, 0, SEEK_SET);
    my_write(fd, g_xDBreal->aiValid, sizeof(DB_INFO::aiValid));
    my_fsync(fd);
    my_close(fd);

    if(piBlkNum && *piBlkNum == 0)
    {
        CheckBackupDB();
        fd = my_open(BACKUP_USERINFO_DAT, O_RDWR | O_CREAT, (mode_t)0777);
        if(is_myfdesc_ptr_valid(fd))
        {
            my_write(fd, g_xDBreal->aiValid, sizeof(DB_INFO::aiValid));
            my_fsync(fd);
            my_close(fd);
        }
    }
#endif // MMAP_MODE
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

    // SMetaInfo xMetaInfo = *pxUserInfo;
    // SFeatInfo xFeatInfo = *pxFeatInfo;

    dbug_printf("[%s] ID=%d\n", __func__, pxUserInfo->iID);

#ifdef MMAP_MODE
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

    if(piBlkNum && *piBlkNum == 0)
    {
        //kkk test
        // CheckBackupDB();
        // FILE* fp = fopen(BACKUP_USERINFO_DAT, "r+b");
        // if(fp)
        // {
        //     fseek(fp, pxUserInfo->iID * sizeof(int), SEEK_SET);
        //     fwrite(&g_aiDBValid[pxUserInfo->iID], sizeof(int), 1, fp);
        //     fseek(fp, sizeof(DB_INFO::aiValid) + pxUserInfo->iID * sizeof(DB_UNIT), SEEK_SET);
        //     fwrite(&g_xDB->ax[pxUserInfo->iID], sizeof(DB_UNIT), 1, fp);
        //     fsync(fileno(fp));
        //     fclose(fp);
        // }
    }
#else // MMAP_MODE
    myfdesc_ptr fd = my_open(DB_USERINFO_DAT, O_RDWR, (mode_t)0777);
    if(!is_myfdesc_ptr_valid(fd))
    {
        my_printf("[%s]failed to open db file.\n", __FUNCTION__);
        return ES_FAILED;
    }

    int _file_len = my_seek(fd, 0, SEEK_END);
    LOG_PRINT("[%s] ul=%d\n", __func__, _file_len);
    my_seek(fd, 0, SEEK_SET);

    g_xDBreal->aiValid[pxUserInfo->iID] = 1;
    g_aiDBValid[pxUserInfo->iID] = 1;

    g_xDBreal->ax[pxUserInfo->iID].xM = *pxUserInfo;
    g_xDBreal->ax[pxUserInfo->iID].xF = *pxFeatInfo;

    int iCheckSum = GetIntCheckSum((int*)pxUserInfo, sizeof(SMetaInfo));
    g_xDBreal->ax[pxUserInfo->iID].iMetaCheckSum = iCheckSum;

    iCheckSum = GetIntCheckSum((int*)pxFeatInfo, sizeof(SFeatInfo));
    g_xDBreal->ax[pxUserInfo->iID].iFeatCheckSum = iCheckSum;

    my_seek(fd, pxUserInfo->iID * sizeof(int), SEEK_SET);
    my_write(fd, &g_aiDBValid[pxUserInfo->iID], sizeof(int));
    my_seek(fd, sizeof(DB_INFO::aiValid) + pxUserInfo->iID * sizeof(DB_UNIT), SEEK_SET);
    my_write(fd, &g_xDBreal->ax[pxUserInfo->iID], sizeof(DB_UNIT));
    my_fsync(fd);
    my_close(fd);

    if(piBlkNum && *piBlkNum == 0)
    {
        dbug_printf("[%s] going to write backup\n", __func__);
        CheckBackupDB();
        fd = my_open(BACKUP_USERINFO_DAT, O_RDWR | O_CREAT, (mode_t)0777);
        if(is_myfdesc_ptr_valid(fd))
        {
            _file_len = my_seek(fd, 0, SEEK_END);
            dbug_printf("[%s] bl=%d\n", __func__, _file_len);
            my_seek(fd, 0, SEEK_SET);
            my_seek(fd, pxUserInfo->iID * sizeof(int), SEEK_SET);
            my_write(fd, &g_aiDBValid[pxUserInfo->iID], sizeof(int));
            my_seek(fd, sizeof(DB_INFO::aiValid) + pxUserInfo->iID * sizeof(DB_UNIT), SEEK_SET);
            my_write(fd, &g_xDBreal->ax[pxUserInfo->iID], sizeof(DB_UNIT));
            my_fsync(fd);
            my_close(fd);
            dbug_printf("[%s] write backup end\n", __func__);
        }
    }
#endif // MMAP_MODE
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
    if (dbm_UpdatePersonFeatInfo(nIndex, pxFeatInfo, piBlkNum) != ES_SUCCESS)
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

#ifdef MMAP_MODE
    int iCheckSum = GetIntCheckSum((int*)pxUserInfo, sizeof(SMetaInfo));
    g_xDB->ax[iID].iMetaCheckSum = iCheckSum;
    g_xDB->ax[iID].xM = *pxUserInfo;

    if (dbm_FlushUserDB(iID, DB_FLUSH_FLAG_DATA) != ES_SUCCESS)
        return ES_FAILED;

    if(piBlkNum && *piBlkNum == 0)
    {
        //kkk test
        // CheckBackupDB();
        // FILE* fp = fopen(BACKUP_USERINFO_DAT, "r+b");
        // if(fp)
        // {
        //     fseek(fp, sizeof(DB_INFO::aiValid) + iID * sizeof(DB_UNIT), SEEK_SET);
        //     fwrite(pxUserInfo, sizeof(SMetaInfo), 1, fp);
        //     fwrite(&iCheckSum, sizeof(int), 1, fp);
        //     fsync(fileno(fp));
        //     fclose(fp);
        // }
    }
#else // MMAP_MODE
    myfdesc_ptr fd = my_open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
    if(!is_myfdesc_ptr_valid(fd))
    {
        my_printf("[%s]failed to open db file.\n", __FUNCTION__);
        return ES_FAILED;
    }

    int iCheckSum = GetIntCheckSum((int*)pxUserInfo, sizeof(SMetaInfo));
    g_xDBreal->ax[iID].iMetaCheckSum = iCheckSum;
    g_xDBreal->ax[iID].xM = *pxUserInfo;

    my_seek(fd, sizeof(DB_INFO::aiValid) + iID * sizeof(DB_UNIT), SEEK_SET);
    my_write(fd, &g_xDBreal->ax[iID], sizeof(DB_UNIT));
    my_fsync(fd);
    my_close(fd);

    if(piBlkNum && *piBlkNum == 0)
    {
        CheckBackupDB();
        fd = my_open(BACKUP_USERINFO_DAT, O_RDWR, (mode_t)0600);
        if(is_myfdesc_ptr_valid(fd))
        {
            my_seek(fd, sizeof(DB_INFO::aiValid) + iID * sizeof(DB_UNIT), SEEK_SET);
            my_write(fd, pxUserInfo, sizeof(SMetaInfo));
            my_write(fd, &iCheckSum, sizeof(int));
            my_fsync(fd);
            my_close(fd);
        }
    }
#endif // MMAP_MODE
    return ES_SUCCESS;
}

int	dbm_UpdatePersonFeatInfo(int nIndex, PSFeatInfo pxFeatInfo, int* piBlkNum)
{
    if (dbfs_get_cur_part() == DB_PART_BACKUP)
        return ES_FAILED;

    if(pxFeatInfo == NULL)
        return ES_FAILED;

    int iID = dbm_GetIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;

#ifdef MMAP_MODE
    int iCheckSum = GetIntCheckSum((int*)pxFeatInfo, sizeof(SFeatInfo));
    g_xDB->ax[iID].iFeatCheckSum = iCheckSum;
    g_xDB->ax[iID].xF = *pxFeatInfo;

    if (dbm_FlushUserDB(iID, DB_FLUSH_FLAG_DATA) != ES_SUCCESS)
        return ES_FAILED;

    if(piBlkNum && *piBlkNum == 0)
    {
        //kkk test
        // CheckBackupDB();
        // FILE* fp = fopen(BACKUP_USERINFO_DAT, "r+b");
        // if(fp)
        // {
        //     fseek(fp, sizeof(DB_INFO::aiValid) + iID * sizeof(DB_UNIT) + sizeof(SMetaInfo) + sizeof(int), SEEK_SET);
        //     fwrite(pxFeatInfo, sizeof(SFeatInfo), 1, fp);
        //     fwrite(&iCheckSum, sizeof(int), 1, fp);
        //     fsync(fileno(fp));
        //     fclose(fp);
        // }
    }
#else // MMAP_MODE
    myfdesc_ptr fd = my_open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
    if(!is_myfdesc_ptr_valid(fd))
    {
        my_printf("[%s]failed to open db file.\n", __FUNCTION__);
        return ES_FAILED;
    }

    int iCheckSum = GetIntCheckSum((int*)pxFeatInfo, sizeof(SFeatInfo));
    g_xDBreal->ax[iID].iFeatCheckSum = iCheckSum;
    g_xDBreal->ax[iID].xF = *pxFeatInfo;

    my_seek(fd, sizeof(DB_INFO::aiValid) + iID * sizeof(DB_UNIT), SEEK_SET);
    my_write(fd, &g_xDBreal->ax[iID], sizeof(DB_UNIT));
    my_fsync(fd);
    my_close(fd);

    if(piBlkNum && *piBlkNum == 0)
    {
        CheckBackupDB();
        fd = my_open(BACKUP_USERINFO_DAT, O_RDWR, (mode_t)0600);
        if(fd)
        {
            my_seek(fd, sizeof(DB_INFO::aiValid) + iID * sizeof(DB_UNIT) + sizeof(SMetaInfo) + sizeof(int), SEEK_SET);
            my_write(fd, pxFeatInfo, sizeof(SFeatInfo));
            my_write(fd, &iCheckSum, sizeof(int));
            my_fsync(fd);
            my_close(fd);
        }
    }
#endif // MMAP_MODE
    return ES_SUCCESS;
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
#ifdef MMAP_MODE
            memcpy(pxUserInfo, &g_xDB->ax[i].xM, sizeof(SMetaInfo));
            if(pxFeatInfo != NULL)
                memcpy(pxFeatInfo, &g_xDB->ax[i].xF, sizeof(SFeatInfo));
#else
            memcpy(pxUserInfo, &g_xDBreal->ax[i].xM, sizeof(SMetaInfo));
            if(pxFeatInfo != NULL)
                memcpy(pxFeatInfo, &g_xDBreal->ax[i].xF, sizeof(SFeatInfo));
#endif
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
#ifdef MMAP_MODE
    return &g_xDB->ax[nID].xM;
#else
    return &g_xDBreal->ax[nID].xM;
#endif
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
#ifdef MMAP_MODE
    return &g_xDB->ax[nID].xF;
#else
    return &g_xDBreal->ax[nID].xF;
#endif
}

PSMetaInfo dbm_GetPersonMetaInfoByIndex(int nPos)
{
    int iID = dbm_GetIDOfIndex(nPos);
    if(iID < 0)
        return NULL;
#ifdef MMAP_MODE
    return &g_xDB->ax[iID].xM;
#else
    return &g_xDBreal->ax[iID].xM;
#endif
}

PSFeatInfo dbm_GetPersonFeatInfoByIndex(int nPos)
{
    int iID = dbm_GetIDOfIndex(nPos);
    if(iID < 0)
        return NULL;

#ifdef MMAP_MODE
    return &g_xDB->ax[iID].xF;
#else
    return &g_xDBreal->ax[iID].xF;
#endif
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
#ifdef MMAP_MODE
    g_xDB->aiValid[nID] = 0;
    g_aiDBValid[nID] = 0;
    dbm_FlushUserDB(nID, DB_FLUSH_FLAG_BIT | DB_FLUSH_FLAG_BACKUP_BIT);
#else // MMAP_MODE
    myfdesc_ptr fd = my_open(DB_USERINFO_DAT, O_RDWR, (mode_t)0600);
    if(!is_myfdesc_ptr_valid(fd))
    {
        my_printf("[%s]failed to open db file.\n", __FUNCTION__);
        return ES_FAILED;
    }

    g_xDBreal->aiValid[nID] = 0;
    g_aiDBValid[nID] = 0;

    my_seek(fd, 0, SEEK_SET);
    my_write(fd, g_xDBreal->aiValid, sizeof(DB_INFO::aiValid));
    my_fsync(fd);
    my_close(fd);

    if(piBlkNum && *piBlkNum == 0)
    {
        CheckBackupDB();
        fd = my_open(BACKUP_USERINFO_DAT, O_RDWR, (mode_t)0600);
        if(fd)
        {
            my_seek(fd, nID * sizeof(int), SEEK_SET);
            my_write(fd, &g_aiDBValid[nID], sizeof(int));
            my_fsync(fd);
            my_close(fd);
        }
    }
#endif // MMAP_MODE
    return ES_SUCCESS;
}

int dbm_RemovePersonByIndex(int nIndex, int* piBlkNum)
{
    int iID = dbm_GetIDOfIndex(nIndex);
    if(iID < 0)
        return ES_FAILED;

    return dbm_RemovePersonByID(iID, piBlkNum);
}

int dbm_FlushUserDB(int nUserID, int nFlushData)
{
    int FILESIZE = sizeof(DB_INFO);

    LOG_PRINT("[%s] start, userid=%d, %0.3f\n", __func__, nUserID, Now());

    if (dbfs_get_cur_part() == DB_PART1)
    {
        if (nUserID == -1)
        {
            my_flash_write(USERDB_START_ADDR, g_xDB, FILESIZE);
            //backup
            my_flash_write(USERDB_START_ADDR + USERDB_SIZE, g_xDB, FILESIZE);
        }
        else
        {
            if (nFlushData & DB_FLUSH_FLAG_DATA)
            {
                my_flash_write(USERDB_START_ADDR + sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), 
                    ((char*)g_xDB) + sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), sizeof(DB_UNIT));
            }
            if (nFlushData & DB_FLUSH_FLAG_BIT)
                my_flash_write(USERDB_START_ADDR, g_xDB->aiValid, sizeof(g_xDB->aiValid));
            //backup
            if (nFlushData & DB_FLUSH_FLAG_BACKUP)
            {
                my_flash_write(USERDB_START_ADDR + USERDB_SIZE + sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), 
                    ((char*)g_xDB) + sizeof(DB_INFO::aiValid) + nUserID * sizeof(DB_UNIT), sizeof(DB_UNIT));
            }
            if (nFlushData & DB_FLUSH_FLAG_BACKUP_BIT)
            {
                my_flash_write(USERDB_START_ADDR + USERDB_SIZE, g_xDB->aiValid, sizeof(g_xDB->aiValid));
            }
        }
        LOG_PRINT("[%s] write end, %0.3f\n", __func__, Now());
    }
    else
        return 1;

    return 0;
}

DATETIME_32 dbm_GetCurDateTime()
{
    DATETIME_32 xTime;
    // int uResetTime = 0;

    time_t curTime = time(NULL);
    struct tm* localTime = localtime(&curTime);

    // if (localTime->tm_mon < 0 || localTime->tm_mon > 11)
    //     uResetTime = 1;
    // else if (localTime->tm_mday < 1 || localTime->tm_mday > 31)
    //     uResetTime = 1;
    // else if (localTime->tm_hour < 0 || localTime->tm_hour > 23)
    //     uResetTime = 1;
    // else if (localTime->tm_min < 0 || localTime->tm_min > 59)
    //     uResetTime = 1;
    // else if (localTime->tm_sec < 0 || localTime->tm_sec > 59)
    //     uResetTime = 1;
    // else if (localTime->tm_wday < 0 || localTime->tm_wday > 6)
    //     uResetTime = 1;

    int iYear = (localTime->tm_year + 1900) - 2000;
    if(iYear < 0)
        iYear =  0;

    if(iYear >= 64)
        iYear = 64;

    xTime.x.iYear = iYear;
    xTime.x.iMon = localTime->tm_mon;
    xTime.x.iDay = localTime->tm_mday;
    xTime.x.iHour = localTime->tm_hour;
    xTime.x.iMin = localTime->tm_min;
    xTime.x.iSec = localTime->tm_sec;

    return xTime;
}

double dbm_GetDiffSec(DATETIME_32 a1, DATETIME_32 b1)
{
    struct tm a = {0};
    a.tm_year = (a1.x.iYear + 2000) - 1900;
    a.tm_mon = a1.x.iMon;
    a.tm_mday = a1.x.iDay;
    a.tm_hour = a1.x.iHour;
    a.tm_min = a1.x.iMin;
    a.tm_sec = a1.x.iSec;

    struct tm b = {0};
    b.tm_year = (b1.x.iYear + 2000) - 1900;
    b.tm_mon = b1.x.iMon;
    b.tm_mday = b1.x.iDay;
    b.tm_hour = b1.x.iHour;
    b.tm_min = b1.x.iMin;
    b.tm_sec = b1.x.iSec;

    time_t x = mktime(&a);
    time_t y = mktime(&b);

    if ( x != (time_t)(-1) && y != (time_t)(-1) )
    {
        double difference = difftime(x, y);
        return difference;
    }

    return -1;
}


DATETIME_32 time_tToDATETIME_32(time_t tTime)
{
    DATETIME_32 xRet = { 0 };
    struct tm* localTime = localtime(&tTime);
    xRet.x.iYear = localTime->tm_year + 1900 - 2000;
    xRet.x.iMon = localTime->tm_mon;
    xRet.x.iDay = localTime->tm_mday;
    xRet.x.iHour = localTime->tm_hour;
    xRet.x.iMin = localTime->tm_min;
    xRet.x.iSec = localTime->tm_sec;

    return xRet;
}

// void LOG_PRINT(const char * format, ...)
// {
// //    if(g_iDebugEn == 1)
// //    {
// //        va_list args;
// //        va_start (args, format);
// //        vprintf (format, args);
// //        va_end (args);
// //    }
// }


//static int g_iCheckCount = 0; //fixed: upx segment fault
int CheckFileSystem(int iIndex)
{
#if 0 //kkk test
    char szCmd[128];
    char dir_path[128];
    my_printf("[%s] start,%d,%d,\n", __FUNCTION__, iIndex, g_iCheckCount);
    if(g_iCheckCount > 5)
        return 0;

    g_iCheckCount++;

    int cur_part = dbfs_get_cur_part();
    if (cur_part >= DB_PART_END)
    {
        my_printf("[%s]failed to get db part.\n", __FUNCTION__);
        return 0;
    }

    if ( cur_part >= DB_PART_BACKUP)
        return 0;

    strcpy(dir_path, "/db");

    sync();

    sprintf(szCmd, "umount -l -f %s", dbfs_part_names[cur_part]);
    my_system(szCmd);

    sprintf(szCmd, "e2fsck -y %s", dbfs_part_names[cur_part]);
    my_system(szCmd);

    if (mount(dbfs_part_names[cur_part], dir_path, DB_FSTYPE, MS_SYNCHRONOUS | MS_NOATIME, ""))
    {
        my_printf("[%s]failed to mount device %s.\n", __FUNCTION__, dbfs_part_names[cur_part]);
        return 0;
    }

    sprintf(szCmd, "rm -rf %s/lost+found", dir_path);
    my_system(szCmd);
#endif
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
#ifndef __RTK_OS__
    int FILESIZE = sizeof(DB_INFO);
    struct stat filecheck;
    if(stat (BACKUP_USERINFO_DAT, &filecheck) != 0 || filecheck.st_size != FILESIZE)
    {
        int fd = open(BACKUP_USERINFO_DAT, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
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
#else // ! __RTK_OS__
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
        dbug_printf("[%s] file length mismatch %d, %d\n", real_file_len, FILESIZE);
        my_create_empty_file(BACKUP_USERINFO_DAT, FILESIZE);
    }
#endif  // ! __RTK_OS__
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

