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

#define IMG_RESOURCEDIR     "./pack"
#define RESOURCEDIR         "./pack"

#if (DEFAULT_CHIP_TYPE == MY_CHIP_D10)
#define FACEENGINEDIR       "./solutions/usb_cam/application/FaceEngine/Dic/D10/"
#elif(DEFAULT_CHIP_TYPE == MY_CHIP_D20)
#define FACEENGINEDIR       "./solutions/usb_cam/application/FaceEngine/Dic/D20/"
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

int req_crypto(const char * filename, unsigned char* file_buf)
{
    int idx = 0;
    while(g_part_files[idx].m_filename != NULL)
    {
        if (strlen(g_part_files[idx].m_filename) > 6 && 
            strstr(filename, g_part_files[idx].m_filename + 5))
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
        if (strlen(g_part_files[idx].m_filename) > 6 && 
            strstr(filename, g_part_files[idx].m_filename + 6))
        {
            if (g_part_files[idx].m_flag & FN_CRYPTO_ZSTD)
            {
                return 1;
            }
        }

        idx ++;
    }
    return 0;
}

int merge_files(const char** file_names, const char* dest_file, int pad_size)
{
    FILE* fp_out;
    int total_size = 0;
    fp_out = fopen(dest_file, "wb");
    if (fp_out == NULL)
    {
        printf("failed to create file: %s\n", dest_file);
        return 1;
    }
    for (int i = 0; file_names[i] != NULL; i++)
    {
        FILE* fp_in;
        int file_size = 0;
        char cmd_[1024];
        if (req_compress(file_names[i]))
        {
            sprintf(cmd_, "rm -f %s.zst", file_names[i]);
            system(cmd_);
            sprintf(cmd_, "zstd --fast=4 %s", file_names[i]);
            system(cmd_);
            sprintf(cmd_, "%s.zst", file_names[i]);
        }
        else
        {
            sprintf(cmd_, "%s", file_names[i]);
        }
        fp_in = fopen(cmd_, "rb");
        if (fp_in == NULL)
        {
            printf("failed to open file:%s\n", file_names[i]);
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
                printf("failed to malloc(%s), len=%d\n", file_names[i], malloc_len);
                continue;
            }
            memset(file_data, 0, malloc_len);
            fseek(fp_in, 0, SEEK_SET);
            fread(file_data, 1, file_size, fp_in);
            req_crypto(file_names[i], (unsigned char*)file_data);
            int cur_off = (int)ftell(fp_out);
            fwrite(file_data, 1, malloc_len, fp_out);

            char temp_path[1024];
            sprintf(temp_path, "%s.out", file_names[i]);

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
    }
    //write null buffer for empty file.
    char buf[64] = {0};
    fwrite(buf, 1, sizeof(buf), fp_out);

    fclose(fp_out);
    printf("created file: %s, size=%d, 0x%08x\n", dest_file, total_size, total_size);
    system("mv -f " FACEENGINEDIR "/Dic/D10/*.out ./pack/");
    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{
    //    char szCmd[256] = { 0 };
    const char* szRoot = ".";
    char szPath[256] = { 0 };

    sprintf(szPath, "%s/../", szRoot);
    chdir(szPath);

    system("rm -rf " IMAGEDIR);
    system("mkdir " IMAGEDIR);
    printf("----------pwd\n");
    system("pwd");
    printf("--------------\n");

#if (USE_TWIN_ENGINE == 1)
    system("" RESOURCEDIR "/utils/Encoder " FACEENGINEDIR "/hdic_1.bin " RESOURCEDIR "/hdic_1_encode.bin");
#endif

    const char* merge_path1[] = {
        USE_RENT_ENGINE ? (FACEENGINEDIR "/wno_c.bin") : (FACEENGINEDIR "/wno.bin"),
        FACEENGINEDIR "/detect.bin",
        FACEENGINEDIR "/dlamk.bin",
    #if (DESMAN_ENC_MODE == 0)
        FACEENGINEDIR "/occ.bin",
        FACEENGINEDIR "/esn.bin",
    #endif // DESMAN_ENC_MODE
    #if (ENGINE_USE_TWO_CAM)
        FACEENGINEDIR "/a1.bin",
        FACEENGINEDIR "/a2.bin",
        FACEENGINEDIR "/b.bin",
        FACEENGINEDIR "/b2.bin",
        FACEENGINEDIR "/c.bin",
    #else // ENGINE_USE_TWO_CAM
        FACEENGINEDIR "/a1_onecam.bin",
        FACEENGINEDIR "/a2_onecam.bin",
        FACEENGINEDIR "/c_onecam.bin",
    #endif // ENGINE_USE_TWO_CAM
    #if (USE_RENT_ENGINE)
        FACEENGINEDIR "/Dic/D10/detect_c.bin",
    #endif
    #if (N_MAX_HAND_NUM)
        FACEENGINEDIR "/detect_h.bin",
        FACEENGINEDIR "/dlamk_h.bin",
        FACEENGINEDIR "/ch.bin",
        FACEENGINEDIR "/wnh.bin",
        FACEENGINEDIR "/lh.bin",
    #endif // N_MAX_HAND_NUM
    #if (USE_TWIN_ENGINE)
        FACEENGINEDIR "/hdic_2.bin",
    #endif
        RESOURCEDIR "/rc/face_ir.bin",
    #if (USE_UAC_MODE)
        RESOURCEDIR "/rc/audiotest.pcm",
    #endif // USE_UAC_MODE
        NULL
    };
    merge_files(merge_path1, IMAGEDIR "/pwx", FN_DICT_ALIGN_SIZE);
    system("dd if=/dev/zero of=" IMAGEDIR "/pusr1 bs=1024 count=8");
    system("dd if=/dev/zero of=" IMAGEDIR "/pusr2 bs=1024 count=8");
    system("dd if=/dev/zero of=" IMAGEDIR "/pst bs=1024 count=8");

    return 0;
}
