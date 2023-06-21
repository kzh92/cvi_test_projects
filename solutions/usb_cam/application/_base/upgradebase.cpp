#include "upgradebase.h"
#ifndef _PACK_OTA_
#include "drv_gpio.h"
#include "shared.h"
#include "folderfile.h"
#include "aescrypt.h"
#include "common_types.h"
#include "check_firmware.h"
#include "aescrypt.h"
#include "cvi_sys.h"
#endif // !_PACK_OTA_

#include <string.h>
#include <stdio.h>
// #include <stdlib.h>
// #include <sys/mount.h>
// #include <unistd.h>

int mount_tmp(int is_usb)
{
#ifndef _PACK_OTA_
    if (is_usb == 1)
    {
        //notify starting...
        GPIO_fast_config(IR_LED, OUT);
        for (int i = 0; i < 30; i ++)
        {
            if (i % 2 == 0)
            {
                GPIO_fast_setvalue(IR_LED, 1);
                my_usleep(100*1000);
            }
            else
            {
                GPIO_fast_setvalue(IR_LED, 0);
                my_usleep(100*1000);
            }
        }
    }
    //system("e2fsck -y /dev/mmcblk0p8");

    if (my_mount(TMP_DEVNAME, UPDATE_SRC_PATH, TMP_FSTYPE, 0, "") == 0)
    {
        return 1;
    }
#endif // !_PACK_OTA_
    return 0;
}

int umount_tmp()
{
#ifndef _PACK_OTA_
    my_umount(UPDATE_SRC_PATH);
#endif
    return 0;
}

void do_reset_tmp()
{
#ifndef _PACK_OTA_
    if(mount_tmp())
    {
        my_system("rm -rf /tmp1/*");
        umount_tmp();
    }
#endif // !_PACK_OTA_
}

int upg_get_aes_key(unsigned char* buf)
{
    const int key_len = 16;
    char s_buf[32] = {0x00};
    snprintf(s_buf, 32, "%s%s", DEVICE_MODEL_NUM, FIRMWARE_MAGIC);
    memcpy(buf, s_buf, key_len);
    for (int i = 0; i < key_len; i++)
        buf[i] = buf[i] ^ 0xAA;
    return 0;
}

int upg_aes_set_key(char* chipSN, char* aesKey)
{
    int i;
    char tmpBuf[8];
    memset(tmpBuf, 0, 8);

    if (memcmp(chipSN, tmpBuf, 8) == 0)
        return -1;

    memcpy(tmpBuf, chipSN, 8);
    for (i = 1; i < 8; i++)
        tmpBuf[i] ^= chipSN[i - 1];

    memcpy(aesKey, tmpBuf, 8);
    memcpy(aesKey + 8, tmpBuf, 8);
    return 0;
}

#ifndef _PACK_OTA_
int upg_do_ota4mem(unsigned char* ota_buf, unsigned int ota_len)
{
    FolderFile ff;
    // int erase_size = 0;
    unsigned int i;
    // unsigned char bFileCheckSum = 0;
    unsigned int iFileSize = ota_len;
    iFileSize--;

    //=== decrypt data ==============================================
    unsigned char* tmp_buf = NULL;
    int blk_size = 64;
    int read_len = 0;
    unsigned char checkSum = 0;
    tmp_buf = (unsigned char*)my_malloc(blk_size);
    for (i = 0; i < iFileSize / blk_size; i++)
    {
        read_len = iFileSize - i * blk_size;
        if (read_len > blk_size)
            read_len = blk_size;
        my_memcpy(tmp_buf, ota_buf + i * blk_size, read_len);
        FolderFile::decryptBuf(tmp_buf, read_len);
        for (int k = 0; k < read_len ;k ++)
            checkSum ^= tmp_buf[k];
        my_memcpy(ota_buf + i * blk_size, tmp_buf, read_len);
    }
    my_free(tmp_buf);

    if (ota_buf[iFileSize] != checkSum)
    {
        my_printf("[%s]checksum error: %02x, %02x\n", __func__, ota_buf[iFileSize], checkSum);
        return 1;
    }

    dbug_printf("[%s] checksum ok\n", __func__);
    for (int i = 0; i < 32; i++)
        dbug_printf("%02x ", ((unsigned char*)ota_buf)[i]);
    dbug_printf("\n===============================================\n");

    extern int extract_7z2mem(void* pInBuffer, int inLength, void** pOutBuffer, int* pOutLength);
    void* extract_buf = NULL;

    dbug_printf("7z extract start %0.3f\n", Now());
    int rc = extract_7z2mem(ota_buf, iFileSize, &extract_buf, &g_xSS.iUpgradeImgLen);
    my_printf("ex_time(%d), %0.3f, %d:", g_xSS.iUpgradeImgLen, Now(), rc);
    if (extract_buf)
    {
        for (int i = 0; i < 32 && i < g_xSS.iUpgradeImgLen; i++)
            dbug_printf("%02x ", ((unsigned char*)extract_buf)[i]);
    }
    else
    {
        my_printf("[%s] extract fail\n", __func__);
        return 2;
    }
    dbug_printf("\n===============================================\n");

    uf_file_header header;

    my_memcpy(&header, extract_buf, sizeof(header));
    dbug_printf("[%s] new version: %d.%d.%d-%d\n", __func__, header.m_major, header.m_minor, header.m_build, header.m_patch);

    //check version.
    int ver_new = header.m_patch*10000 + header.m_build*1000000 + header.m_minor*100000000 + header.m_major*1000000000;
    my_printf("vernew: %d\n", ver_new);

    if(strcmp(header.m_magic, FIRMWARE_MAGIC))
    {
        my_printf("[%s] invalid magic %s:%s\n", __func__, header.m_magic, FIRMWARE_MAGIC);

        // g_xCS.x.bUpgradeFlag = 0;
        // g_xCS.x.bUsbHost = 0;
        // g_xCS.x.bOtaMode = 0;
        // UpdateCommonSettings();
        // return 0;
    }

    int ret = ff.decompressDirectory((unsigned char*)extract_buf, (char*)"/", header);
    if (!ret)
    {
        my_printf("decomp fail\n");
    }

    my_free(extract_buf);

    return 0;
}

int upg_update_part(const char* u_filepath, unsigned char* u_buffer, unsigned int u_size, uf_file_header* u_header)
{
    int idx = 0;
    if (strstr(u_filepath, "easen_check_firmware"))
    {
        g_xCS.x.bCheckFirmware = 1;
        doCheckFirmware();
        return 0;
    } 
    if (strstr(u_filepath, "prim"))
    {
        CVI_U8 pu8SN[8] = {0};
        unsigned char abAesKey[AES_BLOCK_SIZE];
        int encrypt_size = 4 * 1024;
        unsigned char* encryptApp = NULL;
        int encLen = 0;

        CVI_SYS_GetChipSN(pu8SN, 8);
        upg_aes_set_key((char *)pu8SN, (char *)abAesKey);

        AES_Encrypt((unsigned char*)abAesKey, u_buffer, encrypt_size, &encryptApp, &encLen);
        if (!encryptApp || encLen != encrypt_size)
        {
            printf("@@@ encrypt prim fail\n");
            return 1;
        }
        memcpy(u_buffer, encryptApp, encrypt_size);
        my_free(encryptApp);
    }
    while(u_header->m_part_infos[idx].m_size > 0)
    {
        dbug_printf("[%s] %d:%s:%08x\n", __func__, idx, 
            u_header->m_part_infos[idx].m_partname, 
            u_header->m_part_infos[idx].m_offset);
        if (!strcmp(u_header->m_part_infos[idx].m_partname, u_filepath))
            break;
    }
    if (u_header->m_part_infos[idx].m_size > 0)
    {
        my_flash_write(u_header->m_part_infos[idx].m_offset, u_buffer, u_size);
        dbug_printf("write ok\n");
        return 0;
    }
    return 1;
}
#endif // !_PACK_OTA_