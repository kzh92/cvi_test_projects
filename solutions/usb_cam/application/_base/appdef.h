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

#define USE_IRSNR_SC2355    1

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

//protocol encryption mode
#define PROTO_EM_NOENCRYPT              0
#define PROTO_EM_ENCRYPT                1
#define PROTO_EM_ENCRYPT_XOR_LANHENG    2
#define PROTO_EM_ENCRYPT_AES_DEFAULT    3
#define PROTO_EM_ENCRYPT_AES_XLAN       4   //AES, XOR lanheng
#define PROTO_EM_ENCRYPT_END            5

#define PROTO_EM_XOR1_KEY_LANHENG       "ee71535357ad9bb4" //XOR key for 심전람항
#define PROTO_EM_XOR1_KEY_SANJIANG      "eb62f6b9306db575" //XOR key for 범해삼강

//board types
#define BD_TY_CV180xB_DEMO_V1v0     0
#define BD_TY_FSDB_1V0              1   //D10 2.0
#define BD_TY_CV181xC_DEMO_V1v0     2
#define BD_TY_FMDASS_1V0J           3   //D20 3M
#define DEFAULT_BOARD_TYPE          BD_TY_FSDB_1V0

//chip types
#define MY_CHIP_D10                 0
#define MY_CHIP_D20                 1
#define DEFAULT_CHIP_TYPE           MY_CHIP_D10

//camera mipi types
#define CAM_MIPI_TY_121             0 //one sensor on one mipi
#define CAM_MIPI_TY_122             1 //two sensors on one mipi
#define DEFAULT_CAM_MIPI_TYPE       CAM_MIPI_TY_122

//batt test
#define AUTO_TEST                   0     //0 -> normal, 1 -> auto test
#define CAPTURE_SCENE               0
// Enroll Duplication Check Mode
#define EDC_DISABLE                 0   //disable duplication check
#define EDC_ENABLE_WITH_SKIP        1   //enable duplication check, but registering possible on dup error
#define EDC_ENABLE_NO_SKIP          2   //enable duplication check, registering impossible on dup error
#define ENROLL_DUPLICATION_CHECK    EDC_ENABLE_WITH_SKIP
#define ENROLL_HAND_DUP_CHECK       1
#define DEBUG_EN                    0
#define FAKE_DETECTION              1
#define ENROLL_FAKE                 1
#define FAKE_UNIT_COUNT             3
#define FAKE_TOTAL_COUNT            4
#define TEST_ENGINE_MAP             0
#define CHECK_FAKE_USER             1
#define USE_ENROLL_ITG              1
#define AUTO_CTRL_CLR_CAM           1
#define USE_DEMOMODE2               1
#define USE_WAEL_VDB                0       //10fps, jpg size must be less than 50KB
#define USE_SHENAO_VDB              0
#define USE_SHENAO_NEW_VDB          0
#define USE_WAEL_PROTO              0
#define NOTE_INTERVAL_MS            0
#define USE_VDBTASK                 0
#define USE_SMP_CORE1               1
#define CHECK_FIRMWARE              1
#define USE_WIFI_MODULE             0
#define USE_READY0_PROTO            0
#define USE_SANJIANG3_MODE          0       // 범해삼강(암호화있음), 중산칠심, 동관동흔조종기판을 함께 쓰는 방식
#define USE_AES_NOENC_MODE          0       // AES with no encryption
#define USE_NEW_SNAPIMAGE_MODE      1       //0:use Flash, 1:unuse Flash
#define USE_XISHANG_PROTO           0       //0x93 command
#define USE_AUTO_50_REPLY           0
#define USE_FP16_ENGINE             1
#define NOTE_INTERVAL_MS            0
#define USE_NEW_RST_PROTO           1
#define USE_16M_FLASH               1
#define ENROLL_ANGLE_MODE           0
#define ENGINE_USE_TWO_CAM          1
#define YAOYANG_MODE                0
#define SEND_LAST_MSG               0
#define USE_3M_MODE                 0
#define USE_UAC_MODE                0
#define USE_WHITE_LED               0

#define CLR_CAM_WIDTH               1280
#define CLR_CAM_HEIGHT              960
#define IR_CAM_WIDTH                1600
#define IR_CAM_HEIGHT               900
#define CAPTURE_WIDTH               (180)
#define CAPTURE_HEIGHT              (320)

#define UVC_MAX_WIDTH               1280
#define UVC_MAX_HEIGHT              720
#define UVC_MIN_WIDTH               320
#define UVC_MIN_HEIGHT              240
#define UVC_WIDTH                   640
#define UVC_HEIGHT                  480
#define CHECK_CLR_IR_SWITCH_THR     20
#define UVC_CLR_LUMINANCE           0x80
#define UVC_CLR_SAT_Gl              0x50
#define UVC_CLR_SAT_Cb              0x42
#define UVC_CLR_SAT_Cr              0x42
#define UVC_CLR_SHARP               0x85
#define UVC_CLR_AWB_EN              0
#define UVC_CLR_R_GAIN              0x50
#define UVC_CLR_G_GAIN              0x40
#define UVC_CLR_B_GAIN              0x54
#define UVC_UNIQ_COMPRESS           1
#define UVC_RES_720P                1       //1280x720
#define UVC_RES_480P                2       //640x480
#define UVC_RES_432x240             4
#define UVC_RES_480x320             8       //480x320
#define UVC_RES_240P                16      //320x240
#define UVC_RES_320x480             32      //320x480
#define UVC_RES_FLAG                UVC_RES_480P
#define UVC_RES_DEFINE              {1, 1280, 720, 30, 0},
#define UVC_RES_COUNT               1
#define UVC_MAX_FRAME_SIZE          0
#define UVC_PAUSE_LIMIT_TIME        0
#define UVC_MAX_FPS_TIME            40      //25fps
#define UVC_FIX_COMPRATE            0

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
#define MOUNT_RETRY_COUNT           15
#define DEVICE_NID_READY_VER        0 //'C'=0x43, for desman, else 0=NID_READY

#define N_MAX_FACE_FAILED_COUNT     5
#define N_MAX_PASS_FAILED_COUNT     5
#define N_MAX_FP_FAILED_COUNT       5
#define N_MAX_FAILED_TIME           (2.4)//second
#define N_MAX_DNN_FAILED_TIME       6//second
#define N_MAX_EYE_FAILED_TIME		5//second
#define N_SECURE_CHK_CNT            1000

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
#define DEFAULT_LIVENESS_MODE       0   //engine state for liveness, for special use only
#define DEFAULT_SECURE_VALUE        75  //caution!!! DO NOT MODIFY this value.
#define DEFAULT_SECURE_FALSE_VAL    5  //caution!!! DO NOT MODIFY this value.
#define DEFAULT_SECURE_STEP1        8
#define DEFAULT_SECURE_STEP2        24
#define DEFAULT_SECURE_MODE         0
#define DEFAULT_PROTO_ENC_MODE      0  //0: plain text mode, 1: default encryption mode, 2: XOR_1
#define DEFAULT_PROTO_EM_XOR1_KEY   PROTO_EM_XOR1_KEY_LANHENG
#define DEFAULT_PROTO_ENC_KEY_NO    0   //no default key
#define DEFAULT_PROTO_ENC_KEY_1     1   //0 1 2 3 4 5 6 7 8 9 a b c d e f
#define DEFAULT_PROTO_ENC_KEY_ORD   DEFAULT_PROTO_ENC_KEY_NO
#define DEFAULT_MI_AO_VOLUME        (0)   // -60~30
#define DEFAULT_MI_AI_VOLUME        20      // 0~21
#define DEFAULT_UART0_BAUDRATE      Baud_Rate_115200
#define DEFAULT_SNAPIMG_CTRL_CNT    3
#define FIRMWARE_MAGIC              "DBS"

#define DESMAN_ENC_MODE     2       //0 -> dessmman, 1 -> bom, 2 -> test

#define DB_TEST             0       //!!!!!!!Must be 0 for product!!!!!!!
#define FEAT_VER                    "F1.0"

#define CAM_RM_DEFAULT      0
#define CAM_RM_180DEGREE    1
#define CAM_ROTATION_MODE   CAM_RM_DEFAULT

#define IR_LED_ONOFF_MODE   1
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

//////////////////Camera Lens Setting////////////////////
#define ENGINE_LENS_40143       0 //default lens
#define ENGINE_LENS_M277_2409   1
#define ENGINE_LENS_TYPE    ENGINE_LENS_40143

///////////////////   Enroll Mode      //////////////////////////
#define ENROLL_FACE_HAND_SEPERATE   0
#define ENROLL_FACE_HAND_MIX        1
#define ENROLL_FACE_HAND_MODE       ENROLL_FACE_HAND_SEPERATE

#define HAND_VERIFY_PRIORITY_NORMAL 0
#define HAND_VERIFY_PRIORITY_HIGH   1
#define HAND_VERIFY_PRIORITY        HAND_VERIFY_PRIORITY_NORMAL

//#define USE_DEBUG
#define USE_LCD             0

#define my_debug printf

#ifdef USE_DEBUG

#define USE_DEBUG_TRACE

#ifdef USE_DEBUG_TRACE
#endif //USE_DEBUG_TRACE

#endif //USE_DEBUG
///////////////////////////////////////////////

#define MMAP_MODE

#define N_MAX_PERSON_NUM                100
#define N_MAX_HAND_NUM                  0

#define FN_FACE_BIN "/mnt/MISC/face.bin"

//////////////////////////////////////////////////////////////
/// products
//////////////////////////////////////////////////////////////

#define FRM_DBS20_DEFAULT                       0   //default
#define FRM_PT_DEFAULT_3_4                      100   //3.4 default
#define FRM_DBS3M_D20_DEF                       200   //D20, 3M default
#define FRM_DAS3M_PUXIN_UAC                     201

#define FRM_PRODUCT_TYPE                        FRM_DAS3M_PUXIN_UAC

//---------------------------------------------------------
#if (FRM_PRODUCT_TYPE == FRM_DBS20_DEFAULT)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "1.21.0.1_D2"
#define DEVICE_FIRMWARE_VERSION_INNER       "1.21.0.1_D2"

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
#undef DEFAULT_LIVENESS_MODE
#define DEFAULT_LIVENESS_MODE               0   //engine state for liveness, for special use only
#undef N_MAX_PERSON_NUM
#define N_MAX_PERSON_NUM                    100
#undef USE_IRSNR_SC2355
#define USE_IRSNR_SC2355                    1
#undef IR_CAM_WIDTH
#define IR_CAM_WIDTH                        1600
#undef IR_CAM_HEIGHT
#define IR_CAM_HEIGHT                       900
#undef ENGINE_LENS_TYPE
#define ENGINE_LENS_TYPE                    ENGINE_LENS_M277_2409
#undef FIRMWARE_MAGIC
#define FIRMWARE_MAGIC                      "DBS2.0"
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_CV180xB_DEMO_V1v0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_PT_DEFAULT_3_4)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-173"
#define DEVICE_FIRMWARE_VERSION             "5.2.0_RS"
#define DEVICE_FIRMWARE_VERSION_INNER       "5.2.0_RS"

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
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_D20_DEF)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "9.14.1.3_D2"
#define DEVICE_FIRMWARE_VERSION_INNER       "9.14.1.3_D2"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D20
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 640, 480, 30, 0},
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     3
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  2
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDASS_1V0J//BD_TY_CV181xC_DEMO_V1v0//
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_121
// #undef CAM_ROTATION_MODE
// #define CAM_ROTATION_MODE                   CAM_RM_180DEGREE
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DAS3M_PUXIN_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "9.43.0.1_D2"
#define DEVICE_FIRMWARE_VERSION_INNER       "9.43.0.1_D2"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D20
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              2
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 640, 480, 30, 0},
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     1
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  2
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDASS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_121
// #undef CAM_ROTATION_MODE
// #define CAM_ROTATION_MODE                   CAM_RM_180DEGREE
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef CHECK_CLR_IR_SWITCH_THR
#define CHECK_CLR_IR_SWITCH_THR             0
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX

//----------------------------------------------------------

#endif // FRM_PRODUCT_TYPE

#if (UVC_MAX_WIDTH < UVC_WIDTH && UVC_MAX_HEIGHT < UVC_HEIGHT)
#error "uvc width and height must be less or equal than uvc max."
#elif (UVC_MAX_WIDTH < UVC_HEIGHT && UVC_MAX_HEIGHT < UVC_WIDTH)
#error "uvc width and height must be less or equal than uvc max2."
#endif // uvc max

//#include "engine_inner_param.h"

#endif // APPDEF

