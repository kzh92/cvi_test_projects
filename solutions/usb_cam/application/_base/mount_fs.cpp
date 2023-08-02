#include "mount_fs.h"
#include "appdef.h"
#include "EngineDef.h"
#include "i2cbase.h"
#include "DBManager.h"
//#include "lcdtask.h"
#include "shared.h"
#include "upgradebase.h"
//#include "facemodulecmd.h"
#include "facemoduletask.h"
#include "common_types.h"

// #include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <unistd.h>
// #include <sys/mount.h>
// #include <errno.h>
#include <fcntl.h>
// #include <execinfo.h>
// #include <signal.h>

extern FaceModuleTask* g_pFMTask;
int ss_values[10] = {0};

int g_lcdInited = 0;

const char* dbfs_get_part_name_by_id(int id)
{
    if (id >= DB_PART1 && id <= DB_PART_END)
        return dbfs_part_names[id];
    else
        return NULL;
}

const char* dbfs_get_cur_part_name()
{
    if (dbfs_get_cur_part() >= DB_PART1 && dbfs_get_cur_part() <= DB_PART_END)
        return dbfs_part_names[dbfs_get_cur_part()];
    else
        return NULL;
}

int do_make_ext4(const char* dev_path)
{
#if (DB_TYPE != TYPE_EXT4)
    return 0;
#endif
    char szCmd[256];

    sprintf(szCmd, "umount -l -f %s", dev_path);
    if (my_system(szCmd) == -1)
    {
        my_printf("[%s]failed to umount %s.\n", __FUNCTION__, dev_path);
        return 1;
    }
    my_usleep(3000*1000);

    my_system("fdisk -l");
    sprintf(szCmd, "dd if=/dev/zero of=%s", dev_path);
    my_system(szCmd);

    sprintf(szCmd, "/usr/bin/mke2fs1 -F -t ext4 %s", dev_path);
    my_system(szCmd);

//    sprintf(szCmd, "tune2fs -O ^huge_file %s", dev_path);
//    my_system(szCmd);

    my_usleep(50 * 1000);
    sprintf(szCmd, "e2fsck -y %s", dev_path);
    my_system(szCmd);

    return 0;
}

int try_mount_dbfs()
{
    return my_mount_userdb();
    my_printf("mountstatus=%d, mountretry=%d, point=%d,\n",
           g_xROKLog.x.bMountStatus, g_xROKLog.x.bMountRetry, g_xROKLog.x.bMountPoint);
    int iMountPoint = g_xROKLog.x.bMountPoint;
    if (g_xROKLog.x.bMountStatus != 0xAA)
    {
#if (DB_TYPE == TYPE_EXT4)
        if (g_xROKLog.x.bMountRetry < MOUNT_RETRY_COUNT)
#else
        if (g_xROKLog.x.bMountRetry < MOUNT_RETRY_COUNT)
#endif
        {
            g_xROKLog.x.bMountRetry++;
        }
        else
        {
            iMountPoint++;
            g_xROKLog.x.bMountRetry = 0;
            g_xSS.idbPartNoRestore = 1;
        }
        if (iMountPoint > DB_PART_BACKUP)
            iMountPoint = DB_PART_BACKUP;
#if (DB_TYPE == TYPE_EXT4)
        if (g_xROKLog.x.bMountPoint < DB_PART_BACKUP)
        {
            if (g_xROKLog.x.bMountRetry >= MOUNT_RETRY_COUNT)
            {
                UpdateROKLogs();
                do_make_ext4(dbfs_get_part_name_by_id(g_xROKLog.x.bMountPoint));
            }
        }
#endif
    }
    else
        g_xROKLog.x.bMountRetry = 0;

    //mark as starting.
    //my_printf("mount mark start.\n");
    if (iMountPoint != DB_PART_BACKUP)
        g_xROKLog.x.bMountPoint = iMountPoint;
    SetMountStatus(0);

#ifndef __RTK_OS__
    return mount_dbfs(iMountPoint);
#else // !__RTK_OS__
    g_pxSharedLCD->iMountPoints = iMountPoint;
    return my_mount_userdb();
#endif // !__RTK_OS__
}

#ifndef __RTK_OS__
/**
 * @brief mount_dbfs
 * @param type
 * @return  정상일때 0을 돌린다.
            새로 파티션을 창조하였으면 -2를, 파티션검사에서 500ms이상 시간이 지연됬으면 -3을 돌린다.
            파티션Mount에서 실패하였으면 -1을 돌린다.
 */
int mount_dbfs(int part_start)
{
    int ret = -1;
    int i;

    int type = g_xHD2.x.bMountFlag;

    for (i = part_start; i < DB_PART_BACKUP; i ++)
    {
        dbfs_set_cur_part(i);
        my_printf("[Main] ------ Mounting DB_PART%d Type = %d\n", i,  type);

        ret = mount_dbfs_sub(type);
        if(g_xHD2.x.bMountFlag == 1)
        {
            g_xHD2.x.bMountFlag = 0;
            UpdateHeadInfos2();
        }

        if(ret == -1)   //DB파티션 Mount에서 실패하였을때
        {
            continue;
        }
        else if(ret == -2)  //파티션Mount에서 실패하여 새로 파티션을 Mount하였을때
        {
            LOG_PRINT("[Main] ------ ret == %d : Mounting %s partition again\n", ret, dbfs_get_cur_part_name());
            //IncreaseSystemRunningCount();
            g_xHD2.x.bMountFlag = 1;
            UpdateHeadInfos2();
            break;
        }
        else if(ret == -3)  //파티션 Check시간이 500ms이상 걸렸을때
        {
            LOG_PRINT("[Main] ------ ret == %d : %s Checking time is more than 500ms. \n", ret, dbfs_get_cur_part_name());
            g_xHD2.x.bMountFlag = 1;
            UpdateHeadInfos2();
            break;
        }
        else if (ret == 0)
            break;
    }

    if (ret == -1) //failed to mount.
    {
        if (mount_backupdbfs() != 0)
        {
            ret = 1;//poweroff
        }
    }
    return ret;
}


int mount_dbfs_sub(int type)
{
    int i;
    int iMakeFlag = 0;
    int ret = 0;
    const char* dev_path;
    dev_path = dbfs_get_cur_part_name();

#if (DB_TYPE == TYPE_EXT4)
    for (i = 0; i < 3; i++)
#elif (DB_TYPE == TYPE_JFFS)
    for (i = 0; i < 1; i++)
#endif
    {
        my_printf("========mount_dbfs: %s, i = %d\n", dev_path, i);

        ret = my_mount(dev_path, "/db", DB_FSTYPE, MS_NOATIME, "");
        if (ret == 0)
        {
            my_printf("[Mount_FS] mount success\n");

            my_system("touch /db/test_db");

            FILE* fp = fopen("/db/test_db", "wb");
            if(fp == NULL)
            {
                ret = -1;
                my_printf("[Mount_FS] test write fail.\n");
            }
            else
            {
                my_printf("[Mount_FS] test write ok.\n");
                fclose(fp);
            }
        }
        else
            my_printf("[Mount_FS] mount fail\n");

        if (ret == 0)
            break;

        if (ret)
        {
            if (!g_lcdInited)
            {
                g_lcdInited = 1;
            }
            if (i == 0)
            {
                if (fsck_dbfs_part(dbfs_get_cur_part(), type) == 0)
                    iMakeFlag = 2;//delay more than 500ms
            }
            else if (i == 1)
            {
                do_make_ext4(dev_path);
                iMakeFlag = 1;
            }
        }
    }

    if (ret == 0 && iMakeFlag == 1)
        return -2;
    if (ret == 0 && iMakeFlag == 2)
        return -3;

    return ret;
}
#endif // !__RTK_OS__

int umount_dbfs()
{
#ifndef __RTK_OS__
    //my_printf("[%s] before umount /db.\n", __FUNCTION__);
    if (my_system("umount -l -f /db") == -1) {
        my_printf("cannot umount %s\n", "/db");
        return -1;
    }
    my_sync();
#else // !__RTK_OS__
    my_umount("/mnt/db");
#endif // !__RTK_OS__
    return 0;
}

int backup_dbfs(int iBlkNum)
{
#ifndef __RTK_OS__
    int iRet = 0;
#ifdef MMAP_MODE
    users_backup[usercount_backup] = iBlkNum;
    usercount_backup++;
    return  iRet;
#else //MMAP_MODE
    int a;
    float oldTime;
    char szCmd[256];

    //my_printf("[%s] start\n", __FUNCTION__);

    //backup partition was used for data partition.
    if (dbfs_get_cur_part() >= DB_PART_BACKUP)
        return 0;

    oldTime = Now();
    fsck_dbfs_part(DB_PART_BACKUP);

    sprintf(szCmd, "mount %s /backup", dbfs_get_part_name_by_id(DB_PART_BACKUP));

    if (my_system(szCmd) == -1)
    {
        my_printf("cannot mount for backup:%s\n", dbfs_get_part_name_by_id(DB_PART_BACKUP));
        iRet -= 1;
    }
    else
    {
        my_system("rm -rf /backup/lost+found");
        if(iBlkNum == -1)
        {
            my_system("rm -f /backup/*");
            //kzh test
            //my_printf("files in /db\n");
            //my_system("ls /db");
            a = my_system("cp -f /db/userdb*.bin /backup/");

            if (WIFEXITED(a))
            {
                int rc;
                rc = WEXITSTATUS(a);
                my_printf("Exit with status1: %d\n", rc);

                if(rc != 0)
                {
                    CheckFileSystem(1);

                    my_system("rm -f /backup/*");
                    my_system("rm -rf /backup/lost+found");
                    my_system("cp -f /db/userdb*.bin /backup/");
                }
            }
        }
        else
        {
            sprintf(szCmd, "rm -f /backup/%s%05d.bin", PREFIX_USER_DB_NAME, iBlkNum);
            my_system(szCmd);

            sprintf(szCmd, "cp -f /db/%s%05d.bin /backup/", PREFIX_USER_DB_NAME, iBlkNum);
            a = my_system(szCmd);

            if (WIFEXITED(a))
            {
                int rc;
                rc = WEXITSTATUS(a);
                my_printf("Exit with status2: %d\n", rc);

                if(rc != 0)
                {
                    CheckFileSystem(1);

                    my_system("rm -rf /backup/lost+found");

                    sprintf(szCmd, "cp -f /db/%s%05d.bin /backup/", PREFIX_USER_DB_NAME, iBlkNum);
                    a = my_system(szCmd);
                }
            }
        }
        my_sync();
        //kzh test
        //my_printf("files in /backup\n");
        //my_system("ls /backup");
        my_system("umount -l -f /backup");
        my_printf("backup1 success!\n");
        my_usleep(50 * 1000);
    }

    return iRet;
#endif
#else // ! __RTK_OS__
#endif // !__RTK_OS__
    return 0;
}

int delete_dbfs()
{
    int ret = 0;
    char szCmd[256];

    if (dbfs_get_cur_part() == DB_PART_BACKUP)
    {
        my_system("mount -o remount, rw /db");
        my_system("rm -rf /db/*");
        my_system("mount -o remount, ro /db");
        my_usleep(1000*1000);
    }
    else
    {
        sprintf(szCmd, "mount %s /backup", dbfs_get_part_name_by_id(DB_PART_BACKUP));

        if (my_system(szCmd) == -1)
        {
            fprintf(stderr, "cannot mount for delete %s\n", dbfs_get_part_name_by_id(DB_PART_BACKUP));
            ret -= 1;
        }
        else
        {
            my_system("rm -rf /backup/*");
            my_system("umount -l -f /backup");
            my_usleep(1000*1000);
        }
    }

    return ret;
}

/*
 * part_id: DB_PARTxx
 * opt: -p or -y, 0: -y, 1: -p
*/

int fsck_dbfs_part(int part_id, int opt)
{
#if (DB_TYPE != TYPE_EXT4)
    return 1;
#endif

    const char* dev_path = dbfs_get_part_name_by_id(part_id);
    char szCmd[256];
#if 0
    int count = 0;

    //my_printf("[%s] start,%s.\n", __FUNCTION__, dev_path);
    umount2(dev_path, MNT_DETACH | MNT_FORCE | MNT_EXPIRE);
    while (umount2(dev_path, MNT_DETACH | MNT_FORCE | MNT_EXPIRE) != 0)
    {
        count ++;
        if (errno == EINVAL)
        {
            //not mounted?
            break;
        }
        else if (errno != EBUSY && errno != EAGAIN)
        {
            my_printf("[%s]failed to umount %s(%d:%s).\n", __FUNCTION__,
                   dev_path, errno, strerror(errno));
            return 0;
        }
        my_usleep(500*1000);
        if (count > 10)
        {
            my_printf("[%s]failed to try umount %s(%d:%s).\n", __FUNCTION__,
                   dev_path, errno, strerror(errno));
            return 0;
        }
    }
#else
    //my_printf("[%s]trying to umount %s\n", __FUNCTION__, dev_path);
    sprintf(szCmd, "umount -l -f %s", dev_path);
    my_system(szCmd);
    //my_printf("[%s]ended umount %s\n", __FUNCTION__, dev_path);
#endif

    float rOld = Now();

    //my_printf("[%s] trying to check device %s...\n", __FUNCTION__, dev_path);
    sprintf(szCmd, "e2fsck %s %s", (opt == 0? "-y": "-p"), dev_path);
    my_system(szCmd);
    //my_printf("[%s] check device %s ended.\n", __FUNCTION__, dev_path);

    sprintf(szCmd, "tune2fs -j -f %s", dev_path);
    my_system(szCmd);

    if(Now() - rOld > 500)
    {
        my_printf("[FSCheck] timeout : %f\n", Now() - rOld);
        return 0;
    }

    return 1;
}

int restore_dbfs()
{
#if 0//kkk
    if(dbfs_get_cur_part() >= DB_PART_BACKUP)
        return 0;

    my_printf("restoring dbfs...\n");

    int ret = 0;

    my_system("rm -rf /db/lost+found");
    my_system("rm -rf /db/user*.bin");

    int i;
    int mount_points = -1;

    for (i = 0; i < 2; i++)
    {
        if (mount(dbfs_get_part_name_by_id(DB_PART_BACKUP), RESTORE_SRC, DB_FSTYPE, MS_RDONLY | MS_NOATIME, "") == 0)
        {
            mount_points = 0;
            break;
        }
        else
        {
            fprintf(stderr, "%d:cannot mount %s\n", i, dbfs_get_part_name_by_id(DB_PART_BACKUP));
            double oldTime = Now();
            fsck_dbfs_part(DB_PART_BACKUP);
            LOG_PRINT("nandd3 check time = %f\n", Now() - oldTime);
        }
    }

    if(mount_points == 0)
    {
        char file_path[1024];
        int fd_src, fd_dst;
        int read_len;

        my_system("rm -rf /db/lost+found");

        {
            sprintf(file_path, MNT_USERINFO_DAT);
            fd_src = open(file_path, O_RDONLY);
            if (fd_src < 0)
            {
                my_printf("[%s]cannot open /mnt/userinfo.bin\n", __FUNCTION__);
                char szCmd[256] = {0};
                sprintf(szCmd, "ls %s", RESTORE_SRC);
                my_system(szCmd);
                ret = 2;
                goto exit1;
            }
            sprintf(file_path, DB_USERINFO_DAT);
            fd_dst = open(file_path, O_WRONLY | O_CREAT);
            if (fd_dst < 0)
            {
                my_printf("[%s]cannot open /db/userinfo.bin\n", __FUNCTION__);
                my_system("ls /db");
                ret = 2;
                close(fd_src);
                goto exit1;
            }
            while(1)
            {
                read_len = read(fd_src, file_path, 1024);
                if (read_len > 0)
                {
                    if (write(fd_dst, file_path, read_len) != read_len)
                    {
                        ret = 1;
                        break;
                    }
                }
                else //end of file
                    break;
            }
            close(fd_src);
            my_fsync(fd_dst);
            close(fd_dst);
        }
exit1:
        umount2(RESTORE_SRC, MNT_DETACH | MNT_FORCE);
    }
    else
        ret = 1;

    my_printf("restore dbfs done.\n");

    return ret;
#endif
    return 0;
}

int mount_backupdbfs()
{
#if 0 //kkk
    int i;

    my_printf("Using backup partition for db...\n");

    for (i = 0; i < 2; i ++)
    {
        if (my_mount(dbfs_get_part_name_by_id(DB_PART_BACKUP), "/db", DB_FSTYPE, MS_RDONLY | MS_NOATIME, ""))
        {
            fsck_dbfs_part(DB_PART_BACKUP);
        }
        else
            break;
    }
    if (i >= 2)
        return 1;

    my_printf("backupdbfs: nUserCount=%d.\n", g_xCS.x.bUserCount);

    dbfs_set_cur_part(DB_PART_BACKUP);
#endif
    return 0;
}


int mount_backup_db(int iFlag)
{
#ifndef __RTK_OS__
    if (dbfs_get_cur_part() >= DB_PART_BACKUP)
        return -1;

    fsck_dbfs_part(DB_PART_BACKUP);

    if (mount(dbfs_get_part_name_by_id(DB_PART_BACKUP), "/backup", DB_FSTYPE, iFlag, "") == -1)
    {
        fprintf(stderr, "cannot mount for backup %s\n", "/dev/mmcblk0p3");
        return -1;
    }
#else // !__RTK_OS__
    my_mount_userdb_backup();
#endif // !__RTK_OS__
    return 0;
}

int umount_backup_db()
{
#ifndef __RTK_OS__
    my_sync();
    my_umount("/backup");
    my_usleep(50 * 1000);
#else // !__RTK_OS__
    my_umount("/mnt/backup");
#endif // !__RTK_OS__

    return 0;
}

void do_backup_rootfs()
{
#if (ROOTFS_BAK)
    int req_backup = 0;
    FILE* fp = NULL;
    char str_cmd[512];
    fp = fopen("/test/rootfs.bak", "r");
    if (fp == NULL)
    {
        sprintf(str_cmd, "mount %s /home/default", RFS0_DEVNAME);
        my_system(str_cmd);
        fp = fopen("/home/default/test/rootfs.bak", "r");
        if (fp == NULL)
        {
            req_backup = 1;
        }
        else
            fclose(fp);
        my_system("umount /home/default");
    }
    else
        fclose(fp);
    if (req_backup)
    {
        do_backup_exec_checksum();

        //copy files to rootfs backup partitions
        my_printf("Doing backup operations...\n");
        sprintf(str_cmd, "mount %s /home/default", RFS0_DEVNAME);
        my_system(str_cmd);
        my_system("touch /home/default/test/first");
        my_system("mv -f /home/default/etc/init.d/rcS_release /home/default/etc/init.d/rcS");
        my_system("rm -f /home/default/etc/init.d/rcS_act");
        my_system("cp -f /usr/lib/libfaceengine.so.1.0.0 /home/default/usr/lib/");
        my_system("cp -f /test/spoof.bin /home/default/test/");
        //my_system("cp -f /test/spoof.param.bin /home/default/test/");
        my_system("cp -f /test/wno.bin /home/default/test/");
        my_system("cp -f /test/hdic_1.bin /home/default/test/");
        //my_system("cp -f /test/detect.bin /home/default/test/");
        sprintf(str_cmd, "cp -f %s /home/default%s", EXEC_CHECKSUM_FILE, EXEC_CHECKSUM_FILE);
        my_system(str_cmd);
        my_system("touch /home/default/test/rootfs.bak");
        my_system("umount /home/default");

        sprintf(str_cmd, "mount %s /home/default", RFS2_DEVNAME);
        my_system(str_cmd);
        fp = fopen("/home/default/etc/init.d/rcS", "r");
        if (fp)
        {
            fclose(fp);
            my_printf("doing indivisual copy...\n");

            my_system("touch /home/default/test/first");
            my_system("mv -f /home/default/etc/init.d/rcS_release /home/default/etc/init.d/rcS");
            my_system("rm -f /home/default/etc/init.d/rcS_act");
            my_system("cp -f /usr/lib/libfaceengine.so.1.0.0 /home/default/usr/lib/");
            my_system("cp -f /test/spoof.bin /home/default/test/");
            //my_system("cp -f /test/spoof.param.bin /home/default/test/");
            my_system("cp -f /test/wno.bin /home/default/test/");
            my_system("cp -f /test/hdic_1.bin /home/default/test/");
            //my_system("cp -f /test/detect.bin /home/default/test/");
            sprintf(str_cmd, "cp -f %s /home/default%s", EXEC_CHECKSUM_FILE, EXEC_CHECKSUM_FILE);
            my_system(str_cmd);
            my_system("touch /home/default/test/2.bak");
            my_system("touch /home/default/test/rootfs.bak");
            my_system("umount /home/default");
        }
        else
        {
            my_printf("doing whole copy...\n");

            sprintf(str_cmd, "dd if=%s of=%s", RFS0_DEVNAME, RFS2_DEVNAME);
            my_system(str_cmd);

            sprintf(str_cmd, "mount %s /home/default", RFS2_DEVNAME);
            my_system(str_cmd);
            my_system("rm -f /home/default/test/1.bak");
            my_system("touch /home/default/test/2.bak");
            my_system("umount /home/default");
        }

        my_printf("====copying...done\n");

        my_printf("====calculating checksum...\n");
        //calc checksum of rootfs.
        if (get_rootfs_checksum(RFS0_DEVNAME, &g_xRecvHdr.iRfsChecksum0))
        {
            my_printf("calc checksum failed,%s.\n", RFS0_DEVNAME);
            return;
        }
        if (get_rootfs_checksum(RFS2_DEVNAME, &g_xRecvHdr.iRfsChecksum2))
        {
            my_printf("calc checksum failed,%s.\n", RFS2_DEVNAME);
            return;
        }
        recvUpdateHeader();
        my_printf("====calculating checksum...done, %08x, %08x\n",
               g_xRecvHdr.iRfsChecksum0, g_xRecvHdr.iRfsChecksum2);

        //mark as finished
        my_system("mount -o remount, rw /");
        my_system("touch /test/1.bak");

        fp = fopen("/test/rootfs.bak", "wb");
        if(fp)
        {
            fflush(fp);
            my_fsync(fileno(fp));
            fclose(fp);
        }
        else
        {
            my_printf("failed to create rootfs.bak.\n");
            return;
        }

        fp = fopen("/test/first", "wb");
        if(fp)
        {
            fflush(fp);
            my_fsync(fileno(fp));
            fclose(fp);
        }
        else
        {
            my_printf("failed to create first file.\n");
            return;
        }

        my_system("mount -r -o remount /");
        my_printf("====finally...done\n");
    }
#endif // ROOTFS_BAK
}

mythread_ptr   g_restore = 0;
mythread_ptr   g_restoreDB = 0;

void* ProcessRestore(void*)
{
#if (ROOTFS_BAK)
    int iIsFirst = 0;
    iIsFirst = rootfs_is_first();
    if (!iIsFirst)
        check_restore_roofs();

    if(g_xSS.iRestoreRootfs)
    {
        my_printf("ProcessRestore: Start: %f\n", Now());
        do_restore_rootfs();
        my_printf("ProcessRestore: End: %f\n", Now());
        g_xSS.iRestoreRootfs = 2;
    }
    pthread_exit(NULL);
#endif
    return NULL;
}

void* dbPartRestore(void*)
{
#if 1
    my_printf("@@@ Copy Start %f\n", Now());
#if (N_MAX_HAND_NUM == 0)
    int FILESIZE = sizeof(DB_INFO);
#else
    int FILESIZE = sizeof(DB_INFO) + sizeof(DB_HAND_INFO);
#endif
    int real_file_len;
    int write_file_len;
    unsigned char *buf = (unsigned char*)my_malloc(FILESIZE);
    if (buf == NULL)
    {
        printf("@@@ dbPartRestore buf malloc fail\n");
        return NULL;
    }
    real_file_len = my_flash_part_read(dbfs_part_names[DB_PART_BACKUP], 0, buf, FILESIZE);
    if (real_file_len != FILESIZE)
    {
        printf("@@@ read backupdb fail\n");
        my_free(buf);
        return NULL;
    }
    write_file_len = my_flash_part_write(dbfs_part_names[DB_PART1], 0, buf, FILESIZE);
    if (write_file_len != FILESIZE)
    {
        printf("@@@ write userdb fail\n");
        my_free(buf);
        return NULL;
    }
#else
    unsigned int len;
    unsigned int copy_size = 0x2000;
    unsigned char *pbData;
    unsigned int fail_count = 0;

    pbData = (unsigned char *)my_malloc(copy_size);
    memset(pbData, 0xff, copy_size);
    len = USERDB_SIZE;

    my_printf("@@@ Copy Start %f\n", Now());
    len -= g_xROKLog.x.iDBformatCount * copy_size;

    while(len > 0 && g_xSS.idbRestoreStop == 0)
    {
        if (fail_count > 3)
        {
            my_printf("@@@ read/write fail!\n");
            break;
        }
        my_printf("$$$ %d-%f\n", g_xROKLog.x.iDBformatCount, Now());

        if (len > copy_size)
        {
            if (my_flash_read(USERDB_START_ADDR + USERDB_SIZE + g_xROKLog.x.iDBformatCount * copy_size, copy_size, pbData, copy_size) != copy_size)
            {
                fail_count++;
                continue;
            }
            if (my_flash_write(USERDB_START_ADDR + g_xROKLog.x.iDBformatCount * copy_size, pbData, copy_size) != copy_size)
            {
                fail_count++;
                continue;
            }
            len -= copy_size;
        }
        else
        {
            if (my_flash_read(USERDB_START_ADDR + USERDB_SIZE + g_xROKLog.x.iDBformatCount * copy_size, len, pbData, len) != len)
            {
                fail_count++;
                continue;
            }
            if (my_flash_write(USERDB_START_ADDR + g_xROKLog.x.iDBformatCount * copy_size, pbData, len) != len)
            {
                fail_count++;
                continue;
            }
            len = 0;
        }
        if (len == 0)
            g_xROKLog.x.iDBformatCount = 0;
        else
            g_xROKLog.x.iDBformatCount++;
        UpdateROKLogs();
        fail_count = 0;
        my_usleep(1);
    }
#endif
    my_free(buf);
    my_printf("@@@ Copy End %f\n", Now());
    my_thread_exit(NULL);
    return NULL;
}

int check_restore_roofs()
{
#if 0
    my_printf("check_restore_roofs\n");
    FILE* fp = NULL;
    fp = fopen("/test/1.bak", "r");
    if (fp != NULL)
    {
        fclose(fp);
        //restore rootfs every 2nd booting.
        g_xROKLog.x.bMountCounter = (g_xROKLog.x.bMountCounter + 1) % 2;
        UpdateROKLogs();

        if (g_xROKLog.x.bMountCounter == 0)
        {
            my_printf("skip restoring rootfs.\n");
            //next booting may use second rootfs partition again.
            set_kernel_flag_action(0);
            return 1;
        }

        //compare checksum first.
        unsigned int sum1;
        if (get_rootfs_checksum(RFS0_DEVNAME, &sum1))
        {
            //rootfs0 may have some errors.
            my_printf("skip restoring rootfs.\n");
            //next booting may use second rootfs partition again.
            set_kernel_flag_action(0);
            return 1;
        }

        my_printf("checksum %08x,%08x\n", sum1, g_xRecvHdr.iRfsChecksum0);
#if 1
        if (sum1 == g_xRecvHdr.iRfsChecksum0)
        {
            my_printf("checksum matches, skip restoring rootfs.\n");
            //next boot may start on the first partition.
            g_xROKLog.x.bKernelFlag = 0xAA;
            UpdateROKLogs();
            return 1;
        }
#endif

        g_xSS.iRestoreRootfs = 1;

        return 0;
    }
#endif
    return 1;
}

int start_restore_roofs()
{
#if 0 //kkk
    pthread_create (&g_restore, 0, ProcessRestore, NULL);
#endif
    return 0;
}

int start_restore_dbPart()
{
    dbPartRestore(NULL);
    //my_thread_create_ext(&g_restoreDB, 0, dbPartRestore, NULL, (char*)"restoreDB", 4096, MYTHREAD_PRIORITY_LOW);
    return 0;
}
int end_restore_dbPart()
{
    if (g_restoreDB)
    {
        g_xSS.idbRestoreStop = 1;
        my_thread_join(&g_restoreDB);
        g_xSS.idbRestoreStop = 0;
        g_restoreDB = 0;
    }
    return 0;
}

int get_sector_num(char* dev)
{
    int iSectNum = 0;
    char str_cmd[256];
    sprintf(str_cmd, "fdisk -l | grep %s", dev);
    FILE* fp = popen(str_cmd, "r");
    if(fp)
    {
        char szLine[256] = { 0 };
        fgets(szLine, sizeof(szLine), fp);
        fclose(fp);

        int iCount = 0;
        char* szToken = strtok(szLine, " ");
        while(szToken)
        {
            if(iCount == 5)
            {
                iSectNum = atoi(szToken);
                break;
            }
            szToken = strtok(NULL, " ");
            iCount ++;
        }
    }

    return iSectNum;
}

int do_restore_rootfs()
{
#if 0//kkk
    //copy files to rootfs backup partitions
    my_printf("Restoring rootfs... %f\n", Now());

    g_xRecvHdr.iRfsRecoverCount++;
    recvUpdateHeader();

#if 0
    sprintf(str_cmd, "dd if=%s of=%s", RFS2_DEVNAME, RFS0_DEVNAME);
    my_printf("excuting...%s\n", str_cmd);
    my_system(str_cmd);
#else
    int fd_i, fd_o;
    fd_i = open(RFS2_DEVNAME, O_RDWR);
    if(fd_i >= 0)
    {
        fd_o = open(RFS0_DEVNAME, O_RDWR);
        if(fd_o >= 0)
        {
            int iSectNum = get_sector_num(RFS2_DEVNAME);
            my_printf("Sector Num: %d\n", iSectNum);
            unsigned char abData[4096] = { 0 };
            for(int i = 0; i < iSectNum / 8; i ++)
            {
                read(fd_i, abData, sizeof(abData));
                write(fd_o, abData, sizeof(abData));
            }
            my_fsync(fd_o);
            close(fd_o);
        }

        close(fd_i);
    }
#endif

    char str_cmd[512];
    sprintf(str_cmd, "mount %s /home/default", RFS0_DEVNAME);
    my_system(str_cmd);
    my_system("rm -f /home/default/test/2.bak");
    my_system("umount -l -f /home/default");

    my_printf("====restoring...done: %f\n", Now());

//        //reset db mount retry counter.
//        g_xROKLog.x.bMountRetry = 0;
//        UpdateROKLogs();

    unsigned int sum1;
    get_rootfs_checksum(RFS0_DEVNAME, &sum1);

    g_xRecvHdr.iRfsChecksum0 = sum1; //new checksum will be used from now on.
    recvUpdateHeader();

    g_xROKLog.x.bKernelCounter = 0;
    g_xROKLog.x.bKernelFlag = 0xAA;
    UpdateROKLogs();
#endif
    return 0;
}


#if 0
void handler(int sig)
{
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);

//  for (int i = 0; i < 10; i ++)
//      my_printf("%d ", ss_values[i]);
  my_printf("\n");

  M24C64_Open();
  ReadROKLogs();
  g_xROKLog.x.bKernelFlag = 0x55;//kzh
  UpdateROKLogs();
  M24C64_Close();
  my_printf("[kbackup]sigsegv.\n");

  exit(-1);
}


void sigill_handler(int /*sig*/)
{
    M24C64_Open();
    ReadROKLogs();
    g_xROKLog.x.bKernelFlag = 0x55;
    UpdateROKLogs();
    M24C64_Close();
    my_printf("[kbackup]sigill.\n");
    exit(-1);
}
#endif

void process_seg_fault()
{
    //signal(SIGSEGV, handler);   // install our handler
    //signal(SIGILL, sigill_handler);
}

void process_seg_save(int n, int v)
{
    if (n < 10 && n >= 0)
        ss_values[n] = v;
}
