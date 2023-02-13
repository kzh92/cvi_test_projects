#ifndef WATCHTASK_H
#define WATCHTASK_H

#include "thread.h"
#include "mutex.h"

enum
{
    WATCH_TYPE_LOW_BATT,
    WATCH_TYPE_TIMEOUT,
    WATCH_TYPE_TIMER,
    WATCH_TYPE_SIREN
};

#define MAX_TIMER_COUNT 20

class WatchTask : public Thread
{
public:
    WatchTask();
    ~WatchTask();

    void    Init();
    void    Deinit();
    void    Start(int iBattScan);
    void    Stop();
    int     AddTimer(float iMsec);
    void    RemoveTimer(int iTimerID);
    void    ResetTimer(int iTimerID);
    int     GetCounter(int iTimerID);
    void    ThreadProc();

    static void    ScanBattery(int iSendFlag);

protected:
    void    run();

    int     m_iBattScan;
    int     m_iRunning;
    int     m_iIDCounter;

    mymutex_ptr   m_xTimerMutex;
    int     m_iTimerCount;
    int     *m_aiTimerIDs;
    int     *m_aiTimerCounter;
    float   *m_aiTimerMsec;
    float   *m_arTimerTick;
};

#endif // WATCHTASK_H
