#include "upgrade_firmware.h"
#include "check_firmware.h"
#include "common_types.h"
#include "sha1.h"
#include "shared.h"
#include "desinterface.h"
#include "drv_gpio.h"
#include "uartcomm.h"
#include "i2cbase.h"
#include <string.h>
#include <fcntl.h>

//#define DATA1_PART_UPGRADE

extern unsigned char g_abKey[16];
extern int MarkActivationFailed(int iError);
extern int _set_activation_mark();

void encrypt_RTOS(unsigned char *buf, unsigned int len)
{
    unsigned int anUID[4] = { 0 };
    GetSSDID(anUID);
    for (unsigned int i = 0; i < len; i++)
    {
        buf[i] = buf[i] ^ anUID[i % 4];
    }
}

void encryptBuf(unsigned char *buf, unsigned int bufLen, unsigned char *key)
{
    for (unsigned int i = 0; i < bufLen; i++)
    {
        buf[i] = buf[i] ^ (key[i % REH_KEY_LEN] + i / REH_KEY_LEN);
    }
}

void doUpgradeFirmware()
{
    unsigned int rtos_len = DICT_START_ADDR - RTOS_START_ADDR;
    void* buf;
    unsigned int read_len, write_len;
    stRtosEasenHeader header;
    unsigned int iErrorFlag = 0;

    buf = my_malloc(rtos_len);
    read_len = my_flash_read(NEW_RTOS_ADDR, rtos_len, buf, rtos_len);
    if (read_len == rtos_len)
    {
        memcpy((unsigned char *)&header, (unsigned char *)buf, sizeof(stRtosEasenHeader));
        encryptBuf((unsigned char *)&header, sizeof(header) - REH_KEY_LEN, header.key);
        if (!strncmp(header.magic, REH_MAGIC, sizeof(REH_MAGIC)))
        {
            for (unsigned int i = 0; i < header.slot_count; i++)
            {
                encrypt_RTOS((unsigned char *)buf + header.slots[i].offset + sizeof(stRtosEasenHeader), header.slots[i].size);
            }
            write_len = my_flash_erase(RTOS_START_ADDR, rtos_len);
            if (write_len != rtos_len)
                iErrorFlag = 1;
            write_len = my_flash_write_pages(RTOS_START_ADDR, buf, rtos_len);
            if (write_len != rtos_len)
                iErrorFlag = 1;
        }
    }
    else
        iErrorFlag = 1;

    if (buf != NULL)
        my_free(buf);

    if (!iErrorFlag)
    {
        //update settings you want
    }

#ifdef DATA1_PART_UPGRADE
    {

        int iResult = 1;
        const char* szDictNames[1] = { FN_WNO_DICT_PATH};
        const unsigned long szDictLength[1] = {FN_WNO_DICT_SIZE};
        int len = 1;

        for(int i = 0; i < len; i ++)
        {
            int iFileLen = szDictLength[i];
            unsigned char* pbData = (unsigned char*)my_malloc(szDictLength[i]);
            if (pbData == NULL)
            {
                iResult = 0;
                my_printf("failed to malloc, %d\n", i);
                break;
            }
            fr_ReadFileData(szDictNames[i], 0, pbData, szDictLength[i]);

            dbug_printf("proca: %s\n", szDictNames[i]);
            

            //decrypt file.
#if 1
            for (int k = 0; k < iFileLen / 8; k ++)
                DesEncrypt(g_abKey, pbData + k * 8, 8, MCO_DECYPHER);
#endif
            //calculate checksum
            int iCheckSum = 0;
            int* pnData = (int*)pbData;
            for(int k = 0; k < iFileLen / 4; k ++)
                iCheckSum ^= pnData[k];

            //!!!CAUTION: this must be matched with index.
            if (i == 0)//wno.bin
            {
                ReadPermanenceSettings();
                g_xPS.x.iChecksumDNN = iCheckSum;
                UpdatePermanenceSettings();

            }

            char abSN[256] = { 0 };
            GetSN(abSN);

            {
                SHA1 sha1;
                SHA1* pSHA1 = &sha1;
                pSHA1->addBytes(abSN, strlen(abSN));

                unsigned char* pDig1 = (unsigned char*)pSHA1->getDigest();
                if(pDig1 == NULL)
                {
                    iResult = 0;
                    continue;
                }

                int SHA_LEN = 20;
                for(int a = 0; a < iFileLen / SHA_LEN; a ++)
                {
                    for(int b = 0; b < SHA_LEN; b ++)
                    {
                        pbData[a * SHA_LEN + b] = pbData[a * SHA_LEN + b] ^ pDig1[b];
                    }
                }

                fr_WriteFileData(szDictNames[i], 0, pbData, szDictLength[i]);
                my_free(pDig1);
            }

            my_free(pbData);
        }

        if(iResult == 0)
        {
            MarkActivationFailed(-2);
            iErrorFlag = 1;
        }

        rootfs_set_first_flag();
        _set_activation_mark();
        rootfs_set_activated();
    }
#endif //DATA1_PART_UPGRADE
#if 0
    if (!iErrorFlag)
    {
        unsigned char abChipID[8];
        unsigned int aiSSD_ID[4] = { 0 };
        GetSSDID(aiSSD_ID);
        memcpy(abChipID, aiSSD_ID, 8);
        for (int j = 0; j < 8; j ++)
        {
            abChipID[j] = ~(abChipID[j] ^ ((unsigned char*)aiSSD_ID)[j + 8]);
        }
        ReadHeadInfos();
        memcpy(g_xHD.x.bChipID, abChipID, sizeof(g_xHD.x.bChipID));
        UpdateHeadInfos();
    }
#endif
    int iUsbHost = 0;
#ifdef GPIO_USBSense
    iUsbHost = !GPIO_fast_getvalue(GPIO_USBSense);
#endif // GPIO_USBSense
    if (iUsbHost)
    {
        int i = 0;
        GPIO_fast_config(IR_LED, OUT);
        resetUpgradeInfo();
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
}

void resetUpgradeInfo()
{
    my_flash_erase(UPGRADER_INFO_ADDR, 4*1024);
}
