/*
 * @Description: 
 * @version: 
 * @Author: Ricardo Lu<sheng.lu@thundercomm.com>
 * @Date: 2021-06-07 21:49:29
 * @LastEditors: Ricardo Lu
 * @LastEditTime: 2022-02-23 18:27:41
 */
#ifndef __DRAW_RECT_NV12__
#define __DRAW_RECT_NV12__

//#include <stdio.h>
//#include <iostream>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
//typedef unsigned int uint64_t;

//typedef char int8_t;
typedef short int16_t;
typedef int int32_t;

#define     ASCII_WIDTH         (8u)        /* ASCII ���� */
#define     HZ_WIDTH            (16u)       /* ���ֿ��� */
#define     FRONT_HEIGHT        (16u)       /* �ַ��߶� */
#define     ASCII_NUM           (128u)      /* ASCII �ַ����� */
#define     HZK16_FRONT_SIZE    (267616u)   /* �����ֿ��С */
#define     ASCII8_FRONT_SIZE   (4096u)

/* bt.601 �µ� GRB ת yuv */
#define     GET_Y_BT601(R,G,B)  ((uint8_t)(16 +  0.257 * (R) + 0.504 * (G) + 0.098 * (B)))
#define     GET_U_BT601(R,G,B)  ((uint8_t)(128 - 0.148 * (R) - 0.291 * (G) + 0.439 * (B)))
#define     GET_V_BT601(R,G,B)  ((uint8_t)(128 + 0.439 * (R) - 0.368 * (G) - 0.071 * (B)))

/* bt.709 �µ� GRB ת yuv */
#define     GET_Y_BT709(R,G,B)  ((uint8_t)(16   + 0.183 * (R) + 0.614 * (G) + 0.062 * (B)))
#define     GET_U_BT709(R,G,B)  ((uint8_t)(128 - 0.101 * (R) - 0.339 * (G) + 0.439 * (B)))
#define     GET_V_BT709(R,G,B)  ((uint8_t)(128 + 0.439 * (R) - 0.399 * (G) - 0.040 * (B)))


typedef enum
{
	TYPE_YUV422I_UYVY,
	TYPE_YUV422I_YUYV,
	TYPE_YUV420SP_NV12,
	TYPE_YUV420SP_NV21,
	TYPE_YUV422P,
	TYPE_YUV444I,
	TYPE_YUV444P,
	TYPE_YUV420I
}YUVType;

typedef enum
{
	YUV_GREEN,
	YUV_RED,
	YUV_BLUE,
	YUV_PURPLE,
	YUV_DARK_GREEN,
	YUV_YELLOW,
	YUV_LIGHT_BLUE,
	YUV_LIGHT_PURPLE,
	YUV_DARK_BLACK,
	YUV_GRAY,
	YUV_WHITE,
	YUV_COLOR_MAX,
}YUVColor;

typedef struct
{
	uint8_t Y;
	uint8_t U;
	uint8_t V;
}YUVPixColor;

typedef struct
{
	uint16_t x;
	uint16_t y;
}imgPoint;

typedef struct
{
	imgPoint startPoint;
	imgPoint endPoint;
	uint16_t thick;
	YUVColor color;
}lineInfo;

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
}YUVRectangle;

typedef struct
{
	YUVType yuvType;
	uint8_t *imgdata;
	uint16_t width;
	uint16_t height;
}YUVImgInfo;

#ifdef __cplusplus
extern  "C"
{
#endif

void setYUVPix(unsigned char* YBuf, unsigned char * UVBuf, YUVType type, uint16_t width, uint16_t height, imgPoint point, YUVColor color);
void drawLine(YUVImgInfo *img, lineInfo *line);
void drawRectangle(YUVImgInfo *img, YUVRectangle rect, YUVColor color, uint16_t thick);

void osd_Init(void);
void putText(YUVImgInfo *img, imgPoint pt, char* szText, YUVColor color, uint8_t fontSize);

#ifdef __cplusplus
}
#endif

#endif
