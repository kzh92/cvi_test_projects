

#include "ImageProcessing.h"
#include "FaceRetrievalSystem.h"
//#include "jpge.h"

#include <string.h>
// #include <malloc.h>
// #include <stdlib.h>
#include <math.h>
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif // __ARM_NEON
#include "common_types.h"

#define __max(a, b) (a > b ? a : b)
#define __min(a, b) (a < b ? a : b)

unsigned char g_jpgTmpData[N_MAX_JPG_FACE_IMAGE_SIZE * 2];

#ifdef __ARM_NEON
inline static int f_sad_16_neon(const uint8_t* a, const uint8_t* b)
{
    int32_t r[4] = { 0, 0, 0, 0 };
    uint8x16_t va, vb, vr;

    va = vld1q_u8(a);
    vb = vld1q_u8(b);

    vr = vabdq_u8(va, vb);

    uint16x8_t vr1 = vpaddlq_u8(vr);
    uint32x4_t vr2 = vpaddlq_u16(vr1);
    uint64x2_t vr3 = vpaddlq_u32(vr2);

    vst1q_u64(reinterpret_cast<uint64_t*>(r), vr3);

    return r[0] + r[2];
}
#endif // __ARM_NEON

void ScaleImage(unsigned char *pbOrg, unsigned char* pbScaledImage)
{
    memcpy(pbScaledImage, pbOrg, g_xEngineParam.nDetectionWidth * g_xEngineParam.nDetectionHeight);
}

int CalcDiffImageA(unsigned char* pbDst, unsigned char* pbSrcLedOn, unsigned char *pbLedOff, int nDiffX, int nDiffY, int nLeft, int nTop, int nRight, int nBottom)
{
    for(int y = 0; y < g_xEngineParam.nDetectionHeight; y ++)
    {
        int y1 = y + nDiffY;
        if(y1 < 0)
            y1 = 0;
        else if(y1 >= g_xEngineParam.nDetectionHeight)
            y1 = g_xEngineParam.nDetectionHeight - 1;

        for(int x = 0; x < g_xEngineParam.nDetectionWidth; x ++)
        {
            int x1 = x + nDiffX;
            if(x1 < 0)
                x1 = 0;
            else if(x1 >= g_xEngineParam.nDetectionWidth)
                x1 = g_xEngineParam.nDetectionWidth - 1;

            int tmp = pbSrcLedOn[y * g_xEngineParam.nDetectionWidth + x] - pbLedOff[y1 * g_xEngineParam.nDetectionWidth + x1];
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            pbDst[y * g_xEngineParam.nDetectionWidth + x] = tmp;
        }
    }

    return 0;
}

int CalcDiffImage(unsigned char* dst, unsigned char* src1, unsigned char *src2, int diffX, int diffY, int nLeft, int nTop, int nRight, int nBottom, float rAlpha, float rBeta)
{
#if 0
    for(int y = 0; y < g_xEngineParam.nDetectionHeight; y ++)
    {
        int y1 = y + nDiffY;
        if(y1 < 0)
            y1 = 0;
        else if(y1 >= g_xEngineParam.nDetectionHeight)
            y1 = g_xEngineParam.nDetectionHeight - 1;

        for(int x = 0; x < g_xEngineParam.nDetectionWidth; x ++)
        {
            int x1 = x + nDiffX;
            if(x1 < 0)
                x1 = 0;
            else if(x1 >= g_xEngineParam.nDetectionWidth)
                x1 = g_xEngineParam.nDetectionWidth - 1;

            int tmp = pbSrcLedOn[y * g_xEngineParam.nDetectionWidth + x] - pbLedOff[y1 * g_xEngineParam.nDetectionWidth + x1];
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            pbDst[y * g_xEngineParam.nDetectionWidth + x] = tmp;
        }
    }
#else
    //unsigned char *pSrc1 = src1;
    unsigned char *pDes = dst;
    _u8 *pSrc2, *pSrc2_1;
    //memset(dst, 0, g_xEngineParam.nDetectionHeight * g_xEngineParam.nDetectionWidth);
    memcpy(dst, src1, g_xEngineParam.nDetectionHeight * g_xEngineParam.nDetectionWidth);

    int nInitIndex =  nTop * g_xEngineParam.nDetectionWidth + nLeft;
    _u8* pDesInit = dst + nInitIndex;
    //_u8* pSrcInit = src1 + nInitIndex;

    int nDeltaLeft, nDeltaRight;
    nDeltaLeft = nLeft + diffX;
    nDeltaRight = nRight + diffX;
    if (nDeltaLeft < 0)
    {
        nDeltaLeft = 0;
    }

    if(nDeltaRight >= g_xEngineParam.nDetectionWidth)
    {
        nDeltaRight = g_xEngineParam.nDetectionWidth - 1;
    }


    _u8* pInitLeftLimit = src2 + (nTop + diffY) * g_xEngineParam.nDetectionWidth + (nDeltaLeft);
    _u8* pInitRightLimit = src2 + (nTop + diffY) * g_xEngineParam.nDetectionWidth + (nDeltaRight);

    _u8* pMinLeftLimit = src2 + nDeltaLeft;
    _u8* pMinRightLimit = src2 + nDeltaRight;

    _u8* pMaxLeftLimit = pMinLeftLimit + (g_xEngineParam.nDetectionHeight - 1) * g_xEngineParam.nDetectionWidth;
    _u8* pMaxRightLimit = pMinRightLimit + (g_xEngineParam.nDetectionHeight - 1) * g_xEngineParam.nDetectionWidth;
    _u8* pSrc2Init = src2 + (nTop + diffY) * g_xEngineParam.nDetectionWidth + (nLeft + diffX);

    int nAlpha = ((int)(rAlpha * 1024));
    int nBeta = ((int)(rBeta * 1024));

    int nAlphaIsOne, nBetaIsOne;

    nAlphaIsOne = 0;
    nBetaIsOne = 0;

    if (rAlpha == 1.0f)
    {
        nAlphaIsOne = 1;
    }

    if (rBeta == -1.0f)
    {
        nBetaIsOne = 1;
    }


    for (int y = nTop; y < nBottom; y++)
    {

        _u8 *pCurLeftLimit, *pCurRightLimit;
        pCurLeftLimit = pInitLeftLimit;
        pCurRightLimit = pInitRightLimit;
        int y1 = y + diffY;
        if (y1 < 0)
        {
            pCurLeftLimit = pMinLeftLimit;
            pCurRightLimit = pMinRightLimit;
        }
        else if (y1 >= g_xEngineParam.nDetectionHeight)
        {
            pCurLeftLimit = pMaxLeftLimit;
            pCurRightLimit = pMaxRightLimit;
        }

        //pSrc1 = pSrcInit;
        pDes = pDesInit;
        pSrc2 = pSrc2Init;
        for (int x = nLeft; x < nRight; x++)
        {
            pSrc2_1 = pSrc2;
            if(pSrc2_1 < pCurLeftLimit)
            {
                pSrc2_1 = pCurLeftLimit;
            }

            if(pSrc2_1 > pCurRightLimit)
            {
                pSrc2_1 = pCurRightLimit;
            }
            int tmp;
            //int tmp = (int)(*pSrc1) * rAlpha -	*pSrc2_1;

            if (nAlphaIsOne && nBetaIsOne)
            {
                tmp = (int)(*pDes) - *pSrc2_1;
            }
            else if (nBetaIsOne)
            {
                tmp = (((int)(*pDes) * nAlpha) >> 10) - *pSrc2_1;
            }
            else if (nAlphaIsOne)
            {
                tmp = (int)(*pDes) + (((int)(*pSrc2_1) * nBeta) >> 10);
            }
            else
            {
                tmp = (((int)(*pDes) * nAlpha) >> 10) + (((int)(*pSrc2_1) * nBeta) >> 10);
            }


            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            *pDes = tmp;
            pDes ++;
            //pSrc1 ++;
            pSrc2 ++;
        }

        //pSrcInit += g_xEngineParam.nDetectionWidth;
        pDesInit += g_xEngineParam.nDetectionWidth;
        pInitLeftLimit += g_xEngineParam.nDetectionWidth;
        pInitRightLimit += g_xEngineParam.nDetectionWidth;
        pSrc2Init += g_xEngineParam.nDetectionWidth;
    }
#endif

    return 0;
}


int CalcDiffImage_Crop(unsigned char* dst, unsigned char* src1, unsigned char *src2, int diffX, int diffY, int nLeft, int nTop, int nRight, int nBottom, int nCropX, int nCropY, int nCropWidth, int nCropHeight, float rAlpha, float rBeta)
{
#if 0
    for(int y = 0; y < g_xEngineParam.nDetectionHeight; y ++)
    {
        int y1 = y + nDiffY;
        if(y1 < 0)
            y1 = 0;
        else if(y1 >= g_xEngineParam.nDetectionHeight)
            y1 = g_xEngineParam.nDetectionHeight - 1;

        for(int x = 0; x < g_xEngineParam.nDetectionWidth; x ++)
        {
            int x1 = x + nDiffX;
            if(x1 < 0)
                x1 = 0;
            else if(x1 >= g_xEngineParam.nDetectionWidth)
                x1 = g_xEngineParam.nDetectionWidth - 1;

            int tmp = pbSrcLedOn[y * g_xEngineParam.nDetectionWidth + x] - pbLedOff[y1 * g_xEngineParam.nDetectionWidth + x1];
            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            pbDst[y * g_xEngineParam.nDetectionWidth + x] = tmp;
        }
    }
#else
    //unsigned char *pSrc1 = src1;
    unsigned char *pDes = dst;
    _u8 *pSrc2, *pSrc2_1;
    //memset(dst, 0, g_xEngineParam.nDetectionHeight * g_xEngineParam.nDetectionWidth);

    //copy src1->dst
    int nY;
    for(nY = 0; nY < nCropHeight; nY ++)
    {
        memcpy(dst + nY * nCropWidth, src1 + (nY + nCropY) * g_xEngineParam.nDetectionWidth + nCropX, nCropWidth);
    }

    //memcpy(dst, src1, g_xEngineParam.nDetectionHeight * g_xEngineParam.nDetectionWidth);

    int nInitIndex =  (nTop - nCropY) * nCropWidth + nLeft - nCropX;
    _u8* pDesInit = dst + nInitIndex;
    //_u8* pSrcInit = src1 + nInitIndex;

    int nDeltaLeft, nDeltaRight;
    nDeltaLeft = nLeft + diffX;
    nDeltaRight = nRight + diffX;
    if (nDeltaLeft < 0)
    {
        nDeltaLeft = 0;
    }

    if(nDeltaRight >= g_xEngineParam.nDetectionWidth)
    {
        nDeltaRight = g_xEngineParam.nDetectionWidth - 1;
    }


    _u8* pInitLeftLimit = src2 + (nTop + diffY) * g_xEngineParam.nDetectionWidth + (nDeltaLeft);
    _u8* pInitRightLimit = src2 + (nTop + diffY) * g_xEngineParam.nDetectionWidth + (nDeltaRight);

    _u8* pMinLeftLimit = src2 + nDeltaLeft;
    _u8* pMinRightLimit = src2 + nDeltaRight;

    _u8* pMaxLeftLimit = pMinLeftLimit + (g_xEngineParam.nDetectionHeight - 1) * g_xEngineParam.nDetectionWidth;
    _u8* pMaxRightLimit = pMinRightLimit + (g_xEngineParam.nDetectionHeight - 1) * g_xEngineParam.nDetectionWidth;
    _u8* pSrc2Init = src2 + (nTop + diffY) * g_xEngineParam.nDetectionWidth + (nLeft + diffX);

    int nAlpha = ((int)(rAlpha * 1024));
    int nBeta = ((int)(rBeta * 1024));

    int nAlphaIsOne, nBetaIsOne;

    nAlphaIsOne = 0;
    nBetaIsOne = 0;

    if (rAlpha == 1.0f)
    {
        nAlphaIsOne = 1;
    }

    if (rBeta == -1.0f)
    {
        nBetaIsOne = 1;
    }


    for (int y = nTop; y < nBottom; y++)
    {

        _u8 *pCurLeftLimit, *pCurRightLimit;
        pCurLeftLimit = pInitLeftLimit;
        pCurRightLimit = pInitRightLimit;
        int y1 = y + diffY;
        if (y1 < 0)
        {
            pCurLeftLimit = pMinLeftLimit;
            pCurRightLimit = pMinRightLimit;
        }
        else if (y1 >= g_xEngineParam.nDetectionHeight)
        {
            pCurLeftLimit = pMaxLeftLimit;
            pCurRightLimit = pMaxRightLimit;
        }

        //pSrc1 = pSrcInit;
        pDes = pDesInit;
        pSrc2 = pSrc2Init;
        for (int x = nLeft; x < nRight; x++)
        {
            pSrc2_1 = pSrc2;
            if(pSrc2_1 < pCurLeftLimit)
            {
                pSrc2_1 = pCurLeftLimit;
            }

            if(pSrc2_1 > pCurRightLimit)
            {
                pSrc2_1 = pCurRightLimit;
            }
            int tmp;
            //int tmp = (int)(*pSrc1) * rAlpha -	*pSrc2_1;

            if (nAlphaIsOne && nBetaIsOne)
            {
                tmp = (int)(*pDes) - *pSrc2_1;
            }
            else if (nBetaIsOne)
            {
                tmp = (((int)(*pDes) * nAlpha) >> 10) - *pSrc2_1;
            }
            else if (nAlphaIsOne)
            {
                tmp = (int)(*pDes) + (((int)(*pSrc2_1) * nBeta) >> 10);
            }
            else
            {
                tmp = (((int)(*pDes) * nAlpha) >> 10) + (((int)(*pSrc2_1) * nBeta) >> 10);
            }


            if (tmp < 0) tmp = 0;
            if (tmp > 255) tmp = 255;
            *pDes = tmp;
            pDes ++;
            //pSrc1 ++;
            pSrc2 ++;
        }

        //pSrcInit += g_xEngineParam.nDetectionWidth;
        pDesInit += nCropWidth;
        pInitLeftLimit += g_xEngineParam.nDetectionWidth;
        pInitRightLimit += g_xEngineParam.nDetectionWidth;
        pSrc2Init += g_xEngineParam.nDetectionWidth;
    }
#endif

    return 0;
}



void NormalizeImage(int* pnImage, unsigned char* pbNormalized, int nH, int nW)
{
    int		nMax = -100000;
    int		nMin = 100000;
    int nSize = nH * nW;
    for (int i = 0; i < nSize; i++)
    {
        nMax = __max(nMax, pnImage[i]);
        nMin = __min(nMin, pnImage[i]);
    }
    if (nMax == nMin)
    {
        for (int i = 0; i < nSize; i++)
            pbNormalized[i] = 128;
    }
    else
    {
        for (int i = 0; i < nSize; i++)
        {
            pbNormalized[i] = (unsigned char)((pnImage[i] - nMin) * 255 / (nMax - nMin));
            /*if (pbNormalized[i] > 50)
            {
                pbNormalized[i] = 255;
            }
            else
            {
                pbNormalized[i] = 0;
            }*/
        }
    }
}

void Sobel_Process(unsigned char* pbSrc, unsigned char* pbEdge, int nH, int nW)
{
    int	nEdgeX, nEdgeY;
    int* pnEdge, *pnTemp;
    int nSize = nH * nW;
    pnEdge = (int*)malloc(nSize * sizeof(int));
    memset(pnEdge, 0, sizeof(int) * nSize);
    pnTemp = pnEdge + nW + 1;
    pbSrc += (nW + 1);
    for (int nY = 1; nY < nH - 1; nY++)
    {
        for (int nX = 1; nX < nW - 1; nX++)
        {
            nEdgeY =
                *(pbSrc - nW - 1) + (*(pbSrc - nW) << 1) + *(pbSrc - nW + 1) -
                *(pbSrc + nW - 1) - (*(pbSrc + nW) << 1) - *(pbSrc + nW + 1);
            nEdgeX =
                *(pbSrc - nW + 1) + (*(pbSrc + 1) << 1) + *(pbSrc + nW + 1) -
                *(pbSrc - nW - 1) - (*(pbSrc - 1) << 1) - *(pbSrc + nW - 1);
            *pnTemp = abs(nEdgeX) + abs(nEdgeY);
            pnTemp ++;
            pbSrc ++;
        }
        pnTemp += 2;
        pbSrc += 2;
    }

    NormalizeImage(pnEdge, pbEdge, nH, nW);
    free(pnEdge);
}

int GetFaceMotion(unsigned char* pbImage1, unsigned char* pbImage2, int nImageHeight, int nImageWidth, int nLeft, int nTop, int nH, int nW, int* pnXMotion, int* pnYMotion)
{
    unsigned char *pbTmpImage2 = (unsigned char *) malloc(nImageWidth * nImageHeight);
    unsigned char *pbTmpImage1 = (unsigned char *) malloc(nImageWidth * nImageHeight);
    int MAX_OFFSET = MOTION_OFFSET;
    int MAX_OFFSET2 = MAX_OFFSET  * 2;

    int i, j, nMin = 1000000000;
    unsigned char *pbPtr1, *pbPtr2, a, b;
    int **ppnError = (int **) malloc(sizeof(int *) * MAX_OFFSET2);

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        ppnError[i] = (int *) malloc(sizeof(int) * MAX_OFFSET2);
    }

    Sobel_Process(pbImage1, pbTmpImage1, nImageHeight, nImageWidth);
    Sobel_Process(pbImage2, pbTmpImage2, nImageHeight, nImageWidth);

    if (((nLeft + nW) > (nImageWidth - 1)) || ((nTop + nH) > (nImageHeight - 1)) || (nLeft < 0) ||
        (nTop < 0))
    {
        for (i = 0; i < MAX_OFFSET2; i++)
            free(ppnError[i]);
        free(ppnError);

        free(pbTmpImage1);
        free(pbTmpImage2);

        return 0;
    }

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            ppnError[i][j] = 0;
        }
    }

    unsigned char* pbTmpImage2_Limit = pbTmpImage2 + nImageWidth * nImageHeight;

    int nStep;
    nStep = 1;

    unsigned char* pbPtr1_init = pbTmpImage1 + nTop * nImageWidth + nLeft;
    unsigned char* pbPtr2_init = pbTmpImage2 + nTop * nImageWidth + nLeft;

    int nImageWidth_nStep = nImageWidth * nStep;
    int nImageWidth_nStep_nW = nImageWidth_nStep - nW;

    int nI_ImageWidth = - MAX_OFFSET * nImageWidth_nStep;
    int MAX_OFFSET_nStep = MAX_OFFSET * nStep;


    for (i = - MAX_OFFSET; i < MAX_OFFSET; i++)
    {
        int nI_ImageWidth_j = nI_ImageWidth - MAX_OFFSET_nStep;
        for (j = - MAX_OFFSET; j < MAX_OFFSET; j++)
        {
            pbPtr1 = pbPtr1_init;
            pbPtr2 = pbPtr2_init + nI_ImageWidth_j;

            for (int nY = 0; nY < nH; nY += nStep)
            {
                for (int nX = 0; nX < nW; nX += nStep)
                {
                    a = *pbPtr1;
                    //b = *(pbPtr2 + (i * nImageWidth + j) * nStep);
                    if(pbPtr2 < pbTmpImage2)
                    {
                        b = *pbTmpImage2;
                    }
                    else if(pbPtr2 >= pbTmpImage2_Limit)
                    {
                        b = *(pbTmpImage2_Limit - 1);
                    }
                    else
                    {
                        b = *pbPtr2;
                    }
                    ppnError[i + MAX_OFFSET][j + MAX_OFFSET] += (a > b) ? a - b : b - a;
                    pbPtr1 += nStep;
                    pbPtr2 += nStep;
                }
                pbPtr1 += nImageWidth_nStep_nW;
                pbPtr2 += nImageWidth_nStep_nW;
            }
            nI_ImageWidth_j += nStep;
        }
        nI_ImageWidth += nImageWidth_nStep;
    }

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            if (nMin > ppnError[i][j])
            {
                nMin = ppnError[i][j];
                *pnXMotion = (j - MAX_OFFSET);
                *pnYMotion = (i - MAX_OFFSET);
            }
        }
    }

    for (i = 0; i < MAX_OFFSET2; i++)
        free(ppnError[i]);
    free(ppnError);
    free(pbTmpImage1);
    free(pbTmpImage2);

    return 0;
}

void NormalizeImage_Fast(int* pnImage, unsigned char* pbNormalized, int nH, int nW, int nFaceLeft, int nFaceTop, int nFaceH, int nFaceW, int nOff)
{
    int		nMax = -100000;
    int		nMin = 100000;
    int nSize = (nH + nOff * 2) * (nW + nOff * 2);

    int nSLeft, nSTop, nSRight, nSBottom;
    int nOffW;
    nSLeft = __max(nFaceLeft - nOff, 1);
    nSTop = __max(nFaceTop - nOff, 1);
    nSRight = __min(nW - 2, nFaceLeft + nFaceW + nOff - 1);
    nSBottom = __min(nH - 2, nFaceTop + nFaceH + nOff - 1);
    nOffW = nOff * 2 + nW;

    for (int nY = nSTop; nY <= nSBottom; nY++)
    {
        int* pTmpImg = pnImage + nY * nW;
        for (int nX = nSLeft; nX <= nSRight; nX++)
        {
            nMax = __max(nMax, pTmpImg[nX]);
            nMin = __min(nMin, pTmpImg[nX]);
        }
    }

    if (nMax == nMin)
        memset(pbNormalized, 0x80, nSize);
    else
    {
        for (int nY = nSTop + 1; nY <= nSBottom - 1; nY++)
        {
            int* pTmpImg = pnImage + nY * nW;
            unsigned char* pbTmpNor = pbNormalized + (nY + nOff) * nOffW + nOff;
            for (int nX = nSLeft + 1; nX <= nSRight - 1; nX++)
            {
                pbTmpNor[nX] = (unsigned char)((pTmpImg[nX] - nMin) * 255 / (nMax - nMin));
            }
        }
    }
}

void Sobel_Process_Fast_buffer(unsigned char* pbSrc, unsigned char* pbEdge, unsigned char* pbTempBuffer, int nH, int nW, int nFaceLeft, int nFaceTop, int nFaceH, int nFaceW, int nOff)
{
    int nEdgeX, nEdgeY;
    int* pnEdge;
    int nSize = nH * nW;
    pnEdge = (int*)pbTempBuffer;
    memset(pnEdge, 0, sizeof(int)* nSize);

    int nSLeft, nSTop, nSRight, nSBottom;
    nSLeft = __max(nFaceLeft - nOff, 1);
    nSTop = __max(nFaceTop - nOff, 1);
    nSRight = __min(nW - 2, nFaceLeft + nFaceW + nOff - 1);
    nSBottom = __min(nH - 2, nFaceTop + nFaceH + nOff - 1);

    for (int nY = nSTop; nY <= nSBottom; nY++)
    {
        unsigned char* pbCurSrc = pbSrc + nY * nW + nSLeft;
        int* pnCurTemp = pnEdge + nY * nW + nSLeft;

        for (int nX = nSLeft; nX <= nSRight; nX++)
        {
            nEdgeY =
                *(pbCurSrc - nW - 1) + (*(pbCurSrc - nW) << 1) + *(pbCurSrc - nW + 1) -
                *(pbCurSrc + nW - 1) - (*(pbCurSrc + nW) << 1) - *(pbCurSrc + nW + 1);
            nEdgeX =
                *(pbCurSrc - nW + 1) + (*(pbCurSrc + 1) << 1) + *(pbCurSrc + nW + 1) -
                *(pbCurSrc - nW - 1) - (*(pbCurSrc - 1) << 1) - *(pbCurSrc + nW - 1);
            *pnCurTemp = abs(nEdgeX) + abs(nEdgeY);
            pnCurTemp++;
            pbCurSrc++;
        }
    }

    NormalizeImage_Fast(pnEdge, pbEdge, nH, nW, nFaceLeft, nFaceTop, nFaceH, nFaceW, nOff);
    //free(pnEdge);
}

void Sobel_Process_Fast(unsigned char* pbSrc, unsigned char* pbEdge, int nH, int nW, int nFaceLeft, int nFaceTop, int nFaceH, int nFaceW, int nOff)
{
    int	nEdgeX, nEdgeY;
    int* pnEdge;
    int nSize = nH * nW;
    pnEdge = (int*)malloc(nSize * sizeof(int));
    memset(pnEdge, 0, sizeof(int)* nSize);

    int nSLeft, nSTop, nSRight, nSBottom;
    nSLeft = __max(nFaceLeft - nOff, 1);
    nSTop = __max(nFaceTop - nOff, 1);
    nSRight = __min(nW - 2, nFaceLeft + nFaceW + nOff - 1);
    nSBottom = __min(nH - 2, nFaceTop + nFaceH + nOff - 1);

    for (int nY = nSTop; nY <= nSBottom; nY++)
    {
        unsigned char* pbCurSrc = pbSrc + nY * nW + nSLeft;
        int* pnCurTemp = pnEdge + nY * nW + nSLeft;

        for (int nX = nSLeft; nX <= nSRight; nX++)
        {
            nEdgeY =
                *(pbCurSrc - nW - 1) + (*(pbCurSrc - nW) << 1) + *(pbCurSrc - nW + 1) -
                *(pbCurSrc + nW - 1) - (*(pbCurSrc + nW) << 1) - *(pbCurSrc + nW + 1);
            nEdgeX =
                *(pbCurSrc - nW + 1) + (*(pbCurSrc + 1) << 1) + *(pbCurSrc + nW + 1) -
                *(pbCurSrc - nW - 1) - (*(pbCurSrc - 1) << 1) - *(pbCurSrc + nW - 1);
            *pnCurTemp = abs(nEdgeX) + abs(nEdgeY);
            pnCurTemp++;
            pbCurSrc++;
        }
    }

    NormalizeImage_Fast(pnEdge, pbEdge, nH, nW, nFaceLeft, nFaceTop, nFaceH, nFaceW, nOff);
    free(pnEdge);
}

int GetFaceMotion_Fast(unsigned char* pbImage1, unsigned char* pbImage2, int nImageHeight, int nImageWidth, int nFaceLeft, int nFaceTop, int nFaceH, int nFaceW, int* pnXMotion, int* pnYMotion)
{
    int MAX_OFFSET = MOTION_OFFSET;
    int MAX_OFFSET2 = MAX_OFFSET * 2;
    unsigned char *pbTmpImage1 = (unsigned char *)malloc(nImageWidth * nImageHeight);
    unsigned char *pbTmpImage2 = (unsigned char *)malloc((nImageWidth + MAX_OFFSET2) * (nImageHeight + MAX_OFFSET2));

    int i, j, nMin = 1000000000;
    unsigned char *pbPtr1, *pbPtr2;
    int **ppnError = (int **)malloc(sizeof(int *)* MAX_OFFSET2);

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        ppnError[i] = (int *)malloc(sizeof(int)* MAX_OFFSET2);
    }

    Sobel_Process_Fast(pbImage1, pbTmpImage1, nImageHeight, nImageWidth, nFaceLeft, nFaceTop, nFaceH, nFaceW, 0);
    Sobel_Process_Fast(pbImage2, pbTmpImage2, nImageHeight, nImageWidth, nFaceLeft, nFaceTop, nFaceH, nFaceW, MAX_OFFSET);

    if (((nFaceLeft + nFaceW) >(nImageWidth - 1)) || ((nFaceTop + nFaceH) > (nImageHeight - 1)) || (nFaceLeft < 0) ||
        (nFaceTop < 0))
    {
        for (i = 0; i < MAX_OFFSET2; i++)
            free(ppnError[i]);
        free(ppnError);

        free(pbTmpImage1);
        free(pbTmpImage2);

        return 0;
    }

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            ppnError[i][j] = 0;
        }
    }

    int nOffImageW = nImageWidth + MAX_OFFSET2;

    int nSLeft, nSTop, nSRight, nSBottom;
    nSLeft = nFaceLeft;
    nSTop = nFaceTop;
    nSRight = nFaceLeft + nFaceW - 1;
    nSBottom = nFaceTop + nFaceH - 1;

#ifndef __ARM_NEON

    for (int nY = nSTop; nY <= nSBottom; nY++)
    {
        pbPtr1 = pbTmpImage1 + nY * nImageWidth;
        for (int nX = nSLeft; nX <= nSRight; nX++)
        {
            unsigned char a = pbPtr1[nX];
            for (int i = 0; i < MAX_OFFSET2; i++)
            {
                pbPtr2 = pbTmpImage2 + (nY + i) * nOffImageW;
                for (int j = 0; j < MAX_OFFSET2; j++)
                {
                    unsigned char b = pbPtr2[nX + j];
                    ppnError[i][j] += (a > b) ? a - b : b - a;
                }
            }
        }
    }

#else // !__ARM_NEON

    int nRightPad = (nSRight - nSLeft + 1) % 16;
    if (nRightPad > 0)
        nRightPad = 16 - nRightPad;

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            pbPtr1 = pbTmpImage1 + nSTop * nImageWidth;
            pbPtr2 = pbTmpImage2 + j + i * nOffImageW + nSTop * nOffImageW;

            for (int nY = nSTop; nY <= nSBottom; nY++)
            {
                memcpy(pbPtr1 + nSRight + 1, pbPtr2 + nSRight + 1, sizeof(int)* nRightPad);

                for (int nX = nSLeft; nX <= nSRight + nRightPad; nX += 16)
                {
                    ppnError[i][j] += f_sad_16_neon(pbPtr1 + nX, pbPtr2 + nX);
                }

                pbPtr1 += nImageWidth;
                pbPtr2 += nOffImageW;
            }
        }
    }
#endif // !__ARM_NEON

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            if (nMin > ppnError[i][j])
            {
                nMin = ppnError[i][j];
                *pnXMotion = (j - MAX_OFFSET);
                *pnYMotion = (i - MAX_OFFSET);
            }
        }
    }

    for (i = 0; i < MAX_OFFSET2; i++)
        free(ppnError[i]);
    free(ppnError);

    free(pbTmpImage1);
    free(pbTmpImage2);

    return 0;
}

int GetFaceMotion_Fast_fromTempBuffer(unsigned char* pbImage1, unsigned char* pbImage2, int nImageHeight, int nImageWidth, int nFaceLeft, int nFaceTop, int nFaceH, int nFaceW, int* pnXMotion, int* pnYMotion, unsigned char* pTempBuffer, unsigned char* pTempBuffer2)
{
    int MAX_OFFSET = MOTION_OFFSET;
    int MAX_OFFSET2 = MAX_OFFSET * 2;
    unsigned char* pCurTempBuffer = pTempBuffer;
    unsigned char *pbTmpImage1 = pCurTempBuffer;
    pCurTempBuffer += (nImageWidth * nImageHeight);
    unsigned char *pbTmpImage2 = pCurTempBuffer;
    pCurTempBuffer += ((nImageWidth + MAX_OFFSET2) * (nImageHeight + MAX_OFFSET2));

    int i, j, nMin = 1000000000;
    unsigned char *pbPtr1, *pbPtr2;
    //int **ppnError = (int **)malloc(sizeof(int *)* MAX_OFFSET2);
    int **ppnError = (int**)pCurTempBuffer;
    pCurTempBuffer += sizeof(int *)* MAX_OFFSET2;

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        //ppnError[i] = (int *)malloc(sizeof(int)* MAX_OFFSET2);
        ppnError[i] = (int*)pCurTempBuffer;
        pCurTempBuffer += sizeof(int)* MAX_OFFSET2;
    }

    Sobel_Process_Fast_buffer(pbImage1, pbTmpImage1, pTempBuffer2, nImageHeight, nImageWidth, nFaceLeft, nFaceTop, nFaceH, nFaceW, 0);
    Sobel_Process_Fast_buffer(pbImage2, pbTmpImage2, pTempBuffer2, nImageHeight, nImageWidth, nFaceLeft, nFaceTop, nFaceH, nFaceW, MAX_OFFSET);

    if (((nFaceLeft + nFaceW) >(nImageWidth - 1)) || ((nFaceTop + nFaceH) > (nImageHeight - 1)) || (nFaceLeft < 0) ||
        (nFaceTop < 0))
    {
        // for (i = 0; i < MAX_OFFSET2; i++)
        //     free(ppnError[i]);
        // free(ppnError);

        // free(pbTmpImage1);
        // free(pbTmpImage2);

        return 0;
    }

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            ppnError[i][j] = 0;
        }
    }

    int nOffImageW = nImageWidth + MAX_OFFSET2;

    int nSLeft, nSTop, nSRight, nSBottom;
    nSLeft = nFaceLeft;
    nSTop = nFaceTop;
    nSRight = nFaceLeft + nFaceW - 1;
    nSBottom = nFaceTop + nFaceH - 1;

#ifndef __ARM_NEON

    for (int nY = nSTop; nY <= nSBottom; nY++)
    {
        pbPtr1 = pbTmpImage1 + nY * nImageWidth;
        for (int nX = nSLeft; nX <= nSRight; nX++)
        {
            unsigned char a = pbPtr1[nX];
            for (int i = 0; i < MAX_OFFSET2; i++)
            {
                pbPtr2 = pbTmpImage2 + (nY + i) * nOffImageW;
                for (int j = 0; j < MAX_OFFSET2; j++)
                {
                    unsigned char b = pbPtr2[nX + j];
                    ppnError[i][j] += (a > b) ? a - b : b - a;
                }
            }
        }
    }

#else // !__ARM_NEON

    int nRightPad = (nSRight - nSLeft + 1) % 16;
    if (nRightPad > 0)
        nRightPad = 16 - nRightPad;

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            pbPtr1 = pbTmpImage1 + nSTop * nImageWidth;
            pbPtr2 = pbTmpImage2 + j + i * nOffImageW + nSTop * nOffImageW;

            for (int nY = nSTop; nY <= nSBottom; nY++)
            {
                memcpy(pbPtr1 + nSRight + 1, pbPtr2 + nSRight + 1, sizeof(int)* nRightPad);

                for (int nX = nSLeft; nX <= nSRight + nRightPad; nX += 16)
                {
                    ppnError[i][j] += f_sad_16_neon(pbPtr1 + nX, pbPtr2 + nX);
                }

                pbPtr1 += nImageWidth;
                pbPtr2 += nOffImageW;
            }
        }
    }
#endif // !__ARM_NEON

    for (i = 0; i < MAX_OFFSET2; i++)
    {
        for (j = 0; j < MAX_OFFSET2; j++)
        {
            if (nMin > ppnError[i][j])
            {
                nMin = ppnError[i][j];
                *pnXMotion = (j - MAX_OFFSET);
                *pnYMotion = (i - MAX_OFFSET);
            }
        }
    }

    // for (i = 0; i < MAX_OFFSET2; i++)
    //     free(ppnError[i]);
    // free(ppnError);

    // free(pbTmpImage1);
    // free(pbTmpImage2);

    return 0;
}

void ShrinkImage(unsigned char* pbInputImg, int nImgWid, int nImgHei, unsigned char* pbOutImg, int nOutWid, int nOutHei, SRect* pxRegion)
{
    short nHeight, nWidth, nX, nY;
    if (pxRegion)
    {
        nHeight = pxRegion->height;
        nWidth = pxRegion->width;
        nX = pxRegion->x;
        nY = pxRegion->y;
    }
    else
    {
        nHeight = nImgHei;
        nWidth = nImgWid;
        nX = 0;
        nY = 0;
    }

    if (nWidth > nOutWid || nHeight > nOutHei)
    {
        int i, j, nSum;

        short nShrink;
        short nRest;
        short nR, nNR, nSkip, nShift, nC;

        unsigned char bNext;

        short nTemp1, nTemp2, nTemp3, nTemp4;
        unsigned char* pbTemp, *pbBuffer1, *pbBuffer2;

        nShrink = nWidth / nOutWid;

        nRest = nWidth - ( nOutWid * nShrink );

        for ( i = 0 ; i < nOutHei ; i ++ )
        {
            nR = ((short)i) * nHeight / nOutHei;
            nNR = ((short)( i + 1 )) * nHeight / nOutHei;

            bNext = ( ( nNR - nR ) > 1 ) ? 1 : 0;

            nSkip = nShift = 0;

            pbTemp = (unsigned char*)(pbOutImg + nOutWid * i);

            nTemp1 = nY + nR;

            nTemp3 = nTemp1 + bNext;

            pbBuffer1 = pbInputImg + nImgWid * nTemp1;
            pbBuffer2 = pbInputImg + nImgWid * nTemp3;

            for ( j = 0 ; j < nOutWid ; j ++ , pbTemp ++ )
            {
                nSkip += nRest;
                nC = ((short)j) * nShrink + nShift;

                nTemp2 = nX + nC;
                nTemp4 = nTemp2 + 1;

                nSum = *((unsigned char*)(pbBuffer1 + nTemp2)) + *((unsigned char*)(pbBuffer2 + nTemp2));
                if ( nSkip >= nOutWid )
                {
                    nShift ++;
                    nSkip -= nOutWid;
                    nSum += *((unsigned char*)(pbBuffer1 + nTemp4)) + *((unsigned char*)(pbBuffer2 + nTemp4));
                    nSum >>= 2;
                }
                else
                    nSum >>= 1;
                (*pbTemp) = nSum;
            }
        }
    }
    else
    {
        int i, j;
        int nRestWid = ((nOutWid - nWidth) >> 1);
        int nRestHei = ((nOutHei - nHeight) >> 1);
        unsigned char* pbInIdx, *pbOutIdx, *pbTemp;

        pbOutIdx = pbOutImg;
        for (i = 0; i < nOutHei; i ++)
        {
            for (j = 0; j < nOutWid; j ++)
            {
                *pbOutIdx ++ = 0x20;
            }
        }

        pbOutIdx = pbOutImg + nRestHei * nOutWid + nRestWid;
        pbInputImg += (nX + nY * nImgWid);
        for (i = 0; i < nHeight; i ++)
        {
            pbTemp = pbOutIdx;
            pbInIdx = pbInputImg + i * nImgWid;
            for (j = 0; j < nWidth; j ++)
            {
                *pbTemp ++ = *pbInIdx ++;
            }

            pbOutIdx += nOutWid;
        }
    }
}

void ShrinkImage24(unsigned char* pbInputImg, int nImgWid, int nImgHei, unsigned char* pbOutImg, int nOutWid, int nOutHei, SRect* pxRegion)
{
    short nHeight, nWidth, nX, nY;
    if (pxRegion)
    {
        nHeight = pxRegion->height;
        nWidth = pxRegion->width;
        nX = pxRegion->x;
        nY = pxRegion->y;
    }
    else
    {
        nHeight = nImgHei;
        nWidth = nImgWid;
        nX = 0;
        nY = 0;
    }

    if (nWidth > nOutWid || nHeight > nOutHei)
    {
        int i, j, nSum;

        short nShrink;
        short nRest;
        short nR, nNR, nSkip, nShift, nC;

        unsigned char bNext;

        short nTemp1, nTemp2, nTemp3, nTemp4;
        unsigned char* pbTemp, *pbBuffer1, *pbBuffer2;
        nShrink = nWidth / nOutWid;

        nRest = nWidth - ( nOutWid * nShrink );

        for ( i = 0 ; i < nOutHei ; i ++ )
        {
            nR = ((short)i) * nHeight / nOutHei;
            nNR = ((short)( i + 1 )) * nHeight / nOutHei;

            bNext = ( ( nNR - nR ) > 1 ) ? 1 : 0;

            nSkip = nShift = 0;

            pbTemp = (unsigned char*)(pbOutImg + nOutWid * i * 3);

            nTemp1 = nY + nR;

            nTemp3 = nTemp1 + bNext;

            pbBuffer1 = pbInputImg + nImgWid * nTemp1 * 3;
            pbBuffer2 = pbInputImg + nImgWid * nTemp3 * 3;

            for ( j = 0 ; j < nOutWid ; j ++ , pbTemp += 3 )
            {
                nSkip += nRest;
                nC = ((short)j) * nShrink + nShift;

                nTemp2 = (nX + nC);
                nTemp4 = (nTemp2 + 1);

                nSum = *((unsigned char*)(pbBuffer1 + nTemp2 * 3)) + *((unsigned char*)(pbBuffer2 + nTemp2 * 3));
                if ( nSkip >= nOutWid )
                {
                    nShift ++;
                    nSkip -= nOutWid;
                    nSum += *((unsigned char*)(pbBuffer1 + nTemp4 * 3)) + *((unsigned char*)(pbBuffer2 + nTemp4 * 3));
                    nSum >>= 2;
                }
                else
                    nSum >>= 1;
                *(pbTemp + 0) = nSum;

                nSum = *((unsigned char*)(pbBuffer1 + nTemp2 * 3 + 1)) + *((unsigned char*)(pbBuffer2 + nTemp2 * 3 + 1));
                if ( nSkip >= nOutWid )
                {
                    nShift ++;
                    nSkip -= nOutWid;
                    nSum += *((unsigned char*)(pbBuffer1 + nTemp4 * 3 + 1)) + *((unsigned char*)(pbBuffer2 + nTemp4 * 3 + 1));
                    nSum >>= 2;
                }
                else
                    nSum >>= 1;
                *(pbTemp + 1) = nSum;

                nSum = *((unsigned char*)(pbBuffer1 + nTemp2 * 3 + 2)) + *((unsigned char*)(pbBuffer2 + nTemp2 * 3 + 2));
                if ( nSkip >= nOutWid )
                {
                    nShift ++;
                    nSkip -= nOutWid;
                    nSum += *((unsigned char*)(pbBuffer1 + nTemp4 * 3 + 2)) + *((unsigned char*)(pbBuffer2 + nTemp4 * 3 + 2));
                    nSum >>= 2;
                }
                else
                    nSum >>= 1;
                *(pbTemp + 2) = nSum;
            }
        }
    }
    else
    {
        int i, j;
        int nRestWid = ((nOutWid - nWidth) >> 1);
        int nRestHei = ((nOutHei - nHeight) >> 1);
        unsigned char* pbInIdx, *pbOutIdx, *pbTemp;

        pbOutIdx = pbOutImg;
        for (i = 0; i < nOutHei; i ++)
        {
            for (j = 0; j < nOutWid; j ++)
            {
                *pbOutIdx ++ = 0x20;
                *pbOutIdx ++ = 0x20;
                *pbOutIdx ++ = 0x20;
            }
        }

        pbOutIdx = pbOutImg + (nRestHei * nOutWid + nRestWid) * 3;
        pbInputImg += (nX + nY * nImgWid) * 3;
        for (i = 0; i < nHeight; i ++)
        {
            pbTemp = pbOutIdx;
            pbInIdx = pbInputImg + i * nImgWid * 3;
            for (j = 0; j < nWidth; j ++)
            {
                *pbTemp ++ = *pbInIdx ++;
                *pbTemp ++ = *pbInIdx ++;
                *pbTemp ++ = *pbInIdx ++;
            }

            pbOutIdx += nOutWid * 3;
        }
    }
}

int FaceImage2JpgImage(unsigned char* pbFaceImage, unsigned char* pbDstJpgData, int* pnJpgLen)
{
    // int nWriteLen = 0;
//    for(int i = 80; i >= 0; i -= 10)
//    {
//        jpge::params params;
//        params.m_quality = i;
//        params.m_subsampling = jpge::H2V2;

//        nWriteLen = sizeof(g_jpgTmpData);
//        if (!jpge::compress_image_to_jpeg_file_in_memory(g_jpgTmpData, nWriteLen, N_FACE_WID, N_FACE_HEI, 3, pbFaceImage, params))
//        {
//            if(pnJpgLen)
//                pnJpgLen = 0;

//            return -1;
//        }

//        if(nWriteLen <= N_MAX_JPG_FACE_IMAGE_SIZE - 8)
//        {
//            memcpy(pbDstJpgData, g_jpgTmpData, N_MAX_JPG_FACE_IMAGE_SIZE);
//            break;
//        }
//    }

//    if(pnJpgLen)
//        *pnJpgLen = nWriteLen;

    return 0;
}


int GetEvalLight24(unsigned char* pbClrData32, int width, int height)
{
    int nAverage = 0;
    for(int y = 0; y < height; y ++)
    {
        for(int x = 0; x < width; x ++)
        {
            int r = pbClrData32[(y * width + x) * 3 + 0];
            int g = pbClrData32[(y * width + x) * 3 + 1];
            int b = pbClrData32[(y * width + x) * 3 + 2];

            nAverage += (r * 0.299 + g * 0.587 + b * 0.114);
        }
    }


    nAverage /= (width * height);
    return nAverage;
}

int GetEvalLight8(unsigned char* pbGreyData, int width, int height)
{
    int nAverage = 0;
    for(int y = 0; y < height; y ++)
    {
        for(int x = 0; x < width; x ++)
        {
            nAverage += pbGreyData[y * width + x];
        }
    }


    nAverage /= (width * height);
    return nAverage;
}

int GetEvalLightFromRegion(unsigned char *pbYData, int imgWidth, int imgHeight, int left, int top, int width, int height)
{
    if(width == 0 || height == 0)
    {
        width = imgWidth;
        height = imgHeight;
        left = 0;
        top = 0;
    }

    int nAverage = 0;
    for(int y = top; y < top + height; y ++)
    {
        for(int x = left; x < left + width; x ++)
            nAverage += pbYData[y * imgWidth + x];
    }

    if(width * height == 0)
        return 0;
    else
        return nAverage / (width * height);
}

void EqualizeAlignedImage(unsigned char* pImage, int nW, int nH)
{
    int nMean, nSigmaX2;
    int nArea = nW * nH;

    nMean = 0;
    nSigmaX2 = 0;

    int nIndex;
    for (nIndex = 0; nIndex < nArea; nIndex ++)
    {
        nMean += pImage[nIndex];
    }

    nMean = nMean / nArea;

    nSigmaX2 = 0;
    for (nIndex = 0; nIndex < nArea; nIndex++)
    {
        nSigmaX2 += (pImage[nIndex] - nMean) * (pImage[nIndex] - nMean);
    }

    nSigmaX2 = nSigmaX2 / nArea;

    float rSigma = sqrt(nSigmaX2);

    for (nIndex = 0; nIndex < nArea; nIndex++)
    {
        int nDst = 48 / rSigma* (pImage[nIndex] - nMean) + 64;
        if (nDst < 0)
        {
            nDst = 0;
        }
        if (nDst > 255)
        {
            nDst = 255;
        }
        pImage[nIndex] = nDst;
    }
}


#define BACK_SATURATION_THRESHOLD	0.01//Min threshold of saturation of bad color
#define FORE_FACE_AVERAGE_THRESHOLD	30//Max threshold of average value of bad face.

//unsigned char* pBuffer: rgb image buffer
//int nImageWidth: image width
//int nImageHeight: image height
//int nFaceX: x coordination of topleft point of face in image
//int nFaceY: y coordination of topleft point of face in image
//int nFaceWidth: width of face in image
//int nFaceHeight: height of face in image

//return value
//if image is invalid, return 1, else(image is valid) return 0

int CheckValidColorImage(unsigned char* pBuffer, int nImageWidth, int nImageHeight,
                         int nFaceX, int nFaceY, int nFaceWidth, int nFaceHeight)
{
    int nX, nY;
    int nBufferIndex = 0;
    int nFaceAverageValue = 0;

    int nSaturatedPixelCount = 0;

    int nArea = nImageHeight * nImageWidth;

    //check and adjust face rects
    int nFaceRight, nFaceBottom;
    int nFaceArea = 0;

    nFaceRight = nFaceX + nFaceWidth - 1;
    nFaceBottom = nFaceY + nFaceHeight - 1;

    float rSaturatedPixelRate = 0;

    if (nFaceX < 0)
    {
        nFaceX = 0;
    }

    if (nFaceY < 0)
    {
        nFaceY = 0;
    }

    if (nFaceRight >= nImageWidth)
    {
        nFaceRight = nImageWidth - 1;
    }

    if (nFaceBottom >= nImageHeight)
    {
        nFaceBottom = nImageHeight - 1;
    }

    nFaceArea = (nFaceBottom - nFaceY + 1) * (nFaceRight - nFaceX + 1);


    for (nY = 0; nY < nImageHeight; nY ++)
    {
        for (nX = 0; nX < nImageWidth; nX ++)
        {
            int nR, nG, nB;

            nR = pBuffer[nBufferIndex];
            nBufferIndex ++;
            nG = pBuffer[nBufferIndex];
            nBufferIndex ++;
            nB = pBuffer[nBufferIndex];
            nBufferIndex ++;

            int nGray = (unsigned)((299 * nR + 587 * nG + 114 * nB) / 1000);

            if (nY >= nFaceY && nY <= nFaceBottom && nX >= nFaceX && nX <= nFaceRight)
            {
                nFaceAverageValue += nGray;
            }
            else
            {
                if (nGray > 235)
                {
                    nSaturatedPixelCount ++;
                }
            }
        }
    }

    nFaceAverageValue /= nFaceArea;
    rSaturatedPixelRate = (float)nSaturatedPixelCount / nArea;

    if (nFaceAverageValue < FORE_FACE_AVERAGE_THRESHOLD || rSaturatedPixelRate > BACK_SATURATION_THRESHOLD)
    {
        return 1;
    }

    return 0;
}



void Shrink_RGB(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width)
{    
    int* E05BFF1C_2 = (int*)malloc((dst_width + dst_height) * 2 * sizeof(int));
    int* E05C83D4_2 = (int*)malloc((dst_width + dst_height) * 2 * sizeof(int));//E05C83D4;
    int nRateXDesToSrc = ((src_width - 1) << 10) / (dst_width - 1);
    int nRateYDesToSrc = ((src_height - 1) << 10) / (dst_height - 1);

    //LOGE("In shrink 0 = %d, %d", dst_height, dst_width);
    int nX, nY;
    if (dst_width > 0)
    {

        int nSrcX_10 = 0;
        for(nX = 0; nX < dst_width; nX ++)
        {
            int nSrcX = nSrcX_10 >> 10;
            int nDiffX = nSrcX_10 - (nSrcX << 10);
            int nSrcX_1 = nSrcX + 1;
            int n1_DiffX = 0x400 - nDiffX;

            if (src_width - 1 <= nSrcX)
            {
                E05C83D4_2[(dst_height + nX) * 2] = 0x400;
                E05BFF1C_2[(dst_height + nX) * 2] = src_width - 1;
                E05BFF1C_2[(dst_height + nX) * 2 + 1] = src_width - 1;
                E05C83D4_2[(dst_height + nX) * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[(dst_height + nX) * 2] = nSrcX;
                E05C83D4_2[(dst_height + nX) * 2] = n1_DiffX;
                E05BFF1C_2[(dst_height + nX) * 2 + 1] = nSrcX_1;
                E05C83D4_2[(dst_height + nX) * 2 + 1] =  nDiffX;
            }
            nSrcX_10 += nRateXDesToSrc;//
        } //while (R5 != dst_width);//BNE		loc_E0315FAC
    }

    //LOGE("In shrink 1");

    if (dst_height > 0)
    {
        int nSrcY_10 = 0;

        for(nY = 0; nY < dst_height; nY ++)
        {
            int nSrcY = nSrcY_10 >> 10;
            int nDiffY = nSrcY_10 - (nSrcY << 10);
            int n1_DiffY = 0x400 - nDiffY;
            int nSrcY_1 = nSrcY + 1;

            if (src_height - 1 <= nSrcY)
            {
                E05BFF1C_2[nY * 2] = src_height - 1;
                E05C83D4_2[nY * 2] = 0x400;
                E05BFF1C_2[nY * 2 + 1] = src_height - 1;
                E05C83D4_2[nY * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[nY * 2] = nSrcY;
                E05C83D4_2[nY * 2] = n1_DiffY;
                E05BFF1C_2[nY * 2 + 1] = nSrcY_1;
                E05C83D4_2[nY * 2 + 1] = nDiffY;
            }
            nSrcY_10 += nRateYDesToSrc;
        }// while (R5 != R7);//BNE		loc_E0316060
        //LOGE("In shrink 2");
        unsigned char* pDst = dst;
        for(nY = 0; nY < dst_height; nY ++)
        {
            //LOGE("In shrink 4");
            int nSrcIndexY1, nSrcIndexY2;
            nSrcIndexY1 = src_width * E05BFF1C_2[nY * 2];
            nSrcIndexY2 = src_width * E05BFF1C_2[nY * 2 + 1];
            int nYAlpha1, nYAlpha2;
            nYAlpha1 = E05C83D4_2[nY * 2];
            nYAlpha2 = E05C83D4_2[nY * 2 + 1];

            if (dst_width > 0)
            {
                for(nX = 0; nX < dst_width; nX ++)
                {
                    int nSrcIndexX1 = E05BFF1C_2[(dst_height + nX) * 2 + 1];
                    int nSrcIndexX2 = E05BFF1C_2[(dst_height + nX) * 2];
                    int nAlpha1, nAlpha2;
                    nAlpha1 = E05C83D4_2[(dst_height + nX) * 2 + 1];
                    nAlpha2 = E05C83D4_2[(dst_height + nX) * 2];

                    int nColorIndex;
                    for(nColorIndex = 0; nColorIndex < 3; nColorIndex ++)
                    {
                        *pDst = (nYAlpha2 * src[(nSrcIndexY2 + nSrcIndexX2) * 3 + nColorIndex] * nAlpha2 + nYAlpha2 * src[(nSrcIndexY2 + nSrcIndexX1) * 3 + nColorIndex] * nAlpha1 + nAlpha2 * nYAlpha1 * src[(nSrcIndexY1 + nSrcIndexX2) * 3 + nColorIndex] + nAlpha1 * nYAlpha1 * src[(nSrcIndexY1 + nSrcIndexX1) * 3 + nColorIndex] + 0x80000) >> 20;//STRB		R1, [R11],#1
                        pDst ++;
                    }

                    //CMP		R11, R10
                } //while (R11 != R10);//BNE		loc_E0316138
            }
        } //while (R12 != R2);//BNE		loc_E03160F4

    }

    //LOGE("In shrink 3");
    free(E05BFF1C_2);
    free(E05C83D4_2);
}

#define     BAYER_TYPE_BGGR     (0)
#define     BAYER_TYPE_RGGB     (1)

#define     BAYER_TYPE  BAYER_TYPE_BGGR
// #define      BAYER_TYPE  BAYER_TYPE_RGGB

#if BAYER_TYPE == BAYER_TYPE_BGGR
# define RGB2Y(r, g, b) (unsigned char)(((76 * (r) + 149 * (g) + 29 * (b) + 128) >> 8))
# define RGB2YCOEFF     (0x1D954C)
#endif
#if BAYER_TYPE == BAYER_TYPE_RGGB
# define RGB2Y(r, g, b) (unsigned char)(((29 * (r) + 149 * (g) + 76 * (b) + 128) >> 8))
# define RGB2YCOEFF     (0x4C951D)
#endif

void convert_bayer2y_rotate_cm(unsigned char* bayer, int width, int height)
{
    unsigned char* pY_start = bayer;

    int width_2 = width - 2;
    int height_2 = height - 2;

    int t1, t2;
    unsigned char tp[2][1920];
    int tidx1, tidx2;

    {
        //      int y = 0;

        int b00, b01, b10, b11;
        int g00, g01, g10, g11;
        int r00, r01, r10, r11;

        unsigned char* pY = pY_start;

        const unsigned char* pixel_0 = bayer;
        const unsigned char* pixel_m = pixel_0 - width;
        const unsigned char* pixel_1 = pixel_0 + width;
        const unsigned char* pixel_2 = pixel_1 + width;

        tidx1 = 0; tidx2 = 1;
        memcpy(tp[tidx1], pixel_1, width);

        {
            //          int x = 0;

            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p12 = pixel_1[2];
            int p20 = pixel_2[0];
            int p21 = pixel_2[1];
            int p22 = pixel_2[2];

            b00 = p00;
            g00 = (p10 + p01) / 2;
            r00 = p11;

            b01 = (p00 + p02) / 2;
            g01 = p01;
            r01 = p11;

            b10 = (p00 + p20) / 2;
            g10 = p10;
            r10 = p11;

            b11 = (p00 + p20 + p02 + p22) / 4;
            g11 = (p21 + p01 + p10 + p12) / 4;
            r11 = p11;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        for (int x = 2; x < width_2; x += 2)
        {
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p1m = t2; // pixel_1[-1];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p12 = pixel_1[2];
            int p20 = pixel_2[0];
            int p21 = pixel_2[1];
            int p22 = pixel_2[2];

            b00 = p00;
            g00 = (p10 + p0m + p01) / 3;
            r00 = (p1m + p11) / 2;

            b01 = (p00 + p02) / 2;
            g01 = p01;
            r01 = p11;

            b10 = (p00 + p20) / 2;
            g10 = p10;
            r10 = (p1m + p11) / 2;

            b11 = (p00 + p20 + p02 + p22) / 4;
            g11 = (p21 + p01 + p10 + p12) / 4;
            r11 = p11;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        {
            //          int x = width_2;

            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p1m = t2; // pixel_1[-1];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p20 = pixel_2[0];
            int p21 = pixel_2[1];

            b00 = p00;
            g00 = (p10 + p0m + p01) / 3;
            r00 = (p1m + p11) / 2;

            b01 = p00;
            g01 = p01;
            r01 = p11;

            b10 = (p00 + p20) / 2;
            g10 = p10;
            r10 = (p1m + p11) / 2;

            b11 = (p00 + p20) / 2;
            g11 = (p21 + p01 + p10) / 3;
            r11 = p11;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }
    }


    //#pragma omp parallel for num_threads(2)
    for (int y = 2; y < height_2; y += 2)
    {
        int b00, b01, b10, b11;
        int g00, g01, g10, g11;
        int r00, r01, r10, r11;

        unsigned char* pY = pY_start + y * width;

        const unsigned char* pixel_0 = bayer + y * width;
        const unsigned char* pixel_m = tp[tidx1]; // pixel_0 - width;
        const unsigned char* pixel_1 = pixel_0 + width;
        const unsigned char* pixel_2 = pixel_1 + width;

        tidx1 = tidx2; tidx2 = 1 - tidx2;
        memcpy(tp[tidx1], pixel_1, width);

        {
            //          int x = 0;

            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p12 = pixel_1[2];
            int p20 = pixel_2[0];
            int p21 = pixel_2[1];
            int p22 = pixel_2[2];

            b00 = p00;
            g00 = (p10 + pm0 + p01) / 3;
            r00 = (pm1 + p11) / 2;

            b01 = (p00 + p02) / 2;
            g01 = p01;
            r01 = (pm1 + p11) / 2;

            b10 = (p00 + p20) / 2;
            g10 = p10;
            r10 = p11;

            b11 = (p00 + p20 + p02 + p22) / 4;
            g11 = (p21 + p01 + p10 + p12) / 4;
            r11 = p11;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        int count = (width_2 - 2) / 2;
        int remain = count;
        for (; remain > 0; remain--)
        {
            int pmm = pixel_m[-1];
            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p1m = t2; // pixel_1[-1];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p12 = pixel_1[2];
            int p20 = pixel_2[0];
            int p21 = pixel_2[1];
            int p22 = pixel_2[2];

            int q3, q4, q5, q6, q7, q8, q9, q10, q11, q12, q13, q14, q15;

            q6 = pmm;                           // q6(pmm)
            q5 = pm0;                           // q5(pm0), q6(pmm)
            q9 = pm1;                           // q5(pm0), q6(pmm), q9(pm1)

            q3 = p0m;                           // q3(p0m), q5(pm0), q6(pmm), q9(pm1)
            q4 = p00;                           // q3(p0m), q5(pm0), q6(pmm), q9(pm1)                                                                                           ////            q4(p00)
            q8 = p01;                           // q3(p0m), q5(pm0), q6(pmm), q9(pm1)                                                                                           ////            q8(p01)
            q7 = p02;                           // q3(p0m), q5(pm0), q6(pmm), q7(p02), q9(pm1)

            q5 = q5 + q3;                       // q5(pm0+p0m), q6(pmm), q7(p02), q9(pm1)                   // del(q3)
            q7 = q7 + q4;                       // q5(pm0+p0m), q6(pmm), q9(pm1)                                                                                                ////            q7(p00+p02)

            q12 = p1m;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)
            q11 = p10;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)                                                                                      ////            q11(p10)
            q15 = p11;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m)                                                                                      ////            q15(p11)
            q14 = p12;                          // q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)

            q3 = q11 + q8;                      // q3(p10+p01), q5(pm0+p0m), q6(pmm), q9(pm1), q12(p1m), q14(p12)
            q6 = q6 + q12;                      // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q9(pm1), q12(p1m), q14(p12)
            q9 = q9 + q15;                      // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q12(p1m), q14(p12)                                                                    ////            q9(pm1+p11)
            q12 = q12 + q15;                    // q3(p10+p01), q5(pm0+p0m), q6(pmm+p1m), q14(p12)                                                                              ////            q12(p1m+p11)
            q5 = q5 + q3;                       // q3(p10+p01), q6(pmm+p1m), q14(p12)                                                                                           ////            q5(p10+p01+pm0+p0m)
            q6 = q6 + q9;                       // q3(p10+p01), q14(p12)                                                                                                        ////            q6(pmm+p1m+pm1+p11)
            q14 = q14 + q3;                     // q14(p10+p01+p12)                                         // del(q3)

            q10 = p20;                          // q10(p20), q14(p10+p01+p12)
            q3 = p21;                           // q3(p21), q10(p20), q14(p10+p01+p12)
            q13 = p22;                          // q3(p21), q10(p20), q13(p22), q14(p10+p01+p12)

            q14 = q14 + q3;                     // q10(p20), q13(p22)                                       // del(q3)                                                          ////            q14(p10+p01+p12+p21)
            q13 = q13 + q10;                    // q10(p20), q13(p22+p20)
            q10 = q10 + q4;                     // q13(p22+p20)                                                                                                                 ////            q10(p20+p00)
            q13 = q13 + q7;                     //                                                                                                                              ////            q13(p22+p20+p00+p02)

            b00 = q4;
            g00 = q5 >> 2;
            r00 = q6 >> 2;

            b01 = q7 >> 1;
            g01 = q8;
            r01 = q9 >> 1;

            b10 = q10 >> 1;
            g10 = q11;
            r10 = q12 >> 1;

            b11 = q13 >> 2;
            g11 = q14 >> 2;
            r11 = q15;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        {
            //          int x = width_2;

            int pmm = pixel_m[-1];
            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p1m = t2; // pixel_1[-1];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p20 = pixel_2[0];
            int p21 = pixel_2[1];

            b00 = p00;
            g00 = (p10 + pm0 + p0m + p01) / 4;
            r00 = (pmm + p1m + pm1 + p11) / 4;

            b01 = p00;
            g01 = p01;
            r01 = (pm1 + p11) / 2;

            b10 = (p00 + p20) / 2;
            g10 = p10;
            r10 = (p1m + p11) / 2;

            b11 = (p00 + p20) / 2;
            g11 = (p21 + p01 + p10) / 3;
            r11 = p11;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }
    }

    {
        //      int y = height_2;

        int b00, b01, b10, b11;
        int g00, g01, g10, g11;
        int r00, r01, r10, r11;

        unsigned char* pY = pY_start + height_2 * width;

        const unsigned char* pixel_0 = bayer + height_2 * width;
        const unsigned char* pixel_m = tp[tidx1]; // pixel_0 - width;
        const unsigned char* pixel_1 = pixel_0 + width;
        const unsigned char* pixel_2 = pixel_1 + width;

        {
            //          int x = 0;

            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p12 = pixel_1[2];

            b00 = p00;
            g00 = (p10 + pm0 + p01) / 3;
            r00 = (pm1 + p11) / 2;

            b01 = (p00 + p02) / 2;
            g01 = p01;
            r01 = (pm1 + p11) / 2;

            b10 = p00;
            g10 = p10;
            r10 = p11;

            b11 = (p00 + p02) / 2;
            g11 = (p01 + p10 + p12) / 3;
            r11 = p11;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        for (int x = 2; x < width_2; x += 2)
        {
            int pmm = pixel_m[-1];
            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p02 = pixel_0[2];
            int p1m = t2; // pixel_1[-1];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];
            int p12 = pixel_1[2];

            b00 = p00;
            g00 = (p10 + pm0 + p0m + p01) / 4;
            r00 = (pmm + p1m + pm1 + p11) / 4;

            b01 = (p00 + p02) / 2;
            g01 = p01;
            r01 = (pm1 + p11) / 2;

            b10 = p00;
            g10 = p10;
            r10 = (p1m + p11) / 2;

            b11 = (p00 + p02) / 2;
            g11 = (p01 + p10 + p12) / 3;
            r11 = p11;

            t1 = p01;
            t2 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;

            pixel_m += 2;
            pixel_0 += 2;
            pixel_1 += 2;
            pixel_2 += 2;
        }

        {
            //          int x = width_2;

            int pmm = pixel_m[-1];
            int pm0 = pixel_m[0];
            int pm1 = pixel_m[1];
            int p0m = t1; // pixel_0[-1];
            int p00 = pixel_0[0];
            int p01 = pixel_0[1];
            int p1m = t2; // pixel_1[-1];
            int p10 = pixel_1[0];
            int p11 = pixel_1[1];

            b00 = p00;
            g00 = (p10 + pm0 + p0m + p01) / 4;
            r00 = (pmm + p1m + pm1 + p11) / 4;

            b01 = p00;
            g01 = p01;
            r01 = (pm1 + p11) / 2;

            b10 = p00;
            g10 = p10;
            r10 = (p1m + p11) / 2;

            b11 = p00;
            g11 = (p01 + p10) / 2;
            r11 = p11;

            pY[0] = RGB2Y(r00, g00, b00);   // pY[0]
            pY[1] = RGB2Y(r01, g01, b01);   // pY_[0]
            pY[width] = RGB2Y(r10, g10, b10);   // pY[1]
            pY[width + 1] = RGB2Y(r11, g11, b11);   // pY_[1]
            pY += 2;
        }
    }
}

int E05BFF1C_2[(1920 + 1080) * 2];
int E05C83D4_2[(1920 + 1080) * 2];
void Shrink_Grey(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width)
{
    int nRateXDesToSrc = ((src_width - 1) << 10) / (dst_width - 1);
    int nRateYDesToSrc = ((src_height - 1) << 10) / (dst_height - 1);

    //LOGE("In shrink 0 = %d, %d", dst_height, dst_width);
    int nX, nY;
    if (dst_width > 0)
    {

        int nSrcX_10 = 0;
        for (nX = 0; nX < dst_width; nX++)
        {
            int nSrcX = nSrcX_10 >> 10;
            // int nDiffX = nSrcX_10 - (nSrcX << 10);
            // int nSrcX_1 = nSrcX + 1;
            // int n1_DiffX = 0x400 - nDiffX;

            if (src_width - 1 <= nSrcX)
            {
                E05BFF1C_2[(dst_height + nX) * 2] = src_width - 1;
                //E05C83D4_2[(dst_height + nX) * 2] = 0x400;
                //E05BFF1C_2[(dst_height + nX) * 2 + 1] = src_width - 1;
                //E05C83D4_2[(dst_height + nX) * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[(dst_height + nX) * 2] = nSrcX;
                //E05C83D4_2[(dst_height + nX) * 2] = n1_DiffX;
                //E05BFF1C_2[(dst_height + nX) * 2 + 1] = nSrcX_1;
                //E05C83D4_2[(dst_height + nX) * 2 + 1] = nDiffX;
            }
            nSrcX_10 += nRateXDesToSrc;//
        } //while (R5 != dst_width);//BNE       loc_E0315FAC
    }

    //LOGE("In shrink 1");

    if (dst_height > 0)
    {
        int nSrcY_10 = 0;

        for (nY = 0; nY < dst_height; nY++)
        {
            int nSrcY = nSrcY_10 >> 10;
            // int nDiffY = nSrcY_10 - (nSrcY << 10);
            // int n1_DiffY = 0x400 - nDiffY;
            // int nSrcY_1 = nSrcY + 1;

            if (src_height - 1 <= nSrcY)
            {
                E05BFF1C_2[nY * 2] = src_height - 1;
                //E05C83D4_2[nY * 2] = 0x400;
                //E05BFF1C_2[nY * 2 + 1] = src_height - 1;
                //E05C83D4_2[nY * 2 + 1] = 0;
            }
            else
            {
                E05BFF1C_2[nY * 2] = nSrcY;
                //E05C83D4_2[nY * 2] = n1_DiffY;
                //E05BFF1C_2[nY * 2 + 1] = nSrcY_1;
                //E05C83D4_2[nY * 2 + 1] = nDiffY;
            }
            nSrcY_10 += nRateYDesToSrc;
        }// while (R5 != R7);//BNE      loc_E0316060
        //LOGE("In shrink 2");
        unsigned char* pDst = dst;
        for (nY = 0; nY < dst_height; nY++)
        {
            //LOGE("In shrink 4");

            int nSrcY = E05BFF1C_2[nY * 2];
            int nSrcm1Y = nSrcY - 1;
            int nSrcp1Y = nSrcY + 1;

            if (nSrcY == 0)
            {
                nSrcm1Y = 0;
            }
            if (nSrcY >= src_height)
            {
                nSrcp1Y = src_height - 1;
            }

            int nSrcYWidth = nSrcY * src_width;
            int nSrcm1YWidth = nSrcm1Y * src_width;
            int nSrcp1YWidth = nSrcp1Y * src_width;

            if (dst_width > 0)
            {
                for (nX = 0; nX < dst_width; nX++)
                {
                    int nSrcX = E05BFF1C_2[(dst_height + nX) * 2];
                    int nSrcm1X = nSrcX - 1;
                    int nSrcp1X = nSrcX + 1;

                    if (nSrcX == 0)
                    {
                        nSrcm1X = 0;
                    }
                    if (nSrcX >= src_width)
                    {
                        nSrcp1X = src_width - 1;
                    }

                    *pDst = (unsigned char)(((int)src[nSrcYWidth + nSrcX] + src[nSrcYWidth + nSrcm1X] + src[nSrcYWidth + nSrcp1X] + src[nSrcm1YWidth + nSrcX] + src[nSrcp1YWidth + nSrcX]) / 5);//STRB      R1, [R11],#1
                    pDst++;
                    //CMP       R11, R10
                } //while (R11 != R10);//BNE        loc_E0316138
            }
        } //while (R12 != R2);//BNE     loc_E03160F4

    }
}
