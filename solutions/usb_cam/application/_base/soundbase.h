#ifndef SOUND_BASE_H
#define SOUND_BASE_H

#include "appdef.h"

#if (USE_WIFI_MODULE)
#include "settings.h"
enum SoundResNumbers
{
    SID_NONE,
    SID_WIFI_SOUND_BASE = 30,
    SID_END
};

void    SoundEnable(int iEnable);

void    PlaySound(int iSoundID, int iWait = 0);

#endif // USE_WIFI_MODULE

#endif // SOUND_BASE_H
