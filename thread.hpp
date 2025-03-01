#pragma once

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <functional>

namespace ThreadModule
{
    // 线程执行的任务类型
    using func_t = std::function<void()>;
    // 计数器
    static int count = 0;

    // 线程状态
    enum class ThreadStatus
    {
        isNew,
        isRunning,
        isStopped,
        isDetached,
        isJoinable
    };

    class Thread
    {
    private:
        static void *routine(void *arg) // 线程执行函数
        {
            // 获取到this指针
            Thread *_t = static_cast<Thread *>(arg);
            // 更改线程状态为运行状态
            _t->_thStatus = ThreadStatus::isRunning;
            // 执行任务函数
            _t->_func();

            return NULL;
        }

    public:
        Thread(func_t func)
            : _func(func), _thStatus(ThreadStatus::isNew), _joinable(true)
        {
            _name = "Thread" + std::to_string(count++);
        }

        // 创建线程
        bool start()
        {
            if (_thStatus != ThreadStatus::isRunning)
            {
                int ret = pthread_create(&_tid, NULL, routine, (void *)this);

                if (!ret)
                    return false;

                return true;
            }

            return false;
        }

        // 终止线程
        bool cancel()
        {
            if (_thStatus == ThreadStatus::isRunning)
            {
                int ret = pthread_cancel(_tid);
                if (!ret)
                    return false;

                _thStatus = ThreadStatus::isStopped;

                return true;
            }

            return false;
        }

        // 等待线程
        bool join()
        {
            if (_thStatus != ThreadStatus::isDetached || _joinable)
            {
                int ret = pthread_join(_tid, NULL);
                if (ret)
                    return false;

                _thStatus = ThreadStatus::isStopped;

                return true;
            }
            return false;
        }

        // 分离线程
        bool detach()
        {
            if (_joinable)
            {
                pthread_detach(_tid);
                _joinable = false;
                return true;
            }

            return false;
        }

        // 获取线程名称
        std::string getName()
        {
            return _name;
        }

        ~Thread()
        {
        }

    private:
        std::string _name;
        pthread_t _tid;
        ThreadStatus _thStatus;
        bool _joinable;
        func_t _func;
    };
}