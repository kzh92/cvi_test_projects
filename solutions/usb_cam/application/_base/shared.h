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

void    WriteAverageBatt(int batt);
int     ReadAverageBatt();

int     ReadLowBatteryCount();
int     UpdateLowBatteryCount(int iLow_count);

void    ResetDetectTimeout();
void    SetCurDateTime(DATETIME_32 xTime, int iSend);

int     GetVoltage();

void    CSI_PWDN_ON();
void    CSI_PWDN_ON1();

int     CheckRequestButton();
void    ShowUsbUpgradeFail();

void    GetSerialNumber(char* serialData);
int     GetV3SID(unsigned int* piV3SID);
unsigned long long GetSSDID(unsigned int* piSSDID);
void    GetUniquID(char* szDst);
void GetSN(char* serialData);

///////////////TTF Font//////////////////
char*       my_fgetws(MY_WCHAR* out, int max_len, char* pos);
void        wcsprint(const MY_WCHAR* text);
char*       my_strupr(char* str);
////////////////////////////////////

extern float    g_rLastDetectTime;

void        PrintFreeMem();
void        ClearCache();


void    UpdateUserCount();

void    md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);

#if (NFS_DEBUG_EN == 0)
#define APP_LIB_PATH "/usr/lib/libfaceengine.so"
#else
#define APP_LIB_PATH "/mnt/libfaceengine.so"
#endif

#endif // SHARED_H
