#ifndef ENGINE_PARAM_H
#define ENGINE_PARAM_H

#include "engine_inner_param.h"

// gc2145
#define INIT_CLR_EXP        0x300
#define INIT_CLR_GAIN       0x40

#define MIN_CLR_EXP         0xA0
#define MAX_CLR_EXP         0xFA0
#define MID_CLR_EXP         0x9C0
#define MAX_CLR_GAIN        0x80
#define MIN_CLR_GAIN        0x10

#define INIT_CLR_EXP_1      0x200
#define INIT_CLR_GAIN_1     0x50
#define INIT_CLR_EXP_2      0x4F0
#define INIT_CLR_GAIN_2     0x40

#define ENGINE_TYPE         2
#define LED_STATUS          2        //flick
#define IR_FPS              30



#define REMOVE_GLASS        0
#define REMOVE_NOISE        0
#define BORDER_X_DIVIDOR    (6.0)
#define BORDER_Y_DIVIDOR    (6.0)

typedef struct _tagENGINE_PARAM
{
    int     iFps;

    int		iInitClrExp;    //#define INIT_CLR_EXP        (900)//0x384
    int		iInitClrGain;   //#define INIT_CLR_GAIN       (22)//0x16
    int		iMinClrExp;     //#define MIN_CLR_EXP         (10)
    int		iMaxClrExp;     //#define MAX_CLR_EXP         (2000)//0x7D0
    int		iMidClrExp;     //#define MID_CLR_EXP         (1300)//
    int		iMaxClrGain;    //#define MAX_CLR_GAIN        (57)//0x39
    int		iMinClrGain;    //#define MIN_CLR_GAIN        (22)//0x16
} ENGINE_PARAM;

void ResetEngineParams();

extern ENGINE_PARAM g_xEP;

#endif // BASE_H
