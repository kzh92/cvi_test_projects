#ifndef ENGINE_INNER_PARAM
#define ENGINE_INNER_PARAM


#define ENGINE_SECURITY_ONLY_COMMON 0
#define ENGINE_SECURITY_TWIN_COMMON 1
#ifdef USE_TWIN_ENGINE
#define ENGINE_SECURITY_MODE    ENGINE_SECURITY_TWIN_COMMON
#define DEFAULT_TWINS_MODE      0   //
#else // USE_TWIN_ENGINE
#define ENGINE_SECURITY_MODE    ENGINE_SECURITY_ONLY_COMMON
#define DEFAULT_TWINS_MODE      0   //
#endif // USE_TWIN_ENGINE

#define H_DICT_SIZE1        (0x47094)
#define H_DICT_SIZE2        (0x410D5C)

//mfnp_ir01nq
#define DNN_FEAT_CHECKSUM       0xee753512

//fdir_1.0 cv
#define DNN_DETECT_CHECKSUM     0x68d3414

//vfl_1.10 cv
#define DNN_MODELING_CHECKSUM   0x5721b59a

//hand
#define DNN_DETECT_HAND_CHECKSUM        0xee7beb81
#define DNN_MODELING_HAND_CHECKSUM      0xf529de82
#define DNN_CHECKVALID_HAND_CHECKSUM    0x6f1dabb9
#define DNN_FEAT_HAND_CHECKSUM          0x333f25f

////////     Live_SC_1.10 cv     ////////
#define DNN_2D_LIVE_A1_CHECKSUM    	0x9f049758
#define DNN_2D_LIVE_A2_CHECKSUM    	0xc7decdf9
#define DNN_2D_LIVE_B_CHECKSUM  	0xba850a53
#define DNN_2D_LIVE_B2_CHECKSUM 	0x3216f6b8
#define DNN_3D_LIVE_CHECKSUM    	0xc7dd8e2d

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
#define MIN_USER_LUM_HAND   (100)
#define MAX_USER_LUM_HAND   (150)
#define MIN_USER_ENROLL_LUM (120)
#define MIN_SCREEN_LUM        (55)
#define MAX_SCREEN_LUM        (55)
#define MAX_SCREEN_GAIN       (0x2F)
#define EXP_STEP            (2)
#define MIN_EXP             (0x0A)
#define MAX_EXP             (0x33A)

#define MID_EXP			600

#define INIT_EXP            (0x258)
#define INIT_GAIN           (0x01)
#define INIT_FINEGAIN       (0x80)

#define INIT_EXP_1          (0x1F4)
#define INIT_GAIN_1         (0x00)
#define INIT_FINEGAIN_1     (0x80)

//dark env
#define INIT_EXP_2          (0x258)
#define INIT_GAIN_2         (0x01)
#define INIT_FINEGAIN_2     (0x80)
#define INIT_EXP_3          (0x258)
#define INIT_GAIN_3         (0x00)
#define INIT_FINEGAIN_3     (0xC0)
#define INIT_EXP_4          (0x258)
#define INIT_GAIN_4         (0x03)
#define INIT_FINEGAIN_4     (0x80)
#define INIT_EXP_5          (0x33A)
#define INIT_GAIN_5         (0x03)
#define INIT_FINEGAIN_5     (0xC0)


//common indoor env
#define INIT_EXP_6          (0x190)
#define INIT_GAIN_6         (0x00)
#define INIT_FINEGAIN_6     (0x80)
#define INIT_EXP_7          (0x140)
#define INIT_GAIN_7         (0x00)
#define INIT_FINEGAIN_7     (0x80)
#define INIT_EXP_8          (0xA0)
#define INIT_GAIN_8         (0x00)
#define INIT_FINEGAIN_8     (0x80)
#define INIT_EXP_9          (0x50)
#define INIT_GAIN_9     	(0x00)
#define INIT_FINEGAIN_9     (0x80)

//outdoor env
#define INIT_EXP_10          (0x50)
#define INIT_GAIN_10         (0x00)
#define INIT_FINEGAIN_10     (0x80)
#define INIT_EXP_11          (0x30)
#define INIT_GAIN_11         (0x00)
#define INIT_FINEGAIN_11     (0x80)
#define INIT_EXP_12         (0xA0)
#define INIT_GAIN_12        (0x00)
#define INIT_FINEGAIN_12    (0x80)
#define INIT_EXP_13         (0x140)
#define INIT_GAIN_13        (0x00)
#define INIT_FINEGAIN_13    (0x80)
#define INIT_EXP_14         (0x190)
#define INIT_GAIN_14        (0x00)
#define INIT_FINEGAIN_14    (0x80)
#define INIT_EXP_15         (0x258)
#define INIT_GAIN_15        (0x00)
#define INIT_FINEGAIN_15    (0x80)


//added by KSB 20180710
#define MIN_HAND_LUM        (120)
#define MAX_HAND_LUM        (140)
#define INIT_HAND_EXP       (300)
#define INIT_HAND_EXP_PLANB (150)

#define DETECT_FAIL_COUNT   (1)

#define MAX_GAIN            (0x0F)
#define MIN_GAIN            (0x00)
#define MAX_FINEGAIN        (0xFE)
#define MIN_FINEGAIN        (0x00)

//mfnp_ir01nq
#define DNN_VERIFY_THRESHOLD						84.8f
#define DNN_UPDATE_THRESHOLD						86.5f
#define DNN_UPDATE_THRESHOLD_MAX                    92.0f
#define DNN_ENROLL_CHECK_THRESHOLD                  69.0f

#define MIN_DNN_LUM     40      //added by KSB 20180718
#define MIN_DNN_LUM_Hand     40      //added by KSB 20180718

//threshold.txt
#define THRESHOLD1      627      //verify threshold
#define THRESHOLD2      627      //update threshold
#define THRESHOLD3      627      //#ENROLL_DUPLICATION_THRESHOLD	627
#define THRESHOLD4      660		//update max threshold
#define THRESHOLD5      610		//update threshold indoor-outdoor or outdoor-outdoor


#define UPDATE_A            1
#define MOTION_OFFSET       8
#define MOTION_FLAG         1
#define SAT_THRESHOLD       10

#define KDNN_FEAT_SIZE      256

#define TOTAL_ENROLL_MAX_DNNFEATURE_COUNT	8
#define INIT_ENROLL_DNNFEATURE_COUNT        3
#define TOTAL_ENROLL_DNNFEATURE_SIZE        0x2000

#define LEDOFFIMAGE_REDUCE_RATE     5

#endif//ifndef ENGINE_INNER_PARAM

