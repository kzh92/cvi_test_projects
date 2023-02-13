#include "HAlign.h"
#include "convert.h"
#include <math.h>
#include <memory.h>
#include "gammacorrection.h"


float rConstPoints[5][3] =
{
    { -25.0167198, -24.2606792, -4.95942402 },
    { 24.9832897, -24.2606792, -4.95942402 },
    { -19.3741798, 30.3567104, -5.03859901 },
    { 19.3408508, 30.3567104, -5.03862381 },
    { -0.0166723691, 12.5998402, -18.8323593 },
};

typedef int BOOL;

#ifndef BYTE
typedef unsigned char BYTE;
#endif

float fabs1(float r);

typedef struct _tagARM_Point3D_ {
    float rX;
    float rY;
    float rZ;
} ARM_Point3D_;

typedef struct _tagARM_Point2D_ {
    float rX;
    float rY;
} ARM_Point2D_;

typedef struct _tagARM_RotateMat2D_ {
    float prValue[4];
} ARM_RotateMat2D_;

typedef struct _tagARM_RotateMat3D_ {
    float prValue[9];
} ARM_RotateMat3D_;

typedef struct _tagARM_LinearTransform2D_ {
    ARM_RotateMat2D_ xRotateMat;
    ARM_Point2D_ xTranPoint;
} ARM_LinearTransform2D_;

typedef struct _tagARM_SpatialGraph_ {
    int nNodeNum;
    ARM_Point3D_ pxNodes[360];
} ARM_SpatialGraph_;

void Set_ByARM_Point3D_(ARM_Point3D_* pxPoint, float rx, float ry, float rz);
int GetTransform2D_ByARM_SpatialGraph_(ARM_SpatialGraph_* pxSpatialGraph, ARM_SpatialGraph_ *pxInFaceGraph, ARM_LinearTransform2D_* pxTransform2D, int nAltType);
void Transform_ByARM_LinearTransform2D_(ARM_LinearTransform2D_* pxLinearTransform2D, ARM_Point2D_* pxPoint);
void ReverseMat_ByARM_RoateMat2D_(ARM_RotateMat2D_* pxRotateMat2D);
ARM_Point2D_ Operator_Point2D_Mul_ByARM_RoateMat2D_(ARM_RotateMat2D_* pxRotateMat2D, ARM_Point2D_* pxPoint);

ARM_SpatialGraph_ cFaceGraphDest, cFaceGraphSource;
ARM_LinearTransform2D_ cTransform2D;
static inline int clip_(int x, int a, int b)
{
    return x >= a ? (x < b ? x : b - 1) : a;
}
#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif  //	MIN

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif  //	MAX

void halign(unsigned char* pSrcBuf, int nWidth, int nHeight, int nChannel, float rLeftX, float rLeftY, float rRightX, float rRightY, 
			unsigned char* pDstBuf, int nAlignWidth, int nAlignHeight, float rDistEM, int nCenterX, int nCenterY)
{
	float rDistEdst = rDistEM * 0.91545935033106463624863542034555f;
	float rEyeX = rRightX - rLeftX;
	float rEyeY = rRightY - rLeftY;
	float rDistEsrc = hypot(rEyeX, rEyeY);
	float rScale = rDistEdst / rDistEsrc;
	float rScaleCos = rEyeX / rDistEsrc / rScale;
	float rScaleSin = rEyeY / rDistEsrc / rScale;
	float cx = (rRightX + rLeftX) / 2;
	float cy = (rRightY + rLeftY) / 2;

	float dcx = nCenterX * rScaleCos - nCenterY * rScaleSin;
	float dcy = nCenterX * rScaleSin + nCenterY * rScaleCos;

	float rTransX = cx - dcx;
	float rTransY = cy - dcy;

	float rSourceX, rSourceY;

	memset(pDstBuf, 0, nAlignWidth * nAlignHeight * nChannel);
	unsigned char* pProcessDestBuffer = pDstBuf;

	int nX, nY;
	for (nY = 0; nY < nAlignHeight; nY++) 
	{
		for (nX = 0; nX < nAlignWidth; nX++)
		{
			rSourceX = nX * rScaleCos - nY * rScaleSin + rTransX;
			rSourceY = nX * rScaleSin + nY * rScaleCos + rTransY;

			if (rSourceX < 0 || rSourceX > nWidth - 1 || rSourceY < 0 || rSourceY > nHeight - 1) {
				pProcessDestBuffer += nChannel;
				continue;
			} else {
				int nFllorR, nFloorC, nFllorR1, nFloorC1;
				nFllorR = (int)floor(rSourceY);
				nFloorC = (int)floor(rSourceX);

				if (nFllorR == rSourceY && nFloorC == rSourceX) {
					memcpy(pProcessDestBuffer, pSrcBuf + (nFllorR * nWidth + nFloorC) * nChannel, nChannel);
				} else {

					float rXRate, rYRate;
					rYRate = rSourceY - nFllorR;
					rXRate = rSourceX - nFloorC;

					nFloorC1 = nFloorC + 1;
					nFllorR1 = nFllorR + 1;

					if (nFloorC == nWidth - 1)
					{
						nFloorC1 = nFloorC;
						rXRate = 0;
					}

					if (nFllorR == nHeight - 1)
					{
						nFllorR1 = nFllorR;
						rYRate = 0;
					}

					int nChanelIndex;
					unsigned char *pSourceProcessBuffer1, *pSourceProcessBuffer2, *pSourceProcessBuffer3, *pSourceProcessBuffer4;
					pSourceProcessBuffer1 = pSrcBuf + (nFllorR1 * nWidth + nFloorC1) * nChannel;
					pSourceProcessBuffer2 = pSrcBuf + (nFllorR * nWidth + nFloorC1) * nChannel;
					pSourceProcessBuffer3 = pSrcBuf + (nFllorR * nWidth + nFloorC) * nChannel;
					pSourceProcessBuffer4 = pSrcBuf + (nFllorR1 * nWidth + nFloorC) * nChannel;
					for (nChanelIndex = 0; nChanelIndex < nChannel; nChanelIndex++) {
						*pProcessDestBuffer = (char)((float)*pSourceProcessBuffer1 * rXRate * rYRate +
							(float)*pSourceProcessBuffer2 * rXRate * (1 - rYRate) +
							(float)*pSourceProcessBuffer3 * (1 - rXRate) * (1 - rYRate) +
							(float)*pSourceProcessBuffer4 * (1 - rXRate) * rYRate);

						pProcessDestBuffer++;
						pSourceProcessBuffer1++;
						pSourceProcessBuffer2++;
						pSourceProcessBuffer3++;
						pSourceProcessBuffer4++;
					}
				}
			}
		}
	}
}




int Align_Vertical(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nChannelCount, void* pxModelFaceGraph,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY)
{
    ARM_SpatialGraph_* xModelFaceGraph =(ARM_SpatialGraph_*)pxModelFaceGraph;
    float rEyeCenterX, rEyeCenterY, rMouthCenterY;
    rMouthCenterY = (rConstPoints[2][1] + rConstPoints[3][1]) / 2;
    rEyeCenterX = (rConstPoints[0][0] + rConstPoints[1][0]) / 2;
    rEyeCenterY = (rConstPoints[0][1] + rConstPoints[1][1]) / 2;

    float rScale = (float)nDstHeight / 229.39;
    if (rDistanceEye_Mouth != 0.0f)
    {
        if (rMouthCenterY - rEyeCenterY)
        {
            rScale = rDistanceEye_Mouth / (rMouthCenterY - rEyeCenterY);
        }
        else
        {
            rScale = rDistanceEye_Mouth;
        }
    }

    ARM_Point3D_ offset;
    ARM_Point3D_ eyeOffset;

    Set_ByARM_Point3D_(&eyeOffset, -rEyeCenterX, -rEyeCenterY, 0);

    if (rFaceCenterX == -1.0f || rFaceCenterY == -1.0f)
    {
        Set_ByARM_Point3D_(&offset, nDstWidth * 0.5f, nDstHeight * 0.5f, 0);
    }
    else
    {
        Set_ByARM_Point3D_(&offset, rFaceCenterX, rFaceCenterY, 0);
    }

    ARM_SpatialGraph_ cFaceGraphDest, cFaceGraphSource;
    cFaceGraphDest.nNodeNum = 5;
    cFaceGraphSource.nNodeNum = 5;

    int nSelectedPointIndex[2][5] =
    {
        { 0, 1, 10, 11, 15 },
        { 1, 0, 11, 10, 15 },
    };

    int i;
    for (i = 0; i < 5; i++)
    {
        float rDestPointX, rDestPointY, rDestPointZ;
        rDestPointX = rConstPoints[i][0];
        rDestPointY = rConstPoints[i][1];
        rDestPointZ = rConstPoints[i][2];

        rDestPointX += eyeOffset.rX;
        rDestPointY += eyeOffset.rY;
        rDestPointZ += eyeOffset.rZ;

        rDestPointX *= rScale;
        rDestPointY *= rScale;
        rDestPointZ *= rScale;

        rDestPointX += offset.rX;
        rDestPointY += offset.rY;
        rDestPointZ += offset.rZ;

        cFaceGraphDest.pxNodes[i].rX = rDestPointX;
        cFaceGraphDest.pxNodes[i].rY = rDestPointY;
        cFaceGraphDest.pxNodes[i].rZ = rDestPointZ;

        if (xModelFaceGraph->pxNodes[0].rX < xModelFaceGraph->pxNodes[1].rX)
        {
            cFaceGraphSource.pxNodes[i].rX = xModelFaceGraph->pxNodes[nSelectedPointIndex[0][i]].rX;
            cFaceGraphSource.pxNodes[i].rY = xModelFaceGraph->pxNodes[nSelectedPointIndex[0][i]].rY;
            cFaceGraphSource.pxNodes[i].rZ = xModelFaceGraph->pxNodes[nSelectedPointIndex[0][i]].rZ;
        }
        else
        {
            cFaceGraphSource.pxNodes[i].rX = xModelFaceGraph->pxNodes[nSelectedPointIndex[1][i]].rX;
            cFaceGraphSource.pxNodes[i].rY = xModelFaceGraph->pxNodes[nSelectedPointIndex[1][i]].rY;
            cFaceGraphSource.pxNodes[i].rZ = xModelFaceGraph->pxNodes[nSelectedPointIndex[1][i]].rZ;
        }
    }

    ARM_LinearTransform2D_ cTransform2D;

    GetTransform2D_ByARM_SpatialGraph_(&cFaceGraphDest, &cFaceGraphSource, &cTransform2D, 7);

    //cFaceGraphDest.GetTransform2D(&cFaceGraphSource, &cTransform2D, 7);
    ARM_Point3D_ pTempEyePoint[2];
    pTempEyePoint[0] = cFaceGraphSource.pxNodes[0];
    pTempEyePoint[1] = cFaceGraphSource.pxNodes[1];

    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[0]);
    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[1]);

    float rTempCenterX, rTempCenterY;
    rTempCenterX = (pTempEyePoint[0].rX + pTempEyePoint[1].rX) / 2;
    rTempCenterY = (pTempEyePoint[0].rY + pTempEyePoint[1].rY) / 2;

    cTransform2D.xTranPoint.rX += (rFaceCenterX - rTempCenterX);
    cTransform2D.xTranPoint.rY += (rFaceCenterY - rTempCenterY);

    //GetReverseTransform(&cTransform2D);
    ReverseMat_ByARM_RoateMat2D_(&cTransform2D.xRotateMat);
    ARM_Point2D_ pxPoint = Operator_Point2D_Mul_ByARM_RoateMat2D_(&cTransform2D.xRotateMat, &cTransform2D.xTranPoint);
    cTransform2D.xTranPoint.rX = -pxPoint.rX;
    cTransform2D.xTranPoint.rY = -pxPoint.rY;

    memset(pDstBuf, 0, sizeof(BYTE)* nDstWidth * nDstHeight * nChannelCount);

    BYTE* pProcessDestBuffer;
    pProcessDestBuffer = pDstBuf;

    int nX, nY;
    for (nY = 0; nY < nDstHeight; nY++)
    {
        for (nX = 0; nX < nDstWidth; nX++)
        {
            float rSourceX, rSourceY;
            ARM_Point2D_ pointDest, pointSource;

            pointDest.rX = nX;
            pointDest.rY = nY;

            Transform_ByARM_LinearTransform2D_(&cTransform2D, &pointDest);

            pointSource = pointDest;
            rSourceX = pointSource.rX;
            rSourceY = pointSource.rY;

            if (rSourceX < 0 || rSourceX > nSrcWidth - 1 || rSourceY < 0 || rSourceY > nSrcHeight - 1)
            {
                pProcessDestBuffer += nChannelCount;
                continue;
            }
            else
            {
                int nFllorR, nFloorC, nFllorR1, nFloorC1;
                nFllorR = floor(rSourceY);
                nFloorC = floor(rSourceX);

                if (nFllorR == rSourceY && nFloorC == rSourceX)
                {
                    BYTE* pSourceProcessBuffer;
                    pSourceProcessBuffer = pSrcBuf + (nFllorR * nSrcWidth + nFloorC);
                    int nChannelIndex;
                    for (nChannelIndex = 0; nChannelIndex < nChannelCount; nChannelIndex++)
                    {
                        *pProcessDestBuffer = *pSourceProcessBuffer;
                        pProcessDestBuffer++;
                    }
                }
                else
                {
                    float rXRate, rYRate;
                    rYRate = rSourceY - nFllorR;
                    rXRate = rSourceX - nFloorC;

                    nFloorC1 = nFloorC + 1;
                    nFllorR1 = nFllorR + 1;

                    if (nFloorC == nSrcWidth - 1)
                    {
                        nFloorC1 = nFloorC;
                        rXRate = 0;
                    }

                    if (nFllorR == nSrcHeight - 1)
                    {
                        nFllorR1 = nFllorR;
                        rYRate = 0;
                    }

                    int nChanelIndex;
                    BYTE *pSourceProcessBuffer1, *pSourceProcessBuffer2, *pSourceProcessBuffer3, *pSourceProcessBuffer4;
                    pSourceProcessBuffer1 = pSrcBuf + (nFllorR1 * nSrcWidth + nFloorC1) * nChannelCount;
                    pSourceProcessBuffer2 = pSrcBuf + (nFllorR * nSrcWidth + nFloorC1) * nChannelCount;
                    pSourceProcessBuffer3 = pSrcBuf + (nFllorR * nSrcWidth + nFloorC) * nChannelCount;
                    pSourceProcessBuffer4 = pSrcBuf + (nFllorR1 * nSrcWidth + nFloorC) * nChannelCount;
                    for (nChanelIndex = 0; nChanelIndex < nChannelCount; nChanelIndex++)
                    {
                        *pProcessDestBuffer = (char)((float)*pSourceProcessBuffer1 * rXRate * rYRate +
                            (float)*pSourceProcessBuffer2 * rXRate * (1 - rYRate) +
                            (float)*pSourceProcessBuffer3 * (1 - rXRate) * (1 - rYRate) +
                            (float)*pSourceProcessBuffer4 * (1 - rXRate) * rYRate);

                        pProcessDestBuffer++;
                        pSourceProcessBuffer1++;
                        pSourceProcessBuffer2++;
                        pSourceProcessBuffer3++;
                        pSourceProcessBuffer4++;
                    }

                }
            }
        }
    }
    return 0;
}

void alignForModeling(unsigned char* src_buf, int nImageWidth, int nImageHeight, int nCropX, int nCropY, int nCropWidth, int nCropHeight, unsigned char* dst_buf, int nDestWidth, int nDestHeight)
{
    int nCropY1 = nCropY + nCropHeight;

    float scale_x = (float)nCropWidth / nDestWidth, scale_y = (float)nCropHeight / nDestHeight;
    // int iscale_x = (int)round(scale_x);
    // int iscale_y = (int)round(scale_y);

    int xofs[64]; // 64 >= nDestWidth
    int yofs[64]; // 64 >= nDestHeight
    short ialpha[128]; // 128 >= 2 * nDestWidth
    short ibeta[128]; // 128 >= 2 * nDestHeight

    const unsigned char* srows[2];
    int rows[2][64]; // 64 >> nDestWidth
    int prev_sy[2];

    int xmax = nDestWidth, width = nDestWidth; //xmin = 0,
    float fx, fy, ft;
    int sx, sy, dx, dy;
    int k;

    prev_sy[0] = -1;
    prev_sy[1] = -1;

    /// x step
    for (dx = 0; dx < nDestWidth; dx++)
    {
        fx = (float)((dx + 0.5f) * scale_x - 0.5f);
        sx = (int)floor(fx);
        fx -= sx;

        if (sx < 0)
        {
            // xmin = dx + 1;
            fx = 0;
            sx = 0;
        }

        if (sx + 1 >= nCropWidth)
        {
            xmax = MIN(xmax, dx);
            fx = 0;
            sx = nCropWidth - 1;
        }

        ft = fx * 2048.f;
        xofs[dx] = sx + nCropX;
        ialpha[dx * 2 + 0] = (short)round(2048.f - ft);
        ialpha[dx * 2 + 1] = (short)round(ft);
    }

    /// y step
    for (dy = 0; dy < nDestHeight; dy++)
    {
        fy = (float)((dy + 0.5f) * scale_y - 0.5f);
        sy = (int)floor(fy);
        fy -= sy;

        ft = fy * 2048.f;
        yofs[dy] = sy + nCropY;
        ibeta[dy * 2 + 0] = (short)round(2048.f - ft);
        ibeta[dy * 2 + 1] = (short)round(ft);
    }

    const short* beta = (const short*)ibeta;
    int sy0;
    int k0, k1;
    unsigned char* dst = dst_buf;
    for (dy = 0; dy < nDestHeight; dy++, beta += 2, dst += nDestWidth)
    {
        sy0 = yofs[dy];
        k0 = 2;
        k1 = 0;
        for (k = 0; k < 2; k++)
        {
            sy = clip_(sy0 + k, nCropY, nCropY1);
            k1 = (k1 > k) ? k1 : k;
            for (; k1 < 2; k1++)
            {
                if (sy == prev_sy[k1])
                {
                    if (k1 > k)
                        memcpy(rows[k], rows[k1], nDestWidth * sizeof(int));
                    break;
                }
            }
            if (k1 == 2)
                k0 = MIN(k0, k);
            srows[k] = src_buf + sy * nImageWidth;
            prev_sy[k] = sy;
        }

        for (; k0 < 2; k0++)
        {
            const unsigned char *S = srows[k0];
            int *D = rows[k0];
            for (dx = 0; dx < xmax; dx++)
            {
                int sx = xofs[dx];
                D[dx] = S[sx] * ialpha[dx * 2] + S[sx + 1] * ialpha[dx * 2 + 1];
            }

            for (; dx < nDestWidth; dx++)
                D[dx] = S[xofs[dx]] * 2048;
        }

        short b0 = beta[0], b1 = beta[1];
        const int *S0 = rows[0], *S1 = rows[1];

        int x = 0;
        for (; x < width; x++)
            dst[x] = (unsigned char)((((b0 * (S0[x] >> 4)) >> 16) + ((b1 * (S1[x] >> 4)) >> 16) + 2) >> 2);
    }
}


unsigned char getSrcBufferFromRotatedCoordinate(unsigned char* pSrcBuf, int nWidth, int nHeight, int x, int y, int iCamFlip, char* szBAYER_ARRAY)
{
    int nY1, nX1;

    if(iCamFlip == 0)
    {
        nY1 = x;
        nX1 = nHeight - y;
    }
    else
    {
        nY1 = nWidth - x;
        nX1 = y;
    }

    //return pSrcBuf[nY1 * nHeight + nX1];
    return getYFromBAYER((void*)pSrcBuf, nHeight, nWidth, nX1, nY1,	szBAYER_ARRAY);
}


void alignForModeling_bayerConvert(unsigned char* pSrcBuf, int nWidth, int nHeight, int nFaceLeft, int nFaceTop, int nFaceWidth,
    unsigned char* pDstBuf, int nAlignWidth, int nAlignHeight, int iCamFlip, char* szBAYER_ARRAY)
{

    float rScaleDstToSrc =(float)nFaceWidth / nAlignWidth;
    memset(pDstBuf, 0, sizeof(BYTE)* nAlignWidth * nAlignHeight);

    BYTE* pProcessDestBuffer;
    pProcessDestBuffer = pDstBuf;

    int nX, nY;
    for (nY = 0; nY < nAlignHeight; nY++)
    {
        for (nX = 0; nX < nAlignWidth; nX++)
        {
            float rSourceX, rSourceY;

            rSourceX = (float)nX * rScaleDstToSrc + nFaceLeft;
            rSourceY = (float)nY * rScaleDstToSrc + nFaceTop;

            if (rSourceX < 0 || rSourceX > nWidth - 1 || rSourceY < 0 || rSourceY > nHeight - 1)
            {
                pProcessDestBuffer ++;
                continue;
            }
            else
            {
                int nFllorR, nFloorC, nFllorR1, nFloorC1;
                nFllorR = floor(rSourceY);
                nFloorC = floor(rSourceX);

                if (nFllorR == rSourceY && nFloorC == rSourceX)
                {
//                    BYTE* pSourceProcessBuffer;
//                    pSourceProcessBuffer = pSrcBuf + (nFllorR * nWidth + nFloorC);
//                    {
//                        *pProcessDestBuffer = *pSourceProcessBuffer;
//                        pProcessDestBuffer++;
//                    }
                    {
                        *pProcessDestBuffer = getSrcBufferFromRotatedCoordinate(pSrcBuf, nWidth, nHeight, nFloorC, nFllorR, iCamFlip, szBAYER_ARRAY);
                        pProcessDestBuffer++;
                    }

                }
                else
                {
                    float rXRate, rYRate;
                    rYRate = rSourceY - nFllorR;
                    rXRate = rSourceX - nFloorC;

                    nFloorC1 = nFloorC + 1;
                    nFllorR1 = nFllorR + 1;

                    if (nFloorC == nWidth - 1)
                    {
                        nFloorC1 = nFloorC;
                        rXRate = 0;
                    }

                    if (nFllorR == nHeight - 1)
                    {
                        nFllorR1 = nFllorR;
                        rYRate = 0;
                    }

//                    BYTE *pSourceProcessBuffer1, *pSourceProcessBuffer2, *pSourceProcessBuffer3, *pSourceProcessBuffer4;
//                    pSourceProcessBuffer1 = pSrcBuf + (nFllorR1 * nWidth + nFloorC1);
//                    pSourceProcessBuffer2 = pSrcBuf + (nFllorR * nWidth + nFloorC1);
//                    pSourceProcessBuffer3 = pSrcBuf + (nFllorR * nWidth + nFloorC);
//                    pSourceProcessBuffer4 = pSrcBuf + (nFllorR1 * nWidth + nFloorC);
//                    *pProcessDestBuffer = (char)((float)*pSourceProcessBuffer1 * rXRate * rYRate +
//                            (float)*pSourceProcessBuffer2 * rXRate * (1 - rYRate) +
//                            (float)*pSourceProcessBuffer3 * (1 - rXRate) * (1 - rYRate) +
//                            (float)*pSourceProcessBuffer4 * (1 - rXRate) * rYRate);

                    BYTE bSourceProcessBuffer1, bSourceProcessBuffer2, bSourceProcessBuffer3, bSourceProcessBuffer4;
                    bSourceProcessBuffer1 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nWidth, nHeight, nFloorC1, nFllorR1, iCamFlip, szBAYER_ARRAY);
                    bSourceProcessBuffer2 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nWidth, nHeight, nFloorC1, nFllorR, iCamFlip, szBAYER_ARRAY);
                    bSourceProcessBuffer3 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nWidth, nHeight, nFloorC, nFllorR, iCamFlip, szBAYER_ARRAY);
                    bSourceProcessBuffer4 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nWidth, nHeight, nFloorC, nFllorR1, iCamFlip, szBAYER_ARRAY);

                    *pProcessDestBuffer = (char)((float)bSourceProcessBuffer1 * rXRate * rYRate +
                            (float)bSourceProcessBuffer2 * rXRate * (1 - rYRate) +
                            (float)bSourceProcessBuffer3 * (1 - rXRate) * (1 - rYRate) +
                            (float)bSourceProcessBuffer4 * (1 - rXRate) * rYRate);

                    pProcessDestBuffer++;

                }
            }
        }
    }
}



int Align_Vertical_68(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nChannelCount, float* landmark_ptr,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY)
{
    float rEyeCenterX, rEyeCenterY, rMouthCenterY; 
    rMouthCenterY = (rConstPoints[2][1] + rConstPoints[3][1]) / 2;
    rEyeCenterX = (rConstPoints[0][0] + rConstPoints[1][0]) / 2;
    rEyeCenterY = (rConstPoints[0][1] + rConstPoints[1][1]) / 2;

    float rScale = (float)nDstHeight / 229.39;
    if (rDistanceEye_Mouth != 0.0f)
    {
        if (rMouthCenterY - rEyeCenterY)
        {
            rScale = rDistanceEye_Mouth / (rMouthCenterY - rEyeCenterY);
        }
        else
        {
            rScale = rDistanceEye_Mouth;
        }
    }

    ARM_Point3D_ offset;
    ARM_Point3D_ eyeOffset;

    Set_ByARM_Point3D_(&eyeOffset, -rEyeCenterX, -rEyeCenterY, 0);

    if (rFaceCenterX == -1.0f || rFaceCenterY == -1.0f)
    {
        Set_ByARM_Point3D_(&offset, nDstWidth * 0.5f, nDstHeight * 0.5f, 0);
    }
    else
    {
        Set_ByARM_Point3D_(&offset, rFaceCenterX, rFaceCenterY, 0);
    }

    cFaceGraphDest.nNodeNum = 5;
    cFaceGraphSource.nNodeNum = 5;

    int landmark68_id[2][5] = {
        { 36, 42, 48, 54, 33 },
        { 39, 45, 60, 64, 33 },
    };

    int i;
    for (i = 0; i < 5; i++)
    {
        float rDestPointX, rDestPointY, rDestPointZ;
        rDestPointX = rConstPoints[i][0];
        rDestPointY = rConstPoints[i][1];
        rDestPointZ = rConstPoints[i][2];

        rDestPointX += eyeOffset.rX;
        rDestPointY += eyeOffset.rY;
        rDestPointZ += eyeOffset.rZ;

        rDestPointX *= rScale;
        rDestPointY *= rScale;
        rDestPointZ *= rScale;

        rDestPointX += offset.rX;
        rDestPointY += offset.rY;
        rDestPointZ += offset.rZ;

        cFaceGraphDest.pxNodes[i].rX = rDestPointX;
        cFaceGraphDest.pxNodes[i].rY = rDestPointY;
        cFaceGraphDest.pxNodes[i].rZ = rDestPointZ;

        cFaceGraphSource.pxNodes[i].rX = (landmark_ptr[landmark68_id[0][i] * 2] + landmark_ptr[landmark68_id[1][i] * 2]) / 2;
        cFaceGraphSource.pxNodes[i].rY = (landmark_ptr[landmark68_id[0][i] * 2 + 1] + landmark_ptr[landmark68_id[1][i] * 2 + 1]) / 2;

        cFaceGraphSource.pxNodes[i].rZ = 0;
    }

    GetTransform2D_ByARM_SpatialGraph_(&cFaceGraphDest, &cFaceGraphSource, &cTransform2D, 7);

    //cFaceGraphDest.GetTransform2D(&cFaceGraphSource, &cTransform2D, 7);
    ARM_Point3D_ pTempEyePoint[2];
    pTempEyePoint[0] = cFaceGraphSource.pxNodes[0];
    pTempEyePoint[1] = cFaceGraphSource.pxNodes[1];

    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[0]);
    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[1]);

    float rTempCenterX, rTempCenterY;
    rTempCenterX = (pTempEyePoint[0].rX + pTempEyePoint[1].rX) / 2;
    rTempCenterY = (pTempEyePoint[0].rY + pTempEyePoint[1].rY) / 2;

    cTransform2D.xTranPoint.rX += (rFaceCenterX - rTempCenterX);
    cTransform2D.xTranPoint.rY += (rFaceCenterY - rTempCenterY);

    //GetReverseTransform(&cTransform2D);
    ReverseMat_ByARM_RoateMat2D_(&cTransform2D.xRotateMat);
    ARM_Point2D_ pxPoint = Operator_Point2D_Mul_ByARM_RoateMat2D_(&cTransform2D.xRotateMat, &cTransform2D.xTranPoint);
    cTransform2D.xTranPoint.rX = -pxPoint.rX;
    cTransform2D.xTranPoint.rY = -pxPoint.rY;

    memset(pDstBuf, 0, nDstWidth * nDstHeight * nChannelCount);

    BYTE* pProcessDestBuffer = pDstBuf;

    int nX, nY;
    for (nY = 0; nY < nDstHeight; nY++)
    {
        for (nX = 0; nX < nDstWidth; nX++)
        {
            float rSourceX, rSourceY;
            ARM_Point2D_ pointDest, pointSource;

            pointDest.rX = nX;
            pointDest.rY = nY;

            Transform_ByARM_LinearTransform2D_(&cTransform2D, &pointDest);

            pointSource = pointDest;
            rSourceX = pointSource.rX;
            rSourceY = pointSource.rY;

            if (rSourceX < 0 || rSourceX > nSrcWidth - 1 || rSourceY < 0 || rSourceY > nSrcHeight - 1)
            {
                pProcessDestBuffer += nChannelCount;
                continue;
            }
            else
            {
                int nFllorR, nFloorC, nFllorR1, nFloorC1;
                nFllorR = (int)rSourceY;
                nFloorC = (int)rSourceX;

                if (nFllorR == rSourceY && nFloorC == rSourceX)
                {
                    BYTE* pSourceProcessBuffer;
                    pSourceProcessBuffer = pSrcBuf + (nFllorR * nSrcWidth + nFloorC);
                    int nChannelIndex;
                    for (nChannelIndex = 0; nChannelIndex < nChannelCount; nChannelIndex++)
                    {
                        *pProcessDestBuffer = *pSourceProcessBuffer;
                        pProcessDestBuffer++;
                    }
                }
                else
                {
                    float rXRate, rYRate;
                    rYRate = rSourceY - nFllorR;
                    rXRate = rSourceX - nFloorC;

                    nFloorC1 = nFloorC + 1;
                    nFllorR1 = nFllorR + 1;

                    if (nFloorC == nSrcWidth - 1)
                    {
                        nFloorC1 = nFloorC;
                        rXRate = 0;
                    }

                    if (nFllorR == nSrcHeight - 1)
                    {
                        nFllorR1 = nFllorR;
                        rYRate = 0;
                    }

                    int nChanelIndex;
                    BYTE *pSourceProcessBuffer1, *pSourceProcessBuffer2, *pSourceProcessBuffer3, *pSourceProcessBuffer4;
                    pSourceProcessBuffer1 = pSrcBuf + (nFllorR1 * nSrcWidth + nFloorC1) * nChannelCount;
                    pSourceProcessBuffer2 = pSrcBuf + (nFllorR * nSrcWidth + nFloorC1) * nChannelCount;
                    pSourceProcessBuffer3 = pSrcBuf + (nFllorR * nSrcWidth + nFloorC) * nChannelCount;
                    pSourceProcessBuffer4 = pSrcBuf + (nFllorR1 * nSrcWidth + nFloorC) * nChannelCount;
                    for (nChanelIndex = 0; nChanelIndex < nChannelCount; nChanelIndex++)
                    {
                        *pProcessDestBuffer = (char)((float)*pSourceProcessBuffer1 * rXRate * rYRate +
                            (float)*pSourceProcessBuffer2 * rXRate * (1 - rYRate) +
                            (float)*pSourceProcessBuffer3 * (1 - rXRate) * (1 - rYRate) +
                            (float)*pSourceProcessBuffer4 * (1 - rXRate) * rYRate);

                        pProcessDestBuffer++;
                        pSourceProcessBuffer1++;
                        pSourceProcessBuffer2++;
                        pSourceProcessBuffer3++;
                        pSourceProcessBuffer4++;
                    }

                }
            }
        }
    }
    return 0;
}


int Align_Vertical_68_BayerConvert(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nChannelCount, float* landmark_ptr,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY, int iCamFlip, char* szBAYER_ARRAY)
{
    float rEyeCenterX, rEyeCenterY, rMouthCenterY;
    rMouthCenterY = (rConstPoints[2][1] + rConstPoints[3][1]) / 2;
    rEyeCenterX = (rConstPoints[0][0] + rConstPoints[1][0]) / 2;
    rEyeCenterY = (rConstPoints[0][1] + rConstPoints[1][1]) / 2;

    float rScale = (float)nDstHeight / 229.39;
    if (rDistanceEye_Mouth != 0.0f)
    {
        if (rMouthCenterY - rEyeCenterY)
        {
            rScale = rDistanceEye_Mouth / (rMouthCenterY - rEyeCenterY);
        }
        else
        {
            rScale = rDistanceEye_Mouth;
        }
    }

    ARM_Point3D_ offset;
    ARM_Point3D_ eyeOffset;

    Set_ByARM_Point3D_(&eyeOffset, -rEyeCenterX, -rEyeCenterY, 0);

    if (rFaceCenterX == -1.0f || rFaceCenterY == -1.0f)
    {
        Set_ByARM_Point3D_(&offset, nDstWidth * 0.5f, nDstHeight * 0.5f, 0);
    }
    else
    {
        Set_ByARM_Point3D_(&offset, rFaceCenterX, rFaceCenterY, 0);
    }

    cFaceGraphDest.nNodeNum = 5;
    cFaceGraphSource.nNodeNum = 5;

    int landmark68_id[2][5] = {
        { 36, 42, 48, 54, 33 },
        { 39, 45, 60, 64, 33 },
    };

    int i;
    for (i = 0; i < 5; i++)
    {
        float rDestPointX, rDestPointY, rDestPointZ;
        rDestPointX = rConstPoints[i][0];
        rDestPointY = rConstPoints[i][1];
        rDestPointZ = rConstPoints[i][2];

        rDestPointX += eyeOffset.rX;
        rDestPointY += eyeOffset.rY;
        rDestPointZ += eyeOffset.rZ;

        rDestPointX *= rScale;
        rDestPointY *= rScale;
        rDestPointZ *= rScale;

        rDestPointX += offset.rX;
        rDestPointY += offset.rY;
        rDestPointZ += offset.rZ;

        cFaceGraphDest.pxNodes[i].rX = rDestPointX;
        cFaceGraphDest.pxNodes[i].rY = rDestPointY;
        cFaceGraphDest.pxNodes[i].rZ = rDestPointZ;

        cFaceGraphSource.pxNodes[i].rX = (landmark_ptr[landmark68_id[0][i] * 2] + landmark_ptr[landmark68_id[1][i] * 2]) / 2;
        cFaceGraphSource.pxNodes[i].rY = (landmark_ptr[landmark68_id[0][i] * 2 + 1] + landmark_ptr[landmark68_id[1][i] * 2 + 1]) / 2;

        cFaceGraphSource.pxNodes[i].rZ = 0;
    }

    GetTransform2D_ByARM_SpatialGraph_(&cFaceGraphDest, &cFaceGraphSource, &cTransform2D, 7);

    //cFaceGraphDest.GetTransform2D(&cFaceGraphSource, &cTransform2D, 7);
    ARM_Point3D_ pTempEyePoint[2];
    pTempEyePoint[0] = cFaceGraphSource.pxNodes[0];
    pTempEyePoint[1] = cFaceGraphSource.pxNodes[1];

    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[0]);
    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[1]);

    float rTempCenterX, rTempCenterY;
    rTempCenterX = (pTempEyePoint[0].rX + pTempEyePoint[1].rX) / 2;
    rTempCenterY = (pTempEyePoint[0].rY + pTempEyePoint[1].rY) / 2;

    cTransform2D.xTranPoint.rX += (rFaceCenterX - rTempCenterX);
    cTransform2D.xTranPoint.rY += (rFaceCenterY - rTempCenterY);

    //GetReverseTransform(&cTransform2D);
    ReverseMat_ByARM_RoateMat2D_(&cTransform2D.xRotateMat);
    ARM_Point2D_ pxPoint = Operator_Point2D_Mul_ByARM_RoateMat2D_(&cTransform2D.xRotateMat, &cTransform2D.xTranPoint);
    cTransform2D.xTranPoint.rX = -pxPoint.rX;
    cTransform2D.xTranPoint.rY = -pxPoint.rY;

    memset(pDstBuf, 0, nDstWidth * nDstHeight * nChannelCount);

    BYTE* pProcessDestBuffer = pDstBuf;

    int nX, nY;
    for (nY = 0; nY < nDstHeight; nY++)
    {
        for (nX = 0; nX < nDstWidth; nX++)
        {
            float rSourceX, rSourceY;
            ARM_Point2D_ pointDest, pointSource;

            pointDest.rX = nX;
            pointDest.rY = nY;

            Transform_ByARM_LinearTransform2D_(&cTransform2D, &pointDest);

            pointSource = pointDest;
            rSourceX = pointSource.rX;
            rSourceY = pointSource.rY;

            if (rSourceX < 0 || rSourceX > nSrcWidth - 1 || rSourceY < 0 || rSourceY > nSrcHeight - 1)
            {
                pProcessDestBuffer += nChannelCount;
                continue;
            }
            else
            {
                int nFllorR, nFloorC, nFllorR1, nFloorC1;
                nFllorR = (int)rSourceY;
                nFloorC = (int)rSourceX;

                if (nFllorR == rSourceY && nFloorC == rSourceX)
                {
//                    BYTE* pSourceProcessBuffer;
//                    pSourceProcessBuffer = pSrcBuf + (nFllorR * nSrcWidth + nFloorC);
//                    int nChannelIndex;
//                    for (nChannelIndex = 0; nChannelIndex < nChannelCount; nChannelIndex++)
//                    {
//                        *pProcessDestBuffer = *pSourceProcessBuffer;
//                        pProcessDestBuffer++;
//                    }
                    *pProcessDestBuffer = getSrcBufferFromRotatedCoordinate(pSrcBuf, nSrcWidth, nSrcHeight, nFloorC, nFllorR, iCamFlip, szBAYER_ARRAY);
                    pProcessDestBuffer ++;
                }
                else
                {
                    float rXRate, rYRate;
                    rYRate = rSourceY - nFllorR;
                    rXRate = rSourceX - nFloorC;

                    nFloorC1 = nFloorC + 1;
                    nFllorR1 = nFllorR + 1;

                    if (nFloorC == nSrcWidth - 1)
                    {
                        nFloorC1 = nFloorC;
                        rXRate = 0;
                    }

                    if (nFllorR == nSrcHeight - 1)
                    {
                        nFllorR1 = nFllorR;
                        rYRate = 0;
                    }

//                    int nChanelIndex;
//                    BYTE *pSourceProcessBuffer1, *pSourceProcessBuffer2, *pSourceProcessBuffer3, *pSourceProcessBuffer4;
//                    pSourceProcessBuffer1 = pSrcBuf + (nFllorR1 * nSrcWidth + nFloorC1) * nChannelCount;
//                    pSourceProcessBuffer2 = pSrcBuf + (nFllorR * nSrcWidth + nFloorC1) * nChannelCount;
//                    pSourceProcessBuffer3 = pSrcBuf + (nFllorR * nSrcWidth + nFloorC) * nChannelCount;
//                    pSourceProcessBuffer4 = pSrcBuf + (nFllorR1 * nSrcWidth + nFloorC) * nChannelCount;
//                    for (nChanelIndex = 0; nChanelIndex < nChannelCount; nChanelIndex++)
//                    {
//                        *pProcessDestBuffer = (char)((float)*pSourceProcessBuffer1 * rXRate * rYRate +
//                            (float)*pSourceProcessBuffer2 * rXRate * (1 - rYRate) +
//                            (float)*pSourceProcessBuffer3 * (1 - rXRate) * (1 - rYRate) +
//                            (float)*pSourceProcessBuffer4 * (1 - rXRate) * rYRate);

//                        pProcessDestBuffer++;
//                        pSourceProcessBuffer1++;
//                        pSourceProcessBuffer2++;
//                        pSourceProcessBuffer3++;
//                        pSourceProcessBuffer4++;
//                    }

                    BYTE bSourceProcessBuffer1, bSourceProcessBuffer2, bSourceProcessBuffer3, bSourceProcessBuffer4;
                    bSourceProcessBuffer1 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nSrcWidth, nSrcHeight, nFloorC1, nFllorR1, iCamFlip, szBAYER_ARRAY);
                    bSourceProcessBuffer2 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nSrcWidth, nSrcHeight, nFloorC1, nFllorR, iCamFlip, szBAYER_ARRAY);
                    bSourceProcessBuffer3 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nSrcWidth, nSrcHeight, nFloorC, nFllorR, iCamFlip, szBAYER_ARRAY);
                    bSourceProcessBuffer4 = getSrcBufferFromRotatedCoordinate(pSrcBuf, nSrcWidth, nSrcHeight, nFloorC, nFllorR1, iCamFlip, szBAYER_ARRAY);

                    *pProcessDestBuffer = (char)((float)bSourceProcessBuffer1 * rXRate * rYRate +
                            (float)bSourceProcessBuffer2 * rXRate * (1 - rYRate) +
                            (float)bSourceProcessBuffer3 * (1 - rXRate) * (1 - rYRate) +
                            (float)bSourceProcessBuffer4 * (1 - rXRate) * rYRate);
                    pProcessDestBuffer++;
                }
            }
        }
    }
    return 0;
}


BOOL GetTransform2D_ByARM_SpatialGraph_(ARM_SpatialGraph_* pxDicFaceGraph, ARM_SpatialGraph_ *pxInFaceGraph, ARM_LinearTransform2D_* psTransform2D, int nAltType)
{
    float rTemp;
    ARM_LinearTransform2D_ sLinearTransform;
    ARM_Point2D_ sDiffDic, sDiffIn, sInCenterNode, sDicCenterNode, xTempPoint;
    float rQx, rQy, rPx, rPy, rIxDy, rIyDx, rIyDy, rIxDx, rLengthRate, rFeature[8], prTemp[8];
    ARM_RotateMat2D_ sRotateMat;
    ARM_Point3D_* pxDicPoint, *pxInPoint;
    int nMaxIdx, i;

    rQx = rQy = 0;
    rPx = rPy = 0;

    pxDicPoint = pxDicFaceGraph->pxNodes;
    pxInPoint = pxInFaceGraph->pxNodes;
    i = pxInFaceGraph->nNodeNum;
    do
    {
        i --;
        rQx += pxInPoint->rX;
        rQy += pxInPoint->rY;
        rPx += pxDicPoint->rX;
        rPy += pxDicPoint->rY;
        pxDicPoint ++;
        pxInPoint ++;
    } while (i);

    i = pxInFaceGraph->nNodeNum;
    sInCenterNode.rX = rQx / i;
    sInCenterNode.rY = rQy / i;
    sDicCenterNode.rX = rPx / i;
    sDicCenterNode.rY = rPy / i;

    rQx = rPx = rIxDx = rIxDy = rIyDx = rIyDy = 0.0f;
    pxDicPoint = pxDicFaceGraph->pxNodes;
    pxInPoint = pxInFaceGraph->pxNodes;
    do
    {
        i --;
        sDiffDic.rX = pxDicPoint->rX - sDicCenterNode.rX;
        sDiffDic.rY = pxDicPoint->rY - sDicCenterNode.rY;
        sDiffIn.rX = pxInPoint->rX - sInCenterNode.rX;
        sDiffIn.rY = pxInPoint->rY - sInCenterNode.rY;

        rQx += sDiffIn.rX * sDiffIn.rX + sDiffIn.rY * sDiffIn.rY;
        rPx += sDiffDic.rX * sDiffDic.rX + sDiffDic.rY * sDiffDic.rY;

        rIxDx += sDiffIn.rX * sDiffDic.rX;
        rIxDy += sDiffIn.rX * sDiffDic.rY;
        rIyDx += sDiffIn.rY * sDiffDic.rX;
        rIyDy += sDiffIn.rY * sDiffDic.rY;
        pxDicPoint ++;
        pxInPoint ++;
    } while (i);

    if (fabs1(rQx) >= 9.9999997e-21f)
        rLengthRate = sqrt(rPx / rQx);
    else
        rLengthRate = sqrt(rPx / 9.999999999999999e-21f);

    rPx = rIxDx + rIyDy;
    rTemp = - (rIxDy + rIyDx);
    rPy = rIyDy - rIxDx;

    if (fabs1(rPx) >= 9.9999997e-21f)
    {
        rQy = (rIxDy - rIyDx) / rPx;
        rQx = sqrt(1.0f / (rQy * rQy + 1.0f));
        rQy = rQx * rQx;
        rQy = 1 - rQy;
        rQy = sqrt(rQy);
    }
    else
    {
        rQx = 0.0f;
        rQy = 1.0f;
    }

    if (fabs1(rPy) >= 9.9999997e-21f)
    {
        rTemp /= rPy;
        rPx = sqrt(1.0f / (rTemp * rTemp + 1.0f));
        rTemp = rPx * rPx;
        rTemp = 1 - rTemp;
        rPy = sqrt(rTemp);
    }
    else
    {
        rPx = 0.0f;
        rPy = 1.0f;
    }

    prTemp[0] = rQx * rIxDx;
    prTemp[1] = rQx * rIyDy;
    prTemp[2] = rQy * rIxDy;
    prTemp[3] = rQy * rIyDx;
    prTemp[4] = rPx * rIxDx;
    prTemp[5] = rPx * rIyDy;
    prTemp[6] = rPy * rIxDy;
    prTemp[7] = rPy * rIyDx;
    rFeature[0] =  prTemp[0] + prTemp[1] + prTemp[2] - prTemp[3];
    rFeature[1] =  prTemp[0] + prTemp[1] - prTemp[2] + prTemp[3];
    rFeature[2] = -prTemp[0] - prTemp[1] + prTemp[2] - prTemp[3];
    rFeature[3] = -prTemp[0] - prTemp[1] - prTemp[2] + prTemp[3];
    rFeature[4] = -prTemp[4] + prTemp[5] - prTemp[6] - prTemp[7];
    rFeature[5] = -prTemp[4] + prTemp[5] + prTemp[6] + prTemp[7];
    rFeature[6] =  prTemp[4] - prTemp[5] - prTemp[6] - prTemp[7];
    rFeature[7] =  prTemp[4] - prTemp[5] + prTemp[6] + prTemp[7];
    /*
    rFeature[0] =  rQx * rIxDx + rQx * rIyDy + rQy * rIxDy - rQy * rIyDx;
    rFeature[1] =  rQx * rIxDx + rQx * rIyDy - rQy * rIxDy + rQy * rIyDx;
    rFeature[2] = -rQx * rIxDx - rQx * rIyDy + rQy * rIxDy - rQy * rIyDx;
    rFeature[3] = -rQx * rIxDx - rQx * rIyDy - rQy * rIxDy + rQy * rIyDx;
    rFeature[4] = -rPx * rIxDx + rPx * rIyDy - rPy * rIxDy - rPy * rIyDx;
    rFeature[5] = -rPx * rIxDx + rPx * rIyDy + rPy * rIxDy + rPy * rIyDx;
    rFeature[6] =  rPx * rIxDx - rPx * rIyDy - rPy * rIxDy - rPy * rIyDx;
    rFeature[7] =  rPx * rIxDx - rPx * rIyDy + rPy * rIxDy + rPy * rIyDx;
    */
    rIyDx = -4.2949673e9f;
    nMaxIdx = 0;
    for (i = 0; i < 8; i++)
    {
        if (rIyDx < rFeature[i])
        {
            rIyDx = rFeature[i];
            nMaxIdx = i;
        }
    }

    switch (nMaxIdx)
    {
    case 0 :
        sRotateMat.prValue[0] = rQx;
        sRotateMat.prValue[1] = -rQy;
        sRotateMat.prValue[2] = rQy;
        sRotateMat.prValue[3] = rQx;
        break;
    case 1 :
        sRotateMat.prValue[0] = rQx;
        sRotateMat.prValue[1] = rQy;
        sRotateMat.prValue[2] = -rQy;
        sRotateMat.prValue[3] = rQx;
        break;
    case 2 :
        sRotateMat.prValue[0] = -rQx;
        sRotateMat.prValue[1] = -rQy;
        sRotateMat.prValue[2] = rQy;
        sRotateMat.prValue[3] = -rQx;
        break;
    case 3 :
        sRotateMat.prValue[0] = -rQx;
        sRotateMat.prValue[1] = rQy;
        sRotateMat.prValue[2] = -rQy;
        sRotateMat.prValue[3] = -rQx;
        break;
    case 4 :
        sRotateMat.prValue[0] = -rPx;
        sRotateMat.prValue[1] = -rPy;
        sRotateMat.prValue[2] = -rPy;
        sRotateMat.prValue[3] = rPx;
        break;
    case 5 :
        sRotateMat.prValue[0] = -rPx;
        sRotateMat.prValue[1] = rPy;
        sRotateMat.prValue[2] = rPy;
        sRotateMat.prValue[3] = rPx;
        break;
    case 6 :
        sRotateMat.prValue[0] = rPx;
        sRotateMat.prValue[1] = -rPy;
        sRotateMat.prValue[2] = -rPy;
        sRotateMat.prValue[3] = -rPx;
        break;
    case 7 :
        sRotateMat.prValue[0] = rPx;
        sRotateMat.prValue[1] = rPy;
        sRotateMat.prValue[2] = rPy;
        sRotateMat.prValue[3] = -rPx;
        break;
    default :
        sRotateMat.prValue[0] = 0;
        sRotateMat.prValue[1] = 0;
        sRotateMat.prValue[2] = 0;
        sRotateMat.prValue[3] = -0;
        break;
    }

    xTempPoint.rX = (sRotateMat.prValue[0] * sInCenterNode.rX + sRotateMat.prValue[1] * sInCenterNode.rY) * rLengthRate;
    xTempPoint.rY = (sRotateMat.prValue[2] * sInCenterNode.rX + sRotateMat.prValue[3] * sInCenterNode.rY) * rLengthRate;

    sDiffDic.rX = sDicCenterNode.rX - xTempPoint.rX;
    sDiffDic.rY = sDicCenterNode.rY - xTempPoint.rY;

    sLinearTransform.xRotateMat.prValue[0] = sRotateMat.prValue[0] * rLengthRate;
    sLinearTransform.xRotateMat.prValue[1] = sRotateMat.prValue[1] * rLengthRate;
    sLinearTransform.xRotateMat.prValue[2] = sRotateMat.prValue[2] * rLengthRate;
    sLinearTransform.xRotateMat.prValue[3] = sRotateMat.prValue[3] * rLengthRate;

    sLinearTransform.xTranPoint = sDiffDic;
    //sLinearTransform.xTranPoint.rX = sDiffDic.rX;
    //sLinearTransform.xTranPoint.rY = sDiffDic.rY;
    *psTransform2D = sLinearTransform;
    return 1;
}

void Set_ByARM_Point3D_(ARM_Point3D_* pxPoint, float X, float Y, float Z)
{
    pxPoint->rX = X;
    pxPoint->rY = Y;
    pxPoint->rZ = Z;
}

// ARM_LinearTransform2D_
void Transform_ByARM_LinearTransform2D_(ARM_LinearTransform2D_* psLinearTransform2D, ARM_Point2D_* pxPoint)
{
    float rX, rY;
    rX = *psLinearTransform2D->xRotateMat.prValue * pxPoint->rX + *(psLinearTransform2D->xRotateMat.prValue + 1) * pxPoint->rY + psLinearTransform2D->xTranPoint.rX;
    rY = *(psLinearTransform2D->xRotateMat.prValue + 2) * pxPoint->rX + *(psLinearTransform2D->xRotateMat.prValue + 3) * pxPoint->rY + psLinearTransform2D->xTranPoint.rY;
    pxPoint->rX = rX;
    pxPoint->rY = rY;
}

void ReverseMat_ByARM_RoateMat2D_(ARM_RotateMat2D_* psRotateMat2D)
{
    float rDet, rTemp;

    rDet = psRotateMat2D->prValue[0] * psRotateMat2D->prValue[3] - psRotateMat2D->prValue[2] * psRotateMat2D->prValue[1];

    if (rDet != 0.0f)
    {
        rDet = 1.0f / rDet;
        rTemp = psRotateMat2D->prValue[0];
        psRotateMat2D->prValue[0] = psRotateMat2D->prValue[3] * rDet;
        psRotateMat2D->prValue[3] = rTemp * rDet;
        psRotateMat2D->prValue[1] *= -rDet;
        psRotateMat2D->prValue[2] *= -rDet;
    }
    else
    {
        memset(psRotateMat2D->prValue, 0, sizeof(float) * 4);
    }
}

// ARM_RotateMat2D_
ARM_Point2D_ Operator_Point2D_Mul_ByARM_RoateMat2D_(ARM_RotateMat2D_* psRotateMat2D, ARM_Point2D_* pxPoint)
{
    ARM_Point2D_ xOutPoint;
    xOutPoint.rX = psRotateMat2D->prValue[0] * pxPoint->rX + psRotateMat2D->prValue[1] * pxPoint->rY;
    xOutPoint.rY = psRotateMat2D->prValue[2] * pxPoint->rX + psRotateMat2D->prValue[3] * pxPoint->rY;
    return xOutPoint;
}


float fabs1(float r)
{
    if (r < 0)
        return -r;
    return r;
}

int getAreaInSrcImage_68(int nSrcWidth, int nSrcHeight, int nDstWidth, int nDstHeight, float* landmark_ptr,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY, int* pnRectInSrc)
{
    float rEyeCenterX, rEyeCenterY, rMouthCenterY;
    rMouthCenterY = (rConstPoints[2][1] + rConstPoints[3][1]) / 2;
    rEyeCenterX = (rConstPoints[0][0] + rConstPoints[1][0]) / 2;
    rEyeCenterY = (rConstPoints[0][1] + rConstPoints[1][1]) / 2;

    float rScale = (float)nDstHeight / 229.39;
    if (rDistanceEye_Mouth != 0.0f)
    {
        if (rMouthCenterY - rEyeCenterY)
        {
            rScale = rDistanceEye_Mouth / (rMouthCenterY - rEyeCenterY);
        }
        else
        {
            rScale = rDistanceEye_Mouth;
        }
    }

    ARM_Point3D_ offset;
    ARM_Point3D_ eyeOffset;

    Set_ByARM_Point3D_(&eyeOffset, -rEyeCenterX, -rEyeCenterY, 0);

    if (rFaceCenterX == -1.0f || rFaceCenterY == -1.0f)
    {
        Set_ByARM_Point3D_(&offset, nDstWidth * 0.5f, nDstHeight * 0.5f, 0);
    }
    else
    {
        Set_ByARM_Point3D_(&offset, rFaceCenterX, rFaceCenterY, 0);
    }

    cFaceGraphDest.nNodeNum = 5;
    cFaceGraphSource.nNodeNum = 5;

    int landmark68_id[2][5] = {
        { 36, 42, 48, 54, 33 },
        { 39, 45, 60, 64, 33 },
    };

    int i;
    for (i = 0; i < 5; i++)
    {
        float rDestPointX, rDestPointY, rDestPointZ;
        rDestPointX = rConstPoints[i][0];
        rDestPointY = rConstPoints[i][1];
        rDestPointZ = rConstPoints[i][2];

        rDestPointX += eyeOffset.rX;
        rDestPointY += eyeOffset.rY;
        rDestPointZ += eyeOffset.rZ;

        rDestPointX *= rScale;
        rDestPointY *= rScale;
        rDestPointZ *= rScale;

        rDestPointX += offset.rX;
        rDestPointY += offset.rY;
        rDestPointZ += offset.rZ;

        cFaceGraphDest.pxNodes[i].rX = rDestPointX;
        cFaceGraphDest.pxNodes[i].rY = rDestPointY;
        cFaceGraphDest.pxNodes[i].rZ = rDestPointZ;

        cFaceGraphSource.pxNodes[i].rX = (landmark_ptr[landmark68_id[0][i] * 2] + landmark_ptr[landmark68_id[1][i] * 2]) / 2;
        cFaceGraphSource.pxNodes[i].rY = (landmark_ptr[landmark68_id[0][i] * 2 + 1] + landmark_ptr[landmark68_id[1][i] * 2 + 1]) / 2;

        cFaceGraphSource.pxNodes[i].rZ = 0;
    }

    GetTransform2D_ByARM_SpatialGraph_(&cFaceGraphDest, &cFaceGraphSource, &cTransform2D, 7);

    //cFaceGraphDest.GetTransform2D(&cFaceGraphSource, &cTransform2D, 7);
    ARM_Point3D_ pTempEyePoint[2];
    pTempEyePoint[0] = cFaceGraphSource.pxNodes[0];
    pTempEyePoint[1] = cFaceGraphSource.pxNodes[1];

    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[0]);
    Transform_ByARM_LinearTransform2D_(&cTransform2D, (ARM_Point2D_*)&pTempEyePoint[1]);

    float rTempCenterX, rTempCenterY;
    rTempCenterX = (pTempEyePoint[0].rX + pTempEyePoint[1].rX) / 2;
    rTempCenterY = (pTempEyePoint[0].rY + pTempEyePoint[1].rY) / 2;

    cTransform2D.xTranPoint.rX += (rFaceCenterX - rTempCenterX);
    cTransform2D.xTranPoint.rY += (rFaceCenterY - rTempCenterY);

    //GetReverseTransform(&cTransform2D);
    ReverseMat_ByARM_RoateMat2D_(&cTransform2D.xRotateMat);
    ARM_Point2D_ pxPoint = Operator_Point2D_Mul_ByARM_RoateMat2D_(&cTransform2D.xRotateMat, &cTransform2D.xTranPoint);
    cTransform2D.xTranPoint.rX = -pxPoint.rX;
    cTransform2D.xTranPoint.rY = -pxPoint.rY;

    ARM_Point2D_ pointConner[4];
    pointConner[0].rX = -2;
    pointConner[0].rY = -2;
    pointConner[1].rX = nDstWidth + 1;
    pointConner[1].rY = -2;
    pointConner[2].rX = -2;
    pointConner[2].rY = nDstHeight + 1;
    pointConner[3].rX = nDstWidth + 1;
    pointConner[3].rY = nDstHeight + 1;

    Transform_ByARM_LinearTransform2D_(&cTransform2D, &pointConner[0]);
    Transform_ByARM_LinearTransform2D_(&cTransform2D, &pointConner[1]);
    Transform_ByARM_LinearTransform2D_(&cTransform2D, &pointConner[2]);
    Transform_ByARM_LinearTransform2D_(&cTransform2D, &pointConner[3]);

    // int nLinePointIndex[4][2];
    // nLinePointIndex[0][0] = 0;
    // nLinePointIndex[0][1] = 1;
    // nLinePointIndex[1][0] = 0;
    // nLinePointIndex[1][1] = 2;
    // nLinePointIndex[2][0] = 1;
    // nLinePointIndex[2][1] = 3;
    // nLinePointIndex[3][0] = 2;
    // nLinePointIndex[3][1] = 3;

    int nMinY, nMaxY;
    int nMinX, nMaxX;
    nMinY = nSrcHeight;
    nMaxY = 0;
    nMinX = nSrcWidth;
    nMaxX = 0;

    int nPointIndex;
    for (nPointIndex = 0; nPointIndex < 4; nPointIndex++)
    {
        if (pointConner[nPointIndex].rY > nMaxY)
        {
            nMaxY = pointConner[nPointIndex].rY;
        }
        if (pointConner[nPointIndex].rY < nMinY)
        {
            nMinY = pointConner[nPointIndex].rY;
        }

        if (pointConner[nPointIndex].rX > nMaxX)
        {
            nMaxX = pointConner[nPointIndex].rX;
        }
        if (pointConner[nPointIndex].rX < nMinX)
        {
            nMinX = pointConner[nPointIndex].rX;
        }
    }
    if (nMinY < 0)
    {
        nMinY = 0;
    }

    if (nMinX < 0)
    {
        nMinX = 0;
    }

    if (nMaxX > nSrcWidth - 1)
    {
        nMaxX = nSrcWidth - 1;
    }
    if (nMaxY > nSrcHeight - 1)
    {
        nMaxY = nSrcHeight - 1;
    }


    pnRectInSrc[0] = nMinX;
    pnRectInSrc[1] = nMinY;
    pnRectInSrc[2] = nMaxX;
    pnRectInSrc[3] = nMaxY;

    return 0;
}

void CreateShrinkImage_normalize_FixRate(float* prDstImage, unsigned char* pbDstImage, int nDstWidth, int nDstHeight, float *prShrinkScale, unsigned char* pbSrcImage, int nSrcWidth, int nSrcHeight, float rMean, float rNorm)
{
    unsigned char* pSrcBuffer;
    int nWorkImageWidth = nSrcWidth;
    int nWorkImageHeight = nSrcHeight;
    int nXRateCounter = 0;
    int nYRateCounter = 0;
    int nRateY, nRateX, x, y, nPixel0, nPixel1;
    float rRateX, rRateY;

    if(prDstImage)
    {
        memset(prDstImage, 0, nDstWidth * nDstHeight * sizeof(float));
    }
    if(pbDstImage)
    {
        memset(pbDstImage, 0x80, nDstWidth * nDstHeight);
    }
    rRateX = (float)nSrcWidth / nDstWidth;
    rRateY = (float)nSrcHeight / nDstHeight;

    float rMaxRate = rRateX;
    if (rMaxRate < rRateY)
    {
        rMaxRate = rRateY;
    }
    *prShrinkScale = rMaxRate;

    nWorkImageWidth = nSrcWidth / rMaxRate;
    nWorkImageHeight = nSrcHeight / rMaxRate;

    if (nWorkImageWidth > nDstWidth)
    {
        nWorkImageWidth = nDstWidth;
    }
    if (nWorkImageHeight > nDstHeight)
    {
        nWorkImageHeight = nDstHeight;
    }

    nRateX = (int)(rMaxRate * 65536);
    nRateY = (int)(rMaxRate * 65536);

    int nDstIndexInit = 0;


    for (y = 0; y < nWorkImageHeight; y++)
    {
        int nOffY = (int)(nYRateCounter >> 16);
        int nOffY_16 = nOffY << 16;
        //int nTmpRate0 = (int)(nYRateCounter - 65536 * nOffY);
        int nTmpRate0 = (int)(nYRateCounter - nOffY_16);
        int nTmpRate1 = 65536 - nTmpRate0;

        pSrcBuffer = pbSrcImage + nOffY * nSrcWidth;
        //pDestBuffer = prDstImage + nDstIndexInit;
        //pDestTempBuffer = pbDstImage + nDstIndexInit;

        nXRateCounter = 0;
        int nDstIndex = nDstIndexInit;
        for (x = 0; x < nWorkImageWidth; x++)
        {
            int nOffX = (int)(nXRateCounter >> 16);
            int nOffX_16 = nOffX << 16;

            int pwTmp, pwTmp1;
            pwTmp = (((int)(pSrcBuffer[nOffX]) * nTmpRate1 + (int)(pSrcBuffer[nOffX + nSrcWidth]) * nTmpRate0) >> 0x0A);
            pwTmp1 = (((int)(pSrcBuffer[nOffX + 1]) * nTmpRate1 + (int)(pSrcBuffer[nOffX + nSrcWidth + 1]) * nTmpRate0) >> 0x0A);


            //nTmpRate0 = (int)(nXRateCounter - 65536 * nOffX);
            int nTmpRateX0 = (int)(nXRateCounter - nOffX_16);
            int nTmpRateX1 = 65536 - nTmpRateX0;

            nPixel0 = (int)pwTmp * nTmpRateX1;
            nPixel1 = (int)pwTmp1 * nTmpRateX0;

            unsigned char bValue = (unsigned char)((nPixel0 + nPixel1) >> 0x16);
            bValue = getGammaCorrection(bValue);

            if(prDstImage)
            {
                float rValue = (float)bValue;
                rValue = (rValue - rMean) * rNorm;
                prDstImage[nDstIndex] = rValue;
            }
            if(pbDstImage)
            {
                pbDstImage[nDstIndex] = bValue;
            }
            nXRateCounter += nRateX;
            nDstIndex ++;
        }

        nDstIndexInit += nDstWidth;
        nYRateCounter += nRateY;
    }
}



