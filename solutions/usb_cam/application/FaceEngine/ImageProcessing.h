#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "EngineStruct.h"

#ifdef __cplusplus
extern	"C"
{
#endif

LIBFOO_DLL_EXPORTED void    ScaleImage(unsigned char *pbOrg, unsigned char* pbScaledImage);
LIBFOO_DLL_EXPORTED int     CalcDiffImageA(unsigned char* pbDst, unsigned char* pbSrcLedOn, unsigned char *pbLedOff, int nDiffX, int nDiffY, int nLeft, int nTop, int nRight, int nBottom);
LIBFOO_DLL_EXPORTED int     CalcDiffImage(unsigned char* pbDst, unsigned char* pbSrcLedOn, unsigned char *pbLedOff, int nDiffX, int nDiffY, int nLeft, int nTop, int nRight, int nBottom, float rAlpha = 1.0f, float rBeta = 1.0f);
LIBFOO_DLL_EXPORTED void    NormalizeImage(int* pnImage, unsigned char* pbNormalized, int nH, int nW);
LIBFOO_DLL_EXPORTED void    Sobel_Process(unsigned char* pbSrc, unsigned char* pbEdge, int nH, int nW);
LIBFOO_DLL_EXPORTED int     GetFaceMotion(unsigned char* pbImage1, unsigned char* pbImage2, int nImageHeight, int nImageWidth, int nLeft, int nTop, int nH, int nW, int* pnXMotion, int* pnYMotion);
LIBFOO_DLL_EXPORTED int     GetFaceMotion_Fast(unsigned char* pbImage1, unsigned char* pbImage2, int nImageHeight, int nImageWidth, int nFaceLeft, int nFaceTop, int nFaceH, int nFaceW, int* pnXMotion, int* pnYMotion);
LIBFOO_DLL_EXPORTED void    ShrinkImage(unsigned char* pbInputImg, int nImgWid, int nImgHei, unsigned char* pbOutImg, int nOutWid, int nOutHei, SRect* pxRegion);
LIBFOO_DLL_EXPORTED void    ShrinkImage24(unsigned char* pbInputImg, int nImgWid, int nImgHei, unsigned char* pbOutImg, int nOutWid, int nOutHei, SRect* pxRegion);
LIBFOO_DLL_EXPORTED int     FaceImage2JpgImage(unsigned char* faceImage, unsigned char* jpgImage, int* jpgLen = 0);
LIBFOO_DLL_EXPORTED int     GetEvalLight24(unsigned char* pbClrData32, int width, int height);
LIBFOO_DLL_EXPORTED int     GetEvalLight8(unsigned char* pbGreyData, int width, int height);
LIBFOO_DLL_EXPORTED int     GetEvalLightFromRegion(unsigned char *pbYData, int imgWidth, int imgHeight, int left, int top, int width = 0, int height = 0);
LIBFOO_DLL_EXPORTED void    EqualizeAlignedImage(unsigned char* pImage, int nW, int nH);
LIBFOO_DLL_EXPORTED int     CheckValidColorImage(unsigned char* pBuffer, int nImageWidth, int nImageHeight,
                                int nFaceX, int nFaceY, int nFaceWidth, int nFaceHeight);

LIBFOO_DLL_EXPORTED void    Shrink_RGB(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width);
LIBFOO_DLL_EXPORTED void    Shrink_Grey(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width);

#ifdef __cplusplus
}
#endif

#endif
