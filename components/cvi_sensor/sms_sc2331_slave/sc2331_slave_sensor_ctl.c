#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>

#include "sc2331_slave_cmos_ex.h"

#define SC2331_SLAVE_CHIP_ID_ADDR_H	0x3107
#define SC2331_SLAVE_CHIP_ID_ADDR_L	0x3108
#define SC2331_SLAVE_CHIP_ID		0xcc41

#define SC2331_SLAVE_AGAIN_DEFAULT_VAL		0x00
#define SC2331_SLAVE_DGAIN_DEFAULT_VAL		0x00
#define SC2331_SLAVE_DGAIN2_DEFAULT_VAL	0x80

static void sc2331_slave_linear_1200p30_init(VI_PIPE ViPipe);

CVI_U8 sc2331_slave_i2c_addr = 0x30;
const CVI_U32 sc2331_slave_addr_byte = 2;
const CVI_U32 sc2331_slave_data_byte = 1;

int sc2331_slave_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc2331_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int sc2331_slave_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc2331_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);
}

int sc2331_slave_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc2331_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, sc2331_slave_i2c_addr, (CVI_U32)addr, sc2331_slave_addr_byte,
		sc2331_slave_data_byte);
}

int sc2331_slave_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc2331_slave_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, sc2331_slave_i2c_addr, (CVI_U32)addr, sc2331_slave_addr_byte,
		(CVI_U32)data, sc2331_slave_data_byte);
}

int sc2331_slave_read_register_ex(VI_PIPE ViPipe, int addr, int is_left_camera)
{
	return sc2331_slave_read_register(ViPipe, addr);
}

int sc2331_slave_write_register_ex(VI_PIPE ViPipe, int addr, int data, int is_left_camera)
{
	return sc2331_slave_write_register(ViPipe, addr, data);
}

static void delay_ms(int ms)
{
	udelay(ms * 1000);
}

void sc2331_slave_standby(VI_PIPE ViPipe)
{
	sc2331_slave_write_register(ViPipe, 0x0100, 0x00);

	printf("%s\n", __func__);
}

void sc2331_slave_restart(VI_PIPE ViPipe)
{
	sc2331_slave_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	sc2331_slave_write_register(ViPipe, 0x0100, 0x01);

	printf("%s\n", __func__);
}

void sc2331_slave_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastSc2331_slave[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		sc2331_slave_write_register(ViPipe,
				g_pastSc2331_slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastSc2331_slave[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void sc2331_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
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

	sc2331_slave_write_register(ViPipe, 0x3221, val);
}

int sc2331_slave_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	if (sc2331_slave_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = sc2331_slave_read_register(ViPipe, SC2331_SLAVE_CHIP_ID_ADDR_H);
	nVal2 = sc2331_slave_read_register(ViPipe, SC2331_SLAVE_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != SC2331_SLAVE_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		CVI_TRACE_SNS(CVI_DBG_ERR, "nVal:%#x, nVal2:%#x\n", nVal, nVal2);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

int sc2331_slave_gain_reset(VI_PIPE ViPipe)
{
	sc2331_slave_write_register(ViPipe, 0x3e09, SC2331_SLAVE_AGAIN_DEFAULT_VAL);
	sc2331_slave_write_register(ViPipe, 0x3e06, SC2331_SLAVE_DGAIN_DEFAULT_VAL);
	sc2331_slave_write_register(ViPipe, 0x3e07, SC2331_SLAVE_DGAIN2_DEFAULT_VAL);

	return CVI_SUCCESS;
}

int sc2331_slave_pattern_enable(VI_PIPE ViPipe, CVI_U8 enablePattern)
{
	if(enablePattern)
		sc2331_slave_gain_reset(ViPipe);
	
	sc2331_slave_write_register(ViPipe, 0x4501, enablePattern? 0xac : 0xa4);
	return CVI_SUCCESS;
}

void sc2331_slave_init(VI_PIPE ViPipe)
{
	// sc2331_slave_i2c_init(ViPipe);

	sc2331_slave_linear_1200p30_init(ViPipe);

	g_pastSc2331_slave[ViPipe]->bInit = CVI_TRUE;
}

void sc2331_slave_exit(VI_PIPE ViPipe)
{
	sc2331_slave_i2c_exit(ViPipe);
}

static void sc2331_slave_linear_1200p30_init(VI_PIPE ViPipe)
{
	sc2331_slave_write_register(ViPipe, 0x0103, 0x01);
	sc2331_slave_write_register(ViPipe, 0x36e9, 0x80);
	sc2331_slave_write_register(ViPipe, 0x37f9, 0x80);
	sc2331_slave_write_register(ViPipe, 0x3018, 0x1a);
	sc2331_slave_write_register(ViPipe, 0x3019, 0x0e);
	sc2331_slave_write_register(ViPipe, 0x301f, 0x09);
	sc2331_slave_write_register(ViPipe, 0x30b8, 0x33);
	sc2331_slave_write_register(ViPipe, 0x3253, 0x10);
	sc2331_slave_write_register(ViPipe, 0x325f, 0x20);
	sc2331_slave_write_register(ViPipe, 0x3301, 0x04);
	sc2331_slave_write_register(ViPipe, 0x3306, 0x50);
	sc2331_slave_write_register(ViPipe, 0x3308, 0x08);
	sc2331_slave_write_register(ViPipe, 0x3309, 0xa8);
	sc2331_slave_write_register(ViPipe, 0x330a, 0x00);
	sc2331_slave_write_register(ViPipe, 0x330b, 0xd8);
	sc2331_slave_write_register(ViPipe, 0x330d, 0x14);
	sc2331_slave_write_register(ViPipe, 0x3314, 0x13);
	sc2331_slave_write_register(ViPipe, 0x331f, 0x99);
	sc2331_slave_write_register(ViPipe, 0x3333, 0x10);
	sc2331_slave_write_register(ViPipe, 0x3334, 0x40);
	sc2331_slave_write_register(ViPipe, 0x335e, 0x06);
	sc2331_slave_write_register(ViPipe, 0x335f, 0x0a);
	sc2331_slave_write_register(ViPipe, 0x3364, 0x5e);
	sc2331_slave_write_register(ViPipe, 0x337c, 0x02);
	sc2331_slave_write_register(ViPipe, 0x337d, 0x0e);
	sc2331_slave_write_register(ViPipe, 0x3390, 0x01);
	sc2331_slave_write_register(ViPipe, 0x3391, 0x03);
	sc2331_slave_write_register(ViPipe, 0x3392, 0x07);
	sc2331_slave_write_register(ViPipe, 0x3393, 0x04);
	sc2331_slave_write_register(ViPipe, 0x3394, 0x04);
	sc2331_slave_write_register(ViPipe, 0x3395, 0x04);
	sc2331_slave_write_register(ViPipe, 0x3396, 0x08);
	sc2331_slave_write_register(ViPipe, 0x3397, 0x0b);
	sc2331_slave_write_register(ViPipe, 0x3398, 0x1f);
	sc2331_slave_write_register(ViPipe, 0x3399, 0x04);
	sc2331_slave_write_register(ViPipe, 0x339a, 0x0a);
	sc2331_slave_write_register(ViPipe, 0x339b, 0x3a);
	sc2331_slave_write_register(ViPipe, 0x339c, 0xa0);
	sc2331_slave_write_register(ViPipe, 0x33a2, 0x04);
	sc2331_slave_write_register(ViPipe, 0x33ac, 0x08);
	sc2331_slave_write_register(ViPipe, 0x33ad, 0x1c);
	sc2331_slave_write_register(ViPipe, 0x33ae, 0x10);
	sc2331_slave_write_register(ViPipe, 0x33af, 0x30);
	sc2331_slave_write_register(ViPipe, 0x33b1, 0x80);
	sc2331_slave_write_register(ViPipe, 0x33b3, 0x48);
	sc2331_slave_write_register(ViPipe, 0x33f9, 0x60);
	sc2331_slave_write_register(ViPipe, 0x33fb, 0x74);
	sc2331_slave_write_register(ViPipe, 0x33fc, 0x4b);
	sc2331_slave_write_register(ViPipe, 0x33fd, 0x5f);
	sc2331_slave_write_register(ViPipe, 0x349f, 0x03);
	sc2331_slave_write_register(ViPipe, 0x34a6, 0x4b);
	sc2331_slave_write_register(ViPipe, 0x34a7, 0x5f);
	sc2331_slave_write_register(ViPipe, 0x34a8, 0x20);
	sc2331_slave_write_register(ViPipe, 0x34a9, 0x18);
	sc2331_slave_write_register(ViPipe, 0x34ab, 0xe8);
	sc2331_slave_write_register(ViPipe, 0x34ac, 0x01);
	sc2331_slave_write_register(ViPipe, 0x34ad, 0x00);
	sc2331_slave_write_register(ViPipe, 0x34f8, 0x5f);
	sc2331_slave_write_register(ViPipe, 0x34f9, 0x18);
	sc2331_slave_write_register(ViPipe, 0x3630, 0xc0);
	sc2331_slave_write_register(ViPipe, 0x3631, 0x84);
	sc2331_slave_write_register(ViPipe, 0x3632, 0x64);
	sc2331_slave_write_register(ViPipe, 0x3633, 0x32);
	sc2331_slave_write_register(ViPipe, 0x363b, 0x03);
	sc2331_slave_write_register(ViPipe, 0x363c, 0x08);
	sc2331_slave_write_register(ViPipe, 0x3641, 0x38);
	sc2331_slave_write_register(ViPipe, 0x3670, 0x4e);
	sc2331_slave_write_register(ViPipe, 0x3674, 0xc0);
	sc2331_slave_write_register(ViPipe, 0x3675, 0xc0);
	sc2331_slave_write_register(ViPipe, 0x3676, 0xc0);
	sc2331_slave_write_register(ViPipe, 0x3677, 0x86);
	sc2331_slave_write_register(ViPipe, 0x3678, 0x86);
	sc2331_slave_write_register(ViPipe, 0x3679, 0x86);
	sc2331_slave_write_register(ViPipe, 0x367c, 0x48);
	sc2331_slave_write_register(ViPipe, 0x367d, 0x49);
	sc2331_slave_write_register(ViPipe, 0x367e, 0x4b);
	sc2331_slave_write_register(ViPipe, 0x367f, 0x5f);
	sc2331_slave_write_register(ViPipe, 0x3690, 0x32);
	sc2331_slave_write_register(ViPipe, 0x3691, 0x32);
	sc2331_slave_write_register(ViPipe, 0x3692, 0x42);
	sc2331_slave_write_register(ViPipe, 0x369c, 0x4b);
	sc2331_slave_write_register(ViPipe, 0x369d, 0x5f);
	sc2331_slave_write_register(ViPipe, 0x36b0, 0x87);
	sc2331_slave_write_register(ViPipe, 0x36b1, 0x90);
	sc2331_slave_write_register(ViPipe, 0x36b2, 0xa1);
	sc2331_slave_write_register(ViPipe, 0x36b3, 0xd8);
	sc2331_slave_write_register(ViPipe, 0x36b4, 0x49);
	sc2331_slave_write_register(ViPipe, 0x36b5, 0x4b);
	sc2331_slave_write_register(ViPipe, 0x36b6, 0x4f);
	sc2331_slave_write_register(ViPipe, 0x370f, 0x01);
	sc2331_slave_write_register(ViPipe, 0x3722, 0x09);
	sc2331_slave_write_register(ViPipe, 0x3724, 0x41);
	sc2331_slave_write_register(ViPipe, 0x3725, 0xc1);
	sc2331_slave_write_register(ViPipe, 0x3771, 0x09);
	sc2331_slave_write_register(ViPipe, 0x3772, 0x09);
	sc2331_slave_write_register(ViPipe, 0x3773, 0x05);
	sc2331_slave_write_register(ViPipe, 0x377a, 0x48);
	sc2331_slave_write_register(ViPipe, 0x377b, 0x5f);
	sc2331_slave_write_register(ViPipe, 0x3904, 0x04);
	sc2331_slave_write_register(ViPipe, 0x3905, 0x8c);
	sc2331_slave_write_register(ViPipe, 0x391d, 0x04);
	sc2331_slave_write_register(ViPipe, 0x3921, 0x20);
	sc2331_slave_write_register(ViPipe, 0x3926, 0x21);
	sc2331_slave_write_register(ViPipe, 0x3933, 0x80);
	sc2331_slave_write_register(ViPipe, 0x3934, 0x0a);
	sc2331_slave_write_register(ViPipe, 0x3935, 0x00);
	sc2331_slave_write_register(ViPipe, 0x3936, 0x2a);
	sc2331_slave_write_register(ViPipe, 0x3937, 0x6a);
	sc2331_slave_write_register(ViPipe, 0x3938, 0x6a);
	sc2331_slave_write_register(ViPipe, 0x39dc, 0x02);
	sc2331_slave_write_register(ViPipe, 0x3e01, 0x53);
	sc2331_slave_write_register(ViPipe, 0x3e02, 0xe0);
	sc2331_slave_write_register(ViPipe, 0x3e09, 0x00);
	sc2331_slave_write_register(ViPipe, 0x440d, 0x10);
	sc2331_slave_write_register(ViPipe, 0x440e, 0x01);
	sc2331_slave_write_register(ViPipe, 0x4509, 0x20);
	sc2331_slave_write_register(ViPipe, 0x5ae0, 0xfe);
	sc2331_slave_write_register(ViPipe, 0x5ae1, 0x40);
	sc2331_slave_write_register(ViPipe, 0x5ae2, 0x38);
	sc2331_slave_write_register(ViPipe, 0x5ae3, 0x30);
	sc2331_slave_write_register(ViPipe, 0x5ae4, 0x28);
	sc2331_slave_write_register(ViPipe, 0x5ae5, 0x38);
	sc2331_slave_write_register(ViPipe, 0x5ae6, 0x30);
	sc2331_slave_write_register(ViPipe, 0x5ae7, 0x28);
	sc2331_slave_write_register(ViPipe, 0x5ae8, 0x3f);
	sc2331_slave_write_register(ViPipe, 0x5ae9, 0x34);
	sc2331_slave_write_register(ViPipe, 0x5aea, 0x2c);
	sc2331_slave_write_register(ViPipe, 0x5aeb, 0x3f);
	sc2331_slave_write_register(ViPipe, 0x5aec, 0x34);
	sc2331_slave_write_register(ViPipe, 0x5aed, 0x2c);
	sc2331_slave_write_register(ViPipe, 0x36e9, 0x54);
	sc2331_slave_write_register(ViPipe, 0x37f9, 0x23);

	sc2331_slave_default_reg_init(ViPipe);

	sc2331_slave_write_register(ViPipe, 0x0100, 0x01);

	printf("ViPipe:%d, OK\n", ViPipe);
}