
#ifndef _I2C_BASE_H_
#define _I2C_BASE_H_

#ifndef _APP_UPGRADE
#include "EngineStruct.h"
#endif
#include "appdef.h"
#ifndef __RTK_OS__
#include "mutex.h"
#endif


#define WORD_SIZE           16
#define WORD_SIZE_XX        16

#define I2C_ADDR_M24C64         (0x50)

/////////////////////////Base ///////////////////////////

int I2C_Open(const char* szFilePath, int iAddr, int iMode);
int I2C_Close(int iFile);

int I2C_SetPointer8(int iFile, int iAddr);
int I2C_Read8(int iFile, int iAddr, unsigned char* pbData, int iLen);
int I2C_Write8(int iFile, int iAddr, unsigned char* pbData, int iLen);

/////////////////////////24C64/////////////////////////////

#define ADDR_ROK_LOGS           0x0000
#define ADDR_HEAD               0x0010 // 1 head info x 16 byte
#define ADDR_CS                 0x0020 // 1 common settings x 16 byte
#define ADDR_BACKUP_CS          0x0030 //  x 16 byte        //Backup된 Common Settings자료
#define ADDR_PS                 0x0040 //  x 16 byte        //Permaeace Settings
#define ADDR_HEAD2              0x0050 //  x 16 byte        //Permaeace Settings
#define ADDR_RECV_HDR           0x0060
#define ADDR_RECV_HDR_BAK       0x0070
#define ADDR_HEAD_BAK           0x0080 // 1 head info x 16 byte
//reserved
//#define ADDR_MAC                0x0080 // x 48byte          //ATSHA204a MAC
//#define ADDR_MAC_BAK            0x00B0 // x 48byte          //ATSHA204a MAC Backup
#define ADDR_CHK_FLAG           0x00D0 // x 16byte          //check flag
#define ADDR_ENC_KEYPOS         0x00E0 // x 32byte          //encryption key pos

#define ADDR_MY_ALL_SS			0x0000
#define ADDR_MY_ALL_SS_BAK		0x1000

#ifdef __cplusplus
extern  "C"
{
#endif // __cplusplus
int M24C64_ReadWordXX(int iFile, int iAddr, unsigned char* pbData);
int M24C64_WriteWordXX(int iFile, int iAddr, unsigned char* pbData);

int M24C64_Open();
void M24C64_Close();

int M24C64_FactoryReset();

int M24C64_GetCS(unsigned char* abData);
int M24C64_SetCS(unsigned char* abData);
int M24C64_Get16(unsigned char* abData, int iAddr);
int M24C64_Set16(unsigned char* abData, int iAddr);
int M24C64_GetBackupCS(unsigned char* abData);
int M24C64_SetBackupCS(unsigned char* abData);
int M24C64_GetPS(unsigned char* abData);
int M24C64_SetPS(unsigned char* abData);
int M24C64_GetHD(unsigned char* abData);
int M24C64_SetHD(unsigned char* abData);
int M24C64_GetBackupHD(unsigned char* abData);
int M24C64_SetBackupHD(unsigned char* abData);
int M24C64_GetHD2(unsigned char* abData);
int M24C64_SetHD2(unsigned char* abData);
int M24C64_GetBootingLogs(unsigned char* abData);
int M24C64_SetBootingLogs(unsigned char* abData);
int M24C64_GetES(unsigned char* abData);
int M24C64_SetES(unsigned char* abData);
void M24C64_SetResetFlag(int f);
int M24C64_IsResetFlag();

#ifdef __cplusplus
}
#endif // __cplusplus

#ifndef __RTK_OS__
extern Mutex g_xI2CMutex;
#endif // !__RTK_OS__
extern int g_iM24C64;
#endif //_M24C64_H_

