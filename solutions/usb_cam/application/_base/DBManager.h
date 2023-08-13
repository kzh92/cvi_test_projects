#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "EngineStruct.h"
#if (N_MAX_HAND_NUM)
#include "hand/HandRetrival_.h"
#endif

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

#if (N_MAX_HAND_NUM)
#define DB_HANDINFO_DAT "/db/handinfo.bin"
#define BACKUP_HANDINFO_DAT "/backup/handinfo.bin"

typedef struct _tagDB_HAND_UNIT
{
    SMetaInfo       xHM;
    int             iHMetaCheckSum;
    SHandFeatInfo   xHF;
    int             iHFCheckSum;
//    SMetaInfo  xHM;
//    int        iHMetaCheckSum;
//    SFeatInfo  xHF;
//    int        iHFCheckSum;

} DB_HAND_UNIT;

typedef struct _tagDB_HAND_INFO
{
    int   aiHandValid[N_MAX_HAND_NUM];
    DB_HAND_UNIT    ax[N_MAX_HAND_NUM];
} DB_HAND_INFO;

#endif // N_MAX_HAND_NUM

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
LIBFOO_DLL_EXPORTED  int		dbm_UpdatePersonFeatInfo(int nIndex, PSFeatInfo pxFeatInfo, int* piBlkNum, int iUpdateFeatIndex);
LIBFOO_DLL_EXPORTED  int        dbm_UpdatePersonBin(int iFlag, unsigned char* pData, int iLen, int iUserID);
LIBFOO_DLL_EXPORTED  unsigned char* dbm_GetPersonBin();
LIBFOO_DLL_EXPORTED  unsigned char* dbm_GetHandBin();
LIBFOO_DLL_EXPORTED  int		dbm_GetPersonCount();
LIBFOO_DLL_EXPORTED  int		dbm_GetTotalUserCount();
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
LIBFOO_DLL_EXPORTED  int        dbm_RemovePersonByPrivilege(int iPrivilege, int* piBlkNum);

//hand userinfo
#if (N_MAX_HAND_NUM)
LIBFOO_DLL_EXPORTED int         dbm_LoadHandDB();
LIBFOO_DLL_EXPORTED int         dbm_AddHand(PSMetaInfo pxUserInfo, SHandFeatInfo* pxFeatInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED int			dbm_UpdateHand(int nIndex, PSMetaInfo pxUserInfo, SHandFeatInfo* pxFeatInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int		dbm_UpdateHandMetaInfo(int nIndex, PSMetaInfo pxUserInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int		dbm_UpdateHandFeatInfo(int nIndex, SHandFeatInfo* pxFeatInfo, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int		dbm_GetHandCount();
LIBFOO_DLL_EXPORTED  int        dbm_GetHandIDOfIndex(int iIndex);
LIBFOO_DLL_EXPORTED  int        dbm_GetHandIndexOfID(int iID);
LIBFOO_DLL_EXPORTED  PSMetaInfo dbm_GetHandMetaInfoByIndex(int nPos);
LIBFOO_DLL_EXPORTED  SHandFeatInfo*	dbm_GetHandFeatInfoByIndex(int nPos);
LIBFOO_DLL_EXPORTED  PSMetaInfo dbm_GetHandMetaInfoByID(int iID);
LIBFOO_DLL_EXPORTED  SHandFeatInfo*	dbm_GetHandFeatInfoByID(int nID);
LIBFOO_DLL_EXPORTED  int        dbm_GetNewHandUserID();
LIBFOO_DLL_EXPORTED  void       dbm_SetEmptyHandDB(int* piBlkNum);
LIBFOO_DLL_EXPORTED  int        dbm_RemoveHandByID(int nID, int* piBlkNum);
LIBFOO_DLL_EXPORTED  int        dbm_CheckHandBackupDBInfos();
LIBFOO_DLL_EXPORTED  int        dbm_CheckHandBackupDB();
#endif // N_MAX_HAND_NUM

////////////////////////////////////////////////////////////////////////////////////////////////

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
