#ifndef _SN_H
#define _SN_H

#ifdef FE_ENGINE_LIB

#ifdef __cplusplus
extern  "C"
{
#endif // __cplusplus

int     seCheckThreshold(void* param, void* param1, void* param2);
int     fr_setSuitableThreshold(void* param, void* param1, void* param2);
int     fr_resetThresholdState(void* param, void* param1, void* param2);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FE_ENGINE_LIB

#endif // _SN_H
