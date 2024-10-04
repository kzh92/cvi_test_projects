#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "appdef.h"
#include "common_types.h"
#include "config_vars.h"
#include "aescrypt.h"
#include "../z_base/folderfile.h"

#define IMG_RESOURCEDIR     "./pack"
#define RESOURCEDIR         "./pack"
#define APPDIR              "./solutions/usb_cam/application/"

#if (DEFAULT_CHIP_TYPE == MY_CHIP_D10)
#define FACEENGINEDIR       "./solutions/usb_cam/application/FaceEngine/Dic/D10/"
#if (DEFAULT_SUBCHIP_TYPE == MY_SUBCHIP_D10)
#define FIRM_MAGIC          "EASEN8"
#elif (DEFAULT_SUBCHIP_TYPE == MY_SUBCHIP_D10A)
#define FIRM_MAGIC          "EASEN15"
#else
#error "INVALID Chip Type"
#endif
#elif(DEFAULT_CHIP_TYPE == MY_CHIP_D20)
#define FACEENGINEDIR       "./solutions/usb_cam/application/FaceEngine/Dic/D20/"
#if (DEFAULT_SUBCHIP_TYPE == MY_SUBCHIP_D20A)
#define FIRM_MAGIC          "EASEN10"
#else
#define FIRM_MAGIC          "EASEN9"
#endif
#else // DEFAULT_CHIP_TYPE
#error "Invalid chip type"
#endif // DEFAULT_CHIP_TYPE

#define IMAGEDIR            "./pack/images"
#if (USE_16M_FLASH == 0)
#define APP_DIR             "rootfs/test/"
#define DICT_DIR            "rootfs/test/"
#else // USE_16M_FLASH
#define APP_DIR             "data1/"
#define DICT_DIR            "data2/"
#endif // USE_16M_FLASH

#define FIRM_IMG_PATH       "./images.img"

#define AUTO_CONFIG_HEADER_SIZE     8192
char g_auto_config_string[8192];
char g_auto_config_read[8192];
unsigned char g_wno_origin_values[4] = {0};

int req_crypto(const char * filename, unsigned char* file_buf)
{
    int idx = 0;
    while(g_part_files[idx].m_filename != NULL)
    {
        if (strstr(filename, g_part_files[idx].m_filename))
        {
            if (g_part_files[idx].m_flag & FN_CRYPTO_AES)
            {
                unsigned char* out_buf = NULL;
                int out_len = 0;
                AES_Encrypt(g_part_crypto_aes_key, file_buf, g_part_files[idx].m_cryptosize, &out_buf, &out_len);
                if (out_buf && out_len > 0)
                {
                    memcpy(file_buf, out_buf, out_len);
                    free(out_buf);
                }
                printf("crypt(len=%d): %s, %d\n", g_part_files[idx].m_cryptosize, filename, g_part_files[idx].m_filesize);
                if (N_MAX_PERSON_NUM && strstr(filename, "/wno.bin"))
                {
                    memcpy(g_wno_origin_values, file_buf, sizeof(g_wno_origin_values));
                }
                else if (N_MAX_PERSON_NUM && USE_RENT_ENGINE && strstr(filename, "/wno_c.bin"))
                {
                    memcpy(g_wno_origin_values, file_buf, sizeof(g_wno_origin_values));
                }
                else if (!N_MAX_PERSON_NUM && strstr(filename, "/wnh.bin"))
                {
                    memcpy(g_wno_origin_values, file_buf, sizeof(g_wno_origin_values));
                }
                return 1;
            }
            else
            {
                printf("nocrypt(len=%d): %s, %d\n", g_part_files[idx].m_cryptosize, filename, g_part_files[idx].m_filesize);
            }
            break;
        }

        idx ++;
    }
    return 0;
}

int req_compress(const char * filename)
{
    int idx = 0;
    while(g_part_files[idx].m_filename != NULL)
    {
        if (strstr(filename, g_part_files[idx].m_filename))
        {
            if (g_part_files[idx].m_flag & FN_CRYPTO_ZSTD)
            {
                int file_size = 0;
                FILE* fp = fopen(filename, "rb");
                if (fp)
                {
                    fseek(fp, 0, SEEK_END);
                    file_size = ftell(fp);
                    g_part_files[idx].m_filesize_de = file_size;
                    fclose(fp);
                }
                char cmd_[1024];
                sprintf(cmd_, "rm -f %s.zst", filename);
                system(cmd_);
                sprintf(cmd_, "zstd -19 %s", filename);
                system(cmd_);
                sprintf(cmd_, "%s.zst", filename);
                fp = fopen(cmd_, "rb");
                if (fp)
                {
                    fseek(fp, 0, SEEK_END);
                    file_size = ftell(fp);
                    g_part_files[idx].m_filesize = file_size;
                    fclose(fp);
                }
                return 1;
            }
        }

        idx ++;
    }
    return 0;
}

int gen_auto_config_header(void)
{
    int idx = 0;
    char aline[256];
    g_auto_config_string[0] = 0;
    strcat(g_auto_config_string, "//This file is generated automatically. Do not edit!!!\n");
    strcat(g_auto_config_string, "#ifndef _AUTO_CONFIG_VARS_H\n");
    strcat(g_auto_config_string, "#define _AUTO_CONFIG_VARS_H\n");
    while(g_part_files[idx].m_filename != NULL)
    {
        sprintf(aline, "#define %s_SIZE\t%d\n", g_part_files[idx].m_macro_prefix, g_part_files[idx].m_filesize_de);
        strcat(g_auto_config_string, aline);
        sprintf(aline, "#define %s_SIZE_REAL\t%d\n", g_part_files[idx].m_macro_prefix, g_part_files[idx].m_filesize);
        strcat(g_auto_config_string, aline);

        idx ++;
    }
    sprintf(aline, "#define DICT_ACT_ORIGN_VALUES\t{0x%02x, 0x%02x, 0x%02x, 0x%02x}\n",
        g_wno_origin_values[0], g_wno_origin_values[1], g_wno_origin_values[2], g_wno_origin_values[3]);
    strcat(g_auto_config_string, aline);
    strcat(g_auto_config_string, "#endif\n");
    FILE* fp = NULL;
    fp = fopen(APPDIR "/_base/auto_config_vars.h", "rb");
    int len = 0;
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        fread(g_auto_config_read, 1, len, fp);
        fclose(fp);
        fp = NULL;
    }
    if (len > 0 && strlen(g_auto_config_string) == len && strcmp(g_auto_config_string, g_auto_config_read) == 0)
    {
        printf("auto_config_vars.h: already created!!!\n");
    }
    else
    {
        fp = fopen(APPDIR "/_base/auto_config_vars.h", "wb");
        if (fp)
        {
            fwrite(g_auto_config_string, 1, strlen(g_auto_config_string), fp);
            fclose(fp);
            printf("GENERATED auto_config_vars.h!!!\n");
        }
        else
        {
            printf("ERROR! failed to open file: auto_config_vars.h\n");
        }
    }
    return 0;
}

int merge_files(const char* dest_file, int pad_size)
{
    FILE* fp_out;
    int total_size = 0;
    char afilename[1024];
    int i = 0;
    fp_out = fopen(dest_file, "wb");
    if (fp_out == NULL)
    {
        printf("failed to create file: %s\n", dest_file);
        return 1;
    }
    while(g_part_files[i].m_filename != NULL)
    {
        if (g_part_files[i].m_filename[0] == '/')
            snprintf(afilename, sizeof(afilename), FACEENGINEDIR "%s", g_part_files[i].m_filename);
        else
            snprintf(afilename, sizeof(afilename), RESOURCEDIR "/rc/%s", g_part_files[i].m_filename);
        FILE* fp_in;
        int file_size = 0;
        char cmd_[1024];
        if (req_compress(afilename))
        {
            sprintf(cmd_, "%s.zst", afilename);
        }
        else
        {
            sprintf(cmd_, "%s", afilename);
        }
        fp_in = fopen(cmd_, "rb");
        if (fp_in == NULL)
        {
            printf("failed to open file:%s\n", afilename);
            i++;
            continue;
        }
        fseek(fp_in, 0, SEEK_END);
        file_size = ftell(fp_in);
        if (file_size > 0)
        {
            int malloc_len = (file_size + (pad_size - 1)) / pad_size * pad_size;
            char* file_data = (char*)malloc(malloc_len);
            if (file_data == NULL)
            {
                printf("failed to malloc(%s), len=%d\n", afilename, malloc_len);
                continue;
            }
            memset(file_data, 0, malloc_len);
            fseek(fp_in, 0, SEEK_SET);
            fread(file_data, 1, file_size, fp_in);
            req_crypto(afilename, (unsigned char*)file_data);
            int cur_off = (int)ftell(fp_out);
            fwrite(file_data, 1, malloc_len, fp_out);

            char temp_path[1024];
            sprintf(temp_path, "%s.out", afilename);

            FILE* fp_one = NULL;
            fp_one = fopen(temp_path, "wb");
            if (fp_one)
            {
                fwrite(file_data, 1, malloc_len, fp_one);
                fclose(fp_one);
                printf("\t%s(offset=%d)\n", temp_path, cur_off);
            }

            free(file_data);
            total_size += malloc_len;
        }
        fclose(fp_in);
        i++;
    }
    //write null buffer for empty file.
    char buf[64] = {0};
    fwrite(buf, 1, sizeof(buf), fp_out);

    fclose(fp_out);
    printf("created file: %s, size=%d, 0x%08x\n", dest_file, total_size, total_size);
    system("mv -f " FACEENGINEDIR "/*.out ./pack/");
    gen_auto_config_header();
    return 0;
}

int pack_firm()
{
    char str_files[1024];
    const char *pack_file_names[] = {"boot","config.yaml","fip_fsbl.bin","imtb","prim","pst","pusr1","pusr2","pwx"};
    char str_afile_name[256];
    //decompress images.zip
    system("rm -f ./images.img");
    system("rm -f ./images.zip");
    system("rm -rf ./images");
    system("mkdir ./images");
    system("unzip ./images.zip -d ./images");
    system("cp -f ../solutions/usb_cam/generated/images.zip ./");

    FILE* tf = NULL;
    tf = fopen("./images.zip", "rb");
    if (tf == NULL)
    {
        printf ("ERROR! file not found images.zip\n");
        return 0;
    }
    fclose(tf);

    system("unzip ./images.zip -d ./images");
    system("rm -f ./images/misc");
    system("rm -f ./images/boot0");
    //check files
    FILE* fp;
    fp = popen("ls ./images", "r");
    if (fp == NULL)
    {
        printf("ls failed.\n");
        return 0;
    }
    fgets(str_files, sizeof(str_files)-1, fp);
    pclose(fp);

    char *str1, *str2, *token, *subtoken;
    char *saveptr1, *saveptr2;
    int j;

    for (j = 1, str1 = str_files; ; j++) {
       token = strtok_r(str1, "  ", &saveptr1);
       if (token == NULL)
           break;
       printf("%d: %s\n", j, token);

       str1 = saveptr1;
    }

    //make firmware file
    uf_file_header ufh;
    memset(&ufh, 0, sizeof(ufh));
    strcpy(ufh.m_magic, FIRM_MAGIC);
    FolderFile ff("./images");
    ff.compressDirectory((char*)"", (char*)FIRM_IMG_PATH, ufh, NULL);
    return 0;
}

int main(int argc, char** argv)
{
    //    char szCmd[256] = { 0 };
    const char* szRoot = ".";
    char szPath[256] = { 0 };

    if (argc == 1)
    {
    }
    else if (strcmp(argv[1], "pack") == 0)
    {
        pack_firm();
        return 0;
    }

    sprintf(szPath, "%s/../", szRoot);
    chdir(szPath);

    system("rm -rf " IMAGEDIR);
    system("mkdir " IMAGEDIR);
    printf("----------pwd\n");
    system("pwd");
    printf("--------------\n");

#if (DEFAULT_SUBCHIP_TYPE == MY_SUBCHIP_D10A)
    system("rm -f ./components/chip_cv180x/gcc_flash.ld");
    system("cp -f ./components/chip_cv180x/gcc_flash_128M.ld ./components/chip_cv180x/gcc_flash.ld");
    system("cp -f ./boards/cv180xb_evb/bootimgs/fip_fsbl_D10A.bin ./boards/cv180xb_evb/bootimgs/fip_fsbl.bin");
#elif (DEFAULT_SUBCHIP_TYPE == MY_SUBCHIP_D10)
    system("rm -f ./components/chip_cv180x/gcc_flash.ld");
    system("cp -f ./components/chip_cv180x/gcc_flash_64M.ld ./components/chip_cv180x/gcc_flash.ld");
    system("cp -f ./boards/cv180xb_evb/bootimgs/fip_fsbl_D10.bin ./boards/cv180xb_evb/bootimgs/fip_fsbl.bin");
#elif (DEFAULT_SUBCHIP_TYPE == MY_SUBCHIP_D20A)
    system("rm -f ./components/chip_cv181x/gcc_flash.ld");
    system("cp -f ./components/chip_cv181x/gcc_flash_128M.ld ./components/chip_cv181x/gcc_flash.ld");
    system("rm -f ./boards/cv181xc_qfn/bootimgs/fip_fsbl.bin");
    system("cp -f ./boards/cv181xc_qfn/fip_fsbl_D20A.bin ./boards/cv181xc_qfn/bootimgs/fip_fsbl.bin");
#else // D20
    system("rm -f ./components/chip_cv181x/gcc_flash.ld");
    system("cp -f ./components/chip_cv181x/gcc_flash_64M.ld ./components/chip_cv181x/gcc_flash.ld");
    system("rm -f ./boards/cv181xc_qfn/bootimgs/fip_fsbl.bin");
    system("cp -f ./boards/cv181xc_qfn/fip_fsbl_D20.bin ./boards/cv181xc_qfn/bootimgs/fip_fsbl.bin");
#endif // DEFAULT_SUBCHIP_TYPE

#if (USE_TWIN_ENGINE == 1)
    system("" RESOURCEDIR "/utils/Encoder " FACEENGINEDIR "/hdic_1.bin " RESOURCEDIR "/hdic_1_encode.bin");
#endif

    merge_files(IMAGEDIR "/pwx", FN_DICT_ALIGN_SIZE);
    system("dd if=/dev/zero of=" IMAGEDIR "/pusr1 bs=1024 count=8");
    system("dd if=/dev/zero of=" IMAGEDIR "/pusr2 bs=1024 count=8");
    system("dd if=/dev/zero of=" IMAGEDIR "/pst bs=1024 count=8");

#if (UAC_SPK_NR_USE == 2)
    system("cp -f " IMG_RESOURCEDIR "/libs/libcvi_mw_audio.a components/cvi_mmf_sdk_cv180xx/lib/");
    system("cp -f " IMG_RESOURCEDIR "/libs/libcvi_mw_audio.a components/cvi_mmf_sdk_cv181xx/lib/");
#endif

    return 0;
}
