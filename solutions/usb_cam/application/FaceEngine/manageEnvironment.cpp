#include "manageEnvironment.h"

#define TableSize 2

float g_nTableRage[TableSize][2] =
{
    { 0, 19},//0
    {19, 10000}
};

int getEnvRageFromValue(int rAveOffValue, int nExp)
{
    int nRet = 0;
    float rEnvAve = 0;
    if (nExp)
    {
        rEnvAve = (float)rAveOffValue * 100 / nExp;
    }

	int nRangeIndex;

	for (nRangeIndex = 0; nRangeIndex < TableSize; nRangeIndex++)
	{
		if (rEnvAve >= g_nTableRage[nRangeIndex][0] && rEnvAve < g_nTableRage[nRangeIndex][1])
		{
			nRet = nRangeIndex;
            break;
		}
	}

	if (nRet >= TableSize)
	{
		nRet = TableSize - 1;
	}

//    printf("rEnvAve = %f", rEnvAve);
	return nRet;
}

#define CameraConrtolEnvTableSize 3

float g_nTableRageCameraControl[CameraConrtolEnvTableSize][2] =
{
    { 0, 68 },//0
	{ 68, 230 },//0
	{ 230, 1000000 }
};

int getCameraControlEnvRageFromValue(int rAveOffValue, int nExp)
{
    int nRet = 0;
    float rEnvAve = 0;
    if (nExp)
    {
        rEnvAve = (float)rAveOffValue * 1000 / nExp;
    }

    int nRangeIndex;

	for (nRangeIndex = 0; nRangeIndex < CameraConrtolEnvTableSize; nRangeIndex++)
    {
        if (rEnvAve >= g_nTableRageCameraControl[nRangeIndex][0] && rEnvAve < g_nTableRageCameraControl[nRangeIndex][1])
        {
            nRet = nRangeIndex;
            break;
        }
    }

	if (nRet >= CameraConrtolEnvTableSize)
    {
		nRet = CameraConrtolEnvTableSize - 1;
    }

    //    printf("rEnvAve = %f", rEnvAve);
    return nRet;
}




