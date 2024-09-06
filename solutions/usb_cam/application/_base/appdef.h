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

//protocol mode
#define PROTO_MODE_NONE                 0
#define PROTO_MODE_SANJIANG             1
#define PROTO_MODE_QIXIN                2
#define PROTO_MODE_LANHENG              3

//board types
#define BD_TY_CV180xB_DEMO_V1v0     0
#define BD_TY_FSDB_1V0              1   //D10 2.0
#define BD_TY_CV181xC_DEMO_V1v0     2
#define BD_TY_FMDASS_1V0J           3   //D20 3M
#define BD_TY_FMDBSS_1V0J           4   //D10 3M
#define DEFAULT_BOARD_TYPE          BD_TY_FMDBSS_1V0J

//chip types
#define MY_CHIP_D10                 0
#define MY_CHIP_D20                 1
#define DEFAULT_CHIP_TYPE           MY_CHIP_D10

#define MY_SUBCHIP_D10              0
#define MY_SUBCHIP_D10A             1
#define DEFAULT_SUBCHIP_TYPE        MY_SUBCHIP_D10

//camera mipi types
#define CAM_MIPI_TY_121             0 //one sensor on one mipi
#define CAM_MIPI_TY_122             1 //two sensors on one mipi
#define DEFAULT_CAM_MIPI_TYPE       CAM_MIPI_TY_122

//macros for ENGINE_USE_TWO_CAM
#define EUTC_2D_MODE                0
#define EUTC_2V0_MODE               1
#define EUTC_3M_MODE                2
#define EUTC_3V4_MODE               3

//used for engine
#define ISP_Y_LEVEL_0               0   //normal, sc201cs color v2.1.0.11
#define ISP_Y_LEVEL_1               1   //bright, sc201cs color v2.1.0.13
#define ISP_Y_LEVEL_2               2   //dark, sc201cs color v2.1.0.26
#define ISP_Y_LEVEL_3               3   //dark, sc201cs color v2.1.0.50
#define ISP_Y_LEVEL                 ISP_Y_LEVEL_0

//isp bin file version
#define ISP_BIN_VER_103v3           0   //v1.0.3.3
#define ISP_BIN_VER_103v8           1   //v1.0.3.8
#define ISP_BIN_VER_104v1           2   //v1.0.4.1
#define ISP_BIN_VER_103v9           3   //v1.0.3.9
#define ISP_BIN_VER_21v0            2100
#define ISP_BIN_VER_21v1            2101
#define ISP_BIN_VER_21v2            2102
#define ISP_BIN_VER_21v4            2104
#define ISP_BIN_VER_21v7            2107
#define ISP_BIN_VER_21v8            2108
#define ISP_BIN_VER_21v9            2109
#define ISP_BIN_VER_21v10           2110
#define ISP_BIN_VER_21v11           2111
#define ISP_BIN_VER_21v12           2112
#define ISP_BIN_VER_21v13           2113
#define ISP_BIN_VER_21v15           2115
#define ISP_BIN_VER_21v16           2116
#define ISP_BIN_VER_21v17           2117
#define ISP_BIN_VER_21v18           2118
#define ISP_BIN_VER_21v19           2119
#define ISP_BIN_VER_21v20           2120
#define ISP_BIN_VER_21v21           2121
#define ISP_BIN_VER_21v22           2122
#define ISP_BIN_VER_21v23           2123
#define ISP_BIN_VER_21v24           2124
#define ISP_BIN_VER_21v25           2125
#define ISP_BIN_VER_21v26           2126
#define ISP_BIN_VER_21v30           2130
#define ISP_BIN_VER_21v31           2131
#define ISP_BIN_VER_21v32           2132
#define ISP_BIN_VER_21v33           2133
#define ISP_BIN_VER_21v34           2134
#define ISP_BIN_VER_21v35           2135
#define ISP_BIN_VER_21v36           2136
#define ISP_BIN_VER_21v37           2137
#define ISP_BIN_VER_21v38           2138
#define ISP_BIN_VER_21v40           2140
#define ISP_BIN_VER_21v43           2143
#define ISP_BIN_VER_21v44           2144
#define ISP_BIN_VER_21v45           2145
#define ISP_BIN_VER_21v46           2146
#define ISP_BIN_VER_21v47           2147
#define ISP_BIN_VER_21v48           2148
#define ISP_BIN_VER_21v49           2149
#define ISP_BIN_VER_21v50           2150
#define ISP_BIN_VER_21v52           2152
#define ISP_BIN_VER_21v54           2154
#define ISP_BIN_VER_21v55           2155
#define ISP_BIN_VER_21v56           2156
#define ISP_BIN_VER_21v57           2157
#define ISP_BIN_VER_21v58           2158
#define ISP_BIN_VER_21v59           2159
#define ISP_BIN_VER_21v60           2160
#define ISP_BIN_VER_211v0           21100 //v2.1.1.0
#define ISP_BIN_VER_22v0            20
#define ISP_BIN_VER_301v9           21
#define DEFAULT_ISP_BIN_VER         ISP_BIN_VER_21v0

//isp mono file version
#define ISP_BINM_VER_100v1          0   //v1.0.0.1
#define DEFAULT_ISP_BINM_VER        ISP_BINM_VER_100v1

//uvc direction
#define UVC_ROTATION_0              0
#define UVC_ROTATION_90             1
#define UVC_ROTATION_270            2       //180degree of UVC_ROTATION_90

//uvc dwFrameInterval
#define UVC_FI_FPS0                 30
#define UVC_FI_FPS1                 15
#define UVC_FI_FPS2                 10

//USE_3M_MODE
#define U3M_DISABLE                 0
#define U3M_DEFAULT                 1       //얼굴인식에 색카메라리용, 백색레드리용함
#define U3M_IR_ONLY                 2       //얼굴인식에 색카메라를 쓰지 않음
#define U3M_SEMI                    3       //밝을때는 얼굴인식에 색카메라를 리용, 어두운 환경에서는 적외선만 리용, 백색레드쓰지 않음
#define U3M_SEMI_IR                 4       //밝을때는 얼굴인식에 색카메라를 리용, 어두운 환경에서는 색카메라에서 얻은 적외선화상을 리용, 백색레드쓰지 않음(색카메라에 쌍통려파기를 리용함)

//USE_WHITE_LED               0   //0: , 1: , 2: 얼굴인식에서는 백색레드켜고 화상대화에서는 적외선화상을 현시하는 방식
#define UWL_DISABLE                 0       // 백색레드쓰지 않는 방식
#define UWL_EN_NORMAL               1       // 백색레드를 리용하는 방식
#define UWL_EN_F1U0                 2       // face on, uvc off, 얼굴인식에서는 백색레드켜고 화상대화에서는 적외선화상을 현시하는 방식
#define UWL_EN_F0U1                 3       // face off, uvc on, 얼굴인식에는 백색레드를 리용하지 않고 화상대화때에만 리용

//DEFAULT_FR_COLOR_MODE
#define FR_COLOR_MODE_DEF           1       //use whiteled to detect face
#define FR_COLOR_MODE_SEMI          2       //ignore detecting face in dark mode
#define FR_COLOR_MODE_IR            3       //use ir image to detect face in dark mode

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
#define USE_DEMOMODE4HAND           1
#define USE_WAEL_VDB                0       //10fps, jpg size must be less than 50KB
#define USE_SHENAO_VDB              0
#define USE_SHENAO_NEW_VDB          0
#define USE_WAEL_PROTO              0
#define NOTE_INTERVAL_MS            0
#define USE_VDBTASK                 1
#define USE_SMP_CORE1               1
#define CHECK_FIRMWARE              1
#define USE_WIFI_MODULE             0
#define USE_READY0_PROTO            0
#define USE_SANJIANG3_MODE          1       // 범해삼강(암호화있음), 중산칠심, 동관동흔조종기판을 함께 쓰는 방식
#define USE_AES_NOENC_MODE          0       // AES with no encryption
#define USE_NEW_SNAPIMAGE_MODE      1       //0:use Flash, 1:unuse Flash
#define USE_XISHANG_PROTO           0       //0x93 command
#define USE_AUTO_50_REPLY           0
#define USE_FP16_ENGINE             1
#define USE_NEW_RST_PROTO           1
#define USE_16M_FLASH               1
#define USE_RENT_ENGINE             0
#define USE_DB_UPDATE_MODE          1
#define USE_TONGXIN_PROTO           0
#define USE_EKESI_PROTO             0
#define USE_LAIJI_PROTO             0
#define ENROLL_ANGLE_MODE           0
#define ENGINE_USE_TWO_CAM          EUTC_3M_MODE
#define YAOYANG_MODE                0
#define SEND_LAST_MSG               0
#define USE_3M_MODE                 U3M_DISABLE
#define USE_UAC_MODE                1
#define USE_USB_EP_ERR_FIX_MODE     0
#define USE_WHITE_LED               0   //0: 백색레드쓰지 않는 방식, 1: 백색레드를 리용하는 방식, 2: 얼굴인식에서는 백색레드켜고 화상대화에서는 적외선화상을 현시하는 방식
#define USE_WATCHDOG                1
#define USE_SHENAO_HAND             0
#define USE_PRINT_TEMP              0
#define USE_USB_CHECKFIRM_MODE      0
#define USE_CAM_REINIT              1
#define USE_ISP_IR_3DNR             0
#define USE_UAC_DESC_ALT4           0
#define USE_USB_XN_PROTO            0   //use XinNeng USB Protocol
#define USE_UVC_FACE_RECT           0
#define USE_TEMP_MODE               1
#define USE_FUSHI_HAND_PROTO        0
#define USE_SNAPCLR_VENC            0
#define CONFIG_USB_HS               1
#define CONFIG_DWC2_VERSION         0
#define USE_EP0PKGSIZE_PATCH        0
#define CONFIG_SPI_NOR_ER_TIME      2000
#define CONFIG_USB_BULK_UVC         0

#define CLR_CAM_WIDTH               1600
#define CLR_CAM_HEIGHT              1200
#define IR_CAM_WIDTH                1600
#define IR_CAM_HEIGHT               900
#define CAPTURE_WIDTH               (180)
#define CAPTURE_HEIGHT              (320)
#define CAPTURE_MAX_WIDTH           (180)
#define CAPTURE_MAX_HEIGHT          (320)

#define UVC_MAX_WIDTH               1280
#define UVC_MAX_HEIGHT              720
#define UVC_MIN_WIDTH               320
#define UVC_MIN_HEIGHT              240
#define UVC_INIT_WIDTH              640
#define UVC_INIT_HEIGHT             480
#define CHECK_CLR_IR_SWITCH_THR     0
#define NEW_CLR_IR_SWITCH_THR       0x20
#define UVC_CLR2IR_THR4ISP          (-150) //threshold value for turning white led on.
#undef UVC_CLR2IR_THR4ISP
#define UVC_CLR2IR_THR4ISP_TH       (100) //threshold value for returning to color mode
#define UVC_CLR2IR_THR4ENGINE       (-100) //threshold value for turning white led on for face engine.
#undef UVC_CLR2IR_THR4ENGINE
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
#define UVC_MJPEG_BITRATE           20480
#define UVC_H26X_BITRATE            2048
#define UVC_CROP_RESIZE             1
#define ISP_FPS_FOR_UVC             0
#define WLED_PWM_DUTY               100     // %
#define UAC_AUDALGO_USE             1
#define UAC_SPK_NR_USE              0 //0: do not use, 1: manual, 2: use lib
#define UAC_SPEAKER_VOL             32 // 0 ~ 32
#define UAC_MIC_VOL                 12 // 0 ~ 24
#define UAC_SAMPLE_RATE             8000
#define SPECIFIC_LOG_PRINT          0
#define UAC_SPK_EP                  0x82
#define UAC_MIC_EP                  2
#define UVC_VBPOOL1CNT              5
#define UVC_LANDSCAPE               0
#define UVC_ENC_TYPE                0 // 0:mjpeg 1:h264 2:dual
#define UVC_H26X_GOP                20
#define UVC_H26X_MAXIQP             36
#define UVC_H26X_MAXQP              36
#define UVC_H26X_WIDTH              1280
#define UVC_H26X_HEIGHT             720
#define H26X_TYPE                   PT_H264
#define BIN_DATA_SIZE               174513
#define UVC_USBD_PRINT              0
#define UVC_DARK_WATCH_COUNTER      3
#define UAC_EP_WMAXPCKT_SIZE        64  //audio ep wMaxPacketSize
#define MAX_PSPT_SIZE               512 //MAX_PAYLOAD_SIZE_PER_TRANSACTION
#define USB_TPM_CNT                 1   //TRANSACTION_PER_MICROFRAME
#define UVC_IR_FRM_TIMEOUT          60
#define UVC_DELAY_BEFORE_START      0

#define UVC_PIXEL_FMT_NV21          0
#define UVC_PIXEL_FMT_YUV422        1

#define WLED_TEST_TIMEOUT           5 // s
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
#define DEFAULT_UVC_DIR             UVC_ROTATION_90       // 0: not rotate 90, 1: rotate 90
#define DEFAULT_UVC_COMP_PARAM_IMQ  100
#define DEFAULT_UVC_COMP_PARAM_BT_MAX   6
#define DEFAULT_UVC_COMP_PARAM_BT_DEF   4
#define DEFAULT_UVC_COMP_PARAM_RPFR     1
#define DEFAULT_UVC_PIXEL_FMT       UVC_PIXEL_FMT_NV21
#define DEFAULT_LIVENESS_MODE       0   //engine state for liveness, for special use only
#define DEFAULT_SECURE_VALUE        75  //caution!!! DO NOT MODIFY this value.
#define DEFAULT_SECURE_FALSE_VAL    5  //caution!!! DO NOT MODIFY this value.
#define DEFAULT_SECURE_STEP1        8
#define DEFAULT_SECURE_STEP2        24
#define DEFAULT_SECURE_MODE         0
#define DEFAULT_PROTO_ENC_MODE      PROTO_EM_ENCRYPT_XOR_LANHENG  //0: plain text mode, 1: default encryption mode, 2: XOR_1
#define DEFAULT_PROTO_EM_XOR1_KEY   PROTO_EM_XOR1_KEY_LANHENG
#define DEFAULT_PROTO_ENC_KEY_NO    0   //no default key
#define DEFAULT_PROTO_ENC_KEY_1     1   //0 1 2 3 4 5 6 7 8 9 a b c d e f
#define DEFAULT_PROTO_ENC_KEY_ORD   DEFAULT_PROTO_ENC_KEY_NO
#define DEFAULT_MI_AO_VOLUME        (0)   // -60~30
#define DEFAULT_MI_AI_VOLUME        20      // 0~21
#define DEFAULT_UART0_BAUDRATE      Baud_Rate_115200
#define DEFAULT_SNAPIMG_CTRL_CNT    3
#define DEFAULT_CLR_IR_FRAME_RATIO  2
#define DEFAULT_LIVENESS_LEVEL      S_LIVENESS_LEVEL_DEFAULT
#define DEFAULT_SNR4UVC             0    //0: use sensor0 for color, 1: use sensor1 for color, 2: no color
#define FIRMWARE_MAGIC              "DBS"
#define ENROLL_FACE_IMG_MAGIC       "EFIv1"
#define ENROLL_FACE_IMG_MAGIC2      "EFIv2"
#define ENROLL_FACE_IMG_MAGIC3      "EFIv3"
#define DEFAULT_FR_COLOR_MODE       FR_COLOR_MODE_DEF

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
#define ENGINE_LENS_TYPE        ENGINE_LENS_M277_2409

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
#define N_MAX_HAND_NUM                  100

#define FN_FACE_BIN "/mnt/MISC/face.bin"

//////////////////////////////////////////////////////////////
/// products
//////////////////////////////////////////////////////////////

#define FRM_DBS20_DEFAULT                       0   //default
#define FRM_PT_DEFAULT_3_4                      100   //3.4 default
#define FRM_DAS3M_D20_UAC                       200   //D20, 3M default
#define FRM_DAS3M_PUXIN_UAC                     201
#define FRM_DAS3M_HUANGLI_UAC                   202
#define FRM_DAS3M_HUANGLI_NEW_UAC               203
#define FRM_DAS3M_LS35_LH_UAC                   204
#define FRM_DBS3M_DEFAULT                       300   //D10, 3M default
#define FRM_DBS3M_PUXIN                         301
#define FRM_DBS3M_HONGLI_MODE                   302
#define FRM_DBS3M_HUANGLI_NEW_UAC               303
#define FRM_DBS3M_HUANGLI_UAC                   304
#define FRM_DBS3M_TONGXIN_PROTO                 305
#define FRM_DBS3M_LS35_LH_UAC                   306
#define FRM_DBS3M_AIPAI_TOYA_UAC                307
#define FRM_DBS3M_BAOJIAHEZHONG_MODE            308     //심전보가합중
#define FRM_DBS3M_KELINGPU_MODE                 309   //WLED->PWM
#define FRM_DBS3M_FANHAI_MODE                   310
#define FRM_DBS3M_AJISHI_CHANGSI_MODE           311
#define FRM_DBS3M_FANGKUAI_MODE                 312
#define FRM_DBS3M_PUXIN2                        313
#define FRM_DBS3M_DUAL_CAM_AIPAI                314
#define FRM_DBS3M_LANCENS_UAC                   315
#define FRM_DBS3M_XIONGMAI_UAC                  316
#define FRM_DBS3M_BOLATAIN_MODE                 317
#define FRM_DBS3M_LIWEN_UAC                     318
#define FRM_DBS3M_TOYO_UAC                      319
#define FRM_DBS3M_OKEDA_UAC                     320
#define FRM_DBS3M_XIJIN_UAC                     321
#define FRM_DBS3M_KEXIONG_UAC                   322
#define FRM_DBS3M_HAND_PRIO_UAC                 323
#define FRM_DBS3M_7916_UAC                      324
#define FRM_DBS3M_YIHE_UAC                      325
#define FRM_DBS3M_JIASHIBANG_UAC                326
#define FRM_DBS3M_BINRUI10IN_UAC                327
#define FRM_DBS3M_XINNENG_H264                  328
#define FRM_DBS3M_EKESI                         329
#define FRM_DBS3M_XINNENG_UAC                   330
#define FRM_DBS3M_IR_DEFAULT                    331
#define FRM_DBS3M_JINJIAN_UAC                   332
#define FRM_DBS3M_TX_PROTO_IR                   333
#define FRM_DBS3M_TX2_UAC                       334
#define FRM_DBS3M_OKEDA2_UAC                    335
#define FRM_DBS3M_TONGXIN_UVC                   336
#define FRM_DBS3M_AIPAI2_UAC                    337
#define FRM_DBS3M_RENT_UAC                      338
#define FRM_DBS3M_LAIJI_UAC                     339
#define FRM_JIZHI_UAC                           340 //동관극지
#define FRM_DBS3M_TOYA_IR_UAC                   341
#define FRM_DBS3M_SAINAO                        342
#define FRM_DBS3M_SAINAO_TUYA                   343
#define FRM_DBS3M_LIWEN_IR                      344
#define FRM_DBS3M_DUP_DISABLE                   345
#define FRM_DBS3M_KEYU_UAC                      346
#define FRM_DBS3MH_DEFAULT                      347
#define FRM_DBS3M_XM7258_UAC                    348
#define FRM_DBS3M_FANHAI_IR_MODE                349
#define FRM_DBS3M_LS7258_UAC                    350
#define FRM_DBS3M_YIHONG_UAC                    351
#define FRM_DBS3M_OKD_IR_UAC                    352
#define FRM_DBS3M_XINAN                         353
#define FRM_DBS3M_HW7258_TUYA_UAC               354
#define FRM_DBS3M_SUOFEIWAN_UAC                 355
#define FRM_DBS3M_SH_UAC                        356
#define FRM_DBS3M_FUSHI_XM_UAC                  357
#define FRM_DBS3M_LS7258_IR_UAC                 358
#define FRM_DBS3M_YIHE2_UAC                     359
#define FRM_DBS3M_IR_JIGAO                      360
#define FRM_DBS3M_BK7258_UAC                    361
#define FRM_DBS3M_XINGUOXIN_XIONGMAI_UAC        362
#define FRM_DBS3M_BK7256_UAC                    363
#define FRM_DBS3M_AP7258_UAC                    364
#define FRM_DBS3M_JIARUI_UAC                    365
#define FRM_DBS3M_CS_TUYA_UAC                   366
#define FRM_DBS3M_RENT                          367
#define FRM_DBS3M_QINGSONG                      368
#define FRM_DBS3M_JUNLI_UAC                     369
#define FRM_DBS3M_D10A_UAC                      400

#define FRM_PRODUCT_TYPE                        FRM_DBS3M_FANHAI_IR_MODE

//----------------------------------------------------------
#if (FRM_PRODUCT_TYPE == FRM_DBS3M_YIHE_UAC)

// 3.1.5_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.62.12.5_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.62.12.5_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v57
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      2
#undef UVC_CROP_RESIZE
#define UVC_CROP_RESIZE                     0.8
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32 // 0 ~ 32

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_TONGXIN_UVC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.2.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.2.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 640, 480, 30, 0, 8192}, \
                                            {3, 320, 240, 30, 0, 4096}, \
                                            {4, 480, 320, 30, 0, 4096}, \
                                            {5, 320, 480, 30, UVC_ROTATION_0 + 1, 4096},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        0
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1
#undef USE_ISP_IR_3DNR
#define USE_ISP_IR_3DNR                     0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_PUXIN2)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.3.5_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.3.5_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
// #undef ENROLL_FACE_HAND_MODE
// #define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_JIASHIBANG_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.4.3_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.4.3_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef UAC_AUDALGO_USE 
#define UAC_AUDALGO_USE                     0
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef USE_ISP_IR_3DNR
#define USE_ISP_IR_3DNR                     0
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_KEXIONG_UAC)

// 3.5.9_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.69.10_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.69.10_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 864, 480, 30, 0, 8192}, \
                                            {2, 800, 480, 30, 0, 8192}, \
                                            {3, 640, 480, 30, 0, 6144}, \
                                            {4, 480, 320, 30, 0, 4096},
#undef UAC_SAMPLE_RATE
#define UAC_SAMPLE_RATE                     16000
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef ENROLL_ANGLE_MODE
#define ENROLL_ANGLE_MODE                   1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef MAX_PSPT_SIZE
#define MAX_PSPT_SIZE                       1024 //MAX_PAYLOAD_SIZE_PER_TRANSACTION
// #undef UAC_EP_WMAXPCKT_SIZE
// #define UAC_EP_WMAXPCKT_SIZE                32  //audio ep wMaxPacketSize
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 2
// #undef USE_EP0PKGSIZE_PATCH
// #define USE_EP0PKGSIZE_PATCH                1
// #undef CONFIG_USB_BULK_UVC
// #define CONFIG_USB_BULK_UVC                 1
#undef UVC_MAX_WIDTH
#define UVC_MAX_WIDTH                       864
#undef UVC_MAX_HEIGHT
#define UVC_MAX_HEIGHT                      480
#undef USB_TPM_CNT
#define USB_TPM_CNT                         3   //TRANSACTION_PER_MICROFRAME

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_XINNENG_H264)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.6.3.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.6.3.2_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v10
#undef UVC_ENC_TYPE
#define UVC_ENC_TYPE                        2 //dual stream
#undef H26X_TYPE 
#define H26X_TYPE                           PT_H264
#undef UVC_MJPEG_BITRATE
#define UVC_MJPEG_BITRATE                   10240
#undef UVC_H26X_BITRATE
#define UVC_H26X_BITRATE                    120
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef USE_UAC_DESC_ALT4
#define USE_UAC_DESC_ALT4                   1
#undef USE_USB_XN_PROTO
#define USE_USB_XN_PROTO                    1
// #undef UAC_SPK_EP
// #define UAC_SPK_EP                          0x83
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef UVC_H26X_GOP
#define UVC_H26X_GOP                        50
#undef UVC_INIT_WIDTH
#define UVC_INIT_WIDTH                      1280
#undef UVC_INIT_HEIGHT
#define UVC_INIT_HEIGHT                     720

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_AJISHI_CHANGSI_MODE)

//3.7.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.70.5_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.70.5_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32 // 0 ~ 32
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v52
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------

#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_HUANGLI_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.8.1_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.8.1_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {1, 480, 320, 30, 0},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
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
#undef USE_SHENAO_HAND
#define USE_SHENAO_HAND                     1
#undef USE_USB_CHECKFIRM_MODE
#define USE_USB_CHECKFIRM_MODE              0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_LANCENS_UAC)

// 3.9.2_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.61.3.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.61.3.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v60
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_HAND_PRIO_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.10.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.10.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 9000},\
                                            {3, 800, 480, 30, 0, 8192},\
                                            {4, 640, 480, 30, 0, 4096},\
                                            {5, 480, 320, 30, 0, 4096},\
                                            {6, 320, 240, 30, 0, 4096}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef HAND_VERIFY_PRIORITY
#define HAND_VERIFY_PRIORITY                HAND_VERIFY_PRIORITY_HIGH

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_AIPAI_TOYA_UAC)

// 3.11.4_D + 3.1.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.76.7_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.76.7_D"

#undef UVC_RES_DEFINE
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 2
#undef MAX_PSPT_SIZE
#define MAX_PSPT_SIZE                       1024

// #undef DEFAULT_UVC_DIR
// #define DEFAULT_UVC_DIR                     UVC_ROTATION_270
#undef CAM_ROTATION_MODE
#define CAM_ROTATION_MODE                   CAM_RM_180DEGREE

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_XINNENG_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.12.1.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.12.1.2_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v23
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef USE_CAM_REINIT
#define USE_CAM_REINIT                      1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_KELINGPU_MODE)

// 3.13.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.82.4_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.82.4_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32 // 0 ~ 32
#undef WLED_PWM_DUTY
#define WLED_PWM_DUTY                       15
// #undef N_MAX_HAND_NUM
// #define N_MAX_HAND_NUM                      0
#define ENGINE_FOR_ABROAD

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_DEFAULT)

// 3.14.14_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.60.3_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.60.3_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------

#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_BAOJIAHEZHONG_MODE)

// 3.15.3_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.94.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.94.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_BINRUI10IN_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.16.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.16.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_0
#undef UVC_LANDSCAPE
#define UVC_LANDSCAPE                       1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_FANHAI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.71.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.71.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
// #undef CONFIG_DWC2_VERSION
// #define CONFIG_DWC2_VERSION                 1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      0

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI_IR
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_211v0

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_FANHAI_IR_MODE)

// 3.18.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.74.3_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.74.3_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v60
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_270

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_JIGAO_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.19.1_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.19.1_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 960, 30, 0}, \
                                            {2, 1280, 720, 30, 0, 10240}, \
                                            {3, 864, 480, 30, 0, 10240}, \
                                            {4, 800, 480, 30, 0, 10240}, \
                                            {5, 640, 480, 30, 0, 6144}, \
                                            {6, 480, 320, 30, 0, 4096}, \
                                            {7, 320, 240, 30, 0, 4096},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_IR_ONLY
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_301v9
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#undef UVC_MAX_WIDTH
#define UVC_MAX_WIDTH                       1280
#undef UVC_MAX_HEIGHT
#define UVC_MAX_HEIGHT                      960

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_EKESI)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.20.3_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.20.3_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef USE_EKESI_PROTO
#define USE_EKESI_PROTO                     1
// #undef ENROLL_FACE_HAND_MODE
// #define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_FANGKUAI_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.21.7_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.21.7_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_0       // 0: not rotate 90, 1: rotate 90
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 320, 480, 10, 0},
#undef UVC_MAX_FPS_TIME
#define UVC_MAX_FPS_TIME                    60
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16 // 0 ~ 32
// #undef UAC_SAMPLE_RATE
// #define UAC_SAMPLE_RATE                  16000
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef ISP_FPS_FOR_UVC
#define ISP_FPS_FOR_UVC                     10
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef UVC_MJPEG_BITRATE
#define UVC_MJPEG_BITRATE                   4096
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_22v11
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_XIJIN_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.22.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.22.2_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 480, 640, 15, 0, 4096},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_0       // 0: not rotate 90, 1: rotate 90
// #undef UVC_MAX_WIDTH
// #define UVC_MAX_WIDTH                       640
// #undef UVC_MAX_HEIGHT
// #define UVC_MAX_HEIGHT                      480
// #undef UVC_WIDTH
// #define UVC_WIDTH                           640
// #undef UVC_HEIGHT
// #define UVC_HEIGHT                          480
#undef UVC_VBPOOL1CNT
#define UVC_VBPOOL1CNT                      3
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#undef USE_ISP_IR_3DNR
#define USE_ISP_IR_3DNR                     0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_7916_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.23.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.23.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 480, 320, 30, 0, 8192},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_22v0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_HONGLI_MODE)

//3.24.3_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.65.4_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.65.4_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240}, \
                                            {2, 640, 480, 30, 0, 8192}, \
                                            {3, 480, 320, 30, 0, 4096}, \
                                            {4, 320, 240, 30, 0, 2048},
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef DEFAULT_UART0_BAUDRATE
#define DEFAULT_UART0_BAUDRATE              Baud_Rate_9600
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_OKEDA2_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.25.0.8_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.25.0.8_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef USE_ISP_IR_3DNR
#define USE_ISP_IR_3DNR                     0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_SAINAO)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.26.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.26.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_AES_XLAN
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16 // 0 ~ 32
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_TONGXIN_PROTO)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.27.3_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.27.3_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef UAC_AUDALGO_USE 
#define UAC_AUDALGO_USE                     1
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16 // 0 ~ 32
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1
#define UVC_CLR2IR_THR4ISP                  (-200)
#define UVC_CLR2IR_THR4ENGINE               (-100)

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_IR_DEFAULT)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.28.4_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.28.4_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef USE_ISP_IR_3DNR
#define USE_ISP_IR_3DNR                     0
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef USE_TEMP_MODE
#define USE_TEMP_MODE                       1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_AIPAI2_UAC)

//3.29.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.72.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.72.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0},
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     4 // 0 ~ 32

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_JINJIAN_UAC)

// 3.30.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.87.6_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.87.6_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v38
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_270
#undef CAM_ROTATION_MODE
#define CAM_ROTATION_MODE                   CAM_RM_180DEGREE

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_SAINAO_TUYA)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.31.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.31.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_AES_XLAN
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 640, 480, 30, 0, 8192}, \
                                            {2, 640, 360, 30, 0, 2048},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_LS35_LH_UAC)

// 3.32.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.90.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.90.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240}, \
                                            {2, 640, 480, 30, 0, 4096},
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_270
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#define UVC_CLR2IR_THR4ISP                  (-200)
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_TX_PROTO_IR)

// 3.33.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.100.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.100.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_LIWEN_IR)

// 3.35.0_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.64.6_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.64.6_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v24
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32 // 0 ~ 32
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      2
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UAC_SAMPLE_RATE
#define UAC_SAMPLE_RATE                     16000

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_TOYO_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.36.1_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.36.1_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240}, \
                                            {2, 640, 480, 30, 0, 5120}, \
                                            {3, 320, 240, 30, 0, 3072}, \
                                            {4, 480, 320, 30, 0, 5120},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef USE_USB_CHECKFIRM_MODE
#define USE_USB_CHECKFIRM_MODE              0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_TOYA_IR_UAC)

// 3.37.4_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.96.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.96.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 640, 480, 30, 0, 8192}, \
                                            {2, 640, 360, 30, 0, 2048},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v13
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#define ENGINE_FOR_ABROAD

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_TX2_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.38.1.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.38.1.2_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 640, 480, 30, 0, 6144},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v13
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     4 // 0 ~ 32
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_KEYU_UAC)

// 3.40.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.67.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.67.2_D"

#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  0
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_AES_XLAN
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 864, 480, 30, 0, 8192}, \
                                            {2, 800, 480, 30, 0, 8192}, \
                                            {3, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UVC_MAX_WIDTH
#define UVC_MAX_WIDTH                       864
#undef UVC_MAX_HEIGHT
#define UVC_MAX_HEIGHT                      480

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_LS7258_UAC)

// 3.41.0
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.75.8.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.75.8.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 864, 480, 30, 0, 8192}, \
                                            {2, 800, 480, 30, 0, 8192}, \
                                            {3, 640, 480, 30, 0, 4096},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef CONFIG_USB_HS
#define CONFIG_USB_HS                       0
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef DEFAULT_UVC_PIXEL_FMT
#define DEFAULT_UVC_PIXEL_FMT               UVC_PIXEL_FMT_YUV422
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      1
#undef USE_SNAPCLR_VENC
#define USE_SNAPCLR_VENC                    1
#undef CAPTURE_WIDTH
#define CAPTURE_WIDTH                       (640)
#undef CAPTURE_HEIGHT
#define CAPTURE_HEIGHT                      (360)
#undef CAPTURE_MAX_WIDTH
#define CAPTURE_MAX_WIDTH                   (640)
#undef CAPTURE_MAX_HEIGHT
#define CAPTURE_MAX_HEIGHT                  (360)
#undef UVC_MJPEG_BITRATE
#define UVC_MJPEG_BITRATE                   4096
#undef UVC_MAX_WIDTH
#define UVC_MAX_WIDTH                       864
#undef UVC_MAX_HEIGHT
#define UVC_MAX_HEIGHT                      480
#undef UVC_DELAY_BEFORE_START
#define UVC_DELAY_BEFORE_START              700
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_BOLATAIN_MODE)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.42.1_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.42.1_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_RENT_UAC)

// 3.48.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.89.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.89.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef USE_RENT_ENGINE
#define USE_RENT_ENGINE                     1
#undef USE_DB_UPDATE_MODE
#define USE_DB_UPDATE_MODE                  1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_PUXIN)

//3.43.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.84.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.84.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
// #undef ENROLL_FACE_HAND_MODE
// #define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v34
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef USE_TEMP_MODE
#define USE_TEMP_MODE                       1
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#define ENGINE_FOR_ABROAD

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_XIONGMAI_UAC)

//3.44.9_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.63.6_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.63.6_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1
// #undef UAC_SPEAKER_VOL
// #define UAC_SPEAKER_VOL                     16
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
// #undef CONFIG_DWC2_VERSION
// #define CONFIG_DWC2_VERSION                 1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_HUANGLI_NEW_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.45.3.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.45.3.2_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 480, 320, 30, 0, 8192},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef USE_SHENAO_HAND
#define USE_SHENAO_HAND                     1
#undef USE_USB_CHECKFIRM_MODE
#define USE_USB_CHECKFIRM_MODE              0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_22v0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_LIWEN_UAC)

// 3.46.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.92.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.92.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     8
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_INIT_WIDTH
#define UVC_INIT_WIDTH                      1280
#undef UVC_INIT_HEIGHT
#define UVC_INIT_HEIGHT                     720
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef ENROLL_ANGLE_MODE
#define ENROLL_ANGLE_MODE                   1
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_XINAN)

// 3.47.0_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.80.4_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.80.4_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v50
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32 // 0 ~ 32
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_DUAL_CAM_AIPAI)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.49.10_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.49.10_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
// #undef N_MAX_HAND_NUM
// #define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     4 // 0 ~ 32
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_OKEDA_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.50.9_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.50.9_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_SAMPLE_RATE
#define UAC_SAMPLE_RATE                     16000
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     8
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.

// 중산금강랑대상일때 설정해야 하는 항목
// #undef UVC_CROP_RESIZE
// #define UVC_CROP_RESIZE                     0.88

// 동관비부대상일때 설정해야 하는 항목
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef UVC_INIT_WIDTH
#define UVC_INIT_WIDTH                      1280
#undef UVC_INIT_HEIGHT
#define UVC_INIT_HEIGHT                     720
#undef ENROLL_ANGLE_MODE
#define ENROLL_ANGLE_MODE                   1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_IR_JIGAO)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.91.1_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.91.1_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 8192}, \
                                            {3, 800, 480, 30, 0, 8192}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef CONFIG_USB_HS
#define CONFIG_USB_HS                       0
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32 // 0 ~ 32
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      2

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_LAIJI_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.52.0.8_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.52.0.8_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1064, 600, 30, 0, 20480}, \
                                            {2, 1280, 720, 30, 0, 20480}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 8192}, \
                                            {5, 480, 320, 30, 0, 6144}, \
                                            {6, 320, 240, 30, 0, 6144},
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v21
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
// #define UVC_CLR2IR_THR4ISP                  (-50)
#undef USE_ISP_IR_3DNR
#define USE_ISP_IR_3DNR                     0
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef USE_RENT_ENGINE
#define USE_RENT_ENGINE                     1
#undef USE_DB_UPDATE_MODE
#define USE_DB_UPDATE_MODE                  1
#undef USE_USB_CHECKFIRM_MODE
#define USE_USB_CHECKFIRM_MODE              0
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef USE_LAIJI_PROTO
#define USE_LAIJI_PROTO                     1
#undef NEW_CLR_IR_SWITCH_THR
#define NEW_CLR_IR_SWITCH_THR               0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_JIZHI_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.54.0.1_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.54.0.1_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 640, 480, 30, 0, 10240}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef UVC_MAX_WIDTH
#define UVC_MAX_WIDTH                       640
#undef UVC_MAX_HEIGHT
#define UVC_MAX_HEIGHT                      480
#undef UVC_WIDTH
#define UVC_WIDTH                           640
#undef UVC_HEIGHT
#define UVC_HEIGHT                          480
#undef USE_UVC_FACE_RECT
#define USE_UVC_FACE_RECT                   1
#undef USE_RENT_ENGINE
#define USE_RENT_ENGINE                     1

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_DUP_DISABLE)

// 3.14.14_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.66.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.66.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef ENROLL_DUPLICATION_CHECK
#define ENROLL_DUPLICATION_CHECK            EDC_DISABLE
#undef ENROLL_HAND_DUP_CHECK
#define ENROLL_HAND_DUP_CHECK               0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3MH_DEFAULT)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.68.3_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.68.3_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_IR_ONLY
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_270

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_XM7258_UAC)

// 3.55.0
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.73.6_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.73.6_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 8192}, \
                                            {3, 800, 480, 30, 0, 8192}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
// #undef CONFIG_USB_HS
// #define CONFIG_USB_HS                       0
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 2
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      1
// #undef MAX_PSPT_SIZE
// #define MAX_PSPT_SIZE                       1024
#undef CONFIG_USB_BULK_UVC
#define CONFIG_USB_BULK_UVC                 1
#undef UVC_DELAY_BEFORE_START
#define UVC_DELAY_BEFORE_START              700

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_YIHE2_UAC)

// 3.1.6_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.77.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.77.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 10240}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_270
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_YIHONG_UAC)

// 3.62.1_D + USE_TONGXIN_PROTO
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.78.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.78.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef USE_TONGXIN_PROTO
#define USE_TONGXIN_PROTO                   1

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_OKD_IR_UAC)

// 3.5.9_D, use_whiteled=0
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.79.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.79.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 800, 480, 30, 0},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef UAC_SAMPLE_RATE
#define UAC_SAMPLE_RATE                     16000
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     8
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-50) //threshold value for turning white led on.
#undef UVC_INIT_WIDTH
#define UVC_INIT_WIDTH                      800
#undef UVC_INIT_HEIGHT
#define UVC_INIT_HEIGHT                     480
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef MAX_PSPT_SIZE
#define MAX_PSPT_SIZE                       1024 //MAX_PAYLOAD_SIZE_PER_TRANSACTION
#undef UAC_EP_WMAXPCKT_SIZE
#define UAC_EP_WMAXPCKT_SIZE                32  //audio ep wMaxPacketSize
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_HW7258_TUYA_UAC)

// 3.75.4.2_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.81.4_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.81.4_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 20480}, \
                                            {2, 864, 480, 30, 0, 8192}, \
                                            {3, 800, 480, 30, 0, 8192}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
// #undef SPECIFIC_LOG_PRINT
// #define SPECIFIC_LOG_PRINT                  1
// #undef UVC_USBD_PRINT
// #define UVC_USBD_PRINT                      1
#undef CONFIG_USB_HS
#define CONFIG_USB_HS                       0
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef DEFAULT_UVC_PIXEL_FMT
#define DEFAULT_UVC_PIXEL_FMT               UVC_PIXEL_FMT_YUV422
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      2

#undef USE_SNAPCLR_VENC
#define USE_SNAPCLR_VENC                    1
#undef CAPTURE_WIDTH
#define CAPTURE_WIDTH                       (640)
#undef CAPTURE_HEIGHT
#define CAPTURE_HEIGHT                      (360)
#undef CAPTURE_MAX_WIDTH
#define CAPTURE_MAX_WIDTH                   (640)
#undef CAPTURE_MAX_HEIGHT
#define CAPTURE_MAX_HEIGHT                  (360)
#undef UVC_MJPEG_BITRATE
#define UVC_MJPEG_BITRATE                   4096

#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_SUOFEIWAN_UAC)

// 3.61.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.83.4_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.83.4_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef USE_FUSHI_HAND_PROTO
#define USE_FUSHI_HAND_PROTO                1
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEVICE_NID_READY_VER
#define DEVICE_NID_READY_VER                0xf0

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_SH_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.85.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.85.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v12
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#define ENGINE_FOR_ABROAD

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_FUSHI_XM_UAC)

/*
even version: use_whiteled = 0
odd version: use_whiteled = 1
*/

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.86.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.86.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16 // 0 ~ 32
#undef USE_FUSHI_HAND_PROTO
#define USE_FUSHI_HAND_PROTO                1
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEVICE_NID_READY_VER
#define DEVICE_NID_READY_VER                0xf0
#undef CONFIG_SPI_NOR_ER_TIME
#define CONFIG_SPI_NOR_ER_TIME              2000

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_LS7258_IR_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.88.6_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.88.6_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 8192}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef CONFIG_USB_HS
#define CONFIG_USB_HS                       0
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      1
#undef UVC_DELAY_BEFORE_START
#define UVC_DELAY_BEFORE_START              700

#undef USE_FUSHI_HAND_PROTO
#define USE_FUSHI_HAND_PROTO                1
#undef USE_READY0_PROTO
#define USE_READY0_PROTO                    1
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEVICE_NID_READY_VER
#define DEVICE_NID_READY_VER                0xf0

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_BK7258_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.93.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.93.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 480, 864, 30, UVC_ROTATION_0 + 1, 10240},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef CONFIG_USB_HS
#define CONFIG_USB_HS                       0
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef UAC_EP_WMAXPCKT_SIZE
#define UAC_EP_WMAXPCKT_SIZE                16  //audio ep wMaxPacketSize

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_XINGUOXIN_XIONGMAI_UAC)

//3.63.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.95.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.95.0_D"

#undef DEFAULT_CHIP_TYPE
#define DEFAULT_CHIP_TYPE                   MY_CHIP_D10
#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_XOR_LANHENG
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 15, 0, 20480}, \
                                            {2, 864, 480, 12, 0, 9000},\
                                            {3, 800, 480, 12, 0, 9000},\
                                            {4, 480, 320, 12, 0, 9000},\
                                            {5, 320, 240, 12, 0, 9000}
#undef ENGINE_USE_TWO_CAM
#define ENGINE_USE_TWO_CAM                  EUTC_3M_MODE
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_UAC_MODE
#define USE_UAC_MODE                        1
#undef UAC_AUDALGO_USE
#define UAC_AUDALGO_USE                     1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     16
#undef DEFAULT_BOARD_TYPE
#define DEFAULT_BOARD_TYPE                  BD_TY_FMDBSS_1V0J
#undef DEFAULT_CAM_MIPI_TYPE
#define DEFAULT_CAM_MIPI_TYPE               CAM_MIPI_TY_122
#undef USE_VDBTASK
#define USE_VDBTASK                         1
#undef USE_SANJIANG3_MODE
#define USE_SANJIANG3_MODE                  1
#undef N_MAX_HAND_NUM
#define N_MAX_HAND_NUM                      100
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UAC_SPK_EP
#define UAC_SPK_EP                          0x83
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef UAC_MIC_VOL
#define UAC_MIC_VOL                         8

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_BK7256_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.97.0.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.97.0.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 8192}, \
                                            {3, 800, 480, 30, 0, 8192}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef DEFAULT_UVC_PIXEL_FMT
#define DEFAULT_UVC_PIXEL_FMT               UVC_PIXEL_FMT_YUV422

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_AP7258_UAC)

// 3.73.1
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.98.1_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.98.1_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0, 0},\
                                            {2, 864, 480, 30, 0, 10240},\
                                            {3, 800, 480, 30, 0, 9000},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v50
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef CONFIG_USB_HS
#define CONFIG_USB_HS                       0
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_JIARUI_UAC)

// 3.62.1_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.99.0.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.99.0.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 8192}, \
                                            {3, 800, 480, 30, 0, 8192}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v11
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     6 // 0 ~ 32

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_D10A_UAC)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.150.0.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.150.0.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 8192}, \
                                            {3, 800, 480, 30, 0, 8192}, \
                                            {4, 640, 480, 30, 0, 4096},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef DEFAULT_SUBCHIP_TYPE
#define DEFAULT_SUBCHIP_TYPE                MY_SUBCHIP_D10A

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_CS_TUYA_UAC)

// 3.1.5_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.101.0.2_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.101.0.2_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 480, 864, 30, UVC_ROTATION_0 + 1, 8192}, \
                                            {2, 480, 800, 30, UVC_ROTATION_0 + 1, 8192}, \
                                            {3, 480, 854, 30, UVC_ROTATION_0 + 1, 8192}, \
                                            {4, 480, 640, 30, UVC_ROTATION_0 + 1, 6144}, \
                                            {5, 480, 320, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 1
#undef UAC_EP_WMAXPCKT_SIZE
#define UAC_EP_WMAXPCKT_SIZE                16  //audio ep wMaxPacketSize

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_RENT)

// 3.89.0_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.102.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.102.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 480, 640, 30, 0, 4096},
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#undef USE_WHITE_LED
#define USE_WHITE_LED                       1
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef USE_RENT_ENGINE
#define USE_RENT_ENGINE                     1
#undef USE_DB_UPDATE_MODE
#define USE_DB_UPDATE_MODE                  1
#undef USE_UAC_MODE
#define USE_UAC_MODE                        0
#undef USE_SNAPCLR_VENC
#define USE_SNAPCLR_VENC                    1
#undef CAPTURE_WIDTH
#define CAPTURE_WIDTH                       (640)
#undef CAPTURE_HEIGHT
#define CAPTURE_HEIGHT                      (480)
#undef CAPTURE_MAX_WIDTH
#define CAPTURE_MAX_WIDTH                   (640)
#undef CAPTURE_MAX_HEIGHT
#define CAPTURE_MAX_HEIGHT                  (480)
#undef UVC_MAX_WIDTH
#define UVC_MAX_WIDTH                       640
#undef UVC_MAX_HEIGHT
#define UVC_MAX_HEIGHT                      480
#undef UVC_MJPEG_BITRATE
#define UVC_MJPEG_BITRATE                   2048
#undef DEFAULT_UVC_DIR
#define DEFAULT_UVC_DIR                     UVC_ROTATION_0

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_QINGSONG)

#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.105.0_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.105.0_D"

#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 1280, 720, 30, 0}, \
                                            {2, 864, 480, 30, 0, 10240}, \
                                            {3, 800, 480, 30, 0, 10240}, \
                                            {4, 640, 480, 30, 0, 6144},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v57
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      2
#undef UVC_CROP_RESIZE
#define UVC_CROP_RESIZE                     0.8
#undef UAC_SPEAKER_VOL
#define UAC_SPEAKER_VOL                     32 // 0 ~ 32

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------
#elif (FRM_PRODUCT_TYPE == FRM_DBS3M_JUNLI_UAC)

// 3.75.8_D
#define DEVICE_MODEL_NUM                    "BIOAT-FM-175"
#define DEVICE_FIRMWARE_VERSION             "3.106.0.3_D"
#define DEVICE_FIRMWARE_VERSION_INNER       "3.106.0.3_D"

#undef DEFAULT_PROTO_ENC_MODE
#define DEFAULT_PROTO_ENC_MODE              PROTO_EM_ENCRYPT_AES_XLAN
#undef UVC_RES_DEFINE
#define UVC_RES_DEFINE                      {1, 864, 480, 30, 0, 8192}, \
                                            {2, 800, 480, 30, 0, 8192}, \
                                            {3, 640, 480, 30, 0, 4096},
#undef USE_WHITE_LED
#define USE_WHITE_LED                       0
#undef DEFAULT_ISP_BIN_VER
#define DEFAULT_ISP_BIN_VER                 ISP_BIN_VER_21v49
#undef USE_USB_EP_ERR_FIX_MODE
#define USE_USB_EP_ERR_FIX_MODE             1
#undef SPECIFIC_LOG_PRINT
#define SPECIFIC_LOG_PRINT                  1
#undef UVC_USBD_PRINT
#define UVC_USBD_PRINT                      1
#undef CONFIG_DWC2_VERSION
#define CONFIG_DWC2_VERSION                 2
#undef MAX_PSPT_SIZE
#define MAX_PSPT_SIZE                       1024 //MAX_PAYLOAD_SIZE_PER_TRANSACTION
#undef USB_TPM_CNT
#define USB_TPM_CNT                         3   //TRANSACTION_PER_MICROFRAME
#undef DEFAULT_UVC_PIXEL_FMT
#define DEFAULT_UVC_PIXEL_FMT               UVC_PIXEL_FMT_YUV422
#undef UAC_SPK_NR_USE
#define UAC_SPK_NR_USE                      1
#undef USE_SNAPCLR_VENC
#define USE_SNAPCLR_VENC                    1
#undef CAPTURE_WIDTH
#define CAPTURE_WIDTH                       (640)
#undef CAPTURE_HEIGHT
#define CAPTURE_HEIGHT                      (360)
#undef CAPTURE_MAX_WIDTH
#define CAPTURE_MAX_WIDTH                   (640)
#undef CAPTURE_MAX_HEIGHT
#define CAPTURE_MAX_HEIGHT                  (360)
#undef UVC_MJPEG_BITRATE
#define UVC_MJPEG_BITRATE                   4096
#undef UVC_MAX_WIDTH
#define UVC_MAX_WIDTH                       864
#undef UVC_MAX_HEIGHT
#define UVC_MAX_HEIGHT                      480
#undef UVC_DELAY_BEFORE_START
#define UVC_DELAY_BEFORE_START              700
#undef ENROLL_FACE_HAND_MODE
#define ENROLL_FACE_HAND_MODE               ENROLL_FACE_HAND_MIX
#undef UAC_SAMPLE_RATE
#define UAC_SAMPLE_RATE                     16000

#if (USE_WHITE_LED == 0)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_SEMI
#define UVC_CLR2IR_THR4ISP                  (-50)
#undef UVC_DARK_WATCH_COUNTER
#define UVC_DARK_WATCH_COUNTER              10

#elif (USE_WHITE_LED == 1)
#undef USE_3M_MODE
#define USE_3M_MODE                         U3M_DEFAULT
#define UVC_CLR2IR_THR4ISP                  (-200) //threshold value for turning white led on.
#define UVC_CLR2IR_THR4ENGINE               (-30)

#else // USE_WHITE_LED
#error "USE_WHITE_LED must be 0 or 1."
#endif // USE_WHITE_LED

//----------------------------------------------------------

#endif // FRM_PRODUCT_TYPE

#if (UVC_MAX_WIDTH < UVC_WIDTH && UVC_MAX_HEIGHT < UVC_HEIGHT)
#error "uvc width and height must be less or equal than uvc max."
#elif (UVC_MAX_WIDTH < UVC_HEIGHT && UVC_MAX_HEIGHT < UVC_WIDTH)
#error "uvc width and height must be less or equal than uvc max2."
#endif // uvc max

#if (USE_DB_UPDATE_MODE)
#define DB_UPDATE_MAGIC "EasenDB1"
#define DB_UPDATE_MAGIC_2 "EasenDB2" // face
#define DB_UPDATE_MAGIC_3 "EasenDB3" // hand
#define MAGIC_LEN_UPDATE_DB 8
#endif // USE_DB_UPDATE_MODE

#if (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v13 || DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v23 || DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v44)
#undef ISP_Y_LEVEL
#define ISP_Y_LEVEL                 ISP_Y_LEVEL_1
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v26)
#undef ISP_Y_LEVEL
#define ISP_Y_LEVEL                 ISP_Y_LEVEL_2
#elif (DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v50 || DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v57 || DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v58 || DEFAULT_ISP_BIN_VER == ISP_BIN_VER_21v60)
#undef ISP_Y_LEVEL
#define ISP_Y_LEVEL                 ISP_Y_LEVEL_3
#endif

//#include "engine_inner_param.h"

#endif // APPDEF

