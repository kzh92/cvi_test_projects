#ifndef _GAMMACORRECTION_H_
#define _GAMMACORRECTION_H_


#ifdef __cplusplus
extern	"C"
{
#endif

void gammaCorrection1(unsigned char* pBuffer, int nWidth, int nHeight, float rGamma);
unsigned char getGammaCorrection(unsigned char bSrcValue);

#ifdef __cplusplus
}
#endif



#endif
