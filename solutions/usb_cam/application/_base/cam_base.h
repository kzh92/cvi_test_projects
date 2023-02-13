#ifndef __SENSOR2PANEL__H__
#define __SENSOR2PANEL__H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

// #include <pthread.h>
#include "appdef.h"

#if (USE_VDBTASK)
#define DVP_CAM                 0   //color camera
#define TC_MIPI_CAM             1   //left IR
#define TC_MIPI_CAM1            2   //right IR

#define TC_MIPI_CAM_LEFT        TC_MIPI_CAM   //left IR
#define TC_MIPI_CAM_RIGHT       TC_MIPI_CAM1  //right IR

#else // USE_VDBTASK
#define MIPI_0_CAM              0
#define MIPI_1_CAM              1
#endif // USE_VDBTASK

#define MIPI_CAM_SUB0           0 // right IR
#define MIPI_CAM_SUB1           1 // left IR

#define MIPI_CAM_S2RIGHT        MIPI_CAM_SUB0 // switch to right IR
#define MIPI_CAM_S2LEFT         MIPI_CAM_SUB1 // switch to left IR

#define WIDTH_640 640
#define HEIGHT_480 480

#define WIDTH_1280  1280
#define HEIGHT_720  720
#define HEIGHT_800  720

#define WIDTH_960   960
#define HEIGHT_960 	960

#define WIDTH_800   800
#define HEIGHT_600  600

#define IR_BUFFER_SIZE (WIDTH_1280 * HEIGHT_720)
#define CLR_BUFFER_SIZE (WIDTH_1280 * HEIGHT_960 * 2)

enum {
	CAM_ID_CLR,
	CAM_ID_IR1,
	CAM_ID_IR2
};

struct regval_list {
    unsigned char reg_num;
    unsigned char value;
};

#ifndef CLEAR
#define CLEAR(x) memset (&(x), 0, sizeof (x))
#endif

#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))
#define ALIGN_16B_C(x) ((x) & ~(15))

int wait_camera_ready(int id);
int camera_release (int id);

int camera_set_exp_byreg(int id, int value);
int camera_get_exp_byreg(int id);
int camera_set_gain_byreg(int id, int value);
int camera_set_regval(int id, unsigned char regaddr, unsigned char regval);
int camera_get_regval(int id, unsigned char regaddr);
int camera_set_irled(int enable, int count);
int camera_set_pattern_mode(int cam_id, int enable);
void my_mi_use_lock();
void my_mi_use_unlock();
void lockIRBuffer();
void unlockIRBuffer();
#if (USE_VDBTASK)
int camera_init(int id, int width, int height, int switchIR_to);
int camera_set_irled(int id, int enable);
int camera_mipi0_set_regval(unsigned char regaddr, unsigned char regval);
int camera_mipi0_get_regval(unsigned char regaddr);
int camera_mipi1_set_regval(unsigned char regaddr, unsigned char regval);
int camera_mipi1_get_regval(unsigned char regaddr);
int camera_dvp_set_regval(unsigned char regaddr, unsigned char regval);
int camera_dvp_get_regval(unsigned char regaddr);
int camera_switch(int id, int camid);
int camera_get_actIR();
int camera_clr_set_exp(int value);
int camera_clr_set_gain(int value);
int camera_clr_get_exp();
int camera_clr_get_gain();
void lockClrBuffer();
void unlockClrBuffer();
#else // USE_VDBTASK
int camera_init(int id, int width, int height);
#endif // USE_VDBTASK

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__SENSOR2PANEL__H__
