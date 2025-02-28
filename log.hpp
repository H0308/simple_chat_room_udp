#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <unistd.h>
#include <filesystem>
#include <time.h>
#include "mutex.hpp"

namespace LogSystemModule
{
    using namespace MutexModule;

    // 默认的目录路径和文件路径
    const std::string d_dir_path = "./log/";
    const std::string d_file_path = "log.txt";

    // 日志等级
    enum class LogLevel
    {
        // 默认从1开始
        DEBUG = 1,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    // 获取时间函数
    std::string getCurrentTime()
    {
        // 获取时间戳
        time_t time_stamp = time(NULL);
        // 将时间戳转换为时间，考虑使用localtime_r而不是localtime，确保多线程下安全
        struct tm time_struct;
        localtime_r(&time_stamp, &time_struct);

        // 将时间结构写入缓冲区
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%4d-%02d-%02d %02d-%02d-%02d",
                 time_struct.tm_year + 1900, // 获取年
                 time_struct.tm_mon + 1,     // 获取月
                 time_struct.tm_mday,        // 获取日
                 time_struct.tm_hour,        // 获取小时
                 time_struct.tm_min,         // 获取分钟
                 time_struct.tm_sec          // 获取秒
        );

        return buffer;
    }

    // 将枚举值转换为对应的字符串
    std::string level2string(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "WARNING";
        case LogLevel::FATAL:
            return "FATAL";
        default:
            return "";
        }
        return "";
    }

    // 抽象策略类
    class LogStrategy
    {
    public:
        // 使用默认析构函数
        virtual ~LogStrategy() = default;
        // 定义抽象虚函数
        virtual void printLog(const std::string &message) = 0;
    };

    // 具体策略类——控制台输出
    class ConsoleLogStrategy : public LogStrategy
    {
    public:
        ConsoleLogStrategy() = default;

        virtual void printLog(const std::string &message) override
        {
            // 先申请锁
            MutexGuard guard(_lock);
            std::cout << message << std::endl;
        }

        ~ConsoleLogStrategy() = default;

    private:
        Mutex _lock;
    };

    // 具体策略类——文件输出
    class FileLogStrategy : public LogStrategy
    {
    public:
        FileLogStrategy(const std::string &dir_path = d_dir_path, const std::string &file_path = d_file_path)
            : _dir_path(dir_path), _file_path(file_path)
        {
            // 先申请锁
            MutexGuard guard(_lock);

            // 判断目录是否存在
            if (std::filesystem::exists(_dir_path))
                return; // 存在直接返回
            else
            {
                try
                {
                    // 创建目录
                    std::filesystem::create_directories(_dir_path);
                }
                catch (const std::filesystem::filesystem_error &e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
        }

        virtual void printLog(const std::string &message) override
        {
            // 先申请锁
            MutexGuard guard(_lock);

            // 打开文件并追加写入
            const std::string path = _dir_path + _file_path;
            std::fstream in_out(path, std::ios::app);
            in_out << message << "\n";

            // 关闭文件
            in_out.close();
        }

        ~FileLogStrategy()
        {
        }

    private:
        std::string _dir_path;
        std::string _file_path;

        Mutex _lock;
    };

    class LogHandler
    {
    public:
        // 默认使用ConsoleLogStrategy类初始化
        LogHandler()
            : _log(std::make_shared<ConsoleLogStrategy>())
        {
        }

        // 启用控制台输出
        void enableConsoleLog()
        {
            _log = std::make_shared<ConsoleLogStrategy>();
        }

        // 启用文件输出
        void enableFileLog()
        {
            _log = std::make_shared<FileLogStrategy>();
        }

        ~LogHandler()
        {
        }

        class LogMessage
        {
        public:
            LogMessage(LogLevel level, const std::string &filename, int lineno, LogHandler &loghandler)
                : _time(getCurrentTime()), _level(level), _pid(getpid()), _filename(filename), _lineno(lineno), _loghandler(loghandler)
            {
                std::stringstream ss;
                ss << "[" << _time << "] "
                   << "[" << level2string(_level) << "] "
                   << "[" << _pid << "] "
                   << "[" << _filename << "] "
                   << "[" << _lineno << "] - ";

                _message = ss.str();
            }

            // 重载流插入函数
            template <class T>
            LogMessage &operator<<(const T &content)
            {
                std::stringstream ss;
                ss << content;
                _message += ss.str();

                return *this;
            }

            ~LogMessage()
            {
                // 如果父类引用不为空指针，就可以实现日志输出到指定为止
                if (_loghandler._log)
                    _loghandler._log->printLog(_message);
            }

        private:
            std::string _time;       // 时间
            LogLevel _level;         // 日志等级
            pid_t _pid;              // 进程pid
            std::string _filename;   // 文件名
            int _lineno;             // 行号
            std::string _message;    // 记录结果
            LogHandler &_loghandler; // LogHandler对象
        };

        LogMessage operator()(LogLevel level, const std::string filename, int lineno)
        {
            return LogMessage(level, filename, lineno, *this);
        }

    private:
        std::shared_ptr<LogStrategy>
            _log;
    };

    // 创建LogHandler对象
    LogHandler loghandler;

#define LOG(LEVEL) loghandler(LEVEL, __FILE__, __LINE__)

#define ENABLECONSOLELOG() loghandler.enableConsoleLog()
#define ENABLEFILELOG() loghandler.enableFileLog()
}