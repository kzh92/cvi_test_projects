#ifndef _CONFIG_VARS_H
#define _CONFIG_VARS_H

#include <stdio.h>
#include "appdef.h"

#define DICT_START_ADDR                 0x0
#define DICT_PART_SIZE                  (8*1024*1024) //8MB
#define USERDB_START_ADDR               (DICT_START_ADDR + DICT_PART_SIZE)
#define USERDB_SIZE                     0x00100000
#define FN_WNO_DICT_SIZE                3562136
#define FN_WNOH_DICT_SIZE               2732440
#if (USE_FP16_ENGINE == 0)
#define FN_A1_DICT_SIZE                 792992
#define FN_A2_DICT_SIZE                 792992
#define FN_B_DICT_SIZE                  792992
#define FN_B2_DICT_SIZE                 246968
#define FN_C_DICT_SIZE                  792992
#define FN_DETECT_DICT_SIZE             180992
#define FN_DLAMK_DICT_SIZE              1153056
#else // USE_FP16_ENGINE == 0
#define FN_A1_DICT_SIZE                 (442504)
#define FN_A2_DICT_SIZE                 (442504)
#define FN_B_DICT_SIZE                  (442504)
#define FN_B2_DICT_SIZE                 (152920)
#define FN_C_DICT_SIZE                  (442504)
#define FN_CH_DICT_SIZE                 (442504)
#define FN_DETECT_DICT_SIZE             (129320)
#define FN_DETECT_H_DICT_SIZE           (121464)
#define FN_DLAMK_DICT_SIZE              (624696)
#define FN_DLAMK_H_DICT_SIZE            (496008)
#endif // USE_FP16_ENGINE == 0
#define FN_ESN_DICT_SIZE                123860
#define FN_OCC_DICT_SIZE                363416
#define FN_H1_DICT_SIZE                 290964
#define FN_H2_DICT_SIZE                 4263260

#define UPGRADER_INFO_ADDR              (DICT_START_ADDR + DICT_PART_SIZE - 8192)

#define APPLOG_LEN                      4096
#define APPLOG_START_ADDR               0 //(UPGRADER_INFO_ADDR - 8192)
#define APPLOG_SIZE_LEN                 4
#define UPGRADER_INFO_SIZE              64
#define IR_ERROR_SAVE_ADDR              (16*1024*1024 - 1024*1024)

#define FN_WNO_DICT_PATH        "/test/wno.bin"
#define FN_WNOH_DICT_PATH       "/test/wnh.bin"
#define FN_A1_DICT_PATH         "/test/a1.bin"
#define FN_A2_DICT_PATH         "/test/a2.bin"
#define FN_B_DICT_PATH          "/test/b.bin"
#define FN_B2_DICT_PATH         "/test/b2.bin"
#define FN_CH_DICT_PATH         "/test/ch.bin"
#define FN_C_DICT_PATH          "/test/c.bin"
#define FN_DETECT_DICT_PATH     "/test/detect.bin"
#define FN_DETECT_H_DICT_PATH   "/test/detect_h.bin"
#define FN_DLAMK_DICT_PATH      "/test/dlamk.bin"
#define FN_DLAMK_H_DICT_PATH    "/test/dlamk_h.bin"
#define FN_ESN_DICT_PATH        "/test/esn.bin"
#define FN_OCC_DICT_PATH        "/test/occ.bin"
#define FN_H1_DICT_PATH         "/test/hdic_1.bin"
#define FN_H2_DICT_PATH         "/test/hdic_2.bin"

#define FN_DICT_ALIGN_SIZE      64

#define IR_TEST_BIN_WIDTH       752
#define IR_TEST_BIN_HEIGHT      560
#define IR_TEST_BIN_W_START     424
#define IR_TEST_BIN_H_START     166
#define CLR_TEST_BIN_WIDTH      174
#define CLR_TEST_BIN_HEIGHT     136
#define CLR_TEST_BIN_W_START    232
#define CLR_TEST_BIN_H_START    172

#define FN_FACE_BIN_PATH        "/test/face.bin"
#define FN_FACE_IR_BIN_PATH     "/test/face_ir.bin"
#define FN_FACE_CLR_BIN_PATH    "/test/face_clr.bin"

#define FN_FACE_IR_BIN_SIZE     427136

#define FN_031TTS_WAV_PATH  "sound/031TTS.wav"
#define FN_031TTS_WAV_SIZE  250028
#define FN_032TTS_WAV_PATH  "sound/032TTS.wav"
#define FN_032TTS_WAV_SIZE  80684
#define FN_033TTS_WAV_PATH  "sound/033TTS.wav"
#define FN_033TTS_WAV_SIZE  53036
#define FN_034TTS_WAV_PATH  "sound/034TTS.wav"
#define FN_034TTS_WAV_SIZE  148652
#define FN_035TTS_WAV_PATH  "sound/035TTS.wav"
#define FN_035TTS_WAV_SIZE  156716
#define FN_036TTS_WAV_PATH  "sound/036TTS.wav"
#define FN_036TTS_WAV_SIZE  51884
#define FN_TEST_WAV_PATH    "sound/test.wav"
#define FN_TEST_WAV_SIZE    91606

typedef struct {
    char* m_filename;
    int m_filesize;
    int m_checksum;
    int m_flag; //flags for crypto
    int m_cryptosize;
} st_file_offsize;

//flags for crypto
enum {
    FN_CRYPTO_NONE = 0,
    FN_CRYPTO_AES = 1, //aes with static key
    FN_CRYPTO_AES_DYN_ID2 = 2, //aes with dynamic key of cpu id and spi nor flash id
};

#ifdef __cplusplus
extern "C" st_file_offsize g_part_files[];
extern "C" unsigned char g_part_crypto_aes_key[];
#else // __cplusplus
extern st_file_offsize g_part_files[];
extern unsigned char g_part_crypto_aes_key[];
#endif // __cplusplus

#endif // !_CONFIG_VARS_H