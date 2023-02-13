
#define CAMERA_NO_ERROR		0
#define CAMERA_ERROR		1
#define CAMERA_MIDDLE_ERROR		2

int checkCameraPattern(unsigned char *pbCameraPatternBuffer);
//nMode=0:YYUV 1, YUYV 
int checkCameraPattern_BF3A03_0x80_clr(unsigned char *pbClrCameraPatternBuffer, unsigned char* pbRGB_Buffer, int nMode = 0);

//0x8c:0x1C, 0x8d:0x01
int checkCameraPattern_GC2145_clr(unsigned char *pbClrCameraPatternBuffer);
