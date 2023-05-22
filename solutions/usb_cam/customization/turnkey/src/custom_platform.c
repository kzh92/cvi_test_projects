#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include "drv_gpio.h"

void PLATFORM_IoInit(void)
{
	GPIO_fast_init();
//pinmux 切换接口
    PINMUX_CONFIG(PAD_MIPIRX3P, XGPIOC_5);
    PINMUX_CONFIG(PAD_MIPIRX3N, XGPIOC_4);
	PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);

	PINMUX_CONFIG(PAD_MIPI_TXM0, CAM_MCLK1); // MCLK

	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13); // PWDN
	GPIO_fast_setvalue(CAM_PWDN, ON); // XGPIOC_13 RST pin set H

	PINMUX_CONFIG(PAD_MIPI_TXM1, IIC2_SDA);
	PINMUX_CONFIG(PAD_MIPI_TXP1, IIC2_SCL);

	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
	GPIO_fast_setvalue(IR_LED, OFF);
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
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}
