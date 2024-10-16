#ifndef _CONFIG_VARS_H
#define _CONFIG_VARS_H

#include <stdio.h>
#include "appdef.h"

#define DICT_START_ADDR                 0x0
#define DICT_PART_SIZE                  (8*1024*1024) //8MB
#define USERDB_START_ADDR               (DICT_START_ADDR + DICT_PART_SIZE)
#define USERDB_SIZE                     0x00100000
#ifdef _PACK_FIRM_
#define FN_WNO_DICT_SIZE                (USE_RENT_ENGINE ? 3557760: 3562136)
#define FN_WNOH_DICT_SIZE               2410080
#define FN_A1_DICT_SIZE                 (442504)
#define FN_A2_DICT_SIZE                 (442504)
#define FN_B_DICT_SIZE                  (442504)
#define FN_B2_DICT_SIZE                 (152920)
#define FN_C_DICT_SIZE                  (442504)
#define FN_CH_DICT_SIZE                 (442504)
#define FN_DETECT_DICT_SIZE             (129320)
#define FN_DETECT_H_DICT_SIZE           (125328)
#define FN_DETECT_C_DICT_SIZE           (841576)
#define FN_DLAMK_DICT_SIZE              (624696)
#define FN_DLAMK_H_DICT_SIZE            (495328)
#define FN_H_LIVE_DICT_SIZE             442504
#define FN_ESN_DICT_SIZE                241336
#define FN_OCC_DICT_SIZE                738800
#define FN_H1_DICT_SIZE                 290964
#define FN_H2_DICT_SIZE                 4263260

#define FN_WNO_DICT_SIZE_REAL           (USE_RENT_ENGINE ? 1819013: 1821548)
#define FN_WNOH_DICT_SIZE_REAL          1106620
#define FN_A1_DICT_SIZE_REAL            (347897)
#define FN_A2_DICT_SIZE_REAL            (344236)
#define FN_B_DICT_SIZE_REAL             (345726)
#define FN_B2_DICT_SIZE_REAL            (114273)
#define FN_C_DICT_SIZE_REAL             (362995)
#define FN_CH_DICT_SIZE_REAL            (299161)
#define FN_DETECT_DICT_SIZE_REAL        (85012)
#define FN_DETECT_H_DICT_SIZE_REAL      (78252)
#define FN_DETECT_C_DICT_SIZE_REAL      (841576)
#define FN_DLAMK_DICT_SIZE_REAL         (392212)
#define FN_DLAMK_H_DICT_SIZE_REAL       (231113)
#define FN_H_LIVE_DICT_SIZE_REAL        347165

#define FN_FACE_IR_BIN_SIZE             26696
#define FN_FACE_IR_BIN_SIZE_REAL        16458
#define FN_TESTAUDIO_PCM_SIZE           66416
#define FN_TESTAUDIO_PCM_SIZE_REAL      36360
#define FN_ISP_IR_BIN_SIZE              174513
#define FN_ISP_IR_BIN_SIZE_REAL         31766

#else
#include "auto_config_vars.h"
#endif

/*--fast=4
wno.bin : 52.90%   (3562136 => 1884446 bytes, wno.bin.zst) 
detect.bin : 62.97%   (129320 =>  81430 bytes, detect.bin.zst) 
dlamk.bin : 64.20%   (624696 => 401066 bytes, dlamk.bin.zst) 
a1.bin : 80.69%   (442504 => 357060 bytes, a1.bin.zst) 
a2.bin : 80.13%   (442504 => 354590 bytes, a2.bin.zst) 
b.bin : 79.35%   (442504 => 351129 bytes, b.bin.zst) 
b2.bin : 77.17%   (152920 => 118005 bytes, b2.bin.zst) 
c.bin : 85.39%   (442504 => 377862 bytes, c.bin.zst) 
detect_h.bin : 65.05%   (121464 =>  79016 bytes, detect_h.bin.zst) 
dlamk_h.bin : 16.30%   (496008 =>  80833 bytes, dlamk_h.bin.zst) 
ch.bin : 69.11%   (442504 => 305807 bytes, ch.bin.zst) 
wnh.bin : 51.91%   (2732440 => 1418292 bytes, wnh.bin.zst) 
./pack/rc/face_ir.bin :100.01%   (427136 => 427161 bytes, ./pack/rc/face_ir.bin.zst) 
*/

#define UPGRADER_INFO_ADDR              (DICT_START_ADDR + DICT_PART_SIZE - 8192)

#define APPLOG_LEN                      4096
#define APPLOG_START_ADDR               0 //(UPGRADER_INFO_ADDR - 8192)
#define APPLOG_SIZE_LEN                 4
#define UPGRADER_INFO_SIZE              64
#define IR_ERROR_SAVE_ADDR              (16*1024*1024 - 1024*1024)

#define FN_WNO_DICT_PATH        (USE_RENT_ENGINE ? "/wno_c.bin" : "/wno.bin")
#define FN_WNOH_DICT_PATH       "/wnh.bin"
#define FN_A1_DICT_PATH         "/a1.bin"
#define FN_A2_DICT_PATH         "/a2.bin"
#define FN_B_DICT_PATH          "/b.bin"
#define FN_B2_DICT_PATH         "/b2.bin"
#define FN_CH_DICT_PATH         "/ch.bin"
#define FN_C_DICT_PATH          "/c.bin"
#define FN_DETECT_DICT_PATH     "/detect.bin"
#define FN_DETECT_H_DICT_PATH   "/detect_h.bin"
#define FN_DETECT_C_DICT_PATH   "/detect_c.bin"
#define FN_DLAMK_DICT_PATH      "/dlamk.bin"
#define FN_DLAMK_H_DICT_PATH    "/dlamk_h.bin"
#define FN_ESN_DICT_PATH        "/esn.bin"
#define FN_OCC_DICT_PATH        "/occ.bin"
#define FN_H_LIVE_DICT_PATH     "/lh.bin"
#define FN_H1_DICT_PATH         "/hdic_1.bin"
#define FN_H2_DICT_PATH         "/hdic_2.bin"

#define FN_DICT_ALIGN_SIZE      64

#define IR_TEST_BIN_WIDTH       752
#define IR_TEST_BIN_HEIGHT      568
#define IR_TEST_BIN_W_START     424
#define IR_TEST_BIN_H_START     166
#define IR_TEST_BIN_WREAL       188
#define IR_TEST_BIN_HREAL       142
#define CLR_TEST_BIN_WIDTH      174
#define CLR_TEST_BIN_HEIGHT     136
#define CLR_TEST_BIN_W_START    232
#define CLR_TEST_BIN_H_START    172

#define FN_FACE_BIN_PATH                "face.bin"
#define FN_FACE_IR_BIN_PATH             "face_ir.bin"
#define FN_FACE_CLR_BIN_PATH            "face_clr.bin"
#define FN_TESTAUDIO_PCM_PATH           "audiotest.pcm"
#define FN_FACE_IR_BIN_CHKSUM           0x47866b35
#define FN_ISP_IR_BIN_PATH              (UVC_IRLED_ON == 0 ? "v1.0.0.1.bin" : "v2.1.6.2.bin")
#define FN_ISP_IR_BIN_CHKSUM            (UVC_IRLED_ON == 0 ? 0xdcf8b2df : 0xc10749fb)

#define MY_32ALIGNED(a) (((a) + 31) / 32 * 32)

typedef struct {
    char* m_filename;
    int m_filesize;
    int m_filesize_de;
    int m_checksum;
    int m_flag; //flags for crypto
    int m_cryptosize;
    char m_macro_prefix[128];
} st_file_offsize;

//flags for crypto
enum {
    FN_CRYPTO_NONE = 0,
    FN_CRYPTO_AES = 1, //aes with static key
    FN_CRYPTO_AES_DYN_ID2 = 2, //aes with dynamic key of cpu id and spi nor flash id
    FN_CRYPTO_ZSTD = 4, //zstd compression
};

#ifdef __cplusplus
extern "C" st_file_offsize g_part_files[];
extern "C" unsigned char g_part_crypto_aes_key[];
#else // __cplusplus
extern st_file_offsize g_part_files[];
extern unsigned char g_part_crypto_aes_key[];
#endif // __cplusplus

#endif // !_CONFIG_VARS_H