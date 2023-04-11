#ifndef _HCOMMON_H_
#define _HCOMMON_H_

#include "Htypes.h"

extern _s32 width_global;
extern _s32 height_global;
extern _s32 reduceScale;
extern _s32 expandWidth_global;
extern _s32 g_nDetectProcessMode;

_s32 *getElementFromList_E0312B2C(FDInfo_10 *pInfo10, _u32 nIndex);
void shrink_E0315F10(_u8 *src, _s32 src_height, _s32 src_width, _u8 *dst, _s32 dst_height, _s32 dst_width);



#endif


