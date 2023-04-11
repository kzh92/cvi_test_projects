#ifndef _KDNN_ENGINE_INTERFACE_H_
#define _KDNN_ENGINE_INTERFACE_H_

/* Define Constant*/
#define	 KDNN_SUCCESS			1
#define  KDNN_FAILED			0
#define	 KDNN_NOVALUE			-1

int     KdnnCreateLivenessEngine_2DA1(unsigned char* pMem);
int     KdnnCreateLivenessEngine_2DA2(unsigned char* pMem);
int     KdnnCreateLivenessEngine_2DB(unsigned char* pMem);
int     KdnnCreateLivenessEngine_2DB2(unsigned char* pMem);
int     KdnnCreateLivenessEngine_3D(unsigned char* pMem);

float   KdnnDetectLiveness2D_A(unsigned char * pbImage);
float KdnnDetectLiveness_2D_B(unsigned char * pbImage);
float KdnnDetectLiveness_2D_B2(unsigned char * pbImage);
float KdnnDetectLiveness_3D(unsigned char * pbImage);
void generateAlignImageForLiveness(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBufAC, unsigned char* pDstBufB, unsigned char* pDstBufB2, float* landmark_ptr);

int     KdnnFreeEngine_liveness();
int     cacheLivenessDic();



// idvcd15_1.4.3
// #define KDNN_FEAT_CHANNEL   3
// #define KDNN_FEAT_SIZE      256
// #define KDNN_FEAT_ALIGN_W   80
// #define KDNN_FEAT_ALIGN_H   80
// #define KDNN_FEAT_DIST_V    33
// #define KDNN_FEAT_POS_X     40
// #define KDNN_FEAT_POS_Y     22

//irn-stemV3
#define KDNN_FEAT_CHANNEL   3
#define KDNN_FEAT_SIZE      256
#define KDNN_FEAT_ALIGN_W   128
#define KDNN_FEAT_ALIGN_H   128
#define KDNN_FEAT_DIST_V    48
#define KDNN_FEAT_POS_X     64
#define KDNN_FEAT_POS_Y     40

// nir2.3
//#define KDNN_FEAT_CHANNEL   3
//#define KDNN_FEAT_SIZE      128
//#define KDNN_FEAT_ALIGN_W   64
//#define KDNN_FEAT_ALIGN_H   64
//#define KDNN_FEAT_DIST_V    26
//#define KDNN_FEAT_POS_X     32
//#define KDNN_FEAT_POS_Y     18

#define TOTAL_ENROLL_MAX_DNNFEATURE_COUNT	8
#define INIT_ENROLL_DNNFEATURE_COUNT        3
#define TOTAL_ENROLL_DNNFEATURE_SIZE        0x2000

int     KdnnCreateEngine_feat(unsigned char* pMem, int nMode = 0);
int     KdnnDetect_feat(unsigned char * pbImage, unsigned short* prFeatArray, int nMode);

int     KdnnFreeEngine_feat(int nMode);
void    generateAlignImageForFeature(unsigned char* pSrcBuf, int nSrcWidth, int nSrcHeight, unsigned char* pDstBuf, float* landmark_ptr);
int     getFeatEngineLoaded(int nMode);

float   KdnnGetSimilarity(const unsigned short* imgData1, const unsigned short* imgData2);


#endif
