#include <stdlib.h>
#include <stdio.h>

#include "check_camera_pattern.h"
#include "appdef.h"
#include "common_types.h"
#include <memory.h>

#define JX_H62_TESTPATTERN_WIDTH	12
#define CHECK_BAD_VALUE_THRESHOLD	5
#define CHECK_RGB_BAD_VALUE_THRESHOLD	7
#define CHECK_H_DELTA_THRESHOLD		5
#define CHECK_SV_DELTA_THRESHOLD	3

#define CHECK_H_DELTA_THRESHOLD_GC2145		8
#define CHECK_SV_DELTA_THRESHOLD_GC2145		4

#define BAD_PIXEL_THRESHOLD			10
#define BAD_PIXEL_THRESHOLD_GC2145	10
#define BAD_LINE_THRESHOLD			2

int g_nJX_H62_TestPatternGTMeanValue[JX_H62_TESTPATTERN_WIDTH] = { 0, 0, 0, 1, 2, 4, 8, 16, 32, 64, 128, 255 };



#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif  //	MAX

#ifndef BYTE
typedef unsigned char BYTE;
#endif

int isCorrectPixelValueInJXPattern(int nValue, int nX, int nY)
{
	int nIndexInPitch = nX % JX_H62_TESTPATTERN_WIDTH;
	int nDeltaValue = nValue - g_nJX_H62_TestPatternGTMeanValue[nIndexInPitch];
	if (nDeltaValue < 0)
	{
		nDeltaValue = -nDeltaValue;
	}

	if (nDeltaValue > CHECK_BAD_VALUE_THRESHOLD)
	{
		return 0;
	}

	return 1;
}


int checkCameraPattern(unsigned char *pbCameraPatternBuffer)
{
#if (USE_IRSNR_SC2355)
    return checkCameraPattern_SC2355(pbCameraPatternBuffer);
#endif
	int nCheckHeight = 720;
	int nCheckWidth = 1280;
	int nY, nX;
	
	int nBadPixelCount = 0;
	int nBadLineCount = 0;

	for (nY = 1; nY < nCheckHeight - 1; nY ++)
	{
		int nIsBadLine = 0;
		for (nX = 0; nX < nCheckWidth; nX ++)
		{
			unsigned char* pbCurPatchBuffer = pbCameraPatternBuffer + nY * nCheckWidth + nX;
			if (!isCorrectPixelValueInJXPattern(*pbCurPatchBuffer, nX, nY))
			{
				nBadPixelCount++;
				nIsBadLine = 1;
				continue;
			}

			//check next line value
			if (nY < nCheckHeight - 2)
			{
				unsigned char* pbNextLinePatchBuffer = pbCurPatchBuffer + nCheckWidth;
				if (!isCorrectPixelValueInJXPattern(*pbNextLinePatchBuffer, nX, nY + 1))
				{
					continue;
				}

				int nDeltaValue = (int)(*pbCurPatchBuffer) - (int)(*pbNextLinePatchBuffer);
				if (nDeltaValue < 0)
				{
					nDeltaValue = -nDeltaValue;
				}

				if (nDeltaValue > CHECK_BAD_VALUE_THRESHOLD)
				{
					nBadPixelCount++;
					nIsBadLine = 1;
					continue;
				}
			}
		}

		if (nIsBadLine)
		{
			nBadLineCount++;
		}
	}

	if (nBadPixelCount <= BAD_PIXEL_THRESHOLD)
	{
		return CAMERA_NO_ERROR;
	}

	if (nBadLineCount <= BAD_LINE_THRESHOLD)
	{
		return CAMERA_MIDDLE_ERROR;
	}

	return CAMERA_ERROR;
}

#define SC2355_TESTPATTERN_WIDTH	780
unsigned char bSC2355Pattern[SC2355_TESTPATTERN_WIDTH] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1,
	2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6,
	6, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 9, 10, 10, 10, 10, 11, 11,
	11, 12, 12, 12, 13, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16,
	16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21,
	21, 22, 22, 22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26,
	26, 26, 27, 27, 28, 27, 28, 28, 29, 28, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31,
	31, 32, 32, 32, 32, 33, 33, 33, 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36,
	36, 37, 37, 37, 37, 38, 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, 41, 41, 41, 41,
	41, 42, 42, 42, 42, 42, 43, 43, 44, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46,
	47, 47, 47, 47, 47, 47, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 51, 51, 51, 51,
	51, 52, 52, 52, 52, 53, 53, 53, 54, 54, 54, 54, 54, 55, 55, 55, 56, 56, 56, 56,
	56, 57, 57, 57, 58, 57, 58, 58, 58, 58, 59, 59, 59, 59, 60, 60, 60, 61, 61, 61,
	62, 62, 62, 62, 62, 63, 63, 63, 63, 64, 64, 64, 64, 65, 65, 65, 65, 66, 66, 66,
	66, 67, 67, 67, 67, 68, 68, 68, 68, 68, 69, 69, 69, 70, 70, 70, 70, 70, 71, 71,
	71, 72, 72, 72, 72, 73, 73, 73, 73, 74, 74, 74, 74, 75, 75, 75, 75, 76, 76, 76,
	77, 77, 77, 77, 77, 78, 78, 78, 78, 78, 79, 79, 79, 80, 80, 80, 80, 80, 81, 81,
	81, 82, 82, 82, 82, 83, 83, 83, 83, 84, 84, 84, 84, 84, 85, 85, 85, 86, 86, 86,
	87, 86, 87, 87, 88, 87, 88, 88, 88, 89, 89, 89, 89, 90, 90, 90, 90, 90, 91, 91,
	91, 92, 92, 92, 92, 93, 93, 93, 93, 94, 94, 94, 94, 95, 95, 95, 95, 95, 96, 96,
	96, 97, 97, 97, 97, 97, 98, 98, 98, 99, 99, 99, 100, 99, 100, 100, 101, 100, 101, 101,
	102, 102, 102, 102, 102, 103, 103, 103, 103, 104, 104, 104, 105, 105, 105, 105, 106, 105, 106, 106,
	107, 107, 107, 107, 108, 108, 108, 108, 108, 109, 109, 109, 110, 110, 110, 110, 110, 111, 111, 111,
	111, 112, 112, 112, 112, 113, 113, 113, 113, 114, 114, 114, 114, 115, 115, 115, 115, 115, 116, 116,
	116, 116, 117, 117, 117, 118, 118, 118, 118, 118, 119, 119, 119, 120, 120, 120, 120, 121, 121, 121,
	121, 122, 122, 122, 122, 123, 123, 123, 123, 124, 124, 124, 124, 125, 125, 125, 126, 126, 126, 126,
	126, 126, 127, 127, 128, 127, 128, 128, 129, 129, 129, 129, 130, 130, 130, 130, 131, 131, 131, 131,
	131, 132, 132, 132, 132, 133, 133, 133, 133, 134, 134, 134, 134, 134, 135, 135, 135, 136, 136, 136,
	136, 137, 137, 137, 137, 138, 138, 138, 139, 139, 139, 139, 140, 140, 140, 140, 140, 141, 141, 141,
	141, 141, 142, 142, 142, 143, 143, 143, 143, 144, 144, 144, 144, 144, 145, 145, 145, 145, 146, 146,
	146, 147, 147, 147, 147, 148, 148, 148, 148, 149, 149, 149, 150, 149, 150, 150, 151, 151, 151, 151,
	151, 152, 152, 152, 152, 153, 153, 153, 153, 154, 154, 154, 154, 154, 155, 155, 155, 156, 156, 156,
	157, 157, 157, 157, 157, 157, 158, 158, 158, 159, 159, 159, 160, 160, 160, 160, 160, 161, 161, 161,
	161, 161, 162, 162, 162, 163, 163, 163, 163, 164, 164, 164, 165, 165, 165, 165, 165, 166, 166, 166,
	167, 166, 167, 167, 168, 168, 168, 168, 168, 169, 169, 169, 169, 170, 170, 170, 170, 170, 171, 171,
	172, 172, 172, 172, 172, 173, 173, 173, 173, 174, 174, 174, 174, 175, 175, 175, 175, 175, 176, 176,
	176, 177, 177, 177, 177, 178, 178, 178, 179, 179, 179, 179, 179, 180, 180, 180, 180, 181, 181, 181,
	181, 181, 182, 182, 182, 183, 183, 183, 183, 184, 184, 184, 185, 185, 185, 185, 185, 186, 186, 186,
	186, 187, 187, 187, 187, 188, 188, 188, 188, 188, 189, 189, 189, 190, 190, 190, 190, 191, 191,
};


int isCorrectPixelValueInSC2355Pattern(int nValue, int nX, int nY)
{
	int nPatternValue = 0;
	if (nX >= 820)
	{
		nPatternValue = bSC2355Pattern[nX - 820];
	}

	int nDeltaValue = nValue - nPatternValue;
	if (nDeltaValue < 0)
	{
		nDeltaValue = -nDeltaValue;
	}

	if (nDeltaValue > CHECK_BAD_VALUE_THRESHOLD)
	{
		return 0;
	}

	return 1;
}

int checkCameraPattern_SC2355(unsigned char *pbCameraPatternBuffer)
{
	int nCheckHeight = 900;
	int nCheckWidth = 1600;
	int nY, nX;

	int nBadPixelCount = 0;
	int nBadLineCount = 0;

	for (nY = 1; nY < nCheckHeight - 3; nY++)
	{
		int nIsBadLine = 0;
		for (nX = 0; nX < nCheckWidth; nX++)
		{
			unsigned char* pbCurPatchBuffer = pbCameraPatternBuffer + nY * nCheckWidth + nX;
			if (!isCorrectPixelValueInSC2355Pattern(*pbCurPatchBuffer, nX, nY))
			{
				nBadPixelCount++;
				nIsBadLine = 1;
				continue;
			}

			//check next line value
			if (nY < nCheckHeight - 2)
			{
				unsigned char* pbNextLinePatchBuffer = pbCurPatchBuffer + nCheckWidth;
				if (!isCorrectPixelValueInSC2355Pattern(*pbNextLinePatchBuffer, nX, nY + 1))
				{
					continue;
				}

				int nDeltaValue = (int)(*pbCurPatchBuffer) - (int)(*pbNextLinePatchBuffer);
				if (nDeltaValue < 0)
				{
					nDeltaValue = -nDeltaValue;
				}

				if (nDeltaValue > CHECK_BAD_VALUE_THRESHOLD)
				{
					nBadPixelCount++;
					nIsBadLine = 1;
					continue;
				}
			}
		}
		if (nY % 40 == 0)
			my_usleep(1000);

		if (nIsBadLine)
		{
			nBadLineCount++;
		}
	}

	if (nBadPixelCount <= BAD_PIXEL_THRESHOLD)
	{
		return CAMERA_NO_ERROR;
	}

	if (nBadLineCount <= BAD_LINE_THRESHOLD)
	{
		return CAMERA_MIDDLE_ERROR;
	}

	return CAMERA_ERROR;
}


/////////////////////////////////			BF3A03	clr pattern 0xb9:0x80					/////////////////////////////////////////
//unsigned char pbRGB_Buffer[921600];

int ConvertYUVToRGB(int y, int u, int v, unsigned char *pbDstBuf, int nIndex)
{
	y = MAX(0, y - 16);

	int r = (y * 1192 + v * 1634) >> 10;
	int g = (y * 1192 - v * 834 - 400 * u) >> 10;
	int b = (y * 1192 + u * 2066) >> 10;

	r = r > 255 ? 255 : r < 0 ? 0 : r;
	g = g > 255 ? 255 : g < 0 ? 0 : g;
	b = b > 255 ? 255 : b < 0 ? 0 : b;

	pbDstBuf[nIndex * 3] = (BYTE)r;
	pbDstBuf[nIndex * 3 + 1] = (BYTE)g;
	pbDstBuf[nIndex * 3 + 2] = (BYTE)b;

	return 0;
}


/*
convert YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
		YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
		YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
		... 
		UVUVUUVUVUVUVUVUUVUVUVUVUVUUVUVUV
*/
void ConvertYYUV_toRGB888(unsigned char *pbYuvData, int nWidth, int nHeight, unsigned char *pbDstData)
{
	int nSize = nWidth * nHeight;
	int nOffset = nSize;
	int u, v, y1, y2, y3, y4;
	memset(pbDstData, 0, nWidth * nHeight * 3);

	for (int i = 0, k = nOffset; i < nSize; i += 2, k += 2)
	{
		y1 = pbYuvData[i];
		y2 = pbYuvData[i + 1];
		y3 = pbYuvData[nWidth + i];
		y4 = pbYuvData[nWidth + i + 1];

		u = pbYuvData[k];
		v = pbYuvData[k + 1];

		v = v - 128;
		u = u - 128;

		ConvertYUVToRGB(y1, u, v, pbDstData, i);
		ConvertYUVToRGB(y2, u, v, pbDstData, i + 1);
		ConvertYUVToRGB(y3, u, v, pbDstData, nWidth + i);
		ConvertYUVToRGB(y4, u, v, pbDstData, nWidth + i + 1);

		if (i != 0 && (i + 2) % nWidth == 0)
			i += nWidth;
	}
}

/*
convert YUYVYUYVYUYVYUYVYUYVYUYVYUYVYUYV
		YUYVYUYVYUYVYUYVYUYVYUYVYUYVYUYV
		YUYVYUYVYUYVYUYVYUYVYUYVYUYVYUYV
		YUYVYUYVYUYVYUYVYUYVYUYVYUYVYUYV
*/
void ConvertYUYV_toRGB888(unsigned char *pbYuvData, int nWidth, int nHeight, unsigned char *pbDstData)
{
	int nSize = nWidth * nHeight;
	int u, v, y1, y2, y3, y4;
	memset(pbDstData, 0, nWidth * nHeight * 3);
	int nWidth_2 = nWidth * 2;
	
	for (int i = 0; i < nSize; i += 2)
	{
		int nCurYIndex = i * 2;
		y1 = pbYuvData[nCurYIndex];
		y2 = pbYuvData[nCurYIndex + 2];
		y3 = pbYuvData[nCurYIndex + nWidth_2];
		y4 = pbYuvData[nCurYIndex + nWidth_2 + 2];

		u = pbYuvData[nCurYIndex + 1];
		v = pbYuvData[nCurYIndex + 3];

		v = v - 128;
		u = u - 128;

		ConvertYUVToRGB(y1, u, v, pbDstData, i);
		ConvertYUVToRGB(y2, u, v, pbDstData, i + 1);
		ConvertYUVToRGB(y3, u, v, pbDstData, nWidth + i);
		ConvertYUVToRGB(y4, u, v, pbDstData, nWidth + i + 1);

		if (i != 0 && (i + 2) % nWidth == 0)
			i += nWidth;
	}
}



int nBF3A03PatternColor_YYUV[24][3] = {
	{255,255,255},//white
	{234, 255, 0},//yellow
	{0, 255, 255},//blue sky
	{0, 255, 0},//Green
	{255, 0, 255},//magenta
	{255, 0, 0},//red
	{9, 0, 185},//Blue
	{0, 0, 0},//Black
	{219, 255, 55},//between white and yellow 1
	{88, 161, 0},//between white and yellow 2
	{97, 255, 0},//between yellow and sky blue 1
	{0, 209, 0},//between yellow and sky blue 2
	{0, 220, 0},//between yellow and sky blue 3
	{0, 255, 77},//between yellow and sky blue 4
	{26, 246, 81},//between sky blue and green 1
	{0, 179, 13},//between sky blue and green 1
	{255, 0, 173},//between magenta and red 1
	{255, 0, 147},//between magenta and red 2
	{197, 17, 0},//between red and blue 1
	{177, 0, 0},//between red and blue 2
	{0, 0, 108},//between red and blue 3
	{9, 9, 118},//between red and blue 4
	{11, 0, 108},//between blue and black 1
	{0, 0, 96},//between blue and black 2
};

int nBF3A03PatternColor_YUYV[8][3] = {
	{ 255, 255, 255 },//white
	{ 243, 255, 0 },//yellow
	{ 0, 255, 255 },//blue sky
	{ 0, 255, 0 },//Green
	{ 255, 0, 255 },//magenta
	{ 255, 0, 0 },//red
	{ 20, 0, 190 },//Blue
	{ 0, 0, 0 },//Black
};



int nBetweenGreenAndMagenta[16][3] = 
{
	{ 124, 196, 0 }, { 199, 255, 51 }, { 255, 24, 127 }, {255, 6, 108},
	{ 124, 196, 0 }, { 124, 196, 0 }, { 255, 0, 69 }, {255, 6, 108},
	{ 75, 222, 0 }, { 149, 255, 51 }, { 255, 33, 81 }, {255, 15, 62},
	{ 75, 222, 0 }, { 75, 222, 0 }, { 255, 0, 22 }, {255, 15, 62}
};


int nBF3A03ComonAreaRangeX_YYUV[24][2] = {
	{0, 77},//white
	{80, 157},//yellow
	{162, 239},//blue sky
	{242, 319},//Green
	{324, 401},//magenta
	{404, 481},//red
	{486, 563},//Blue
	{566, 639},//Black
	{78, 78},//between white and yellow 1
	{79, 79},//between white and yellow 2
	{158,158},//between yellow and sky blue 1
	{159,159},//between yellow and sky blue 2
	{160, 160},//between yellow and sky blue 3
	{161, 161},//between yellow and sky blue 4
	{240, 240},//between sky blue and green 1
	{241, 241},//between sky blue and green 2
	{402, 402},//between magenta and red 1
	{403, 403},//between magenta and red 2
	{ 482, 482 },//between red and blue 1
	{483, 483},//between red and blue 2
	{484, 484},//between red and blue 3
	{485, 485},//between red and blue 4
	{564, 564},//between blue and black 1
	{565, 565},//between blue and black 2
};


int nBF3A03ComonAreaRangeX_YUYV[8][2] = {
	{ 0, 78 },//white
	{ 82, 157 },//yellow
	{ 162, 240 },//blue sky
	{ 244, 320 },//Green
	{ 323, 403 },//magenta
	{ 404, 482 },//red
	{ 486, 564 },//Blue
	{ 566, 639 },//Black
};

int rgb2hsv(int r, int g, int b, int& h, int& s, int& v)
{
	double      min, max, delta;

	double rr, rg, rb;
	double rh, rs, rv;
	rr = (double)r / 255;
	rg = (double)g / 255;
	rb = (double)b / 255;


	min = rr < rg ? rr : rg;
	min = min  < rb ? min : rb;

	max = rr > rg ? rr : rg;
	max = max  > rb ? max : rb;

	rv = max;                                // v
	delta = max - min;
	if (delta < 0.00001)
	{
		h = 0;
		s = 0;
		v = (int)(rv * 100);
		return 0;
	}
	if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
		rs = (delta / max);                  // s
	}
	else {
		// if max is 0, then r = g = b = 0              
		// s = 0, h is undefined
		h = 0;
		s = 0;
		v = (int)(rv * 100);
		return 0;
	}
	if (rr >= max)                           // > is bogus, just keeps compilor happy
		rh = ((rg - rb) / delta);        // between yellow & magenta
	else
	if (rg >= max)
		rh = 2.0 + (rb - rr) / delta;  // between cyan & yellow
	else
		rh = 4.0 + (rr - rg) / delta;  // between magenta & cyan

	rh *= 60.0;                              // degrees

	if (rh < 0.0)
		rh += 360.0;

	h = (int)rh;
	s = (int)(rs * 100);
	v = (int)(rv * 100);

	return 0;
}


int isCorrectPixelValueInBF3A03Pattern(unsigned char* pbRGB_Buffer, int nWidth, int nX, int nY, int nMode = 0)
{
	unsigned char* pCurBuffer = pbRGB_Buffer + (nY * nWidth + nX) * 3;
	int nR, nG, nB;
	int nH, nS, nV;
	int nGT_R, nGT_G, nGT_B;
	int nGT_H, nGT_S, nGT_V;

	nR = *pCurBuffer;
	nG = *(pCurBuffer + 1);
	nB = *(pCurBuffer + 2);

	int nSectionSelected = -1;
	int i;
	for (i = 0; i < 8; i++)
	{
		int nStartX, nEndX;
		if (nMode == 0)
		{
			nStartX = nBF3A03ComonAreaRangeX_YYUV[i][0];
			nEndX = nBF3A03ComonAreaRangeX_YYUV[i][1];
		}
		else
		{
			nStartX = nBF3A03ComonAreaRangeX_YUYV[i][0];
			nEndX = nBF3A03ComonAreaRangeX_YUYV[i][1];
		}
		if (nX >= nStartX && nX < nEndX)
		{
			nSectionSelected = i;
			break;
		}
	}

	if (nSectionSelected == -1)
	{
		return 1;
	}
	
	if (nMode == 0)
	{
		nGT_R = nBF3A03PatternColor_YYUV[nSectionSelected][0];
		nGT_G = nBF3A03PatternColor_YYUV[nSectionSelected][1];
		nGT_B = nBF3A03PatternColor_YYUV[nSectionSelected][2];
	}
	else
	{
		nGT_R = nBF3A03PatternColor_YUYV[nSectionSelected][0];
		nGT_G = nBF3A03PatternColor_YUYV[nSectionSelected][1];
		nGT_B = nBF3A03PatternColor_YUYV[nSectionSelected][2];
	}

	rgb2hsv(nR, nG, nB, nH, nS, nV);
	rgb2hsv(nGT_R, nGT_G, nGT_B, nGT_H, nGT_S, nGT_V);

	int nDeltaH, nDeltaS, nDeltaV;
	nDeltaH = nH - nGT_H;
	nDeltaS = nS - nGT_S;
	nDeltaV = nV - nGT_V;

	if (nDeltaH < 0)
	{
		nDeltaH = -nDeltaH;
	}
	if (nDeltaS < 0)
	{
		nDeltaS = -nDeltaS;
	}
	if (nDeltaV < 0)
	{
		nDeltaV = -nDeltaV;
	}

	if (nDeltaH > CHECK_H_DELTA_THRESHOLD)
	{
		return 0;
	}
	if (nDeltaS > CHECK_SV_DELTA_THRESHOLD)
	{
		return 0;
	}
	if (nDeltaV > CHECK_SV_DELTA_THRESHOLD)
	{
		return 0;
	}

	return 1;
}

int checkCameraPattern_BF3A03_0x80_clr(unsigned char *pbClrCameraPatternBuffer, unsigned char* pbRGB_Buffer, int nMode)
{
	int nBF3A03_Clr_Width = 640;
	int nBF3A03_Clr_Height = 480;

	if (nMode == 0)
	{
		ConvertYYUV_toRGB888(pbClrCameraPatternBuffer, nBF3A03_Clr_Width, nBF3A03_Clr_Height, pbRGB_Buffer);
	}
	else
	{
		ConvertYUYV_toRGB888(pbClrCameraPatternBuffer, nBF3A03_Clr_Width, nBF3A03_Clr_Height, pbRGB_Buffer);
	}

// 	FILE* pFile;
// 	pFile = fopen("G:/temp/clr/temp/2.bin", "wb");
// 	fwrite(pbRGB_Buffer, nBF3A03_Clr_Width * nBF3A03_Clr_Height * 3, 1, pFile);
// 	fclose(pFile);

	int nPatternIsBad = 0;
	int nBadPixelCount = 0;
	int nBadLineCount = 0;

	int nX, nY;
	for (nY = 1; nY < nBF3A03_Clr_Height - 2; nY ++)
	{
		int nIsBadLine = 0;

		for (nX = 0; nX < nBF3A03_Clr_Width; nX++)
		{
			unsigned char* pCurBuffer = pbRGB_Buffer + (nY * nBF3A03_Clr_Width + nX) * 3;
			int nR, nG, nB;

			nR = *pCurBuffer;
			nG = *(pCurBuffer + 1);
			nB = *(pCurBuffer + 2);

			if (!isCorrectPixelValueInBF3A03Pattern(pbRGB_Buffer, nBF3A03_Clr_Width, nX, nY, nMode))
			{
				nBadPixelCount++;
				nIsBadLine = 1;
				continue;
			}

			//check next line value
			if (nY < nBF3A03_Clr_Height - 2)
			{
				if (!isCorrectPixelValueInBF3A03Pattern(pbRGB_Buffer, nBF3A03_Clr_Width, nX + 1, nY, nMode))
				{
					continue;
				}
				if (!isCorrectPixelValueInBF3A03Pattern(pbRGB_Buffer, nBF3A03_Clr_Width, nX, nY + 1, nMode))
				{
					continue;
				}


				int nSectionSelected = -1;
				int i;
				for (i = 0; i < 8; i++)
				{
					int nStartX, nEndX;
					if (nMode == 0)
					{
						nStartX = nBF3A03ComonAreaRangeX_YYUV[i][0];
						nEndX = nBF3A03ComonAreaRangeX_YYUV[i][1];
					}
					else
					{
						nStartX = nBF3A03ComonAreaRangeX_YUYV[i][0];
						nEndX = nBF3A03ComonAreaRangeX_YUYV[i][1];
					}

					if (nX >= nStartX && nX < nEndX)
					{
						nSectionSelected = i;
						break;
					}
				}

				if (nSectionSelected == -1)
				{
					continue;
				}
				int nNeigh1R, nNeigh1G, nNeigh1B, nNeigh2R, nNeigh2G, nNeigh2B;
				nNeigh1R = *(pbRGB_Buffer + (nY * nBF3A03_Clr_Width + (nX + 1)) * 3);
				nNeigh1G = *(pbRGB_Buffer + (nY * nBF3A03_Clr_Width + (nX + 1)) * 3 + 1);
				nNeigh1B = *(pbRGB_Buffer + (nY * nBF3A03_Clr_Width + (nX + 1)) * 3 + 2);
				nNeigh2R = *(pbRGB_Buffer + ((nY + 1) * nBF3A03_Clr_Width + (nX)) * 3);
				nNeigh2G = *(pbRGB_Buffer + ((nY + 1) * nBF3A03_Clr_Width + (nX)) * 3 + 1);
				nNeigh2B = *(pbRGB_Buffer + ((nY + 1) * nBF3A03_Clr_Width + (nX)) * 3 + 2);

				int nDeltaValue[6];
				nDeltaValue[0] = nNeigh1R - nR;
				nDeltaValue[1] = nNeigh1G - nG;
				nDeltaValue[2] = nNeigh1B - nB;
				nDeltaValue[3] = nNeigh2R - nR;
				nDeltaValue[4] = nNeigh2G - nG;
				nDeltaValue[5] = nNeigh2B - nB;

				nPatternIsBad = 0;
				int nIndex;
				for (nIndex = 0; nIndex < 6; nIndex++)
				{
					if (nDeltaValue[nIndex] < -CHECK_BAD_VALUE_THRESHOLD || nDeltaValue[nIndex] > CHECK_BAD_VALUE_THRESHOLD)
					{
						nPatternIsBad = 1;
						break;
					}
				}

				if (nPatternIsBad)
				{
					nBadPixelCount++;
					nIsBadLine = 1;
					continue;
				}
			}
		}
		if (nIsBadLine)
		{
			nBadLineCount++;
		}
	}

	if (nBadPixelCount <= BAD_PIXEL_THRESHOLD)
	{
		return CAMERA_NO_ERROR;
	}

	if (nBadLineCount <= BAD_LINE_THRESHOLD)
	{
		return CAMERA_MIDDLE_ERROR;
	}

	return CAMERA_ERROR;
}

#define GC2145_Pattern_Rect_Count	21
int nGC2145_pattern[GC2145_Pattern_Rect_Count][7] = {
    { 0, 0, 121, 106, 255, 0, 24 },
    { 129, 0, 250, 105, 0, 255, 0 },
    { 257, 0, 376, 105, 191, 0, 255 },
    { 383, 0, 506, 105, 0, 255, 255 },
    { 511, 0, 634, 105, 255, 0, 255 },
    { 641, 0, 761, 105, 255, 255, 0 },
    { 769, 0, 890, 105, 255, 255, 255 },
    { 897, 0, 1018, 105, 0, 0, 0 },
    { 0, 111, 122, 234, 0, 0, 0 },
    { 127, 111, 250, 234, 255, 255, 255 },
    { 257, 111, 375, 234, 255, 255, 0 },
    { 383, 111, 506, 234, 255, 0, 255 },
    { 512, 112, 634, 234, 0, 255, 255 },
    { 641, 112, 760, 233, 191, 0, 255 },
    { 769, 112, 890, 234, 0, 255, 0 },
    { 897, 111, 1018, 233, 255, 0, 24 },
    { 767, 239, 1016, 362, 255, 255, 255 },
    { 767, 368, 1016, 489, 255, 0, 24 },
    { 767, 495, 1016, 618, 0, 255, 0 },
    { 767, 624, 1016, 745, 191, 0, 255 },
    { 767, 751, 1016, 938, 255, 255, 255 },
//	{ 0, 254, 37, 599, 0, 0, 0 },
};

int isCorrectPixelValueInGC21453Pattern(unsigned char* pbRGB_Buffer, int nWidth, int nX, int nY)
{
	int nGT_R, nGT_G, nGT_B, nGT_H, nGT_S, nGT_V;
	int nR, nG, nB, nH, nS, nV;
	
	int nPointInPattern = 0;
	int nPatternIndex;
	for (nPatternIndex = 0; nPatternIndex < GC2145_Pattern_Rect_Count; nPatternIndex++)
	{
		if (nGC2145_pattern[nPatternIndex][0] <= nX && nGC2145_pattern[nPatternIndex][2] >= nX && 
			nGC2145_pattern[nPatternIndex][1] <= nY && nGC2145_pattern[nPatternIndex][3] >= nY)
		{
			nPointInPattern = 1;
			nGT_R = nGC2145_pattern[nPatternIndex][4];
			nGT_G = nGC2145_pattern[nPatternIndex][5];
			nGT_B = nGC2145_pattern[nPatternIndex][6];
			break;
		}
	}
	if (!nPointInPattern)
	{
		return 1;
	}

	nR = pbRGB_Buffer[0];
	nG = pbRGB_Buffer[1];
	nB = pbRGB_Buffer[2];

	rgb2hsv(nR, nG, nB, nH, nS, nV);
	rgb2hsv(nGT_R, nGT_G, nGT_B, nGT_H, nGT_S, nGT_V);

	int nDeltaH, nDeltaS, nDeltaV;
	int nDeltaR, nDeltaG, nDeltaB;
	nDeltaH = nH - nGT_H;
	nDeltaS = nS - nGT_S;
	nDeltaV = nV - nGT_V;
	nDeltaR = nGT_R - nR;
	nDeltaG = nGT_G - nG;
	nDeltaB = nGT_B - nB;


	if (nDeltaH < 0)
	{
		nDeltaH = -nDeltaH;
	}
	if (nDeltaS < 0)
	{
		nDeltaS = -nDeltaS;
	}
	if (nDeltaV < 0)
	{
		nDeltaV = -nDeltaV;
	}
	if (nDeltaR < 0)
	{
		nDeltaR = -nDeltaR;
	}
	if (nDeltaG < 0)
	{
		nDeltaG = -nDeltaG;
	}
	if (nDeltaB < 0)
	{
		nDeltaB = -nDeltaB;
	}

	if ((nDeltaH <= CHECK_H_DELTA_THRESHOLD_GC2145 && nDeltaS <= CHECK_SV_DELTA_THRESHOLD_GC2145 && nDeltaV <= CHECK_SV_DELTA_THRESHOLD_GC2145) || 
		(nDeltaR <= CHECK_RGB_BAD_VALUE_THRESHOLD && nDeltaG <= CHECK_RGB_BAD_VALUE_THRESHOLD && nDeltaB <= CHECK_RGB_BAD_VALUE_THRESHOLD))
		return 1;
	
	return 0;
}

int checkCameraPattern_GC2145_clr(unsigned char *pbClrCameraPatternBuffer)
{
	int nGC2145_Clr_Width = 1280;
	int nGC2145_Clr_Height = 960;
#if 0
	unsigned char pbRGBBuf[3];
	int nBadPixelCount = 0;

	int nX, nY;
	for (nY = 1; nY < nGC2145_Clr_Height - 1; nY++)
	{
		for (nX = 1; nX < nGC2145_Clr_Width - 1; nX++)
		{
			int nCurYIndex = (nY * nGC2145_Clr_Width + nX) << 1;
			int y, u, v;
			y = pbClrCameraPatternBuffer[nCurYIndex];
			
			nCurYIndex = (((nY >> 1) << 1) * nGC2145_Clr_Width + ((nX >> 1) << 1)) << 1;
            u = pbClrCameraPatternBuffer[nCurYIndex + 1];
            v = pbClrCameraPatternBuffer[nCurYIndex + 3];

			v = v - 128;
			u = u - 128;

			ConvertYUVToRGB(y, u, v, pbRGBBuf, 0);
            if (!isCorrectPixelValueInGC21453Pattern(pbRGBBuf, nGC2145_Clr_Width, nX, nY))
			{
				nBadPixelCount++;
			}
		}
	}

	if (nBadPixelCount <= BAD_PIXEL_THRESHOLD_GC2145)
	{
		return CAMERA_NO_ERROR;
	}

	return CAMERA_ERROR;
#else
    unsigned char fixed[4];
    for (unsigned int i = 0; i < sizeof(fixed); i ++)
        fixed[i] = pbClrCameraPatternBuffer[i];
    int nX, nY;
    unsigned int pixelNo = 0;
    for (nY = 0; nY < nGC2145_Clr_Height; nY++)
    {
        for (nX = 0; nX < nGC2145_Clr_Width; nX++)
        {
            pixelNo = nY * nGC2145_Clr_Width + nX;
            if (pbClrCameraPatternBuffer[pixelNo] != fixed[pixelNo % 4])
                return CAMERA_ERROR;
        }
    }
    return CAMERA_NO_ERROR;
#endif
}

