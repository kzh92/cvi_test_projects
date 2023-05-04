#include "upgradebase.h"
#ifndef _PACK_OTA_
#include "drv_gpio.h"
#include "shared.h"
#endif

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