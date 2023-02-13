#include "manageIRCamera.h"
#include "FaceRetrievalSystem.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define LIMIT_INCREASE_RATE	1.5f
int g_nFaceFailed = 0;
//get init Exp
void setInitExp()
{
    int		iInitExps[10];
    int		iInitGains[10];

    iInitExps[0] = INIT_EXP_2;
    iInitExps[1] = INIT_EXP_3;
    iInitExps[2] = INIT_EXP_4;
    iInitExps[3] = INIT_EXP_5;
    iInitExps[4] = INIT_EXP_6;
    iInitExps[5] = INIT_EXP_7;
    iInitExps[6] = INIT_EXP_8;
    iInitExps[7] = INIT_EXP_9;
    iInitExps[8] = INIT_EXP_10;
    iInitExps[9] = INIT_EXP_11;

    iInitGains[0] = INIT_GAIN_2;
    iInitGains[1] = INIT_GAIN_3;
    iInitGains[2] = INIT_GAIN_4;
    iInitGains[3] = INIT_GAIN_5;
    iInitGains[4] = INIT_GAIN_6;
    iInitGains[5] = INIT_GAIN_7;
    iInitGains[6] = INIT_GAIN_8;
    iInitGains[7] = INIT_GAIN_9;
    iInitGains[8] = INIT_GAIN_10;
    iInitGains[9] = INIT_GAIN_11;

    //calc cur init exp
    int nCurEnv;
    int nStartEnvIndex = 0;
    int nEndEnvIndex = 0;
    int nEnvIndex;

    nCurEnv = fr_GetCurEnvForCameraControl();
    if (nCurEnv == 0)//dark
    {
        nStartEnvIndex = 0;
        nEndEnvIndex = 1;
    }
    else if (nCurEnv == 1)//indoor
    {
        nStartEnvIndex = 2;
        nEndEnvIndex = 5;
    }
    else//outdoor
    {
        nStartEnvIndex = 6;
        nEndEnvIndex = 9;
    }
    int nSelectedEnvIndex = -1;
    int nNewIndex = nStartEnvIndex;
    for (nEnvIndex = nStartEnvIndex; nEnvIndex <= nEndEnvIndex; nEnvIndex += 2)
    {
        if (*fr_GetExposure() == iInitExps[nEnvIndex] && *fr_GetGain() == iInitGains[nEnvIndex] &&
            *fr_GetExposure2() == iInitExps[nEnvIndex + 1] && *fr_GetGain2() == iInitGains[nEnvIndex + 1])
        {
            nSelectedEnvIndex = nEnvIndex;
        }
    }

    if (nSelectedEnvIndex == -1)
    {
        nNewIndex = nStartEnvIndex;
    }
    else if (nSelectedEnvIndex < nEndEnvIndex - 1)
    {
        nNewIndex = nSelectedEnvIndex + 1;
    }
    else
    {
        nNewIndex = nStartEnvIndex;
    }

    *fr_GetExposure() = iInitExps[nNewIndex];
    *fr_GetGain() = iInitGains[nNewIndex];
    *fr_GetExposure2() = iInitExps[nNewIndex + 1];
    *fr_GetGain2() = iInitGains[nNewIndex + 1];
}

float getGainRateFromGain(int nGain)
{
    float rGainRate = 1.0f;
    float rGainRate1, rGainRate2;
    rGainRate1 = (float)(nGain & 0xF) / 0x10 + 1.0f;
    rGainRate2 = pow(2.0f, ((nGain >> 4) & 0x7));

    rGainRate = rGainRate1 * rGainRate2;
    return rGainRate;
}



int getGainFromGainRate(float rGainRate, float rMaxScore)
{
    int nGain;
    if (rGainRate <= 1)
    {
        return 0;
    }
    if (rGainRate > rMaxScore)
    {
        rGainRate = rMaxScore;
    }
    int nPowNum = (int)(log(rGainRate) / log(2));
    if (nPowNum < 0)
    {
        nPowNum = 0;
    }

    float rRate1 = rGainRate / pow(2.0f, nPowNum);
    int nAlpaValue = (int)((rRate1 - 1) * 16);
    if (nAlpaValue < 0)
    {
        nAlpaValue = 0;
    }
    if (nAlpaValue > 15)
    {
        nAlpaValue = 15;
    }
    nGain = (nPowNum << 4) | nAlpaValue;

    return nGain;
}


void CalcNextExposure_inner()
{
    //changed by KSB 20180711
    int nNewGain, nNewExposure;

    if(!(*fr_GetFaceDetected()))
    {
        g_nFaceFailed ++;
        if(g_nFaceFailed >= DETECT_FAIL_COUNT)
        {
           setInitExp();

           g_nFaceFailed = 0;
        }

        return;
    }

    float rIncrease_rate_limit = LIMIT_INCREASE_RATE;

    if (fr_GetCurEnvForCameraControl())
    {
        rIncrease_rate_limit = 2.5f;
    }

    g_nFaceFailed = 0;
    int nAverageValueInDiff;

    //calc average values
    nAverageValueInDiff = 0;

    float rSaturatedRate = fr_GetSaturatedRate();

    int* pnAverageDiffImage = fr_GetAverageLedOnImage();
    int nAverageDiffImage = 0;
    if(pnAverageDiffImage)
        nAverageDiffImage = *pnAverageDiffImage;

    nAverageValueInDiff = nAverageDiffImage;

    float rDeltaValue = 1.0f;
    // float rGain2Step = 10.0f;
    float rMaxGainRate = 4.0f;
    //LOGE("AAA rSaturatedRate = %f", rSaturatedRate);

    if (nAverageValueInDiff < MIN_USER_LUM && rSaturatedRate < SAT_THRESHOLD)
    {
        //calc top level
        int nThresLevel = fr_GetBrightUpThresholdLevel();
        if (nThresLevel > 250)
        {
            nThresLevel = 250;
        }

        float rAvailableIncrease = ((float)250) / nThresLevel;
        if (rAvailableIncrease > rIncrease_rate_limit)
        {
            rAvailableIncrease = rIncrease_rate_limit;
        }
        rDeltaValue = (float)MIN_USER_LUM / nAverageValueInDiff;

        if (rDeltaValue > rAvailableIncrease)
        {
            rDeltaValue = rAvailableIncrease;
        }
    }
    else
    {
        if (rSaturatedRate > SAT_THRESHOLD)
        {
            float rProcessSaturatdRate;
            rProcessSaturatdRate = rSaturatedRate;

            if (rProcessSaturatdRate > 20.0f)
            {
                rProcessSaturatdRate = 20.0f;
            }

            rDeltaValue = 1.0f - rProcessSaturatdRate / (20.0f * 2.0f);
        }
        else if (nAverageValueInDiff > MAX_USER_LUM)
        {
            rDeltaValue = (float)MAX_USER_LUM / nAverageValueInDiff;
        }
    }

    float rExposureRate, rAvailableExposureRate;
    float rExposureRateUnderMid, rExposureRateUpperMid;
    float rAvailableGainRate;
    float rGainRate;

    int nMainControlCameraIndex = *fr_MainControlCameraIndex();

    int nOldGain, nOldExposure;

    if(nMainControlCameraIndex == 0)
    {
        nOldGain = *fr_GetGain();
        nOldExposure = *fr_GetExposure();
    }
    else
    {
        nOldGain = *fr_GetGain2();
        nOldExposure = *fr_GetExposure2();
    }
    nNewGain = nOldGain;
    nNewExposure = nOldExposure;

    *fr_GetExposure() = nNewExposure;
    *fr_GetGain() = nNewGain;
    *fr_GetExposure2() = nNewExposure;
    *fr_GetGain2() = nNewGain;

    //printf("CalcNextExposure_inner nOldExposure = %d\n", nOldExposure);
    //printf("CalcNextExposure_inner nOldGain = %d\n", nOldGain);

    if (rDeltaValue != 1.0f)
    {
        if (rDeltaValue > 1.0f)
        {
            rExposureRateUnderMid = 1;
            rExposureRateUpperMid = 1;

            rGainRate = 1;
            rAvailableGainRate = rMaxGainRate / getGainRateFromGain(nOldGain);
            rAvailableExposureRate = 1;

            rExposureRate = 1;

            if (nOldExposure < MID_EXP)
            {
                rExposureRateUnderMid = (float)MID_EXP / nOldExposure;
            }

            if (nOldExposure < MAX_EXP)
            {
                rAvailableExposureRate = (float)MAX_EXP / nOldExposure;
            }


            if (rDeltaValue <= rExposureRateUnderMid)
            {
                rExposureRate = rDeltaValue;
                rGainRate = 1.0f;
            }
            else
            {
                rGainRate = rDeltaValue / rExposureRateUnderMid;
                if (rGainRate < rAvailableGainRate)
                {
                    rExposureRate = rExposureRateUnderMid;
                }
                else
                {
                    rGainRate = rAvailableGainRate;
                    rExposureRateUpperMid = rDeltaValue / (rExposureRateUnderMid * rGainRate);
                    rExposureRate = rExposureRateUpperMid * rExposureRateUnderMid;
                }
             }

             if (rExposureRate > rAvailableExposureRate)
             {
                rExposureRate = rAvailableExposureRate;
             }
        }
        else
        {
            rExposureRateUnderMid = 1;
            rExposureRateUpperMid = 1;
            rExposureRate = 1;
            rGainRate = 1;
            rAvailableGainRate = 1.0f / getGainRateFromGain(nOldGain);

            rAvailableExposureRate = 1;
            rExposureRate = 1;

            if (nOldExposure > MID_EXP)
            {
                rExposureRateUpperMid = (float)MID_EXP / nOldExposure;
            }

            if (rDeltaValue > rExposureRateUpperMid)
            {
                rExposureRate = rDeltaValue;
                rGainRate = 1;
            }
            else
            {
                rGainRate = rDeltaValue / rExposureRateUpperMid;
                if (rGainRate > rAvailableGainRate)
                {
                    rExposureRate = rExposureRateUpperMid;
                }
                else
                {
                    rGainRate = rAvailableGainRate;
                    rExposureRateUnderMid = rDeltaValue / (rExposureRateUnderMid * rGainRate);
                    rExposureRate = rExposureRateUpperMid * rExposureRateUnderMid;
                }
            }

            if (nOldExposure > MIN_EXP)
            {
                rAvailableExposureRate = (float)MIN_EXP / nOldExposure;
            }

            if (rExposureRate < rAvailableExposureRate)
            {
                rExposureRate = rAvailableExposureRate;
            }
        }

        nNewExposure = (int)(rExposureRate * nOldExposure);
        float rNewGainRate = rGainRate * getGainRateFromGain(nOldGain);
        nNewGain = getGainFromGainRate(rNewGainRate, rMaxGainRate);

        if (nNewGain < MIN_GAIN)
        {
            nNewGain = MIN_GAIN;
        }
        if (nNewGain > MAX_GAIN)
        {
            nNewGain = MAX_GAIN;
        }
        if (nNewExposure > MAX_EXP)
        {
            nNewExposure = MAX_EXP;
        }
        if (nNewExposure < MIN_EXP)
        {
            nNewExposure = MIN_EXP;
        }

        *fr_GetExposure() = nNewExposure;
        *fr_GetGain() = nNewGain;
        *fr_GetExposure2() = nNewExposure;
        *fr_GetGain2() = nNewGain;
    }

    //printf("CalcNextExposure_inner nNewExposure = %d\n", nNewExposure);
    //printf("CalcNextExposure_inner nNewGain = %d\n", nNewGain);
}

void CalcNextExposure_ir_screen_inner(int nMode)
{
    int nNewGain, nNewExposure;

    float rDeltaValue = 1.0f;
    float rMaxGainRate = 8.0f;

    if(nMode == IR_SCREEN_GETIMAGE_MODE)
    {
        int* pnAverageLedOnImage = fr_GetAverageLedOnImage();
        int nAverageOnImage = 0;
        if(pnAverageLedOnImage)
            nAverageOnImage = *pnAverageLedOnImage;

        //printf("nAverageOnImage = %d\r\n", nAverageOnImage);
        if (nAverageOnImage < MIN_SCREEN_LUM)
        {
            //calc top level
            rDeltaValue = (float)MIN_SCREEN_LUM / nAverageOnImage;
        }
        else if (nAverageOnImage > MAX_SCREEN_LUM)
        {
            rDeltaValue = (float)MAX_SCREEN_LUM  / nAverageOnImage;
        }

    }
    else
    {
        float rSatration = fr_GetSaturatedRate();
        if (rSatration > 0.1)
        {
            rDeltaValue = 1.0f / (rSatration / 6  +  1.0f);
            if(rDeltaValue < 0.5f)
            {
                rDeltaValue = 0.5f;
            }
        }
        else
        {
            int nThresLevel = fr_GetBrightUpThresholdLevel();
            if (nThresLevel > 220)
            {
                nThresLevel = 220;
            }
            if(nThresLevel && nThresLevel < 160)
            {
                rDeltaValue = ((float)220 / ((float)nThresLevel));
            }
        }
    }


    float rExposureRate, rAvailableExposureRate;
    float rExposureRateUnderMid, rExposureRateUpperMid;
    float rAvailableGainRate;
    float rGainRate;

//    nNewGain = *fr_GetGain();
//    nNewExposure = *fr_GetExposure();


    if (rDeltaValue != 1.0f)
    {
        if (rDeltaValue > 1.0f)
        {
            rExposureRateUnderMid = 1;
            rExposureRateUpperMid = 1;

            rGainRate = 1;
            rAvailableGainRate = rMaxGainRate / getGainRateFromGain(*fr_GetGain());
            rAvailableExposureRate = 1;

            rExposureRate = 1;

            if (*fr_GetExposure() < MID_EXP)
            {
                rExposureRateUnderMid = (float)MID_EXP / *fr_GetExposure();
            }

            if (*fr_GetExposure() < MAX_EXP)
            {
                rAvailableExposureRate = (float)MAX_EXP / *fr_GetExposure();
            }


            if (rDeltaValue <= rExposureRateUnderMid)
            {
                rExposureRate = rDeltaValue;
                rGainRate = 1.0f;
            }
            else
            {
                rGainRate = rDeltaValue / rExposureRateUnderMid;
                if (rGainRate < rAvailableGainRate)
                {
                    rExposureRate = rExposureRateUnderMid;
                }
                else
                {
                    rGainRate = rAvailableGainRate;
                    rExposureRateUpperMid = rDeltaValue / (rExposureRateUnderMid * rGainRate);
                    rExposureRate = rExposureRateUpperMid * rExposureRateUnderMid;
                }
             }

             if (rExposureRate > rAvailableExposureRate)
             {
                rExposureRate = rAvailableExposureRate;
             }
        }
        else
        {
            rExposureRateUnderMid = 1;
            rExposureRateUpperMid = 1;
            rExposureRate = 1;
            rGainRate = 1;
            rAvailableGainRate = 1.0f / getGainRateFromGain(*fr_GetGain());

            rAvailableExposureRate = 1;
            rExposureRate = 1;

            if (*fr_GetExposure() > MID_EXP)
            {
                rExposureRateUpperMid = (float)MID_EXP / *fr_GetExposure();
            }

            if (rDeltaValue > rExposureRateUpperMid)
            {
                rExposureRate = rDeltaValue;
                rGainRate = 1;
            }
            else
            {
                rGainRate = rDeltaValue / rExposureRateUpperMid;
                if (rGainRate > rAvailableGainRate)
                {
                    rExposureRate = rExposureRateUpperMid;
                }
                else
                {
                    rGainRate = rAvailableGainRate;
                    rExposureRateUnderMid = rDeltaValue / (rExposureRateUnderMid * rGainRate);
                    rExposureRate = rExposureRateUpperMid * rExposureRateUnderMid;
                }
            }

            if (*fr_GetExposure() > MIN_EXP)
            {
                rAvailableExposureRate = (float)MIN_EXP / *fr_GetExposure();
            }

            if (rExposureRate < rAvailableExposureRate)
            {
                rExposureRate = rAvailableExposureRate;
            }
        }

        nNewExposure = (int)(rExposureRate * *fr_GetExposure());
        float rNewGainRate = rGainRate * getGainRateFromGain(*fr_GetGain());
        nNewGain = getGainFromGainRate(rNewGainRate, rMaxGainRate);

        if (nNewGain < MIN_GAIN)
        {
            nNewGain = MIN_GAIN;
        }
        if (nNewGain > MAX_SCREEN_GAIN)
        {
            nNewGain = MAX_SCREEN_GAIN;
        }
        if (nNewExposure > MAX_EXP)
        {
            nNewExposure = MAX_EXP;
        }
        if (nNewExposure < MIN_EXP)
        {
            nNewExposure = MIN_EXP;
        }

        *fr_GetExposure() = nNewExposure;
        *fr_GetGain() = nNewGain;
        *fr_GetExposure2() = nNewExposure;
        *fr_GetGain2() = nNewGain;
    }
}

void    InitIRCamera_ExpGain()
{
    g_nFaceFailed = 0;
    *fr_GetExposure() = INIT_EXP;
    *fr_GetGain() = INIT_GAIN;
    *fr_GetExposure2() = INIT_EXP_1;
    *fr_GetGain2() = INIT_GAIN_1;
}



