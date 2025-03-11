#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include <drv/gpio.h>
#include <stdio.h>
#include <drv/irq.h>

#define IN  0
#define OUT 1

#define IR_LED                  421 /* PWR_GPIO[21] Group:4 Num:21 */
#define CAM_MIPI0_PWDN          208 /* XGPIOC[8] Group:2 Num:8*/
#define CAM_MIPI1_PWDN          207 /* XGPIOC[7] Group:2 Num:7*/
#define WHITE_LED               422 /* PWR_GPIO[22] Group:4 Num:22 */
#define PSENSE_DET              423 /* PWR_GPIO[23] Group:4 Num:22 */
#define AUDIO_EN                15 /* XGPIOA[15] Group:0 Num:15*/

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

#define GPIO_SPKEN_GRP 0
#define GPIO_SPKEN_NUM 15

static void _SensorPinmux()
{
    //Sensor Pinmux
    PINMUX_CONFIG(PAD_MIPIRX0P, CAM_MCLK0);
	PINMUX_CONFIG(PAD_MIPIRX1P, IIC1_SDA);
	PINMUX_CONFIG(PAD_MIPIRX0N, IIC1_SCL);
    PINMUX_CONFIG(PAD_MIPIRX1N, XGPIOC_8);
}

static void _MipiRxPinmux(void)
{
//mipi rx pinmux
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
    PINMUX_CONFIG(PAD_MIPIRX3P, XGPIOC_5);
    PINMUX_CONFIG(PAD_MIPIRX3N, XGPIOC_4);
    // PINMUX_CONFIG(PAD_MIPIRX2P, XGPIOC_7);
    // PINMUX_CONFIG(PAD_MIPIRX2N, XGPIOC_6);
}

static void _MipiTxPinmux(void)
{
//mipi tx pinmux
}

#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

static void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level)
{
	csi_error_t ret;
	csi_gpio_t gpio = {0};

	ret = csi_gpio_init(&gpio, gpio_grp);
	if(ret != CSI_OK) {
		printf("csi_gpio_init failed\r\n");
		return;
	}
	// gpio write
	ret = csi_gpio_dir(&gpio , GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

	if(ret != CSI_OK) {
		printf("csi_gpio_dir failed\r\n");
		return;
	}
	csi_gpio_write(&gpio , GPIO_PIN_MASK(gpio_num), level);
	//printf("test pin end and success.\r\n");
	csi_gpio_uninit(&gpio);
}

static void _UsbPinmux(void)
{
	// SOC_PORT_SEL
	PINMUX_CONFIG(SD1_GPIO0, PWR_GPIO_25);
	PINMUX_CONFIG(SD1_GPIO1, PWR_GPIO_26);
}

static void _UsbIoInit(void)
{
#if CONFIG_SUPPORT_USB_HC
	_GPIOSetValue(4, 25, 1);
	_GPIOSetValue(4, 26, 1);
#elif CONFIG_SUPPORT_USB_DC
	_GPIOSetValue(4, 25, 0);
	_GPIOSetValue(4, 26, 0);
#endif
}

#endif

void PLATFORM_IoInit(void)
{
//pinmux 切换接口
#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)
	_UsbPinmux();
	_UsbIoInit();
#endif
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();

    //config UART1(IIC0 pins)
    PINMUX_CONFIG(IIC0_SCL, UART1_TX);
    PINMUX_CONFIG(IIC0_SDA, UART1_RX);
    GPIO_fast_init();
    //camera power
    GPIO_fast_config(CAM_MIPI1_PWDN, OUT);
    GPIO_fast_config(CAM_MIPI0_PWDN, OUT);
    GPIO_fast_setvalue(CAM_MIPI1_PWDN, 1);    
    GPIO_fast_setvalue(CAM_MIPI0_PWDN, 1);

    PINMUX_CONFIG(SD1_D0, PWR_GPIO_21); //IR LED pin
    GPIO_fast_config(IR_LED, OUT);
    GPIO_fast_setvalue(IR_LED, 1);
}

void PLATFORM_PowerOff(void)
{
//下电休眠前调用接口
}

int PLATFORM_PanelInit(void)
{
    return CVI_SUCCESS;
}

void PLATFORM_PanelBacklightCtl(int level)
{

}

void PLATFORM_SpkMute(int value)
{
//0静音 ，1非静音
    printf("set spkMute = %d\r\n",value);
    PINMUX_CONFIG(SPK_EN, XGPIOA_15);
    if(value == 0) {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
    } else {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
    }
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}