
#ifndef EsnDetection_hpp
#define EsnDetection_hpp

#include "esn.h"
#include "HAlign.h"

#define g_DNN_EyeCloseness_input_width   48
#define g_DNN_EyeCloseness_input_height  48

#if 0
//************************************
// Method:    init_eye_closeness
// Returns:   int						- if is 0, success, else fail
// Parameter: char * dict_path[in]		- eye closeness dict's path 
//************************************
int init_esn_detection(const char* dict_path, unsigned char* pbMemBuf = NULL);
int init_esn_detection(unsigned char* pbMemBuf = NULL);

//************************************
// Method:    deinit_eye_closeness
// Returns:   int						- value is 0
//************************************
int deinit_esn_detection();


//************************************
// Method:    get_eye_closeness
// Returns:   int -if is 0, CLOSE eye, else OPEN eye
// Parameter: unsigned char * img[in]	 - input image
// Parameter: int width[in]				 - width of input image
// Parameter: int height[in]			 - height of input image
// Parameter: float * landmark[in]		 - landmark point as modeling result(arm)
// Parameter: float * score[out]		 - output score
//************************************
int get_esn_detection(unsigned char* img, int width, int height, float* landmark, float* score, unsigned char* pTempBuffer);
#endif

#endif
