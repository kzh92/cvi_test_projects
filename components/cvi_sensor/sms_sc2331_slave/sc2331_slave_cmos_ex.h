#ifndef __SC2331_SLAVE_CMOS_EX_H_
#define __SC2331_SLAVE_CMOS_EX_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define syslog(level, fmt, ...) printf(fmt, ##__VA_ARGS__)

enum sc2331_slave_linear_regs_e {
	LINEAR_SHS1_0_ADDR,
	LINEAR_SHS1_1_ADDR,
	LINEAR_SHS1_2_ADDR,
	LINEAR_AGAIN_ADDR,
	LINEAR_DGAIN_0_ADDR,
	LINEAR_DGAIN_1_ADDR,
	LINEAR_VMAX_0_ADDR,
	LINEAR_VMAX_1_ADDR,

	LINEAR_REGS_NUM
};

typedef enum _SC2331_SLAVE_MODE_E {
	SC2331_SLAVE_MODE_1600X1200P30 = 0,
	SC2331_SLAVE_MODE_LINEAR_NUM,
	SC2331_SLAVE_MODE_NUM
} SC2331_SLAVE_MODE_E;

typedef struct _SC2331_SLAVE_STATE_S {
	CVI_U32		u32Sexp_MAX;
} SC2331_SLAVE_STATE_S;

typedef struct _SC2331_SLAVE_MODE_S {
	ISP_WDR_SIZE_S astImg[2];
	CVI_FLOAT f32MaxFps;
	CVI_FLOAT f32MinFps;
	CVI_U32 u32HtsDef;
	CVI_U32 u32VtsDef;
	SNS_ATTR_S stExp[2];
	SNS_ATTR_LARGE_S stAgain[2];
	SNS_ATTR_LARGE_S stDgain[2];
	char name[64];
} SC2331_SLAVE_MODE_S;

/****************************************************************************
 * external variables and functions                                         *
 ****************************************************************************/

extern ISP_SNS_STATE_S *g_pastSc2331_slave[VI_MAX_PIPE_NUM];
extern ISP_SNS_COMMBUS_U g_aunSc2331_slave_BusInfo[];
extern ISP_SNS_MIRRORFLIP_TYPE_E g_aeSc2331_slave_MirrorFip[VI_MAX_PIPE_NUM];
extern CVI_U8 sc2331_slave_i2c_addr;
extern const CVI_U32 sc2331_slave_addr_byte;
extern const CVI_U32 sc2331_slave_data_byte;
extern void sc2331_slave_init(VI_PIPE ViPipe);
extern void sc2331_slave_exit(VI_PIPE ViPipe);
extern void sc2331_slave_standby(VI_PIPE ViPipe);
extern void sc2331_slave_restart(VI_PIPE ViPipe);
extern void sc2331_slave_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);
extern int  sc2331_slave_write_register(VI_PIPE ViPipe, int addr, int data);
extern int  sc2331_slave_read_register(VI_PIPE ViPipe, int addr);
extern int  sc2331_slave_write_register_ex(VI_PIPE ViPipe, int addr, int data, int is_left_camera);
extern int  sc2331_slave_read_register_ex(VI_PIPE ViPipe, int addr, int is_left_camera);
extern int  sc2331_slave_probe(VI_PIPE ViPipe);
// extern int  sc2331_slave_switch(VI_PIPE ViPipe, CVI_U8 switchCam);
extern int  sc2331_slave_pattern_enable(VI_PIPE ViPipe, CVI_U8);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC2331_SLAVE_CMOS_EX_H_ */

