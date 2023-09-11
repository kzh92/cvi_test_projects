/*******************************************************************************
*******************************************************************************/

#include <drv/gpio.h>
#include <stdio.h>
#include <drv/irq.h>
#include <drv/pin.h>
#include <drv/pwm.h>
#include "drv_gpio.h"
#include "appdef.h"
#include "common_types.h"

static csi_gpio_t gpio[5] = {0};
static int g_csi_gpio_inited = 0;

csi_pwm_t pwm;
uint32_t wled_pwm_channel = 4; // PWM_8

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
    ret = csi_gpio_dir(&gpio[gpio_grp], GPIO_PIN_MASK(gpio_num), inout);

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

void WLED_pwm_init()
{
    uint32_t period_us = 200, duty_us;
    duty_us = period_us * WLED_PWM_DUTY / 100;

    csi_pwm_init(&pwm, 2); // SD1_CMD -> PWM_8
    csi_pwm_out_config(&pwm, wled_pwm_channel, period_us, duty_us, PWM_POLARITY_HIGH);
}

void WLED_pwm_on(unsigned char on)
{
    if (on)
        csi_pwm_out_start(&pwm, wled_pwm_channel);
    else
        csi_pwm_out_stop(&pwm, wled_pwm_channel);
}
