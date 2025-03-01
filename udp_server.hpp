#pragma once

#include <functional>
#include <string>
#include <string.h>
#include "sockaddr_in_t.hpp"
#include "errors.hpp"
#include "log.hpp"
#include "user.hpp"
#include "userInfo.hpp"
#include "ThreadPool.hpp"

using namespace UserManageModule;

using add_user_t = std::function<void(const User &)>;
using dispatch_msg_t = std::function<void(int, const std::string &)>;
using del_user_t = std::function<void(const User &)>;
using task_t = std::function<void()>;

namespace UdpServerModule
{
    // 默认端口和IP地址
    const uint16_t default_port = 8080;

    using namespace LogSystemModule;
    using namespace SockAddrInModule;
    using namespace ThreadPoolModule;

    class UdpServer
    {
    public:
        UdpServer(add_user_t addUser, dispatch_msg_t dispatchMsg, del_user_t delUser, uint16_t port = default_port)
            : _socketfd(-1), _sa_in(port), _isRunning(false), _addUser(addUser), _dispatch_message(dispatchMsg), _delUser(delUser)
        {
            // 创建服务器套接字
            _socketfd = socket(AF_INET, SOCK_DGRAM, 0);

            _tp = ThreadPool<task_t>::getInstance();

            if (!_tp)
            {
                LOG(LogLevel::ERROR) << "线程池启动失败";
                return;
            }

            _tp->startThreads();

            if (_socketfd < 0)
            {
                LOG(LogLevel::FATAL) << "Server initiate error：" << strerror(errno);
                exit(static_cast<int>(ErrorNumber::SocketFail));
            }

            LOG(LogLevel::INFO) << "Server initiated：" << _socketfd;

            int ret = bind(_socketfd, &_sa_in, _sa_in.getLength());

            if (ret < 0)
            {
                LOG(LogLevel::FATAL) << "Bind error" << strerror(errno);
                exit(static_cast<int>(ErrorNumber::BindSocketFail));
            }

            LOG(LogLevel::INFO) << "Bind success";
        }

        // 启动服务器
        void start()
        {
            if (!_isRunning)
            {
                _isRunning = true;
                while (true)
                {
                    // 1. 接收客户端信息
                    char buffer[1024] = {0};
                    struct sockaddr_in peer;
                    socklen_t length = sizeof(peer);
                    ssize_t ret = recvfrom(_socketfd, buffer, sizeof(buffer) - 1, 0, reinterpret_cast<struct sockaddr *>(&peer), &length);

                    if (ret > 0)
                    {
                        // 1. 根据收到的消息构建User对象
                        SockAddrIn netUser(peer);
                        // 切割字符串
                        std::string fullInfo = buffer;
                        // 获取名字
                        auto pos = fullInfo.find(":", 0);
                        std::string name = fullInfo.substr(0, pos);
                        // 获取消息
                        std::string message = fullInfo.substr(pos + 1, fullInfo.size());

                        if (message.size() == 0)
                            continue;

                        // LOG(LogLevel::DEBUG) << "你好" << name << "信息：" << message;

                        User user(netUser.getPort(), netUser.getIp(), name);

                        // 1.1 判断是删除还是添加
                        if (strcmp(message.c_str(), "quit") == 0)
                        {
                            // 删除用户
                            _delUser(user);
                            message = user.getName() + ":" + "(" + user.getSockAddrIn().getIp() + ":" + std::to_string(user.getSockAddrIn().getPort()) + ")" + " offline";
                        }
                        else if (strcmp(message.c_str(), "online") == 0)
                        {
                            // 添加用户
                            _addUser(user);
                            message = user.getName() + ":" + "(" + user.getSockAddrIn().getIp() + ":" + std::to_string(user.getSockAddrIn().getPort()) + ")：" + message;
                        }

                        // 2. 创建线程池并添加任务
                        task_t task = std::bind(UdpServer::_dispatch_message, _socketfd, message);
                        _tp->pushTasks(task);
                    }
                }
            }
        }

        // 停止服务器
        void stop()
        {
            if (_isRunning)
                close(_socketfd);
        }

        ~UdpServer()
        {
            stop();
        }

    private:
        int _socketfd; // 套接字文件描述符
        SockAddrIn _sa_in;
        bool _isRunning; // 服务器是否正在运行
        std::shared_ptr<ThreadPool<task_t>> _tp;

        add_user_t _addUser;              // 添加用户函数
        dispatch_msg_t _dispatch_message; // 分发消息函数
        del_user_t _delUser;              // 删除用户函数
    };
} // namespace UdpServerModule
