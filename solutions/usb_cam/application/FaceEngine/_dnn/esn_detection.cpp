#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "esn_detection.h"
#include <math.h>
#include "dic_manage.h"
#include "common_types.h"

int g_used_arm_modeling = false;
ESN g_esn_detector;
unsigned char m_nEsnEngineLoaded = 0;

extern void APP_LOG(const char * format, ...);
extern int g_nStopEngine;

void get_eye_align(unsigned char* img, int width, int height, unsigned char* eye_img, int eye_width, int eye_height, float* landmark, bool is_left)
{
    float padding_rate = 1.6;
    int left_eye_center_x, left_eye_center_y, left_eye_size;
    int right_eye_center_x, right_eye_center_y, right_eye_size;

    if (g_used_arm_modeling)
    {
        if (landmark[0] < landmark[2])
        {
            left_eye_center_x = landmark[0];
            left_eye_center_y = landmark[1];
            left_eye_size = (landmark[32] - landmark[36]) * padding_rate;

            right_eye_center_x = landmark[2];
            right_eye_center_y = landmark[3];
            right_eye_size = (landmark[38] - landmark[42]) * padding_rate;
        }
        else
        {
            left_eye_center_x = landmark[2];
            left_eye_center_y = landmark[3];
            left_eye_size = (landmark[42] - landmark[38]) * padding_rate;

            right_eye_center_x = landmark[0];
            right_eye_center_y = landmark[1];
            right_eye_size = (landmark[36] - landmark[32]) * padding_rate;
        }
    }
    else
    {
        left_eye_center_x = int((landmark[74] + landmark[76] + landmark[80] + landmark[82]) / 4);
        left_eye_center_y = int((landmark[75] + landmark[77] + landmark[81] + landmark[83]) / 4);
        left_eye_size = int((landmark[78] - landmark[72]) * padding_rate);

        right_eye_center_x = int((landmark[86] + landmark[88] + landmark[92] + landmark[94]) / 4);
        right_eye_center_y = int((landmark[87] + landmark[89] + landmark[93] + landmark[95]) / 4);
        right_eye_size = int((landmark[90] - landmark[84]) * padding_rate);
    }

    int eye_dist_x = right_eye_center_x - left_eye_center_x;
    int eye_dist_y = right_eye_center_y - left_eye_center_y;
    float eye_dist = sqrt(eye_dist_x * eye_dist_x + eye_dist_y * eye_dist_y);

    float org_eye_size, org_center_x, org_center_y;
    if (is_left)
    {
        org_center_x = left_eye_center_x;
        org_center_y = left_eye_center_y;
        org_eye_size = left_eye_size;
    }
    else
    {
        org_center_x = right_eye_center_x;
        org_center_y = right_eye_center_y;
        org_eye_size = right_eye_size;
    }

    double M[6] = { 0 };
    float scale = eye_width / org_eye_size;
    M[0] = eye_dist_x / eye_dist;
    M[1] = eye_dist_y / eye_dist;
    M[2] = (1 - M[0]) * org_eye_size / 2 - M[1] * org_eye_size / 2;
    M[3] = -M[1];
    M[4] = M[0];
    M[5] = M[1] * org_eye_size / 2 + (1 - M[0]) * org_eye_size / 2;

    double D = M[0] * M[4] - M[1] * M[3];
    D = D != 0 ? 1. / D : 0;
    double A11 = M[4] * D, A22 = M[0] * D;
    M[0] = A11; M[1] *= -D;
    M[3] *= -D; M[4] = A22;
    double b1 = -M[0] * M[2] - M[1] * M[5];
    double b2 = -M[3] * M[2] - M[4] * M[5];
    M[2] = b1; M[5] = b2;

    float offset_x = org_center_x - org_eye_size / 2;
    float offset_y = org_center_y - org_eye_size / 2;

    for (int y = 4; y < eye_height-4; y++)
    {
        for (int x = 4; x < eye_width-4; x++)
        {
            float org_rx = offset_x + M[0] * x / scale + M[1] * y / scale + M[2];
            float org_ry = offset_y + M[3] * x / scale + M[4] * y / scale + M[5];

            int org_ix = (int)org_rx;
            int org_iy = (int)org_ry;

            float rx_diff = org_rx - org_ix;
            float ry_diff = org_ry - org_iy;

            int index = org_iy * width + org_ix;
            eye_img[(y-4) * (eye_width-8) + x-4] = (unsigned char)(img[index] * (1 - rx_diff) * (1 - ry_diff) + img[index + 1] * rx_diff * (1 - ry_diff) +
                img[index + width] * (1 - rx_diff) * ry_diff + img[index + width + 1] * rx_diff * ry_diff);
        }
    }
}

int init_esn_detection(const char* dict_path, unsigned char* pbMemBuf)
{
    int ret = g_esn_detector.dnn_create(dict_path, pbMemBuf);

    return ret;
}


int init_esn_detection(unsigned char* pbMemBuf)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_ESN))
    {
        return 1;
    }
    if(m_nEsnEngineLoaded)
    {
        return 0;
    }

    int ret = 1;
    if(g_dic_esn)
    {
        int nDicSize = g_esn_detector.dnn_dic_size();
        ret = g_esn_detector.dnn_create(g_dic_esn, nDicSize, pbMemBuf);
        if(!ret)
        {
            m_nEsnEngineLoaded = 1;
        }
    }
	return ret;
}

int deinit_esn_detection()
{
    g_esn_detector.dnn_free();
    releaseMachineDic(MachineFlagIndex_DNN_ESN);
    m_nEsnEngineLoaded = 0;

	return 0;
}

int get_esn_detection(unsigned char* img, int width, int height, float* landmark, float* score, unsigned char* pTempBuffer)
{
    if(!m_nEsnEngineLoaded || !getDicChecSumChecked(MachineFlagIndex_DNN_ESN))
    {
        return 0;
    }
	// Left Eye Estimation
	if (g_nStopEngine)
	{
		return 0;
	}
    get_eye_align(img, width, height, pTempBuffer, g_DNN_EyeCloseness_input_width+8, g_DNN_EyeCloseness_input_height+8, landmark, true);
	if (g_nStopEngine)
	{
		return 0;
	}
    float* rScore = g_esn_detector.dnn_forward(pTempBuffer);
	if (g_nStopEngine)
	{
		return 0;
	}
	score[0] = rScore[0];
	score[1] = rScore[1];

	// Right Eye Estimation
    get_eye_align(img, width, height, pTempBuffer, g_DNN_EyeCloseness_input_width+8, g_DNN_EyeCloseness_input_height+8, landmark, false);
	if (g_nStopEngine)
	{
		return 0;
	}
    rScore = g_esn_detector.dnn_forward(pTempBuffer);
	if (g_nStopEngine)
	{
		return 0;
	}
	score[2] = rScore[0];
	score[3] = rScore[1];

	if ((score[0] < score[1]) || (score[2] < score[3]))
		return 1;
	return 0;
}
