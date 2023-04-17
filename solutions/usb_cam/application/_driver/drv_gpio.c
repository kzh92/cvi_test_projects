/*******************************************************************************
*******************************************************************************/

#include <drv/gpio.h>
#include <stdio.h>
#include <drv/irq.h>
#include <drv/pin.h>
#include "drv_gpio.h"
#include "appdef.h"

static csi_gpio_t gpio[4] = {0};

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

int GPIO_fast_init()
{
	for (int i = 0 ; i < 4 ; i++)
		csi_gpio_init(&gpio[i], i);
    return 0;
}

void GPIO_fast_deinit()
{
}

int GPIO_fast_config(int gpio, int inout)
{
    return 0;
}

int GPIO_fast_setvalue(int gpio_pin, int value)
{
	csi_error_t ret;

	unsigned int gpio_grp = gpio_pin / 100;
	unsigned int gpio_num = gpio_pin % 100;
	
	// gpio write
	ret = csi_gpio_dir(&gpio[gpio_grp], GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

	if(ret != CSI_OK) {
		printf("csi_gpio_dir failed\r\n");
		return -1;
	}
	csi_gpio_write(&gpio[gpio_grp], GPIO_PIN_MASK(gpio_num), value);
	//printf("test pin end and success.\r\n");
    return 0;
}

int GPIO_fast_getvalue(int gpio)
{
    return 0;
}