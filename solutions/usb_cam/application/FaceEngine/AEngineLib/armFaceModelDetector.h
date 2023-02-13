#ifndef _ARMFaceModelExtractor_H_
#define _ARMFaceModelExtractor_H_

#include "armCommon.h"

#ifndef __EMULATOR_MODE__

#define MAX_MODEL_NUM			25
#define ARM_MODEL_DETECTOR_PATCH_SIZE				32

#ifdef __cplusplus
extern	"C"
{
#endif

float Extract_ByARM_FaceModelDetector(ARM_Face* pxFace, unsigned char* pbImage,  int nHeight, int nWidth);
float Extract_ByARM_FaceModelDetectorFrmoModel(ARM_Face* pxArmFace, float rConfidence, unsigned char* pbImage,  int nHeight, int nWidth);
void LoadDictionary_ByARM_FaceModelDetector(unsigned char* pbDict);


#ifdef __cplusplus
}
#endif
#endif
#endif // _ARMFaceModelExtractor_H_
