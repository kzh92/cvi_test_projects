#ifndef _HDETECT_H_
#define _HDETECT_H_

#include "Htypes.h"

#define MaxFDListCount	4
#define MaxFaceCount	20
#define FDListStatic

_s32 releaseMemoryForDetection();
_s32 FaceDetectProcess(HImage img, _s32* arg2, FaceDetetResult* result_detect, _u8* ret, int nMode);
_s32 chaeckFAceIsValid_E0328DE0(HImage *hImage, _s32 left, _s32 top, _s32 right, _s32 bottom);
_s32 generateHDetectionFromRectProcess(FaceDetetResult* result_detect, _s32* pnRect);


#endif



