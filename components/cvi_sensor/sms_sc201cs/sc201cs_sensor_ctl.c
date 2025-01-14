#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "sc201cs_cmos_ex.h"

#define SC201CS_CHIP_ID_ADDR_H	0x3107
#define SC201CS_CHIP_ID_ADDR_L	0x3108
#define SC201CS_CHIP_ID		0xcb5c

static void sc201cs_linear_1200p30_init(VI_PIPE ViPipe);

CVI_U8 sc201cs_i2c_addr = 0x30;
const CVI_U32 sc201cs_addr_byte = 2;
const CVI_U32 sc201cs_data_byte = 1;

int sc201cs_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc201cs_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc201cs_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc201cs_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc201cs_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc201cs_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc201cs_i2c_addr, (CVI_U32)addr, sc201cs_addr_byte,
		sc201cs_data_byte);
}

int sc201cs_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc201cs_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc201cs_i2c_addr, (CVI_U32)addr, sc201cs_addr_byte,
		(CVI_U32)data, sc201cs_data_byte);
}

static void delay_ms(int ms)
{
	udelay(ms * 1000);
}

void sc201cs_standby(VI_PIPE ViPipe)
{
	sc201cs_write_register(ViPipe, 0x0100, 0x00);

	printf("%s\n", __func__);
}

void sc201cs_restart(VI_PIPE ViPipe)
{
	sc201cs_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc201cs_write_register(ViPipe, 0x0100, 0x01);

	printf("%s\n", __func__);
}

void sc201cs_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSc201cs[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc201cs_write_register(ViPipe,
				g_pastSc201cs[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSc201cs[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void sc201cs_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x6;
		break;
	case ISP_SNS_FLIP:
		val |= 0x60;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x66;
		break;
	default:
		return;
	}

	sc201cs_write_register(ViPipe, 0x3221, val);
}

int sc201cs_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	if (sc201cs_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = sc201cs_read_register(ViPipe, SC201CS_CHIP_ID_ADDR_H);
	nVal2 = sc201cs_read_register(ViPipe, SC201CS_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SC201CS_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		CVI_TRACE_SNS(CVI_DBG_ERR, "nVal:%#x, nVal2:%#x\n", nVal, nVal2);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void sc201cs_init(VI_PIPE ViPipe)
{
	// sc201cs_i2c_init(ViPipe);

	sc201cs_linear_1200p30_init(ViPipe);

	g_pastSc201cs[ViPipe]->bInit = CVI_TRUE;
}

void sc201cs_exit(VI_PIPE ViPipe)
{
	sc201cs_i2c_exit(ViPipe);
}

static void sc201cs_linear_1200p30_init(VI_PIPE ViPipe)
{
	sc201cs_write_register(ViPipe, 0x0103, 0x01);
	sc201cs_write_register(ViPipe, 0x0100, 0x00);
	sc201cs_write_register(ViPipe, 0x36e9, 0x80);
	sc201cs_write_register(ViPipe, 0x37f9, 0x80);
	sc201cs_write_register(ViPipe, 0x3018, 0x1a);
	sc201cs_write_register(ViPipe, 0x3019, 0x0e);
	sc201cs_write_register(ViPipe, 0x301f, 0x20);
	sc201cs_write_register(ViPipe, 0x3258, 0x0e);
	sc201cs_write_register(ViPipe, 0x3301, 0x06);
	sc201cs_write_register(ViPipe, 0x3302, 0x10);
	sc201cs_write_register(ViPipe, 0x3304, 0x68);
	sc201cs_write_register(ViPipe, 0x3306, 0x90);
	sc201cs_write_register(ViPipe, 0x3308, 0x18);
	sc201cs_write_register(ViPipe, 0x3309, 0x80);
	sc201cs_write_register(ViPipe, 0x330a, 0x01);
	sc201cs_write_register(ViPipe, 0x330b, 0x48);
	sc201cs_write_register(ViPipe, 0x330d, 0x18);
	sc201cs_write_register(ViPipe, 0x331c, 0x02);
	sc201cs_write_register(ViPipe, 0x331e, 0x59);
	sc201cs_write_register(ViPipe, 0x331f, 0x71);
	sc201cs_write_register(ViPipe, 0x3333, 0x10);
	sc201cs_write_register(ViPipe, 0x3334, 0x40);
	sc201cs_write_register(ViPipe, 0x3364, 0x56);
	sc201cs_write_register(ViPipe, 0x3390, 0x08);
	sc201cs_write_register(ViPipe, 0x3391, 0x09);
	sc201cs_write_register(ViPipe, 0x3392, 0x0b);
	sc201cs_write_register(ViPipe, 0x3393, 0x0a);
	sc201cs_write_register(ViPipe, 0x3394, 0x2a);
	sc201cs_write_register(ViPipe, 0x3395, 0x2a);
	sc201cs_write_register(ViPipe, 0x3396, 0x48);
	sc201cs_write_register(ViPipe, 0x3397, 0x49);
	sc201cs_write_register(ViPipe, 0x3398, 0x4b);
	sc201cs_write_register(ViPipe, 0x3399, 0x06);
	sc201cs_write_register(ViPipe, 0x339a, 0x0a);
	sc201cs_write_register(ViPipe, 0x339b, 0x30);
	sc201cs_write_register(ViPipe, 0x339c, 0x48);
	sc201cs_write_register(ViPipe, 0x33ad, 0x2c);
	sc201cs_write_register(ViPipe, 0x33ae, 0x38);
	sc201cs_write_register(ViPipe, 0x33b3, 0x40);
	sc201cs_write_register(ViPipe, 0x349f, 0x02);
	sc201cs_write_register(ViPipe, 0x34a6, 0x09);
	sc201cs_write_register(ViPipe, 0x34a7, 0x0f);
	sc201cs_write_register(ViPipe, 0x34a8, 0x30);
	sc201cs_write_register(ViPipe, 0x34a9, 0x28);
	sc201cs_write_register(ViPipe, 0x34f8, 0x5f);
	sc201cs_write_register(ViPipe, 0x34f9, 0x28);
	sc201cs_write_register(ViPipe, 0x3630, 0xc6);
	sc201cs_write_register(ViPipe, 0x3633, 0x33);
	sc201cs_write_register(ViPipe, 0x3637, 0x6b);
	sc201cs_write_register(ViPipe, 0x363c, 0xc1);
	sc201cs_write_register(ViPipe, 0x363e, 0xc2);
	sc201cs_write_register(ViPipe, 0x3670, 0x2e);
	sc201cs_write_register(ViPipe, 0x3674, 0xc5);
	sc201cs_write_register(ViPipe, 0x3675, 0xc7);
	sc201cs_write_register(ViPipe, 0x3676, 0xcb);
	sc201cs_write_register(ViPipe, 0x3677, 0x44);
	sc201cs_write_register(ViPipe, 0x3678, 0x48);
	sc201cs_write_register(ViPipe, 0x3679, 0x48);
	sc201cs_write_register(ViPipe, 0x367c, 0x08);
	sc201cs_write_register(ViPipe, 0x367d, 0x0b);
	sc201cs_write_register(ViPipe, 0x367e, 0x0b);
	sc201cs_write_register(ViPipe, 0x367f, 0x0f);
	sc201cs_write_register(ViPipe, 0x3690, 0x33);
	sc201cs_write_register(ViPipe, 0x3691, 0x33);
	sc201cs_write_register(ViPipe, 0x3692, 0x33);
	sc201cs_write_register(ViPipe, 0x3693, 0x84);
	sc201cs_write_register(ViPipe, 0x3694, 0x85);
	sc201cs_write_register(ViPipe, 0x3695, 0x8d);
	sc201cs_write_register(ViPipe, 0x3696, 0x9c);
	sc201cs_write_register(ViPipe, 0x369c, 0x0b);
	sc201cs_write_register(ViPipe, 0x369d, 0x0f);
	sc201cs_write_register(ViPipe, 0x369e, 0x09);
	sc201cs_write_register(ViPipe, 0x369f, 0x0b);
	sc201cs_write_register(ViPipe, 0x36a0, 0x0f);
	sc201cs_write_register(ViPipe, 0x36ec, 0x0c);
	sc201cs_write_register(ViPipe, 0x370f, 0x01);
	sc201cs_write_register(ViPipe, 0x3722, 0x05);
	sc201cs_write_register(ViPipe, 0x3724, 0x20);
	sc201cs_write_register(ViPipe, 0x3725, 0x91);
	sc201cs_write_register(ViPipe, 0x3771, 0x05);
	sc201cs_write_register(ViPipe, 0x3772, 0x05);
	sc201cs_write_register(ViPipe, 0x3773, 0x05);
	sc201cs_write_register(ViPipe, 0x377a, 0x0b);
	sc201cs_write_register(ViPipe, 0x377b, 0x0f);
	sc201cs_write_register(ViPipe, 0x3900, 0x19);
	sc201cs_write_register(ViPipe, 0x3905, 0xb8);
	sc201cs_write_register(ViPipe, 0x391b, 0x80);
	sc201cs_write_register(ViPipe, 0x391c, 0x04);
	sc201cs_write_register(ViPipe, 0x391d, 0x81);
	sc201cs_write_register(ViPipe, 0x3933, 0xc0);
	sc201cs_write_register(ViPipe, 0x3934, 0x08);
	sc201cs_write_register(ViPipe, 0x3940, 0x72);
	sc201cs_write_register(ViPipe, 0x3941, 0x00);
	sc201cs_write_register(ViPipe, 0x3942, 0x00);
	sc201cs_write_register(ViPipe, 0x3943, 0x09);
	sc201cs_write_register(ViPipe, 0x3946, 0x10);
	sc201cs_write_register(ViPipe, 0x3957, 0x86);
	sc201cs_write_register(ViPipe, 0x3e01, 0x8b);
	sc201cs_write_register(ViPipe, 0x3e02, 0xd0);
	sc201cs_write_register(ViPipe, 0x3e08, 0x00);
	sc201cs_write_register(ViPipe, 0x440e, 0x02);
	sc201cs_write_register(ViPipe, 0x4509, 0x28);
	sc201cs_write_register(ViPipe, 0x450d, 0x10);
	sc201cs_write_register(ViPipe, 0x4819, 0x09);
	sc201cs_write_register(ViPipe, 0x481b, 0x05);
	sc201cs_write_register(ViPipe, 0x481d, 0x14);
	sc201cs_write_register(ViPipe, 0x481f, 0x04);
	sc201cs_write_register(ViPipe, 0x4821, 0x0a);
	sc201cs_write_register(ViPipe, 0x4823, 0x05);
	sc201cs_write_register(ViPipe, 0x4825, 0x04);
	sc201cs_write_register(ViPipe, 0x4827, 0x05);
	sc201cs_write_register(ViPipe, 0x4829, 0x08);
	sc201cs_write_register(ViPipe, 0x5780, 0x66);
	sc201cs_write_register(ViPipe, 0x578d, 0x40);
	sc201cs_write_register(ViPipe, 0x5799, 0x06);
	sc201cs_write_register(ViPipe, 0x36e9, 0x20);
	sc201cs_write_register(ViPipe, 0x37f9, 0x27);

	// sc201cs_default_reg_init(ViPipe);

	sc201cs_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d,===SC2331 Init OK!===\n", ViPipe);
}