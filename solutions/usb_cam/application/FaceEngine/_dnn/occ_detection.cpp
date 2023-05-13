#include "occ_detection.h"
#include "dic_manage.h"
#include "common_types.h"

#if 0

Occlusion g_occ_detector;
extern int g_nStopEngine;


//unsigned char occ_img_ptr[g_DNN_OccDetection_input_width * g_DNN_OccDetection_input_height];
unsigned char m_nOccEngineLoaded = 0;

extern void APP_LOG(const char * format, ...);


int init_occ_detection(const char* dict_path, unsigned char* pbMemBuf)
{
    int ret = g_occ_detector.dnn_create(dict_path, pbMemBuf);
	return ret;
}

int init_occ_detection(unsigned char* pbMemBuf)
{
    if(!getLoadedDicFlag(MachineFlagIndex_DNN_OCC))
    {
        return 1;
    }
    if(m_nOccEngineLoaded)
    {
        return 0;
    }
    int nDicSize = g_occ_detector.dnn_dic_size();
    int ret = 1;
    if(g_dic_occ)
    {
        ret = g_occ_detector.dnn_create(g_dic_occ, nDicSize, pbMemBuf);
    }
    if(!ret)
    {
        m_nOccEngineLoaded = 1;
    }
    return ret;
}

int deinit_occ_detection()
{
    g_occ_detector.dnn_free();
    releaseMachineDic(MachineFlagIndex_DNN_OCC);
    m_nOccEngineLoaded = 0;
	return 0;
}

int get_occ_detection_arm(unsigned char* img, int width, int height, void* model_face_graph_ptr, float* score)
{
	unsigned char occ_img_ptr[g_DNN_OccDetection_input_width * g_DNN_OccDetection_input_height];
	Align_Vertical(img, width, height, occ_img_ptr, g_DNN_OccDetection_input_width, g_DNN_OccDetection_input_height,
        1, (void*)model_face_graph_ptr, 36, 32, 8);

    float* rScore = g_occ_detector.dnn_forward(occ_img_ptr);
	score[0] = rScore[0];
	score[1] = rScore[1];

	if (score[0] < score[1])
		return 1;
	return 0;
}

int get_occ_detection(unsigned char* img, int width, int height, float* landmark, float* score, unsigned char* pTempBuffer)
{
    if(!m_nOccEngineLoaded || !getDicChecSumChecked(MachineFlagIndex_DNN_OCC))
    {
        return 0;
    }
	if (g_nStopEngine)
	{
		return 0;
	}
    Align_Vertical_68(img, width, height, pTempBuffer, g_DNN_OccDetection_input_width, g_DNN_OccDetection_input_height,
        1, landmark, 36, 32, 8);

	if (g_nStopEngine)
	{
		return 0;
	}
    float* rScore = g_occ_detector.dnn_forward(pTempBuffer);
	if (g_nStopEngine)
	{
		return 0;
	}

	score[0] = rScore[0];
    score[1] = rScore[1];

    if (score[0] < score[1])
        return 1;
    return 0;
}
#endif