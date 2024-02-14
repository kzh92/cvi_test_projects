#ifndef SENSE_LOCK_MESSAGE_H
#define SENSE_LOCK_MESSAGE_H

// #include <stdio.h>
#include <stdint.h>
// #include <string.h>
// #include <stdarg.h>
#include "appdef.h"

#define USER_NAME_SIZE                  32
#define USER_NAME_SIZE_EXT              256
#define VERSION_INFO_BUFFER_SIZE        32
#define UID_INFO_BUFFER_SIZE            32
#define MAX_USER_COUNTS                 (N_MAX_PERSON_NUM > N_MAX_HAND_NUM ? N_MAX_PERSON_NUM : N_MAX_HAND_NUM)
#define MAX_IMAGE_SIZE                  (4000)
#define TRANS_FILE_MAX_SIZE             (1024*1024)

#if (USE_FUSHI_PROTO)
#define FUSHI_HEAD1     0x53
#define FUSHI_HEAD2     0x46
#endif // USE_FUSHI_PROTO

#define SENSE_HEAD1     0xEF
#define SENSE_HEAD2     0xAA

enum
{
    OTA_RECV_PCK,
    OTA_RECV_DONE_OK,
    OTA_RECV_DONE_STOP,
    OTA_RECV_CHECKSUM_ERROR,
    OTA_RECV_PACKET_ERROR,
    OTA_RECV_TIMEOUT,
    OTA_USB_START,
    OTA_USB_DETECTED,
    SENSE_READY_DETECTED,
    OTA_CUS_DATA = 0xff,
};

#define EKESI_ID_TYPE_FACE                                  1
#define EKESI_ID_TYPE_HAND                                  2

//unlock status
#define ST_FACE_MODULE_STATUS_UNLOCK_OK                     (USE_EKESI_PROTO ? EKESI_ID_TYPE_FACE : 200)
#define ST_FACE_MODULE_STATUS_UNLOCK_WITH_EYES_CLOSE        204
#if (ENROLL_FACE_HAND_MODE != ENROLL_FACE_HAND_MIX && USE_TONGXIN_PROTO == 0)
#if (USE_EKESI_PROTO)
#define ST_FACE_MODULE_STATUS_UNLOCK_HAND_OK                EKESI_ID_TYPE_HAND
#else
#define ST_FACE_MODULE_STATUS_UNLOCK_HAND_OK                (USE_SHENAO_HAND ? (ST_FACE_MODULE_STATUS_UNLOCK_OK) : 250)
#endif
#else // ENROLL_FACE_HAND_MODE
#define ST_FACE_MODULE_STATUS_UNLOCK_HAND_OK                (USE_EKESI_PROTO ? EKESI_ID_TYPE_HAND : ST_FACE_MODULE_STATUS_UNLOCK_OK)
#endif // ENROLL_FACE_HAND_MODE

#pragma pack(push, 1)


/* communication message ID definitions*/
typedef uint8_t s_msg_id;
// Module to Host (m->h)
#define MID_REPLY                       0x00    // request(command) reply message, success with data or fail with reason
#define MID_NOTE                        0x01    // note to host e.g. the position or angle of the face
#define MID_IMAGE                       0X02    // send image to host
// Host to Module (h->m)
#define MID_RESET                       0x10    // stop and clear all in-processing messages. enter standby mode
#define MID_GETSTATUS                   0x11    // to ping the module and get the status
#define MID_VERIFY                      0x12    // to verify the person in front of the camera
#define MID_ENROLL                      0x13    // to enroll and register the persion in front of the camera
#define MID_SNAPIMAGE                   0x16    // to snap a picture and save it
#define MID_GETSAVEDIMAGE               0x17    // to get size of saved image
#define MID_UPLOADIMAGE                 0x18    // upload images
#define MID_ENROLL_SINGLE               0x1D    // to enroll and register the persion in front of the camera, single frame
#define MID_DELUSER                     0x20    // Delete the specified user with user id
#define MID_DELALL                      0x21    // Delete all registerred users
#define MID_GETUSERINFO                 0x22    // Get user info
#define MID_FACERESET                   0x23    // Reset face status
#define MID_GET_ALL_USERID              0x24    // get all users ID
#define MID_ENROLL_ITG                  0x26    // Enroll user, extended mode
#define MID_GET_VERSION                 0x30    // get version information
#define MID_WRITE_SN                    0x31    // write sn to board
#define MID_GET_SN                      0x35    // get the serial number of the device
#define MID_START_OTA                   0x40    // ask the module to enter OTA mode
#define MID_STOP_OTA                    0x41    // ask the module to exit OTA mode
#define MID_GET_OTA_STATUS              0x42    // query the current ota status
#define MID_OTA_HEADER                  0x43    // the ota header data
#define MID_OTA_PACKET                  0x44    // the data packet, carries real firmware data
#define MID_INIT_ENCRYPTION             0x50    // initialize encrypted communication
#define MID_CONFIG_BAUDRATE             0x51    // config uart baudrate
#define MID_SET_RELEASE_ENC_KEY         0x52    // set release encrypted key(Warning!!!:Once set, the KEY will not be able to modify)
#define MID_SET_ENC_KEY                 0x53    // set encrypted key, MID_ENCKEY
#define MID_GET_LOGFILE                 0x60    // get log file
#define MID_UPLOAD_LOGFILE              0x61    // upload log file cmd to cloud
#if 0
#define MID_VIDEO_ON                    0x70    // video on
#define MID_VIDEO_OFF                   0x71    // video off
#endif
#define MID_UVC_DIR                     0x76    // rotate uvc image
#define MID_UVC_SET_COMPRESS_PARAM      0x77    // set UVC compression parameters
#define MID_TRANS_FILE_PACKET           0x90    // receive file data from master
#define MID_ENROLL_FROM_IMAGE           0x91    // enroll with image data of MID_TRANS_FILE_PACKET
#define MID_GET_UID                     0x93    // XiShang require module ID
#define MID_MX_GET_ALL_USERID           0x9B    // 등록된 사용자수얻기
#define MID_SEND_LAST_MSG               0xA0    // 더스만모듈에서 응답을 받아서 주동적으로 체계를 끄는 방식설정
#define MID_ENABLE_LOGFILE              0xA1    // 엔진상세로그보관설정
#define MID_SET_LIVENESS_MODE           0xA2    // set liveness mode
#define MID_SET_ENCRYPTION_MODE         0xA3    // set encryption mode
#define MID_HIJACK                      0xB2    // Enable or disable Hijack function
#define MID_FACTORY_TEST                0xC0    // factory test
#define MID_DDR_TEST                    0xC1    // DDR test
#define MID_CAMERA_FLIP                 0xC2    // Camera flip, 180 degree
#define MID_POWERDOWN                   0xC3    // be prepared to power off, changed to C3
#define MID_SET_TWINS_MODE              0xC5    // use twins engine.
#define MID_GET_TWINS_MODE              0xC6    // use twins engine.
#define MID_FUNC_CTRL                   0xC7
#define MID_SET_THRESHOLD_LEVEL         0xD4    // Set threshold level
#define MID_POWERDOWN_ED                0xED    // be prepared to power off, ignore now
#define MID_DEBUG_MODE                  0xF0
#define MID_GET_DEBUG_INFO              0xF1    // get size of debug information
#define MID_UPLOAD_DEBUG_INFO           0xF2    // upload debug information
#define MID_DEMOMODE                    0xFE    // enter demo mode, verify flow will skip feature comparation step.
#define MID_SNAPIMAGE2                  0xFF    // snapimage for renthouse
/* communication message ID definitions end */

/* message result code */
typedef uint8_t s_msg_result;
#define MR_SUCCESS                          0      // success
#define MR_REJECTED                         1      // module rejected this command
#define MR_ABORTED                          2      // algo aborted
#define MR_FAILED4_CAMERA                   4  // camera open failed
#define MR_FAILED4_UNKNOWNREASON            5  // UNKNOWN_ERROR
#define MR_FAILED4_INVALIDPARAM             6   // invalid param
#define MR_FAILED4_NOMEMORY                 7       // no enough memory
#define MR_FAILED4_UNKNOWNUSER              8    // no user enrolled
#define MR_FAILED4_MAXUSER                  9        // exceed maximum user number
#define MR_FAILED4_FACEENROLLED             10  // this face has been enrolled
#define MR_FAILED4_LIVENESSCHECK            12 // liveness check failed
#define MR_FAILED4_TIMEOUT                  13       // exceed the time limit
#define MR_FAILED4_AUTHORIZATION            14 // authorization failed
#define MR_FAILED4_CAMERAFOV                15     // camera fov test failed
#define MR_FAILED4_CAMERAQUA                16     // camera quality test failed
#define MR_FAILED4_CAMERASTRU               17    // camera structure test failed
#define MR_FAILED4_BOOT_TIMEOUT             18  // boot up timeout
#define MR_FAILED4_READ_FILE                19     // read file failed
#define MR_FAILED4_WRITE_FILE               20    // write file failed
#define MR_FAILED4_NO_ENCRYPT               21    // encrypt must be set
#define MR_FAILED4_NOCAMERA                 240     // MID_GETSAVEDIMAGE, failed to get scene picture, no color camera.
#if (ENROLL_FACE_HAND_MODE != ENROLL_FACE_HAND_MIX && USE_TONGXIN_PROTO == 0)
#define MR_FAILED4_UNKNOWN_HANDUSER         239   // no hand user enrolled
#define MR_FAILED4_HANDENROLLED             241 // this hand has been enrolled
#elif (USE_EKESI_PROTO == 1)
#define MR_FAILED4_UNKNOWN_HANDUSER         22   // no hand user enrolled
#define MR_FAILED4_HANDENROLLED             MR_FAILED4_FACEENROLLED // this hand has been enrolled
#else // ENROLL_FACE_HAND_MODE
#define MR_FAILED4_UNKNOWN_HANDUSER         MR_FAILED4_UNKNOWNUSER   // no hand user enrolled
#define MR_FAILED4_HANDENROLLED             MR_FAILED4_FACEENROLLED // this hand has been enrolled
#endif // ENROLL_FACE_HAND_MODE
/* message result code end */

/* module state */
typedef uint8_t s_mstate;
#define MS_STANDBY 0  // IDLE, waiting for commands
#define MS_BUSY 1     // in working of processing commands
#define MS_ERROR 2    // in error state. can't work properly
#define MS_INVALID 3  // not initialized
#define MS_OTA 4      // in ota state
/* module state end */

// a general message adapter
typedef struct {
    uint8_t mid;       // the message id
    uint8_t size_heb;  // high eight bits
    uint8_t size_leb;  // low eight bits
    uint8_t data[0];
} s_msg;

/* message data definitions */
/* module -> host note */
/* msg note id */
typedef uint8_t s_note_id;
#define NID_READY 0         // module ready for handling request (command)
#define NID_FACE_STATE 1    // the detected face status description
#define NID_UNKNOWNERROR 2  // unknown error
#define NID_OTA_DONE 3      // OTA upgrading processing done
#define NID_EYE_STATE 4      // OTA upgrading processing done
/* msg note id end */

//module type in note command
#define NMT_FACE            0
#define NMT_HAND            1
#define NMT_IRIS            2
#define NMT_FACE_HAND       3

/* msg face direction */
typedef uint8_t s_face_dir;
#define FACE_DIRECTION_UP       0x10        // face up
#define FACE_DIRECTION_DOWN     0x08        // face down
#define FACE_DIRECTION_LEFT     0x04        // face left
#define FACE_DIRECTION_RIGHT    0x02        // face right
#define FACE_DIRECTION_MIDDLE   0x01        // face middle
#define FACE_DIRECTION_UNDEFINE 0x00        // face undefine
#define FACE_DIRECTION_RENT     0xFC        // register face for rent
#define FACE_DIRECTION_HAND     0xFD        // register hand
#define FACE_DIRECTION_PICTURE  0xFE        // register face with picture

#define IS_FACE_DIR_VALID(a) \
    ((a) == FACE_DIRECTION_UNDEFINE || \
    (a) == FACE_DIRECTION_UP || \
    (a) == FACE_DIRECTION_DOWN || \
    (a) == FACE_DIRECTION_LEFT || \
    (a) == FACE_DIRECTION_RIGHT || \
    (a) == FACE_DIRECTION_MIDDLE || \
    (a) == FACE_DIRECTION_HAND)
/* msg face direction end */

#define FACE_STATE_NORMAL                           0  // normal state, the face is available
#define FACE_STATE_NOFACE                           1  // no face detected
#define FACE_STATE_TOOUP                            2  // face is too up side
#define FACE_STATE_TOODOWN                          3  // face is too down side
#define FACE_STATE_TOOLEFT                          4  // face is too left side
#define FACE_STATE_TOORIGHT                         5  // face is too right side
#define FACE_STATE_TOOFAR                           6  // face is too far
#define FACE_STATE_TOOCLOSE                         7  // face is too near
#define FACE_STATE_EYEBROW_OCCLUSION                8      // eyebrow occlusion
#define FACE_STATE_EYE_OCCLUSION                    9      // eye occlusion
#define FACE_STATE_FACE_OCCLUSION                   10     // face occlusion
#define FACE_STATE_DIRECTION_ERROR                  11     // face direction error
#define FACE_STATE_EYE_CLOSE_STATUS_OPEN_EYE        12    // eye close time out
#define FACE_STATE_EYE_CLOSE_STATUS                 13      // confirm eye close status
#define FACE_STATE_EYE_CLOSE_UNKNOW_STATUS          14  // eye close unknow status
#if (ENROLL_FACE_HAND_MODE != ENROLL_FACE_HAND_MIX)
#define FACE_STATE_HAND_NORMAL                      128 // hand detected
#define FACE_STATE_HAND_FAR                         129 // hand is far
#define FACE_STATE_HAND_CLOSE                       130 // hand is close
#define FACE_STATE_HAND_TOOUP                       131 // hand is too up side
#define FACE_STATE_HAND_TOODOWN                     132 // hand is too down side
#define FACE_STATE_HAND_TOOLEFT                     133 // hand is too left side
#define FACE_STATE_HAND_TOORIGHT                    134 // hand is too right side
#else // ENROLL_FACE_HAND_MODE
#define FACE_STATE_HAND_NORMAL                      FACE_STATE_NORMAL // hand detected
#define FACE_STATE_HAND_FAR                         FACE_STATE_TOOFAR // hand is far
#define FACE_STATE_HAND_CLOSE                       FACE_STATE_TOOCLOSE // hand is close
#define FACE_STATE_HAND_TOOUP                       FACE_STATE_TOOUP // face is too up side
#define FACE_STATE_HAND_TOODOWN                     FACE_STATE_TOODOWN // face is too down side
#define FACE_STATE_HAND_TOOLEFT                     FACE_STATE_TOOLEFT // face is too left side
#define FACE_STATE_HAND_TOORIGHT                    FACE_STATE_TOORIGHT // face is too right side
#endif // ENROLL_FACE_HAND_MODE

/* logfile type */
#define LOG_FILE_KERNEL 0  // kernel log
#define LOG_FILE_SENSELOCK_APP 1     // senselock app log
#define LOG_FILE_FACE_MODULE 2    // face module log
/* logfile type end */

typedef struct {
    int16_t state; // corresponding to FACE_STATE_*

    // position
    int16_t left;  // in pixel
    int16_t top;
    int16_t right;
    int16_t bottom;

    // pose
    uint8_t user_id_heb; // high eight bits of user_id to be deleted
    uint8_t user_id_leb; // low eight bits pf user_id to be deleted
    int16_t yaw;   // up and down in vertical orientation
    int16_t pitch; // right or left turned in horizontal orientation
    int16_t roll;  // slope
} s_note_data_face;

typedef struct {
    uint8_t eye_state; // corresponding to FACE_STATE_*
} s_note_data_eye;

typedef struct {
    uint8_t nid;    // note id
    uint8_t data[0];
} s_msg_note_data;
/* module -> host note end */

// enroll user
typedef struct {
    uint8_t admin; // the user will be set to admin
    uint8_t user_name[USER_NAME_SIZE];
    s_face_dir face_direction;
    uint8_t timeout; // timeout, unit second default 10s
} s_msg_enroll_data;

// enroll user intergrated
typedef struct {
    uint8_t admin; // the user will be set to admin, 1:admin user, 0:normal user
    uint8_t user_name[USER_NAME_SIZE];
    uint8_t face_direction; // reference FACE_DIRECTION_*
    uint8_t enroll_type; // reference FACE_ENROLL_TYPE_*
    uint8_t enable_duplicate; // enable user enroll duplicatly, 1:enable, 0:disable
                              // when enroll_type is equal to FACE_ENROLL_TYPE_RGB， the enable_duplicate can be set to 2, it means that cant duplicate enroll with username
    uint8_t timeout; // timeout unit second, default 10s
} s_msg_enroll_itg;

#define SF_DEF_FACE_ENROLL_TIMEOUT          10 //10 secs
#define IS_SF_FACE_ENROLL_TIMEOUT_VALID(a)  (((unsigned int)(a)) <= 255)

enum {
    FACE_ENROLL_TYPE_INTERACTIVE, //0
    FACE_ENROLL_TYPE_SINGLE, //1
    FACE_ENROLL_TYPE_FACE_HAND, //2
    FACE_ENROLL_TYPE_HAND, //3
    FACE_ENROLL_TYPE_END,
};

#define IS_FACE_ENROLL_TYPE_VALID(a) ((a) >= FACE_ENROLL_TYPE_INTERACTIVE && (a) < FACE_ENROLL_TYPE_END)

//duplication checking mode
#define FACE_ENROLL_DCM_NFACE_NAME          0   //not allow same face, allow same name
#define FACE_ENROLL_DCM_FACE_NAME           1   //allow same face, allow same name
#define FACE_ENROLL_DCM_FACE_NNAME          2   //allow same face, not allow same name
#define IS_FACE_ENROLL_DCM_VALID(a) (((unsigned int)(a)) <= FACE_ENROLL_DCM_FACE_NNAME)

#define SF_ENROLL_NOR2ITG(org, dst) \
    do { \
        memset((dst), 0, sizeof(*(dst))); \
        (dst)->admin = (((org)->admin == 1) ? 1 : 0); \
        strcpy((char*)(dst)->user_name, (const char*)(org)->user_name); \
        (dst)->face_direction = (org)->face_direction; \
        (dst)->timeout = (org)->timeout; \
    } while (0)

// verify
typedef struct {
    uint8_t pd_rightaway; // power down right away after verifying
    uint8_t timeout; // timeout, unit second, default 10s
} s_msg_verify_data;

// delete user
typedef struct {
    uint8_t user_id_heb; // high eight bits of user_id to be deleted
    uint8_t user_id_leb; // low eight bits pf user_id to be deleted
    uint8_t user_type; //face or hand
} s_msg_deluser_data;

//user_type of s_msg_deluser_data
enum {
    SM_DEL_USER_TYPE_DEFAULT, //face user
#if (N_MAX_HAND_NUM)
    SM_DEL_USER_TYPE_HAND, //hand user
#endif
    SM_DEL_USER_TYPE_END,
};

// get user info
typedef struct {
    uint8_t user_id_heb; // high eight bits of user_id to get info
    uint8_t user_id_leb; // low eight bits of user_id to get info
} s_msg_getuserinfo_data;

// start ota data MID_START_OTA
typedef struct {
    uint8_t v_primary;   // primary version number
    uint8_t v_secondary; // secondary version number
    uint8_t v_revision;  // revision number
} s_msg_startota_data;

// ota header MID_OTA_HEADER
typedef struct {
    uint8_t fsize_b[4];  // OTA FW file size int -> [b1, b2, b3, b4]
    uint8_t num_pkt[4];  // number packet to be divided for transferring, int -> [b1, b2, b3, b4]
    uint8_t pkt_size[2]; // raw data size of single packet
    uint8_t md5_sum[32]; // md5 check sum
} s_msg_otaheader_data;

// ota packet MID_OTA_PACKET
typedef struct {
    uint8_t pid[2];   // the packet id
    uint8_t psize[2]; // the size of this package
    uint8_t data[0];  // data 0 start
} s_msg_otapacket_data;

// MID_INIT_ENCRYPTION data
typedef struct {
    uint8_t seed[4]; // random data as a seed
    uint8_t mode;    // reserved for selecting encrytion mode
    uint8_t crttime[4];
} s_msg_init_encryption_data;

// demo data
typedef struct {
    uint8_t enable; // enable demo or not
} s_msg_demomode_data;

enum {
    E_FACTORY_TEST_OFF = 0,
    E_FACTORY_TEST_NORMAL = 1,
    E_FACTORY_TEST_IRLED_OFF = 0xFD,
    E_FACTORY_TEST_IRLED_ON = 0xFE,
};

typedef s_msg_demomode_data s_msg_factorytest_data;
typedef s_msg_demomode_data s_msg_set_twins_mode_data;
typedef s_msg_demomode_data s_msg_hijack_data;

// uvc_dir data
typedef struct {
    uint8_t rotate; // rotate uvc image
} s_msg_uvc_dir_data;

#define SI_MAX_IMAGE_COUNT      10
#define SI_MAX_IMAGE_SIZE       (10*1024)

// snap image data
typedef struct {
    uint8_t image_counts; // number of stored images
    uint8_t start_number; // start number of stored images
} s_msg_snap_image_data;

typedef struct {
    uint8_t reserved1; //
    uint8_t reserved2; //
    uint8_t reserved3; //
    uint8_t crop_face; // 1: crop face area, 0: whole image
    uint8_t timeout_sec; //timeout in seconds
} s_msg_snap_image2_data;

// get saved image data
typedef struct {
    uint8_t image_number; // number of saved image
    uint16_t image_width;
    uint16_t image_height;
} s_msg_get_saved_image_data;

// get usderdb data
typedef struct {
    uint8_t image_number; // number of saved image
    uint8_t user_id_heb; // high eight bits of user_id to get info
    uint8_t user_id_leb; // low eight bits of user_id to get info
} s_msg_get_userdb_data;

// upload image
typedef struct {
    uint8_t upload_image_offset[4]; // upload image offset, int -> [o1 o2 o3 o4]
    uint8_t upload_image_size[4];   // uploade image size, int -> [s1 s2 s3 s4]
} s_msg_upload_image_data;

// upload logfile
typedef struct {
    uint8_t upload_logfile_offset[4]; // upload logfile offset, int -> [o1 o2 o3 o4]
    uint8_t upload_logfile_size[4];   // uploade logfile size, int -> [s1 s2 s3 s4]
} s_msg_upload_logfile_data;

// DDR test
typedef struct {
    uint8_t ddr_test_counts; // number of DDR testing
} s_msg_ddr_test_data;

/* message reply data definitions */
typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t user_name[USER_NAME_SIZE];
    uint8_t admin;
    uint8_t unlockStatus;
#if (USE_TONGXIN_PROTO)
    uint8_t module_type; // module type
#endif
} s_msg_reply_verify_data;

typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t face_direction;
#if (USE_TONGXIN_PROTO)
    uint8_t recog_type; // enroll recog type
#elif (USE_EKESI_PROTO)
    uint8_t fea_type;   //
#endif
    uint8_t face_data[0];
} s_msg_reply_enroll_data;

//enroll recog type
#define ERT_FACE            0
#define ERT_HAND            1
#define ERT_IRIS            2

typedef struct {
    uint8_t status;
} s_msg_reply_getstatus_data;

typedef struct {
    uint8_t version_info[VERSION_INFO_BUFFER_SIZE];
} s_msg_reply_version_data;

typedef struct {
    uint8_t uid_info[UID_INFO_BUFFER_SIZE];
} s_msg_reply_uid_data;

typedef struct {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t user_name[USER_NAME_SIZE];
    uint8_t admin;
} s_msg_reply_getuserinfo_data;

typedef struct {
    uint8_t user_counts;      // number of enrolled users
    uint8_t users_id[MAX_USER_COUNTS*2];   //use 2 bytes to save a user ID and save high eight bits firstly
} s_msg_reply_all_userid_data;

// get all userid data for extension
typedef struct {
    uint8_t fmt; // data format
} s_msg_get_all_userid_data;

#define SM_USERID_DATA_FMT_DEFAULT      0   // by default, 2 bytes for one user
#define SM_USERID_DATA_FMT_BIT1         1   // 1 bit for one user
#define SM_USERID_DATA_FMT_HAND1        2   // 1 bit for one user
#define SM_USERID_DATA_FMT_BIT_EXT      3   // 1 bit for one user, extended format

typedef struct {
    uint8_t user_counts;      // number of enrolled users
    uint8_t users_id[(MAX_USER_COUNTS + 7) / 8];   //use 2 bytes to save a user ID and save high eight bits firstly
} s_msg_reply_all_userid_data_fmt1;

typedef struct {
    uint8_t magic;      // magic must be 0xFF
    uint16_t max_user_counts;      // reserved, must be 0xFF
    uint16_t user_counts;      // reserved, must be 0xFF
    uint8_t users_id[(MAX_USER_COUNTS + 7) / 8];   //use 2 bytes to save a user ID and save high eight bits firstly
} s_msg_reply_all_userid_data_fmt_ext;

typedef struct {
    uint8_t time_heb;      // high eight bits of factory test time
    uint8_t time_leb;      // low eight bits of facetory test time
} s_msg_reply_factory_test_data;

// MID_INIT_ENCRYPTION reply
typedef struct {
    uint8_t device_id[20]; // the unique ID of this device, string type
} s_msg_reply_init_encryption_data;

// REPLY MID_GET_OTA_STATUS
typedef struct {
    uint8_t ota_status; // current ota status
    uint8_t next_pid_e[2]; // expected next pid, [b0,b1]
} s_msg_reply_getotastatus_data;

typedef struct {
    uint8_t image_size[4];   // image size int -> [s1, s2, s3, s4]
} s_msg_reply_get_saved_image_data;

typedef struct {
    uint8_t  mid; // the command(message) id to reply (the request message ID)
    uint8_t  result; // command handling result: success or failed -> s_msg_result
    uint8_t  data[0];
} s_msg_reply_data;

typedef struct {
    uint8_t  ota_result;//0 :is sucess; 1: is faile;
} s_note_ota_result;

typedef struct {
uint8_t baudrate_index;//1: is 115200 (115200*1); 2 is 230400 (115200*2); 3 is 460800 (115200*4); 4 is 1500000
} s_msg_config_baudrate;

#define SF_OTA_BAUDRATE_115200      1
#define SF_OTA_BAUDRATE_230400      2
#define SF_OTA_BAUDRATE_460800      3
#define SF_OTA_BAUDRATE_1500000     4

#define SF_IS_OTA_BAUDRATE_VALID(a) ((a) >= SF_OTA_BAUDRATE_115200 && (a) <= SF_OTA_BAUDRATE_1500000)

typedef struct {
    uint8_t enc_key_number[16];   //
} s_msg_enc_key_number_data;

typedef struct {
    uint8_t image_quality;      //1~100 percent
    uint8_t bitrate_max;        //4~10
    uint8_t bitrate_default;    //3~9
    uint8_t repeat_frame;       //1~100
} s_msg_uvc_set_compressparam_data;

// liveness mode data
typedef struct {
    uint8_t mode; // 0, 1
} s_msg_livenessmode_data;

typedef s_msg_livenessmode_data s_msg_encryption_mode_data;

#define UVC_COMP_PARAM_IMQ_MIN  1
#define UVC_COMP_PARAM_IMQ_MAX  100
#define UVC_COMP_PARAM_BT_MIN   3
#define UVC_COMP_PARAM_BT_MAX   10
#define UVC_COMP_PARAM_RPFR_MIN   1
#define UVC_COMP_PARAM_RPFR_MAX   100

// delall data
typedef struct {
    uint8_t type;
    uint8_t begin_user_id_heb; // high eight bits of user_id to be deleted
    uint8_t begin_user_id_leb; // low eight bits pf user_id to be deleted
    uint8_t end_user_id_heb; // high eight bits of user_id to be deleted
    uint8_t end_user_id_leb; // low eight bits pf user_id to be deleted
} s_msg_del_all_data;

// type of s_msg_del_all_data
enum {
    SM_DEL_ALL_TYPE_DEFAULT, //delete all user data
    SM_DEL_ALL_TYPE_FACE, //delete only face users
    SM_DEL_ALL_TYPE_HAND, //delete only hand users
    SM_DEL_ALL_TYPE_NORMAL_FACE_USERS, //delete face users that does not have admin flag
    SM_DEL_ALL_TYPE_RANGE_USERS, //delete users in the given range
    SM_DEL_ALL_TYPE_END,
};

typedef struct {
    uint8_t store_type; // must be 0, store in memory
    uint8_t feat_size[4]; // transfer file size, int -> [s1 s2 s3 s4]
    uint8_t offset[4]; // transfer file offset
    uint8_t psize[2]; // packet size
    uint8_t data[0]; // data 0 start
} s_msg_trans_file_data;

#define S_TRANS_STORE_TYPE_DEFAULT          0
#define S_TRANS_STORE_TYPE_UID              0xFE

typedef struct {
    uint8_t m_usercount; // number of registered users
    uint16_t m_userids[0]; // registered user IDs, starts at 1
} s_msg_reply_trans_file_data;

typedef enum {
    EIT_RGB,
    EIT_IR,
    EIT_END
} E_EnrollImageType;

typedef struct {
    uint8_t admin; // the user will be set to admin
    uint8_t user_name[USER_NAME_SIZE];
    uint8_t img_type; // 0 for rgb, 1 for ir, other is invalid
    uint8_t timeout; // timeout, unit second default 10s
} s_msg_enroll_from_img_data;

enum {
    S_VERIFY_LEVEL_VERY_LOW,
    S_VERIFY_LEVEL_LOW,
    S_VERIFY_LEVEL_DEFAULT,
    S_VERIFY_LEVEL_HIGH,
    S_VERIFY_LEVEL_VERY_HIGH,
};

enum {
    S_LIVENESS_LEVEL_VERY_LOW,
    S_LIVENESS_LEVEL_LOW,
    S_LIVENESS_LEVEL_DEFAULT,
    S_LIVENESS_LEVEL_HIGH,
    S_LIVENESS_LEVEL_VERY_HIGH,
};

typedef struct {
    uint8_t verify_threshold_level; // level 0~4, safety from low to high, default 2
    uint8_t liveness_threshold_level; // level 0~4, safety from low to high, default 2
} s_msg_algo_threshold_level;

typedef struct {
    unsigned short user_counts;
} s_msg_reply_mx_get_all_userid_data;

#pragma pack(pop)

#endif // SENSE_LOCK_MESSAGE_H
