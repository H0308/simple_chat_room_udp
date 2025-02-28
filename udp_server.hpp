#pragma once

#include <functional>
#include <string>
#include "sockaddr_in_t.hpp"
#include "errors.hpp"
#include "log.hpp"

using task_t = std::function<std::string(const std::string &)>;

namespace UdpServerModule
{
    // 默认端口和IP地址
    // const std::string default_ip = "127.0.0.1"; 去除
    const uint16_t default_port = 8080;

    using namespace LogSystemModule;
    using namespace SockAddrInModule;

    class UdpServer
    {
    public:
        UdpServer(task_t t, uint16_t port = default_port)
            : _socketfd(-1), _sa_in(port), _isRunning(false), _translate(t)
        {
            // 创建服务器套接字
            _socketfd = socket(AF_INET, SOCK_DGRAM, 0);

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
                        SockAddrIn temp(peer);
                        LOG(LogLevel::INFO) << "Client: "
                                            << temp.getIp() << ":"
                                            << temp.getPort()
                                            << " send: " << buffer;

                        // 翻译
                        std::string ret = _translate(buffer);
                        ssize_t n = sendto(_socketfd, ret.c_str(), ret.size(), 0, &temp, temp.getLength());

                        if (n > 0)
                        {
                            LOG(LogLevel::INFO) << "Server received: "
                                                << buffer
                                                << ", and send " << ret << " to: "
                                                << temp.getIp() << ":"
                                                << temp.getPort();
                        }
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

        task_t _translate; // 翻译函数
    };
} // namespace UdpServerModule
