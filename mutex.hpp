#pragma once

#include <iostream>
#include <pthread.h>

namespace MutexModule
{
    class Mutex
    {
    private:
        Mutex(const Mutex &m) = delete;
        Mutex operator=(const Mutex &m) = delete;

    public:
        Mutex()
        {
            pthread_mutex_init(&_mutex, NULL);
        }

        void lock()
        {
            pthread_mutex_lock(&_mutex);
        }

        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }

        pthread_mutex_t *getLockAddress()
        {
            return &_mutex;
        }

        ~Mutex()
        {
            pthread_mutex_destroy(&_mutex);
        }

    private:
        pthread_mutex_t _mutex;
    };

    class MutexGuard
    {
    public:
        MutexGuard(Mutex &mutex)
            : _mutex(mutex)
        {
            // 创建对象时就自动上锁
            _mutex.lock();
        }

        ~MutexGuard()
        {
            // 销毁对象时就自动解锁
            _mutex.unlock();
        }

    private:
        Mutex &_mutex;
    };
}