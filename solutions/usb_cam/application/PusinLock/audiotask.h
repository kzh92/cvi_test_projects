#ifndef AUDIOTASK_H
#define AUDIOTASK_H

#include "appdef.h"

#if (USE_UAC_MODE)
void test_Audio();
#endif

#if (USE_WIFI_MODULE)

#include "thread.h"
#include "mutex.h"

class   MySpiThread;
void    StartAudioTask();
void    StopAudioTask();
void	test_Audio();
void 	StartWAVPlay(unsigned int id);

#endif // USE_WIFI_MODULE

#endif // AUDIOTASK_H
