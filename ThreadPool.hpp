#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include "thread.hpp"
#include "log.hpp"
#include "mutex.hpp"
#include "cond.hpp"

namespace ThreadPoolModule
{
    using namespace ThreadModule;
    using namespace LogSystemModule;
    using namespace ConditionModule;
    const int d_num = 5; // 默认线程个数

    void test()
    {
    }

    template <class T>
    class ThreadPool
    {
    private:
        // 判断任务队列是否为空
        bool isEmpty()
        {
            return _tasks.empty();
        }

        // 获取并执行任务
        void get_executeTasks()
        {
            while (true)
            {
                T t;
                {
                    // 申请锁
                    MutexGuard guard(_lock);
                    while (isEmpty() && _isRunning)
                    {
                        _wait_num++;
                        _cond.wait(_lock);
                        _wait_num--;
                    }

                    // 任务队列为空且线程池已经结束直接退出
                    if (isEmpty() && !_isRunning)
                        break;

                    // 此时存在任务，取出任务
                    t = _tasks.front();
                    _tasks.pop();
                }

                // 执行任务之前确保已经释放互斥锁
                t();
            }
        }

        // 私有构造函数
        ThreadPool(int num = d_num)
            : _num(num), _isRunning(false)
        {
            // 创建指定个数个线程
            for (int i = 0; i < _num; i++)
            {
                _threads.push_back(Thread(std::bind(&ThreadPool<T>::get_executeTasks, this))); // 创建并组织线程，假设当前线程的任务为test

                // 打印相关日志
                LOG(LogLevel::INFO) << "创建线程：" << _threads.back().getName();

                // 启动线程池
                // startThreads();
            }
        }

        // 禁用拷贝和赋值
        ThreadPool(const ThreadPool &tp) = delete;
        ThreadPool &operator=(ThreadPool &tp) = delete;

    public:
        // 获取线程池对象
        static std::shared_ptr<ThreadPool<T>> getInstance(int num = d_num)
        {
            if (!_tp_ptr)
            {
                MutexGuard guard(_s_lock);
                if (!_tp_ptr)
                {
                    _tp_ptr = std::shared_ptr<ThreadPool<T>>(new ThreadPool<T>(num));
                }
            }
            return _tp_ptr;
        }

        // 启动线程
        void startThreads()
        {
            // 如果当前线程池已经处于运行状态，则不再重复启动
            if (_isRunning)
                return;

            // 否则启动线程池中的所有线程
            _isRunning = true;

            for (auto &thread : _threads)
            {
                thread.start();
                LOG(LogLevel::INFO) << "当前线程：" << thread.getName() << "启动";
            }
        }

        // 回收线程
        void waitThreads()
        {
            for (auto &thread : _threads)
            {
                thread.join();
                LOG(LogLevel::INFO) << "当前线程：" << thread.getName() << "被回收";
            }
        }

        // 插入任务
        void pushTasks(T &task)
        {
            MutexGuard guard(_lock);

            // 线程池结束，不允许插入任务
            if (!_isRunning)
                return;

            // 插入任务
            _tasks.push(task);

            // 有任务时唤醒指定线程执行任务
            if (_wait_num > 0)
                _cond.notify();
        }

        // 结束线程
        void stopThreads()
        {
            // 确保线程池处于运行状态
            if (_isRunning)
            {
                // 修改线程池运行状态，确保不会再有任务插入
                _isRunning = false;

                // 唤醒所有线程
                if (_wait_num > 0)
                    _cond.notifyAll();
            }
        }

        ~ThreadPool()
        {
        }

    private:
        std::vector<Thread> _threads; // 组织所有线程
        size_t _num;                  // 线程个数
        std::queue<T> _tasks;         // 任务队列
        bool _isRunning;              // 用于判断线程池是否处于运行状态
        Mutex _lock;                  // 任务锁
        Condition _cond;              // 任务条件变量
        int _wait_num;                // 等待任务的线程个数

        static Mutex _s_lock;                          // 静态单例锁
        static std::shared_ptr<ThreadPool<T>> _tp_ptr; // 单例线程池对象指针
    };

    // 初始化指针
    template <typename T>
    std::shared_ptr<ThreadPool<T>> ThreadPool<T>::_tp_ptr = nullptr;

    // 初始化单例锁
    template <typename T>
    Mutex ThreadPool<T>::_s_lock;
} // namespace ThreadPool
