#ifndef UPGRADE_BASE_H
#define UPGRADE_BASE_H
#include "appdef.h"
///* UPGRADE FIRMWARE *///
#define UPDATE_FIRM_ZIP_PATH "/tmp1/upfirm.7z"
#define UPDATE_FIRM_PATH "/tmp1/upfirm.img"
#define UPGRADE_APP_PATH "/tmp1/app_upgrade"
#if (NFS_DEBUG_EN)
#define UPGRADE_PATH "/mnt/Upgrader"
#else
#define UPGRADE_PATH "/test/Upgrader"
#endif
#define UPDATE_SRC_PATH "/tmp1"
#define UPDATE_FIRM_ZIMG_PATH "/home/upfirm~1.zim"
///////

#define UF_MAX_PART_CNT         32
typedef struct {
    char m_partname[16];
    unsigned int m_offset;
    unsigned int m_size;
    unsigned int m_flags;
} s_uf_part_info;

typedef struct st_uf_file_header
{
    char m_magic[8];
    char m_model[64];
    int m_major;
    int m_minor;
    int m_build;
    int m_patch;
    char m_subtype[32];
    int app_start;
    int app_size;
    int n_file;
    int n_dir;
    int n_link;
    char m_back_ver[32];
    s_uf_part_info m_part_infos[UF_MAX_PART_CNT];
} uf_file_header;

#ifdef __cplusplus
extern  "C"
{
#endif

int mount_tmp(int is_usb = 0);
int umount_tmp();
void do_reset_tmp();
int upg_get_aes_key(unsigned char* buf);
#ifndef _PACK_OTA_
int upg_do_ota4mem(unsigned char* ota_buf, unsigned int ota_len);
int upg_update_part(const char* u_filepath, unsigned char* u_buffer, unsigned int u_size, uf_file_header* u_header);
#endif // !_PACK_OTA_

#ifdef __cplusplus
}
#endif

#endif // UPGRADE_BASE_H

