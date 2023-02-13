#include "playthread.h"

#if (USE_WIFI_MODULE)

#include "shared.h"
#include "mutex"
#include "drv_gpio.h"
#include "i2cbase.h"
#include "soundbase.h"
#include "uartcomm.h"
#include "msg.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

PlayThread* g_pPlayInstance = NULL;
mymutex_ptr   g_xPlayMutex = 0;
int     g_iSoundCounter = 0;

PlayThread::PlayThread()
{
    m_iRunning = 0;
    g_xPlayMutex = my_mutex_init();
}

void PlayThread::PlaySound(int iSoundID, int iWait, int iSoundVolume)
{
    my_mutex_lock(g_xPlayMutex);
    PlayThread* pPlayThread = PlayThread::GetInstance();
    pPlayThread->Stop();

    pPlayThread->Play(iSoundID, iSoundVolume);
    if(iWait)
        pPlayThread->Wait();
    my_mutex_unlock(g_xPlayMutex);
}

void PlayThread::IncCounter()
{
    g_iSoundCounter ++;
}

int PlayThread::GetCounter()
{
    return g_iSoundCounter;
}


PlayThread* PlayThread::GetInstance()
{
    if(g_pPlayInstance == NULL)
    {
        PlayThread mythread;
        g_pPlayInstance = &mythread;
    }

    return g_pPlayInstance;
}

void PlayThread::WaitForFinished()
{
    if(g_pPlayInstance == NULL)
        return;

    g_pPlayInstance->Wait();
}

void PlayThread::StopSound()
{
    if(g_pPlayInstance == NULL)
        return;

    g_pPlayInstance->Stop();
}

void PlayThread::Play(int iSoundID, int iSoundVolume)
{
#if 0//darkhorse
    m_iSoundID = iSoundID;
    if (iSoundVolume > 0)
    {
        m_iRunning = 1;
        Thread::Start();
    }
#endif
}

int getpidofaplay()
{
#if 0//darkhorse
    char line[10];
    FILE* cmd = popen("pidof -s mi_audio", "r");
    if(cmd == NULL)
        return 0;

    long pid = 0;

    fgets(line, 10, cmd);
    pid = strtoul(line, NULL, 10);
    pclose(cmd);

    return pid;
#endif
    return 0;
}

void PlayThread::Stop()
{
#if 0//darkhorse
    GPIO_fast_setvalue(AUDIO_EN, OFF);
    if (m_iRunning == 1)
        my_usleep(100*1000);
    int pid = getpidofaplay();
    if(pid > 0)
        kill(pid, SIGKILL);

    Wait();
#endif
}

void PlayThread::run()
{
#if 0//darkhorse
#if 0
    if(m_iID < 0)
        return;

    if(m_iSoundVolume > 0)
        MainSTM_PlaySound(m_iID, m_iSoundVolume);

    int iDelay = g_aiSoundInfo[m_iID][1];
    for(int i = 0; i < iDelay && m_iRunning; i += 10)
        my_usleep(10 * 1000);

    if(m_iRunning == 0)
        MainSTM_StopSound();
#else
    m_iRunning = 1;
    SoundEnable(1);

//    my_printf("SoundID: %d, %d, %f\n", m_iSoundID, sizeof(g_szSoundPath) / sizeof(char*), Now());
    char szPath[128];
#if NFS_DEBUG_EN
    sprintf(szPath, "/mnt/%03dTTS.wav", m_iSoundID);
#else
    sprintf(szPath, "/test/sound/%03dTTS.wav", m_iSoundID);
#endif

    GPIO_fast_setvalue(AUDIO_EN, ON);
    my_usleep(120 * 1000);

    char szCmd[1024] = { 0 };
#if NFS_DEBUG_EN
    sprintf(szCmd, "/mnt/mi_audio -O -i %s -V %d -D 0", szPath, DEFAULT_MI_AO_VOLUME);
#else
    sprintf(szCmd, "mi_audio -O -i %s -V %d -D 0", szPath, DEFAULT_MI_AO_VOLUME);
#endif
//    sprintf(szCmd, "aplay -q %s", szPath);
    system(szCmd);

    GPIO_fast_setvalue(AUDIO_EN, OFF);
    m_iRunning = 0;

    SendGlobalMsg(MSG_SOUND, SOUND_PLAY_FINISHED, m_iSoundID, g_iSoundCounter);
#endif
#endif
}

#endif // USE_WIFI_MODULE
