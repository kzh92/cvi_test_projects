

#include "soundbase.h"
#include "appdef.h"
#include "settings.h"
#include "playthread.h"
#include "drv_gpio.h"
#include "i2cbase.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#if (USE_WIFI_MODULE)
int g_iSoundEnabled = 0;

void SoundEnable(int iEnable)
{
    if(iEnable == 1)
    {
        if(g_iSoundEnabled == 0)
        {
            g_iSoundEnabled = 1;
            GPIO_fast_config(AUDIO_EN, OUT);
        }
    }
    else
    {
        if(g_iSoundEnabled == 1)
        {
            GPIO_fast_setvalue(AUDIO_EN, OFF);
            g_iSoundEnabled = 0;
        }
    }
}

void PlaySound(int iSoundID, int iWait)
{
    if(g_xSS.iNoSoundPlayFlag == 0)
        PlayThread::PlaySound(iSoundID, iWait, DEFAULT_SOUND);

    g_xSS.iNoSoundPlayFlag = 0;
}

#define DECL_SOUND_FUNC_WITH_WAIT(func_name, soundNo) \
void func_name(int iWait) \
{ \
    if(g_xSS.iNoSoundPlayFlag == 0) \
        PlayThread::PlaySound(soundNo, iWait, DEFAULT_SOUND); \
    g_xSS.iNoSoundPlayFlag = 0; \
}

#endif // USE_WIFI_MODULE