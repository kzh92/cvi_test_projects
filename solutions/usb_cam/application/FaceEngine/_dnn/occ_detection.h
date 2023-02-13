#ifndef OccDetection_hpp
#define OccDetection_hpp

#include "HAlign.h"
#include "occ.h"
#include <stdlib.h>


#define g_DNN_OccDetection_input_width   64
#define g_DNN_OccDetection_input_height  64

//************************************
// Method:    init_occ_detection
// Returns:   int						- if is 0, success, else fail
// Parameter: char * dict_path[in]		- occ detection dict's path 
//************************************
int init_occ_detection(const char* dict_path, unsigned char* pbMemBuf = NULL);
int init_occ_detection(unsigned char* pbMemBuf = NULL);

//************************************
// Method:    deinit_occ_detection
// Returns:   int						- value is 0
//************************************
int deinit_occ_detection();


//************************************
// Method:    get_occ_detection
// Returns:   int -if is 0, occlusion, else normal state
// Parameter: unsigned char * img[in]	 - input image
// Parameter: int width[in]				 - width of input image
// Parameter: int height[in]			 - height of input image
// Parameter: void * landmark[in]		 - landmark point as modeling result(arm)
// Parameter: float * score[out]		 - output score
//************************************
int get_occ_detection_arm(unsigned char* img, int width, int height, void* model_face_graph_ptr, float* score);

//************************************
// Method:    get_occ_detection_dnn
// Returns:   int -if is 0, occlusion, else normal state
// Parameter: unsigned char * img[in]	 - input image
// Parameter: int width[in]				 - width of input image
// Parameter: int height[in]			 - height of input image
// Parameter: void * landmark[in]		 - landmark point as modeling result(arm)
// Parameter: float * score[out]		 - output score
//************************************
int get_occ_detection(unsigned char* img, int width, int height, float* landmark, float* score, unsigned char* pTempBuffer);



#endif
