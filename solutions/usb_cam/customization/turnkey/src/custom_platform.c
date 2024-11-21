#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include <aos/kernel.h>
#include "cvi_type.h"

static void EthIOInit(void)
{
    mmio_write_32(0x03009804, 1);
    mmio_write_32(0x03009808, 1);
    mmio_write_32(0x03009800, 0x0905);
    aos_msleep(1);

    mmio_write_32(0x0300907c, 0x0500);
    mmio_write_32(0x03009078, 0x0F00);
    mmio_write_32(0x03009074, 0x0606);
    mmio_write_32(0x03009070, 0x0606);
}

void PLATFORM_IoInit(void)
{
	EthIOInit();
    
    PINMUX_CONFIG(PAD_MIPIRX3P, XGPIOC_5); //clk_p
    PINMUX_CONFIG(PAD_MIPIRX3N, XGPIOC_4); //clk_n
	PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3); //d0_p
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2); //d0_n
	PINMUX_CONFIG(PAD_ETH_RXM, CAM_MCLK0); // MCLK
	// i2c4 for camera mipi rx0
	PINMUX_CONFIG(PAD_MIPIRX2P, IIC4_SDA);
    PINMUX_CONFIG(PAD_MIPIRX2N, IIC4_SCL);

	// mipi rx1
	PINMUX_CONFIG(PAD_MIPIRX1P, XGPIOC_9);
    PINMUX_CONFIG(PAD_MIPIRX1N, XGPIOC_8);
	PINMUX_CONFIG(PAD_MIPIRX0P, XGPIOC_11);
    PINMUX_CONFIG(PAD_MIPIRX0N, XGPIOC_10);
    PINMUX_CONFIG(PAD_ETH_RXP, CAM_MCLK1); // MCLK
    // i2c1 for camera mipi rx1
	PINMUX_CONFIG(PAD_ETH_TXM, IIC1_SDA);
    PINMUX_CONFIG(PAD_ETH_TXP, IIC1_SCL);

	//IR LED GPIO
	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
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