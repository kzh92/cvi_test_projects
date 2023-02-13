#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "EngineStruct.h"

#include <sys/time.h>

extern char*	g_szDataPath;
extern int  g_iDebugEn;

#ifndef __RTK_OS__
#define DB_USERINFO_DAT "/db/userinfo.bin"
#define BACKUP_USERINFO_DAT "/backup/userinfo.bin"
#if (NFS_DEBUG_EN)
#define MNT_USERINFO_DAT "/tmp1/userinfo.bin"
#define RESTORE_SRC "/tmp1"
#else // NFS_DEBUG_EN
#define MNT_USERINFO_DAT "/mnt/userinfo.bin"
#define RESTORE_SRC "/mnt"
#endif // NFS_DEBUG_EN
#else // !__RTK_OS__
#define DB_USERINFO_DAT "/mnt/db/userinfo.bin"
#define BACKUP_USERINFO_DAT "/mnt/backup/userinfo.bin"
#endif // !__RTK_OS__

#pragma pack(push, 1)

typedef struct _tagDB_UNIT
{
    SMetaInfo  xM;
    int        iMetaCheckSum;
    SFeatInfo  xF;
    int        iFeatCheckSum;
} DB_UNIT;

typedef struct _tagDB_INFO
{
    int         aiValid[N_MAX_PERSON_NUM];
    DB_UNIT     ax[N_MAX_PERSON_NUM];
} DB_INFO;

#pragma pack(pop)

#ifdef __cplusplus
extern	"C"
{
#endif

////////////////////////////////////////User Mangaement////////////////////////////////////////////////////
LIBFOO_DLL_EXPORTED  void		dbm_Init(char* szDataPath);
LIBFOO_DLL_EXPORTED int         dbm_Free();
LIBFOO_DLL_EXPORTED  int		dbm_LoadPersonDB();
LIBFOO_DLL_EXPORTED  void		dbm_SetEmptyPersonDB(int* piBlkNum);

LIBFOO_DLL_EXPORTED int         dbm_AddPerson(PSMetaInfo pxUserInfo, PSFeatInfo pxFeatInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED int			dbm_UpdatePerson(int nIndex, PSMetaInfo pxUserInfo, PSFeatInfo pxFeatInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int		dbm_UpdatePersonMetaInfo(int nIndex, PSMetaInfo pxUserInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int		dbm_UpdatePersonFeatInfo(int nIndex, PSFeatInfo pxFeatInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int		dbm_GetPersonCount();
LIBFOO_DLL_EXPORTED  int        dbm_GetIDOfIndex(int iIndex);
LIBFOO_DLL_EXPORTED  int        dbm_GetIndexOfID(int iID);
LIBFOO_DLL_EXPORTED  int        dbm_IsValidOfID(int iID);

LIBFOO_DLL_EXPORTED  int        dbm_GetNewUserID();
LIBFOO_DLL_EXPORTED  int        dbm_GetUserCount(int iUserRole = -1);
LIBFOO_DLL_EXPORTED int         dbm_GetUserInfoByIndex(int nIndex, PSMetaInfo pxMetaInfo, PSFeatInfo pxFeatInfo);

LIBFOO_DLL_EXPORTED  PSMetaInfo	dbm_GetPersonMetaInfoByID(int nID);
LIBFOO_DLL_EXPORTED  PSFeatInfo	dbm_GetPersonFeatInfoByID(int nID);
LIBFOO_DLL_EXPORTED  PSMetaInfo	dbm_GetPersonMetaInfoByIndex(int nPos);
LIBFOO_DLL_EXPORTED  PSFeatInfo	dbm_GetPersonFeatInfoByIndex(int nPos);
LIBFOO_DLL_EXPORTED  int		dbm_RemovePersonByID(int nID, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int		dbm_RemovePersonByIndex(int nIndex, int* piBlkNum);

////////////////////////////////////////////////////////////////////////////////////////////////

LIBFOO_DLL_EXPORTED  DATETIME_32 dbm_GetCurDateTime();
LIBFOO_DLL_EXPORTED  double      dbm_GetDiffSec(DATETIME_32 a, DATETIME_32 b);        //a - b
LIBFOO_DLL_EXPORTED  DATETIME_32 time_tToDATETIME_32(time_t tTime);

//LIBFOO_DLL_EXPORTED  void        LOG_PRINT(const char * format, ...);
LIBFOO_DLL_EXPORTED  int         CheckFileSystem(int iIndex);
LIBFOO_DLL_EXPORTED  int*        GetDebugEn();
LIBFOO_DLL_EXPORTED  int         CopyFile(char* szDstPath, char* szSrcPath);
LIBFOO_DLL_EXPORTED  int         cp_rm_dir1(const char *inputDir, bool is_copy, const char* destDir, const char* file_ext, const char* file_starts);
LIBFOO_DLL_EXPORTED  int         GetIntCheckSum(int* piData, int iSize);
LIBFOO_DLL_EXPORTED  int         CheckBackupDBInfos();
LIBFOO_DLL_EXPORTED  int         CheckBackupDB();

LIBFOO_DLL_EXPORTED  int         is_backup_partition();


#ifdef __cplusplus
}
#endif

#endif//DBMANAGER_H
