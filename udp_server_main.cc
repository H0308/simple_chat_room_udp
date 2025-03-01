#include "udp_server.hpp"
#include "user.hpp"
#include "log.hpp"
#include <memory>

using namespace UdpServerModule;
using namespace UserManageModule;
using namespace LogSystemModule;

int main(int argc, char *argv[])
{
    // 创建UserManager对象
    std::shared_ptr<UserManager> usm = std::make_shared<UserManager>();
    // 创建UdpServerModule对象
    std::shared_ptr<UdpServer> udp_server;
    if (argc == 1)
    {
        udp_server = std::make_shared<UdpServer>([&usm](const User &user)
                                                 { usm->addUser(user); },
                                                 [&usm](int sockfd, const std::string &message)
                                                 { usm->dispatchMessage(sockfd, message); },
                                                 [&usm](const User &user)
                                                 { usm->delUser(user); });
    }
    else if (argc == 2)
    {
        uint16_t port = std::stoi(argv[1]);
        udp_server = std::make_shared<UdpServer>([&usm](const User &user)
                                                 { usm->addUser(user); },
                                                 [&usm](int sockfd, const std::string &message)
                                                 { usm->dispatchMessage(sockfd, message); },
                                                 [&usm](const User &user)
                                                 { usm->delUser(user); }, port);
    }
    else
    {
        LOG(LogLevel::ERROR) << "错误使用，正确使用：" << argv[0] << " 端口（或者不写）";
        exit(4);
    }

    udp_server->start();

    return 0;
}