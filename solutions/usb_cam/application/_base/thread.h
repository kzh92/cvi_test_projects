#ifndef THREAD_H
#define THREAD_H

//#include <pthread.h>
#include "common_types.h"

class Thread
{
public:
    Thread();

    void Start();
    void Wait();
    void Exit();

    void ThreadProc();

protected:
    virtual void run();

    mythread_ptr   m_thread;
};

#endif // THREAD_H
