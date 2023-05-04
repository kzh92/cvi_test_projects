#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "appdef.h"
#include "../z_base/folderfile.h"
#include "aescrypt.h"
//#define USE_APP_UPGRADE

//list of upgrade files
s_uf_part_info g_upg_list[] = {
    {"pusr1", 0x00e90000, 8192, 0},
    {"", 0, 0, 0} //terminator
};

int get_versions(const char* str_ver, int *m, int *n, int *b, int *p, char* sub_type)
{
    char str[128];
    int i;
    int prev = 0;
    *m = -1;
    *n = -1;
    *b = -1;
    *p = -1;
    strcpy(str, str_ver);

    //get sub type version.
    for (i = 0; str[i] != 0; i ++)
    {
        if (str[i] == '.')
        {
            str[i] = '\0';
            if (*m == -1)
                sscanf(str + prev, "%d", m);
            else if (*n == -1)
                sscanf(str + prev, "%d", n);
            prev = i + 1;
        }
        if (str[i] == ' ')
        {
            str[i] = '\0';
            if (*b == -1)
                sscanf(str + prev, "%d", b);
            strcpy(sub_type, str+i+1);
            break;
        }
    }
    for (i = 0; i < strlen(sub_type); i ++)
    {
        if (sub_type[i] == '-')
        {
            //get patch version.
            sscanf(sub_type+i+1, "%d", p);
            sub_type[i] = '\0';
            break;
        }
    }
    return 0;
}

#define FN_IMG_PATH "./upfirm.img"

char filename[1024];
char upexec_path[1024];
char str_version[1024];
char str_model[1024];
char srcdir_path[1024];

int main(int argc, char** argv)
{
    uf_file_header ufh;
    FILE *fp;

    memset(&ufh, 0, sizeof(ufh));
    memcpy(ufh.m_part_infos, g_upg_list, sizeof(g_upg_list));

    sprintf(filename, "upfirm.zimg");
    sprintf(str_version, "%s", DEVICE_FIRMWARE_VERSION);
    sprintf(str_model, "%s", DEVICE_MODEL_NUM);
    sprintf(upexec_path, "%s", "../__build493/PusinLock_Product/app_upgrade");
    sprintf(srcdir_path, "./updata");

    get_versions(str_version, &ufh.m_major, &ufh.m_minor, &ufh.m_build, &ufh.m_patch, ufh.m_subtype);

    if (strlen(FIRMWARE_MAGIC) > sizeof(ufh.m_magic) - 1)
    {
        printf("ERROR! string length of firmware magic(%s) must not exceed %d.\n", FIRMWARE_MAGIC, (int)sizeof(ufh.m_magic));
        return 0;
    }
    strcpy(ufh.m_magic, FIRMWARE_MAGIC);
    strcpy(ufh.m_model, str_model);

    FolderFile ff(srcdir_path);
    ff.compressDirectory((char*)"", (char*)FN_IMG_PATH, ufh, upexec_path);

    system("rm -f ./upfirm.7z");
    system("7zr a ./upfirm.7z " FN_IMG_PATH);

    fp = fopen("./upfirm.7z", "rb");
    fseek(fp, 0, SEEK_END);
    int iFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    printf("iFileSize : %d\n", iFileSize);
    unsigned char checkSum = 0;

    FILE* out = fopen("./upfirm.zimg", "wb");

#if 0
    unsigned int iPieceSize = 5120;
    unsigned char* tmp_buf = NULL;
    tmp_buf = (unsigned char*)malloc(iPieceSize);

    int iTotal = iFileSize / iPieceSize;
    for(unsigned int i = 0; i < iTotal; i++)
    {
        memset(tmp_buf, 0, iPieceSize);
        fread(tmp_buf, iPieceSize, 1, fp);
        for(unsigned int j = 0; j < iPieceSize; j++)
        {
            checkSum ^= tmp_buf[j];
            tmp_buf[j] = tmp_buf[j] ^ (((i * iPieceSize + j) ^ (iFileSize - (i * iPieceSize + j))) % 128);
        }
        fwrite(tmp_buf, iPieceSize, 1, out);
    }
    unsigned int iRemain = iFileSize % iPieceSize;
    if(iRemain != 0)
    {
        memset(tmp_buf, 0, iPieceSize);
        fread(tmp_buf, iRemain, 1, fp);
        for(unsigned int j = 0; j < iRemain; j++)
        {
            checkSum ^= tmp_buf[j];
            tmp_buf[j] = tmp_buf[j] ^ (((iTotal * iPieceSize + j) ^ (iFileSize - (iTotal * iPieceSize + j))) % 128);
        }
        fwrite(tmp_buf, iRemain, 1, out);
    }
    fwrite(&checkSum, 1, 1, out);
    free(tmp_buf);
#else
    unsigned char* out_buf;
    unsigned char akey[AES_BLOCK_SIZE];
    unsigned char* tmp_buf = NULL;
    int blk_size = 4096;
    int read_len = 0;
    tmp_buf = (unsigned char*)malloc(blk_size);
    for (int i = 0; i < iFileSize / blk_size; i++)
    {
        read_len = iFileSize - i * blk_size;
        if (read_len > blk_size)
            read_len = blk_size;
        fread(tmp_buf, 1, read_len, fp);
        for (int k = 0; k < read_len ;k ++)
            checkSum ^= tmp_buf[i];
        FolderFile::encryptBuf(tmp_buf, read_len);
        fwrite(tmp_buf, 1, read_len, out);
    }
    free(tmp_buf);
    fwrite(&checkSum, 1, 1, out);
#endif
    fclose(fp);
    fclose(out);
    printf("Created upgrade firmware!\n");
    return 0;
}
