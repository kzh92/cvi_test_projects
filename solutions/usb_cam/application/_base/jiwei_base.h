#ifndef JIWEI_DRIVER_H
#define JIWEI_DRIVER_H

#include "appdef.h"
#include <stdio.h>
#include <stdbool.h>

#if (USE_WIFI_MODULE)

#ifdef __cplusplus
extern  "C"
{
#endif

#define JIWEI_WIDTH   480
#define JIWEI_HEIGHT  640

#define JW_DIVP1_CHN     4
#define JW_DIVP2_CHN     5
#define JW_VENC_CHN     1

int JW_BaseModuleInit(int id, int iRotate, bool iFlip, bool iMirror);
int JW_BaseModuleUnInit(int id);
int JW_InsertFrame2DIVP1(int id, unsigned char* pbBuf);
int JW_GetFrameFromVENC(unsigned char* pbBuf, int* iSize);

#ifdef __cplusplus
}
#endif

#endif // USE_WIFI_MODULE

#endif // JIWEI_DRIVER_H
