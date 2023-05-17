#ifndef SHARED_H
#define SHARED_H

#include "EngineStruct.h"
#include "settings.h"
#include "common_types.h"
#define UNIQUIE_MARK (0x55AA55AA)

#ifndef __min
#define __min(a,b)  ((a) > (b) ? (b) : (a))
#endif  //	MIN

#ifndef __max
#define __max(a,b)  ((a) < (b) ? (b) : (a))
#endif  //	MAX

typedef wchar_t MY_WCHAR;
#define STR_MAX_LEN 128
#define STR_MAX_PATH 512

#ifdef __cplusplus
extern  "C"
{
#endif

void    ResetDetectTimeout();
void    ShowUsbUpgradeFail();

void    GetSerialNumber(char* serialData);
unsigned long long GetSSDID(unsigned int* piSSDID);
void    GetUniquID(char* szDst);
void 	GetSN(char* serialData);
int 	GetAesKey4ChipID(void* buf);

extern float    g_rLastDetectTime;

void        PrintFreeMem();


void    UpdateUserCount();

void    md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);
#ifdef USE_TWIN_ENGINE
int     MainSTM_GetDict(unsigned char *pbData, int len);
#endif

#ifdef __cplusplus
}
#endif

#if (NFS_DEBUG_EN == 0)
#define APP_LIB_PATH "/usr/lib/libfaceengine.so"
#else
#define APP_LIB_PATH "/mnt/libfaceengine.so"
#endif

#endif // SHARED_H
