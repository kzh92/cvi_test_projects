#ifndef __FACE_RETRIEVAL_SYSTEM_H_H__
#define __FACE_RETRIEVAL_SYSTEM_H_H__

void    fr_InitEngine_h();
int     fr_PreExtractFace_h(unsigned char *pbClrImage, unsigned char *pbLedOnImage);
int     fr_ExtractFace_h(unsigned char *pbBayerFromCamera2);
int     fr_RegisterFace_h(int iFaceDir);
int     fr_Retrieval_h();
int     fr_calc_Off_h(unsigned char *pbLedOffImage);
void    fr_LoadFeatureForDuplicateCheck_h(int nUpdateID);
int     fr_GetRegisteredFeatInfo_h(void* pFeatInfo);
void    fr_LoadFeatEngine();

#endif//__FACE_RETRIEVAL_SYSTEM_H_H__
