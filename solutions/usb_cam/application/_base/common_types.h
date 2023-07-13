#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

#include "appdef.h"
#include "engine_inner_param.h"
#include "config_vars.h"
#include <pthread.h>
#include <stdio.h>

#define MY_LE2INT4(ba) ((((unsigned char*)(ba))[3] << 24) | (((unsigned char*)(ba))[2] << 16) | (((unsigned char*)(ba))[1] << 8) | ((unsigned char*)(ba))[0])
#define MY_LE2INT2(ba) ((((unsigned char*)(ba))[1] << 8) | ((unsigned char*)(ba))[0])
#define MY_BE2INT4(ba) ((((unsigned char*)(ba))[0] << 24) | (((unsigned char*)(ba))[1] << 16) | (((unsigned char*)(ba))[2] << 8) | ((unsigned char*)(ba))[3])
#define MY_BE2INT2(ba) ((((unsigned char*)(ba))[0] << 8) | ((unsigned char*)(ba))[1])
#define MY_BE2DBYTES4(v, ba) \
    do { \
        ((unsigned char*)(ba))[0] = ((v) >> 24) & 0xff; \
        ((unsigned char*)(ba))[1] = ((v) >> 16) & 0xff; \
        ((unsigned char*)(ba))[2] = ((v) >> 8) & 0xff; \
        ((unsigned char*)(ba))[3] = (v) & 0xff; \
    } while(0)
#define MY_BE2DBYTES2(v, ba) \
    do { \
        ((unsigned char*)(ba))[0] = ((v) >> 8) & 0xff; \
        ((unsigned char*)(ba))[1] = (v) & 0xff; \
    } while(0)
#define MY_LE2DBYTES4(v, ba) \
    do { \
        ((unsigned char*)(ba))[3] = ((v) >> 24) & 0xff; \
        ((unsigned char*)(ba))[2] = ((v) >> 16) & 0xff; \
        ((unsigned char*)(ba))[1] = ((v) >> 8) & 0xff; \
        ((unsigned char*)(ba))[0] = (v) & 0xff; \
    } while(0)
#define MY_LE2DBYTES2(v, ba) \
    do { \
        ((unsigned char*)(ba))[1] = ((v) >> 8) & 0xff; \
        ((unsigned char*)(ba))[0] = (v) & 0xff; \
    } while(0)

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

#define MYTHREAD_PRIORITY_VERY_LOW      0
#define MYTHREAD_PRIORITY_LOW           0
#define MYTHREAD_PRIORITY_MEDIUM        0
#define MYTHREAD_PRIORITY_HIGH          0
#define MYTHREAD_PRIORITY_VERY_HIGH     36

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
int             xor_encrypt(unsigned char* buf, int length, unsigned char* key, int key_length);

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
void            my_mutex_unlock_real_debug(mymutex_ptr mtx, const char*, int);
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
int             rootfs_set_activated(int flag, int is_sync);

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
int             my_flash_part_read(const char* part_name, unsigned int offset, void* buf, unsigned int length);
int             my_flash_part_write(const char* part_name, unsigned int offset, void* buf, unsigned int length);
unsigned int    my_flash_read(unsigned int u32_bytes_offset, unsigned int u32_limit, void* u32_address, unsigned int u32_size);
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
int32_t         cli_handle_input(char *inbuf);

int my_msync(void*, int);
int my_munmap(void*, int);

int fr_ReadFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
int fr_WriteFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length);
// #define my_printf printf
#define my_printf(...)
// #define dbug_printf printf
#define dbug_printf(...)
#define dbug_line dbug_printf("[%s] %s:%d, %0.3f\n", __func__, __FILE__, __LINE__, Now())

#ifdef __cplusplus
}
#endif

#if 0
#define my_malloc(sz) my_malloc_real_debug(sz, __FILE__, __LINE__)
#define my_calloc(n, sz) my_calloc_real_debug(n, sz, __FILE__, __LINE__)
#define my_free(ptr) my_free_real_debug(ptr, __FILE__, __LINE__)
// #define my_mutex_lock(a) my_mutex_lock_real_debug(a, __FILE__, __LINE__)
// #define my_mutex_unlock(a) my_mutex_unlock_real_debug(a, __FILE__, __LINE__)
#else
#define my_malloc(sz) my_malloc_real(sz)
#define my_calloc(n, sz) my_calloc_real(n, sz)
#define my_free(ptr) my_free_real(ptr)
#endif
#define my_mutex_lock(a) my_mutex_lock_real(a)
#define my_mutex_unlock(a) my_mutex_unlock_real(a)

#define my_memcpy           memcpy
#define my_memset           memset

extern const char*  dbfs_part_names[DB_PART_END+1];
extern SHARED_MEM*  g_pxSharedMem;
extern SHARED_MEM*  g_pxSharedLCD;

#endif //!_COMMON_TYPES_H
