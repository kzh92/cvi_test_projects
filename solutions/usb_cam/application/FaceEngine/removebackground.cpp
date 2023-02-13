#include "removebackground.h"
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

unsigned char pbGlobalBuffer[0x4B000];
#define DIFF_THRESHOLD			10
#define DIFF_CERTAIN_THRESHOLD	3
#define VALUE_SKY				170
#define CERTAIN_VALUE_SKY		170

int g_rHandAverageLedOnImage = 0;

int     getHandAverageLedOnImage()
{
    return g_rHandAverageLedOnImage;
}

int removeBackground(unsigned char* pbLedOnImage, unsigned char* pbLedOffImage, int nWidth, int nHeight)
{
    int nBufferIndex = 0;
    int nMaskBufferIndex = 0;
    unsigned char* pbMaskBuffer = pbGlobalBuffer;
    unsigned char* pbSmoothMaskBuffer = pbGlobalBuffer + 0x12C00;

    int nBlockWidth = 2;
    int nX, nY;
    int nMaskWidth, nMaskHeight;
    nMaskWidth = nWidth / nBlockWidth;
    nMaskHeight = nHeight / nBlockWidth;

    int nLedOffSigma = 0;

    for (nY = 0; nY < nMaskHeight; nY ++)
    {
        for (nX = 0; nX < nMaskWidth; nX ++)
        {
            int nDeltaValue = pbLedOnImage[nBufferIndex] - pbLedOffImage[nBufferIndex];

            nLedOffSigma += pbLedOffImage[nBufferIndex];
	
            pbMaskBuffer[nMaskBufferIndex] = 1;
            if (nDeltaValue < DIFF_CERTAIN_THRESHOLD || (nDeltaValue < DIFF_THRESHOLD && pbLedOnImage[nBufferIndex] > VALUE_SKY) || (pbLedOnImage[nBufferIndex] > CERTAIN_VALUE_SKY))
            {
                pbMaskBuffer[nMaskBufferIndex] = 0;
            }


            nBufferIndex += nBlockWidth;
            nMaskBufferIndex++;
        }
        nBufferIndex += (nBlockWidth - 1) * nWidth;
    }
	
    int nAveLedOff = nLedOffSigma / (nMaskHeight * nMaskWidth);
    if (nAveLedOff < 3)
    {
        return 0;
    }

    //Smoothing Mask
    memcpy(pbSmoothMaskBuffer, pbMaskBuffer, nMaskWidth);
    for (nY = 1; nY < nMaskHeight; nY++)
    {
        for (nX = 1; nX < nMaskWidth; nX++)
        {

			int nDeltaX, nDeltaY; 
            int nValuInBlock = 0;
            int nCurMaskBufferIndex = nY * nMaskWidth + nX;
			

            for (nDeltaY = -1; nDeltaY <= 0; nDeltaY ++)
            {
                for (nDeltaX = -1; nDeltaX <= 0; nDeltaX++)
                {
                    nValuInBlock += pbMaskBuffer[(nY + nDeltaY) * nMaskWidth + nX + nDeltaX];
                }
            }

            if (nValuInBlock >= 2)
            {
                pbSmoothMaskBuffer[nCurMaskBufferIndex] = 1;
            }
            else
            {
                pbSmoothMaskBuffer[nCurMaskBufferIndex] = 0;
            }
        }
    }

    nBufferIndex = 0;

    for (nY = 0; nY < nHeight; nY++)
    {
        for (nX = 0; nX < nWidth; nX++)
        {
            int nMaskIndex = (nY / nBlockWidth) * nMaskWidth + (nX / nBlockWidth);
            //pbLedOnImage[nBufferIndex] = pbLedOnImage[nBufferIndex] * pbSmoothMaskBuffer[nMaskIndex];
            //pbLedOnImage[nBufferIndex] = pbLedOnImage[nBufferIndex];

            int tmp;
            tmp = pbLedOnImage[nBufferIndex] - pbLedOffImage[nBufferIndex];

            if (pbSmoothMaskBuffer[nMaskIndex])
            {
                tmp = pbLedOnImage[nBufferIndex] - ((int)pbLedOffImage[nBufferIndex] * 75 / 100);
            }
            else
            {
                tmp = pbLedOnImage[nBufferIndex] - pbLedOffImage[nBufferIndex];
            }

            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;

            pbLedOnImage[nBufferIndex] = (unsigned char)tmp;

            nBufferIndex++;
        }
    }

    return 1;
}

void rotatePoint(int nOrgWidth, int nOrgHeight, int& nPointX, int& nPointY, int nDegreeAngle)
{
    int nCosThita[4] = { 1, 0, -1, 0 };
    int nSinThita[4] = { 0, 1, 0, -1 };

    int nIndex = (nDegreeAngle / 90) % 4;

    int nDstWidth = nOrgWidth;
    int nDstHeight = nOrgHeight;


    if (nIndex % 2 == 1)
    {
        nDstWidth = nOrgHeight;
        nDstHeight = nOrgWidth;
    }

    int nCurCos = nCosThita[nIndex];
    int nCurSin = nSinThita[nIndex];

    int nOrgCenterX, nOrgCenterY;
    int nDstCenterX, nDstCenterY;

    nOrgCenterX = nOrgWidth >> 1;
    nOrgCenterY = nOrgHeight >> 1;

    nDstCenterX = nDstWidth >> 1;
    nDstCenterY = nDstHeight >> 1;

    int nDeltaX = nPointX - nOrgCenterX;
    int nDeltaY = nPointY - nOrgCenterY;

    int nDesX = nDeltaX * nCurCos - nDeltaY * nCurSin + nDstCenterX;
    int nDesY = nDeltaX * nCurSin + nDeltaY * nCurCos + nDstCenterY;

    nPointX = nDesX;
    nPointY = nDesY;
}




unsigned char bGammaValue_066[256] =
{
/*0.66_Table*/
0,
6,
10,
13,
16,
19,
21,
23,
25,
28,
30,
32,
33,
35,
37,
39,
41,
42,
44,
45,
47,
49,
50,
52,
53,
55,
56,
57,
59,
60,
62,
63,
64,
66,
67,
68,
70,
71,
72,
73,
75,
76,
77,
78,
79,
81,
82,
83,
84,
85,
87,
88,
89,
90,
91,
92,
93,
94,
95,
97,
98,
99,
100,
101,
102,
103,
104,
105,
106,
107,
108,
109,
110,
111,
112,
113,
114,
115,
116,
117,
118,
119,
120,
121,
122,
123,
124,
125,
126,
127,
128,
129,
130,
131,
131,
132,
133,
134,
135,
136,
137,
138,
139,
140,
141,
141,
142,
143,
144,
145,
146,
147,
148,
149,
149,
150,
151,
152,
153,
154,
155,
155,
156,
157,
158,
159,
160,
160,
161,
162,
163,
164,
165,
165,
166,
167,
168,
169,
170,
170,
171,
172,
173,
174,
174,
175,
176,
177,
178,
178,
179,
180,
181,
182,
182,
183,
184,
185,
185,
186,
187,
188,
189,
189,
190,
191,
192,
192,
193,
194,
195,
195,
196,
197,
198,
198,
199,
200,
201,
201,
202,
203,
204,
204,
205,
206,
207,
207,
208,
209,
209,
210,
211,
212,
212,
213,
214,
215,
215,
216,
217,
217,
218,
219,
220,
220,
221,
222,
222,
223,
224,
225,
225,
226,
227,
227,
228,
229,
229,
230,
231,
232,
232,
233,
234,
234,
235,
236,
236,
237,
238,
238,
239,
240,
240,
241,
242,
242,
243,
244,
244,
245,
246,
247,
247,
248,
249,
249,
250,
251,
251,
252,
253,
253,
254,
255,
};

unsigned char bGammaValue_032A[256] =
{
	/*032ATabel*/
    0, 3, 9, 15, 20, 23,
    25, 28, 31, 33,
    35, 36,
    38,
    41,
    44,
    47,
    50,
    55,
    59,
    64,
    69,
    72,
    76,
    79,
    83,
    86,
    88,
    91,
    94,
    97,
    100,
    102,
    105,
    107,
    110,
    112,
    114,
    116,
    118,
    119,
    120,
    121,
    123,
    125,
    126,
    128,
    130,
    132,
    134,
    135,
    136,
    137,
    138,
    139,
    141,
    142,
    144,
    146,
    148,
    149,
    150,
    151,
    152,
    153,
    154,
    155,
    156,
    157,
    158,
    159,
    160,
    161,
    162,
    163,
    164,
    165,
    166,
    167,
    168,
    169,
    170,
    171,
    172,
    173,
    174,
    175,
    176,
    177,
    178,
    179,
    180,
    181,
    182,
    183,
    184,
    184,
    185,
    185,
    186,
    186,
    187,
    187,
    188,
    188,
    189,
    189,
    190,
    190,
    191,
    192,
    192,
    193,
    194,
    195,
    196,
    196,
    197,
    197,
    198,
    198,
    199,
    199,
    200,
    200,
    201,
    201,
    202,
    202,
    203,
    203,
    204,
    204,
    205,
    205,
    206,
    206,
    207,
    207,
    208,
    208,
    209,
    209,
    210,
    210,
    211,
    211,
    212,
    212,
    213,
    213,
    214,
    214,
    215,
    215,
    216,
    216,
    217,
    218,
    218,
    219,
    219,
    220,
    220,
    221,
    222,
    222,
    222,
    223,
    223,
    223,
    224,
    224,
    224,
    225,
    225,
    225,
    226,
    226,
    226,
    227,
    227,
    228,
    228,
    229,
    229,
    230,
    231,
    231,
    232,
    232,
    232,
    233,
    233,
    234,
    234,
    234,
    235,
    235,
    235,
    236,
    236,
    236,
    237,
    237,
    238,
    238,
    239,
    239,
    240,
    240,
    240,
    241,
    241,
    241,
    242,
    242,
    242,
    242,
    243,
    243,
    243,
    244,
    244,
    244,
    245,
    246,
    246,
    247,
    247,
    248,
    248,
    248,
    249,
    249,
    250,
    251,
    251,
    251,
    251,
    251,
    252,
    252,
    252,
    252,
    252,
    253,
    253,
    253,
    253,
    254,
    254,
    254,
    254,
    254,
    254,
    255,
};

void gammaCorrection(unsigned char* pBuffer, int nWidth, int nHeight, int nMode)
{

#if 1
	int nImageSize = nWidth * nHeight;
	int nBufferIndex;

	for (nBufferIndex = 0; nBufferIndex < nImageSize; nBufferIndex++)
	{
		unsigned char bOrgValue, bNewValue;
		bNewValue = 0;
		bOrgValue = pBuffer[nBufferIndex];
		if (nMode == Gamma_032A)
		{
			bNewValue = bGammaValue_032A[bOrgValue];
		}
		else if (nMode == Gamma_066)
		{
			bNewValue = bGammaValue_066[bOrgValue];
		}
		// 		int nNewValue = pow((float)bOrgValue / 255.0f, rGamma) * 255;
		// 		if (nNewValue > 255)
		// 		{
		// 			nNewValue = 255;
		// 		}
		// 		bNewValue = (unsigned char)nNewValue;
		pBuffer[nBufferIndex] = bNewValue;
	}
#endif

	return;
}


#ifndef PI
#define PI	3.1415926549389793
#endif // !PI


//added by KSB 20180801
void ImageRotation_toLandScape(unsigned char* pBuffer, int nSrcWidth, int nSrcHeight, int nDesWidth, int nDesHeight, int rotationAngle)
{
    int nBufferSize = nDesWidth * nDesHeight;
    unsigned char* tmpImageData = (unsigned char*)malloc(nSrcWidth * nSrcHeight);
    memcpy(tmpImageData, pBuffer, nSrcWidth * nSrcHeight);
    memset(pBuffer, 0x0, nBufferSize);

    int nY, nX;

    float rDesCenterX, rDesCenterY;
    rDesCenterX = nDesWidth / 2;
    rDesCenterY = nDesHeight / 2;

    float rSrcCenterX, rSrcCenterY;
    rSrcCenterX = nSrcWidth / 2;
    rSrcCenterY = nSrcHeight / 2;

    if (rotationAngle < 0)
    {
        rotationAngle += 360;
    }
    if (rotationAngle >= 360)
    {
        rotationAngle -= 360;
    }

    float rRadAngle = ((float)rotationAngle * PI) / 180;

    float rCosThita = cos(rRadAngle);
    float rSinThita = sin(rRadAngle);

    for (nY = 0; nY < nDesHeight; nY++)
    {
        for (nX = 0; nX < nDesWidth; nX++)
        {
            int nDesBufferIndex = nY * nDesWidth + nX;

            float rDeltaX, rDeltaY;
            float rSrcX, rSrcY;

            rDeltaX = nX - rDesCenterX;
            rDeltaY = nY - rDesCenterY;

            rSrcX = rDeltaX * rCosThita + rDeltaY * rSinThita + rSrcCenterX;
            rSrcY = rDeltaY * rCosThita - rDeltaX * rSinThita + rSrcCenterY;

            int nSrcX, nSrcY;
            nSrcX = (int)rSrcX;
            nSrcY = (int)rSrcY;

            if (rSrcX < 0 || rSrcX >= nSrcWidth - 2 || rSrcY < 0 || rSrcY >= nSrcHeight - 2)
            {
                //pDesBuffer[nBufferIndex] = 0;
            }
            else
            {
                int nSourceIndex;
                nSourceIndex = (nSrcY * nSrcWidth + nSrcX);

                if (nSrcX == rSrcX && nSrcY == rSrcY)
                {
                    pBuffer[nDesBufferIndex] = tmpImageData[nSourceIndex];
                }
                else
                {
                    float rOffsetX = rSrcX - nSrcX;
                    float rOffsetY = rSrcY - nSrcY;

                    pBuffer[nDesBufferIndex] = (unsigned char)((float)tmpImageData[nSourceIndex] * (1 - rOffsetX) * (1 - rOffsetY) +
                        (float)tmpImageData[nSourceIndex + nSrcWidth] * (1 - rOffsetX) * (rOffsetY)+
                        (float)tmpImageData[nSourceIndex + 1] * (rOffsetX)* (1 - rOffsetY) +
                        (float)tmpImageData[nSourceIndex + nSrcWidth + 1] * (rOffsetX)* (rOffsetY));
                }
            }
        }
    }
	
    free(tmpImageData);
	
    return;
}
void rotatePointInFixSize(int nWidth, int nHeight, int& nPointX, int& nPointY, int nDegreeAngle)
{
    float rRadAngle = ((float)nDegreeAngle * PI) / 180;

    float rCosThita = cos(rRadAngle);
    float rSinThita = sin(rRadAngle);

    int nCenterX, nCenterY;

    nCenterX = nWidth >> 1;
    nCenterY = nHeight >> 1;

    int nDeltaX = nPointX - nCenterX;
    int nDeltaY = nPointY - nCenterY;
    int nDesX = nDeltaX * rCosThita - nDeltaY * rSinThita + nCenterX;
    int nDesY = nDeltaX * rSinThita + nDeltaY * rCosThita + nCenterY;

    nPointX = nDesX;
    nPointY = nDesY;
}


#define VALID_MIN_WIDTH	210
#define VALID_MAX_WIDTH	360


void applySharpenFilter(unsigned char* pBuffer, int nWidth, int nHeight)
{
    if (!pBuffer || nWidth < 0|| nHeight < 0)
    {
        return;
    }
    int nX, nY;
    for (nY = 1; nY < nHeight - 1; nY++)
    {
        for (nX = 1; nX < nWidth - 1; nX++)
        {
            int nBufferIndex = nY * nWidth + nX;
            int nValue = (int)(pBuffer[nBufferIndex]) * 5 - (pBuffer[nBufferIndex - 1]) - (pBuffer[nBufferIndex + 1])
                - (pBuffer[nBufferIndex - nWidth + 1]) - (pBuffer[nBufferIndex + nWidth]);
            if (nValue > 255)
            {
                nValue = 255;
            }
            if (nValue < 0)
            {
                nValue = 0;
            }
            pbGlobalBuffer[nBufferIndex] = (int)nValue;
        }
    }
    memcpy(pBuffer, pbGlobalBuffer, 0x4B000);
    return;
}
