#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace SockAddrInModule
{
    class SockAddrIn
    {
    private:
        // 大端转换为本地端口值
        void NetPort2Local()
        {
            _port = ntohs(_port);
        }

        // 将IP地址转换为小端字节序并按照点分十进制存储
        void NetIP2Local()
        {
            char buffer[1024] = {0};
            _ip = inet_ntop(AF_INET, _ip.c_str(), buffer, sizeof(buffer));
        }

    public:
        // 无参构造
        SockAddrIn()
        {
        }

        // 根据指定的sockaddr_in对象进行构造
        SockAddrIn(const struct sockaddr_in &s)
            : _s_addr_in(s)
        {
            // 转换为本地小端用于使用
            NetIP2Local();
            NetPort2Local();
        }

        // 根据具体端口构造
        SockAddrIn(uint16_t port)
            : _port(port)
        {
            // 内部通过传入的端口对sockaddr_in对象进行初始化
            _s_addr_in.sin_family = AF_INET;
            _s_addr_in.sin_port = htons(_port);
            _s_addr_in.sin_addr.s_addr = INADDR_ANY;
        }

        // 根据端口和IP地址构造
        SockAddrIn(uint16_t port, std::string ip)
            : _ip(ip), _port(port)
        {
            _s_addr_in.sin_family = AF_INET;
            _s_addr_in.sin_port = htons(_port);
            _s_addr_in.sin_addr.s_addr = inet_addr(_ip.c_str());
        }

        // 重载&
        struct sockaddr *operator&()
        {
            return reinterpret_cast<struct sockaddr *>(&_s_addr_in);
        }

        // 重载==
        bool operator==(const SockAddrIn &s)
        {
            return _ip == s._ip && _port == _port;
        }

        // 获取struct sockaddr_in对象长度
        socklen_t getLength()
        {
            return sizeof(_s_addr_in);
        }

        // 返回IP地址
        std::string getIp()
        {
            return _ip;
        }

        // 返回端口号
        uint16_t getPort()
        {
            return _port;
        }

        ~SockAddrIn()
        {
        }

    private:
        struct sockaddr_in _s_addr_in;
        std::string _ip;
        uint16_t _port;
    };
}