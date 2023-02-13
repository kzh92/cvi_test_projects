#include "upgradebase.h"
#include "drv_gpio.h"
#include "shared.h"

#include <string.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/mount.h>
// #include <unistd.h>

int mount_tmp(int is_usb)
{
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

    return 0;
}

int umount_tmp()
{
    my_umount(UPDATE_SRC_PATH);
    return 0;
}


void do_reset_tmp()
{
    if(mount_tmp())
    {
        my_system("rm -rf /tmp1/*");
        umount_tmp();
    }
}
