#include "thread.h"
#include "common_types.h"
//#include <stdio.h>
//#include <stdlib.h>

#ifndef USE_QT

void* ThreadProc1(void*);

Thread::Thread()
{
    m_thread = 0;
}

void Thread::Start()
{
#if 0
    if(pthread_create(&m_thread, NULL, ThreadProc1, this) != 0)
        perror ("can't create thread\n");
#else
    if(my_thread_create(&m_thread, NULL, ThreadProc1, this))
        my_printf("create thread error.\n");
#endif
}

void Thread::Wait()
{
#if 0 
    if(m_thread != 0)
    {
        pthread_join(m_thread, NULL);
        m_thread = 0;
    }
#else
    if (m_thread != NULL)
    {
        my_thread_join(&m_thread);
        m_thread = NULL;
    }
#endif
}

void Thread::Exit()
{
#if 0
    if(m_thread != 0)
    {
        pthread_cancel(m_thread);
        m_thread = 0;
    }
#endif
}

void Thread::ThreadProc()
{
    run();
}

void Thread::run()
{
    
}

void* ThreadProc1(void* param)
{
#if 0
    Thread* pThread = (Thread*)(param);
    pThread->ThreadProc();

    pthread_exit(NULL);
#else
    Thread* pThread = (Thread*)(param);
    pThread->ThreadProc();
#endif
    return NULL;
}

#else // ! USE_QT

Thread::Thread(QObject* parent)
    : QThread(parent)
{

}

void Thread::Start()
{
    QThread::start();
}

void Thread::Wait()
{
    QThread::wait();
}

void Thread::Exit()
{
    QThread::terminate();
}

#endif // ! USE_QT
