#ifndef _ARMFEATUREEXTRACTOR_Fix_H_
#define _ARMFEATUREEXTRACTOR_Fix_H_

#include "armCommon.h"
#include "armFeatureMatcher.h"
///////////////Advanced_Fix////////////////////////

//ARM_HistoEqual
typedef struct _tagARM_HistoEqual
{
	ARM_Vec xHistoVec;
}ARM_HistoEqual;

//ARM_Cue
typedef struct _tagARM_Cue
{
	int                 pnFeature[250];
	int                 nFeatureSize;
}ARM_Cue;

//Arm_Gabor
typedef struct _tagARM_GABOR
{	
	short	m_pwAngKern[256];
	int		m_nAngKernSize;
	short	m_pwGaussKern[1024];
	short	m_pwGaussX[400];
	int		m_nGaussXSize;
	short	m_pwGaussY[400];
	int		m_nGaussYSize;
}ARM_GABOR;

//ARM_CueInfo
typedef struct _tagARM_CueInfo
{
	ARM_Point3D xPos;
	ARM_Point3D xSurface;
	float rK;
	float rFactor;
	float rSigma;
	float rRadius;
	float rAng;
	float rAxp;
	int	nLevels;
	int nSpin;
	float rLogPrec;
	float rScore;
	unsigned char	szbUnk[0x14];
}ARM_CueInfo;

//ARM_SubVecMap
typedef struct _tagARM_SubVecMap
{
	int nOffs;
}ARM_SubVecMap;

//ARM_CompactVec
typedef struct _tagARM_CompactVec
{
	int			nLen;
	float		rBwdFactor;
	unsigned short*		pwUnk;
	int			nSize1C;
}ARM_CompactVec;

//ARM_CompactMat
typedef struct _tagARM_CompactMat
{
	int						nWidth;
	int						nHeight;
	ARM_CompactVec			xComVecArr[90];
}ARM_CompactMat;

//ARM_CompactAlt
typedef struct _tagARM_CompactAlt
{
	ARM_CompactMat	xCompactMat;
	ARM_Vec			xVec;
}ARM_CompactAlt;

//ARM_PrjVecMap
typedef struct _tagARM_PrjVecMap
{
	ARM_CompactMat	xcSubSpaceMat;
	ARM_Vec			xcAdjVec;
}ARM_PrjVecMap;

//ARM_VecMapArrChnVecMap
typedef struct _tagARM_VecMapArrChnVecMap
{
	ARM_SubVecMap   xSubVecMap;
	ARM_CompactAlt  xCompactAlt;
	ARM_PrjVecMap   xPrjVecMap;
}ARM_VecMapArrChnVecMap;

//ARM_VecMapPrlArrVecMap
typedef struct _tagARM_VecMapPrlArrVecMap
{
	ARM_VecMapArrChnVecMap	xVecChnMapArr[180];
	int						m_nCount;
}ARM_VecMapPrlArrVecMap;

//ARM_AdvancedFvc
typedef struct _tagARM_AdvancedFvc
{	
	ARM_CueInfo		       pxCueInfoList[180];
	int				       nCueCnt;
	ARM_VecMapPrlArrVecMap xVecPrlMap;
	ARM_RBFMap2D		   xRBFMap;
	int                    nIndex25Arr[27];
	int	                   nIndex68Arr[27];
}ARM_AdvancedFvc;

//ARM_Quantizer
typedef struct _tagARM_Quantizer
{
	ARM_VecMapPrlArrVecMap		xPrlVecMap;
	int*						pnIdxArr;
	ARM_Vec						xVecTH;
}ARM_Quantizer;

/////////ARM_FeatureExtractor//////////////////////////////////////

void Extract_ByARM_FeatureExtractor(ARM_Face* pxFace, ARM_Vec* pxFeatureVec);
/////////////////////ARM_FeaAdvanced///////////////////////////////////////
//ARM_Cue
void Operator_Cue_Equal_ByARM_Cue(ARM_Cue* pDstCue, ARM_Cue* pSrcCue);

//ARM_HistoEqual
void Create_ByARM_HistoEqual(ARM_HistoEqual* pxDestHEqual);
void Process_ByARM_HistoEqual(ARM_HistoEqual* pxDestHEqual, ARM_Image* pxFace);

//Arm_Gabor
void Init_ByArm_Gabor(ARM_GABOR* pxGabor);

//ARM_CueInfo
int Read_ByARM_CueInfo(ARM_CueInfo* pxDest, int nIdx, int nFlag);
void Filp_ByARM_CueInfo(ARM_CueInfo* pxDest);
void Operator_CueInfo_Equal_ByARM_CueInfo(ARM_CueInfo* pxDesCueInfo, ARM_CueInfo* pxSrcCueInfo);

//ARM_SubVecMap
int Read_ByARM_SubVecMap(ARM_SubVecMap* pxDest, int nArrVMapIdx, int nflag);
void Sub(ARM_SubVecMap* pxSrc, ARM_Vec* pxInVec, ARM_Vec* pxOutVec);

//ARM_CompactVec
int Read_ByARM_CompactVec(ARM_CompactVec* pxDest, int nArrVMapIdx, int nVecIdx, int nVecCnt, int nflag);
float sub_10055360_ByARM_CompactVec(ARM_CompactVec* pxSrc, float* prData);
void Compact_ByARM_CompactVec(ARM_CompactVec* pxDest, ARM_Vec* pxInVec);

//ARM_CompactMat
int Read_ByARM_CompactMat(ARM_CompactMat* pxDestMat, int nArrVMapIdx, int nflag);
void Map_ByARM_CompactMat(ARM_CompactMat* pxSrctMat, ARM_Vec* pxInVec, ARM_Vec* pxOutVec);
/*void sub_1004BC20_ByARM_CompactMat( ARM_Vec*  pxArg_0, ARM_Vec* pxArg_4, ARM_Vec* pxArg_8 );*/

//ARM_CompactAlt
int	Read_ByARM_CompactAlt(ARM_CompactAlt* pxDestAlt, int nArrVMapIdx, int nflag);
void Map_ByARM_CompactAlt( ARM_CompactAlt* pxSrcAlt, ARM_Vec* pxInVec, ARM_Vec* pxOutVec );

//ARM_PrjVecMap
int Read_ByARM_PrjVecMap( ARM_PrjVecMap* pxDestPrj, int nflag);
void Map_ByARM_PrjVecMap( ARM_PrjVecMap* pxSrcPrj, ARM_Vec* pxInVec, ARM_Vec* pxOutVec);

//ARM_VecMapArrChnVecMap
int	Read_ByARM_VecMapArrChnVecMap(ARM_VecMapArrChnVecMap* pxDestChnMap, int nArrMapIdx, int nflag);
void Map_ByARM_VecMapArrChnVecMap(ARM_VecMapArrChnVecMap* pxSrcChnMap, ARM_Vec* pxInVec, ARM_Vec* pxOutVec, int nflag);

//ARM_VecMapPrlArrVecMap
int Read_ByARM_VecMapPrlArrVecMap(ARM_VecMapPrlArrVecMap* pxDestPrlMap, int nflag);
void Map_ByARM_VecMapPrlArrVecMap(ARM_VecMapPrlArrVecMap* pxSrcPrlMap, ARM_Vec* pxInVec, ARM_Vec* pxOutVec, int nflag );	

//ARM_AdvancedFvc
int LoadDict_ByARM_AdvancedFvc(ARM_AdvancedFvc* pxDestAdvancedFvc, int nflag);
void GetJet_ByARM_AdvancedFvc(ARM_RBFMap2D* pxRBFMap2D, ARM_Image* pxFace, ARM_CueInfo* pxCueInfo, ARM_LinearTransform3D* pxTransform, ARM_Vec* pxFeatureVec);
void ExtractFeature_ByARM_AdvancedFvc(ARM_AdvancedFvc* pxSrcAdvancedFvc, ARM_Face* pxFace, ARM_Vec* pxFeatureVec, int nNo);

//ARM_Quantizer
int Read_ByARM_Quantizer(ARM_Quantizer* pxQt, int nflag);
int	Quantize_ByARM_Quantizer(ARM_Quantizer* pxQtize, unsigned char* pmProcessImg, int nCol, int nRow);
void Map_ByARM_Quantizer( ARM_Quantizer* pxQtize, ARM_Vec* pxInVec, ARM_Cue* pxOutCue, int nflag );

//Other
void Normalize(ARM_Vec* pxVec);
void Process_ByARM_Histogram(ARM_Image* pxFace, ARM_Vec* pxOutVec, int nflag);
ARM_CueInfo GetFlip(ARM_CueInfo* pxCueInfoList, int index);
int	Rounds(float r);
void sub_1011E2E0( ARM_Point3D* pECX,  ARM_GABOR* arg_0, float* arg_4, unsigned char* pECX_22C, int nRows, int nCols, ARM_Point2D* arg_C, float arg_10, float arg_14 );
int sub_100FC0E0(float* arg_4, float arg_8 );

#ifdef __cplusplus
extern "C"{
#endif
	int Create_ByARM_FeatureExtractor(void);
	void ExtractFeature_ByARM_FeatureExtractor(ARM_Face* pxFace, ARM_Feature* pxFeature);
	
#ifdef __cplusplus
}
#endif
#endif // _ARMFEATUREEXTRACTOR_Fix_H_	
