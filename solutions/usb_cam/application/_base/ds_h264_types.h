#ifndef _DS_H264_TYPES_H
#define _DS_H264_TYPES_H

#include "appdef.h"
#include <stdio.h>

#if (USE_USB_XN_PROTO)

//dualstreaming usb user request commands
#define DS_UUR_XIN_REQ                          0xA5
#define DS_UUR_XIN_FORCE_KEYFRAME               0x06
#define DS_UUR_XIN_GET_VIDEO_RES                0x0A    // get video res
#define DS_UUR_XIN_SET_VIDEO_RES                0x09    // set video res
#define DS_UUR_XIN_GET_AUDIO_RES                0x0B    // get audio res
#define DS_UUR_XIN_SET_AUDIO_RES                0x0C    // set audio res

typedef enum
{
    DS_VENC_TYPE_H264 = 1,
    DS_VENC_TYPE_H265 = 2,
} e_ds_venc_type;

typedef enum
{
    DS_VRC_MODE_CBR = 0,    // H26x, MJPEG
    DS_VRC_MODE_VBR = 1,    // H26x
    DS_VRC_MODE_FIXQP = 2,  // H26x, MJPEG
    DS_VRC_MODE_AVBR = 3,   // H26x
} e_ds_vrc_mode;

#pragma pack(push, 1)

typedef struct
{
    unsigned short usWidth; //
    unsigned short usHeight;
    unsigned short usGop;
    unsigned short usDataSize; //Reserved
    unsigned int uiBitrate; //Kbps
    unsigned char ucFps;
    unsigned char ucCheckSum;
    unsigned char ucEncodeType; //e_ds_venc_type
    unsigned char ucRcMode; //e_ds_vrc_mode
    unsigned char ucUcodeMode; // 1：基础, 2：高级
} s_ds_h264_video_res;

typedef struct
{
    unsigned char  ucAiVol;
    unsigned char  ucAoVol;
    //可扩展
} s_ds_h264_audio_res;

#define __bswap_16(x) ((uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#define __bswap_32(x) ((uint32_t) ((((x) >> 24) & 0xff) | \
                   (((x) >> 8) & 0xff00) | \
                   (((x) & 0xff00) << 8) | \
                   (((x) & 0xff) << 24)))

#define UVC_VENC_H26X_CHN   (1)
#define H26X_FRAME_OFFSET       (0x14)
#define H26X_HEADER_SIZE        (14)
#define MAX_H26X_PACKET_SIZE    (0xFFF3)
#define MAX_H26X_TAG_SIZE       (H26X_HEADER_SIZE + MAX_H26X_PACKET_SIZE)
#define MAX_H26X_FRAME_SIZE     (3 * MAX_H26X_PACKET_SIZE)

typedef struct{
    unsigned short app_mark;
    unsigned short total_size;     ///< 不包括usAppMaker字段
    unsigned char strm_type;       ///< H264:0x01, H265:0x02
    unsigned char fps;
    unsigned char frm_type;        ///<  1:I  2:P
    unsigned char checksum;        ///< H265 payload数据校验和，仅添加到APP7即可
    unsigned int seq;            ///< 帧序号
    unsigned short data_size;      ///< 包括本字段2bytes
} __attribute((packed)) ds_mjpeg_app_header;

#pragma pack(pop)

#endif // USE_USB_XN_PROTO

#endif //!_DS_H264_TYPES_H
