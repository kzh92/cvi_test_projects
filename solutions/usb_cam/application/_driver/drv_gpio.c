/*******************************************************************************
*******************************************************************************/

#include <drv/gpio.h>
#include <stdio.h>
#include <drv/irq.h>
#include <drv/pin.h>
#include "drv_gpio.h"
#include "appdef.h"
#include "common_types.h"

static csi_gpio_t gpio[5] = {0};
static int g_csi_gpio_inited = 0;

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

int GPIO_fast_init()
{
    if (g_csi_gpio_inited)
        return 0;
    g_csi_gpio_inited = 1;
    for (int i = 0 ; i < 5 ; i++)
        csi_gpio_init(&gpio[i], i);
    return 0;
}

void GPIO_fast_deinit()
{
}

int GPIO_fast_config(int gpio_pin, int inout)
{
    csi_error_t ret;

    unsigned int gpio_grp = gpio_pin / 100;
    unsigned int gpio_num = gpio_pin % 100;
    // dbug_printf("[%s] pin=%d, g=%d, n=%d\n", __func__, gpio_pin, gpio_grp, gpio_num);
    
    // gpio write
    ret = csi_gpio_dir(&gpio[gpio_grp], GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

    if(ret != CSI_OK) {
        printf("csi_gpio_dir failed\r\n");
        return -1;
    }
    return 0;
}

int GPIO_fast_setvalue(int gpio_pin, int value)
{
    unsigned int gpio_grp = gpio_pin / 100;
    unsigned int gpio_num = gpio_pin % 100;
    // dbug_printf("[%s] pin=%d, g=%d, n=%d, v=%d\n", __func__, gpio_pin, gpio_grp, gpio_num, value);
    
    csi_gpio_write(&gpio[gpio_grp], GPIO_PIN_MASK(gpio_num), value);
    return 0;
}

int GPIO_fast_getvalue(int gpio_pin)
{
    unsigned int gpio_grp = gpio_pin / 100;
    unsigned int gpio_num = gpio_pin % 100;
    // dbug_printf("[%s] pin=%d, g=%d, n=%d, v=%d\n", __func__, gpio_pin, gpio_grp, gpio_num, value);
    
    return csi_gpio_read(&gpio[gpio_grp], GPIO_PIN_MASK(gpio_num));
}