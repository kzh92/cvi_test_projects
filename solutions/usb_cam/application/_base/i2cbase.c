#include "i2cbase.h"
#include "settings.h"
// #include "cam_drv_i2c.h"
#include "common_types.h"
#include <string.h>

#ifndef _APP_UPGRADE
#ifndef __RTK_OS__
#include "DBManager.h"
#include "shared.h"
#endif // !__RTK_OS__
#include "drv_gpio.h"
#include "uartcomm.h"
#else // !_APP_UPGRADE
#endif // !_APP_UPGRADE

#ifndef __RTK_OS__
#include "mutex.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
//#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c.h>
#endif // !__RTK_OS__

//#define USE_IOCTL

int g_iM24C64 = -1;
#ifndef __RTK_OS__

Mutex g_xI2CMutex;
#define IS_24C64_OPEN() (g_iM24C64 > -1)

#else // !__RTK_OS__

// tI2cHandle g_i2cHandleM24C64 = {-1, NULL};
// #define IS_24C64_OPEN() (g_i2cHandleM24C64.nPortNum != -1)

#endif // !__RTK_OS__
static  int g_i24C04Reset = 0;


unsigned short Little16ToBig16(unsigned short sVal)
{
    return (sVal & 0xFF) << 8 | ((sVal >> 8) & 0xFF);
}

#ifndef __RTK_OS__
//////////////////////////////////////////////I2C Base////////////////////////////////////////////
/**
 * @brief I2C_Open
 * @param szFilePath
 * @param iAddr
 * @param iMode
 * @return
 */
int I2C_Open(const char* szFilePath, int iAddr, int iMode)
{
    int iFile = -1;
    if ((iFile = open(szFilePath,iMode)) < 0)
    {
        my_printf("Failed to open the bus: Addr = %x, %s\n", iAddr, strerror(errno));
        return 0;
    }
#ifndef USE_IOCTL
    if (ioctl(iFile,I2C_SLAVE,iAddr) < 0)
    {
        my_printf("Failed to acquire bus access and/or talk to slave.\n");
        return 0;
    }
#endif
    return iFile;
}

/**
 * @brief I2C_Close
 * @param iFile
 * @return
 */
int I2C_Close(int iFile)
{
    close(iFile);
    return 0;
}

/**
 * @brief I2C_SetPointer8
 * @param iFile
 * @param iAddr
 * @return
 */
int I2C_SetPointer8(int iFile, int iAddr)
{
    char szBuf[2] = { 0 };
    if(iFile <= 0)
        return -1;

    szBuf[0] = iAddr;
    if (write(iFile, szBuf, 1) != 1)
    {
        return -1;
    }
    else
    {
    }

    return 0;
}
#endif // !__RTK_OS__

/**
 * @brief I2C_Read8
 * @param iFile
 * @param iAddr
 * @param pbData
 * @return
 */
int I2C_Read8_Sub(int iFile, int iAddr, unsigned char* pbData, int iLen)
{
    return (my_settings_read(iAddr, pbData, iLen) > 0 ? 0: -1);
}

int I2C_Read8(int iFile, int iAddr, unsigned char* pbData, int iLen)
{
    int ret = 0;
    for (int i = 0; i < 3; i ++)
    {
        ret = I2C_Read8_Sub(iFile, iAddr, pbData, iLen);
        if (ret == 0)
            break;
        my_printf("[%s]retry %d, addr=%08x, len=%d\n", __func__, i, iAddr, iLen);
        my_usleep(10*1000);
    }
    return ret;
}
/**
 * @brief I2C_Write8
 * @param iFile
 * @param iAddr
 * @param pbData
 * @return
 */
int I2C_Write8_Sub(int iFile, int iAddr, unsigned char* pbData, int iLen)
{
    float rOld = Now();
    int ret = my_settings_write(iAddr, pbData, iLen);
    if (rOld > 0)
        dbug_printf("[%s]wtime: %0.3f, off=%08x, len=%d\n", __func__, Now() - rOld, iAddr, iLen);
    return ret > 0 ? 0: -1;
}

/*
* return 0:ok, other: fail
*/
int I2C_Write8(int iFile, int iAddr, unsigned char* pbData, int iLen)
{
    unsigned char _buf[WORD_SIZE*2];
    int read_len = iLen;
    if ((int)sizeof(_buf) < read_len)
    {
        read_len = (int)sizeof(_buf);
        my_printf("[%s]warning: length exceed.\n", __func__);
    }

    for (int i = 0; i < 3; i ++)
    {
        I2C_Write8_Sub(iFile, iAddr, pbData, iLen);
        memset(_buf, 0xff, sizeof(_buf));
        I2C_Read8(iFile, iAddr, _buf, read_len);
        if (!memcmp(_buf, pbData, read_len))
        {
            return 0;
        }
        my_printf("[%s]retry %d, addr=%08x, len=%d\n", __func__, i, iAddr, iLen);
    }

    return -1;
}

#ifndef __RTK_OS__
/**
 * @brief I2C_SetPointer16
 * @param iFile
 * @param iAddr
 * @return
 */
int I2C_SetPointer16(int iFile, int iAddr)
{
    char szBuf[4] = { 0 };
    if(iFile <= 0)
        return -1;

    szBuf[0] = (iAddr >> 8);
    szBuf[1] = (iAddr & 0xFF);
    if (write(iFile, szBuf, 2) != 2)
    {
//        my_printf("[I2C] Error setting pointer\n");
        return -1;
    }
    else
    {
//        LOG_PRINT("[I2C] address: w_0x%0*x\n", 2, szBuf[0]);
    }

    return 0;
}

/**
 * @brief I2C_Read16
 * @param iFile
 * @param iAddr
 * @param pbData
 * @return
 */
int I2C_Read16(int iFile, int iAddr, unsigned char* pbData, int iLen)
{
    int iRet = -1;
    g_xI2CMutex.Lock();
    if(iFile < 0)
    {
        g_xI2CMutex.Unlock();
        return -1;
    }

    iRet = I2C_SetPointer16(iFile, iAddr);
    if(iRet < 0)
    {
        g_xI2CMutex.Unlock();
        return -1;
    }

    if (read(iFile, pbData, iLen) != iLen)
    {
        my_printf("[I2C-2] Error reading %i bytes, %x\n", iLen, iAddr);
    }
    else
    {
        LOG_PRINT("[I2C-2] Read 0x%0*x: ", 2, iAddr);
        for (int i=0; i< iLen; i++)
            LOG_PRINT("%0*x, ", 2, pbData[i]);
        LOG_PRINT("\n");

        g_xI2CMutex.Unlock();
        return 0;
    }

    g_xI2CMutex.Unlock();
    return -1;
}

/**
 * @brief I2C_Write16
 * @param iFile
 * @param iAddr
 * @param pbData
 * @return
 */
int I2C_Write16(int iFile, int iAddr, unsigned char* pbData, int iLen)
{
    unsigned char abData[WORD_SIZE * 2];
    g_xI2CMutex.Lock();
    if(iFile < 0)
    {
        g_xI2CMutex.Unlock();
        return -1;
    }

    abData[0] = (iAddr >> 8) & 0xFF;
    abData[1] = (iAddr & 0xFF);
    memcpy(abData + 2, pbData, iLen);

    if (write(iFile, abData, iLen + 2) != iLen + 2)
    {
        my_printf("[I2C-2] Error writing %i bytes, %x\n", iLen, iAddr);
    }
    else
    {
        LOG_PRINT("[I2C-2] Write 0x%0*x: ", 2, iAddr);
        for (int i = 0; i < iLen; i++)
        {
            LOG_PRINT("%0*x, ", 2, pbData[i]);
        }
        LOG_PRINT("\n\r");
        my_usleep(10000);

        g_xI2CMutex.Unlock();
        return 0;
    }

    my_usleep(10000);
    g_xI2CMutex.Unlock();
    return -1;
}
#endif // !__RTK_OS__

/**
 * @brief M24C64_ReadWord
 * @param iFile
 * @param iAddr
 * @param pbData
 * @param iLen
 * @return
 */
int M24C64_ReadWordXX(int iFile, int iAddr, unsigned char* pbData)
{
    int iRet = -1;
    for(int i = 0; i < WORD_SIZE_XX / WORD_SIZE; i ++)
        iRet = I2C_Read8(iFile, iAddr + i * WORD_SIZE, pbData + i * WORD_SIZE, WORD_SIZE);

    return iRet;
}

/**
 * @brief M24C64_WriteWordXX
 * @param iFile
 * @param iAddr
 * @param pbData
 * @param iLen
 * @return
 */
int M24C64_WriteWordXX(int iFile, int iAddr, unsigned char* pbData)
{
    int iRet = -1;
    int iCount = (WORD_SIZE_XX / WORD_SIZE);
    for(int i = 0; i < iCount; i ++)
        iRet = I2C_Write8(iFile, iAddr + i * WORD_SIZE, pbData + i * WORD_SIZE, WORD_SIZE);

    return iRet;
}

/////////////////////////////////////24C64관리/////////////////////////////////

/**
 * @brief M24C64_Open
 * @return
 */
int M24C64_Open()
{
#ifndef __RTK_OS__
    if (g_iM24C64 > -1)
        return g_iM24C64;
    g_iM24C64 = I2C_Open("/dev/i2c-1", I2C_ADDR_M24C64, O_RDWR);
//    my_printf("M24C64: %d\n", g_iM24C64);
    return g_iM24C64;
#else // !__RTK_OS__
    // if (IS_24C64_OPEN())
    //     return 0;
// #if (USE_SSD210)
//     CamI2cOpen(&g_i2cHandleM24C64, 0);
// #else
//     CamI2cOpen(&g_i2cHandleM24C64, 1);
// #endif
    return 0;
#endif // !__RTK_OS__
}

/**
 * @brief M24C64_Close
 */
void M24C64_Close()
{
#ifndef __RTK_OS__
    if (g_iM24C64 > -1)
    {
        close(g_iM24C64);
        g_iM24C64 = -1;
    }
#else // !__RTK_OS__
    // if (IS_24C64_OPEN())
    // {
    //     CamI2cClose(&g_i2cHandleM24C64);
    // }
#endif // !__RTK_OS__
}

int M24C64_Read16(int iAddr, unsigned char* pbData)
{
    return I2C_Read8(g_iM24C64, iAddr, pbData, WORD_SIZE);
}

int M24C64_Write16(int iAddr, unsigned char* pbData)
{
    return I2C_Write8(g_iM24C64, iAddr, pbData, WORD_SIZE);
}

#ifndef _APP_UPGRADE

/**
 * @brief M24C64_FactoryReset: 24C64를 공장초기화한다.
 * @return
 */
int M24C64_FactoryReset()
{
    ResetHeadInfos();
    ResetHeadInfos2();
    ResetCommonSettings();
    ResetEncryptSettings();

    SetModifyUser(1);

    return 0;
}

#endif // !_APP_UPGRADE

int M24C64_Get16(unsigned char* abData, int iAddr)
{
    return I2C_Read8(g_iM24C64, iAddr, abData, WORD_SIZE);
}

int M24C64_Set16(unsigned char* abData, int iAddr)
{
    return I2C_Write8(g_iM24C64, iAddr, abData, WORD_SIZE);
}

/**
 * @brief M24C64_GetCS: 일반설정정보를 24C64에서 읽는다.
 * @param abData
 * @return
 */
int M24C64_GetCS(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_SetCS: 일반설정정보를 24C64에서 쓰기한다.
 * @param abData
 * @return
 */
int M24C64_SetCS(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_GetBackupCS: Backup된 일반설정정보를 24C64에서 읽는다.
 * @param abData
 * @return
 */
int M24C64_GetBackupCS(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_SetBackupCS: Backup된 일반설정정보를 24C64에 쓰기한다.
 * @param abData
 * @return
 */
int M24C64_SetBackupCS(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_GetPS: 영구설정정보를 24C64에서 읽는다.
 * @param abData
 * @return
 */
int M24C64_GetPS(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_SetPS: 영구설정정보를 24C64에 쓰기한다.
 * @param abData
 * @return
 */
int M24C64_SetPS(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_GetHD: Head정보를 24C64에서 읽는다.
 * @param abData
 * @return
 */
int M24C64_GetHD(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_SetHD: Head정보를 24C64에 쓰기한다.
 * @param abData
 * @return
 */
int M24C64_SetHD(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_GetBackupHD: Backup된 일반설정정보를 24C64에서 읽는다.
 * @param abData
 * @return
 */
int M24C64_GetBackupHD(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_SetBackupHD: Backup된 일반설정정보를 24C64에 쓰기한다.
 * @param abData
 * @return
 */
int M24C64_SetBackupHD(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_GetHD2: Head2정보를 24C64에서 읽는다.
 * @param abData
 * @return
 */
int M24C64_GetHD2(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_SetHD2: Head2정보를 24C64에 쓰기한다.
 * @param abData
 * @return
 */
int M24C64_SetHD2(unsigned char* abData)
{
    return 0;
}

int M24C64_GetBootingLogs(unsigned char* abData)
{
    return 0;
}

int M24C64_SetBootingLogs(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_GetES:
 * @param abData
 * @return
 */
int M24C64_GetES(unsigned char* abData)
{
    return 0;
}

/**
 * @brief M24C64_SetES: 일반설정정보를 24C64에서 쓰기한다.
 * @param abData
 * @return
 */
int M24C64_SetES(unsigned char* abData)
{
    return 0;
}

void M24C64_SetResetFlag(int f)
{
    g_i24C04Reset = f;
}

int M24C64_IsResetFlag()
{
    return g_i24C04Reset;
}