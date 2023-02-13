#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include "appdef.h"

#if (USE_WIFI_MODULE)
#include "thread.h"
#include "mutex.h"

#define SOUND_PLAY_FINISHED 1


class PlayThread : public Thread
{
public:
    PlayThread();

    static PlayThread* GetInstance();
    static void PlaySound(int iSoundID, int iWait, int iSoundVolume);
    static void WaitForFinished();
    static void StopSound();
    static void IncCounter();
    static int  GetCounter();

    void    Play(int iSoundID, int iSoundVolume);
    void    Stop();

protected:
    void    run();
    
private:


    int     m_iRunning;
    int     m_iSoundID;
};
#endif // USE_WIFI_MODULE

#endif // PLAYTHREAD_H
