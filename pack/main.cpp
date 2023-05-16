#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "appdef.h"
#include "common_types.h"

#define IMG_RESOURCEDIR     "./pack"
#define RESOURCEDIR         "./pack"
#define FACEENGINEDIR       "./solutions/usb_cam/application/FaceEngine"
#define IMAGEDIR            "./pack/images"
#if (USE_16M_FLASH == 0)
#define APP_DIR             "rootfs/test/"
#define DICT_DIR            "rootfs/test/"
#else // USE_16M_FLASH
#define APP_DIR             "data1/"
#define DICT_DIR            "data2/"
#endif // USE_16M_FLASH

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
        fp_in = fopen(file_names[i], "rb");
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
            fwrite(file_data, 1, malloc_len, fp_out);
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

    system("cp -f " FACEENGINEDIR "/Dic/wno.bin " RESOURCEDIR "/wno_encode.bin");
    //system("" RESOURCEDIR "/utils/Encoder " FACEENGINEDIR "/Dic/wno.bin " RESOURCEDIR "/wno_encode.bin");
#if (N_MAX_HAND_NUM)
    //system("" RESOURCEDIR "/utils/Encoder " FACEENGINEDIR "/Dic/wnh.bin " RESOURCEDIR "/wnh_encode.bin");
    system("cp -f " FACEENGINEDIR "/Dic/wnh.bin " RESOURCEDIR "/wnh_encode.bin");
#endif

#if (USE_TWIN_ENGINE == 1)
    system("" RESOURCEDIR "/utils/Encoder " FACEENGINEDIR "/Dic/hdic_1.bin " RESOURCEDIR "/hdic_1_encode.bin");
#endif

    const char* merge_path1[] = {
        RESOURCEDIR "/wno_encode.bin",
        FACEENGINEDIR "/Dic/detect.bin",
        FACEENGINEDIR "/Dic/dlamk.bin",
    #if (DESMAN_ENC_MODE == 0)
        FACEENGINEDIR "/Dic/occ.bin",
        FACEENGINEDIR "/Dic/esn.bin",
    #endif // DESMAN_ENC_MODE
    #if (ENGINE_USE_TWO_CAM)
        FACEENGINEDIR "/Dic/a1.bin",
        FACEENGINEDIR "/Dic/a2.bin",
        FACEENGINEDIR "/Dic/b.bin",
        FACEENGINEDIR "/Dic/b2.bin",
        FACEENGINEDIR "/Dic/c.bin",
    #else // ENGINE_USE_TWO_CAM
        FACEENGINEDIR "/Dic/a1_onecam.bin",
        FACEENGINEDIR "/Dic/a2_onecam.bin",
        FACEENGINEDIR "/Dic/c_onecam.bin",
    #endif // ENGINE_USE_TWO_CAM
    #if (N_MAX_HAND_NUM)
        FACEENGINEDIR "/Dic/detect_h.bin",
        FACEENGINEDIR "/Dic/dlamk_h.bin",
        FACEENGINEDIR "/Dic/ch.bin",
        RESOURCEDIR "/wnh_encode.bin",
    #endif // N_MAX_HAND_NUM
    #if (USE_TWIN_ENGINE)
        FACEENGINEDIR "/Dic/hdic_2.bin",
    #endif
        RESOURCEDIR "/rc/face_ir.bin",
        NULL
    };
    merge_files(merge_path1, IMAGEDIR "/pwx", FN_DICT_ALIGN_SIZE);
    system("dd if=/dev/zero of=" IMAGEDIR "/pusr1 bs=1024 count=8");
    system("dd if=/dev/zero of=" IMAGEDIR "/pusr2 bs=1024 count=8");
    system("dd if=/dev/zero of=" IMAGEDIR "/pst bs=1024 count=8");

    return 0;
}
