#pragma once

#include "sockaddr_in_t.hpp"
#include "errors.hpp"
#include "log.hpp"

namespace UdpClientModule
{
    using namespace LogSystemModule;
    using namespace SockAddrInModule;

    // 默认服务器端口和IP地址
    const std::string default_ip = "127.0.0.1";
    const uint16_t default_port = 8080;

    class UdpClient
    {
    public:
        UdpClient(const std::string ip = default_ip, uint16_t port = default_port)
            : _socketfd(-1), _isRunning(false), _sa_in(port, ip)
        {
            _socketfd = socket(AF_INET, SOCK_DGRAM, 0);

            if (_socketfd < 0)
            {
                LOG(LogLevel::FATAL) << "Client initiate error: " << strerror(errno);

                exit(static_cast<int>(ErrorNumber::SocketFail));
            }

            LOG(LogLevel::INFO) << "Client initiated: " << _socketfd;
        }

        // 启动客户端
        void start()
        {
            if (!_isRunning)
            {
                _isRunning = true;
                while (true)
                {
                    // 1.1 读取输入信息
                    std::string message;
                    getline(std::cin, message);

                    // 1.2 发送数据
                    ssize_t ret = sendto(_socketfd, message.c_str(), message.size(), 0, &_sa_in, _sa_in.getLength());

                    if (ret < 0)
                        LOG(LogLevel::WARNING) << "Client send failed";

                    // 2. 回显服务器的信息
                    struct sockaddr_in temp;
                    socklen_t length = sizeof(temp);
                    char buffer[1024] = {0};
                    ssize_t n = recvfrom(_socketfd, buffer, sizeof(buffer) - 1, 0, reinterpret_cast<struct sockaddr *>(&temp), &length);

                    if (n > 0)
                        LOG(LogLevel::INFO) << "Received message from server: " << buffer;
                }
            }
        }

        // 结束客户端
        void stop()
        {
            if (_isRunning)
                close(_socketfd);
        }

        ~UdpClient()
        {
            stop();
        }

    private:
        int _socketfd; // 套接字文件描述符
        SockAddrIn _sa_in;
        bool _isRunning; // 标记客户端是否已经启动
    };
}