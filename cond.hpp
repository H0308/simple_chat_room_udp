#pragma once

#include <iostream>
#include <pthread.h>
#include "mutex.hpp"

namespace ConditionModule
{
    using namespace MutexModule;
    class Condition
    {
    public:
        Condition()
        {
            // 初始化条件变量
            pthread_cond_init(&_cond, NULL);
        }

        void wait(Mutex &mutex)
        {
            pthread_cond_wait(&_cond, mutex.getLockAddress());
        }

        void notify()
        {
            // 唤醒一个线程
            pthread_cond_signal(&_cond);
        }

        void notifyAll()
        {
            // 唤醒多个线程
            pthread_cond_broadcast(&_cond);
        }

        ~Condition()
        {
            // 销毁条件变量
            pthread_cond_destroy(&_cond);
        }

    private:
        pthread_cond_t _cond;
    };
}