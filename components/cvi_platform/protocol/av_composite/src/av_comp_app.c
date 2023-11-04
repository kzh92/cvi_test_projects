#include <stdio.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include "usbd_core.h"
#include "usbd_video.h"
#include "media_video.h"
// #include "video.h"
#include "cvi_venc.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include "av_comp_descriptor.h"
#include "uac.h"
#include "uac_descriptor.h"
#include <core/core_rv64.h>
#include "appdef.h"
#include "settings.h"
#include "cam_base.h"

#define USB_OTG_HS                     0U
#define USB_OTG_HS_IN_FULL             1U
#define USB_OTG_FS                     3U

#define VIDEO_IN_EP 0x81

#define WIDTH  (unsigned int)(1280)
#define HEIGHT (unsigned int)(720)

#define CAM_FPS        (30)
#define INTERVAL       (unsigned long)(10000000 / CAM_FPS)
#define MAX_FRAME_SIZE (unsigned long)(WIDTH * HEIGHT * 2 / 3)
#define DEFAULT_FRAME_SIZE (unsigned long)(WIDTH * HEIGHT * 2 / 3)

#define UVC_VENC_CHN   (0)
#define UVC_VPSS_CHN   (0)
#define UVC_VPSS_GRP   (0)

#define MJPEG_FORMAT_INDEX  (1)
#define H264_FORMAT_INDEX   (2)
#define YUYV_FORMAT_INDEX   (3)
#define NV21_FORMAT_INDEX   (4)

volatile bool tx_flag = CVI_FALSE;
volatile unsigned int uvc_update = 0;
static int uvc_session_init_flag = CVI_FALSE;
static aos_event_t _gslUvcEvent;
static volatile bool g_uvc_event_flag;

static uint8_t *packet_buffer_uvc;

CVI_S32 is_media_info_update();
void uvc_parse_media_info(uint8_t bFormatIndex, uint8_t bFrameIndex);
int uvc_media_update();

static struct usbd_endpoint audio_in_ep = {
    .ep_cb = usbd_audio_in_callback,
    .ep_addr = AUDIO_IN_EP
};

static struct usbd_endpoint audio_out_ep = {
    .ep_cb = usbd_audio_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

#if 0
static struct uvc_frame_info_st yuy2_frame_info[] = {
    {1, 800, 600, 30, 0},
    {2, 640, 360, 30, 0},
    {3, 400, 300, 30, 0},
    {5, 480, 320, 30, 0},
    {6, 480, 360, 30, 0},
    {7, 1280, 720, 30, 0},
    // {8, 1920, 1080, 30, 0},
    // {9, 960, 640, 15, 0},
    // {10, 320, 240, 15, 0},
};
#endif

static struct uvc_frame_info_st mjpeg_frame_info[] = {
    UVC_RES_DEFINE
};

#if 0
static struct uvc_frame_info_st h264_frame_info[] = {
    {1, 800, 600, 30, 0},
    {2, 1280, 720, 30, 0},
    {3, 640, 480, 30, 0},
    {4, 400, 300, 30, 0},
    {5, 1920, 1080, 30, 0},
};

static struct uvc_frame_info_st nv21_frame_info[] = {
    {1, 800, 600, 30, 0},
    {2, 1280, 720, 30, 0},
    {3, 640, 480, 30, 0},
};
#endif

static struct uvc_format_info_st uvc_format_info[] = {
    {MJPEG_FORMAT_INDEX, UVC_FORMAT_MJPEG, 1, ARRAY_SIZE(mjpeg_frame_info), mjpeg_frame_info},
#if 0
    {H264_FORMAT_INDEX, UVC_FORMAT_H264, 1, ARRAY_SIZE(h264_frame_info), h264_frame_info},
    {YUYV_FORMAT_INDEX, UVC_FORMAT_YUY2, 1, ARRAY_SIZE(yuy2_frame_info), yuy2_frame_info},
    {NV21_FORMAT_INDEX, UVC_FORMAT_NV21, 1, ARRAY_SIZE(nv21_frame_info), nv21_frame_info},
#endif
};

CVI_S32 is_media_info_update(){
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	VENC_CHN_ATTR_S stVencChnAttr,*pstVencChnAttr = &stVencChnAttr;
	VPSS_CHN_ATTR_S stVpssChnAttr, *pstVpssChnAttr = &stVpssChnAttr;
	PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
	CVI_U8 u8VencInitStatus = pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stChnParam.u8InitStatus;
	int iUvcSensor = DEFAULT_SNR4UVC;

	struct uvc_format_info_st uvc_format_info;
	struct uvc_frame_info_st uvc_frame_info;

	uvc_get_video_format_info(&uvc_format_info);
	uvc_get_video_frame_info(&uvc_frame_info);

	CVI_VPSS_GetChnAttr(iUvcSensor, UVC_VPSS_CHN, pstVpssChnAttr);

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

	CVI_VENC_GetChnAttr(UVC_VENC_CHN, pstVencChnAttr);
	if((pstVencChnAttr->stVencAttr.enType != enType)||
		(pstVencChnAttr->stVencAttr.u32PicWidth != uvc_frame_info.width) ||
		(pstVencChnAttr->stVencAttr.u32PicHeight != uvc_frame_info.height))
		return CVI_TRUE;

	return CVI_FALSE;

}

void uvc_parse_media_info(uint8_t bFormatIndex, uint8_t bFrameIndex)
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
}

int uvc_media_update(){
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	PARAM_VENC_CFG_S *pstVencCfg = PARAM_getVencCtx();
	VPSS_CHN_ATTR_S stVpssChnAttr;
	CVI_U8 u8VencInitStatus = pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stChnParam.u8InitStatus;
	int iSensor = DEFAULT_SNR4UVC;

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

	if(u8VencInitStatus == 1)
		MEDIA_VIDEO_VencDeInit(pstVencCfg);

	CVI_VPSS_GetChnAttr(iSensor, UVC_VPSS_CHN, &stVpssChnAttr);
	stVpssChnAttr.enPixelFormat = enPixelFormat;
	if (g_xSS.iUvcDirect == UVC_ROTATION_0)
	{
		stVpssChnAttr.u32Width = uvc_frame_info.height;
		stVpssChnAttr.u32Height = uvc_frame_info.width;
		CVI_VPSS_SetChnRotation(DEFAULT_SNR4UVC, 0, ROTATION_90);
	}
	else
	{
		stVpssChnAttr.u32Width = uvc_frame_info.width;
		stVpssChnAttr.u32Height = uvc_frame_info.height;
		CVI_VPSS_SetChnRotation(DEFAULT_SNR4UVC, 0, ROTATION_0);
	}
	if (g_xSS.iUvcDirect == UVC_ROTATION_270 || g_xSS.iUvcSensor != DEFAULT_SNR4UVC)
	{
		stVpssChnAttr.bFlip = (DEFAULT_SNR4UVC == 0 ? CVI_TRUE : CVI_FALSE);
		stVpssChnAttr.bMirror = (DEFAULT_SNR4UVC == 0 ? CVI_TRUE : CVI_FALSE);
	}
	else
	{
		stVpssChnAttr.bFlip = (DEFAULT_SNR4UVC == 0 ? CVI_FALSE : CVI_TRUE);
		stVpssChnAttr.bMirror = (DEFAULT_SNR4UVC == 0 ? CVI_FALSE : CVI_TRUE);
	}
	VPSS_CROP_INFO_S pstCropInfo;
    MEDIA_CHECK_RET(CVI_VPSS_GetChnCrop(iSensor, UVC_VPSS_CHN, &pstCropInfo), "CVI_VPSS_GetChnCrop failed\n");
    if (stVpssChnAttr.u32Width * 3 / 4 == stVpssChnAttr.u32Height)
    {
    	pstCropInfo.bEnable = CVI_FALSE;
    }
    else
    {
    	int real_width = CLR_CAM_WIDTH * UVC_CROP_RESIZE;
    	int real_height = CLR_CAM_WIDTH * stVpssChnAttr.u32Height / stVpssChnAttr.u32Width * UVC_CROP_RESIZE;
		pstCropInfo.bEnable = CVI_TRUE;
		pstCropInfo.stCropRect.s32X = CLR_CAM_WIDTH > real_width ? ((CLR_CAM_WIDTH - real_width) / 2) : 0;
		pstCropInfo.stCropRect.s32Y = CLR_CAM_HEIGHT > real_height ? ((CLR_CAM_HEIGHT - real_height) / 2) : 0;
		pstCropInfo.stCropRect.u32Width = real_width;
		pstCropInfo.stCropRect.u32Height = real_height;
    }
	MEDIA_CHECK_RET(CVI_VPSS_SetChnCrop(iSensor, UVC_VPSS_CHN, &pstCropInfo), "CVI_VPSS_SetChnCrop failed\n");

	CVI_VPSS_SetChnAttr(iSensor,UVC_VPSS_CHN, &stVpssChnAttr);

	pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stChnParam.u8DevId = iSensor;
	pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stChnParam.u16Width = uvc_frame_info.width;
	pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stChnParam.u16Height = uvc_frame_info.height;
	pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stChnParam.u16EnType = enType;
	pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stRcParam.u16BitRate = (enType == PT_MJPEG)?UVC_MJPEG_BITRATE:2048;
	if (g_xSS.iUvcBitrate > 0)
		pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stRcParam.u16BitRate = g_xSS.iUvcBitrate;
	if (g_xSS.iUvcFps > 0 && g_xSS.iUvcFps <= 20)
		camera_set_vi_fps(DEFAULT_SNR4UVC, g_xSS.iUvcFps);
	pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stRcParam.u16RcMode = (enType == PT_MJPEG)?VENC_RC_MODE_MJPEGCBR:VENC_RC_MODE_H264CBR;
	printf("uvc(%dx%d),%dbr\n", uvc_frame_info.width, uvc_frame_info.height, pstVencCfg->pstVencChnCfg[UVC_VENC_CHN].stRcParam.u16BitRate);

	if(MJPEG_FORMAT_INDEX == uvc_format_info.format_index || H264_FORMAT_INDEX == uvc_format_info.format_index)
		MEDIA_VIDEO_VencInit(pstVencCfg);
	return 0;
}

void uvc_streaming_on(int is_on) {
	USB_LOG_INFO("streaming %s\n", is_on ? "on" : "off");
	tx_flag = is_on;

	if(is_on && is_media_info_update())
		uvc_update = 1;

	if (is_on)
	{
		g_xSS.bUVCRunning = 1;
		g_xSS.rLastSenseCmdTime = aos_now_ms();
#if (USE_WHITE_LED == 0)
		if(g_xSS.iCurClrGain > (0xf80 - NEW_CLR_IR_SWITCH_THR))
	    {
	        g_xSS.iUvcSensor = (DEFAULT_SNR4UVC + 1) % 2;
	        camera_set_mono_chrome(1);
	        uvc_update = 2;
	    }
#endif
	}
	else
		g_xSS.bUVCRunning = 0;

	g_uvc_event_flag = false;
}

void uvc_set_reinit_flag()
{
	if(is_media_info_update())
		uvc_update = 2;
}

void usbd_configure_done_callback(void)
{
    /* no out ep, so do nothing */
}

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

static volatile uint32_t offset = 0;
static volatile uint32_t total_len = 0;
static void uvc_tx_complete(uint8_t ep, uint32_t nbytes)
{
    // aos_debug_printf("%d bytes of data sent at ep(%d)\n", nbytes, ep);
	uint32_t data_len = 0;
	offset += nbytes;
	if (total_len > nbytes) {
		total_len -= nbytes;
	} else {
		total_len = 0;
	}

	if (total_len > 0) {
		data_len = total_len < MAX_PAYLOAD_SIZE ? total_len : MAX_PAYLOAD_SIZE;
		usbd_ep_start_write(VIDEO_IN_EP, packet_buffer_uvc + offset, data_len);
	} else {
    	g_uvc_event_flag = false;
		offset = 0;
		total_len = 0;
	}
}

void usbd_video_commit_set_cur(struct video_probe_and_commit_controls *commit)
{
	uvc_parse_media_info(commit->bFormatIndex, commit->bFrameIndex);
//    aos_debug_printf("commit format idx:%d, frame idx:%d\n", commit->bFormatIndex, commit->bFrameIndex);
}

static uvc_event_callbacks_t uvc_evt_callbks = {
    .uvc_event_stream_on = uvc_streaming_on,
    .uvc_event_setup_class_control = uvc_setup_class_control,
    .uvc_event_setup_class_streaming = uvc_setup_class_streaming,
    .uvc_event_data_out = uvc_data_out,
    .uvc_event_tx_complete = uvc_tx_complete,
};

static void *send_to_uvc()
{
    uint32_t out_len, i = 0, ret = 0;
	uint32_t buf_len = 0,buf_len_stride = 0, packets = 0;
	uint8_t *packet_buffer_media = (uint8_t *)aos_ion_malloc(DEFAULT_FRAME_SIZE);
    memset(packet_buffer_media, 0, DEFAULT_FRAME_SIZE);
    memset(packet_buffer_uvc, 0, DEFAULT_FRAME_SIZE);
	extern volatile bool tx_flag;
    VENC_STREAM_S stStream = {0},*pstStream= &stStream;
	VENC_PACK_S *ppack;
	VIDEO_FRAME_INFO_S stVideoFrame, *pstVideoFrame=&stVideoFrame;
	VPSS_CHN_ATTR_S stChnAttr,*pstChnAttr = &stChnAttr;
	struct uvc_format_info_st uvc_format_info;
	int print_flag = 0;
	int skip_count = 0;

    while (uvc_session_init_flag) {
        if (tx_flag) {
			
			if(uvc_update){
				uvc_media_update();
				uvc_get_video_format_info(&uvc_format_info);
				if (g_xSS.iUvcSensor != DEFAULT_SNR4UVC && uvc_update == 2)
					skip_count = 1;
#if (FRM_PRODUCT_TYPE == FRM_DBS3M_XIONGMAI_UAC)
				skip_count = 4;
#endif
				uvc_update = 0;
			}

			
			if(H264_FORMAT_INDEX == uvc_format_info.format_index || 
				MJPEG_FORMAT_INDEX == uvc_format_info.format_index){
				
		        ret = MEDIA_VIDEO_VencGetStream(UVC_VENC_CHN,pstStream,2000);
				if(ret != CVI_SUCCESS){
	//				printf("MEDIA_VIDEO_VencGetStream failed\n");
					aos_msleep(10);
					continue;
				}
				if (skip_count)
				{
					skip_count--;
					MEDIA_VIDEO_VencReleaseStream(UVC_VENC_CHN,pstStream);
#if (USE_WHITE_LED == 0)
					g_xSS.iUVCIRDataReady = 0;
#endif
					continue;
				}
				for (i = 0; i < pstStream->u32PackCount; ++i)
				{
					if(buf_len < DEFAULT_FRAME_SIZE){
						ppack = &pstStream->pstPack[i];
						memcpy(packet_buffer_media, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
						buf_len = (ppack->u32Len - ppack->u32Offset);
					}
					else{
							printf("venc buf_len oversize\n");
							MEDIA_VIDEO_VencReleaseStream(0,pstStream);
							continue;
					}
				}
#if (USE_WHITE_LED == 0 && 0)
				if (g_xSS.iUvcSensor != DEFAULT_SNR4UVC && g_xSS.iUVCIRDataReady == 0)
				{
					MEDIA_VIDEO_VencReleaseStream(UVC_VENC_CHN,pstStream);
					continue;
				}
				g_xSS.iUVCIRDataReady = 0;
#endif
				if (print_flag ++ < 8)
				{
					if (print_flag == 8)
						printf("mjpeg len=%d, %d\n", buf_len, (int)aos_now_ms());
				}
				ret = MEDIA_VIDEO_VencReleaseStream(UVC_VENC_CHN,pstStream);
				if(ret != CVI_SUCCESS)
					printf("MEDIA_VIDEO_VencReleaseStream failed\n");

				}else 
			if(YUYV_FORMAT_INDEX == uvc_format_info.format_index){
				ret = CVI_VPSS_GetChnFrame(UVC_VPSS_GRP, UVC_VPSS_CHN, pstVideoFrame, -1);
				if(ret != CVI_SUCCESS){
//					printf("CVI_VPSS_GetChnFrame failed\n");
					aos_msleep(1);
					continue;
				}
				CVI_VPSS_GetChnAttr(UVC_VPSS_GRP, UVC_VPSS_CHN, pstChnAttr);

				pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *) pstVideoFrame->stVFrame.u64PhyAddr[0];
				int data_len = pstChnAttr->u32Width * 2;
				for (i = 0;i < (pstChnAttr->u32Height); ++i)
				{
					memcpy(packet_buffer_media + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] + 
						buf_len_stride, data_len);

					buf_len += pstChnAttr->u32Width * 2;
					buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
				}
				pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

				ret = CVI_VPSS_ReleaseChnFrame(UVC_VPSS_GRP, UVC_VPSS_CHN, pstVideoFrame);
				if(ret != CVI_SUCCESS)
					printf("CVI_VPSS_ReleaseChnFrame failed\n");
			}else
			if(NV21_FORMAT_INDEX == uvc_format_info.format_index){
				ret = CVI_VPSS_GetChnFrame(UVC_VPSS_GRP, UVC_VPSS_CHN, pstVideoFrame, -1);
				if(ret != CVI_SUCCESS){
//					printf("CVI_VPSS_GetChnFrame failed\n");
					aos_msleep(1);
					continue;
				}
				CVI_VPSS_GetChnAttr(UVC_VPSS_GRP, UVC_VPSS_CHN, pstChnAttr);

				pstVideoFrame->stVFrame.pu8VirAddr[0] = (uint8_t *) pstVideoFrame->stVFrame.u64PhyAddr[0];
				int data_len = pstChnAttr->u32Width;
				for (i = 0;i < ((pstChnAttr->u32Height * 3) >>1); ++i)
				{
					memcpy(packet_buffer_media + buf_len, pstVideoFrame->stVFrame.pu8VirAddr[0] + 
						buf_len_stride, data_len);
					buf_len += pstChnAttr->u32Width;
					buf_len_stride += pstVideoFrame->stVFrame.u32Stride[0];
				}
				pstVideoFrame->stVFrame.pu8VirAddr[0] = NULL;

				ret = CVI_VPSS_ReleaseChnFrame(UVC_VPSS_GRP, UVC_VPSS_CHN, pstVideoFrame);
				if(ret != CVI_SUCCESS)
					printf("CVI_VPSS_ReleaseChnFrame failed\n");
			}

			packets = usbd_video_payload_fill(packet_buffer_media, buf_len, packet_buffer_uvc, &out_len);

			buf_len = 0;
			buf_len_stride = 0;

            /* dwc2 must use this method */
			if (tx_flag && packets > 0) {
				offset = 0;
				total_len = out_len;
				g_uvc_event_flag = true;
				usbd_ep_start_write(VIDEO_IN_EP, packet_buffer_uvc, MAX_PAYLOAD_SIZE);
				while(tx_flag && g_uvc_event_flag) {
					aos_task_yield();
					aos_msleep(1);
				}
			}
#if (UVC_MAX_FPS_TIME != 40)
			aos_msleep(UVC_MAX_FPS_TIME);
#endif
        }else {
			aos_msleep(1);
		}

    }
	
    return 0;
}


static struct usbd_endpoint video_in_ep;
static struct usbd_interface uvc_inf0;
static struct usbd_interface uvc_inf1;
static struct usbd_interface uac_intf0;
static struct usbd_interface uac_intf1;
static struct usbd_interface uac_intf2;
static uint8_t *av_comp_descriptor = NULL;

void usb_av_comp_init()
{
    av_comp_descriptor = av_comp_build_descriptors(uvc_format_info, ARRAY_SIZE(uvc_format_info));

    usbd_desc_register(av_comp_descriptor);
    usbd_add_interface(usbd_video_init_intf(&uvc_inf0, INTERVAL, MAX_FRAME_SIZE, MAX_PAYLOAD_SIZE));
    usbd_add_interface(usbd_video_init_intf(&uvc_inf1, INTERVAL, MAX_FRAME_SIZE, MAX_PAYLOAD_SIZE));
    usbd_add_endpoint(usbd_video_init_ep(&video_in_ep, VIDEO_IN_EP, NULL));
    usbd_video_register_uvc_callbacks(&uvc_evt_callbks);

    usbd_add_interface(usbd_audio_init_intf(&uac_intf0));
    usbd_add_interface(usbd_audio_init_intf(&uac_intf1));
    usbd_add_interface(usbd_audio_init_intf(&uac_intf2));
    usbd_add_endpoint(&audio_in_ep);
    usbd_add_endpoint(&audio_out_ep);

    usbd_audio_add_entity(0x02, AUDIO_CONTROL_FEATURE_UNIT);
    usbd_audio_add_entity(0x05, AUDIO_CONTROL_FEATURE_UNIT);

    usbd_initialize();
}

int MEDIA_AV_Init()
{
    char threadname[64] = {0};
	struct sched_param param;
	pthread_attr_t pthread_attr;
	pthread_t pthreadId = 0;

	// csi_dcache_clean_invalid();
	// csi_dcache_disable();

	usb_av_comp_init();

	MEDIA_UAC_Init();

	packet_buffer_uvc = (uint8_t *)usb_iomalloc(DEFAULT_FRAME_SIZE);

	// Wait until configured
	while (!usb_device_is_configured()) {
		aos_msleep(100);
	}
	
	uvc_session_init_flag = CVI_TRUE;
	aos_event_new(&_gslUvcEvent, 0);
	param.sched_priority = 31;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	pthread_attr_setschedparam(&pthread_attr, &param);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 6*1024);
	pthread_create(&pthreadId,&pthread_attr,send_to_uvc,NULL);
	snprintf(threadname,sizeof(threadname),"uvc_send%d",0);
	pthread_setname_np(pthreadId, threadname);

	return 0;
}

int MEDIA_AV_DeInit()
{
	uvc_session_init_flag = CVI_FALSE;
	aos_msleep(100);
	usbd_deinitialize();
	aos_event_free(&_gslUvcEvent);
	MEDIA_UAC_deInit();
    if (av_comp_descriptor) {
        av_comp_destroy_descriptors(av_comp_descriptor);
    }

	if (packet_buffer_uvc) {
		usb_iofree(packet_buffer_uvc);
		packet_buffer_uvc = NULL;
	}

	return 0;
}

void av_comp_app_init()
{
	MEDIA_AV_Init();
}

void av_comp_app_deinit()
{
	MEDIA_AV_DeInit();
}

ALIOS_CLI_CMD_REGISTER(av_comp_app_init, av_comp_app_init, av_comp_app_init);
ALIOS_CLI_CMD_REGISTER(av_comp_app_deinit, av_comp_app_deinit, av_comp_app_deinit);


