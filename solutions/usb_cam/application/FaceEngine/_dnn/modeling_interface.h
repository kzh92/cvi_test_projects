//
//  UltraFace.hpp
//  UltraFaceTest
//
//  Created by vealocia on 2019/10/17.
//  Copyright © 2019 vealocia. All rights reserved.
//

#ifndef modeling_interface_hpp
#define modeling_interface_hpp

#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#define g_DNN_Modeling_input_width   64
#define g_DNN_Modeling_input_height  64

int createModelingEngine(unsigned char* pMem);
/*
* pnFaceRect:[0]:reft, [1]:top, [2]:width, [3]:height
*/
int getFaceModelPoint(unsigned char* pImageBuffer, int nImageWidth, int nImageHeight, unsigned char* tempCropBuffer, float* prFaceRect, float* prLandmarkPoint);
int releaseModelingEngine();


#endif /* modeling_interface_hpp */
