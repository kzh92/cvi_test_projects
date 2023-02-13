#include "qrOTP_base.h"

#if (USE_WIFI_MODULE)

#include "shared.h"

#include <../_quirc/quirc.h>
#include <../_quirc/dthash.h>
#include <../_quirc/qr_convert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

struct quirc *qr = NULL;
static int video_width = 0;
static int video_height = 0;
static int printer_timeout = 2;

void create_qrreader(int src_w, int src_h)
{
    if(qr)
        return;
    qr = quirc_new();
    if (!qr) {
        my_printf("couldn't allocate QR decoder\n");
    }

    if (quirc_resize(qr, src_w, src_h) < 0) {
        my_printf("couldn't allocate QR buffer\n");
        quirc_destroy(qr);
    }

    video_width = src_w;
    video_height = src_h;
}

void destroy_qrreader()
{
    if(!qr)
        return;
    quirc_destroy(qr);
    qr = NULL;
}

int scan_candidate4OTP(unsigned char* src_buf, char* result)
{
    int iCandidateCount = 0;

    if(src_buf == NULL)
    {
        my_printf("!!!!!!!!  src_buf is NULL\n");
        return 0;
    }

    struct dthash *dt = (struct dthash *)my_malloc(sizeof(struct dthash));
    if (dt == NULL)
    {
        my_printf("malloc dt failed\n");
        return 0;
    }

    dthash_init(dt, printer_timeout);

    uint8_t *gray_buffer = quirc_begin(qr, NULL, NULL);
#if 1
    yuyv_to_luma(src_buf, video_width * 2,
                  video_width, video_height,
                  gray_buffer,
                  video_width);
#else
    memcpy(gray_buffer, src_buf, video_width * video_height);
#endif

#if 0
    char szName[256] = {0};
    sprintf(szName, "/tmp/gray.bin");
    FILE* fp = fopen(szName, "wb");
    if(fp)
    {
        my_printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2\n");
        fwrite(gray_buffer, video_width * video_height, 1, fp);
        fflush(fp);
        fclose(fp);
    }
#endif
    quirc_end(qr);

    int count = quirc_count(qr);
    if(count == 0)
    {
        my_free(dt);
        return 0;
    }
    else if(count > SCAN_LIMIT_NUM)
    {
        my_printf("too many qrcodes :  %d\n", count);
        count = 1;
//        return 0;
    }

    if(count > 0)
        ResetDetectTimeout();

    struct quirc_code *code = (struct quirc_code*)my_malloc(sizeof(struct quirc_code));
    struct quirc_data *data = (struct quirc_data*)my_malloc(sizeof(struct quirc_data));
    if (code == NULL || data == NULL)
    {
        return 0;
    }
    for (int i = 0; i < count; i++) {
        quirc_decode_error_t err;
        int j;
        int x1 = video_width;
        int y1 = video_height;
        int x2 = 0;
        int y2 = 0;

        quirc_extract(qr, i, code);

        for (j = 0; j < 4; j++) {
            struct quirc_point *a = &code->corners[j];
            x1 = (x1 > a->x) ? a->x : x1;
            y1 = (y1 > a->y) ? a->y : y1;
            x2 = (x2 < a->x) ? a->x : x2;
            y2 = (y2 < a->y) ? a->y : y2;
        }

        err = quirc_decode(code, data);

        if (err) {
            my_printf("quirc decode error!\n");
            continue;
        } else {
//            my_printf("%s(%d)\n", data.payload, data.payload_len);
            strncpy(result, (char*)data->payload, data->payload_len);

            iCandidateCount++;
        }
    }
    my_free(code);
    my_free(data);

    my_free(dt);

    return iCandidateCount;
}

#endif // USE_WIFI_MODULE
