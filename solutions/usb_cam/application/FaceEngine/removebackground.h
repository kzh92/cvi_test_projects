#ifndef _REMOVEBACKGROUND_H_
#define _REMOVEBACKGROUND_H_

#include "EngineDef.h"
#include "EngineStruct.h"

#ifdef __cplusplus
extern	"C"
{
#endif

LIBFOO_DLL_EXPORTED int     removeBackground(unsigned char* pbLedOnImage, unsigned char* pbLedOffImage, int nWidth, int nHeight);

enum GammaMode
{
	Gamma_032A,
	Gamma_066,
};

LIBFOO_DLL_EXPORTED void    gammaCorrection(unsigned char* pBuffer, int nWidth, int nHeight, int nMode);

LIBFOO_DLL_EXPORTED int     rotate_DetectHand(unsigned char* imageData, int imageWidth, int imageHeight, int& nDetectedDeltaAngle, void* axResultPoints, int nPrevAngle);
LIBFOO_DLL_EXPORTED void    ImageRotation_toLandScape(unsigned char* pBuffer, int nSrcWidth, int nSrcHeight, int nDesWidth, int nDesHeight, int rotationAngle);
LIBFOO_DLL_EXPORTED void    rotatePointInFixSize(int nWidth, int nHeight, int& nPointX, int& nPointY, int nDegreeAngle);
LIBFOO_DLL_EXPORTED int     getHandAverageLedOnImage();
LIBFOO_DLL_EXPORTED void	applySharpenFilter(unsigned char* pBuffer, int nWidth, int nHeight);


#ifdef __cplusplus
}
#endif

#endif
