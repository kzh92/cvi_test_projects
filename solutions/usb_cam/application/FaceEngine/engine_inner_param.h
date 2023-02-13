#ifndef ENGINE_INNER_PARAM
#define ENGINE_INNER_PARAM

#define ENGINE_SECURITY_ONLY_COMMON 0
#define ENGINE_SECURITY_TWIN_COMMON 1
#ifdef USE_TWIN_ENGINE
#define ENGINE_SECURITY_MODE    ENGINE_SECURITY_TWIN_COMMON
#define DEFAULT_TWINS_MODE      0   //
#else // USE_TWIN_ENGINE
#define ENGINE_SECURITY_MODE    ENGINE_SECURITY_ONLY_COMMON
#define DEFAULT_TWINS_MODE      1   //
#endif // USE_TWIN_ENGINE

#define H_DICT_SIZE1        (0x47094)
#define H_DICT_SIZE2        (0x410D5C)

//mfnp_ir01nq
#define DNN_FEAT_CHECKSUM       0xee753512
//fdir_1.0
#define DNN_DETECT_CHECKSUM     0xf356043f
//vfl_1.10
#define DNN_MODELING_CHECKSUM   0x75e6b55c

////////     Live_JX-H62_2.04       ////////
#define DNN_2D_LIVE_A1_CHECKSUM    	0x136db36a
#define DNN_2D_LIVE_A2_CHECKSUM    	0x69095c48
#define DNN_2D_LIVE_B_CHECKSUM  	0x8cbc59a1
#define DNN_2D_LIVE_B2_CHECKSUM 	0x80e12267
#define DNN_3D_LIVE_CHECKSUM    	0xfa279d1a

#define DNN_ESN_CHECKSUM        0x554ad340
#define DNN_OCC_CHECKSUM        0x7e3a5a09

#define DNN_H_1_CHECKSUM        0x5042499f
#define DNN_H_2_CHECKSUM        0x469ea0d5

#define MAX_FACE_NUM        5

#define GLOBAL_MEM_ADDR     0x43200000
#define GLOBAL_MEM_SIZE     0x190000

//camera control params
#define MIN_USER_LUM        (100)
#define MAX_USER_LUM        (150)
#define MIN_USER_ENROLL_LUM (120)
#define MIN_SCREEN_LUM        (55)
#define MAX_SCREEN_LUM        (55)
#define MAX_SCREEN_GAIN       (0x2F)
#define EXP_STEP            (2)
#define MIN_EXP             (0x0A)
#define MAX_EXP             (0x33A)
#define INIT_EXP            (0x258)
#define INIT_EXP_PLANB      (0x190)

#define MID_EXP			600

#define INIT_EXP_1		0x12C
#define INIT_GAIN_1		0x00

//dark env
#define INIT_EXP_2		0x258
#define INIT_GAIN_2		0x0F
#define INIT_EXP_3		0x258
#define INIT_GAIN_3		0x08

//common indoor env
#define INIT_EXP_4		0x12C
#define INIT_GAIN_4		0x00
#define INIT_EXP_5		0xC8
#define INIT_GAIN_5		0x00
#define INIT_EXP_6		0x64
#define INIT_GAIN_6		0x00
#define INIT_EXP_7		0x32
#define INIT_GAIN_7		0x00

//outdoor env
#define INIT_EXP_8		0x32
#define INIT_GAIN_8		0x00
#define INIT_EXP_9		0x1E
#define INIT_GAIN_9		0x00
#define INIT_EXP_10		0x64
#define INIT_GAIN_10	0x00
#define INIT_EXP_11		0xC4
#define INIT_GAIN_11	0x00


//added by KSB 20180710
#define MIN_HAND_LUM        (120)
#define MAX_HAND_LUM        (140)
#define INIT_HAND_EXP       (300)
#define INIT_HAND_EXP_PLANB (150)

#define DETECT_FAIL_COUNT   (1)
#define MAX_GAIN            (0x1F)
#define MIN_GAIN            (0x00)
#define INIT_GAIN           (0x08)
#define INIT_GAIN_PLANB     (0x00)


//mfnp_ir01nq
#define DNN_VERIFY_THRESHOLD						84.8f
#define DNN_UPDATE_THRESHOLD						86.5f
#define DNN_UPDATE_THRESHOLD_MAX                    92.0f
#define DNN_ENROLL_CHECK_THRESHOLD                  69.0f

#define MIN_DNN_LUM     40      //added by KSB 20180718

//threshold.txt
#define THRESHOLD1      627      //verify threshold
#define THRESHOLD2      627      //update threshold
#define THRESHOLD3      627      //#ENROLL_DUPLICATION_THRESHOLD	627
#define THRESHOLD4      605		//verify threshold indoor-outdoor or outdoor-outdoor
#define THRESHOLD5      610		//update threshold indoor-outdoor or outdoor-outdoor


#define UPDATE_A            1
#define MOTION_OFFSET       8
#define MOTION_FLAG         1
#define SAT_THRESHOLD       15

#define KDNN_FEAT_SIZE      256

#define TOTAL_ENROLL_MAX_DNNFEATURE_COUNT	8
#define INIT_ENROLL_DNNFEATURE_COUNT        3
#define TOTAL_ENROLL_DNNFEATURE_SIZE        0x2000

#endif//ifndef ENGINE_INNER_PARAM

