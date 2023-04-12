#include "thread.h"
#include "common_types.h"
//#include <stdio.h>
//#include <stdlib.h>

void* ThreadProc1(void*);

Thread::Thread()
{
    m_thread = 0;
}

void Thread::Start()
{
    if(my_thread_create(&m_thread, NULL, ThreadProc1, this))
        my_printf("create thread error.\n");
}

void Thread::Wait()
{
    if (m_thread != NULL)
    {
        my_thread_join(&m_thread);
        m_thread = NULL;
    }
}

void Thread::Exit()
{
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
    Thread* pThread = (Thread*)(param);
    pThread->ThreadProc();
    return NULL;
}
