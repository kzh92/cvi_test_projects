#ifndef HTYPE_H
#define HTYPE_H

#define _s8 char
#define _u8 unsigned char
#define _s16 short
#define _u16 unsigned short
#define _s32 int
#define _u32 unsigned int
#define _s64 long long
#define _u64 unsigned long long

#ifndef PI
#define PI 3.1415927410125732
#endif


#define TOTAL_ENROLL_REFINED_FEATURE_COUNT	4
#define TOTAL_ENROLL_RAW_FEATURE_COUNT		5
#define TOTAL_ENROLL_MAX_FEATURE_COUNT		10
#define UNIT_ENROLL_FEATURE_SIZE	0x1B04
#define TOTAL_ENROLL_FEATURE_SIZE	0x6C14
#define VERIFY_FEATURE_SIZE			0x5104

typedef struct _tagFDInfo_05_Data
{
	struct _tagFDInfo_05_Data* pData1;
	struct _tagFDInfo_05_Data* pData2;
} FDInfo_05_Data;

typedef struct _tagFDInfo_05
{
	FDInfo_05_Data* pData0;
	FDInfo_05_Data* pData;//param1
	struct _tagFDInfo_05* pInfo05_next;
	_s32 nSize;//param3
	_s32 nEOD_End;//param4
} FDInfo_05;

typedef struct _tagFDInfo_10
{
	struct _tagFDInfo_10* pInfo10_0;
	struct _tagFDInfo_10* pInfo10_1;
	_s32 nUnitCount;//param2
	_s32 nUnitSize;//param3
	_u8* pEOD;//param4
	_u8* pCurEOD;//param5
	_s32 nUnitMaxCount;//param6
	FDInfo_05* pInfo05;//pInfo05
	_s32 param8;
	struct _tagFDInfo_10* pInfo10_9;
} FDInfo_10;

typedef struct _tagHImage
{
	_u16	width;
	_u16	height;
	_u8*	data;
}HImage;

typedef struct _tagHModelResult
{
	_s32		faceLeft;
	_s32		faceTop;
	_s32		faceRight;
	_s32		faceBottom;
	_s32		modelSuccessFlag__;
	_s32		eyeLX;
	_s32		eyeLY;
	_s32		eyeRX;
	_s32		eyeRY;
}HModelResult;

struct FaceDetetResult
{
	HImage redusedImage;
	_s32 nFaceCount;
	FDInfo_10* pFaceRectArray;
	_s32* pnCoorValue;
	_s32* pnFaceRectValid;
};

#endif