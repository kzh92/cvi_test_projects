
#ifndef HALIGN_H__INCLUDED
#define HALIGN_H__INCLUDED

//#include "armCommon.h"

void halign(unsigned char* srcBuf, int nWidth, int nHeight, int nChannel, float rLeftX, float rLeftY, float rRightX, float rRightY,
	unsigned char* dstBuf, int nAlignWidth, int nAlignHeight, float rDistEM, int nCenterX, int nCenterY);

int Align_Vertical(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nChannelCount, void* xModelFaceGraph,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY);

void alignForModeling(unsigned char* src_buf, int nImageWidth, int nImageHeight, int nCropX, int nCropY, int nCropWidth, int nCropHeight, unsigned char* dst_buf, int nDestWidth, int nDestHeight);

void alignForModeling_bayerConvert(unsigned char* pSrcBuf, int nWidth, int nHeight, int nFaceLeft, int nFaceTop, int nFaceWidth,
    unsigned char* pDstBuf, int nAlignWidth, int nAlignHeight, int iCamFlip, char* szBAYER_ARRAY);

int Align_Vertical_68(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nChannelCount, float* landmark_ptr,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY);

int Align_Vertical_68_BayerConvert(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nChannelCount, float* landmark_ptr,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY, int iCamFlip, char* szBAYER_ARRAY);

int getAreaInSrcImage_68(int nSrcWidth, int nSrcHeight, int nDstWidth, int nDstHeight, float* landmark_ptr,
    float rDistanceEye_Mouth, float rFaceCenterX, float rFaceCenterY, int* pnRectInSrc);

void CreateShrinkImage_normalize_FixRate(float* prDstImage, unsigned char* pbDstImage, int nDstWidth, int nDstHeight, float *prShrinkScale,
                                         unsigned char* pbSrcImage, int nSrcWidth, int nSrcHeight, float rMean, float rNorm);
#endif // HALIGN_H__INCLUDED
