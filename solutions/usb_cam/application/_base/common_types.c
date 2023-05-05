#include "common_types.h"
#include "settings.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <aos/kernel.h>
#include <fcntl.h>
#include <yoc/partition.h>
#include "cvi_sys.h"

mymutex_ptr g_FlashReadWriteLock = 0;
mymutex_ptr g_MyPrintfLock = 0;

const char* dbfs_part_names[DB_PART_END+1] =
{
    "pusr1",
    "pusr2",
    "null"
};

SHARED_MEM* g_pxSharedMem = NULL;
SHARED_MEM* g_pxSharedLCD = NULL;

int g_iShmid;
int g_iShmidLCD;

int bSetKernelFlag = 1;

int my_misc_read(unsigned int offset, void* buf, unsigned int length);
int my_misc_write(unsigned int offset, void* buf, unsigned int length);
int my_wx_read(unsigned int offset, void* buf, unsigned int length);
int my_wx_write(unsigned int offset, void* buf, unsigned int length);
//prebuilt functions

typedef struct {
    char* m_filename;
    int m_filesize;
} st_file_offsize;

st_file_offsize g_part_files[] = {
    {FN_WNO_DICT_PATH, FN_WNO_DICT_SIZE},
    {FN_DETECT_DICT_PATH, FN_DETECT_DICT_SIZE},
    {FN_DLAMK_DICT_PATH, FN_DLAMK_DICT_SIZE},
#if (DESMAN_ENC_MODE == 0)
    {FN_OCC_DICT_PATH, FN_OCC_DICT_SIZE},
    {FN_ESN_DICT_PATH, FN_ESN_DICT_SIZE},
#endif // DESMAN_ENC_MODE
    {FN_A1_DICT_PATH, FN_A1_DICT_SIZE},
    {FN_A2_DICT_PATH, FN_A2_DICT_SIZE},
#if (ENGINE_USE_TWO_CAM)
    {FN_B_DICT_PATH, FN_B_DICT_SIZE},
    {FN_B2_DICT_PATH, FN_B2_DICT_SIZE},
#endif // ENGINE_USE_TWO_CAM
    {FN_C_DICT_PATH, FN_C_DICT_SIZE},
#if (N_MAX_HAND_NUM)
    {FN_DETECT_H_DICT_PATH, FN_DETECT_H_DICT_SIZE},
    {FN_DLAMK_H_DICT_PATH, FN_DLAMK_H_DICT_SIZE},
    {FN_CH_DICT_PATH, FN_CH_DICT_SIZE},
#endif // N_MAX_HAND_NUM
#if (USE_TWIN_ENGINE)
    {FN_H1_DICT_PATH, FN_H1_DICT_SIZE},
    {FN_H2_DICT_PATH, FN_H2_DICT_SIZE},
#endif
    {FN_FACE_IR_BIN_PATH, FN_FACE_IR_BIN_SIZE},
    {NULL, 0},
};

int fr_ReadFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length)
{
    int read_len = -1;
    int file_offset = 0;
    int align_byte = FN_DICT_ALIGN_SIZE;
    int idx = 0;
    file_offset = DICT_START_ADDR;
    while(g_part_files[idx].m_filename != NULL)
    {
        if (!strcmp(g_part_files[idx].m_filename, filename))
            break;
        file_offset += g_part_files[idx].m_filesize;
        file_offset = (file_offset + align_byte - 1) / align_byte * align_byte;
        idx ++;
    }
    if (g_part_files[idx].m_filename)
    {
        //found file
        read_len = my_wx_read(file_offset + u32_offset, buf, u32_length);
    }
    else
    {
        //file not found
        my_printf("file not found: %s\n", filename);
    }
    dbug_printf("[%s] %s, off=%d, %d, %p.\n", __func__, filename, file_offset, read_len, buf);
    return read_len;
}
int fr_WriteFileData(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length)
{
    int read_len = -1;
    int file_offset = 0;
    int align_byte = FN_DICT_ALIGN_SIZE;
    int idx = 0;
    file_offset = DICT_START_ADDR;
    while(g_part_files[idx].m_filename != NULL)
    {
        if (!strcmp(g_part_files[idx].m_filename, filename))
            break;
        file_offset += g_part_files[idx].m_filesize;
        file_offset = (file_offset + align_byte - 1) / align_byte * align_byte;
        idx ++;
    }
    if (g_part_files[idx].m_filename)
    {
        //found file
        read_len = my_wx_write(file_offset + u32_offset, buf, u32_length);
    }
    else
    {
        //file not found
        my_printf("file not found: %s\n", filename);
    }
    return read_len;
}

#if (USE_WIFI_MODULE)
struct spi_slave *spi_dev;
int iInitSpi = 0;
struct spi_slave *my_spi_init(void)
{
    int bus = 0;
    int cs = 0;
    int max_hz = 40000000;
    int mode = 0;//E_MSPI_MODE0;
    int usedma = true;
    struct spi_slave *slave = spi_setup_slave(bus,cs,max_hz,mode,usedma);
    iInitSpi = 1;
    return slave;
}

int my_spi_write(char* tx, int len)
{
    if (!iInitSpi)
    {
        spi_dev = my_spi_init();
        spi_select_pad(spi_dev, PINMUX_FOR_SPI0_MODE_4);
    }
    // slave->pfMspiCbFunc = Mspi_Cb;
    if (spi_xfer_Async(spi_dev, len, tx, NULL) == 0)
        return len;
    else
        return -1;
}

int my_spi_read(unsigned char* rx, int len)
{
    if (!iInitSpi)
    {
        spi_dev = my_spi_init();
        spi_select_pad(spi_dev, PINMUX_FOR_SPI0_MODE_4);
    }
    // slave->pfMspiCbFunc = Mspi_Cb;
    //if (spi_xfer_Async(spi_dev, len, NULL, rx) == 0)
    if (spi_xfer(spi_dev, len, NULL, rx) == 0)
        return len;
    else
        return -1;
}
#endif // USE_WIFI_MODULE

void CreateSharedMem()
{
    if(g_pxSharedMem != NULL)
        return;
#if 0 // kkk test
    g_iShmid = shmget(SHMKEY, sizeof(SHARED_MEM), IPC_CREAT);
    g_pxSharedMem = (SHARED_MEM*)shmat(g_iShmid, NULL, 0);
    if (!g_pxSharedMem)
    {
        my_printf("[ascript]failed to allocate gl_shmdata.\n");
        return;
    }
#endif
    if(g_pxSharedMem->iHeader != 0x55AA)
    {
        g_pxSharedMem->iHeader = 0x55AA;
        g_pxSharedMem->iData0 = 0;
    }

    return;
}

int CreateSharedLCD()
{
    int is_created = 0;
    if(g_pxSharedLCD != NULL)
        return 0;
#ifndef __RTK_OS__
    g_iShmidLCD = shmget(SHMKEY_LCD, sizeof(SHARED_MEM), 0);
    if (g_iShmidLCD < 0)
    {
        g_iShmidLCD = shmget(SHMKEY_LCD, sizeof(SHARED_MEM), IPC_CREAT);
        is_created = 1;
    }

    g_pxSharedLCD = (SHARED_MEM*)shmat(g_iShmidLCD, NULL, 0);
    if (!g_pxSharedLCD)
    {
        my_printf("[ascript]failed to allocate gl_shmdata.\n");
        return 1;
    }
#else // !__RTK_OS__
    if (g_pxSharedLCD == NULL)
    {
        is_created = 1;
        g_pxSharedLCD = (SHARED_MEM*)my_malloc(sizeof(*g_pxSharedLCD));
    }
#endif // !__RTK_OS__
    if(is_created)
    {
        g_pxSharedLCD->iHeader = 0x55AA;
        g_pxSharedLCD->iData0 = 0;
        g_pxSharedLCD->iMountPoints = 0;
    }

    return 0;
}

int my_fsync(myfdesc_ptr fd)
{
    int ret = 0;
#ifndef __RTK_OS__
    do {
        ret = fsync(fd);
    } while(ret == -1 && errno == EINTR);
    if (ret)
    {
        my_printf("my_fsync failed:%d(%s).\n", errno, strerror(errno));
    }
#else // ! __RTK_OS__
#endif // ! __RTK_OS__
    return ret;
}

void set_kernel_flag_action(int b)
{
    bSetKernelFlag = b;
}

int get_kernel_flag_action()
{
    return bSetKernelFlag;
}

int my_system(const char* command)
{
#ifndef __RTK_OS__
    unsigned int cs;
    int ret = -1;
    char str_cmd[64] = {0};
    int i;
    char buf[256];

    for (i = 0; i < 63; i ++)
    {
        if (command[i] == 0 || command[i] == ' ')
            break;
        str_cmd[i] = command[i];
    }

    //my_printf("[%s]trying to open %s.\n", __FUNCTION__, str_cmd);
    FILE* fp = fopen(str_cmd, "r");
    if (fp == NULL)
    {
        for (i = 0; rfs_dir_names[i] != NULL; i++)
        {
            sprintf(buf, "%s/%s", rfs_dir_names[i], str_cmd);
            //my_printf("[%s]trying to open %s.\n", __FUNCTION__, buf);
            fp = fopen(buf, "r");
            if (fp)
                break;
        }
    }

    if (fp)
    {
        fclose(fp);
        ret = get_execfile_checksum_val(str_cmd, &cs);
        if (ret == 0 && get_execfile_checksum(str_cmd) != cs)
        {
            //rootfs partition may be destroyed.
            //do not set kernel flag.
            set_kernel_flag_action(0);
            return -1;
        }
        ret = system(command);
        if (WIFSIGNALED(ret) &&
                (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
        {
            set_kernel_flag_action(0);
        }
    }
    return ret;
#else
    return 0;
#endif
}

void do_backup_exec_checksum()
{
#ifndef __RTK_OS__
    int i;
    FILE* fp;
    unsigned int cs;
    my_system("mount -o remount, rw /");
    fp = fopen(EXEC_CHECKSUM_FILE, "w");
    if (fp)
    {
        for (i = 0; rfs_exec_filenames[i] != NULL; i++)
        {
            cs = get_execfile_checksum(rfs_exec_filenames[i]);
            fwrite(&cs, sizeof(unsigned int), 1, fp);
        }
        my_fsync(fileno(fp));
        fclose(fp);
    }
    else
    {
        my_printf("Failed to create checksum backup.\n");
    }
    my_system("mount -r -o remount /");
#endif // !__RTK_OS__
}
unsigned int get_execfile_checksum(const char* path)
{
#ifndef __RTK_OS__
    FILE* fp = NULL;
    unsigned int i;
    unsigned int len;
    unsigned int ret = 0;
    char buf[256];
    fp = fopen(path, "r");
    if (fp == NULL)
    {
        for (i = 0; rfs_dir_names[i] != NULL; i++)
        {
            sprintf(buf, "%s/%s", rfs_dir_names[i], path);
            fp = fopen(buf, "r");
            if (fp)
                break;
        }
    }
    if (fp)
    {
        i = 0;
        len = fread(buf, 1, 256, fp);
        while(len > 0)
        {
            for (i = 0; i < len; i += 4)
                ret = ret ^ (*((unsigned int*)(buf + i)));
            len = fread(buf, 1, 256, fp);
        }
        fclose(fp);
    }
    return ret;
#endif // !__RTK_OS__
    return 0;
}

/*
* return 0: ok, 1: no result
*/
int get_execfile_checksum_val(const char* path, unsigned int *cs_out)
{
#ifndef __RTK_OS__
    int i;
    for (i = 0; rfs_exec_filenames[i] != NULL; i++ )
    {
        if (!strcmp(path, rfs_exec_filenames[i]))
            break;
    }
    if (rfs_exec_filenames[i] != NULL)
    {
        FILE* fp;
        fp = fopen(EXEC_CHECKSUM_FILE, "r");
        if (fp)
        {
            fseek(fp, i * sizeof(unsigned int), SEEK_SET);
            fread((void*)cs_out, sizeof(unsigned int), 1, fp);
            fclose(fp);
            return 0;
        }
    }
    *cs_out = 0;
#endif // !__RTK_OS__
    return 1;
}

//caution: use this function before activation.
// after activation, detection method is different.
int get_cur_rootfs_part()
{
    int ret = RFS_PART0;
#ifndef __RTK_OS__
#if (!ROOTFS_BAK)
    return ret;
#endif
    char str_cmd[256];
    const char* rfs_names[] = { RFS0_DEVNAME, RFS1_DEVNAME, RFS2_DEVNAME };

    for(int i = RFS_PART0; i < RFS_PART_END; i++)
    {
        sprintf(str_cmd, "df -h %s", rfs_names[i]);
        int a = system(str_cmd);
        if(a == 0)
        {
            ret = i;
            break;
        }
    }
#endif // !__RTK_OS__
    return ret;
}

int get_rootfs_checksum(const char *dev_path, unsigned int* n_checksum_ptr)
{
#ifndef __RTK_OS__
    unsigned int sum = 0;
    int ret = 0;
    unsigned int buf[4096];
    int len_to_read, read_len;
    int total_len = 0;
    FILE* fp_rfs = NULL;
    fp_rfs = fopen(dev_path, "r");
    if (fp_rfs)
    {
        for (int i = 0; i < SEC2BYTES(RFS_SEC_COUNT); i += sizeof(buf))
        {
            len_to_read = ((SEC2BYTES(RFS_SEC_COUNT) - i) < (int)sizeof(buf)) ? (SEC2BYTES(RFS_SEC_COUNT) - i): sizeof(buf);
            read_len = fread(buf, 1, len_to_read, fp_rfs);
            if (read_len == 0)
            {
                //my_printf("failed to read %s:off=%08xh\n", dev_path, i);
                //ret = 1;
                break;
            }
            total_len += read_len;
            for (int j = 0; j < read_len / 4; j ++)
            {
                sum ^= buf[j];
            }
        }
    }
    if (ret == 0)
    {
        *n_checksum_ptr = sum;
        my_printf("%s, len=0x%08x, checksum=0x%08x\n", dev_path, total_len, sum);
    }
    return ret;
#endif // !__RTK_OS__
    return 0;
}

void* my_malloc_real(unsigned int nSize)
{
    return malloc(nSize);
}

void* my_malloc_real_debug(unsigned int nSize, const char* strFile, int nLine)
{
    void* ptrRet = malloc(nSize);
    my_printf("[%s] %p, %d, %s:%d\n", __func__, ptrRet, nSize, strFile, nLine);
    return ptrRet;
}

void* my_calloc_real(unsigned int nmemb, unsigned int n_size)
{
    return calloc(nmemb, n_size);
}

void* my_calloc_real_debug(unsigned int nmemb, unsigned int n_size, const char* strFile, int nLine)
{
    void* ptrRet = calloc(nmemb, n_size);
    my_printf("[%s] %p, %d, %s:%d\n", __func__, ptrRet, nmemb * n_size, strFile, nLine);
    return ptrRet;
}

void* my_realloc(void* pPtr, unsigned int nSize)
{
    return realloc(pPtr, nSize);
}

void my_free_real(void* pPtr)
{
    free(pPtr);
}

void my_free_real_debug(void* pPtr, const char* strFile, int nLine)
{
    my_printf("[%s] %p, %s:%d\n", __func__, pPtr, strFile, nLine);
    free(pPtr);
}

void my_usleep(int nUsec)
{
    aos_msleep((unsigned int)nUsec / 1000);
}

// void my_printf(const char * format, ...)
// {
//     my_mutex_lock(g_MyPrintfLock);
//     char buf[1024] = {0};
//     va_list args;
//     va_start (args, format);
//     vsnprintf(buf, 1023, format, args);
//     va_end (args);
//     printf("%s", buf);
//     my_mutex_unlock(g_MyPrintfLock);
// }

// void LOG_PRINT(const char * format, ...)
// {
// #if 0
//     my_mutex_lock(g_MyPrintfLock);
//     char buf[1024] = {0};
//     va_list args;
//     va_start (args, format);
//     vsnprintf(buf, 1023, format, args);
//     va_end (args);
//     CamOsPrintf(KERN_ERR "%s", buf);
//     my_mutex_unlock(g_MyPrintfLock);
// #endif
// }

float Now(void)
{
    return (1.0* aos_now_ms());
}

float GetMonoTime(void)
{
    return 0;
}

myfdesc_ptr my_open(const char *szpath, unsigned int nflag, unsigned int nmode)
{
    int* fd = (int*)my_malloc(sizeof(int));
    *fd = open(szpath, nflag);
    if (*fd > -1)
        return fd;
    else
    {
        my_free(fd);
        return NULL;
    }
}

int my_close(myfdesc_ptr _fd)
{
    return close(*_fd);
}

int my_read(myfdesc_ptr fd, void *buf, unsigned int count)
{
    dbug_printf("[%s] buf=%p, count=%d\n",
        __func__, buf, count);
    return read(*fd, buf, count);
}

int my_read_ext(myfdesc_ptr fd, void *buf, unsigned int count)
{
    const int alen = 64;
    int remain = count % alen;
    int read_len = count - remain;
    int ret = -1;
    dbug_printf("[%s] buf=%p, count=%d, read_len=%d\n",
        __func__, buf, count, read_len);
    ret = my_read(fd, buf, read_len);
    if (ret == read_len && remain > 0)
    {
        ret += my_read(fd, (void*)((char*)buf + read_len), remain);
    }
    return ret;
}

int my_write(myfdesc_ptr fd, const void *buf, unsigned int count)
{
    return write(*fd, buf, count);
}

int my_seek(myfdesc_ptr fd, unsigned int offset, unsigned int whence)
{
    return lseek(*fd, offset, whence);
}

mymutex_ptr my_mutex_init()
{
    pthread_mutex_t* mt = (pthread_mutex_t*)my_malloc(sizeof(pthread_mutex_t));
    if (mt == NULL)
        return NULL;
    if (pthread_mutex_init(mt, NULL))
    {
        my_free(mt);
        return NULL;
    }
    return (mymutex_ptr)mt;
}

void my_mutex_destroy(mymutex_ptr mtx)
{
    if (mtx == NULL)
        return;
    pthread_mutex_destroy(mtx);
    my_free(mtx);
}

void my_mutex_lock_real(mymutex_ptr mtx)
{
    if (mtx == NULL)
        return;
    pthread_mutex_lock(mtx);
}

void my_mutex_unlock_real(mymutex_ptr mtx)
{
    if (mtx == NULL)
        return;
    pthread_mutex_unlock(mtx);
}

void my_mutex_lock_real_debug(mymutex_ptr mtx, const char* str_file, int n_line)
{
    if (mtx == NULL)
        return;
    my_printf("[%s] %p, %s:%d\n", __func__, mtx, str_file, n_line);
    pthread_mutex_lock(mtx);
}

void my_mutex_unlock_real_debug(mymutex_ptr mtx, const char* str_file, int n_line)
{
    if (mtx == NULL)
        return;
    my_printf("[%s] %p, %s:%d\n", __func__, mtx, str_file, n_line);
    pthread_mutex_unlock(mtx);
}

int my_thread_create(mythread_ptr *thread, void *attr, void *(*start_routine) (void *), void *arg)
{
    if (thread == NULL)
        return -1;
    pthread_t* pThread = my_malloc(sizeof(pthread_t));
    *thread = NULL;
    if (pThread == NULL)
        return -2;
    memset(pThread, 0, sizeof(*pThread));
    //if (attr != NULL)
    {
        if (pthread_create(pThread, attr, start_routine, arg))
        {
            my_free(pThread);
            return -3;
        }
    }

    *thread = pThread;
    return 0;
}

int my_thread_create_ext(mythread_ptr *thread, void *attr, void *(*start_routine) (void *), void *arg, char* thd_name, int stack_size, int priority)
{
    pthread_attr_t a;
    if (thread == NULL)
        return -1;
    pthread_t* pThread = my_malloc(sizeof(pthread_t));
    *thread = NULL;
    if (pThread == NULL)
        return -2;
    memset(pThread, 0, sizeof(*pThread));
    if (attr == NULL)
    {
        pthread_attr_init(&a);
        a.stacksize = stack_size;
        attr = (void*)&a;
    }
    
    //if (attr != NULL)
    {
        if (pthread_create(pThread, attr, start_routine, arg))
        {
            my_free(pThread);
            return -3;
        }
    }
    *thread = pThread;
    return 0;
}

int my_thread_join(mythread_ptr *thread)
{
    if (thread == NULL)
        return -1;
    return pthread_join(*thread, NULL);
}

int my_sync()
{
    //todo
    return 0;
}

int my_mount(const char *source, const char *target,
                          const char *filesystemtype, unsigned long mountflags,
                          const void *data)
{
    //todo
    return 0;
}

int my_umount(const char *target)
{
    //todo
    //return CamFsUnmount(target);
    return 0;
}

int mount_db1()
{
    return 0;
}

int umount_db1()
{
    return 0;
}

int my_mount_userdb()
{
// #ifndef MMAP_MODE
//     my_printf("[%s] start\n", __func__);
//     return CamFsMount(CAM_FS_FMT_FIRMWAREFS, "USERDB1", "/mnt/db");
// #else // !MMAP_MODE
    return 0;
// #endif // !MMAP_MODE
}

int my_mount_userdb_backup()
{
// #ifndef MMAP_MODE
//     return CamFsMount(CAM_FS_FMT_FIRMWAREFS, "USERDB2", "/mnt/backup");
// #else // !MMAP_MODE
    return 0;
// #endif // !MMAP_MODE
}

int my_mount_misc()
{
    // return CamFsMount(CAM_FS_FMT_FIRMWAREFS, "MISC", "/mnt/MISC");
    return 0;
}

unsigned long long my_get_chip_id()
{
    CVI_U8 pu8SN[8] = {0};

    CVI_SYS_GetChipSN(pu8SN, 8);
    unsigned long long ret = *(unsigned long long*)pu8SN;
    my_printf("uuid: %llx\n", ret);
    return ret;
}

int my_print_callstack()
{
    return 0;
}

int my_i2c_open(int num, myi2cdesc_ptr* pptr)
{
    // tI2cHandle* p_handle = NULL;
    // p_handle = my_malloc(sizeof(tI2cHandle));
    // if (p_handle == NULL)
    //     return -1;

    // p_handle->nPortNum = -1;
    // p_handle->pAdapter = NULL;
    // CamI2cOpen(p_handle, num);
    // *pptr = (void*)p_handle;
    return 0;
}

int my_i2c_close(myi2cdesc_ptr ptr)
{
    // CamI2cClose((tI2cHandle*)ptr);
    my_free(ptr);
    return 0;
}

int my_i2c_write8(myi2cdesc_ptr ptr, unsigned char addr, unsigned char* buf, unsigned int len)
{
    //no need to use mutex
    // tI2cMsg msg;
    // if (len == 0)
    //     return -1;
    // //read data
    // msg.addr = addr;
    // msg.flags = 0;
    // msg.buf = buf;
    // msg.len = len;
    // CamI2cTransfer((tI2cHandle*)ptr, &msg, 1);
    // return (int)len;
    return 0;
}

int my_i2c_read8(myi2cdesc_ptr ptr, unsigned char addr, unsigned char* buf, unsigned int len)
{
    //no need to use mutex
    // tI2cMsg msg;
    // if (len == 0)
    //     return -1;
    // //read data
    // msg.addr = addr;
    // msg.flags = I2C_M_RD;
    // msg.buf = buf;
    // msg.len = len;
    // CamI2cTransfer((tI2cHandle*)ptr, &msg, 1);
    // return (int)len;
    return 0;
}

#define MY_PATH_FIRST_FLAG  "/mnt/MISC/first.bin"
#define MY_PATH_ACTD        "/mnt/MISC/actd.bin"

/*
 * @return 0: not first boot, 1: first boot, 2: 3rd rootfs partition but no first flag
*/
int rootfs_is_first()
{
    if (g_xPS.x.bIsFirst != 0xAA)
        return 1;
    else
        return 0;
}

int rootfs_set_first_flag()
{
    g_xPS.x.bIsFirst = 0xAA;
    UpdatePermanenceSettings();
    return 0;
}

void test_led(int n)
{
#if 0
    for (int i = 0; i < n; i++)
    {
        GPIO_fast_setvalue(IR_LED, ON);
        my_usleep(1000);
        GPIO_fast_setvalue(IR_LED, OFF);
        my_usleep(1000);
    }
    my_usleep(5*1000);
#endif
}

void test_led2(int n)
{
#if 0
    for (int i = 0; i < n; i++)
    {
        GPIO_fast_setvalue(GPIO_USBSense, ON);
        my_usleep(1000);
        GPIO_fast_setvalue(GPIO_USBSense, OFF);
        my_usleep(1000);
    }
    my_usleep(5*1000);
#endif
}

void test_led3(int n)
{
#if 0
    for (int i = 0; i < n; i++)
    {
        GPIO_fast_setvalue(GPIO_USBSense, ON);
        my_usleep(1000);
        GPIO_fast_setvalue(GPIO_USBSense, OFF);
        my_usleep(1000);
    }
    my_usleep(5*1000);
#endif
}

int my_create_empty_file(const char* path, int file_size)
{
    myfdesc_ptr fd;
#define WRITE_BUF_LEN 512
    unsigned char buf[WRITE_BUF_LEN] = { 0 };
    if (file_size <= 0)
        return -1;
    fd = my_open(path, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (!is_myfdesc_ptr_valid(fd))
    {
        my_printf("[%s]create file fail.\n", __func__);
        return -1;
    }
    for (int i = 0; i < file_size / WRITE_BUF_LEN; i++)
        my_write(fd, buf, WRITE_BUF_LEN);
    int remain_len = file_size - ((file_size / WRITE_BUF_LEN) * WRITE_BUF_LEN);
    if (remain_len > 0)
        my_write(fd, buf, remain_len);
    my_close(fd);
    my_printf("[%s]create file ok, %d.\n", __func__, file_size);
    return 0;
}

/*
* determine if activated.
* return, 2: not activated
*         1: activated
*/
int rootfs_is_activated()
{
    return 1;
    if (g_xPS.x.bIsActivated == 0xAA)
        return 1;
    else
        return 2;
}

int rootfs_set_activated()
{
    g_xPS.x.bIsActivated = 0xAA;
    UpdatePermanenceSettings();
    return 0;
}

unsigned int my_flash_read(unsigned int u32_bytes_offset, unsigned int u32_limit, void* u32_address, unsigned int u32_size)
{
    int ret = -1;
#if 1
    int read_len = 0;
    int i = 0;
    const int buf_len = 4096;
    int start_off = 0;
    ret = 0;
    for (i = start_off; i < u32_size; i += buf_len)
    {
        if (u32_size - ret < buf_len)
            read_len = u32_size - ret;
        else
            read_len = buf_len;
        my_mutex_lock(g_FlashReadWriteLock);
        //read_len = MDRV_FLASH_read(u32_bytes_offset + i, read_len, u32_address + i, read_len);

        my_mutex_unlock(g_FlashReadWriteLock);
        if (read_len <= 0)
            break;
        ret += read_len;
    }
    // LOG_PRINT("[%s] read=", __func__);
    // for (i = 0; i < 16 && i < u32_size; i ++)
    // {
    //     LOG_PRINT("%02x ", ((unsigned char*)u32_address)[u32_size - i - 1]);
    // }
    // LOG_PRINT("\n");
#else
    ret = MDRV_FLASH_read(u32_bytes_offset, u32_limit, u32_address, u32_size);
#endif
    return ret;
}

unsigned int my_flash_write_pages(unsigned int u32_bytes_offset, void* u32_address, unsigned int u32_size)
{
    // U32 ret;
    // my_mutex_lock(g_FlashReadWriteLock);
    // // ret = MDRV_FLASH_write_pages(u32_bytes_offset, u32_address, u32_size);
    // ret = 0;
    // my_mutex_unlock(g_FlashReadWriteLock);
    // return ret;
    return 0;
}

unsigned int my_flash_write_parts(unsigned int u32_bytes_offset, unsigned int u32_limit, void* u32_address, unsigned int u32_size)
{
    // U32 ret;
    // my_mutex_lock(g_FlashReadWriteLock);
    // // ret = MDRV_FLASH_write_parts(u32_bytes_offset, u32_limit, u32_address, u32_size);
    // ret = 0;
    // my_mutex_unlock(g_FlashReadWriteLock);
    // return ret;
    return 0;
}

unsigned int my_flash_erase(unsigned int u32_bytes_offset, unsigned int u32_size)
{
    // U32 ret;
    // my_mutex_lock(g_FlashReadWriteLock);
    // // ret = MDRV_FLASH_erase(u32_bytes_offset, u32_size);
    // ret = 0;
    // my_mutex_unlock(g_FlashReadWriteLock);
    // return ret;
    return 0;
}

unsigned int my_flash_write(unsigned int u32_bytes_offset, void* u32_address, unsigned int u32_size)
{
    const int flash_page_size = 4*1024;
    unsigned int start_off = u32_bytes_offset - (u32_bytes_offset % flash_page_size);
    unsigned int page_count = (((u32_bytes_offset + u32_size) - start_off) + (flash_page_size - 1)) / flash_page_size;
    unsigned int end_off = start_off + page_count * flash_page_size;
    unsigned char* _tmp_buf;
    unsigned int write_len;
    unsigned int write_off;
    unsigned int total_len = 0;
    LOG_PRINT("[%s] %08x, %p, %08x, p=%d, s=%08x, e=%08x\n", __func__, 
        u32_bytes_offset, u32_address, u32_size, 
        page_count, start_off, end_off);
    _tmp_buf = (unsigned char*)my_malloc(flash_page_size);
    if (_tmp_buf == NULL)
        return 0;
    for (; start_off < end_off; start_off += flash_page_size)
    {
        my_flash_read(start_off, flash_page_size, _tmp_buf, flash_page_size);
        write_len = flash_page_size;
        write_off = 0;
        if (start_off + flash_page_size > u32_bytes_offset + u32_size)
            write_len = u32_bytes_offset + u32_size - start_off;
        if (start_off < u32_bytes_offset)
        {
            write_off = u32_bytes_offset - start_off;
            write_len = write_len - write_off;
        }
        if (memcmp(_tmp_buf + write_off, (void*)(u32_address + total_len), write_len))
        {
            memcpy(_tmp_buf + write_off, (void*)(u32_address + total_len), write_len);
            my_flash_erase(start_off, flash_page_size);
            my_flash_write_parts(start_off, flash_page_size, _tmp_buf, flash_page_size);
        }
        total_len += write_len;
    }
    my_free(_tmp_buf);
    // LOG_PRINT("[%s] write=", __func__);
    // for (int i = 0; i < 16 && i < u32_size; i ++)
    // {
    //     LOG_PRINT("%02x ", ((unsigned char*)u32_address)[u32_size - i - 1]);
    // }
    // LOG_PRINT("\n");
    return total_len;
}

int my_msync(void* addr, int len)
{
    return 0;
}

int my_munmap(void* addr, int len)
{
    return 0;
}

// extern void RtkDumpMemoryStatus(rtk_MemStatusLv_e level);
int my_memstat()
{
    my_printf("[%s] 1\n", __func__);
    // CamOsDirectMemStat();
    // my_printf("[%s] 2\n", __func__);
    // RtkDumpMemoryStatus(RTK_MEM_STATUS_HEAP_USAGE | RTK_MEM_STATUS_POOL_USAGE);
    return 0;
}

int fr_InitAppLog()
{
    int write_len = -1;
    int file_offset = APPLOG_START_ADDR;
    unsigned char wBuf[APPLOG_LEN];

    memset(wBuf, 0, sizeof(wBuf));

    write_len = my_misc_write(file_offset, wBuf, APPLOG_LEN);
    return write_len;
}

int fr_GetAppLogLen()
{
    int read_len = 0;
    int file_offset = 0;
    unsigned char buf[APPLOG_SIZE_LEN];
    file_offset = APPLOG_START_ADDR;
    read_len = my_misc_read(file_offset, buf, APPLOG_SIZE_LEN);
    if (read_len != sizeof(buf))
        return 0;

    read_len = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    if (read_len < 0 || read_len > APPLOG_LEN) read_len = 0;
    return read_len;
}

int fr_ReadAppLog(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length)
{
#if(MY_DICT_FLASH)
    int read_len = -1;
    int file_offset = 0;
    file_offset = APPLOG_START_ADDR;
    if (strstr(filename, "app_log.txt"))
        goto off_read_file;

    //invalid dict file.
    return -1;

off_read_file:
    read_len = my_misc_read(file_offset + u32_offset, buf, u32_length);
    return read_len;
#else // MY_DICT_FLASH
    int ret = -1;
    myfdesc_ptr f;
    f = my_open(filename, O_RDONLY, 0777);
    if (is_myfdesc_ptr_valid(f))
    {
        ret = my_read_ext(f, buf, u32_length);
        my_close(f);
    }
    return ret;
#endif // MY_DICT_FLASH
}

int fr_WriteAppLog(const char* filename, unsigned int u32_offset, void* buf, unsigned int u32_length)
{
#if(MY_DICT_FLASH)
    int write_len = -1;
    int file_offset = 0;
    int read_len = -1;
    unsigned char wBuf[APPLOG_LEN];
    unsigned int nWriteOffset = 0;
    unsigned int nUpdatedWriteOffset = 0;

    file_offset = APPLOG_START_ADDR;
    if (strstr(filename, "app_log.txt"))
    {
        read_len = fr_ReadAppLog(filename, u32_offset, wBuf, sizeof(wBuf));
        if (read_len != sizeof(wBuf))
            return -1;

        nWriteOffset = wBuf[0] << 24 | wBuf[1] << 16 | wBuf[2] << 8 | wBuf[3];
        if (nWriteOffset < 0 || nWriteOffset > APPLOG_LEN)
        {
            my_printf("@@@@@@ miss len = %d\n", nWriteOffset);
            nWriteOffset = 0;
        }
        nUpdatedWriteOffset = nWriteOffset + u32_length;

        if (nUpdatedWriteOffset >= (APPLOG_LEN - 4))
        {
            my_printf("####### miss len = %d\n", nUpdatedWriteOffset);
            nWriteOffset = 0;
            nUpdatedWriteOffset = u32_length;
        }
        wBuf[0] = (nUpdatedWriteOffset >> 24) & 0xFF;
        wBuf[1] = (nUpdatedWriteOffset >> 16) & 0xFF;
        wBuf[2] = (nUpdatedWriteOffset >> 8) & 0xFF;
        wBuf[3] = nUpdatedWriteOffset & 0xFF;

        memcpy(wBuf + nWriteOffset + APPLOG_SIZE_LEN, buf, u32_length);
        goto off_write_file;
    }

    //invalid log file.
    return -1;

off_write_file:
    write_len = my_misc_write(file_offset + u32_offset, wBuf, APPLOG_LEN);
    return write_len;
#else // MY_DICT_FLASH
    int ret = -1;
    myfdesc_ptr f;
    f = my_open(filename, O_RDONLY, 0777);
    if (is_myfdesc_ptr_valid(f))
    {
        ret = my_read_ext(f, buf, u32_length);
        my_close(f);
    }
    return ret;
#endif // MY_DICT_FLASH
}

int fr_WriteUSBScanEnableState()
{
#if(MY_DICT_FLASH)
    int write_len = -1;
    unsigned char buf[UPGRADER_INFO_SIZE * 2];

    my_flash_read(UPGRADER_INFO_ADDR, sizeof(buf), buf, sizeof(buf));
    
    buf[UPGRADER_INFO_SIZE] = 0x7E;
    buf[UPGRADER_INFO_SIZE + 1] = 0x55;
    buf[UPGRADER_INFO_SIZE + 2] = 0xCC;
    buf[UPGRADER_INFO_SIZE + 3] = 0xDD;

    goto off_write_file;

    //invalid log file.
    return -1;

off_write_file:
    write_len = my_flash_write(UPGRADER_INFO_ADDR, buf, sizeof(buf));
    return write_len;
#else // MY_DICT_FLASH
    int ret = -1;
    myfdesc_ptr f;
    f = my_open(filename, O_RDONLY, 0777);
    if (is_myfdesc_ptr_valid(f))
    {
        ret = my_read_ext(f, buf, u32_length);
        my_close(f);
    }
    return ret;
#endif // MY_DICT_FLASH
}

int g_userdb_cur = DB_PART1;

int my_userdb_open(int part_no)
{
    my_mutex_lock(g_FlashReadWriteLock);
    g_userdb_cur = part_no;
    my_mutex_unlock(g_FlashReadWriteLock);
    return 0;
}

int my_flash_part_read(const char* part_name, unsigned int offset, void* buf, unsigned int length)
{
    int ret;
    my_mutex_lock(g_FlashReadWriteLock);
    partition_t partition = partition_open(part_name);
    if (partition < 0)
    {
        my_printf("[%s]part open fail: %s\n", __func__, part_name);
        return 0;
    }
    ret = partition_read(partition, offset, buf, length);
    if (ret)
    {
        my_printf("[%s]part read fail: %s\n", __func__, part_name);
        return 0;
    }
    partition_close(partition);
    my_mutex_unlock(g_FlashReadWriteLock);
    if (ret)
        return 0;
    else
        return length;
}

int my_flash_part_write(const char* part_name, unsigned int offset, void* buf, unsigned int length)
{
    my_mutex_lock(g_FlashReadWriteLock);
    partition_t partition = partition_open(part_name);
    if (partition < 0)
    {
        my_printf("[%s]part open fail: %s\n", __func__, part_name);
        return 0;
    }

    const int flash_page_size = 4*1024;
    unsigned int start_off = offset - (offset % flash_page_size);
    unsigned int page_count = (((offset + length) - start_off) + (flash_page_size - 1)) / flash_page_size;
    unsigned int end_off = start_off + page_count * flash_page_size;
    unsigned char* _tmp_buf;
    unsigned int write_len;
    unsigned int write_off;
    unsigned int total_len = 0;
    LOG_PRINT("[%s] %08x, %p, %08x, p=%d, s=%08x, e=%08x\n", __func__, 
        offset, buf, length, 
        page_count, start_off, end_off);
    _tmp_buf = (unsigned char*)my_malloc(flash_page_size);
    if (_tmp_buf == NULL)
        return 0;
    for (; start_off < end_off; start_off += flash_page_size)
    {
        partition_read(partition, start_off, _tmp_buf, flash_page_size);
        write_len = flash_page_size;
        write_off = 0;
        if (start_off + flash_page_size > offset + length)
            write_len = offset + length - start_off;
        if (start_off < offset)
        {
            write_off = offset - start_off;
            write_len = write_len - write_off;
        }
        if (memcmp(_tmp_buf + write_off, (void*)((char*)buf + total_len), write_len))
        {
            memcpy(_tmp_buf + write_off, (void*)((char*)buf + total_len), write_len);
            partition_erase_size(partition, start_off, flash_page_size);
            partition_write(partition, start_off, _tmp_buf, flash_page_size);
        }
        total_len += write_len;
    }
    my_free(_tmp_buf);
    partition_close(partition);
    my_mutex_unlock(g_FlashReadWriteLock);
    return total_len;
}

int my_userdb_read_real(int p_type, unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_read(dbfs_part_names[p_type], offset, buf, length);
}

int my_userdb_write_real(int p_type, unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_write(dbfs_part_names[p_type], offset, buf, length);
}

int my_userdb_read(unsigned int offset, void* buf, unsigned int length)
{
    return my_userdb_read_real(g_userdb_cur, offset, buf, length);
}

int my_userdb_write(unsigned int offset, void* buf, unsigned int length)
{
    return my_userdb_write_real(g_userdb_cur, offset, buf, length);
}

int my_userdb_close()
{
    return 0;
}

int my_backupdb_read(unsigned int offset, void* buf, unsigned int length)
{
    return my_userdb_read_real(DB_PART_BACKUP, offset, buf, length);
}

int my_backupdb_write(unsigned int offset, void* buf, unsigned int length)
{
    return my_userdb_write_real(DB_PART_BACKUP, offset, buf, length);
}

int my_settings_read(unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_read(MY_PART_SETT, offset, buf, length);
}

int my_settings_write(unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_write(MY_PART_SETT, offset, buf, length);
}

int my_misc_read(unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_read(MY_PART_MISC, offset, buf, length);
}

int my_misc_write(unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_write(MY_PART_MISC, offset, buf, length);
}

int my_wx_read(unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_read(MY_PART_WX, offset, buf, length);
}

int my_wx_write(unsigned int offset, void* buf, unsigned int length)
{
    return my_flash_part_write(MY_PART_WX, offset, buf, length);
}

int dbfs_get_cur_part()
{
    my_printf("[%s]mount_point=%d,\n", __FUNCTION__, g_pxSharedLCD->iMountPoints);
    return g_pxSharedLCD->iMountPoints;
}

void dbfs_set_cur_part(int part_no)
{
    if (part_no >= DB_PART1 && part_no <= DB_PART_END)
        g_pxSharedLCD->iMountPoints = part_no;
}
