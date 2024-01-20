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

#pragma pack(pop)

#endif // USE_USB_XN_PROTO

#endif //!_DS_H264_TYPES_H
