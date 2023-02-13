
#include "engineparam.h"

#include <memory.h>


ENGINE_PARAM g_xEP = { 0 };

void ResetEngineParams()
{
    memset(&g_xEP, 0, sizeof(g_xEP));

    g_xEP.iFps = IR_FPS;

    g_xEP.iInitClrExp = INIT_CLR_EXP;           //(900)//0x384
    g_xEP.iInitClrGain = INIT_CLR_GAIN;         // (22)//0x16
    g_xEP.iMinClrExp = MIN_CLR_EXP;             //(10)
    g_xEP.iMaxClrExp = MAX_CLR_EXP;             //(2000)//0x7D0
    g_xEP.iMidClrExp = MID_CLR_EXP;             //(1300)//
    g_xEP.iMaxClrGain = MAX_CLR_GAIN;           //(57)//0x39
    g_xEP.iMinClrGain = MIN_CLR_GAIN;           //(22)//0x16
}
