#ifndef __Memory_MAP_H_
#define __Memory_MAP_H_


#include "Htypes.h"
#include <memory>


extern _s32* g_pnIntegralForFD;
extern _u8* g_Memory;
extern _u8* g_00012C00;
extern _u8* g_000133D4;
extern _u8* g_0016C42E;
extern _u8* g_0016C42F;
extern _u8* g_0016C430;
extern _s32* g_0016C432;
extern _s32* g_0016C436;
extern _u8* g_0016C43A;
extern _u8* g_00083410;

int allocGlobalMemory();
int releaseGlobalMemory();

#endif
