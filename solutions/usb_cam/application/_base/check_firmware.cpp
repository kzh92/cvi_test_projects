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
    ReadCommonSettings();
    if (g_xCS.x.bCheckFirmware == 1)
    {
        g_xCS.x.bCheckFirmware = 0;
        UpdateCommonSettings();
    }
    char szSN[256] = { 0 };
    int iErrorFlag = 0;
    const char* szDictNames[1] = { FN_WNO_DICT_PATH};
    const unsigned long szDictLength[1] = {FN_WNO_DICT_SIZE};
    int iCheckSum = 0;
    {
        int iFileLen = szDictLength[0];
        unsigned char* pbData = (unsigned char*)my_malloc(szDictLength[0]);
        if (pbData != NULL)
        {
            GetSN(szSN);

            SHA1 sha1;
            SHA1* pSHA1 = &sha1;
            pSHA1->addBytes(szSN, strlen(szSN));

            unsigned char* pDig1 = (unsigned char*)pSHA1->getDigest();

            fr_ReadFileData(szDictNames[0], 0, pbData, szDictLength[0]);

            const int SHA_LEN = 20;

            for(int a = 0; a < iFileLen / SHA_LEN; a ++)
            {
                for(int b = 0; b < SHA_LEN; b ++)
                    pbData[a * SHA_LEN + b] = pbData[a * SHA_LEN + b] ^ pDig1[b];
            }

	        //calculate checksum
            int* pnData = (int*)pbData;
            for(int k = 0; k < iFileLen / 4; k ++)
                iCheckSum ^= pnData[k];
            my_free(pbData);
            my_free(pDig1);
        }
        ReadPermanenceSettings();
        my_printf("nnn %08x\n", iCheckSum);
        if (0xee753512 != (unsigned int)iCheckSum)
        {
            iErrorFlag = 1;
            my_printf("nnn mismatch\n");
        }
        if (iCheckSum != g_xPS.x.iChecksumDNN)
        {
            iErrorFlag = 2;
            my_printf("nnn,24 mismatch, %08x, %08x\n", iCheckSum, g_xPS.x.iChecksumDNN);
        }
    }

    const int file_count = 9;
    const char* file_names[file_count] = {
        "/test/a1.bin",
        "/test/a2.bin",
        "/test/b.bin",
        "/test/b2.bin",
        "/test/c.bin",
        "/test/detect.bin",
        "/test/dlamk.bin",
        "/mnt/MISC/face.bin",
        NULL
    };
    const int dict_ver_cnt = 1;
    int dict_ver_index = 0;
    int file_sums[dict_ver_cnt][file_count] = {
        {
            (int)0x136db36a,
            (int)0x69095c48,
            (int)0x8cbc59a1,
            (int)0x80e12267,
            (int)0xfa279d1a,
            (int)0xf356043f,
            (int)0x75e6b55c,
            (int)0x1663c913,
            0
        },
    };
    int file_lens[dict_ver_cnt][file_count] = {
        {
            FN_A1_DICT_SIZE,
            FN_A2_DICT_SIZE,
            FN_B_DICT_SIZE,
            FN_B2_DICT_SIZE,
            FN_C_DICT_SIZE,
            FN_DETECT_DICT_SIZE,
            FN_DLAMK_DICT_SIZE,
            1280*720, //face.bin
            0
        },
    };
    if (iErrorFlag == 0)
    {
        int i = 0;
        int sum;
        int *file_data = NULL;
        while (file_names[i] != NULL)
        {
            file_data = (int*)my_malloc(file_lens[dict_ver_index][i]);
            if (file_data == NULL)
            {
                iErrorFlag = 4;
                my_printf("err: %s, 1\n", file_names[i]);
                break;
            }
            if (i == (sizeof(file_sums[0])/sizeof(int) - 2)) //face.bin
            {
            	myfdesc_ptr fd = NULL;
                my_mount_misc();
                fd = my_open(file_names[i], O_RDONLY, 0);
                if (is_myfdesc_ptr_valid(fd))
                {
                    my_read(fd, file_data, file_lens[dict_ver_index][i]);
                    my_close(fd);
                }
            }
            else
            {
                if (file_lens[dict_ver_index][i] != fr_ReadFileData(file_names[i], 0, file_data, file_lens[dict_ver_index][i]))
                {
                    iErrorFlag = 4;
                    my_printf("err: %s, 2\n", file_names[i]);
                    my_free(file_data);
                    break;
                }
            }
            sum = 0;
            for (int k = 0; k < file_lens[dict_ver_index][i] / (int)sizeof(int); k ++)
            {
                sum = sum ^ file_data[k];
            }
            my_free(file_data);
            if (i == 0)
            {
                for (int k = 0; k < dict_ver_cnt; k ++)
                {
                    if (sum == file_sums[k][i])
                    {
                        dict_ver_index = k;
                        break;
                    }
                }
            }
            if (sum != file_sums[dict_ver_index][i])
            {
                iErrorFlag = 6;
                my_printf("err: %s, 5, %08x, %08x\n", file_names[i], sum, file_sums[dict_ver_index][i]);
                //break;
            }
            i ++;
        }
    }
    {
        //check chip ID
        unsigned char abChipID[8];
        unsigned int aiSSD_ID[4] = { 0 };
        GetSSDID(aiSSD_ID);
        memcpy(abChipID, aiSSD_ID, 8);
        for (int j = 0; j < 8; j ++)
        {
            abChipID[j] = ~(abChipID[j] ^ ((unsigned char*)aiSSD_ID)[j + 8]);
        }

        ReadHeadInfos();
        if (memcmp(g_xHD.x.bChipID, (unsigned char *)abChipID, 8))
        {
            my_printf("Cid mismatch\n");
            iErrorFlag = 5;
        }
        else
            my_printf("Cid ok\n");
    }

    my_printf("sv: %d.\n", g_xCS.x.bSecureFlag);
    if (g_xCS.x.bSecureFlag != DEFAULT_SECURE_VALUE)
    {
        iErrorFlag = 9;
    }

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

    ReadROKLogs();
    if (g_xROKLog.bMountPoint != 0)
    {
        iErrorFlag = 10;
        my_printf("mount retry err\n");
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

#ifndef __RTK_OS__
    const char* dbfs_part_names[2] =
    {
        "/dev/mtdblock5",
        "/dev/mtdblock6"
    };
    my_printf("--------------\n");

    for (int i = 0; i < 2; i++)
    {
        int ret = mount(dbfs_part_names[i], "/db", "jffs2", MS_NOATIME, "");
        if (ret == 0)
        {
            my_printf("mount success, %d\n", i);

            system("touch /db/test_db");

            FILE* fp = fopen("/db/test_db", "wb");
            if(fp == NULL)
            {
                ret = -1;
                my_printf("test write fail.\n");
                iErrorFlag = 11;
                break;
            }
            else
            {
                my_printf("test write ok.\n");
                fclose(fp);
            }
            system("umount -f /db");
        }
        else
        {
            my_printf("mount fail, %d\n", i);
            iErrorFlag = 12;
            break;
        }
    }
#endif // !__RTK_OS__

    //int iUpgradeFlag = g_xCS.x.bUpgradeFlag;
    int iUpgradeBaudrate = g_xCS.x.bUpgradeBaudrate;
    int iUsbHost = 0;
#ifdef GPIO_USBSense
    iUsbHost = !GPIO_fast_getvalue(GPIO_USBSense);
#endif // GPIO_USBSense
    my_printf("UsbHost = %d\n", iUsbHost);

    {
        g_xCS.x.bUpgradeFlag = 0;
        g_xCS.x.bUsbHost = 0;
        g_xCS.x.bOtaMode = 0;
        UpdateCommonSettings();

        //send otag done success!
        unsigned char abOtaCmd[] = {0xEF, 0xAA, 0x01, 0x00, 0x02, 0x03, 0x00, 0x00};
        if (iErrorFlag != 0)
            abOtaCmd[5] = 0x90 + iErrorFlag;
        abOtaCmd[7] = SenseLockTask_Get_CheckSum(abOtaCmd + 2, sizeof(abOtaCmd) - 3);

        if(iUpgradeBaudrate == 1)
        {
            UART_SetBaudrate(B115200);
        }
        else if(iUpgradeBaudrate == 2)
        {
            UART_SetBaudrate(B230400);
            my_usleep(10 * 1000);
        }
        else if(iUpgradeBaudrate == 3)
        {
            UART_SetBaudrate(B460800);
            my_usleep(10 * 1000);
        }
        else if(iUpgradeBaudrate == 4)
        {
            UART_SetBaudrate(B1500000);
        }
        else
            UART_SetBaudrate(B115200);

        ///Ota Done을 받지 못하는 문제가 있어서 100ms지연을 줌
        my_usleep(100 * 1000);
        UART_Send(abOtaCmd, sizeof(abOtaCmd));
        my_usleep(10 * 1000);
        my_printf("Send Ota Finished! ok\n");
    }

    if (iUsbHost)
    {
        int i = 0;
        GPIO_fast_config(IR_LED, OUT);
#ifdef UPGRADE_MODE
        resetUpgradeInfo();
#endif
        while(1)
        {
            //do not finish program
            if (iErrorFlag == 0)
            {
                //
                GPIO_fast_setvalue(IR_LED, 1);
            }
            else
            {
                i = (i + 1) % 2;
                GPIO_fast_setvalue(IR_LED, i);
            }
            my_usleep(200*1000);
        }
    }
    GPIO_fast_deinit();
}

#endif // CHECK_FIRMWARE == 1