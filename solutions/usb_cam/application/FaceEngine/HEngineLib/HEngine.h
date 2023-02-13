#ifndef __ENGINE_INTERFACE_H__
#define __ENGINE_INTERFACE_H__
#include "Htypes.h"

#ifdef __cplusplus
extern "C" {
#endif

    int LoadHEngine(unsigned char* pbDictData);
    int LoadHEngine1(unsigned char* pbDictData1, unsigned char* pbDictData2);
	int ReleaseHEngine();
	int getAllFeatures(HImage orgImage, unsigned char* pEnrollFeature, unsigned char* pVerifyFeature);
	int getEnrollFeatures(HImage orgImage, unsigned char* pEnrollFeature);
	int getVerifyFeatures(HImage orgImage, unsigned char* pVerifyFeature);
	int getFaceModel(HImage *orgImage, HModelResult* pModelResult);
	int verify_feature_extract(HImage orgImage, HModelResult* modelResult, _u8* featureData);
	int enroll_feature_extract(HImage orgImage, HModelResult* modelResult, _u8* featureData);
	int calcSimilarityScoreOneToOne(_u8* pEnrollFeature, _u8* pVerifyFeature);
	int getFaceRects(HImage* pImage, HModelResult* pModelResult);
    _s32 generateHDetectionFromRect(FaceDetetResult* result_detect, _s32* pnRect);

	int FaceModeling(HImage hImage, FaceDetetResult* detect_result, HModelResult *modeling_result, _u8 *bSuccess);
	int FaceDetect(HImage* hImage, FaceDetetResult* detect_result, int nMode = 0);
	int getBestFace(FaceDetetResult* detect_result, int* pnRetRects);
	int getAverageValueInFace(int nLeft, int nTop, int nRight, int nBottom);
    int getGlassesEquipted();

	_s32 calcSimilaryScore(_u8** ppbEnrollFeature, _s32 nEnrollFeatureCount, _u8* pbVerifyFeature ,_s32 *result);
	_s32 calcSimilaryScore_changable_Size(_u8** ppbEnrollFeature, _s32* pnEnrollFeatureCount, _s32 nEnrollPersonCount, _u8* pbVerifyFeature ,_s32 *result);
	_s32 calcSimilaryScore_changable_Size_FeatureIndex(_u8** ppbEnrollFeature, _s32* pnEnrollFeatureCount, _s32 nEnrollPersonCount, _u8* pbVerifyFeature, _s32 *result, _s32 *pnMatchedFeatureIndex);
    _s32 calcSimilaryScore_changable_Size_EnrollFeatureToEnrollFeature(_u8** ppbEnrollFeature, _s32* pnEnrollFeatureCount, _s32 nEnrollPersonCount, _u8* pbFeatureToEnroll, _s32 nFeatureToEnrollCount,_s32 *result);
	int refineEnrollFeature(_u8* pPureEnrollFeature, _s32 nPureEnrollFeatureSize, _u8* pRefinedEnrollFeature);
	int updateEnrollFeature(_u8* pEnrollFeatureToUpdate, _u8* pVerifyFeatureToUpdate);
	int updateEnrollFeature_sizeChanged(_u8* pEnrollFeatureToUpdate, int* pnEnrolledFeatureCount, _u8* pVerifyFeatureToUpdate);
	int updateEnrollFeatureByEnrollFeature(_u8* pEnrollFeatureToUpdate, int* pnEnrolledFeatureCount, _u8* pEnrolledFeatureByUpdate);
    int setStopFlag_h(int nStopFlag);

#ifdef __cplusplus
}
#endif

#endif
