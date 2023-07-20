#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "cvi_type.h"
#include "cvi_debug.h"
#include "cvi_comm_sns.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "cvi_ae_comm.h"
#include "cvi_awb_comm.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"

#include "sc031iot_raw_cmos_ex.h"
#include "sc031iot_raw_cmos_param.h"

#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif



#define DIV_0_TO_1(a)   ((0 == (a)) ? 1 : (a))
#define DIV_0_TO_1_FLOAT(a) ((((a) < 1E-10) && ((a) > -1E-10)) ? 1 : (a))
#define SC031IOT_RAW_ID 0x9a46
#define SC031IOT_RAW_I2C_ADDR_1 0x68
#define SC031IOT_RAW_I2C_ADDR_2 0x68
#define SC031IOT_RAW_I2C_ADDR_IS_VALID(addr)      ((addr) == SC031IOT_RAW_I2C_ADDR_1 || (addr) == SC031IOT_RAW_I2C_ADDR_2)

/****************************************************************************
 * global variables                                                         *
 ***************************************************************************/

ISP_SNS_STATE_S *g_pastSc031iot_raw[VI_MAX_PIPE_NUM] = {CVI_NULL};

#define SC031IOT_RAW_SENSOR_GET_CTX(dev, pstCtx)   (pstCtx = g_pastSc031iot_raw[dev])
#define SC031IOT_RAW_SENSOR_SET_CTX(dev, pstCtx)   (g_pastSc031iot_raw[dev] = pstCtx)
#define SC031IOT_RAW_SENSOR_RESET_CTX(dev)         (g_pastSc031iot_raw[dev] = CVI_NULL)

ISP_SNS_COMMBUS_U g_aunSc031iot_raw_BusInfo[VI_MAX_PIPE_NUM] = {
	[0] = { .s8I2cDev = 2},
	[1 ... VI_MAX_PIPE_NUM - 1] = { .s8I2cDev = -1}
};

ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc031iot_raw_MirrorFip[VI_MAX_PIPE_NUM] = {ISP_SNS_NORMAL};

CVI_U16 g_au16Sc031iot_raw_GainMode[VI_MAX_PIPE_NUM] = {0};
CVI_U16 g_au16Sc031iot_raw_UseHwSync[VI_MAX_PIPE_NUM] = {0};

/****************************************************************************
 * local variables and functions                                            *
 ***************************************************************************/

static CVI_U32 g_au32InitExposure[VI_MAX_PIPE_NUM]  = {0};
static CVI_U32 g_au32LinesPer500ms[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16InitWBGain[VI_MAX_PIPE_NUM][3] = {{0} };
static CVI_U16 g_au16SampleRgain[VI_MAX_PIPE_NUM] = {0};
static CVI_U16 g_au16SampleBgain[VI_MAX_PIPE_NUM] = {0};
static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg);
/*****Sc031iot_raw Lines Range*****/
#define SC031IOT_RAW_FULL_LINES_MAX  (0xFFFF)

/*****Sc031iot_raw Register Address*****/
#define SC031IOT_RAW_PAGE_ADDR					0xF0
#define SC031IOT_RAW_PAGE_EXP_ADDR				0x00
#define SC031IOT_RAW_EXP_0_ADDR					0x8D
#define SC031IOT_RAW_EXP_1_ADDR					0x8E
#define SC031IOT_RAW_PAGE_AGAIN_ADDR			0X00
#define SC031IOT_RAW_AGAIN_0_ADDR				0X70
#define SC031IOT_RAW_AGAIN_1_ADDR				0X8B
#define SC031IOT_RAW_PAGE_DGAIN_ADDR			0X00
#define SC031IOT_RAW_DGAIN_0_ADDR				0X9E
#define SC031IOT_RAW_PAGE_VMAX_ADDR				0X32
#define SC031IOT_RAW_VMAX_0_ADDR				0X0C // HTS
#define SC031IOT_RAW_VMAX_1_ADDR				0X0D // HTS

#define SC031IOT_RAW_RES_IS_480P(w, h)      ((w) <= 640 && (h) <= 480)

static CVI_S32 cmos_get_ae_default(VI_PIPE ViPipe, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FlickerFreq = 50 * 256;
	pstAeSnsDft->u32FullLinesMax = SC031IOT_RAW_FULL_LINES_MAX;
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * 30);

	pstAeSnsDft->stIntTimeAccu.enAccuType = AE_ACCURACY_LINEAR;
	pstAeSnsDft->stIntTimeAccu.f32Accuracy = 1;
	pstAeSnsDft->stIntTimeAccu.f32Offset = 0;
	pstAeSnsDft->stAgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stAgainAccu.f32Accuracy = 1;
	pstAeSnsDft->stDgainAccu.enAccuType = AE_ACCURACY_TABLE;
	pstAeSnsDft->stDgainAccu.f32Accuracy = 1;

	pstAeSnsDft->u32ISPDgainShift = 8;
	pstAeSnsDft->u32MinISPDgainTarget = 1 << pstAeSnsDft->u32ISPDgainShift;
	pstAeSnsDft->u32MaxISPDgainTarget = 2 << pstAeSnsDft->u32ISPDgainShift;

	if (g_au32LinesPer500ms[ViPipe] == 0)
		pstAeSnsDft->u32LinesPer500ms = pstSnsState->u32FLStd * 30 / 2;
	else
		pstAeSnsDft->u32LinesPer500ms = g_au32LinesPer500ms[ViPipe];
	switch (pstSnsState->enWDRMode) {
	default:
	case WDR_MODE_NONE:   /*linear mode*/
		pstAeSnsDft->f32Fps = g_astSc031iot_raw_mode[SC031IOT_RAW_MODE_640X480P30].f32MaxFps;
		pstAeSnsDft->f32MinFps = g_astSc031iot_raw_mode[SC031IOT_RAW_MODE_640X480P30].f32MinFps;
		pstAeSnsDft->au8HistThresh[0] = 0xd;
		pstAeSnsDft->au8HistThresh[1] = 0x28;
		pstAeSnsDft->au8HistThresh[2] = 0x60;
		pstAeSnsDft->au8HistThresh[3] = 0x80;

		pstAeSnsDft->u32MaxAgain = g_astSc031iot_raw_mode[SC031IOT_RAW_MODE_640X480P30].stAgain[0].u32Max;
		pstAeSnsDft->u32MinAgain = 1024;
		pstAeSnsDft->u32MaxAgainTarget = pstAeSnsDft->u32MaxAgain;
		pstAeSnsDft->u32MinAgainTarget = pstAeSnsDft->u32MinAgain;

		pstAeSnsDft->u32MaxDgain = g_astSc031iot_raw_mode[SC031IOT_RAW_MODE_640X480P30].stDgain[0].u32Max;
		pstAeSnsDft->u32MinDgain = 1024;
		pstAeSnsDft->u32MaxDgainTarget = pstAeSnsDft->u32MaxDgain;
		pstAeSnsDft->u32MinDgainTarget = pstAeSnsDft->u32MinDgain;

		pstAeSnsDft->u8AeCompensation = 40;
		pstAeSnsDft->u32InitAESpeed = 64;
		pstAeSnsDft->u32InitAETolerance = 5;
		pstAeSnsDft->u32AEResponseFrame = 4;
		pstAeSnsDft->enAeExpMode = AE_EXP_HIGHLIGHT_PRIOR;
		pstAeSnsDft->u32InitExposure = g_au32InitExposure[ViPipe] ?
			g_au32InitExposure[ViPipe] : g_astSc031iot_raw_mode[SC031IOT_RAW_MODE_640X480P30].stExp[0].u16Def;

		pstAeSnsDft->u32MaxIntTime = pstSnsState->u32FLStd - 8;
		pstAeSnsDft->u32MinIntTime = g_astSc031iot_raw_mode[SC031IOT_RAW_MODE_640X480P30].stExp[0].u16Min;
		pstAeSnsDft->u32MaxIntTimeTarget = 65535;
		pstAeSnsDft->u32MinIntTimeTarget = 1;

		break;
	}

	return CVI_SUCCESS;
}

/* the function of sensor set fps */
static CVI_S32 cmos_fps_set(VI_PIPE ViPipe, CVI_FLOAT f32Fps, AE_SENSOR_DEFAULT_S *pstAeSnsDft)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	CVI_U32 u32HMAX;
	CVI_FLOAT f32MaxFps = 0;
	CVI_FLOAT f32MinFps = 0;
	CVI_U32 u32Hts = 0;
	CVI_U32 u32Vts = 0;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_FLOAT f32LineTime = 0;

	CMOS_CHECK_POINTER(pstAeSnsDft);
	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u32Hts = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].u32HtsDef;
	u32Vts = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;
	f32MaxFps = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].f32MaxFps;
	f32MinFps = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].f32MinFps;

	if ((f32Fps <= f32MaxFps) && (f32Fps >= f32MinFps)) {
		u32HMAX = u32Hts * f32MaxFps / DIV_0_TO_1_FLOAT(f32Fps);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Fps: %f\n", f32Fps);
		return CVI_FAILURE;
	}

	u32HMAX = (u32HMAX > SC031IOT_RAW_FULL_LINES_MAX) ? SC031IOT_RAW_FULL_LINES_MAX : u32HMAX;

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_0_ADDR].u32Data = ((u32HMAX & 0xFF00) >> 8);
		pstSnsRegsInfo->astI2cData[LINEAR_VMAX_1_ADDR].u32Data = (u32HMAX & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	pstSnsState->u32FLStd = u32HMAX;

	pstAeSnsDft->f32Fps = f32Fps;
	f32LineTime = (CVI_FLOAT)1000000 / u32Vts / f32Fps;
	pstAeSnsDft->u32LinesPer500ms = (CVI_U32)(500000 / f32LineTime);
	pstAeSnsDft->u32FullLinesStd = pstSnsState->u32FLStd;
	pstAeSnsDft->u32MaxIntTime = (CVI_U32)(1000000 / pstAeSnsDft->f32Fps / f32LineTime) - 8;
	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstAeSnsDft->u32FullLines = pstSnsState->au32FL[0];
	pstAeSnsDft->u32HmaxTimes = (1000000) / (pstSnsState->u32FLStd * DIV_0_TO_1_FLOAT(f32Fps));

	return CVI_SUCCESS;
}

/* while isp notify ae to update sensor regs, ae call these funcs. */
static CVI_S32 cmos_inttime_update(VI_PIPE ViPipe, CVI_U32 *u32IntTime)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(u32IntTime);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	CVI_U32 u32TmpIntTime = u32IntTime[0];
	/* linear exposure reg range:
		* min : 1
		* max : 2 * (vts - 4)
		* step : 1
		*/
	u32TmpIntTime = (u32TmpIntTime > (pstSnsState->au32FL[0] - 8)) ?
			(pstSnsState->au32FL[0] - 8) : u32TmpIntTime;
	u32TmpIntTime = u32TmpIntTime << 1;
	if (!u32TmpIntTime)
		u32TmpIntTime = 1;

	pstSnsRegsInfo->astI2cData[LINEAR_EXP_0_ADDR].u32Data = ((u32TmpIntTime & 0xFF00) >> 8);
	pstSnsRegsInfo->astI2cData[LINEAR_EXP_1_ADDR].u32Data = ((u32TmpIntTime & 0x00FF));

	return CVI_SUCCESS;
}

typedef struct again_tbl_info_s {
	CVI_U16	gain0;
	CVI_U16	gain1;
	CVI_U32	gainValue;
} again_tbl_info_s;

static struct again_tbl_info_s AgainInfo[] = {
	{
		.gain0 = 0x0,
		.gain1 = 0x0,
		.gainValue = 1024,
	},
	{
		.gain0 = 0x1,
		.gain1 = 0x0,
		.gainValue = 1996,
	},
	{
		.gain0 = 0x1,
		.gain1 = 0x1,
		.gainValue = 3993,
	},
	{
		.gain0 = 0x1,
		.gain1 = 0x3,
		.gainValue = 7987,
	},
	{
		.gain0 = 0x1,
		.gain1 = 0x7,
		.gainValue = 15974,
	},
	{
		.gain0 = 0x1,
		.gain1 = 0xf,
		.gainValue = 31948,
	},
	{
		.gain0 = 0x1,
		.gain1 = 0x1f,
		.gainValue = 63897,
	},
};

typedef struct gain_tbl_info_s {
	CVI_U16	gainMax;
	CVI_U16	idxBase;
	CVI_U8	regGain;
	CVI_U8	regGainFineBase;
	CVI_U8	regGainFineStep;
} gain_tbl_info_s;

static CVI_U32 Dgain_table[] = {
	1024, 1085, 1157, 1218, 1280, 1341, 1413, 1474, 1536, 1597, 1669, 1730, 1792, 1853, 1925, 1986,
	2048, 2109, 2181, 2242, 2304, 2365, 2437, 2498, 2560, 2621, 2693, 2754, 2816, 2877, 2949, 3010,
	3072, 3133, 3205, 3266, 3328, 3389, 3461, 3522, 3584, 3645, 3717, 3778, 3840, 3901, 3973, 4034,
	4096, 4157, 4229, 4290, 4352, 4413, 4485, 4546, 4608, 4669, 4741, 4802, 4864, 4925, 4997, 5058,
	5120, 5181, 5253, 5314, 5376, 5437, 5509, 5570, 5632, 5693, 5765, 5826, 5888, 5949, 6021, 6082,
	6144, 6205, 6277, 6338, 6400, 6461, 6533, 6594, 6656, 6717, 6789, 6850, 6912, 6973, 7045, 7106,
	7168, 7229, 7301, 7362, 7424, 7485, 7557, 7618, 7680, 7741, 7813, 7874, 7936, 7997, 8069, 8130,
};

static CVI_S32 cmos_again_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32AgainLin, CVI_U32 *pu32AgainDb)
{
	int i;

	(void) ViPipe;

	CMOS_CHECK_POINTER(pu32AgainLin);
	CMOS_CHECK_POINTER(pu32AgainDb);

	int Again_table_size = sizeof(AgainInfo) / sizeof(AgainInfo[0]);

	if (*pu32AgainLin >= AgainInfo[Again_table_size - 1].gainValue) {
		*pu32AgainLin = AgainInfo[Again_table_size - 1].gainValue;
		*pu32AgainDb = Again_table_size - 1;
		return CVI_SUCCESS;
	}

	for (i = 1; i < Again_table_size; i++) {
		if (*pu32AgainLin < AgainInfo[i].gainValue) {
			*pu32AgainLin = AgainInfo[i - 1].gainValue;
			*pu32AgainDb = i - 1;
			break;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_dgain_calc_table(VI_PIPE ViPipe, CVI_U32 *pu32DgainLin, CVI_U32 *pu32DgainDb)
{
	int i;

	(void) ViPipe;

	CMOS_CHECK_POINTER(pu32DgainLin);
	CMOS_CHECK_POINTER(pu32DgainDb);

	int Dgain_table_size = sizeof(Dgain_table) / sizeof(Dgain_table[0]);

	if (*pu32DgainLin >= Dgain_table[Dgain_table_size - 1]) {
		*pu32DgainLin = Dgain_table[Dgain_table_size - 1];
		*pu32DgainDb = Dgain_table_size - 1 + 0x10;
		return CVI_SUCCESS;
	}

	for (i = 1; i < Dgain_table_size; i++) {
		if (*pu32DgainLin < Dgain_table[i]) {
			*pu32DgainLin = Dgain_table[i - 1];
			*pu32DgainDb = i - 1 + 0x10;
			break;
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_gains_update(VI_PIPE ViPipe, CVI_U32 *pu32Again, CVI_U32 *pu32Dgain)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	CVI_U32 u32Again;
	CVI_U32 u32Dgain;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pu32Again);
	CMOS_CHECK_POINTER(pu32Dgain);
	pstSnsRegsInfo = &pstSnsState->astSyncInfo[0].snsCfg;

	u32Again = pu32Again[0];
	u32Dgain = pu32Dgain[0];

	if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
		/* linear mode */

		/* find Again register setting. */

		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_0_ADDR].u32Data = 0x4c;
		if (AgainInfo[u32Again].gain0) {
			pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_0_ADDR].u32Data = 0x5c;
		}
		pstSnsRegsInfo->astI2cData[LINEAR_AGAIN_1_ADDR].u32Data = AgainInfo[u32Again].gain1 & 0x1F;

		pstSnsRegsInfo->astI2cData[LINEAR_DGAIN_0_ADDR].u32Data = (u32Dgain & 0xFF);
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_ae_exp_function(AE_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AE_SENSOR_EXP_FUNC_S));
	pstExpFuncs->pfn_cmos_get_ae_default    = cmos_get_ae_default;
	pstExpFuncs->pfn_cmos_fps_set           = cmos_fps_set;
	pstExpFuncs->pfn_cmos_inttime_update    = cmos_inttime_update;
	pstExpFuncs->pfn_cmos_gains_update      = cmos_gains_update;
	pstExpFuncs->pfn_cmos_again_calc_table  = cmos_again_calc_table;
	pstExpFuncs->pfn_cmos_dgain_calc_table  = cmos_dgain_calc_table;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_awb_default(VI_PIPE ViPipe, AWB_SENSOR_DEFAULT_S *pstAwbSnsDft)
{
	CMOS_CHECK_POINTER(pstAwbSnsDft);
	UNUSED(ViPipe);
	memset(pstAwbSnsDft, 0, sizeof(AWB_SENSOR_DEFAULT_S));

	pstAwbSnsDft->u16InitGgain = 1024;
	pstAwbSnsDft->u8AWBRunInterval = 1;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_awb_exp_function(AWB_SENSOR_EXP_FUNC_S *pstExpFuncs)
{
	CMOS_CHECK_POINTER(pstExpFuncs);

	memset(pstExpFuncs, 0, sizeof(AWB_SENSOR_EXP_FUNC_S));

	pstExpFuncs->pfn_cmos_get_awb_default = cmos_get_awb_default;

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_isp_default(VI_PIPE ViPipe, ISP_CMOS_DEFAULT_S *pstDef)
{
	UNUSED(ViPipe);
	memset(pstDef, 0, sizeof(ISP_CMOS_DEFAULT_S));

	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_blc_default(VI_PIPE ViPipe, ISP_CMOS_BLACK_LEVEL_S *pstBlc)
{
	CMOS_CHECK_POINTER(pstBlc);
	UNUSED(ViPipe);
	memset(pstBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));

	memcpy(pstBlc, &g_stIspBlcCalibratio, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	return CVI_SUCCESS;
}

static CVI_S32 cmos_get_wdr_size(VI_PIPE ViPipe, ISP_SNS_ISP_INFO_S *pstIspCfg)
{
	const SC031IOT_RAW_MODE_S *pstMode = CVI_NULL;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstMode = &g_astSc031iot_raw_mode[pstSnsState->u8ImgMode];

	if (pstSnsState->enWDRMode != WDR_MODE_NONE) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport WDRMode: %d\n", pstSnsState->enWDRMode);
	} else {
		pstIspCfg->frm_num = 1;
		memcpy(&pstIspCfg->img_size[0], &pstMode->astImg[0], sizeof(ISP_WDR_SIZE_S));
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_wdr_mode(VI_PIPE ViPipe, CVI_U8 u8Mode)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	pstSnsState->bSyncInit = CVI_FALSE;

	switch (u8Mode) {
	case WDR_MODE_NONE:
		pstSnsState->u8ImgMode = SC031IOT_RAW_MODE_640X480P30;
		pstSnsState->enWDRMode = WDR_MODE_NONE;
		pstSnsState->u32FLStd = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].u32VtsDef;
		break;
	default:
		CVI_TRACE_SNS(CVI_DBG_ERR, "NOT support this mode!\n");
		return CVI_FAILURE;
	}

	pstSnsState->au32FL[0] = pstSnsState->u32FLStd;
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];
	memset(pstSnsState->au32WDRIntTime, 0, sizeof(pstSnsState->au32WDRIntTime));

	return CVI_SUCCESS;
}

static CVI_U32 sensor_cmp_wdr_size(ISP_SNS_ISP_INFO_S *pstWdr1, ISP_SNS_ISP_INFO_S *pstWdr2)
{
	CVI_U32 i;

	if (pstWdr1->frm_num != pstWdr2->frm_num)
		goto _mismatch;
	for (i = 0; i < 2; i++) {
		if (pstWdr1->img_size[i].stSnsSize.u32Width != pstWdr2->img_size[i].stSnsSize.u32Width)
			goto _mismatch;
		if (pstWdr1->img_size[i].stSnsSize.u32Height != pstWdr2->img_size[i].stSnsSize.u32Height)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.s32X != pstWdr2->img_size[i].stWndRect.s32X)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.s32Y != pstWdr2->img_size[i].stWndRect.s32Y)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.u32Width != pstWdr2->img_size[i].stWndRect.u32Width)
			goto _mismatch;
		if (pstWdr1->img_size[i].stWndRect.u32Height != pstWdr2->img_size[i].stWndRect.u32Height)
			goto _mismatch;
	}

	return 0;
_mismatch:
	return 1;
}

static CVI_U32 sensor_cmp_cif_wdr(ISP_SNS_CIF_INFO_S *pstWdr1, ISP_SNS_CIF_INFO_S *pstWdr2)
{
	if (pstWdr1->wdr_manual.l2s_distance != pstWdr2->wdr_manual.l2s_distance)
		goto _mismatch;
	if (pstWdr1->wdr_manual.lsef_length != pstWdr2->wdr_manual.lsef_length)
		goto _mismatch;

	return 0;
_mismatch:
	return 1;
}

static CVI_S32 cmos_get_sns_regs_info(VI_PIPE ViPipe, ISP_SNS_SYNC_INFO_S *pstSnsSyncInfo)
{
	CVI_U32 i;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;
	ISP_SNS_REGS_INFO_S *pstSnsRegsInfo = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg0 = CVI_NULL;
	ISP_SNS_SYNC_INFO_S *pstCfg1 = CVI_NULL;
	ISP_I2C_DATA_S *pstI2c_data = CVI_NULL;

	CMOS_CHECK_POINTER(pstSnsSyncInfo);
	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	pstSnsRegsInfo = &pstSnsSyncInfo->snsCfg;
	pstCfg0 = &pstSnsState->astSyncInfo[0];
	pstCfg1 = &pstSnsState->astSyncInfo[1];
	pstI2c_data = pstCfg0->snsCfg.astI2cData;

	if ((pstSnsState->bSyncInit == CVI_FALSE) || (pstSnsRegsInfo->bConfig == CVI_FALSE)) {
		pstCfg0->snsCfg.enSnsType = SNS_I2C_TYPE;
		pstCfg0->snsCfg.unComBus.s8I2cDev = g_aunSc031iot_raw_BusInfo[ViPipe].s8I2cDev;
		pstCfg0->snsCfg.u8Cfg2ValidDelayMax = 0;
		pstCfg0->snsCfg.use_snsr_sram = CVI_TRUE;
		pstCfg0->snsCfg.u32RegNum = LINEAR_REGS_NUM;

		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			pstI2c_data[i].bUpdate = CVI_TRUE;
			pstI2c_data[i].u8DevAddr = sc031iot_raw_i2c_addr;
			pstI2c_data[i].u32AddrByteNum = sc031iot_raw_addr_byte;
			pstI2c_data[i].u32DataByteNum = sc031iot_raw_data_byte;
		}

		switch (pstSnsState->enWDRMode) {
		case WDR_MODE_NONE:
			//Linear Mode Regs
			pstI2c_data[LINEAR_PAGE_EXP_ADDR].u32RegAddr = SC031IOT_RAW_PAGE_ADDR;
			pstI2c_data[LINEAR_PAGE_EXP_ADDR].u32Data = SC031IOT_RAW_PAGE_EXP_ADDR;
			pstI2c_data[LINEAR_PAGE_EXP_ADDR].bUpdate = CVI_TRUE;
			pstI2c_data[LINEAR_EXP_0_ADDR].u32RegAddr = SC031IOT_RAW_EXP_0_ADDR;
			pstI2c_data[LINEAR_EXP_1_ADDR].u32RegAddr = SC031IOT_RAW_EXP_1_ADDR;

			pstI2c_data[LINEAR_PAGE_AGAIN_ADDR].u32RegAddr = SC031IOT_RAW_PAGE_ADDR;
			pstI2c_data[LINEAR_PAGE_AGAIN_ADDR].u32Data = SC031IOT_RAW_PAGE_AGAIN_ADDR;
			pstI2c_data[LINEAR_AGAIN_0_ADDR].u32RegAddr = SC031IOT_RAW_AGAIN_0_ADDR;
			pstI2c_data[LINEAR_AGAIN_1_ADDR].u32RegAddr = SC031IOT_RAW_AGAIN_1_ADDR;

			pstI2c_data[LINEAR_PAGE_DGAIN_ADDR].u32RegAddr = SC031IOT_RAW_PAGE_ADDR;
			pstI2c_data[LINEAR_PAGE_DGAIN_ADDR].u32Data = SC031IOT_RAW_PAGE_DGAIN_ADDR;
			pstI2c_data[LINEAR_DGAIN_0_ADDR].u32RegAddr = SC031IOT_RAW_DGAIN_0_ADDR;


			pstI2c_data[LINEAR_PAGE_VMAX_ADDR].u32RegAddr = SC031IOT_RAW_PAGE_ADDR;
			pstI2c_data[LINEAR_PAGE_VMAX_ADDR].u32Data = SC031IOT_RAW_PAGE_VMAX_ADDR;
			pstI2c_data[LINEAR_PAGE_VMAX_ADDR].bUpdate = CVI_TRUE;
			pstI2c_data[LINEAR_VMAX_0_ADDR].u32RegAddr = SC031IOT_RAW_VMAX_0_ADDR;
			pstI2c_data[LINEAR_VMAX_1_ADDR].u32RegAddr = SC031IOT_RAW_VMAX_1_ADDR;

			break;
		default:
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support WDR: %d\n", pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
		pstSnsState->bSyncInit = CVI_TRUE;
		pstCfg0->snsCfg.need_update = CVI_TRUE;
		/* recalcualte WDR size */
		cmos_get_wdr_size(ViPipe, &pstCfg0->ispCfg);
		pstCfg0->ispCfg.need_update = CVI_TRUE;
	} else {
		pstCfg0->snsCfg.need_update = CVI_FALSE;
		for (i = 0; i < pstCfg0->snsCfg.u32RegNum; i++) {
			if (pstCfg0->snsCfg.astI2cData[i].u32Data == pstCfg1->snsCfg.astI2cData[i].u32Data) {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_FALSE;
			} else {
				pstCfg0->snsCfg.astI2cData[i].bUpdate = CVI_TRUE;
				pstCfg0->snsCfg.need_update = CVI_TRUE;

				if (i == LINEAR_VMAX_0_ADDR || i == LINEAR_VMAX_1_ADDR) {
					pstI2c_data[LINEAR_PAGE_VMAX_ADDR].bUpdate = CVI_TRUE;
				}
				else if (i == LINEAR_EXP_0_ADDR || i == LINEAR_EXP_1_ADDR
					|| i == LINEAR_AGAIN_0_ADDR || i == LINEAR_AGAIN_1_ADDR
					|| i == LINEAR_DGAIN_0_ADDR) {
					pstI2c_data[LINEAR_PAGE_EXP_ADDR].bUpdate = CVI_TRUE;
				}
			}
		}

		/* check update isp crop or not */
		pstCfg0->ispCfg.need_update = (sensor_cmp_wdr_size(&pstCfg0->ispCfg, &pstCfg1->ispCfg) ?
				CVI_TRUE : CVI_FALSE);

		/* check update cif wdr manual or not */
		pstCfg0->cifCfg.need_update = (sensor_cmp_cif_wdr(&pstCfg0->cifCfg, &pstCfg1->cifCfg) ?
				CVI_TRUE : CVI_FALSE);
	}

	pstSnsRegsInfo->bConfig = CVI_FALSE;
	memcpy(pstSnsSyncInfo, &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	memcpy(&pstSnsState->astSyncInfo[1], &pstSnsState->astSyncInfo[0], sizeof(ISP_SNS_SYNC_INFO_S));
	pstSnsState->au32FL[1] = pstSnsState->au32FL[0];

	return CVI_SUCCESS;
}

static CVI_S32 cmos_set_image_mode(VI_PIPE ViPipe, ISP_CMOS_SENSOR_IMAGE_MODE_S *pstSensorImageMode)
{
	CVI_U8 u8SensorImageMode = 0;
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	CMOS_CHECK_POINTER(pstSensorImageMode);
	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);

	u8SensorImageMode = pstSnsState->u8ImgMode;
	pstSnsState->bSyncInit = CVI_FALSE;

	if (pstSensorImageMode->f32Fps <= 30) {
		if (pstSnsState->enWDRMode == WDR_MODE_NONE) {
			if (SC031IOT_RAW_RES_IS_480P(pstSensorImageMode->u16Width, pstSensorImageMode->u16Height))
				u8SensorImageMode = SC031IOT_RAW_MODE_640X480P30;
			else {
				CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
					      pstSensorImageMode->u16Width,
					      pstSensorImageMode->u16Height,
					      pstSensorImageMode->f32Fps,
					      pstSnsState->enWDRMode);
				return CVI_FAILURE;
			}
		} else {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Not support! Width:%d, Height:%d, Fps:%f, WDRMode:%d\n",
				      pstSensorImageMode->u16Width,
				      pstSensorImageMode->u16Height,
				      pstSensorImageMode->f32Fps,
				      pstSnsState->enWDRMode);
			return CVI_FAILURE;
		}
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Not support this Fps:%f\n", pstSensorImageMode->f32Fps);
		return CVI_FAILURE;
	}

	if ((pstSnsState->bInit == CVI_TRUE) && (u8SensorImageMode == pstSnsState->u8ImgMode)) {
		/* Don't need to switch SensorImageMode */
		return CVI_FAILURE;
	}

	pstSnsState->u8ImgMode = u8SensorImageMode;

	return CVI_SUCCESS;
}

static CVI_VOID sensor_global_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER_VOID(pstSnsState);

	pstSnsState->bInit = CVI_FALSE;
	pstSnsState->bSyncInit = CVI_FALSE;
	pstSnsState->u8ImgMode = SC031IOT_RAW_MODE_640X480P30;
	pstSnsState->enWDRMode = WDR_MODE_NONE;
	pstSnsState->u32FLStd  = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[0] = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].u32VtsDef;
	pstSnsState->au32FL[1] = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].u32VtsDef;

	memset(&pstSnsState->astSyncInfo[0], 0, sizeof(ISP_SNS_SYNC_INFO_S));
	memset(&pstSnsState->astSyncInfo[1], 0, sizeof(ISP_SNS_SYNC_INFO_S));
}

static CVI_S32 sensor_rx_attr(VI_PIPE ViPipe, SNS_COMBO_DEV_ATTR_S *pstRxAttr)
{
	ISP_SNS_STATE_S *pstSnsState = CVI_NULL;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pstSnsState);
	CMOS_CHECK_POINTER(pstSnsState);
	CMOS_CHECK_POINTER(pstRxAttr);

	memcpy(pstRxAttr, &sc031iot_raw_rx_attr, sizeof(*pstRxAttr));

	pstRxAttr->img_size.width = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Width;
	pstRxAttr->img_size.height = g_astSc031iot_raw_mode[pstSnsState->u8ImgMode].astImg[0].stSnsSize.u32Height;
	if (pstSnsState->enWDRMode == WDR_MODE_NONE)
		pstRxAttr->mipi_attr.wdr_mode = CVI_MIPI_WDR_MODE_NONE;

	return CVI_SUCCESS;

}

static CVI_S32 sensor_patch_rx_attr(RX_INIT_ATTR_S *pstRxInitAttr)
{
	SNS_COMBO_DEV_ATTR_S *pstRxAttr = &sc031iot_raw_rx_attr;
	int i;

	CMOS_CHECK_POINTER(pstRxInitAttr);

	if (pstRxInitAttr->stMclkAttr.bMclkEn)
		pstRxAttr->mclk.cam = pstRxInitAttr->stMclkAttr.u8Mclk;

	if (pstRxInitAttr->MipiDev >= 2)
		return CVI_SUCCESS;
	pstRxAttr->devno = pstRxInitAttr->MipiDev;

	if (pstRxAttr->input_mode == INPUT_MODE_MIPI) {
		struct mipi_dev_attr_s *attr = &pstRxAttr->mipi_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	} else {
		struct lvds_dev_attr_s *attr = &pstRxAttr->lvds_attr;

		for (i = 0; i < MIPI_LANE_NUM + 1; i++) {
			attr->lane_id[i] = pstRxInitAttr->as16LaneId[i];
			attr->pn_swap[i] = pstRxInitAttr->as8PNSwap[i];
		}
	}

	return CVI_SUCCESS;
}

static CVI_S32 cmos_init_sensor_exp_function(ISP_SENSOR_EXP_FUNC_S *pstSensorExpFunc)
{
	CMOS_CHECK_POINTER(pstSensorExpFunc);

	memset(pstSensorExpFunc, 0, sizeof(ISP_SENSOR_EXP_FUNC_S));

	pstSensorExpFunc->pfn_cmos_sensor_init = sc031iot_raw_init;
	pstSensorExpFunc->pfn_cmos_sensor_exit = sc031iot_raw_exit;
	pstSensorExpFunc->pfn_cmos_sensor_global_init = sensor_global_init;
	pstSensorExpFunc->pfn_cmos_set_image_mode = cmos_set_image_mode;
	pstSensorExpFunc->pfn_cmos_set_wdr_mode = cmos_set_wdr_mode;
	pstSensorExpFunc->pfn_cmos_get_isp_default = cmos_get_isp_default;
	pstSensorExpFunc->pfn_cmos_get_isp_black_level = cmos_get_blc_default;
	pstSensorExpFunc->pfn_cmos_get_sns_reg_info = cmos_get_sns_regs_info;

	return CVI_SUCCESS;
}

/****************************************************************************
 * callback structure                                                       *
 ****************************************************************************/
static CVI_VOID sensor_patch_i2c_addr(CVI_S32 s32I2cAddr)
{
	if (SC031IOT_RAW_I2C_ADDR_IS_VALID(s32I2cAddr))
		sc031iot_raw_i2c_addr = s32I2cAddr;
}

static CVI_S32 sc031iot_raw_set_bus_info(VI_PIPE ViPipe, ISP_SNS_COMMBUS_U unSNSBusInfo)
{
	g_aunSc031iot_raw_BusInfo[ViPipe].s8I2cDev = unSNSBusInfo.s8I2cDev;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_ctx_init(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);

	if (pastSnsStateCtx == CVI_NULL) {
		pastSnsStateCtx = (ISP_SNS_STATE_S *)malloc(sizeof(ISP_SNS_STATE_S));
		if (pastSnsStateCtx == CVI_NULL) {
			CVI_TRACE_SNS(CVI_DBG_ERR, "Isp[%d] SnsCtx malloc memory failed!\n", ViPipe);
			return -ENOMEM;
		}
	}

	memset(pastSnsStateCtx, 0, sizeof(ISP_SNS_STATE_S));

	SC031IOT_RAW_SENSOR_SET_CTX(ViPipe, pastSnsStateCtx);

	return CVI_SUCCESS;
}

static CVI_VOID sensor_ctx_exit(VI_PIPE ViPipe)
{
	ISP_SNS_STATE_S *pastSnsStateCtx = CVI_NULL;

	SC031IOT_RAW_SENSOR_GET_CTX(ViPipe, pastSnsStateCtx);
	SENSOR_FREE(pastSnsStateCtx);
	SC031IOT_RAW_SENSOR_RESET_CTX(ViPipe);
}

static CVI_S32 sensor_register_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret;
	ISP_SENSOR_REGISTER_S stIspRegister;
	AE_SENSOR_REGISTER_S  stAeRegister;
	AWB_SENSOR_REGISTER_S stAwbRegister;
	ISP_SNS_ATTR_INFO_S   stSnsAttrInfo;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = sensor_ctx_init(ViPipe);

	if (s32Ret != CVI_SUCCESS)
		return CVI_FAILURE;

	stSnsAttrInfo.eSensorId = SC031IOT_RAW_ID;

	s32Ret  = cmos_init_sensor_exp_function(&stIspRegister.stSnsExp);
	s32Ret |= CVI_ISP_SensorRegCallBack(ViPipe, &stSnsAttrInfo, &stIspRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_ae_exp_function(&stAeRegister.stAeExp);
	s32Ret |= CVI_AE_SensorRegCallBack(ViPipe, pstAeLib, &stSnsAttrInfo, &stAeRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret  = cmos_init_awb_exp_function(&stAwbRegister.stAwbExp);
	s32Ret |= CVI_AWB_SensorRegCallBack(ViPipe, pstAwbLib, &stSnsAttrInfo, &stAwbRegister);

	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor register callback function to awb lib failed!\n");
		return s32Ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 sensor_unregister_callback(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib, ALG_LIB_S *pstAwbLib)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	CMOS_CHECK_POINTER(pstAeLib);
	CMOS_CHECK_POINTER(pstAwbLib);

	s32Ret = CVI_ISP_SensorUnRegCallBack(ViPipe, SC031IOT_RAW_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AE_SensorUnRegCallBack(ViPipe, pstAeLib, SC031IOT_RAW_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to ae lib failed!\n");
		return s32Ret;
	}

	s32Ret = CVI_AWB_SensorUnRegCallBack(ViPipe, pstAwbLib, SC031IOT_RAW_ID);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "sensor unregister callback function to awb lib failed!\n");
		return s32Ret;
	}

	sensor_ctx_exit(ViPipe);

	return s32Ret;
}

static CVI_S32 sensor_set_init(VI_PIPE ViPipe, ISP_INIT_ATTR_S *pstInitAttr)
{
	CMOS_CHECK_POINTER(pstInitAttr);

	g_au32InitExposure[ViPipe] = pstInitAttr->u32Exposure;
	g_au32LinesPer500ms[ViPipe] = pstInitAttr->u32LinesPer500ms;
	g_au16InitWBGain[ViPipe][0] = pstInitAttr->u16WBRgain;
	g_au16InitWBGain[ViPipe][1] = pstInitAttr->u16WBGgain;
	g_au16InitWBGain[ViPipe][2] = pstInitAttr->u16WBBgain;
	g_au16SampleRgain[ViPipe] = pstInitAttr->u16SampleRgain;
	g_au16SampleBgain[ViPipe] = pstInitAttr->u16SampleBgain;
	g_au16Sc031iot_raw_GainMode[ViPipe] = pstInitAttr->enGainMode;
	g_au16Sc031iot_raw_UseHwSync[ViPipe] = pstInitAttr->u16UseHwSync;

	return CVI_SUCCESS;
}

static CVI_S32 sensor_probe(VI_PIPE ViPipe)
{
	return sc031iot_raw_probe(ViPipe);
}

ISP_SNS_OBJ_S stSnsSC031IOT_RAW_Obj = {
	.pfnRegisterCallback    = sensor_register_callback,
	.pfnUnRegisterCallback  = sensor_unregister_callback,
	.pfnStandby             = NULL,
	.pfnRestart             = NULL,
	.pfnWriteReg            = sc031iot_raw_write_register,
	.pfnReadReg             = sc031iot_raw_read_register,
	.pfnSetBusInfo          = sc031iot_raw_set_bus_info,
	.pfnSetInit             = sensor_set_init,
	.pfnMirrorFlip          = sc031iot_raw_mirror_flip,
	.pfnPatchRxAttr         = sensor_patch_rx_attr,
	.pfnPatchI2cAddr        = sensor_patch_i2c_addr,
	.pfnGetRxAttr           = sensor_rx_attr,
	.pfnExpSensorCb         = cmos_init_sensor_exp_function,
	.pfnExpAeCb             = cmos_init_ae_exp_function,
	.pfnSnsProbe            = sensor_probe,
};
