#ifndef APPDEF
#define APPDEF

#define PACK_FOR_X5S        0 // 0: V3s, 1: X5S

#define USE_222MODE         1
#define USE_SSD210          0

#define PT_SEMI_AUTO        0
#define PT_AUTO             1
#define PRODUCT_TYPE        PT_AUTO

#define SAMPLE_AIPAI        0
#define SAMPLE_TONGXIN      1
#define SAMPLE_NAME         SAMPLE_TONGXIN

#define LM_AUTO             0
#define LM_SEMI_AUTO        1

//zigbee mode
#define ZIGBEE_BASE         0x10
#define WIFI_BASE           0x20
#define ZIGBEE_MAX_NUM      5

#define ZIGBEE_DISABLE      0
#define ZIGBEE_OUR          (ZIGBEE_BASE + 0)
#define WIFI_YINGHUA_JIWEI  (WIFI_BASE + 0)
#define WIFI_YINGHUA_SIGE   (WIFI_BASE + 1)
#define WIFI_GESANG_SIGE    (WIFI_BASE + 2)

#define TOUCH_GT9XX         0
#define TOUCH_FT            1
#define TOUCH_MODE          TOUCH_FT

#define FP_NONE             0
#define FP_ZHIAN            1
#define FP_TUZHENG          2
#define FP_GOWEI            3

#define FM_EASEN            0
#define FM_DESMAN           1
#define FM_PROTOCOL         FM_DESMAN

#if (PRODUCT_TYPE == PT_SEMI_AUTO)
    #define LOCK_MODE           LM_SEMI_AUTO
    #define ZIGBEE_MODE         ZIGBEE_OUR
    #define HUMAN_SENSOR        0
    #define FP_MODE             FP_NONE                   //0 -> none, 1 -> v3s, 2 -> in stm

#elif (PRODUCT_TYPE == PT_AUTO)
    #define LOCK_MODE           LM_AUTO
    #define ZIGBEE_MODE         ZIGBEE_DISABLE

    #define HUMAN_SENSOR        1
    #define FP_MODE             FP_GOWEI
#endif

//uart baud rate
enum E_Baud_Rate
{
    Baud_Rate_Min = 0,
    Baud_Rate_115200 = 1, // 115200
    Baud_Rate_230400 = 2, // 230400
    Baud_Rate_460800 = 3, // 460800
    Baud_Rate_1500000 = 4, // 1500000
    Baud_Rate_9600 = 5, // 9600
    Baud_Rate_19200 = 6, // 19200
    Baud_Rate_38400 = 7, // 38400
    Baud_Rate_57600 = 8, // 57600
    Baud_Rate_End = 9,
};

#define BR_IS_VALID(b) ((b) > Baud_Rate_Min && (b) < Baud_Rate_End)

//batt test
#define AUTO_TEST           0     //0 -> normal, 1 -> auto test
#define CAPTURE_SCENE       0
#define ENROLL_DUPLICATION_CHECK    1
#define DEBUG_EN            0
#define FAKE_DETECTION      1
#define ENROLL_FAKE         1
#define FAKE_UNIT_COUNT     3
#define FAKE_TOTAL_COUNT    4
#define TEST_ENGINE_MAP     0
#define CHECK_FAKE_USER     1
#define USE_ENROLL_ITG      1
#define AUTO_CTRL_CLR_CAM   1
#define USE_DEMOMODE2       1
#define USE_WAEL_VDB        0   //10fps, jpg size must be less than 50KB
#define USE_SHENAO_VDB      0
#define USE_SHENAO_NEW_VDB  0
#define USE_WAEL_PROTO      0
#define NOTE_INTERVAL_MS    0

#define UVC_MAX_WIDTH       1280
#define UVC_MAX_HEIGHT      720
#define UVC_MIN_WIDTH       320
#define UVC_MIN_HEIGHT      240
#define UVC_WIDTH           640
#define UVC_HEIGHT          480
#define CHECK_CLR_IR_SWITCH_THR         40
#define UVC_CLR_LUMINANCE   0x80
#define UVC_CLR_SAT_Gl      0x50
#define UVC_CLR_SAT_Cb      0x42
#define UVC_CLR_SAT_Cr      0x42
#define UVC_CLR_SHARP       0x85
#define UVC_CLR_AWB_EN      0
#define UVC_CLR_R_GAIN      0x50
#define UVC_CLR_G_GAIN      0x40
#define UVC_CLR_B_GAIN      0x54
#define UVC_UNIQ_COMPRESS   1
#define UVC_RES_720P        1       //1280x720
#define UVC_RES_480P        2       //640x480
#define UVC_RES_432x240     4
#define UVC_RES_480x320     8       //480x320
#define UVC_RES_240P        16      //320x240
#define UVC_RES_320x480     32      //320x480
#define UVC_RES_FLAG        UVC_RES_480P
#define UVC_RES_COUNT       1

#define USE_VDBTASK         0
#define USE_SMP_CORE1       1
#define CHECK_FIRMWARE      1
#define USE_WIFI_MODULE     0
#define USE_READY0_PROTO    0
#define USE_AUTO_50_REPLY   0

#define SETTING_TIMEOUT             30
#define RESET_TIMEOUT               9
#define MSG_SHOW_TIMEOUT            3
#define ENROLL_CARD_TIMEOUT         10
#define ENROLL_FP_TIMEOUT           10
#define GUOGU_RECV_TIMEOUT          4
#define PASSCODE_NO_ENTER_TIMEOUT   3 * 60
#define GROUP_TIMEOUT 1.5
#define DETECTION_TIMEOUT           10
#define PASSCODE_TIMEOUT            8
#define VIEW_TIMEOUT                5//second
#define USB_UPGRADE_TIMEOUT         8
#define USB_DETECT_TIMEOUT          3

#define N_MAX_FACE_FAILED_COUNT     5
#define N_MAX_PASS_FAILED_COUNT     5
#define N_MAX_FP_FAILED_COUNT       5
#define N_MAX_FAILED_TIME           (2.4)//second
#define N_MAX_DNN_FAILED_TIME       6//second
#define N_MAX_EYE_FAILED_TIME		5//second
#define N_SECURE_CHK_CNT            1000
#define N_DEMO_FACTORY_MODE         0x10

//기본설정값
#define DEFAULT_THEME               0
#define DEFAULT_SHOW_CAM            2
#define DEFAULT_LANGUAGE            0
#define DEFAULT_SOUND               10
#define DEFAULT_PRESENATATION       0
#define DEFAULT_KEEP_OPEN           0
#define DEFAULT_LOCK_TYPE           1
#define DEFAULT_ZIGBEE              0
#define DEFAULT_MOTOR_TYPE          0
#define DEFAULT_MOTOR_POLARITY      0
#define DEFAULT_HOUR_FORMAT         0
#define DEFAULT_DATE_FORMAT         0
#define DEFAULT_GYROSCOPE_ANGLE     1
#define DEFAULT_FINGERPRINT         (FP_MODE > 0 ? 1 : 0)       //대상에 맞게 설정
#define DEFAULT_HUMANSENSOR         HUMAN_SENSOR       //대상에 맞게 설정
#define DEFAULT_LOGO                0
#define DEFAULT_ZIGBEE_MODE         ZIGBEE_MODE
#define DEFAULT_SHOW_DATETIME       1
#define DEFAULT_PINYIN_MODE         1
#define DEFAULT_SEMI_MOTOR_TYPE     0
#define DEFAULT_SEMI_MOTOR_POLARITY 0
#define DEFAULT_SEMI_MOTOR_TIME     30
#define DEFAULT_HUMANSENSOR_PARAM   5       // 1~10까지 파라메터변경방법
#define DEFAULT_GYROSCOPE_TIME      1       // [1,2,3] = [10, 20, 30]
#define DEFAULT_MOTOR_OPEN_TIME     26      //대상에 맞게 설정
#define DEFAULT_MOTOR_BACK_TIME     7
#define DEFAULT_UVC_DIR             0       // 0: not rotate 90, 1: rotate 90
#define DEFAULT_UVC_COMP_PARAM_IMQ  100
#define DEFAULT_UVC_COMP_PARAM_BT_MAX   6
#define DEFAULT_UVC_COMP_PARAM_BT_DEF   4
#define DEFAULT_UVC_COMP_PARAM_RPFR     1
#define DEFAULT_LIVENESS_MODE       1   //engine state for liveness, for special use only
#define DEFAULT_SECURE_VALUE        75  //caution!!! DO NOT MODIFY this value.
#ifdef USE_TWIN_ENGINE
#define DEFAULT_SECURE_FALSE_VAL    45  //caution!!! DO NOT MODIFY this value.
#else // USE_TWIN_ENGINE
#define DEFAULT_SECURE_FALSE_VAL    5  //caution!!! DO NOT MODIFY this value.
#endif // USE_TWIN_ENGINE
#define DEFAULT_SECURE_STEP1        8
#define DEFAULT_SECURE_STEP2        24
#define DEFAULT_PROTO_ENC_MODE      0  //0: plain text mode, 1: default encryption mode, 2: XOR_1
#define DEFAULT_PROTO_EM_XOR1_KEY   "ee71535357ad9bb4" //XOR key for 심전람항
#define DESMAN_ENC_MODE             2       //0 -> dessmman, 1 -> bom, 2 -> test
#define DEFAULT_MI_AO_VOLUME        (0)   // -60~30
#define DEFAULT_MI_AI_VOLUME        20      // 0~21
#define FIRMWARE_MAGIC              "FTSVD"

#if (LOCK_MODE == LM_SEMI_AUTO)
    //batt
    #define BATT_LOW           3700
    #define BATT_LEVEL_STEP    10
    #define BATT_LOW_NEXT      4800
    #define BATT_MENU          4000
    #define BATT_POWER_DOWN    3500
    #define BATT_UPDATE_THRE   700
    #define BATT_SOUND_OFF     4300

    #define DEFAULT_OVERCURRENT         3000    //대상에 맞게 설정
#elif (LOCK_MODE == LM_AUTO)
    //batt
    #define BATT_LOW           6400
    #define BATT_LEVEL_STEP    12
    #define BATT_LOW_NEXT      6600
    #define BATT_MENU          6600
    #define BATT_POWER_DOWN    6100
    #define BATT_UPDATE_THRE   500
    #define BATT_SOUND_OFF     5000

    #define DEFAULT_OVERCURRENT         3000    //대상에 맞게 설정
#endif

#define DESMAN_ENC_MODE     2       //0 -> dessmman, 1 -> bom, 2 -> test

#define DB_TEST             0       //!!!!!!!Must be 0 for product!!!!!!!
#define FEAT_VER                    "F1.0"

#define CAM_RM_DEFAULT      0
#define CAM_RM_180DEGREE    1
#define CAM_ROTATION_MODE   CAM_RM_DEFAULT

#define IR_LED_ONOFF_MODE   1
#if (DESMAN_ENC_MODE == 0)
#define SEND_LAST_MSG       1       //0 -> normal, 1 -> for dessman
#else
#define SEND_LAST_MSG       0       //0 -> normal, 1 -> for dessman
#endif

#define NO_ENCRYPT_FRM3_4   1

#define ES_LIVENESS_MODE    0   //engine state for liveness, for special use only

#define TYPE_EXT4           0
#define TYPE_JFFS           1
#define DB_TYPE             TYPE_JFFS

#define DISABLE_V4L2        1
#define KEEP_LED_ON_FRAMES  4       // keeps led on while x frames

#define DB_BAK3
#define ROOTFS_BAK          0
#define TEST_GYROSCORE_TIME 1
#define TEST_DUTY_CYCLE     0
#define USE_CARD_ENCRYPT    1

#define NFS_DEBUG_EN        0
#define UBOOT_LOAD_SPOOF    0
#define ENGINE_DATA_PACKET_SIZE 128

//////////////////TTF Font////////////////////
#define MY_LINUX
#define USE_BUFFER2
#define USE_FB_FOR_CAP


//#define USE_DEBUG
#define USE_LCD             0

#define my_debug printf

#ifdef USE_DEBUG

#define USE_DEBUG_TRACE

#ifdef USE_DEBUG_TRACE
#endif //USE_DEBUG_TRACE

#endif //USE_DEBUG
///////////////////////////////////////////////

#define SCRIPT_POS          (3072 * 1024)
#define SCRIPT_POS2         (3584 * 1024)
#define UBOOT_POS           (8 * 1024)
#define SPOOF_POS           (232 * 1024 * 1024)

#define MMAP_MODE

#if (FM_PROTOCOL == FM_EASEN)
#define N_MAX_PERSON_NUM                100
#elif (FM_PROTOCOL == FM_DESMAN)
#if (NO_ENCRYPT_FRM3_4 == 0)
#define N_MAX_PERSON_NUM                20
#else // NO_ENCRYPT_FRM3_4 == 0
#define N_MAX_PERSON_NUM                100
#endif // NO_ENCRYPT_FRM3_4 == 0
#endif

#define FN_FACE_BIN "/mnt/MISC/face.bin"

//////////////////////////////////////////////////////////////
/// products
//////////////////////////////////////////////////////////////

#define FRM_PT_DEFAULT              0   //default
#define FRM_PT_LANHANG_MODE         1   //lanhang mode
#define FRM_PT_WAEL_MODE            2   //wael mode
#define FRM_PT_YAOYANG_MODE         3   //yaoyang mode
#define FRM_PT_FUSHI_MODE           4   //fushi protocol
#define FRM_PT_XISHANG_MODE         5   //xishang mode 西尚
#define FRM_PT_SANJIANG_MODE        6   //sanjiang mode
#define FRM_PT_S22_DEFAULT          7   //S22 default
#define FRM_PT_S22_SANJIANG_MODE    8
#define FRM_PT_S22_LANHANG_MODE     9   //lanhang mode
#define FRM_PT_S22_XISHANG_MODE     10
#define FRM_PT_S22_DESSMANN         11  //dessmann

#define FRM_PT_DEFAULT_3_4          100   //3.4 default
#define FRM_PT_3_4_SHUNHUI_MODE     101   //3.4 shunhui mode
#define FRM_PT_3_4_PULIAN_MODE      102   //
#define FRM_PT_3_4_HUANGLI_MODE     103   //
#define FRM_PT_3_4_XIAODIAN_MODE    104   //
#define FRM_PT_3_4_HAIFUSI_MODE     105   //
#define FRM_PT_3_4_GUANGYU_MODE     106   //
#define FRM_PT_3_4_XIJIN_MODE       107   //
#define FRM_PT_3_4_WAEL_MODE        108
#define FRM_PT_S22_DEFAULT_3_4      120   //
#define FRM_PT_S22_3_4_HUANGLI_MODE 121   //
#define FRM_PT_S22_3_4_CHANGSI_MODE 122   //
#define FRM_PT_S22_3_4_XIAODIAN_MODE    123   //
#define FRM_PT_S22_3_4_WAEL_MODE    124
#define FRM_PT_S22_3_4_SANJJIANG_MODE   125   //
#define FRM_PT_S22_3_4_SJ_ENCMODE   126   //
#define FRM_PT_3_5_DEFAULT_MODE     150   //3.5 default
#define FRM_PT_3_5_JIUBANG_MODE     151   //
#define FRM_PT_S22_3_5_DEFAULT_MODE 152   //
#define FRM_PT_S22_3_5_JIUBANG_MODE 153   //
#define FRM_PT_S22_DENAITE_MODE     154     // 절강득내특(浙江得耐特)
#define FRM_PT_3_4_XINNENGYUAN_MODE 155
#define FRM_PT_S22_3_4_XINNENGYUAN_MODE 156

#define FRM_PT_DEFAULT_3_3          200   //3.3 default
#define FRM_PT_DEFAULT_3_3_720P     201   //3.3 720p
#define FRM_PRODUCT_TYPE            FRM_PT_S22_DEFAULT

//---------------------------------------------------------
#if (FRM_PRODUCT_TYPE == FRM_PT_DEFAULT)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "1.9.6_RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.9.6_RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//---------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_DEFAULT)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "1.9.2_RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.9.2_RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//---------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_DESSMANN)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "DS_CA_V1.1.0"
#define DEVICE_FIRMWARE_VERSION_INNER       "DS_CA_V1.1.0"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0x43 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            0
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              1
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef NOTE_INTERVAL_MS
#define NOTE_INTERVAL_MS                    100
#undef DESMAN_ENC_MODE
#define DESMAN_ENC_MODE                     0
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#define ENGINE_FOR_DESSMAN

//---------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_LANHANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "1.11.1_RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.11.1_RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2  //0: plain text mode, 1: default encryption mode, 2: XOR_1
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//---------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_LANHANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "1.11.0_RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.11.0_RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2  //0: plain text mode, 1: default encryption mode, 2: XOR_1
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//---------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_YAOYANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "6.2.0_RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "6.2.0_RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0  //0: plain text mode, 1: default encryption mode, 2: XOR_1
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef YAOYANG_MODE
#define YAOYANG_MODE                        1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//------------------------------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_XISHANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_TYPE_NUM                     "#ES2001\\"
#define DEVICE_FIRMWARE_VERSION             "1.17.0_RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.17.0_RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//------------------------------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_XISHANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_TYPE_NUM                     "#ES2001V"
#define DEVICE_FIRMWARE_VERSION             "1.17.2.1_RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.17.2.1_RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//------------------------------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_SANJIANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "1.14.2_RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.14.2_RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            0
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#undef DEFAULT_PROTO_EM_XOR1_KEY
#define DEFAULT_PROTO_EM_XOR1_KEY           "eb62f6b9306db575" //XOR key
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//------------------------------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_SANJIANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "1.14.1_RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.14.1_RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            0
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#undef DEFAULT_PROTO_EM_XOR1_KEY
#define DEFAULT_PROTO_EM_XOR1_KEY           "eb62f6b9306db575" //XOR key
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//---------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_FUSHI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "1.0.2_RFS "
#define DEVICE_FIRMWARE_VERSION_INNER       "1.0.2_RFS "

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_WAEL_MODE
#define USE_WAEL_MODE                       0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_DEFAULT_3_4)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.2.5 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.2.5 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    80
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   0
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P | UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             40
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x88
#undef UVC_CLR_SAT_Gl
#define UVC_CLR_SAT_Gl                      0x50
#undef UVC_CLR_R_GAIN
#define UVC_CLR_R_GAIN                      0x54
#undef UVC_CLR_G_GAIN
#define UVC_CLR_G_GAIN                      0x40
#undef UVC_CLR_B_GAIN
#define UVC_CLR_B_GAIN                      0x58
#undef UVC_CLR_SAT_Cb
#define UVC_CLR_SAT_Cb                      0x30
#undef UVC_CLR_SAT_Cr
#define UVC_CLR_SAT_Cr                      0x30
#undef UVC_CLR_SHARP
#define UVC_CLR_SHARP                       0xCC
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_DEFAULT_3_4)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.2.4 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.2.4 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    50
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_XIAODIAN_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.1.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.1.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     0
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    0
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_4_XIAODIAN_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.1.0 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.1.0 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     0
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    0
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_SHUNHUI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.3.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.3.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    60      //15fps
#define UVC_FIX_COMPRATE                    90
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x90
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             30
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_WAEL_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.4.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.4.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     0
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    50
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_WAEL_PROTO
#define USE_WAEL_PROTO                      1

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_4_WAEL_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.4.1 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.4.1 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     0
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    50
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_WAEL_PROTO
#define USE_WAEL_PROTO                      1

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_5_JIUBANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.7.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.7.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   0
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P | UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  0
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_5_JIUBANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.7.0 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.7.0 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   0
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P | UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  0
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_HUANGLI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.8.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.8.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  25
#define UVC_MAX_FPS_TIME                    60      //15fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x90
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             30
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_4_HUANGLI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.8.1.1 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.8.1.1 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  25
#define UVC_MAX_FPS_TIME                    60      //15fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x90
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             30
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_4_SJ_ENCMODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.14.1.2 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.14.1.2 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            0
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    0
#undef UVC_WIDTH
#define UVC_WIDTH                           1280
#undef UVC_HEIGHT
#define UVC_HEIGHT                          720
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#undef DEFAULT_PROTO_EM_XOR1_KEY
#define DEFAULT_PROTO_EM_XOR1_KEY           "eb62f6b9306db575" //XOR key

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_4_SANJJIANG_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.15.0 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.15.0 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    50
#undef UVC_WIDTH
#define UVC_WIDTH                           1280
#undef UVC_HEIGHT
#define UVC_HEIGHT                          720
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#undef USE_AUTO_50_REPLY
#define USE_AUTO_50_REPLY                   1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_4_CHANGSI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.23.2.2 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.23.2.2 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    90
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           1280
#undef UVC_HEIGHT
#define UVC_HEIGHT                          720
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef UVC_CLR_SAT_Cb
#define UVC_CLR_SAT_Cb                      0x52
#undef UVC_CLR_SAT_Cr
#define UVC_CLR_SAT_Cr                      0x52

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_5_DEFAULT_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.9.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.9.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   0
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P | UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  0
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_5_DEFAULT_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.9.1 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.9.1 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   0
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P | UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  0
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_XINNENGYUAN_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.12.4 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.12.4 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    60      //25fps
#define UVC_FIX_COMPRATE                    0
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           1280
#undef UVC_HEIGHT
#define UVC_HEIGHT                          720
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             40
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x98
#undef UVC_CLR_SAT_Gl
#define UVC_CLR_SAT_Gl                      0x50
#undef UVC_CLR_R_GAIN
#define UVC_CLR_R_GAIN                      0x54
#undef UVC_CLR_G_GAIN
#define UVC_CLR_G_GAIN                      0x40
#undef UVC_CLR_B_GAIN
#define UVC_CLR_B_GAIN                      0x58
#undef UVC_CLR_SAT_Cb
#define UVC_CLR_SAT_Cb                      0x30
#undef UVC_CLR_SAT_Cr
#define UVC_CLR_SAT_Cr                      0x30
#undef UVC_CLR_SHARP
#define UVC_CLR_SHARP                       0xCC
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_3_4_XINNENGYUAN_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.12.0 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.12.0 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    70
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_720P | UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             40
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0xA0
#undef UVC_CLR_R_GAIN
#define UVC_CLR_R_GAIN                      0x58
#undef UVC_CLR_G_GAIN
#define UVC_CLR_G_GAIN                      0x40
#undef UVC_CLR_B_GAIN
#define UVC_CLR_B_GAIN                      0x58
#undef UVC_CLR_SAT_Gl
#define UVC_CLR_SAT_Gl                      0x60
#undef UVC_CLR_SAT_Cb
#define UVC_CLR_SAT_Cb                      0x60
#undef UVC_CLR_SAT_Cr
#define UVC_CLR_SAT_Cr                      0x60
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_HAIFUSI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.13.0 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.13.0 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    0
#undef UVC_WIDTH
#define UVC_WIDTH                           320
#undef UVC_HEIGHT
#define UVC_HEIGHT                          240
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_240P | UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  0
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             50
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_GUANGYU_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.19.0 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.19.0 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     2
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  25
#define UVC_MAX_FPS_TIME                    60      //15fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x90
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             40
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef CAM_ROTATION_MODE
#define CAM_ROTATION_MODE                   CAM_RM_180DEGREE

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_PULIAN_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.20.0 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.20.0 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     2
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    60      //15fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   0
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             40
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_3_4_XIJIN_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.22.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.22.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     0
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    0
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   1
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200

#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_S22_DENAITE_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "5.26.0 RB"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.26.0 RB"

#undef USE_SSD210
#define USE_SSD210                          1
#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     2
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#undef UVC_MAX_FRAME_SIZE
#define UVC_MAX_FRAME_SIZE                  30
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    30
#undef UVC_UNIQ_COMPRESS
#define UVC_UNIQ_COMPRESS                   0
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_480P | UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       2
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                2200
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  0
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x80
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             20
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef CAM_ROTATION_MODE
#define CAM_ROTATION_MODE                   CAM_RM_180DEGREE

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_DEFAULT_3_3)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "7.3.1 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "7.3.1 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     0
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    50
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             50
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef USE_WIFI_MODULE
#define USE_WIFI_MODULE                     1
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_DEFAULT_3_3_720P)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-143"
#define DEVICE_FIRMWARE_VERSION             "7.6.0 RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "7.6.0 RS"

#define DEVICE_NID_READY_VER                0 //'C'=0x43, for desman, else 0=NID_READY
#undef SEND_LAST_MSG
#define SEND_LAST_MSG                       0       //0 -> normal, 1 -> for dessman
#undef NO_ENCRYPT_FRM3_4
#define NO_ENCRYPT_FRM3_4                   1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            1
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              0
#undef USE_DEMOMODE2
#define USE_DEMOMODE2                       1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     0
#undef USE_WAEL_VDB
#define USE_WAEL_VDB                        0
#define UVC_MAX_FPS_TIME                    40      //25fps
#define UVC_FIX_COMPRATE                    90
#undef UVC_WIDTH
#define UVC_WIDTH                           1280
#undef UVC_HEIGHT
#define UVC_HEIGHT                          720
#undef UVC_RES_FLAG
#define UVC_RES_FLAG                        (UVC_RES_720P)
#undef UVC_RES_COUNT
#define UVC_RES_COUNT                       1
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             30
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_UVC_PAUSE_MODE
#define USE_UVC_PAUSE_MODE                  1
#undef UVC_PAUSE_LIMIT_TIME
#define UVC_PAUSE_LIMIT_TIME                200
#undef USE_WIFI_MODULE
#define USE_WIFI_MODULE                     1
#undef USE_SHENAO_NEW_VDB
#define USE_SHENAO_NEW_VDB                  1
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef UVC_CLR_LUMINANCE
#define UVC_CLR_LUMINANCE                   0x90

#endif // FRM_PRODUCT_TYPE

#if (UVC_MAX_WIDTH < UVC_WIDTH && UVC_MAX_HEIGHT < UVC_HEIGHT)
#error "uvc width and height must be less or equal than uvc max."
#elif (UVC_MAX_WIDTH < UVC_HEIGHT && UVC_MAX_HEIGHT < UVC_WIDTH)
#error "uvc width and height must be less or equal than uvc max2."
#endif // uvc max

//#include "engine_inner_param.h"

#endif // APPDEF

