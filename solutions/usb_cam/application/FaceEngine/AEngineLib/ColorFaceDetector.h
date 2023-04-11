#ifndef COLOR_FACE_DETECTOR_H
#define COLOR_FACE_DETECTOR_H

#ifndef BYTE
typedef unsigned char       BYTE;
#endif

typedef struct {
	int width;
	int height;
	BYTE* pbRBuffer;
	BYTE* pbGBuffer;
	BYTE* pbBBuffer;
}RGBImage;

#ifndef SRect1
typedef struct {
	int left;
	int top;
	int right;
	int bottom;
}SRect1;
#endif

#ifndef SPoint
typedef struct {
	int x;
	int y;
}SPoint;
#endif

#ifndef __min
#define __min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef NULL
#define NULL       0
#endif

#define DIC_FEATURE_NUM 5
#define FACE_WIDTH 24
#define FACE_HEIGHT 24
#define FACE_SIZE 576
#define NMX_BLUR_RADIUS 2
#define BLOCK_ROWS				2
#define BLOCK_COLS				2
#define BLOCK_NUM				4
#define HIST_BIN_NUM			118
#define FACE_WIDTH3	22
#define FEATURE_SIZE 472 //BLOCK_NUM * HIST_BIN_NUM
#define DIST_TH 2680

#define	 MALLOC2(ptr, sizeH, sizeW, type)				\
{													\
	int imalloc, size, linesize;					\
	linesize = sizeof(type)* (sizeW);				\
	size = sizeof(type*)* (sizeH)+(sizeH)* linesize;	\
	(ptr) = (type**)malloc(size);						\
	for (imalloc = 0; imalloc < (sizeH); imalloc++)	\
		(ptr)[imalloc] = (type*)((char*)(ptr)+\
		sizeof(type*)* (sizeH)+linesize * imalloc);	\
}

#ifdef __cplusplus
extern "C"{
#endif
	int CreateDetector();
    int DetectFace(RGBImage* pxInImage, BYTE* pbGrayImage, SRect1* pxFaceRect);
#ifdef __cplusplus
}
#endif

#endif
