#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "EngineStruct.h"

#ifdef __cplusplus
extern	"C"
{
#endif
LIBFOO_DLL_EXPORTED void    Shrink_RGB(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width);
LIBFOO_DLL_EXPORTED void    Shrink_Grey(unsigned char *src, int src_height, int src_width, unsigned char *dst, int dst_height, int dst_width);
LIBFOO_DLL_EXPORTED void 	convert_bayer2y_rotate_cm(unsigned char* bayer, int width, int height);
#ifdef __cplusplus
}
#endif

#endif
