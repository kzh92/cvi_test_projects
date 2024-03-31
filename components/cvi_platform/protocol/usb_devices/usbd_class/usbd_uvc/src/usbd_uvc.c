#include <stdio.h>
#include <aos/kernel.h>
#include <k_atomic.h>
#include <aos/cli.h>
#include "usbd_core.h"
#include "usbd_video.h"
#include "media_video.h"
#include "cvi_venc.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include <core/core_rv64.h>

#include "usbd_uvc.h"
#include "usbd_comp.h"
#include "usbd_uvc_descriptor.h"
#include "appdef.h"
#include "settings.h"
#include "cam_base.h"
#include "ds_h264_types.h"

#define CAM_FPS        (30)
#define INTERVAL       (unsigned long)(10000000 / CAM_FPS)
#define MAX_FRAME_SIZE (unsigned long)(UVC_MAX_WIDTH * UVC_MAX_HEIGHT * 2 / 3)
#define DEFAULT_FRAME_SIZE (unsigned long)(UVC_MAX_WIDTH * UVC_MAX_HEIGHT * 2 / 3 / 2)

#define MJPEG_FORMAT_INDEX  (1)
#define H264_FORMAT_INDEX   (2)
#define YUYV_FORMAT_INDEX   (3)
#define NV21_FORMAT_INDEX   (4)

static int av_session_init_flag = CVI_FALSE;
static aos_event_t _gslUvcEvent;

static atomic_t uvc_pause_flag = CVI_FALSE;
static atomic_t uvc_pause_done = CVI_FALSE;

#if CONFIG_USB_BULK_UVC
#define FRM_BUFFER_LEN 2
static volatile uint32_t rx_frm_idx = 0;
static volatile uint32_t tx_frm_idx = 0;
static volatile uint32_t frm_sz[FRM_BUFFER_LEN] = {0};
static aos_workqueue_t uvc_workqueue;
static aos_work_t uvc_frame_submmit;
#define FRM_BUFFER_GET_IDX(idx) (idx&(FRM_BUFFER_LEN-1))
#endif

#if 0
static struct uvc_frame_info_st yuy2_frame_info[] = {
    {1, 800, 600, 15, 0},
    {2, 640, 360, 15, 0},
    {3, 400, 300, 15, 0},
    {5, 480, 320, 15, 0},
    {6, 480, 360, 15, 0},
    {7, 1280, 720, 15, 0},
};
#endif

#if (UVC_ENC_TYPE == 0 || UVC_ENC_TYPE == 2)
static struct uvc_frame_info_st mjpeg_frame_info[] = {
    UVC_RES_DEFINE
};
#endif

#if (UVC_ENC_TYPE == 1)
// #undef H264_FORMAT_INDEX
// #define H264_FORMAT_INDEX   (1)
// #undef MJPEG_FORMAT_INDEX
// #define MJPEG_FORMAT_INDEX  (2)
static struct uvc_frame_info_st h264_frame_info[] = {
    UVC_RES_DEFINE
};
#endif

#if 0
static struct uvc_frame_info_st h264_frame_info[] = {
    {1, 800, 600, 30, 0},
    {2, 1280, 720, 30, 0},
    {3, 640, 480, 30, 0},
    {4, 400, 300, 30, 0},
    {5, 1920, 1080, 30, 0},
};
#endif

//  static struct uvc_frame_info_st nv21_frame_info[] = {
//      {1, 800, 600, 15, 0},
//      {2, 1280, 720, 15, 0},
//      {3, 640, 480, 15, 0},
//  };

static struct uvc_format_info_st uvc_format_info[] = {
#if (UVC_ENC_TYPE == 0 || UVC_ENC_TYPE == 2)
    {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info), mjpeg_frame_info},
#endif
#if (UVC_ENC_TYPE == 1)
    {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
#endif
    // {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info), yuy2_frame_info},
    // {NV21_FORMAT_INDEX, UVC_FORMAT_NV21, 1, ARRAY_SIZE(nv21_frame_info), nv21_frame_info},
};

static struct uvc_device_info uvc[USBD_UVC_MAX_NUM] = {
    {
        // .ep = 0x81,
        .format_info = uvc_format_info,
        .formats = ARRAY_SIZE(uvc_format_info),
        .video = {0, 0, 0},
    },
    {
        // .ep = 0x82,
        .format_info = uvc_format_info,
        .formats = ARRAY_SIZE(uvc_format_info),
        .video = {1, 1, 0},
    },
    {
        // .ep = 0x83,
        .format_info = uvc_format_info,
        .formats = ARRAY_SIZE(uvc_format_info),
        .video = {1, 1, 0},
    }
};

static uint8_t media_buffer[DEFAULT_FRAME_SIZE] __attribute__((aligned(64)));


static CVI_S32 is_media_info_update(struct uvc_device_info *info){
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	VENC_CHN_ATTR_S stVencChnAttr,*pstVencChnAttr = &stVencChnAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr, *pstVpssChnAttr = &stVpssChnAttr;
	PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
	CVI_U8 u8VencInitStatus = pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus;
    // int iUvcSensor = DEFAULT_SNR4UVC;

	struct uvc_format_info_st uvc_format_info;
	struct uvc_frame_info_st uvc_frame_info;

	uvc_get_video_format_info(&uvc_format_info);
	uvc_get_video_frame_info(&uvc_frame_info);

	CVI_VPSS_GetChnAttr(info->video.vpss_group, info->video.vpss_channel, pstVpssChnAttr);

	switch(uvc_format_info.format_index){
	case YUYV_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_YUYV;
		break;
	case NV21_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	default:
		enPixelFormat = PIXEL_FORMAT_MAX;
		break;
	}

	if(u8VencInitStatus == 0 && enPixelFormat == PIXEL_FORMAT_MAX)
		return CVI_TRUE;

	if((pstVpssChnAttr->enPixelFormat != enPixelFormat) ||
		(pstVpssChnAttr->u32Width != uvc_frame_info.width) ||
		(pstVpssChnAttr->u32Height != uvc_frame_info.height))
		return CVI_TRUE;

	if(u8VencInitStatus == 0 && enPixelFormat != PIXEL_FORMAT_MAX)
		return CVI_FALSE;


	switch(uvc_format_info.format_index){
	case MJPEG_FORMAT_INDEX:
		enType = PT_MJPEG;
		break;
	case H264_FORMAT_INDEX:
		enType = PT_H264;
		break;
	default:
		enType = PT_BUTT;
		break;
	}

	if(u8VencInitStatus == 0 && enType != PT_BUTT)
		return CVI_TRUE;

	CVI_VENC_GetChnAttr(info->video.venc_channel, pstVencChnAttr);
	if((pstVencChnAttr->stVencAttr.enType != enType)||
		(pstVencChnAttr->stVencAttr.u32PicWidth != uvc_frame_info.width) ||
		(pstVencChnAttr->stVencAttr.u32PicHeight != uvc_frame_info.height))
		return CVI_TRUE;

	return CVI_FALSE;

}

static void uvc_parse_media_info(uint8_t bFormatIndex, uint8_t bFrameIndex)
{
    const struct uvc_format_info_st *format_info;
    const int uvcout_format_cnt = ARRAY_SIZE(uvc_format_info);

    if (bFormatIndex < 0)
        bFormatIndex = uvcout_format_cnt + bFormatIndex;
    if (bFormatIndex < 0 || bFormatIndex > uvcout_format_cnt)
    {
        aos_debug_printf("format_cnt =%d, format %d error!\r\n", uvcout_format_cnt, bFormatIndex);
        return;
    }

    format_info = &uvc_format_info[bFormatIndex - 1];

    const int nframes = format_info->frame_cnt;
    if (bFrameIndex < 0)
        bFrameIndex = nframes + bFrameIndex;
    if (bFrameIndex < 0 || bFrameIndex > nframes)
    {
        aos_debug_printf("nframes = %d, frame %d error!\r\n", nframes, bFrameIndex);
        return;
    }

	uvc_set_video_format_info(format_info);
	uvc_set_video_frame_info(&format_info->frames[bFrameIndex - 1]);
	setUvcWindow(format_info->frames[bFrameIndex - 1].width, format_info->frames[bFrameIndex - 1].height);
	g_xSS.iUvcBitrate = format_info->frames[bFrameIndex - 1].bitrate;
	g_xSS.iUvcFps = format_info->frames[bFrameIndex - 1].fps;
	if (format_info->frames[bFrameIndex - 1].rotate_flag == 0)
		g_xSS.iUvcDirect = DEFAULT_UVC_DIR;
	else
		g_xSS.iUvcDirect = format_info->frames[bFrameIndex - 1].rotate_flag - 1;
}

static int uvc_media_update(struct uvc_device_info *info){
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
	VPSS_CHN_ATTR_S stVpssChnAttr;
	CVI_U8 u8VencInitStatus = pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8InitStatus;
	int iSensor = g_xSS.iUvcSensor;

	struct uvc_format_info_st uvc_format_info;
	struct uvc_frame_info_st uvc_frame_info;
	uvc_get_video_format_info(&uvc_format_info);
	uvc_get_video_frame_info(&uvc_frame_info);

	switch(uvc_format_info.format_index){
	case YUYV_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_YUYV;
		break;
	case NV21_FORMAT_INDEX:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	default:
		enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	}


	switch(uvc_format_info.format_index){
	case MJPEG_FORMAT_INDEX:
		enType = PT_MJPEG;
		break;
	case H264_FORMAT_INDEX:
		enType = PT_H264;
		break;
	default:
		enType = PT_MJPEG;
		break;
	}

	if(u8VencInitStatus == 1) {
		MEDIA_VIDEO_VencDeInit(pstVencCfg);
		aos_debug_printf("venc chn %d deinit\n", info->video.venc_channel);
	}

	if (uvc_frame_info.width == 0)
	{
		uvc_frame_info.width = UVC_INIT_WIDTH;
		uvc_frame_info.height = UVC_INIT_HEIGHT;
	}

	CVI_VPSS_GetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);
	stVpssChnAttr.enPixelFormat = enPixelFormat;
	if (g_xSS.iUvcDirect == UVC_ROTATION_0)
	{
		stVpssChnAttr.u32Width = uvc_frame_info.height;
		stVpssChnAttr.u32Height = uvc_frame_info.width;
		CVI_VPSS_SetChnRotation(iSensor, 0, ROTATION_90);
	}
	else
	{
		stVpssChnAttr.u32Width = uvc_frame_info.width;
		stVpssChnAttr.u32Height = uvc_frame_info.height;
		CVI_VPSS_SetChnRotation(iSensor, 0, ROTATION_0);
	}
	if ((g_xSS.iUvcDirect == UVC_ROTATION_270 && g_xSS.iUvcSensor == DEFAULT_SNR4UVC) ||
		(g_xSS.iUvcDirect == UVC_ROTATION_90 && g_xSS.iUvcSensor != DEFAULT_SNR4UVC) ||
		(g_xSS.iUvcDirect == UVC_ROTATION_0 && g_xSS.iUvcSensor != DEFAULT_SNR4UVC))
	{
		stVpssChnAttr.bFlip = (DEFAULT_SNR4UVC == 0 ? CVI_TRUE : CVI_FALSE);
		stVpssChnAttr.bMirror = (DEFAULT_SNR4UVC == 0 ? CVI_TRUE : CVI_FALSE);
	}
	else
	{
		stVpssChnAttr.bFlip = (DEFAULT_SNR4UVC == 0 ? CVI_FALSE : CVI_TRUE);
		stVpssChnAttr.bMirror = (DEFAULT_SNR4UVC == 0 ? CVI_FALSE : CVI_TRUE);
	}
#if (USE_RENT_ENGINE)
	if (g_xSS.iSnapImageFace)
	{
		stVpssChnAttr.u32Width = 320;
		stVpssChnAttr.u32Height = 320;
	}
#endif // USE_RENT_ENGINE
	CVI_VPSS_SetChnAttr(info->video.vpss_group, info->video.vpss_channel, &stVpssChnAttr);

    VPSS_CROP_INFO_S pstCropInfo;
    CVI_VPSS_GetChnCrop(info->video.vpss_group, info->video.vpss_channel, &pstCropInfo);
    if (stVpssChnAttr.u32Width * 3 / 4 == stVpssChnAttr.u32Height)
    {
    	pstCropInfo.bEnable = CVI_FALSE;
    }
    else
    {
#if (UVC_LANDSCAPE == 0)
    	int real_width = CLR_CAM_WIDTH * UVC_CROP_RESIZE;
    	int real_height = CLR_CAM_WIDTH * stVpssChnAttr.u32Height / stVpssChnAttr.u32Width * UVC_CROP_RESIZE;
#else
		int real_width = CLR_CAM_HEIGHT * uvc_frame_info.height / uvc_frame_info.width;
		int real_height = CLR_CAM_HEIGHT;
#endif
		pstCropInfo.bEnable = CVI_TRUE;
		pstCropInfo.stCropRect.s32X = CLR_CAM_WIDTH > real_width ? ((CLR_CAM_WIDTH - real_width) / 2) : 0;
		pstCropInfo.stCropRect.s32Y = CLR_CAM_HEIGHT > real_height ? ((CLR_CAM_HEIGHT - real_height) / 2) : 0;
		pstCropInfo.stCropRect.u32Width = real_width;
		pstCropInfo.stCropRect.u32Height = real_height;
    }
#if (USE_RENT_ENGINE)
    if (g_xSS.iSnapImageFace)
    {
    	int real_width = 0;
    	int real_height = 0;
    	int w = (g_xSS.note_data_face.right - g_xSS.note_data_face.left) * 2;
    	int h = (g_xSS.note_data_face.bottom - g_xSS.note_data_face.top) * 2;
    	int cx = g_xSS.note_data_face.left*2 + w / 2;
    	int cy = g_xSS.note_data_face.top*2 + h / 2;
    	int w2 = w * 1.2 + 200;
    	int h2 = h * 1.2 + 200;
    	int x2 = cx - w2/2;
    	int y2 = cy - h2/2;
    	real_width = y2;
    	y2 = (900 - x2 - w2) + (1200 - 900) / 2;
    	x2 = real_width;

    	real_width = h2;
    	real_height = w2;
    	x2 = CLR_CAM_WIDTH - x2 - real_width;
    	y2 = CLR_CAM_HEIGHT - y2 - real_height;
    	if (x2 < 0)
    		x2 = 0;
    	if (y2 < 0)
    		y2 = 0;
    	int x3 = x2 + real_width;
    	int y3 = y2 + real_height;
    	int balign = 64;
    	x2 = x2 / balign * balign;
    	y2 = y2 / balign * balign;
    	x3 = (x3 + balign - 1) / balign * balign;
    	y3 = (y3 + balign - 1) / balign * balign;

    	if (x3 > CLR_CAM_WIDTH)
    		x3 = CLR_CAM_WIDTH;
    	if (y3 > CLR_CAM_HEIGHT)
    		y3 = CLR_CAM_HEIGHT;
    	real_width = x3 - x2;
    	real_height = y3 - y2;
    	int max_h = real_width > real_height ? real_width: real_height;
    	x2 = x3 - max_h;
    	y2 = y3 - max_h;
    	real_width = max_h;
    	real_height = max_h;

		pstCropInfo.bEnable = CVI_TRUE;
		pstCropInfo.stCropRect.s32X = x2;
		pstCropInfo.stCropRect.s32Y = y2;
		pstCropInfo.stCropRect.u32Width = real_width;
		pstCropInfo.stCropRect.u32Height = real_height;
    }
#endif // USE_RENT_ENGINE
    CVI_VPSS_SetChnCrop(info->video.vpss_group, info->video.vpss_channel, &pstCropInfo);

	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Width = uvc_frame_info.width;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16Height = uvc_frame_info.height;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u16EnType = enType;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16BitRate = (enType == PT_MJPEG)?UVC_MJPEG_BITRATE:UVC_H26X_BITRATE;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16RcMode = (enType == PT_MJPEG)?VENC_RC_MODE_MJPEGCBR:VENC_RC_MODE_H264CBR;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8ModId = CVI_ID_VPSS;
	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8DevId = info->video.vpss_group;
   	pstVencCfg->pstVencChnCfg[info->video.venc_channel].stChnParam.u8DevChnid = info->video.vpss_channel;
	if (g_xSS.iUvcBitrate > 0 && enType == PT_MJPEG)
	{
		pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16BitRate = g_xSS.iUvcBitrate;
		if (g_xSS.iUvcSensor != DEFAULT_SNR4UVC)
		{
			pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16BitRate = g_xSS.iUvcBitrate / 2;
		}
	}
	if (g_xSS.iUvcFps > 0 && g_xSS.iUvcFps <= 20)
		camera_set_vi_fps(iSensor, g_xSS.iUvcFps);
	if (uvc_frame_info.width > 0)
	{
		printf("uvc(%dx%d),%dbr\n", uvc_frame_info.width, uvc_frame_info.height, 
			pstVencCfg->pstVencChnCfg[info->video.venc_channel].stRcParam.u16BitRate);
	}

#if (UVC_ENC_TYPE == 2)	
	pstVencCfg->pstVencChnCfg[UVC_VENC_H26X_CHN].stChnParam.u8DevId = iSensor;
	pstVencCfg->pstVencChnCfg[UVC_VENC_H26X_CHN].stChnParam.u16Width = uvc_frame_info.width;
	pstVencCfg->pstVencChnCfg[UVC_VENC_H26X_CHN].stChnParam.u16Height = uvc_frame_info.height;
	// pstVencCfg->pstVencChnCfg[UVC_VENC_H26X_CHN].stChnParam.u16EnType = H26X_TYPE;
	pstVencCfg->pstVencChnCfg[UVC_VENC_H26X_CHN].stRcParam.u16BitRate = UVC_H26X_BITRATE;
	// pstVencCfg->pstVencChnCfg[UVC_VENC_H26X_CHN].stRcParam.u16RcMode = (H26X_TYPE == PT_H265) ? VENC_RC_MODE_H265CBR : VENC_RC_MODE_H264CBR;
#endif // UVC_ENC_TYPE

	if(MJPEG_FORMAT_INDEX == uvc_format_info.format_index || H264_FORMAT_INDEX == uvc_format_info.format_index) {
		if (MEDIA_VIDEO_VencInit(pstVencCfg) == CVI_SUCCESS)
		{
			aos_debug_printf("venc chn %d init\n", info->video.venc_channel);
		}
		else
		{
			aos_debug_printf("chn init fail %d\n", info->video.venc_channel);
		}
    }
    return 0;
}

struct uvc_device_info *uvc_container_of_ep(uint8_t ep)
{
    for (int i = 0; i < USBD_UVC_NUM; i++) {
        if(ep == uvc[i].ep)
            return &uvc[i];
    }
    return NULL;
}

struct uvc_device_info *uvc_container_of_vs_intf(uint8_t intf)
{
    for (int i = 0; i < USBD_UVC_NUM; i++) {
        if(intf == uvc[i].vs_intf.intf_num)
            return &uvc[i];
    }
    return NULL;
}

static void uvc_streaming_on(uint8_t intf, int is_on) {
    struct uvc_device_info *uvc = uvc_container_of_vs_intf(intf);
	USB_LOG_INFO("streaming %s\n", is_on ? "on" : "off");

	if(is_on && is_media_info_update(uvc)) {
		uvc->update_flag = 1;
	}

	if (is_on)
	{
		g_xSS.bUVCRunning = 1;
		g_xSS.rLastSenseCmdTime = aos_now_ms();
	}
	else
		g_xSS.bUVCRunning = 0;

	uvc->header_flip = false;
	uvc->xfer_flag = false;
	uvc->streaming_on = is_on;
}

void uvc_set_reinit_flag()
{
	uvc->update_flag = 1;
}

#if 0
void usbd_configure_done_callback(void)
{
    /* no out ep, so do nothing */
}
#endif

static void uvc_setup_class_control(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

static void uvc_setup_class_streaming(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

static void uvc_data_out(struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    aos_debug_printf("%s:%d\n", __FUNCTION__, __LINE__);
}

#if CONFIG_USB_BULK_UVC
static void uvc_tx_complete(uint8_t ep, uint32_t nbytes)
{
	// aos_debug_printf("%d bytes of data sent at ep(%d)\n", nbytes, ep);
	struct uvc_device_info *uvc = uvc_container_of_ep(ep);
	tx_frm_idx++;
	uvc->xfer_flag = false;
	//aos_work_run(&uvc_workqueue, &uvc_frame_submmit);
}

static void usbd_video_frame_submmit(void *args) {
	struct uvc_device_info *uvc = (struct uvc_device_info *)args;
	uint32_t idx = FRM_BUFFER_GET_IDX(tx_frm_idx);

	if (!uvc->streaming_on || uvc->xfer_flag) {
		return;
	}

	if(tx_frm_idx < rx_frm_idx && frm_sz[idx]>0) {
		uvc->xfer_flag = true;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc + idx * DEFAULT_FRAME_SIZE, frm_sz[idx]);
	}
}
#else
static void uvc_tx_complete(uint8_t ep, uint32_t nbytes)
{
    // aos_debug_printf("%d bytes of data sent at ep(%d)\n", nbytes, ep);
    struct uvc_device_info *uvc = uvc_container_of_ep(ep);
	uint32_t data_len = 0;

    uvc->xfer_offset += nbytes;
	if (uvc->xfer_len > nbytes) {
		uvc->xfer_len -= nbytes;
	} else {
		uvc->xfer_len = 0;
	}

	if (uvc->xfer_len > 0) {
		data_len = uvc->xfer_len < MAX_PAYLOAD_SIZE ? uvc->xfer_len : MAX_PAYLOAD_SIZE;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc + uvc->xfer_offset, data_len);
	} else {
		uvc->xfer_flag = false;
		uvc->xfer_offset = 0;
		uvc->xfer_len = 0;
	}
}
#endif

static uint32_t uvc_payload_fill(struct uvc_device_info *uvc, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *out_len)
{
    uint32_t packets;
    uint32_t last_packet_size;
    uint32_t picture_pos = 0;
    uint8_t uvc_header[12] = { 0x0c, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint32_t size_uvc_header = sizeof(uvc_header);
    uint32_t size_per_packet = MAX_PAYLOAD_SIZE;
    uint32_t size_payload = size_per_packet - size_uvc_header;

    if (size_payload > 10240) {
        USB_LOG_ERR("the size of payload is too long!!!!\n");
    }

	if (input_len + size_uvc_header > DEFAULT_FRAME_SIZE) {
		USB_LOG_ERR("input_len + size_uvc_header (%u) > DEFAULT_FRAME_SIZE (%u)\n",
			input_len + size_uvc_header, DEFAULT_FRAME_SIZE);
		return 0;
	}

	if (!uvc->header_flip) {
		uvc_header[1] = 0x8c;
	}
	uvc->header_flip = !uvc->header_flip;

    // The following equals to packets = roundup(input_len / size_payload)
    packets = (input_len + size_payload - 1) / (size_payload);
    last_packet_size = input_len - ((packets - 1) * size_payload) + size_uvc_header;

    for (size_t i = 0; i < packets; i++) {
        output[size_per_packet* i] = uvc_header[0];
        output[size_per_packet * i + 1] = uvc_header[1];
        if (i == (packets - 1)) {
            memcpy(&output[size_uvc_header + size_per_packet * i],
                &input[picture_pos], last_packet_size - size_uvc_header);
            output[size_per_packet * i + 1] |= (1 << 1);
        } else {
            memcpy(&output[size_uvc_header + size_per_packet * i],
                &input[picture_pos], size_payload);
            picture_pos += size_payload;
        }
    }

    *out_len = (input_len + size_uvc_header * packets);
    return packets;
}

void usbd_video_commit_set_cur(struct video_probe_and_commit_controls *commit)
{
	uvc_parse_media_info(commit->bFormatIndex, commit->bFrameIndex);
    aos_debug_printf("commit format idx:%d, frame idx:%d\n", commit->bFormatIndex, commit->bFrameIndex);
}

static uvc_event_callbacks_t uvc_evt_callbks = {
    .uvc_event_stream_on = uvc_streaming_on,
    .uvc_event_setup_class_control = uvc_setup_class_control,
    .uvc_event_setup_class_streaming = uvc_setup_class_streaming,
    .uvc_event_data_out = uvc_data_out,
    .uvc_event_tx_complete = uvc_tx_complete,
};

static void vedio_streaming_send(struct uvc_device_info *uvc)
{
	int i, ret = 0;
	uint32_t data_len = 0;
	uint32_t buf_len = 0, buf_len_stride = 0, packets = 0;
	struct uvc_format_info_st uvc_format_info;
    VENC_STREAM_S stStream = {0},*pstStream= &stStream;
	VENC_PACK_S *ppack;
	VIDEO_FRAME_INFO_S stVideoFrame, *pstVideoFrame=&stVideoFrame;
	VPSS_CHN_ATTR_S stChnAttr,*pstChnAttr = &stChnAttr;
	static int print_flag = 0;
#if (UVC_ENC_TYPE == 0)
	// static int skip_count = 0;
#elif (UVC_ENC_TYPE == 2 && USE_USB_XN_PROTO)
    VENC_STREAM_S stH26XStream = {0},*pstH26XStream= &stH26XStream;
	static uint32_t frameCnt = 0;
#endif // UVC_ENC_TYPE

	if(uvc->update_flag){
		uvc_media_update(uvc);
// #if (UVC_ENC_TYPE == 0)
// 		if (g_xSS.iUvcSensor != DEFAULT_SNR4UVC && uvc->update_flag)
// 			skip_count = 1;
// #endif
		uvc->update_flag = 0;
	}

	uvc_get_video_format_info(&uvc_format_info);
	switch(uvc_format_info.format_index) {
	case H264_FORMAT_INDEX:
	case MJPEG_FORMAT_INDEX:
#if (UVC_ENC_TYPE == 2 && USE_USB_XN_PROTO)
		if (g_xSS.iForceVencIDR > 0)
		{
			MEDIA_VIDEO_VencRequstIDR(UVC_VENC_H26X_CHN);
			g_xSS.iForceVencIDR--;
		}
		ret = MEDIA_VIDEO_VencGetStream(UVC_VENC_H26X_CHN, pstH26XStream, 2000);
		if(ret == CVI_SUCCESS)
		{
			//printf("h26x data len=0x%x, type=%d, %d\n", buf_len, frameType, (int)aos_now_ms());
			buf_len = xndsParseH264CviStream((void*)pstH26XStream);
			if (buf_len < 0)
			{
				//overflow
				buf_len = 0;
				MEDIA_VIDEO_VencReleaseStream(UVC_VENC_H26X_CHN,pstH26XStream);
				aos_msleep(1);
        		return;
			}
			frameCnt++;
			buf_len = xndsMakeMediaPacket(frameCnt, media_buffer, (void*)pstH26XStream);
			MEDIA_VIDEO_VencReleaseStream(UVC_VENC_H26X_CHN,pstH26XStream);
		}
		else
		{
			if (print_flag ++ < 8)
				printf("h26x get fail\n");
			buf_len = 0;
		}
#endif // UVC_ENC_TYPE

		ret = MEDIA_VIDEO_VencGetStream(uvc->video.venc_channel, pstStream, 2000);
		if(ret != CVI_SUCCESS){
			aos_msleep(1);
			return;
		}

		for (i = 0; i < pstStream->u32PackCount; ++i)
		{
			ppack = &pstStream->pstPack[i];
			if (buf_len + (ppack->u32Len - ppack->u32Offset) < DEFAULT_FRAME_SIZE)
			{
#if (UVC_ENC_TYPE == 2 && USE_USB_XN_PROTO)
				if (i == 0)
				{
					memcpy(media_buffer, ppack->pu8Addr + ppack->u32Offset, H26X_FRAME_OFFSET);
					memcpy(&media_buffer[H26X_FRAME_OFFSET + buf_len], ppack->pu8Addr + ppack->u32Offset + H26X_FRAME_OFFSET, ppack->u32Len - ppack->u32Offset - H26X_FRAME_OFFSET);
					buf_len += (ppack->u32Len - ppack->u32Offset);
				}
				else
				{
					memcpy(media_buffer + buf_len, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
					buf_len += (ppack->u32Len - ppack->u32Offset);
				}
#else // UVC_ENC_TYPE
				memcpy(media_buffer + buf_len,
						ppack->pu8Addr + ppack->u32Offset,
						ppack->u32Len - ppack->u32Offset);
				buf_len += (ppack->u32Len - ppack->u32Offset);
#endif // UVC_ENC_TYPE
			} else {
				printf("venc buf_len oversize\n");
				MEDIA_VIDEO_VencReleaseStream(uvc->video.venc_channel, pstStream);
				aos_msleep(1);
				return;
			}
		}
		if (print_flag ++ < 8)
		{
			if (print_flag == 8)
				aos_debug_printf("enc data len=%d, %d\n", buf_len, (int)aos_now_ms());
		}
		ret = MEDIA_VIDEO_VencReleaseStream(uvc->video.venc_channel, pstStream);
		if(ret != CVI_SUCCESS)
			aos_debug_printf("MEDIA_VIDEO_VencReleaseStream failed\n");
		break;
	case YUYV_FORMAT_INDEX:
		ret = CVI_VPSS_GetChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame, -1);
		if(ret != CVI_SUCCESS){
			printf("CVI_VPSS_GetChnFrame failed\n");
			aos_msleep(1);
			return;
		}
		CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

		pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *)pstVideoFrame->stVFrame.u64PhyAddr[0];
		data_len = pstChnAttr->u32Width * 2;
		for (i = 0;i < (pstChnAttr->u32Height); ++i)
		{
			memcpy(media_buffer + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] +
				buf_len_stride, data_len);

			buf_len += pstChnAttr->u32Width * 2;
			buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
		}
		pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

		ret = CVI_VPSS_ReleaseChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame);
		if(ret != CVI_SUCCESS)
			printf("CVI_VPSS_ReleaseChnFrame failed\n");
		break;
	case NV21_FORMAT_INDEX:
		ret = CVI_VPSS_GetChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame, -1);
		if(ret != CVI_SUCCESS){
			printf("CVI_VPSS_GetChnFrame failed\n");
			aos_msleep(1);
			return;
		}
		CVI_VPSS_GetChnAttr(uvc->video.vpss_group, uvc->video.vpss_channel, pstChnAttr);

		pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *) pstVideoFrame->stVFrame.u64PhyAddr[0];
		data_len = pstChnAttr->u32Width;
		for (i = 0;i < ((pstChnAttr->u32Height * 3) >>1); ++i)
		{
			memcpy(media_buffer + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] +
				buf_len_stride, data_len);
			buf_len += pstChnAttr->u32Width;
			buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
		}
		pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

		ret = CVI_VPSS_ReleaseChnFrame(uvc->video.vpss_group, uvc->video.vpss_channel, pstVideoFrame);
		if(ret != CVI_SUCCESS)
			printf("CVI_VPSS_ReleaseChnFrame failed\n");
		break;
	default:
		break;
	}

#if CONFIG_USB_BULK_UVC
	packets = uvc_payload_fill(uvc, media_buffer, buf_len,
			uvc->packet_buffer_uvc + FRM_BUFFER_GET_IDX(rx_frm_idx) * DEFAULT_FRAME_SIZE,
			&data_len);
	frm_sz[FRM_BUFFER_GET_IDX(rx_frm_idx)] = data_len;
	rx_frm_idx++;
#else
	packets = uvc_payload_fill(uvc, media_buffer, buf_len, uvc->packet_buffer_uvc, &data_len);
#endif
	buf_len = 0;
	buf_len_stride = 0;

    /* dwc2 must use this method */
	if (uvc->streaming_on && packets > 0) {
		uvc->xfer_offset = 0;
		uvc->xfer_len = data_len;
	#if CONFIG_USB_BULK_UVC
		aos_work_run(&uvc_workqueue, &uvc_frame_submmit);
	#else
		uvc->xfer_flag = true;
		usbd_ep_start_write(uvc->ep, uvc->packet_buffer_uvc, MAX_PAYLOAD_SIZE);
		while(uvc->streaming_on && uvc->xfer_flag) {
			aos_task_yield();
			aos_msleep(1);
		}
	#endif
	}
}

static void *send_to_uvc()
{
    uint32_t i = 0;

    while (av_session_init_flag) {
		if (rhino_atomic_get(&uvc_pause_flag)) {
			rhino_atomic_inc(&uvc_pause_done);
			while (rhino_atomic_get(&uvc_pause_done)) {
				aos_msleep(1);
			}

			for (i = 0; i < USBD_UVC_NUM; i++) {
				uvc[i].update_flag = 1;
			}

		}

		for (i = 0; i < USBD_UVC_NUM; i++) {
			// printf("uvc[%d].streaming_on:%d\n", i, uvc[i].streaming_on);
			if(uvc[i].streaming_on) {
				g_xSS.rUvcFrameTime = aos_now_ms();
				vedio_streaming_send(&uvc[i]);
			}
			else {
				aos_msleep(1);
			}
		}
    }

    return 0;
}

static uint8_t *uvc_descriptor = NULL;

static void uvc_desc_register_cb()
{
	uvc_destroy_descriptor(uvc_descriptor);
}

static void uvc_desc_register()
{
	uint32_t desc_len;

	for(uint8_t i = 0; i < USBD_UVC_NUM; i++) {
		uvc[i].ep = comp_get_available_ep(1);
		USB_LOG_INFO("uvc[%d].ep:%#x\n", i, uvc[i].ep);
	}
	uvc[0].interface_nums = comp_get_interfaces_num();
	USB_LOG_INFO("interface_nums:%d\n", uvc[0].interface_nums);

	uvc_descriptor = uvc_build_descriptors(uvc, &desc_len, USBD_UVC_NUM);
	comp_register_descriptors(USBD_TYPE_UVC, uvc_descriptor, desc_len, uvc[0].interface_nums, uvc_desc_register_cb);

	// printf("multi uvc num:%u\n", USBD_UVC_NUM);
	// printf("MAX_PAYLOAD_SIZE:%u\n", MAX_PAYLOAD_SIZE);
    for (uint8_t i = 0; i < USBD_UVC_NUM; i++) {
        usbd_add_interface(usbd_video_control_init_intf(&uvc[i].vc_intf, INTERVAL, MAX_FRAME_SIZE, MAX_PAYLOAD_SIZE));
        usbd_add_interface(usbd_video_stream_init_intf(&uvc[i].vs_intf, INTERVAL, MAX_FRAME_SIZE, MAX_PAYLOAD_SIZE));
        usbd_add_endpoint(usbd_video_init_ep(&uvc[i].video_in_ep, uvc[i].ep, NULL));
	#if CONFIG_USB_BULK_UVC
		aos_workqueue_create_ext(&uvc_workqueue, "uvc_submmit", 15, 4096);
		aos_work_init(&uvc_frame_submmit, usbd_video_frame_submmit, &uvc[i], 5);
		uvc[i].packet_buffer_uvc = (uint8_t *)usb_iomalloc(DEFAULT_FRAME_SIZE * FRM_BUFFER_LEN);
		memset(uvc[i].packet_buffer_uvc, 0, DEFAULT_FRAME_SIZE * FRM_BUFFER_LEN);
	#else
		uvc[i].packet_buffer_uvc = (uint8_t *)usb_iomalloc(DEFAULT_FRAME_SIZE);
		memset(uvc[i].packet_buffer_uvc, 0, DEFAULT_FRAME_SIZE);
	#endif
    }
    usbd_video_register_uvc_callbacks(&uvc_evt_callbks);

}

int uvc_init(void)
{
    char threadname[64] = {0};
	struct sched_param param;
	pthread_attr_t pthread_attr;
	pthread_t pthreadId = 0;

	uvc_desc_register();

	av_session_init_flag = CVI_TRUE;
	aos_event_new(&_gslUvcEvent, 0);
	param.sched_priority = 31;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	pthread_attr_setschedparam(&pthread_attr, &param);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 16*1024);
	pthread_create(&pthreadId,&pthread_attr,send_to_uvc,NULL);
	snprintf(threadname,sizeof(threadname),"uvc_send%d",0);
	pthread_setname_np(pthreadId, threadname);

	return 0;
}

int uvc_deinit(void)
{
	av_session_init_flag = CVI_FALSE;
	aos_msleep(100);
	aos_event_free(&_gslUvcEvent);

    for (int i = 0; i < USBD_UVC_NUM; i++) {
	if (uvc[i].packet_buffer_uvc) {
		usb_iofree(uvc[i].packet_buffer_uvc);
		uvc[i].packet_buffer_uvc = NULL;
	}
	}

	return 0;
}

void uvc_switch(int argc, char** argv)
{
    uint32_t id = 0;
	if(argc < 5){
		printf("Usage: %s [UVC_ID] [VENC_ID] [VPSS_GrpID] [VPSS_ChnID]\n\n", argv[0]);
		return;
	}

	rhino_atomic_inc(&uvc_pause_flag);
	while (!rhino_atomic_get(&uvc_pause_done)) {
		aos_msleep(1);
	}

    id = atoi(argv[1]);
	uvc[id].video.venc_channel = atoi(argv[2]);
	uvc[id].video.vpss_group = atoi(argv[3]);
	uvc[id].video.vpss_channel = atoi(argv[4]);

	rhino_atomic_dec(&uvc_pause_flag);
	rhino_atomic_dec(&uvc_pause_done);
}

void uvc_get_info(int argc, char** argv)
{
	printf("UVC-0: UVC_VENC_CHN:%d, UVC_VPSS_GRP:%d, UVC_VPSS_CHN:%d\n",
	        uvc[0].video.venc_channel, uvc[0].video.vpss_group, uvc[0].video.vpss_channel);
#if USBD_UVC_NUM > 1
	printf("UVC-1: UVC_VENC_CHN:%d, UVC_VPSS_GRP:%d, UVC_VPSS_CHN:%d\n",
	        uvc[1].video.venc_channel, uvc[1].video.vpss_group, uvc[1].video.vpss_channel);
#endif
#if USBD_UVC_NUM > 2
	printf("UVC-2: UVC_VENC_CHN:%d, UVC_VPSS_GRP:%d, UVC_VPSS_CHN:%d\n",
	        uvc[2].video.venc_channel, uvc[2].video.vpss_group, uvc[2].video.vpss_channel);
#endif
}

ALIOS_CLI_CMD_REGISTER(uvc_switch, uvc_switch, uvc_switch);
ALIOS_CLI_CMD_REGISTER(uvc_get_info, uvc_get_info, uvc_get_info);
