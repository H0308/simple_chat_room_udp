#pragma once

#include <iostream>
#include <string>
#include <list>
#include <memory>
#include <algorithm>

#include "sockaddr_in_t.hpp"
#include "log.hpp"

namespace UserManageModule
{
    using namespace SockAddrInModule;
    using namespace LogSystemModule;

    // 观察者基类
    class UserObserver
    {
    public:
        virtual ~UserObserver() = default;
        virtual void sendMessage(int sockfd, const std::string &message) = 0;
    };

    // 观察者实现类
    class User : public UserObserver
    {
    public:
        User(uint16_t port, std::string ip, std::string name)
            : _name(name), _sa_in(port, ip)
        {
        }

        virtual void sendMessage(int sockfd, const std::string &message) override
        {
            // 打印日志
            LOG(LogLevel::INFO) << "send message: " << message << "to: " << _sa_in.getIp() << ":" << _sa_in.getPort();

            // 发送信息给自己
            ssize_t ret = sendto(sockfd, message.c_str(), message.size(), 0, &_sa_in, _sa_in.getLength());
            (void)ret;
        }

        std::string getName()
        {
            return _name;
        }

        SockAddrIn getSockAddrIn()
        {
            return _sa_in;
        }

        // 重载==
        bool operator==(const User &u)
        {
            return _name == u._name && _sa_in == u._sa_in;
            // return _sa_in == u._sa_in;
        }

    private:
        std::string _name;
        SockAddrIn _sa_in;
    };

    // 主题基类
    class UserManagerSubject
    {
    public:
        virtual ~UserManagerSubject() = default;
        // 添加方法
        virtual void addUser(const User &user) = 0;
        // 删除方法
        virtual void delUser(const User &user) = 0;
        // 通知方法
        virtual void dispatchMessage(int sockfd, const std::string &message) = 0;
    };

    // 主题实现类
    class UserManager : public UserManagerSubject
    {
    public:
        UserManager()
        {
        }

        // 实现添加方法
        virtual void addUser(const User &user) override
        {
            // 先申请锁
            MutexGuard guard(_mutex);
            // 确保用户不存在
            for (auto &u : _u_list)
                if (*u == user)
                {
                    LOG(LogLevel::INFO) << "用户已存在";
                    return;
                }

            // 不存在时插入
            _u_list.push_back(std::make_shared<User>(user));

            // 打印当前在线用户
            printUsers();
        }

        void printUsers()
        {
            for (auto &u : _u_list)
            {
                LOG(LogLevel::INFO) << "当前在线用户：" << u->getName() << "(" << u->getSockAddrIn().getIp() << ":" << u->getSockAddrIn().getPort() << ")";
            }
        }

        // 实现删除方法
        virtual void delUser(const User &user) override
        {
            MutexGuard guard(_mutex);

            auto pos = std::remove_if(_u_list.begin(), _u_list.end(), [&user](std::shared_ptr<User> &u)
                                      { return *u == user; });

            _u_list.erase(pos, _u_list.end());
            // 打印当前在线用户
            printUsers();
        }

        // 通知方法
        virtual void dispatchMessage(int sockfd, const std::string &message) override
        {
            MutexGuard guard(_mutex);
            LOG(LogLevel::INFO) << "分发任务";
            for (auto &u : _u_list)
                u->sendMessage(sockfd, message);
        }

    private:
        std::list<std::shared_ptr<User>> _u_list; // 用户链表
        Mutex _mutex;                             // 链表互斥锁
    };
}