#ifndef _GLASSREMOVE_H_
#define _GLASSREMOVE_H_
#include <stdlib.h>
#include "armCommon.h"
#include "type.h"

//#define TestMode

#include <vector>
using namespace  std;


#ifdef TestMode
int GlassesRemove(BYTE* pbImage, int nHeight, int nWidth, ARM_Point3D* pxModelPoints, int nModelPointNum, int fNormal, BYTE** pbDesImage, int* pnDesHeight, int* pnDesWidth, BYTE** pbDesImage1, int* pnDesHeight1, int* pnDesWidth1);
#else
int GlassesRemove(BYTE* pbImage, int nHeight, int nWidth, ARM_Point3D* pxModelPoints, int nModelPointNum, int fNormal);
#endif // TestMode

extern "C" {
int GlassRemoveInIR(BYTE* pbImage, int nHeight, int nWidth, ARM_Point3D* pxModelPoints, int nModelPointNum);
int RemoveReflectionInIR(unsigned char* pbImage, int nHeight, int nWidth, ARM_FaceRect xFaceRect);
}
#endif
