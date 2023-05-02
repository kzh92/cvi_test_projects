#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

#include "appdef.h"
#include "engine_inner_param.h"
#include <pthread.h>
#include <stdio.h>

#define SHMKEY              0x123321
#define SHMKEY_LCD          0x123322

#define RFS0_DEVNAME        "/dev/mmcblk0p1"
#define RFS1_DEVNAME        "/dev/mmcblk0p6"
#define RFS2_DEVNAME        "/dev/mmcblk0p7"

#define TMP_DEVNAME         "/dev/mtdblock7"
#define TMP_FSTYPE          "jffs2"
#if (DB_TYPE == TYPE_EXT4)
#define DB_FSTYPE           "ext4"
#elif (DB_TYPE == TYPE_JFFS)
#define DB_FSTYPE           "jffs2"
#endif

#define RFS_SEC_COUNT       114688
#define RFS_SEC_SIZE        512
#define SEC2BYTES(a)        ((a)*RFS_SEC_SIZE)

#define MY_PART_SETT            "pst"
#define MY_PART_MISC            "misc"
#define MY_PART_WX              "pwx"

#ifndef NULL
#define NULL 0
#endif

enum {
    RFS_PART0,
    RFS_PART1,
    RFS_PART2,
    RFS_PART_END
};

#define MAX_ROOTFS_FAIL_COUNT 7

#define EXEC_CHECKSUM_FILE  "/test/exec_checksum.dat"

#define ACT_MARK_LEN        8

enum
{
    DB_PART1,
    DB_PART_BACKUP,
    DB_PART_END
};

typedef struct _tagSHARED_MEM
{
    int     iHeader;
    int     iData0;
    int     iTheme;
    int     iMountPoints;
} SHARED_MEM;

#ifdef __RTK_OS__
typedef int* myfdesc_ptr;
#define is_myfdesc_ptr_valid(a) ((a) != NULL)
#define my_thread_exit(ret)
#else // __RTK_OS__
typedef int myfdesc_ptr;
#define is_myfdesc_ptr_valid(a) ((a) > 0)
#define my_thread_exit(ret) pthread_exit(ret)
#endif // __RTK_OS__
typedef pthread_mutex_t* mymutex_ptr;
typedef pthread_t* mythread_ptr;
typedef void* myi2cdesc_ptr;

#define MYTHREAD_PRIORITY_VERY_LOW      80
#define MYTHREAD_PRIORITY_LOW           85
#define MYTHREAD_PRIORITY_MEDIUM        90
#define MYTHREAD_PRIORITY_HIGH          93
#define MYTHREAD_PRIORITY_VERY_HIGH     97

#define DICT_START_ADDR                 0x0
#define DICT_PART_SIZE                  (8*1024*1024) //8MB
#define USERDB_START_ADDR               (DICT_START_ADDR + DICT_PART_SIZE)
#define USERDB_SIZE                     0x00100000
#define FN_WNO_DICT_SIZE                1292988
#if (USE_FP16_ENGINE == 0)
#define FN_A1_DICT_SIZE                 792992
#define FN_A2_DICT_SIZE                 792992
#define FN_B_DICT_SIZE                  792992
#define FN_B2_DICT_SIZE                 246968
#define FN_C_DICT_SIZE                  792992
#define FN_DETECT_DICT_SIZE             180992
#define FN_DLAMK_DICT_SIZE              1153056
#else // USE_FP16_ENGINE == 0
#define FN_A1_DICT_SIZE                 (792992 / 2)
#define FN_A2_DICT_SIZE                 (792992 / 2)
#define FN_B_DICT_SIZE                  (792992 / 2)
#define FN_B2_DICT_SIZE                 (246968 / 2)
#define FN_C_DICT_SIZE                  (792992 / 2)
#define FN_CH_DICT_SIZE                 (792992 / 2)
#define FN_DETECT_DICT_SIZE             (180992 / 2)
#define FN_DETECT_H_DICT_SIZE           (180992 / 2)
#define FN_DLAMK_DICT_SIZE              (1153056 / 2)
#define FN_DLAMK_H_DICT_SIZE            451356
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


#ifdef __cplusplus
extern  "C"
{
#endif

#if (USE_WIFI_MODULE)
int my_spi_write(char* tx, int len);
int my_spi_read(unsigned char* rx, int len);
#endif

void            CreateSharedMem();
int             CreateSharedLCD();

void            set_kernel_flag_action(int b);
int             get_kernel_flag_action();
unsigned int    get_execfile_checksum(const char* path);
int             get_execfile_checksum_val(const char* path, unsigned int *cs_out);
void            do_backup_exec_checksum();


int             my_system(const char* command);
int             my_fsync(myfdesc_ptr _fd);

int             get_cur_rootfs_part();
int             get_rootfs_checksum(const char *dev_path, unsigned int* n_checksum_ptr);

void*           my_malloc_real(unsigned int nSize);
void*           my_malloc_real_debug(unsigned int nSize, const char*, int);
void*           my_realloc(void* pPtr, unsigned int nSize);
void*           my_calloc_real(unsigned int nmemb, unsigned int n_size);
void*           my_calloc_real_debug(unsigned int nmemb, unsigned int n_size, const char*, int);
void            my_free_real(void* pPtr);
void            my_free_real_debug(void* pPtr, const char*, int);
void            my_usleep(int nUsec);
// void            my_printf(const char * format, ...);
//void            LOG_PRINT(const char * format, ...);
#define LOG_PRINT(...)
// #define LOG_PRINT my_printf
myfdesc_ptr     my_open(const char *szpath, unsigned int nflag, unsigned int nmode);
int             my_close(myfdesc_ptr _fd);
int             my_read(myfdesc_ptr fd, void *buf, unsigned int count);
int             my_read_ext(myfdesc_ptr fd, void *buf, unsigned int count);
int             my_write(myfdesc_ptr fd, const void *buf, unsigned int count);
int             my_seek(myfdesc_ptr fd, unsigned int offset, unsigned int whence);
int             my_sync();
mymutex_ptr     my_mutex_init();
void            my_mutex_destroy(mymutex_ptr mtx);
void            my_mutex_lock_real(mymutex_ptr mtx);
void            my_mutex_unlock_real(mymutex_ptr mtx);
void            my_mutex_lock_real_debug(mymutex_ptr mtx, const char*, int);
void            my_mutex_unlock_readl_debug(mymutex_ptr mtx, const char*, int);
int             my_thread_create(mythread_ptr *thread, void *attr, void *(*start_routine) (void *), void *arg);
int             my_thread_create_ext(mythread_ptr *thread, void *attr, void *(*start_routine) (void *), void *arg, char* thd_name, int stack_size, int priority);
int             my_thread_join(mythread_ptr *thread);
int             my_mount(const char *source, const char *target,
                          const char *filesystemtype, unsigned long mountflags,
                          const void *data);
int             my_umount(const char *target);
int             my_i2c_open(int num, myi2cdesc_ptr* pptr);
int             my_i2c_close(myi2cdesc_ptr ptr);
int             my_i2c_write8(myi2cdesc_ptr ptr, unsigned char addr, unsigned char* buf, unsigned int len);
int             my_i2c_read8(myi2cdesc_ptr ptr, unsigned char addr, unsigned char* buf, unsigned int len);

unsigned long long my_get_chip_id();
int             mount_db1();
int             umount_db1();
int             my_mount_userdb();
int             my_mount_userdb_backup();
int             my_mount_misc();
int             my_create_empty_file(const char* path, int file_size);
int             my_print_callstack();

int             rootfs_is_first();
int             rootfs_set_first_flag();
int             rootfs_is_activated();
int             rootfs_set_activated();

void            test_led(int n);
void            test_led2(int n);
void            test_led3(int n);

float           Now(void);
float           GetMonoTime(void);

int             fr_InitAppLog();
int             fr_GetAppLogLen();
int             fr_ReadAppLog(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
int             fr_WriteAppLog(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
int             fr_WriteUSBScanEnableState(void);
//read, write flash pages
unsigned int    my_flash_read(unsigned int u32_bytes_offset, unsigned int u32_limit, void* u32_address, unsigned int u32_size);
unsigned int    my_flash_write_pages(unsigned int u32_bytes_offset, void* u32_address, unsigned int u32_size);
unsigned int    my_flash_write_parts(unsigned int u32_bytes_offset, unsigned int u32_limit, void* u32_address, unsigned int u32_size);
unsigned int    my_flash_erase(unsigned int u32_bytes_offset, unsigned int u32_size);
unsigned int    my_flash_write(unsigned int u32_bytes_offset, void* u32_address, unsigned int u32_size);
int             my_memstat();
int             my_userdb_open(int part_no);
int             my_userdb_read(unsigned int offset, void* buf, unsigned int length);
int             my_userdb_write(unsigned int offset, void* buf, unsigned int length);
int             my_backupdb_read(unsigned int offset, void* buf, unsigned int length);
int             my_backupdb_write(unsigned int offset, void* buf, unsigned int length);
int             my_settings_read(unsigned int offset, void* buf, unsigned int length);
int             my_settings_write(unsigned int offset, void* buf, unsigned int length);
int             dbfs_get_cur_part();
void            dbfs_set_cur_part(int part_no);

int my_msync(void*, int);
int my_munmap(void*, int);

#ifdef UPGRADE_MODE
int fr_ReadFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
int fr_WriteFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
#endif
#define my_printf printf
//#define dbug_printf printf
#define dbug_printf(...)
#define dbug_line dbug_printf("[%s] %s:%d, %0.3f\n", __func__, __FILE__, __LINE__, Now())

#ifdef __cplusplus
}
#endif

#if 0
#define my_malloc(sz) my_malloc_real_debug(sz, __FILE__, __LINE__)
#define my_calloc(n, sz) my_calloc_real_debug(n, sz, __FILE__, __LINE__)
#define my_free(ptr) my_free_real_debug(ptr, __FILE__, __LINE__)
#define my_mutex_lock(a) my_mutex_lock_real_debug(a, __FILE__, __LINE__)
#define my_mutex_unlock(a) my_mutex_unlock_real_debug(a, __FILE__, __LINE__)
#else
#define my_malloc(sz) my_malloc_real(sz)
#define my_calloc(n, sz) my_calloc_real(n, sz)
#define my_free(ptr) my_free_real(ptr)
#define my_mutex_lock(a) my_mutex_lock_real(a)
#define my_mutex_unlock(a) my_mutex_unlock_real(a)
#endif

extern const char*  dbfs_part_names[DB_PART_END+1];
extern SHARED_MEM*  g_pxSharedMem;
extern SHARED_MEM*  g_pxSharedLCD;

#endif //!_COMMON_TYPES_H
