#ifndef __D_UTIL__
#define __D_UTIL__

#include "type.h"

#include <iostream>
#include <sstream>
#include <cstdint>
#include <vector>
using namespace std;


//const bool DEBUG = true;
const bool fDEBUG = false;

//debug output
extern ostringstream sout;

template < class T >
string toString(const T &arg)
{
	ostringstream	out;

	out << arg;

	return (out.str());
}

ostream& tab(ostream& output);

void printMatrix(double *in, int m, int n);
void printMatrix(float *in, int m, int n);
void fprintMatrix(FILE* pFile, double *in, int m, int n);//matrix must be m * n matrix

void transposeAndFlipY(double *in, int m, int n, double *out);

void transpose(float *in, int m, int n, float *out);
void transpose(double *in, int m, int n, double* out);

void transpose3dim(unsigned char *image, int gWidth , int gHeight,unsigned char *imageOutput);
void transpose3dimBGR(unsigned char *image, int gWidth , int gHeight,unsigned char *imageOutput);

void getOpenGLMatrices(double *A, double *R, double *T, int width, int height, double mv[16],
		double projectionMatrix[16]);

void getCameraMatricesFromOpenGL(double *A, double *R, double *T, int width, int height,
		double mv[16], double projectionMatrix[16]);


void**	AllocDoubleArray ( int nRowNum, int nColumnNum, short sItemSize );
void	FreeDoubleArray ( void** ppvBuffer);

int getIntFromChar(char *strNum);

void unitalizaVector(double* pVector, int nSize);

double norm(double* pVector, int nSize, double* pVector2 = 0, int nFlag = 0);

double projectVetor(double* pSrcVector, double* rStandardUnitVector, int nSize);

double* MulMatrix(double *pMatrixA, double *pMatrixB, int nM, int nN, int nL); //martixA: nM*nN, matrixB:nN * nL

#define GEMM_A_T 1
#define GEMM_B_T 2
//C = alpha * A * beta * B, A: M * N matrix, B: N * L matrix, C: M * L matrix 
void	MulMat(void *pMatrixA, void *pMatrixB, void *pMatrixC, int nM, int nN, int nL, MAT_TYPE nType, double rAlpha = 1.0f, double rBeta = 1.0, int nFlag = 0);

//C = alpha * A + beta * B
void	AddMat(void *pMatrixA, void *pMatrixB, void *pMatrixC, int nM, int nN, MAT_TYPE nType, double rAlpha = 1.0f, double rBeta = 1.0);


//Get Avrage Value in each channel. nM: Number of Pixel, nN: number of Channel, pAvgValue: Average Value [4]
void	AvgMat(void *pMatrix, int nM, int nN, MAT_TYPE nType, double* pAvgValue);

//Get Transpose Mat , pSrcMat: M * N matrix, pDesMat: N * M matrix
void	TransposeMat(void* pSrcMat, void* pDesMat, int nM, int nN, MAT_TYPE nType);

//Calc Desmat = (pSrcMat - pDelta) * (pSrcMat - pDelta)',(nOrder = 0) or (pSrcMat - pDelta)' * (pSrcMat - pDelta)(nOrder = 1)
void	MulTransposedMat(void* pSrcMat, int nM, int nN, void* pDesMat, int nOrder, MAT_TYPE nType, double* pDelta = NULL);

//Set Mat To Identy
void	setIdentyMat(void* pSrcMat, int nRow, int nCol, MAT_TYPE nType);
//Scale mat
void	ScaleMat(void* pSrcMat, void* pDesMat, int nRow, int nCol, double rScale, MAT_TYPE nType);

//Set Scalar to Matrix
void	setScalarMat(void* pSrcMat, int nRow, int nCol, double rScalar, MAT_TYPE nType, unsigned char* pMask = 0);

double detOfMat(void* pNormalMat, int nRows, int Col, MAT_TYPE nTYPE);

void completeSymm(char* _m, int nRows, int nCols, int nElementSize, bool LtoR = false);

//Vector1, Vector2, Vector3 must be 1 * 3 vector
void crossProduct(void* pVector1, void* pVector2, void* pVector3, MAT_TYPE nType);

//get Col
void getCol(void* pSrc, int nRow, int nCol, void* pDst, int nStartCol, int nEndCol = 0, MAT_TYPE nType = TYPE_DOUBLE);

//set col
void setCol(void* pDst, int nRow, int nCol, void* pSrc, int nStartCol, int nEndCol, MAT_TYPE nType = TYPE_DOUBLE);


size_t alignSize(size_t sz, int n);

int getElementNumInFile(FILE* pFile, int nSizeofElement);

void GetGaussianFilter(float* rFilter, int nSize, float rSigma);

float dotProduct(float* prVector1, float* prVector2, int nVectorSize);

unsigned char getValueFromBuffer(unsigned char* pBuffer, int nWidth, int nHeight, float rX, float rY);

#endif
