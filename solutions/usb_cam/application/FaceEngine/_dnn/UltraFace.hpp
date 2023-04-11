//
//  UltraFace.hpp
//  UltraFaceTest
//
//  Created by vealocia on 2019/10/17.
//  Copyright © 2019 vealocia. All rights reserved.
//

#ifndef UltraFace_hpp
#define UltraFace_hpp

#pragma once

//#include "gpu.h"

#include <algorithm>
#include <iostream>
#include <string>
//#include <vector>

#define RFB 0
#define RFB_lite 1
#define Mn_lite 2
#define MODEL_TYPE Mn_lite

#if MODEL_TYPE == RFB
#define num_featuremap 4
#endif

#if MODEL_TYPE == RFB_lite
#define num_featuremap 3
#endif

#if MODEL_TYPE == Mn_lite
#define num_featuremap 4
#endif

#define hard_nms 1
#define blending_nms 2 /* mix nms was been proposaled in paper blaze face, aims to minimize the temporal jitter*/

#if MODEL_TYPE == RFB_lite
#define g_DNN_Detection_input_width   90
#define g_DNN_Detection_input_height  160
#define g_DNN_Detection_input_width_base   120
#define g_DNN_Detection_input_height_base  160
#endif

#if MODEL_TYPE == Mn_lite
#define g_DNN_Detection_input_width   108
#define g_DNN_Detection_input_height  192
#define g_DNN_Detection_input_width_base   240
#define g_DNN_Detection_input_height_base  320
#define g_DNN_Detection_hand_input_width   56
#define g_DNN_Detection_hand_input_height  96
#endif

#define Detect_Mode_Face 0
#define Detect_Mode_Hand 1

typedef struct FaceInfo {
    float x1;
    float y1;
    float x2;
    float y2;
    float score;
} FaceInfo;

int loadDetectDic();
int createDetectEngine(unsigned char* pMemp, int nMode = Detect_Mode_Face);
int releaseDetectEngine(int nMode = Detect_Mode_Face);
int getDetectMenSize();
int detect(unsigned char* imgBuffer, int imageWidth, int imageHeight, FaceInfo* face_list, int nMaxFaceNum, int* pn_facelist_cnt, unsigned char* pTempBuffer, int nDetectMode = Detect_Mode_Face);
int checkFace(unsigned char* img, int bufferWidth, int bufferHeight);

/*
 * pnRect:[0]-left, [1]-top, [2]-right, [3]-bottom
*/
void refineRect(float* pnRect, int nWidth, int nHeight);

#endif /* UltraFace_hpp */
