/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef ANDROID_CAMERA_CAMERA_FORMAT_CONVERTERS_H
#define ANDROID_CAMERA_CAMERA_FORMAT_CONVERTERS_H

/*
* Contains declaration of the API that allows converting frames from one
* pixel format to another.
*
* For the emulator, we really need to convert into two formats: YV12, which is
* used by the camera framework for video, and RGB32 for preview window.
*/

//#include "camera-common.h"

/* Checks if conversion between two pixel formats is available.
* Param:
*  from - Pixel format to convert from.
*  to - Pixel format to convert to.
* Return:
*  boolean: 1 if converter is available, or 0 if no conversion exists.
*/
//extern int has_converter(uint32_t from, uint32_t to);

/* Converts a frame into multiple framebuffers.
* When camera service replies to a framebuffer request from the client, it
* usualy sends two framebuffers in the reply: one for video, and another for
* preview window. Since these two framebuffers have different pixel formats
* (most of the time), we need to do two conversions for each frame received from
* the camera. This is the main intention behind this routine: to have a one call
* that produces as many conversions as needed.
* Param:
*  frame - Frame to convert.
*  pixel_format - Defines pixel format for the converting framebuffer.
*  framebuffer_size, width, height - Converting framebuffer byte size, width,
*      and height.
*  framebuffers - Array of framebuffers where to convert the frame. Size of this
*      array is defined by the 'fbs_num' parameter. Note that the caller must
*      make sure that buffers are large enough to contain entire frame captured
*      from the device.
*  fbs_num - Number of entries in the 'framebuffers' array.
*  r_scale, g_scale, b_scale - White balance scale.
*  exp_comp - Expsoure compensation.
* Return:
*  0 on success, or non-zero value on failure.
*/
//extern int convert_frame(const void* frame,
//	uint32_t pixel_format,
//	size_t framebuffer_size,
//	int width,
//	int height,
//	ClientFrameBuffer* framebuffers,
//	int fbs_num,
//	float r_scale,
//	float g_scale,
//	float b_scale,
//	float exp_comp);

#include <inttypes.h>

typedef struct ShiftInitConfig
{
    float fZeroPlaneDistance;      // 40
    float fZeroPlanePixelSize;     // 0.037037f
    float fEmitterDCmosDistance;   // 4.0f
    uint32_t nDeviceMaxShiftValue; // 1024
    uint32_t nDeviceMaxDepthValue; // 2000 : 12bit ok, 10000, fail.

    uint32_t nConstShift;      // 200
    uint32_t nPixelSizeFactor; // 1
    uint32_t nParamCoeff;      // 4
    uint32_t nShiftScale;      // 10

    uint16_t nDepthMinCutOff; // 0
    uint16_t nDepthMaxCutOff; // 65534

    uint16_t w; // 480
    uint16_t h; // 640
} ShiftInitConfig;

typedef struct ShiftToDepthConfig
{
	/** The zero plane distance in depth units. */
	float fZeroPlaneDistance;
	/** The zero plane pixel size */
	float fZeroPlanePixelSize;
	/** The distance between the emitter and the Depth Cmos */
	float fEmitterDCmosDistance;
	/** The maximum possible shift value from this device. */
	uint32_t nDeviceMaxShiftValue;
	/** The maximum possible depth from this device (as opposed to a cut-off). */
	uint32_t nDeviceMaxDepthValue;

	uint32_t nConstShift;
	uint32_t nPixelSizeFactor;
	uint32_t nParamCoeff;
	uint32_t nShiftScale;

	uint16_t nDepthMinCutOff;
	uint16_t nDepthMaxCutOff;

	float fVerticalFOV;
	float fHorizontalFOV;
} ShiftToDepthConfig;

typedef struct ShiftToDepthTables
{
	int bIsInitialized;
	/** The shift-to-depth table. */
	uint16_t *pShiftToDepthTable;
	/** The number of entries in the shift-to-depth table. */
	uint32_t nShiftsCount;
	/** The depth-to-shift table. */
	uint16_t *pDepthToShiftTable;
	/** The number of entries in the depth-to-shift table. */
	uint32_t nDepthsCount;
} ShiftToDepthTables;

enum OpticsRelationShip
{
	ProjectLeftOfCMOS,
	ProjectRightOfCMOS
};

typedef struct WorldConversionCache
{
    float xzFactor;
    float yzFactor;
    float coeffX;
    float coeffY;
    int resolutionX;
    int resolutionY;
    int halfResX;
    int halfResY;
    float zFactor;
} WorldConversionCache;

#define _PIXEL_FORMAT_DEPTH_1_MM 1
#define _PIXEL_FORMAT_DEPTH_100_UM 100

#ifdef __cplusplus
extern "C" {
#endif
void
BAYER2YUV(const void* bayer,
			void* yuv,
			int width,
			int height,
			float r_scale,
			float g_scale,
			float b_scale,
			float exp_comp);

void BAYER2YUV_min(const void* bayer,
	void* yuv,
	int width,
	int height,
	int* pnRect = 0,
	char* szBAYER_ARRAY = 0);


void
BAYER2RGB(const void* bayer,
			void* rgb,
			int width,
			int height,
			float r_scale,
			float g_scale,
			float b_scale,
			float exp_comp);


unsigned char getYFromBAYER(
	const void* bayer,
	int width,
	int height,
	int nX, int nY,
	char* szBAYER_ARRAY = 0);


// int OrbbecDevice_init(ShiftInitConfig p);
// int OrbbecDevice_refreshWorldConversionCache(int resolutionX, int resolutionY, int zFactor);
// int OrbbecDevice_shift9_2ToDepth(uint16_t *src, uint16_t *dst, int w, int h);
// int parseFirstLine(unsigned short *pInput);
// 
// void shift_depth_init(int nWidth, int nHeight);

void BAYERToY_neon(const void* bayer, void* pY, int width, int height);


#ifdef __cplusplus
}
#endif



#endif  /* ANDROID_CAMERA_CAMERA_FORMAT_CONVERTERS_H */
