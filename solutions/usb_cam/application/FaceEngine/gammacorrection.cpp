#include "gammacorrection.h"
//#include <math.h>
//#include <stdio.h>

//gamma = 0.6f
/*
unsigned char bGammaValue[256] =
{
	0, 6, 10, 13, 16, 19, 21, 23, 25, 28,
	30, 32, 33, 35, 37, 39, 41, 42, 44, 45,
	47, 49, 50, 52, 53, 55, 56, 57, 59, 60,
	62, 63, 64, 66, 67, 68, 70, 71, 72, 73,
	75, 76, 77, 78, 79, 81, 82, 83, 84, 85,
	87, 88, 89, 90, 91, 92, 93, 94, 95, 97,
	98, 99, 100, 101, 102, 103, 104, 105, 106, 107,
	108, 109, 110, 111, 112, 113, 114, 115, 116, 117,
	118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 131, 132, 133, 134, 135, 136,
	137, 138, 139, 140, 141, 141, 142, 143, 144, 145,
	146, 147, 148, 149, 149, 150, 151, 152, 153, 154,
	155, 155, 156, 157, 158, 159, 160, 160, 161, 162,
	163, 164, 165, 165, 166, 167, 168, 169, 170, 170,
	171, 172, 173, 174, 174, 175, 176, 177, 178, 178,
	179, 180, 181, 182, 182, 183, 184, 185, 185, 186,
	187, 188, 189, 189, 190, 191, 192, 192, 193, 194,
	195, 195, 196, 197, 198, 198, 199, 200, 201, 201,
	202, 203, 204, 204, 205, 206, 207, 207, 208, 209,
	209, 210, 211, 212, 212, 213, 214, 215, 215, 216,
	217, 217, 218, 219, 220, 220, 221, 222, 222, 223,
	224, 225, 225, 226, 227, 227, 228, 229, 229, 230,
	231, 232, 232, 233, 234, 234, 235, 236, 236, 237,
	238, 238, 239, 240, 240, 241, 242, 242, 243, 244,
	244, 245, 246, 247, 247, 248, 249, 249, 250, 251,
	251, 252, 253, 253, 254, 255,
};
*/

//gamma = 0.7
unsigned char bGammaValue1[256] =
{
	0, 5, 8, 11, 13, 16, 18, 20, 22, 24,
	26, 28, 30, 31, 33, 35, 36, 38, 39, 41,
	42, 44, 45, 47, 48, 50, 51, 52, 54, 55,
	57, 58, 59, 60, 62, 63, 64, 66, 67, 68,
	69, 70, 72, 73, 74, 75, 76, 78, 79, 80,
	81,82,83,84,86,87,88,89,90,91,
		92,93,94,95,96,97,99,100,101,102,
		103,104,105,106,107,108,109,110,111,112,
		113,114,115,116,117,118,119,120,121,122,
		123,123,124,125,126,127,128,129,130,131,
		132,133,134,135,136,137,137,138,139,140,
		141,142,143,144,145,146,146,147,148,149,
		150,151,152,153,153,154,155,156,157,158,
		159,159,160,161,162,163,164,165,165,166,
		167,168,169,170,170,171,172,173,174,175,
		175,176,177,178,179,179,180,181,182,183,
		184, 184, 185, 186, 187, 188, 188, 189, 190, 191,
		191, 192, 193, 194, 195, 195, 196, 197, 198, 199,
		199, 200, 201, 202, 202, 203, 204, 205, 206, 206,
		207, 208, 209, 209, 210, 211, 212, 212, 213, 214,
		215, 215, 216, 217, 218, 218, 219, 220, 221, 221,
		222, 223, 224, 224, 225, 226, 227, 227, 228, 229,
		229, 230, 231, 232, 232, 233, 234, 235, 235, 236,
		237, 237, 238, 239, 240, 240, 241, 242, 242, 243,
		244, 245, 245, 246, 247, 247, 248, 249, 250, 250,
		251, 252, 252, 253, 254, 255,
};

unsigned char getGammaCorrection(unsigned char bSrcValue)
{
    return bGammaValue1[bSrcValue];
}

void gammaCorrection1(unsigned char* pBuffer, int nWidth, int nHeight, float rGamma)
{

#if 1
	int nImageSize = nWidth * nHeight;
	int nBufferIndex;

	for (nBufferIndex = 0; nBufferIndex < nImageSize; nBufferIndex++)
	{
		unsigned char bOrgValue, bNewValue;
		bOrgValue = pBuffer[nBufferIndex];
        bNewValue = bGammaValue1[bOrgValue];
		pBuffer[nBufferIndex] = bNewValue;
	}
#endif
	
#if 0
	int nImageSize = nWidth * nHeight;
	int nBufferIndex;

	for (nBufferIndex = 0; nBufferIndex < nImageSize; nBufferIndex++)
	{
		unsigned char bOrgValue, bNewValue;
		bOrgValue = pBuffer[nBufferIndex];
		float rOrgValue = (float)bOrgValue / 255;

		float rNewValue = pow(rOrgValue, rGamma);
		
		if (rNewValue > 255)
		{
			rNewValue = 255;
		}

		bNewValue = (unsigned char)(rNewValue * 255);
		
//		bNewValue = bGammaValue[bOrgValue];
		pBuffer[nBufferIndex] = bNewValue;
	}
#endif


#if 0

	int nOrgValue;
	
	FILE* pFile = fopen("E:\\1.txt", "wb");

	for (nOrgValue = 0; nOrgValue <= 255; nOrgValue ++)
	{
		float rOrgValue = (float)nOrgValue / 255;
		float rNewValue = pow(rOrgValue, rGamma);

		if (rNewValue > 255)
		{
			rNewValue = 255;
		}
		fprintf(pFile, "%d,", (int)(rNewValue * 255));
		if ((nOrgValue + 1) % 10 == 0)
		{
			fprintf(pFile, "\r\n");
		}
	}
	fclose(pFile);

#endif


	return;
}






