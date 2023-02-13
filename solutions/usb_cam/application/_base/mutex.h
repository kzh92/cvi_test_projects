#ifndef MUTEX_H
#define MUTEX_H

#ifndef __RTK_OS__
#ifndef USE_QT

//#include <pthread.h>
#include "common_types.h"

class Mutex
{
public:
    Mutex();
    ~Mutex();

    void    Lock();
    void    Unlock();

private:
    mymutex_ptr m_mutex;
};

#else

#include <QMutex>

class Mutex : public QMutex
{
public:
    Mutex();

    void    Lock();
    void    Unlock();

private:
};

#endif

#endif // !__RTK_OS__

#endif // MUTEX_H
