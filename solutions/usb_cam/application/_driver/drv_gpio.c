/*******************************************************************************
Project  : FaceLock Test(Baling)
Module   : drv-gpio.h
Version  : 0.4
Date     : 2017.08.10.
Authors  : Liang HaiDong
Company  : EASEN
Comments :
Chip type: A20
*******************************************************************************/

#include "drv_gpio.h"
#include "appdef.h"
#include "mdrv_gpio.h"

extern void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO);
extern void MDrv_GPIO_Pad_Oen(U8 u8IndexGPIO);
extern void MDrv_GPIO_Pad_Odn(U8 u8IndexGPIO);
extern void MDrv_GPIO_Pull_Low(U8 u8IndexGPIO);
extern void MDrv_GPIO_Pull_High(U8 u8IndexGPIO);

int GPIO_fast_init()
{
    return 0;
}

void GPIO_fast_deinit()
{
}

int GPIO_fast_config(int gpio, int inout)
{
    MDrv_GPIO_Pad_Set(gpio);
    if (inout == OUT)
        MDrv_GPIO_Pad_Oen(gpio);
    else
        MDrv_GPIO_Pad_Odn(gpio);
    return 0;
}

int GPIO_fast_setvalue(int gpio, int value)
{
    if (value == OFF)
        MDrv_GPIO_Pull_Low(gpio);
    else
        MDrv_GPIO_Pull_High(gpio);
    return 0;
}

int GPIO_fast_getvalue(int gpio)
{
    return (MDrv_GPIO_Pad_Read(gpio) != 0);
}