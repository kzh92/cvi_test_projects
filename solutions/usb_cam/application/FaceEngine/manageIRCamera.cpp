#include "manageIRCamera.h"
#include "FaceRetrievalSystem.h"
#include "FaceRetrievalSystem_base.h"
#include "EngineDef.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define LIMIT_INCREASE_RATE	1.5f
int g_nFaceFailed = 0;


#include "HandRetrival_.h"
int g_nHandFailed = 0;

//static int g_exposure = 0;
//static int g_nGain = 0;
//static int g_nFineGain = 0;
//static int g_exposure2 = 0;
//static int g_nGain2 = 0;
//static int g_nFineGain2 = 0;


//get init Exp
void setInitExp()
{
    int		iInitExps[14];
    int		iInitGains[14];
    int		iInitFineGains[14];

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
    iInitExps[10] = INIT_EXP_12;
    iInitExps[11] = INIT_EXP_13;
    iInitExps[12] = INIT_EXP_14;
    iInitExps[13] = INIT_EXP_15;

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
    iInitGains[10] = INIT_GAIN_12;
    iInitGains[11] = INIT_GAIN_13;
    iInitGains[12] = INIT_GAIN_14;
    iInitGains[13] = INIT_GAIN_15;

    iInitFineGains[0] = INIT_FINEGAIN_2;
    iInitFineGains[1] = INIT_FINEGAIN_3;
    iInitFineGains[2] = INIT_FINEGAIN_4;
    iInitFineGains[3] = INIT_FINEGAIN_5;
    iInitFineGains[4] = INIT_FINEGAIN_6;
    iInitFineGains[5] = INIT_FINEGAIN_7;
    iInitFineGains[6] = INIT_FINEGAIN_8;
    iInitFineGains[7] = INIT_FINEGAIN_9;
    iInitFineGains[8] = INIT_FINEGAIN_10;
    iInitFineGains[9] = INIT_FINEGAIN_11;
    iInitFineGains[10] = INIT_FINEGAIN_12;
    iInitFineGains[11] = INIT_FINEGAIN_13;
    iInitFineGains[12] = INIT_FINEGAIN_14;
    iInitFineGains[13] = INIT_FINEGAIN_15;

    //calc cur init exp
    int nCurEnv;
    int nStartEnvIndex = 0;
    int nEndEnvIndex = 0;
    int nEnvIndex;

    nCurEnv = fr_GetCurEnvForCameraControl();
    int nSelectedEnvIndex = -1;
    int nNewIndex = 0;

    int nFaceCameraCount = 2;

#if (ENGINE_USE_TWO_CAM != 1)
    nFaceCameraCount = 1;
#else//ENGINE_USE_TWO_CAM == 1
#if (N_MAX_HAND_NUM) && (HAND_VERIFY_PRIORITY == HAND_VERIFY_PRIORITY_HIGH)
    nFaceCameraCount = 1;
#endif
#endif

    if(nFaceCameraCount == 2)
    {
        if (nCurEnv == 0)//dark
        {
            nStartEnvIndex = 0;
            nEndEnvIndex = 3;
        }
        else if (nCurEnv == 1)//indoor
        {
            nStartEnvIndex = 4;
            nEndEnvIndex = 7;
        }
        else//outdoor
        {
            nStartEnvIndex = 8;
            nEndEnvIndex = 13;
        }
        for (nEnvIndex = nStartEnvIndex; nEnvIndex <= nEndEnvIndex; nEnvIndex += 2)
        {
            if (*fr_GetExposure_bkup() == iInitExps[nEnvIndex] && *fr_GetGain_bkup() == iInitGains[nEnvIndex] &&
                *fr_GetExposure2_bkup() == iInitExps[nEnvIndex + 1] && *fr_GetGain2_bkup() == iInitGains[nEnvIndex + 1])
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
            nNewIndex = nSelectedEnvIndex + 2;
        }
        else
        {
            nNewIndex = nStartEnvIndex;
        }
        *fr_GetExposure() = iInitExps[nNewIndex];
        *fr_GetGain() = iInitGains[nNewIndex];
        *fr_GetFineGain() = iInitFineGains[nNewIndex];
        *fr_GetExposure2() = iInitExps[nNewIndex + 1];
        *fr_GetGain2() = iInitGains[nNewIndex + 1];
        *fr_GetFineGain2() = iInitFineGains[nNewIndex + 1];
    }
    else//one camera control
    {
        if (nCurEnv == 0)//dark
        {
            nStartEnvIndex = 0;
            nEndEnvIndex = 3;
        }
        else if (nCurEnv == 1)//indoor
        {
            nStartEnvIndex = 4;
            nEndEnvIndex = 7;
        }
        else//outdoor
        {
            nStartEnvIndex = 8;
            nEndEnvIndex = 11;
        }

        for (nEnvIndex = nStartEnvIndex; nEnvIndex <= nEndEnvIndex; nEnvIndex ++)
        {
            if (*fr_GetExposure_bkup() == iInitExps[nEnvIndex] && *fr_GetGain_bkup() == iInitGains[nEnvIndex])
            {
                nSelectedEnvIndex = nEnvIndex;
            }
        }
        if (nSelectedEnvIndex == -1)
        {
            nNewIndex = nStartEnvIndex;
        }
        else if (nSelectedEnvIndex < nEndEnvIndex)
        {
            nNewIndex = nSelectedEnvIndex + 1;
        }
        else
        {
            nNewIndex = nStartEnvIndex;
        }
        *fr_GetExposure() = iInitExps[nNewIndex];
        *fr_GetGain() = iInitGains[nNewIndex];
        *fr_GetFineGain() = iInitFineGains[nNewIndex];

#if (N_MAX_HAND_NUM) && (HAND_VERIFY_PRIORITY == HAND_VERIFY_PRIORITY_HIGH)
        *fr_GetExposure2() = INIT_HAND_EXP;
        *fr_GetGain2() = INIT_HAND_GAIN;
        *fr_GetFineGain2() = INIT_HAND_FINEGAIN;
#endif
    }
}

#define SC2355_COARSE_GAIN_COUNT    5
int g_nSC2355_COARSE_GAIN_TABLE[SC2355_COARSE_GAIN_COUNT][2] =
{
    {0x00,1},
    {0x01,2},
    {0x03,4},
    {0x07,8},
    {0x0F,16},
};

float getGainRateFromGain_SC2355(int nCoarseGain, int nFineGain)
{
    float rCoarseGainRate, rFineGainRate;
    float rGainRate = 1.0f;

    int nCourseGainIndex = 0;
    int nIndex;
    for(nIndex = 0; nIndex < SC2355_COARSE_GAIN_COUNT; nIndex ++)
    {
        if(g_nSC2355_COARSE_GAIN_TABLE[nIndex][0] == nCoarseGain)
        {
            nCourseGainIndex = nIndex;
            break;
        }
    }

    rCoarseGainRate = g_nSC2355_COARSE_GAIN_TABLE[nCourseGainIndex][1];
    if(nFineGain < 0x80)
    {
        nFineGain = 0x80;
    }
    if(nFineGain > 0xfe)
    {
        nFineGain = 0xfe;
    }

    rFineGainRate = (float)nFineGain / 0x80;
    rGainRate = rCoarseGainRate * rFineGainRate;
    return rGainRate;
}

int getGainFromGainRate_SC2355(float rGainRate, float rMaxScore, int &nCoarseGain, int &nFineGain)
{
    if (rGainRate <= 1)
    {
        rGainRate = 1;
    }
    if (rGainRate > rMaxScore)
    {
        rGainRate = rMaxScore;
    }

    int nPowIndex = (int)(log(rGainRate) / log(2));
    if (nPowIndex < 0)
    {
        nPowIndex = 0;
    }

    nCoarseGain = g_nSC2355_COARSE_GAIN_TABLE[nPowIndex][0];
    float rRate1 = rGainRate / pow(2.0f, nPowIndex);
    nFineGain = (int)(rRate1 * 0x80);
    if(nFineGain < 0x80)
    {
        nFineGain = 0x80;
    }
    if(nFineGain > 0xfe)
    {
        nFineGain = 0xfe;
    }
    return 0;
}

#define SC2355_CONTROL_MILSTONE_COUNT   5
float SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT][2] =
{
    {MIN_EXP,1.0f},
    {MID_EXP,1.0f},
    {MID_EXP,4.0f},
    {MAX_EXP,4.0f},
    {MAX_EXP,8.0f},
};

void CalcNextExposure_inner()
{
    //changed by KSB 20180711
    int nNewGain, nNewFineGain, nNewExposure;

    if(!(*fr_GetFaceDetected()))
    {
        g_nFaceFailed ++;
        if(g_nFaceFailed >= DETECT_FAIL_COUNT)
        {
           setInitExp();
           g_nFaceFailed = 0;
           g_nNeedToCalcNextExposure = 1;
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
    float rMaxGainRate = 8.0f;
    //LOGE("AAA rSaturatedRate = %f", rSaturatedRate);

    if (nAverageValueInDiff < MIN_USER_LUM && rSaturatedRate < SAT_THRESHOLD)
    {
        //calc top level
        int nThresLevel = fr_GetBrightUpThresholdLevel();

        nThresLevel++;
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
            rDeltaValue = ((float)(MAX_USER_LUM + MIN_USER_LUM) / 2)/ nAverageValueInDiff;
        }
    }

    float rGainRate = 1.0;

    int nMainControlCameraIndex = *fr_MainControlCameraIndex();

    int nOldGain, nOldExposure, nOldFineGain;

    if(nMainControlCameraIndex == 0)
    {
        nOldGain = *fr_GetGain_bkup();
        nOldExposure = *fr_GetExposure_bkup();
        nOldFineGain = *fr_GetFineGain_bkup();
    }
    else
    {
        nOldGain = *fr_GetGain2_bkup();
        nOldExposure = *fr_GetExposure2_bkup();
        nOldFineGain = *fr_GetFineGain2_bkup();
    }

    nNewGain = nOldGain;
    nNewExposure = nOldExposure;
    nNewFineGain = nOldFineGain;

    //printf("rDeltaValue = %f\n", rDeltaValue);
    //printf("org exp = %d, Gain = %d %d\n", nNewExposure, nNewGain, nNewFineGain);

    *fr_GetExposure() = nNewExposure;
    *fr_GetGain() = nNewGain;
    *fr_GetFineGain() = nNewFineGain;
    *fr_GetExposure2() = nNewExposure;
    *fr_GetGain2() = nNewGain;
    *fr_GetFineGain2() = nNewFineGain;

    if (rDeltaValue != 1.0f)
    {
        float rCurExpGainValue, rNewExpGainValue;

        rCurExpGainValue = (float)(nOldExposure) * getGainRateFromGain_SC2355(nNewGain, nOldFineGain);
        rNewExpGainValue = rCurExpGainValue * rDeltaValue;

        if(rNewExpGainValue < (float)SC2355_CONTROL_MILSTONE[0][0] * SC2355_CONTROL_MILSTONE[0][1])
        {
            rGainRate = SC2355_CONTROL_MILSTONE[0][1];
            nNewExposure = (int)(SC2355_CONTROL_MILSTONE[0][0]);
        }
        else if(rNewExpGainValue > (float)SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][0] * SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][1])
        {
            rGainRate = SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][1];
            nNewExposure = (int)(SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][0]);
        }
        else
        {
            int nMileStoneIndex, nSelectedIndex;
            nSelectedIndex = 0;
            for(nMileStoneIndex = 0; nMileStoneIndex < SC2355_CONTROL_MILSTONE_COUNT - 1; nMileStoneIndex ++)
            {
                if(rNewExpGainValue >= (float)SC2355_CONTROL_MILSTONE[nMileStoneIndex][0] * SC2355_CONTROL_MILSTONE[nMileStoneIndex][1] &&
                   rNewExpGainValue < (float)SC2355_CONTROL_MILSTONE[nMileStoneIndex + 1][0] * SC2355_CONTROL_MILSTONE[nMileStoneIndex + 1][1])
                {
                    nSelectedIndex = nMileStoneIndex;
                }
            }

            float rDeltaRate = rNewExpGainValue / ((float)SC2355_CONTROL_MILSTONE[nSelectedIndex][0] * SC2355_CONTROL_MILSTONE[nSelectedIndex][1]);
            nNewExposure = (int)((float)SC2355_CONTROL_MILSTONE[nSelectedIndex][0] * rDeltaRate);
            if(nNewExposure > (int)SC2355_CONTROL_MILSTONE[nSelectedIndex + 1][0])
            {
                nNewExposure = (int)SC2355_CONTROL_MILSTONE[nSelectedIndex + 1][0];
            }
            rGainRate = rNewExpGainValue / nNewExposure;
        }
        getGainFromGainRate_SC2355(rGainRate, rMaxGainRate, nNewGain, nNewFineGain);

        if (nNewGain < MIN_GAIN)
        {
            nNewGain = MIN_GAIN;
        }
        if (nNewGain > MAX_GAIN)
        {
            nNewGain = MAX_GAIN;
        }
        if (nNewFineGain < MIN_FINEGAIN)
        {
            nNewFineGain = MIN_FINEGAIN;
        }
        if (nNewFineGain > MAX_FINEGAIN)
        {
            nNewFineGain = MAX_FINEGAIN;
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
        *fr_GetFineGain() = nNewFineGain;
        *fr_GetExposure2() = nNewExposure;
        *fr_GetGain2() = nNewGain;
        *fr_GetFineGain2() = nNewFineGain;
        g_nNeedToCalcNextExposure = 1;
    }
    else
    {
        if(*fr_GetExposure() != *fr_GetExposure_bkup() || *fr_GetExposure2() != *fr_GetExposure2_bkup() ||
           *fr_GetGain() != *fr_GetGain_bkup() || *fr_GetGain2() != *fr_GetGain2_bkup() ||
            *fr_GetFineGain() != *fr_GetFineGain_bkup() || *fr_GetFineGain2() != *fr_GetFineGain2_bkup())
        {
            g_nNeedToCalcNextExposure = 1;
        }

    }
}

#if (N_MAX_HAND_NUM)
void CalcNextExposure_inner_hand()
{
    //changed by KSB 20180711
    int nNewGain, nNewFineGain, nNewExposure;

    if(!(*fr_GetHandDetected()) && !(*fr_GetFaceDetected()))
    {
        g_nHandFailed ++;
        if(g_nHandFailed >= DETECT_FAIL_COUNT)
        {
            setInitExp();
            g_nHandFailed = 0;
            g_nNeedToCalcNextExposure = 1;
        }

        return;
    }
    if(!(*fr_GetHandDetected()) && (*fr_GetFaceDetected()))
    {
        //g_nNeedToCalcNextExposure = 0;
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

    float rSaturatedRate = fr_GetSaturatedRate_Hand();

    int* pnAverageDiffImage = fr_GetAverageLedOnImage_Hand();
    int nAverageDiffImage = 0;
    if(pnAverageDiffImage)
        nAverageDiffImage = *pnAverageDiffImage;

    nAverageValueInDiff = nAverageDiffImage;

    float rDeltaValue = 1.0f;
    float rMaxGainRate = 8.0f;
    int nForceDownForSaturation = 0;

    //LOGE("AAA rSaturatedRate = %f", rSaturatedRate);
    //printf("CalcNextExposure_inner_hand nAverageValueInDiff %d rSaturatedRate %f\n", nAverageValueInDiff, rSaturatedRate);
    if (nAverageValueInDiff < MIN_USER_LUM_HAND && rSaturatedRate < SAT_THRESHOLD)
    {
        //calc top level
        int nThresLevel = fr_GetBrightUpThresholdLevel_Hand();

        nThresLevel++;
        if (nThresLevel > 250)
        {
            nThresLevel = 250;
        }

        float rAvailableIncrease = ((float)250) / nThresLevel;
        if (rAvailableIncrease > rIncrease_rate_limit)
        {
            rAvailableIncrease = rIncrease_rate_limit;
        }
        rDeltaValue = (float)MIN_USER_LUM_HAND / nAverageValueInDiff;

        if (rDeltaValue > rAvailableIncrease)
        {
            rDeltaValue = rAvailableIncrease;
        }
    }
    else
    {
        if (rSaturatedRate > SAT_THRESHOLD)
        {
            if(rSaturatedRate > 50 && ((float)(*fr_GetExposure_bkup()) * getGainRateFromGain_SC2355(*fr_GetGain_bkup(), *fr_GetFineGain_bkup())) > 300)
            {
                nForceDownForSaturation = 1;
            }
            else
            {
                if(rSaturatedRate > 90)
                {
                    rDeltaValue = 1.0f / 8;
                }
                else if(rSaturatedRate > 70)
                {
                    rDeltaValue = 1.0f / 6.0f;
                }
                else if(rSaturatedRate > 50)
                {
                    rDeltaValue = 1.0f / 3.5f;
                }
                else
                {
                    float rProcessSaturatdRate;
                    rProcessSaturatdRate = rSaturatedRate;

                    if (rProcessSaturatdRate > 20.0f)
                    {
                        rProcessSaturatdRate = 20.0f;
                    }

                    rDeltaValue = 1.0f - rProcessSaturatdRate / (20.0f * 2.0f);
                }
            }
        }
        else if (nAverageValueInDiff > MAX_USER_LUM_HAND)
        {
            rDeltaValue = ((float)(MAX_USER_LUM_HAND + MIN_USER_LUM_HAND) / 2)/ nAverageValueInDiff;
        }
    }

    float rGainRate = 1.0;

    int nMainControlCameraIndex = *fr_MainControlCameraIndex();

    int nOldGain, nOldExposure, nOldFineGain;

    if(nMainControlCameraIndex == 0)
    {
        nOldGain = *fr_GetGain_bkup();
        nOldExposure = *fr_GetExposure_bkup();
        nOldFineGain = *fr_GetFineGain_bkup();
    }
    else
    {
        nOldGain = *fr_GetGain2_bkup();
        nOldExposure = *fr_GetExposure2_bkup();
        nOldFineGain = *fr_GetFineGain2_bkup();
    }

    nNewGain = nOldGain;
    nNewExposure = nOldExposure;
    nNewFineGain = nOldFineGain;

    //printf("rDeltaValue = %f\n", rDeltaValue);
    //printf("org exp = %d, Gain = %d %d\n", nNewExposure, nNewGain, nNewFineGain);

    *fr_GetExposure() = nNewExposure;
    *fr_GetGain() = nNewGain;
    *fr_GetFineGain() = nNewFineGain;
    *fr_GetExposure2() = nNewExposure;
    *fr_GetGain2() = nNewGain;
    *fr_GetFineGain2() = nNewFineGain;
    g_nNeedToCalcNextExposure = 0;
    //printf("CalcNextExposure_inner_hand rDeltaValue = %f\n", rDeltaValue);
    if (rDeltaValue != 1.0f || nForceDownForSaturation)
    {
        if(rDeltaValue != 1.0f)
        {
            float rCurExpGainValue, rNewExpGainValue;

            rCurExpGainValue = (float)(nOldExposure) * getGainRateFromGain_SC2355(nOldGain, nOldFineGain);
            rNewExpGainValue = rCurExpGainValue * rDeltaValue;

            if(rNewExpGainValue < (float)SC2355_CONTROL_MILSTONE[0][0] * SC2355_CONTROL_MILSTONE[0][1])
            {
                rGainRate = SC2355_CONTROL_MILSTONE[0][1];
                nNewExposure = (int)(SC2355_CONTROL_MILSTONE[0][0]);
            }
            else if(rNewExpGainValue > (float)SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][0] * SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][1])
            {
                rGainRate = SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][1];
                nNewExposure = (int)(SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][0]);
            }
            else
            {
                int nMileStoneIndex, nSelectedIndex;
                nSelectedIndex = 0;
                for(nMileStoneIndex = 0; nMileStoneIndex < SC2355_CONTROL_MILSTONE_COUNT - 1; nMileStoneIndex ++)
                {
                    if(rNewExpGainValue >= (float)SC2355_CONTROL_MILSTONE[nMileStoneIndex][0] * SC2355_CONTROL_MILSTONE[nMileStoneIndex][1] &&
                       rNewExpGainValue < (float)SC2355_CONTROL_MILSTONE[nMileStoneIndex + 1][0] * SC2355_CONTROL_MILSTONE[nMileStoneIndex + 1][1])
                    {
                        nSelectedIndex = nMileStoneIndex;
                    }
                }

                float rDeltaRate = rNewExpGainValue / ((float)SC2355_CONTROL_MILSTONE[nSelectedIndex][0] * SC2355_CONTROL_MILSTONE[nSelectedIndex][1]);
                nNewExposure = (int)((float)SC2355_CONTROL_MILSTONE[nSelectedIndex][0] * rDeltaRate);
                if(nNewExposure > (int)SC2355_CONTROL_MILSTONE[nSelectedIndex + 1][0])
                {
                    nNewExposure = (int)SC2355_CONTROL_MILSTONE[nSelectedIndex + 1][0];
                }
                rGainRate = rNewExpGainValue / nNewExposure;
            }
            getGainFromGainRate_SC2355(rGainRate, rMaxGainRate, nNewGain, nNewFineGain);
        }
        else//nForceDownForSaturation
        {
            nNewGain = 0x00;
            nNewFineGain = 0x80;
            nNewExposure = 75;
            
#if (ENGINE_LENS_TYPE == ENGINE_LENS_M277_2409)
            int nHandWidth = *getHandWidth();
            if(nHandWidth < 700)
            {
                nNewExposure = (int)(710.0f  * 75 / nHandWidth);
            }
#endif
        }

        if (nNewGain < MIN_GAIN)
        {
            nNewGain = MIN_GAIN;
        }
        if (nNewGain > MAX_GAIN)
        {
            nNewGain = MAX_GAIN;
        }
        if (nNewFineGain < MIN_FINEGAIN)
        {
            nNewFineGain = MIN_FINEGAIN;
        }
        if (nNewFineGain > MAX_FINEGAIN)
        {
            nNewFineGain = MAX_FINEGAIN;
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
        *fr_GetFineGain() = nNewFineGain;
        *fr_GetExposure2() = nNewExposure;
        *fr_GetGain2() = nNewGain;
        *fr_GetFineGain2() = nNewFineGain;
        g_nNeedToCalcNextExposure = 1;
    }
    else
    {
        if(*fr_GetExposure() != *fr_GetExposure_bkup() || *fr_GetExposure2() != *fr_GetExposure2_bkup() ||
           *fr_GetGain() != *fr_GetGain_bkup() || *fr_GetGain2() != *fr_GetGain2_bkup() ||
            *fr_GetFineGain() != *fr_GetFineGain_bkup() || *fr_GetFineGain2() != *fr_GetFineGain2_bkup())
        {
            g_nNeedToCalcNextExposure = 1;
        }
    }
}
#endif //N_MAX_HAND_NUM

void CalcNextExposure_ir_screen_inner(int nMode)
{
    int nNewGain, nNewFineGain, nNewExposure;

    float rDeltaValue = 1.0f;
    float rMaxGainRate = 8.0f;
    // float rExposureRate, rAvailableExposureRate;
    // float rExposureRateUnderMid, rExposureRateUpperMid;
    // float rAvailableGainRate;

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


    float rGainRate;

    nNewGain = *fr_GetGain();
    nNewExposure = *fr_GetExposure();
    nNewFineGain = *fr_GetFineGain();

    if (rDeltaValue != 1.0f)
    {
        float rCurExpGainValue, rNewExpGainValue;

        rCurExpGainValue = (float)(*fr_GetExposure()) * getGainRateFromGain_SC2355(*fr_GetGain(), *fr_GetFineGain());
        rNewExpGainValue = rCurExpGainValue * rDeltaValue;

        if(rNewExpGainValue < (float)SC2355_CONTROL_MILSTONE[0][0] * SC2355_CONTROL_MILSTONE[0][1])
        {
            rGainRate = SC2355_CONTROL_MILSTONE[0][1];
            nNewExposure = (int)(SC2355_CONTROL_MILSTONE[0][0]);
        }
        else if(rNewExpGainValue > (float)SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][0] * SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][1])
        {
            rGainRate = SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][1];
            nNewExposure = (int)(SC2355_CONTROL_MILSTONE[SC2355_CONTROL_MILSTONE_COUNT - 1][0]);
        }
        else
        {
            int nMileStoneIndex, nSelectedIndex;
            nSelectedIndex = 0;
            for(nMileStoneIndex = 0; nMileStoneIndex < SC2355_CONTROL_MILSTONE_COUNT - 1; nMileStoneIndex ++)
            {
                if(rNewExpGainValue >= (float)SC2355_CONTROL_MILSTONE[nMileStoneIndex][0] * SC2355_CONTROL_MILSTONE[nMileStoneIndex][1] &&
                   rNewExpGainValue < (float)SC2355_CONTROL_MILSTONE[nMileStoneIndex + 1][0] * SC2355_CONTROL_MILSTONE[nMileStoneIndex + 1][1])
                {
                    nSelectedIndex = nMileStoneIndex;
                }
            }

            float rDeltaRate = rNewExpGainValue / ((float)SC2355_CONTROL_MILSTONE[nSelectedIndex][0] * SC2355_CONTROL_MILSTONE[nSelectedIndex][1]);
            nNewExposure = (int)((float)SC2355_CONTROL_MILSTONE[nSelectedIndex][0] * rDeltaRate);
            if(nNewExposure > (int)SC2355_CONTROL_MILSTONE[nSelectedIndex + 1][0])
            {
                nNewExposure = (int)SC2355_CONTROL_MILSTONE[nSelectedIndex + 1][0];
            }
            rGainRate = rNewExpGainValue / nNewExposure;
        }
        getGainFromGainRate_SC2355(rGainRate, rMaxGainRate, nNewGain, nNewFineGain);

        if (nNewGain < MIN_GAIN)
        {
            nNewGain = MIN_GAIN;
        }
        if (nNewGain > MAX_GAIN)
        {
            nNewGain = MAX_GAIN;
        }
        if (nNewFineGain < MIN_FINEGAIN)
        {
            nNewFineGain = MIN_FINEGAIN;
        }
        if (nNewFineGain > MAX_FINEGAIN)
        {
            nNewFineGain = MAX_FINEGAIN;
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
        *fr_GetFineGain() = nNewFineGain;
        *fr_GetExposure2() = nNewExposure;
        *fr_GetGain2() = nNewGain;
        *fr_GetFineGain2() = nNewFineGain;
    }
}

void    InitIRCamera_ExpGain()
{
    g_nFaceFailed = 0;
    *fr_GetExposure() = INIT_EXP;
    *fr_GetGain() = INIT_GAIN;
    *fr_GetFineGain() = INIT_FINEGAIN;

    *fr_GetExposure2() = INIT_EXP_1;
    *fr_GetGain2() = INIT_GAIN_1;
    *fr_GetFineGain2() = INIT_FINEGAIN_1;

}

void    BackupIRCamera_ExpGain()
{

}


