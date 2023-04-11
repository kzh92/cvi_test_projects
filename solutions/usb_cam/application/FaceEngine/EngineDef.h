#ifndef ENGINEDEF_H
#define ENGINEDEF_H

#include "appdef.h"

#define LIBFOO_DLL_EXPORTED             __attribute__((__visibility__("default")))

#define N_FACE_WID                      60
#define N_FACE_HEI                      80
#define N_FACE_SIZE                     (N_FACE_WID * N_FACE_HEI)

#define N_USER_PERM_BEGIN_ID            0

#define N_MAX_JPG_FACE_IMAGE_SIZE       2000

#define MAX_CALCOFFSET_COUNT            10
#define MAX_CALCOFFSETPROCESS_COUNT     12
#define N_MAX_NAME_LEN                  32

#define E_IMAGE_WIDTH                   1600
#define E_IMAGE_HEIGHT                  900
#define E_CLR_WIDTH                     320
#define E_CLR_HEIGHT                    240

#define N_D_ENGINE_SIZE                 (E_IMAGE_WIDTH * E_IMAGE_HEIGHT)
#define N_M_ENGINE_SIZE                 (E_IMAGE_WIDTH * E_IMAGE_HEIGHT)
#define N_C_ENGINE_SIZE                 (E_CLR_WIDTH * E_CLR_HEIGHT)

//////////////////////////LOG FILTER////////////////////////////////

#define LOG_FILTER_NUM                  8

#define LOG_FILTER_FACE                 0
#define LOG_FILTER_MGR                  1

//////////////////////////LOG NUM////////////////////////////////

#define N_MAX_FACE_LOG_NUM              1000
#define N_MAX_LOG_NUM                   1000

//////////////////////////LOG TYPE////////////////////////////////

#define LOG_TYPE_FACE                   0

//////////////////////////META INFO////////////////////////////////
#define PREFIX_USER_DB_NAME             "userdb"
#define PREFIX_USER_LOG_DB_NAME         "logdb_face"

//////////////////////////LOG INFO////////////////////////////////

#define LOG_INFO_FAILED                 0
#define LOG_INFO_SUCCESS                1
#define LOG_INFO_CARD_SAFE_BOLT         3
#define LOG_INFO_CARD_SUCCESS           4
#define LOG_INFO_KEY_LOG                5
#define LOG_INFO_OPEN_LOCK_FAILED       6
#define LOG_INFO_OPEN_LOCK_SUCCESS      7
#define LOG_INFO_REQUEST_UNLOCK         8
#define LOG_INFO_THREAD_PASS_SUCCESS    9
#define LOG_INFO_TMEMP_PASS_SUCCESS     10
#define LOG_INFO_OPT_PASS_SUCCESS       11
#define LOG_INFO_PASS_SAFE_BOLT         12
#define LOG_INFO_FACE_SAFE_BOLT         13
#define LOG_INFO_OPEN_LOCK_SAFE_BOLT    14
#define LOG_INFO_FP_FAILED              15
#define LOG_INFO_FP_SUCCESS             16
#define LOG_INFO_FP_SAFEBOLT            17
#define LOG_INFO_CARD_FAILED            18
#define LOG_INFO_HAND_FAILED            19
#define LOG_INFO_HAND_SUCCESS           20
#define LOG_INFO_HAND_SAFEBOLT          21
#define LOG_INFO_KEEP_OPEN_LOCK         30

/////////////////////////////BLOCK/////////////////////////////////
#define N_USER_BLOCK_SIZE 10
#define N_LOG_BLOCK_SIZE 40
//////////////////////////////////////////////////////////////////

enum EAUTHORITY
{
    EUSER_ROLE_MIN = 1,
    EUSER_ROLE1 = 1,
    EUSER_ROLE2 = 2,
    EUSER_ROLE3 = 3,
    EUSER_ROLE_MAX = 3
};

enum ENOWSTATE
{
	ENS_VERIFY,
	ENS_REGISTER
};

enum EEnroll_Kind
{
    EEK_Face,
    EEK_Hand
};

enum EEngineState
{
    ES_SUCCESS,
    ES_FAILED,
    ES_PROCESS,

    ES_LARGEFACE,
    ES_SMALLFACE,
    ES_NOCHANGE,

    ES_FRONTAL,
    ES_LEFTFACE,
    ES_RIGHTFACE,

    ES_ENEXT,
    ES_EFINISH,
    ES_EPREV,
    ES_UPDATE,

    ES_DUPLICATED,
    ES_FULL,
    ES_FILE_ERROR,
    ES_INVALID,
    ES_SPOOF_FACE,
    ES_SUCCESS_A,
    ES_SUCCESS_B,
    ES_SUCCESS_AB,
	ES_DIRECTION_ERROR,
};

#endif // ARMDEF_H
