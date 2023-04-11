#ifndef _DIC_MANAGE_H_INCLUDED_
#define _DIC_MANAGE_H_INCLUDED_

enum MachineFlagIndex
{
    MachineFlagIndex_DNN_Detect,
    MachineFlagIndex_DNN_Detect_Hand,
    MachineFlagIndex_DNN_Modeling,
    MachineFlagIndex_DNN_Modeling_Hand,
    MachineFlagIndex_DNN_Liveness_A1,
    MachineFlagIndex_DNN_Liveness_A2,
    MachineFlagIndex_DNN_Liveness_B,
    MachineFlagIndex_DNN_Liveness_B2,
    MachineFlagIndex_DNN_Liveness_C,
    MachineFlagIndex_DNN_CheckValid_Hand,
    MachineFlagIndex_DNN_Feature,
    MachineFlagIndex_DNN_Feature_Hand,
    MachineFlagIndex_DNN_ESN,
    MachineFlagIndex_DNN_OCC,
    MachineFlagIndex_H_1,
    MachineFlagIndex_H_2,
    MachineFlagIndex_MAX,
};
extern unsigned char* g_dic_detect;
extern unsigned char* g_dic_detect_hand;
extern unsigned char* g_dic_model;
extern unsigned char* g_dic_model_hand;
extern unsigned char*  g_dic_checkValid_hand;
extern unsigned char*  g_dic_live_a1;
extern unsigned char*  g_dic_live_a2;
extern unsigned char*  g_dic_live_b;
extern unsigned char*  g_dic_live_b2;
extern unsigned char*  g_dic_live_c;
extern unsigned char*  g_dic_occ;
extern unsigned char*  g_dic_esn;
extern unsigned char*  g_dic_feature;
extern unsigned char*  g_dic_H_1;
extern unsigned char*  g_dic_H_2;
extern unsigned char*  g_dic_feature_hand;

extern int g_nDicCheckSum_FEAT;
extern int g_nDicCheckSum_H_1;

int     loadMachineDic(int nMachineIndex);
int     releaseMachineDic(int nMachineIndex);

int getLoadedDicFlag(int nIndex);
int setLoadedDicFlag(int nIndex, int nValue);

int getDicChecSumChecked(int nIndex);

int     SetKdnnFeatData(unsigned char* abData);


#endif //_DIC_MANAGE_H_INCLUDED_
