#include "check_firmware.h"
#include "upgrade_firmware.h"
#if (CHECK_FIRMWARE == 1)
#include "common_types.h"
#include "sha1.h"
#include "shared.h"
#ifndef UPGRADE_MODE
#include "FaceRetrievalSystem.h"
#endif
#include "drv_gpio.h"
#include "uartcomm.h"
#include "i2cbase.h"
#include <string.h>
#include <fcntl.h>

#pragma pack(push, 1)
struct _tagCheckFlag {
    unsigned int iMinCheck;
    unsigned char bMinRes;
    unsigned int iMaxCheck;
    unsigned char bMaxRes;
    unsigned int iFirstCheck;
    unsigned char bFirstRes;
    unsigned char bCheckSum;
};
#pragma pack(pop)

unsigned char SenseLockTask_Get_CheckSum(unsigned char* pbData, int iLen)
{
    unsigned char bCheckSum = 0;
    for(int i = 0; i < iLen; i ++)
        bCheckSum ^= pbData[i];

    return bCheckSum;
}

void doCheckFirmware()
{
    // ReadCommonSettings();
    // if (g_xCS.x.bCheckFirmware == 1)
    // {
    //     g_xCS.x.bCheckFirmware = 0;
    //     UpdateCommonSettings();
    // }
    int iErrorFlag = 0;
    int *file_data = NULL;

    if (g_xSS.iUsbHostMode)
    {
        int timeWLedlimit = 50;//5s
        GPIO_fast_setvalue(WHITE_LED, 1);
        while(timeWLedlimit--)
            my_usleep(100*1000);
        GPIO_fast_setvalue(WHITE_LED, 0);
    }

    int idx = 0;
    while(g_part_files[idx].m_filename != NULL)
    {
        file_data = (int*)my_malloc(g_part_files[idx].m_filesize);
        if (!file_data)
        {
            my_printf("check firmware, malloc failed.\n");
            break;
        }
        fr_ReadFileData(g_part_files[idx].m_filename, 0, file_data, g_part_files[idx].m_filesize);
        int sum = 0;
        for (int k = 0; k < g_part_files[idx].m_filesize / (int)sizeof(int); k ++)
        {
            sum = sum ^ file_data[k];
        }
        my_free(file_data);
        if (sum != g_part_files[idx].m_checksum)
        {
            iErrorFlag = idx + 1;
            if (iErrorFlag > 15)
                iErrorFlag = 15;
            my_printf("error %s: %08x <> %08x\n", g_part_files[idx].m_filename, sum, g_part_files[idx].m_checksum);
        }
        else
        {
            my_printf("pass ok: %s\n", g_part_files[idx].m_filename);
        }

        idx ++;
    }

    // my_printf("sv: %d.\n", g_xCS.x.bSecureFlag);
    // if (g_xCS.x.bSecureFlag != DEFAULT_SECURE_VALUE)
    // {
    //     iErrorFlag = 9;
    // }

    my_printf("xxxxxx: \n");
    unsigned char _buf[16];
    for (int i = 0x0000; i < 0x0100; i += 16)
    {
        M24C64_Get16(_buf, i);
        my_printf("%04x: ", i);
        for (int k = 0; k < 16; k ++)
            my_printf("%02x ", _buf[k]);
        my_printf("\n");
    }

    //mark check flag
    ReadHeadInfos2();
    my_printf("hd2, %d\n", g_xHD2.x.iBootingCount);
    _tagCheckFlag cf;
    M24C64_Get16((unsigned char*)&cf, ADDR_CHK_FLAG);
    int chksum = GetSettingsCheckSum((unsigned char*)&cf, sizeof(cf));
    for (int k = 0; k < 16; k ++)
        my_printf("%02x ", ((unsigned char*)&cf)[k]);
    my_printf("\n");
    if (chksum != cf.bCheckSum)
    {
        memset(&cf, 0, sizeof(cf));
        cf.iMinCheck = 0xffff;
        cf.iMaxCheck = 0;
    }
    else
    {
        if (cf.iMaxCheck > g_xHD2.x.iBootingCount)
        {
            memset(&cf, 0, sizeof(cf));
            cf.iMinCheck = 0xffff;
            cf.iMaxCheck = 0;
        }
    }

    {
        cf.iMaxCheck = g_xHD2.x.iBootingCount;
        cf.bMaxRes = iErrorFlag;
    }
    if (cf.iFirstCheck == 0)
    {
        cf.iFirstCheck = g_xHD2.x.iBootingCount;
        cf.bFirstRes = iErrorFlag;
    }
    else
    {
        if (g_xHD2.x.iBootingCount < cf.iMinCheck)
        {
            cf.iMinCheck = g_xHD2.x.iBootingCount;
            cf.bMinRes = iErrorFlag;
        }
    }
    chksum = GetSettingsCheckSum((unsigned char*)&cf, sizeof(cf));
    cf.bCheckSum = chksum;
    my_printf("min, max: %d, %d\n", cf.iMinCheck, cf.iMaxCheck);
    for (int k = 0; k < 16; k ++)
        my_printf("%02x ", ((unsigned char*)&cf)[k]);
    my_printf("\n");
    M24C64_Set16((unsigned char*)&cf, ADDR_CHK_FLAG);
    my_usleep(10*1000);

    int iUpgradeBaudrate = g_xCS.x.bUpgradeBaudrate;

    if (g_xSS.iUsbHostMode)
    {
        int i = 0;
        int timeIRlimit = 150;//30s
        while(timeIRlimit--)
        {
            //do not finish program
            if (iErrorFlag == 0)
            {
                GPIO_fast_setvalue(IR_LED, 1);
            }
            else
            {
                i = (i + 1) % 2;
                GPIO_fast_setvalue(IR_LED, i);
            }
            my_usleep(200*1000);
        }
        GPIO_fast_setvalue(IR_LED, 0);
        while(1)
        {
            my_usleep(100 * 1000);
        }
    }
    else
    {
        //send otag done success!
        unsigned char abOtaCmd[] = {0xEF, 0xAA, 0x01, 0x00, 0x02, 0x03, 0x00, 0x00};
        if (iErrorFlag != 0)
            abOtaCmd[5] = 0x90 + iErrorFlag;
        abOtaCmd[7] = SenseLockTask_Get_CheckSum(abOtaCmd + 2, sizeof(abOtaCmd) - 3);

        if (BR_IS_VALID(iUpgradeBaudrate))
        {
            my_usleep(50 * 1000);
            UART_SetBaudrate(UART_Baudrate(iUpgradeBaudrate));
            my_usleep(10 * 1000);
        }

        ///Ota Done을 받지 못하는 문제가 있어서 100ms지연을 줌
        my_usleep(100 * 1000);
        UART_Send(abOtaCmd, sizeof(abOtaCmd));
        my_usleep(10 * 1000);
        my_printf("Send Ota Finished! ok\n");
    }
}

#endif // CHECK_FIRMWARE == 1