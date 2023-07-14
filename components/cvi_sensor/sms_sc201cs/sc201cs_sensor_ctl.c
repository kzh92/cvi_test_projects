#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include "appdef.h"
#include <unistd.h>

#include "sc201cs_cmos_ex.h"

#define SC201CS_CHIP_ID_ADDR_H	0x3107
#define SC201CS_CHIP_ID_ADDR_L	0x3108
#define SC201CS_CHIP_ID		0xeb2c

#define SC201CS_AGAIN_DEFAULT_VAL		0x00
#define SC201CS_DGAIN_DEFAULT_VAL		0x00
#define SC201CS_DGAIN2_DEFAULT_VAL		0x80

#define I2C_ADDR_LEFT			0x30
#define I2C_ADDR_RIGHT			0x32
#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
#define DEFAULT_LEFT_FLAG		0 // ir cam
#else
#define DEFAULT_LEFT_FLAG		(DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_121 ? 0 : 1)
#endif
CVI_U8 g_is_left_cam = DEFAULT_LEFT_FLAG;

static void sc201cs_linear_1200p30_init(VI_PIPE ViPipe);

#if (USE_3M_MODE && DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_122)
CVI_U8 sc201cs_i2c_addr = 0x32;
#else
CVI_U8 sc201cs_i2c_addr = 0x30;
#endif

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

	sc201cs_i2c_addr = g_is_left_cam ? I2C_ADDR_LEFT : I2C_ADDR_RIGHT;

	return sensor_i2c_read(i2c_id, sc201cs_i2c_addr, (CVI_U32)addr, sc201cs_addr_byte,
		sc201cs_data_byte);
}

int sc201cs_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc201cs_BusInfo[ViPipe].s8I2cDev;

	sc201cs_i2c_addr = g_is_left_cam ? I2C_ADDR_LEFT : I2C_ADDR_RIGHT;

	return sensor_i2c_write(i2c_id, sc201cs_i2c_addr, (CVI_U32)addr, sc201cs_addr_byte,
		(CVI_U32)data, sc201cs_data_byte);
}

int sc201cs_read_register_ex(VI_PIPE ViPipe, int addr, int is_left_camera)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc201cs_BusInfo[ViPipe].s8I2cDev;

	sc201cs_i2c_addr = is_left_camera ? I2C_ADDR_LEFT : I2C_ADDR_RIGHT;

	return sensor_i2c_read(i2c_id, sc201cs_i2c_addr, (CVI_U32)addr, sc201cs_addr_byte,
		sc201cs_data_byte);
}

int sc201cs_write_register_ex(VI_PIPE ViPipe, int addr, int data, int is_left_camera)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunSc201cs_BusInfo[ViPipe].s8I2cDev;

	sc201cs_i2c_addr = is_left_camera ? I2C_ADDR_LEFT : I2C_ADDR_RIGHT;

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
	return;
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

int sc201cs_switch(VI_PIPE ViPipe, CVI_U8 switchCam)
{
	if (DEFAULT_CAM_MIPI_TYPE == CAM_MIPI_TY_121)
		return CVI_SUCCESS;
	if (switchCam == g_is_left_cam)
		return CVI_SUCCESS;

	sc201cs_write_register(ViPipe, 0x3019,0xff);
	sc201cs_write_register(ViPipe, 0x0100,0x00);

	g_is_left_cam = switchCam;

	sc201cs_write_register(ViPipe, 0x0100,0x01);
	sc201cs_write_register(ViPipe, 0x3019,0xfe);

	return CVI_SUCCESS;
}

int sc201cs_gain_reset(VI_PIPE ViPipe)
{
	sc201cs_write_register(ViPipe, 0x3e09, SC201CS_AGAIN_DEFAULT_VAL);
	sc201cs_write_register(ViPipe, 0x3e06, SC201CS_DGAIN_DEFAULT_VAL);
	sc201cs_write_register(ViPipe, 0x3e07, SC201CS_DGAIN2_DEFAULT_VAL);

	return CVI_SUCCESS;
}

int sc201cs_pattern_enable(VI_PIPE ViPipe, CVI_U8 enablePattern)
{
	int iCam = g_is_left_cam;
	g_is_left_cam = 0;

	if(enablePattern)
		sc201cs_gain_reset(ViPipe);

	sc201cs_write_register(ViPipe, 0x4501, enablePattern? 0xac : 0xa4);

	g_is_left_cam = 1;

	if(enablePattern)
		sc201cs_gain_reset(ViPipe);

	sc201cs_write_register(ViPipe, 0x4501, enablePattern? 0xac : 0xa4);

	g_is_left_cam = iCam;

	return CVI_SUCCESS;
}

void sc201cs_init(VI_PIPE ViPipe)
{
	// sc201cs_i2c_init(ViPipe);
	int old_value = g_is_left_cam;

	for (int i = 0 ; i < 2; i++)
	{
		g_is_left_cam = (i != 0);
		if (DEFAULT_CAM_MIPI_TYPE != CAM_MIPI_TY_121 || g_is_left_cam == DEFAULT_LEFT_FLAG)
			sc201cs_linear_1200p30_init(ViPipe);
		if ((old_value && i == 1) || (!old_value && i == 0))
		{
			sc201cs_write_register(ViPipe, 0x0100,0x01);
			sc201cs_write_register(ViPipe, 0x3019,0xfe);
		}
	}
	g_is_left_cam = old_value;

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
	sc201cs_write_register(ViPipe, 0x36ea, 0x0f);
	sc201cs_write_register(ViPipe, 0x36eb, 0x25);
	sc201cs_write_register(ViPipe, 0x36ed, 0x04);
	sc201cs_write_register(ViPipe, 0x36e9, 0x01);
	sc201cs_write_register(ViPipe, 0x3019, 0xff);
	sc201cs_write_register(ViPipe, 0x301f, 0x01);
	sc201cs_write_register(ViPipe, 0x320e, 0x05);//VTS:0x04e2 -> 0x05e2
	sc201cs_write_register(ViPipe, 0x3248, 0x02);
	sc201cs_write_register(ViPipe, 0x3253, 0x0a);
	sc201cs_write_register(ViPipe, 0x3301, 0xff);
	sc201cs_write_register(ViPipe, 0x3302, 0xff);
	sc201cs_write_register(ViPipe, 0x3303, 0x10);
	sc201cs_write_register(ViPipe, 0x3306, 0x28);
	sc201cs_write_register(ViPipe, 0x3307, 0x02);
	sc201cs_write_register(ViPipe, 0x330a, 0x00);
	sc201cs_write_register(ViPipe, 0x330b, 0xb0);
	sc201cs_write_register(ViPipe, 0x3318, 0x02);
	sc201cs_write_register(ViPipe, 0x3320, 0x06);
	sc201cs_write_register(ViPipe, 0x3321, 0x02);
	sc201cs_write_register(ViPipe, 0x3326, 0x12);
	sc201cs_write_register(ViPipe, 0x3327, 0x0e);
	sc201cs_write_register(ViPipe, 0x3328, 0x03);
	sc201cs_write_register(ViPipe, 0x3329, 0x0f);
	sc201cs_write_register(ViPipe, 0x3364, 0x0f);
	sc201cs_write_register(ViPipe, 0x33b3, 0x40);
	sc201cs_write_register(ViPipe, 0x33f9, 0x2c);
	sc201cs_write_register(ViPipe, 0x33fb, 0x38);
	sc201cs_write_register(ViPipe, 0x33fc, 0x0f);
	sc201cs_write_register(ViPipe, 0x33fd, 0x1f);
	sc201cs_write_register(ViPipe, 0x349f, 0x03);
	sc201cs_write_register(ViPipe, 0x34a6, 0x01);
	sc201cs_write_register(ViPipe, 0x34a7, 0x1f);
	sc201cs_write_register(ViPipe, 0x34a8, 0x40);
	sc201cs_write_register(ViPipe, 0x34a9, 0x30);
	sc201cs_write_register(ViPipe, 0x34ab, 0xa6);
	sc201cs_write_register(ViPipe, 0x34ad, 0xa6);
	sc201cs_write_register(ViPipe, 0x3622, 0x60);
	sc201cs_write_register(ViPipe, 0x3625, 0x08);
	sc201cs_write_register(ViPipe, 0x3630, 0xa8);
	sc201cs_write_register(ViPipe, 0x3631, 0x84);
	sc201cs_write_register(ViPipe, 0x3632, 0x90);
	sc201cs_write_register(ViPipe, 0x3633, 0x43);
	sc201cs_write_register(ViPipe, 0x3634, 0x09);
	sc201cs_write_register(ViPipe, 0x3635, 0x82);
	sc201cs_write_register(ViPipe, 0x3636, 0x48);
	sc201cs_write_register(ViPipe, 0x3637, 0xe4);
	sc201cs_write_register(ViPipe, 0x3641, 0x22);
	sc201cs_write_register(ViPipe, 0x3670, 0x0e);
	sc201cs_write_register(ViPipe, 0x3674, 0xc0);
	sc201cs_write_register(ViPipe, 0x3675, 0xc0);
	sc201cs_write_register(ViPipe, 0x3676, 0xc0);
	sc201cs_write_register(ViPipe, 0x3677, 0x86);
	sc201cs_write_register(ViPipe, 0x3678, 0x88);
	sc201cs_write_register(ViPipe, 0x3679, 0x8c);
	sc201cs_write_register(ViPipe, 0x367c, 0x01);
	sc201cs_write_register(ViPipe, 0x367d, 0x0f);
	sc201cs_write_register(ViPipe, 0x367e, 0x01);
	sc201cs_write_register(ViPipe, 0x367f, 0x0f);
	sc201cs_write_register(ViPipe, 0x3690, 0x43);
	sc201cs_write_register(ViPipe, 0x3691, 0x43);
	sc201cs_write_register(ViPipe, 0x3692, 0x53);
	sc201cs_write_register(ViPipe, 0x369c, 0x01);
	sc201cs_write_register(ViPipe, 0x369d, 0x1f);
	sc201cs_write_register(ViPipe, 0x3900, 0x0d);
	sc201cs_write_register(ViPipe, 0x3904, 0x06);
	sc201cs_write_register(ViPipe, 0x3905, 0x98);
	sc201cs_write_register(ViPipe, 0x391b, 0x81);
	sc201cs_write_register(ViPipe, 0x391c, 0x10);
	sc201cs_write_register(ViPipe, 0x391d, 0x19);
	sc201cs_write_register(ViPipe, 0x3949, 0xc8);
	sc201cs_write_register(ViPipe, 0x394b, 0x64);
	sc201cs_write_register(ViPipe, 0x3952, 0x02);
	sc201cs_write_register(ViPipe, 0x3650, 0x30); // MIPI LP 0x31 -> 0x30
	sc201cs_write_register(ViPipe, 0x3e00, 0x00);
	sc201cs_write_register(ViPipe, 0x3e01, 0x4d);
	sc201cs_write_register(ViPipe, 0x3e02, 0xe0);
	sc201cs_write_register(ViPipe, 0x4502, 0x34);
	sc201cs_write_register(ViPipe, 0x4509, 0x30);
	sc201cs_write_register(ViPipe, 0x0100, 0x00);

	sc201cs_default_reg_init(ViPipe);

	printf("ViPipe:%d, OK\n", ViPipe);
}
