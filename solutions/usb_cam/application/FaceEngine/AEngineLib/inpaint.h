#include "type.h"

// CVAPI(void) cvInpaint( const CvArr* src, const CvArr* inpaint_mask,
// 					  CvArr* dst, double inpaintRange, int flags );
void inpaint_process(SMat* src, SMat* inpaintMask, SMat* dst, int inpaintRange);
