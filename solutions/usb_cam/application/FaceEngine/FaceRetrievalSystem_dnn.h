#ifndef __FACE_RETRIEVAL_SYSTEM_DNN_H__
#define __FACE_RETRIEVAL_SYSTEM_DNN_H__


void    fr_InitEngine_dnn();
int     fr_PreExtractFace_dnn(unsigned char *pbClrImage, unsigned char *pbLedOnImage);
int		fr_PreExtractFace2_dnn(unsigned char *pbBayerFromCamera2);
int     fr_ExtractFace_dnn();
int     fr_RegisterFace_dnn(int iFaceDir);
int     fr_Retrieval_dnn();
int     fr_calc_Off_dnn(unsigned char *pbLedOffImage);
void    fr_LoadFeatureForDuplicateCheck_dnn(int nUpdateID);
void    generateAlignImageForLiveness_dnn();
int     fr_GetRegisteredFeatInfo_dnn(void* pFeatInfo);
int     fr_RevertEnrollStep_dnn();

#endif//__FACE_RETRIEVAL_SYSTEM_DNN_H__
