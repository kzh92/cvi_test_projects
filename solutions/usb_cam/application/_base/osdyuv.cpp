//#include "stdafx.h"
#include <stdio.h>
#include "osdyuv.h"

static YUVPixColor s_color_table[YUV_COLOR_MAX] = {
    {0x00, 0x00, 0x00}, // green
    {0x00, 0x00, 0xff}, // red
    {0x00, 0xff, 0x00}, // blue
    {0x00, 0xff, 0xff}, // purple
    {0xff, 0x00, 0x00}, // dark green
    {0xff, 0x00, 0xff}, // yellow
    {0xff, 0xff, 0x00}, // light blue
    {0xff, 0xff, 0xff}, // light purple
    {0x00, 0x80, 0x80}, // dark black
    {0x80, 0x80, 0x80}, // gray
    {0xff, 0x80, 0x80}, // white
};

/*----------------------------------------------*/
/*                  �����Ͷ���                  */
/*----------------------------------------------*/
#define HZK16_FRONT_PATH        ("./HZK16")
#define ASCII8_FRONT_PATH       ("./ASCII8")


/*----------------------------------------------*/
/*                 �ṹ�嶨��                   */
/*----------------------------------------------*/

/*----------------------------------------------*/
/*                  ��������                    */
/*----------------------------------------------*/

/*----------------------------------------------*/
/*                  ȫ�ֱ���                    */
/*----------------------------------------------*/
/* �ֿ� */
extern "C" const uint8_t g_Ascii8Dot[];          /* ASCII �ֿ� 8*16*/
extern "C" const unsigned int ASCII8_LEN;
const uint8_t g_Hzk16Dot[1] = { 0 };           /* �����ֿ� 16*16 */
const unsigned int HZK16_LEN = 1;

/* ������ұ� */
//uint8_t dotTableNormal_u8[256][8];        /* �������� */
uint16_t dotTableNormal_u16[256][8];      /* �������� */
//uint16_t dotTableReverse_u16[256][8];     /* ���Ҿ��� */

/*----------------------------------------------*/
/*                  ��������                    */
/*----------------------------------------------*/

/**
* @function:   osd_Init
* @brief:      ��� dot table
* @param[in]:  void
* @param[out]: None
* @return:     void
*/
void osd_Init(void)
{
	uint16_t i = 0;
	// uint8_t tabColorMask_u8 = 0xFFu;
	uint16_t tabColorMask = 0xFFFFu;

	for (i = 0; i < 256; i++)
	{
#if 0
        dotTableNormal_u8[i][0] = (i & 0x80u) ? tabColorMask_u8 : 0x00u;
		dotTableNormal_u8[i][1] = (i & 0x40u) ? tabColorMask_u8 : 0x00u;
		dotTableNormal_u8[i][2] = (i & 0x20u) ? tabColorMask_u8 : 0x00u;
		dotTableNormal_u8[i][3] = (i & 0x10u) ? tabColorMask_u8 : 0x00u;
		dotTableNormal_u8[i][4] = (i & 0x08u) ? tabColorMask_u8 : 0x00u;
		dotTableNormal_u8[i][5] = (i & 0x04u) ? tabColorMask_u8 : 0x00u;
		dotTableNormal_u8[i][6] = (i & 0x02u) ? tabColorMask_u8 : 0x00u;
		dotTableNormal_u8[i][7] = (i & 0x01u) ? tabColorMask_u8 : 0x00u;
#endif
		dotTableNormal_u16[i][0] = (i & 0x80) ? tabColorMask : 0x0000;
		dotTableNormal_u16[i][1] = (i & 0x40) ? tabColorMask : 0x0000;
		dotTableNormal_u16[i][2] = (i & 0x20) ? tabColorMask : 0x0000;
		dotTableNormal_u16[i][3] = (i & 0x10) ? tabColorMask : 0x0000;
		dotTableNormal_u16[i][4] = (i & 0x08) ? tabColorMask : 0x0000;
		dotTableNormal_u16[i][5] = (i & 0x04) ? tabColorMask : 0x0000;
		dotTableNormal_u16[i][6] = (i & 0x02) ? tabColorMask : 0x0000;
		dotTableNormal_u16[i][7] = (i & 0x01) ? tabColorMask : 0x0000;
#if 0
		dotTableReverse_u16[i][7] = (i & 0x80) ? tabColorMask : 0x0000;
		dotTableReverse_u16[i][6] = (i & 0x40) ? tabColorMask : 0x0000;
		dotTableReverse_u16[i][5] = (i & 0x20) ? tabColorMask : 0x0000;
		dotTableReverse_u16[i][4] = (i & 0x10) ? tabColorMask : 0x0000;
		dotTableReverse_u16[i][3] = (i & 0x08) ? tabColorMask : 0x0000;
		dotTableReverse_u16[i][2] = (i & 0x04) ? tabColorMask : 0x0000;
		dotTableReverse_u16[i][1] = (i & 0x02) ? tabColorMask : 0x0000;
		dotTableReverse_u16[i][0] = (i & 0x01) ? tabColorMask : 0x0000;
#endif
	}
	return;
}


/**
* @function:   getCharFrontAddr
* @brief:      ��ȡ�����ֿ��ַ
* @param[in]:  uint16_t charHz
* @param[out]: None
* @return:     uint8_t *
*/
uint8_t * getCharFrontAddr(uint16_t charCode)
{
	if (charCode < 0xFF)
	{
		return (uint8_t *)(g_Ascii8Dot + charCode * 16);
	}
	else
	{
		uint32_t offset = (((charCode & 0xFFu) - 0xA1u) + 94u * ((charCode >> 8u) - 0xA1u)) << 5;

		return (uint8_t *)(g_Hzk16Dot + offset);
	}
}

void setYUVPix(unsigned char* YBuf,
                unsigned char * UVBuf,
                YUVType type,
                uint16_t width,
                uint16_t height,
                imgPoint point,
                YUVColor color)
{
    switch(type)
    {
        case TYPE_YUV422I_UYVY:
        case TYPE_YUV422I_YUYV:
        {
            /*
                UYVY UYVY UYVY UYVY
            */
            uint32_t tmp = point.y * width * 2;
            uint32_t y_offset = 0, u_offset = 0, v_offset = 0;
            if (type == TYPE_YUV422I_UYVY) {
                u_offset = tmp + point.x / 2 * 4;
                v_offset = u_offset + 2;
                y_offset = u_offset + 1;
            }
            else {
                y_offset = tmp + point.x / 2 * 4;
                u_offset = y_offset + 1;
                v_offset = u_offset + 2;
            }
            YBuf[y_offset] = s_color_table[color].Y;
            YBuf[y_offset + 2] = s_color_table[color].Y;
            YBuf[u_offset] = s_color_table[color].U;
            YBuf[v_offset] = s_color_table[color].V;
        }break;
        case TYPE_YUV420SP_NV12:
        case TYPE_YUV420SP_NV21:
        {
            /*
                YY YY
                YY YY
                UV UV
            */
            uint32_t y_offset = point.y * width + point.x;
            uint32_t u_offset = 0, v_offset = 0;
            YBuf[y_offset] = s_color_table[color].Y;
            #if 0
            Int32 x_flag = 1, y_flag = 1;
            if (point.y % 2 == 0) {
                YBuf[y_offset + width] = s_color_table[color].Y;
                y_flag = 1;
            }
            else {
                YBuf[y_offset - width] = s_color_table[color].Y;
                y_flag = -1;
            }
 
            if (point.x % 2 == 0) {
                YBuf[y_offset + 1] = s_color_table[color].Y;
                x_flag = 1;
            }
            else {
                YBuf[y_offset - 1] = s_color_table[color].Y;
                x_flag = -1;
            }
            YBuf[y_offset + width * y_flag + 1 * x_flag] = s_color_table[color].Y;
            #endif
            
            if (type == TYPE_YUV420SP_NV12) {
                u_offset = (point.y / 2) * width + point.x / 2 * 2;
                v_offset = u_offset + 1;
            }
            else {
                v_offset = (point.y / 2) * width + point.x / 2 * 2;
                u_offset = v_offset + 1;
            }
            UVBuf[u_offset] = s_color_table[color].U;
            UVBuf[v_offset] = s_color_table[color].V;
        }break;
        case TYPE_YUV444P:
        {
            /*
                YYYYYYYY
                UUUUUUUU
                VVVVVVVV
            */
            uint32_t y_offset = 0, u_offset = 0, v_offset = 0;
            uint32_t plane_size = width * height;
            y_offset = point.y * width + point.x;
            u_offset = y_offset;
            v_offset = plane_size + u_offset;
            YBuf[y_offset] = s_color_table[color].Y;
            UVBuf[u_offset] = s_color_table[color].U;
            UVBuf[v_offset] = s_color_table[color].V;
        }break;
        case TYPE_YUV444I:
        {
            /*
                YUV YUV YUV YUV YUV YUV YUV YUV
            */
            uint32_t y_offset = 0, u_offset = 0, v_offset = 0;
            y_offset = point.y * width * 3 + point.x * 3;
            u_offset = y_offset + 1;
            v_offset = u_offset + 1;
            YBuf[y_offset] = s_color_table[color].Y;
            YBuf[u_offset] = s_color_table[color].U;
            YBuf[v_offset] = s_color_table[color].V;
        }break;
        case TYPE_YUV422P:
        {
            /*
                YYYYYYYY
                UUUU
                VVVV
            */
            uint32_t y_offset = 0, u_offset = 0, v_offset = 0;
            uint32_t plane_size = width * height / 2;
            y_offset = point.y * width + point.x;
            u_offset = y_offset / 2;
            v_offset = plane_size + y_offset / 2;
            YBuf[y_offset] = s_color_table[color].Y;
            UVBuf[u_offset] = s_color_table[color].U;
            UVBuf[v_offset] = s_color_table[color].V;
        }break;
        case TYPE_YUV420I:
        {
            /*
                YYYYYYYY
                UU
                VV
            */
            uint32_t y_offset = 0, u_offset = 0, v_offset = 0;
            uint32_t plane_size = width * height / 4;
            y_offset = point.y * width + point.x;
            u_offset = point.y / 2 * width / 2 + point.x/2;
            v_offset = u_offset + plane_size;
            YBuf[y_offset] = s_color_table[color].Y;
            UVBuf[u_offset] = s_color_table[color].U;
            UVBuf[v_offset] = s_color_table[color].V;
        }break;
    }
}

void drawLine(YUVImgInfo *img, lineInfo *line)
{
    if (!img || !img->imgdata) return;
 
    uint8_t *YBuff = NULL, *UVBuff = NULL;
    if (line->thick == 0) line->thick = 1;
    
    uint16_t i = 0;
    for (i = 0; i < line->thick; i++) {
        uint16_t x0 = line->startPoint.x, y0 = line->startPoint.y;
        uint16_t x1 = line->endPoint.x, y1 = line->endPoint.y;
        x0 = (x0 >= img->width) ? (x0 - line->thick) : x0;
        x1 = (x1 >= img->width) ? (x1 - line->thick) : x1;
        y0 = (y0 >= img->height) ? (y0 - line->thick) : y0;
        y1 = (y1 >= img->height) ? (y1 - line->thick) : y1;
        
        uint16_t dx = (x0 > x1) ? (x0 - x1) : (x1 - x0);
        uint16_t dy = (y0 > y1) ? (y0 - y1) : (y1 - y0);

        if (dx <= dy) {
            x0 += i;
            x1 += i;
        } else {
            y0 += i;
            y1 += i;
        }

        int16_t xstep = (x0 < x1) ? 1 : -1;
        int16_t ystep = (y0 < y1) ? 1 : -1;
        int16_t nstep = 0, eps = 0;

        imgPoint draw_point;
        draw_point.x = x0;
        draw_point.y = y0;

        switch (img->yuvType) {
            case TYPE_YUV422I_UYVY:
            case TYPE_YUV422I_YUYV:
            case TYPE_YUV444I:
                YBuff = img->imgdata;
                UVBuff = NULL;
                break;
            case TYPE_YUV420SP_NV12:
            case TYPE_YUV420SP_NV21:
            case TYPE_YUV444P:
            case TYPE_YUV422P:
            case TYPE_YUV420I:
                YBuff = img->imgdata;
                UVBuff = img->imgdata + img->width * img->height;
                break;
            default:
                return;
        }

        // 布雷森汉姆算法画线
        if (dx > dy){
            while (nstep <= dx) {
                setYUVPix(YBuff, UVBuff, img->yuvType, img->width, img->height, draw_point, line->color);
                eps += dy;
                if ( (eps << 1) >= dx ) {
                    draw_point.y += ystep;
                    eps -= dx;
                }
                draw_point.x += xstep;
                nstep++;
            }
        }else {
            while (nstep <= dy){	
                setYUVPix(YBuff, UVBuff, img->yuvType, img->width, img->height, draw_point, line->color);
                eps += dx;
                if ( (eps << 1) >= dy ) {
                    draw_point.x += xstep;
                    eps -= dy;
                }
                draw_point.y += ystep;
                nstep++;
            }
        }
    }
}

void drawRectangle(YUVImgInfo *img, YUVRectangle rect, YUVColor color, uint16_t thick)
{
    lineInfo line;
    line.color = color;
    line.thick = thick;

    line.startPoint.x = rect.x;
    line.startPoint.y = rect.y;
    line.endPoint.x = rect.x + rect.width - 1;
    line.endPoint.y = rect.y;
    drawLine(img, &line);

    line.startPoint.x = rect.x;
    line.startPoint.y = rect.y + thick;
    line.endPoint.x = rect.x;
    line.endPoint.y = rect.y + rect.height - thick;
    drawLine(img, &line);

    line.startPoint.x = rect.x;
    line.startPoint.y = rect.y + rect.height - thick;
    line.endPoint.x = rect.x + rect.width - 1;
    line.endPoint.y = rect.y + rect.height - thick;
    drawLine(img, &line);

    line.startPoint.x = rect.x + rect.width -thick;
    line.startPoint.y = rect.y + thick;
    line.endPoint.x = rect.x + rect.width - thick;
    line.endPoint.y = rect.y + rect.height - thick;
    drawLine(img, &line);
}

void putText(YUVImgInfo *img, imgPoint pt, char* szText, YUVColor color, uint8_t fontSize)
{
	if (!img || !img->imgdata) return;

    if (szText == NULL)
    {
        return;
    }

	uint8_t *YBuff = NULL, *UVBuff = NULL;
	if (fontSize == 0)
		fontSize = 1;

	uint8_t *   pFont = NULL;

	uint16_t * pDotTableValue = NULL;
	uint16_t hIdx, k, j;
	uint16_t i = 0;
	uint16_t nTextLen = 0;
	imgPoint draw_point;

	while(szText[nTextLen])
	{
		uint16_t x0 = pt.x, y0 = pt.y;

		x0 = (x0 >= img->width) ? (x0 - fontSize) : x0;
		y0 = (y0 >= img->height) ? (y0 - fontSize) : y0;
		
		switch (img->yuvType) {
		case TYPE_YUV422I_UYVY:
		case TYPE_YUV422I_YUYV:
		case TYPE_YUV444I:
			YBuff = img->imgdata;
			UVBuff = NULL;
			break;
		case TYPE_YUV420SP_NV12:
		case TYPE_YUV420SP_NV21:
		case TYPE_YUV444P:
		case TYPE_YUV422P:
		case TYPE_YUV420I:
			YBuff = img->imgdata;
			UVBuff = img->imgdata + img->width * img->height;
			break;
		default:
			return;
		}

		pFont = getCharFrontAddr(szText[nTextLen]);

		if (pFont == NULL)
        {
            printf("getCharFrontAddr %c pFont == NULL\n", szText[nTextLen]);
            //break;
            nTextLen ++;
            continue;
        }

		for (hIdx = 0; hIdx < FRONT_HEIGHT; hIdx++)
		{
			pDotTableValue = dotTableNormal_u16[pFont[hIdx]];
			for (k = 0; k < ASCII_WIDTH; k++)
			{	
				if (pDotTableValue[k])
				{
					for (i = 0; i < fontSize; i++)
					{
						draw_point.x = x0 + (nTextLen * ASCII_WIDTH + k) * fontSize + i;
						for (j = 0; j < fontSize; j++)
						{
							draw_point.y = y0 + hIdx * fontSize + j;
							setYUVPix(YBuff, UVBuff, img->yuvType, img->width, img->height, draw_point, color);
						}
					}
				}
			}
		}

		nTextLen ++;
	}
}
