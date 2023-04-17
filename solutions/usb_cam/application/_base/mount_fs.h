#ifndef _MOUNT_FS_H_
#define _MOUNT_FS_H_

#include "common_types.h"

#if 1
int             mount_dbfs(int type);
int             umount_dbfs();
int             backup_dbfs(int iBlkNum);
int             delete_dbfs();
int             fsck_dbfs(int);
int             format_dbfs();

int             mount_dbfs_sub(int type);
int             mount_backup_db(int iFlag);
int             umount_backup_db();


int             fsck_dbfs_part(int part_id, int opt = 0);
const char*     dbfs_get_part_name_by_id(int id);
const char*     dbfs_get_cur_part_name();
int             do_make_ext4(const char* dev_path);
int             restore_dbfs();
int             mount_backupdbfs();

int             try_mount_dbfs();

#endif

void            do_backup_rootfs();
int             check_restore_roofs();
int             do_restore_rootfs();
int             start_restore_roofs();
int             start_restore_dbPart();
int             end_restore_dbPart();
int             get_sector_num(char* dev);

void            process_seg_fault();

#endif /* _MOUNT_FS_H_ */

