#include "mutex.h"
#include "common_types.h"

#ifndef __RTK_OS__

#ifndef USE_QT

Mutex::Mutex()
{
#if 0
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutex_init(&m_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
#else
	m_mutex = my_mutex_init();
#endif
}

Mutex::~Mutex()
{
    my_mutex_destroy(m_mutex);
}

void Mutex::Lock()
{
#if 0
    pthread_mutex_lock(&m_mutex);
#else
	my_mutex_lock(m_mutex);
#endif
}

void Mutex::Unlock()
{
#if 0
    pthread_mutex_unlock(&m_mutex);
#else
	my_mutex_unlock(m_mutex);
#endif
}

#else // ! USE_QT

Mutex::Mutex()
{

}

void Mutex::Lock()
{
    QMutex::lock();
}

void Mutex::Unlock()
{
    QMutex::unlock();
}

#endif // ! USE_QT

#endif // !__RTK_OS__