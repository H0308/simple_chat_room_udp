#include "udp_client.hpp"

#include "user.hpp"
#include <memory>
#include <signal.h>

using namespace UdpClientModule;
using namespace LogSystemModule;

std::shared_ptr<UdpClient> client;

void quit(int sig)
{
    (void)sig;
    std::string userinfo = (client->getName()) + ":" + "quit";
    int ret = sendto(client->getSocketfd(), userinfo.c_str(), userinfo.size(), 0, &(client->getSockAddrIn()), (client->getSockAddrIn()).getLength());
    exit(0);
}

int main(int argc, char *argv[])
{
    // 捕捉2号新号
    signal(2, quit);

    // 获取到用户输入的端口和IP地址
    std::string ip = argv[1];
    uint16_t port = std::stoi(argv[2]);

    // 获取用户名字
    std::string name = argv[3];

    // 创建客户端对象——用户自定义端口和IP地址
    client = std::make_shared<UdpClient>(name, ip, port);

    // 启动客户端
    client->start();

    return 0;
}