#ifndef _ARMFaceRegionExtractor_H_
#define _ARMFaceRegionExtractor_H_

#include "armPatternMatcher.h"
#include "armCommon.h"

#define ARM_DETECTOR_PATCH_SIZE			28
#define ARM_SLIDING_STEP				5
#define ARM_RATE1						1.25f
#define ARM_RATE2						1.1892101f
#define ARM_FACE_MAX_NUM				1200
#define ARM_FACE_SUB_MAX_NUM			200
#define ARM_OVERLAPPED_RATE				35
#define	ARM_SEC_IMG_HEIGHT				59
#define ARM_BORDER_HEIGHT				8
#define ARM_BORDER_WIDTH				8

typedef enum {RECT_PATTEN = 0, QUAD_PATTERN, WAVE_PATTERN} PATTEN_TYPE;

#ifdef __cplusplus
extern	"C"
{
#endif

//int nMinValidOverlappedRegionCount: min overlap region count that not ignore.
//float rDeltaClassifierThreshold: delta value for classifier threshold
int ExtractFace_ByARM_FaceDetector(ARM_Face *pxFaceList, int nFaceListSize,unsigned char* pbImage, int nLandScape, int nAllDetectMode, int nMinValidOverlappedRegionCount, float rDeltaClassifierThreshold);
int ExtractFace_ByARM_FaceDetector_freeSize(ARM_Face *pxFaceList, int nFaceListSize, unsigned char* pbImage, int nW, int nH, int nAllDetectMode, int nMinValidOverlappedRegionCount, float rDeltaClassifierThreshold);
int ExtractFace_ByARM_FaceDetector_unlimited(ARM_Face *pxFaceList, int nFaceListSize, unsigned char* pbImage, int nW, int nH, int nIsIDPhoto, int nAllDetectMode, int nMinValidOverlappedRegionCount, float rDeltaClassifierThreshold);
//pnRect: face rect, pnRect[0]:nX, pnRect[1]:nY, pnRect[2]:width, pnRect[3]:height
int CheckFace_ByARM_FaceDetector_simple(unsigned char* pbImage, int nW, int nH, int* pnRect, int nFlip);
int ExtractFace_ByARM_FaceDetector_unlimited_tracking(ARM_Face *pxFaceList, int nFaceListSize, unsigned char* pbImage, int nW, int nH, int nMinValidOverlappedRegionCount, float rDeltaClassifierThreshold, int* pnFaceRects);

void LoadDictionary_ByARM_FaceDetector(unsigned char* pbDict);

int DetectFace_ByARM_FaceDetector(ARM_Face *pxFaceList, unsigned char* pbImage, int nLandScape, int nAllDetectMode, int nMinValidOverlappedRegionCount, float rDeltaClassifierThreshold, int nMinFaceSize);
int DetectFace_ByARM_FaceDetector_freeSize(ARM_Face *pxFaceList, unsigned char* pbImage, int nW, int nH, int nAllDetectMode, int nMinValidOverlappedRegionCount, float rDeltaClassifierThreshold, int nMinFaceSize);
int DetectFace_ByARM_FaceDetector_unlimited(ARM_Face *pxFaceList, unsigned char* pbImage, int nW, int nH, int nIsIDPhoto, int nAllDetectMode, int nMinValidOverlappedRegionCount, float rDeltaClassifierThreshold, int nMinFaceSize);

int getReduceSize(int* pnWidth, int* pnHeight);
int setReduceSize(int nWidth, int nHeight);
int getPyramidMaxNum();
int setPyramidMaxNum(int nPyramidMaxNum);

#ifdef __cplusplus
}
#endif

#endif //_ARMFaceRegionExtractor_H_

