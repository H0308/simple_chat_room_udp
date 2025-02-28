#include "udp_client.hpp"

#include <memory>

using namespace UdpClientModule;
using namespace LogSystemModule;

int main(int argc, char *argv[])
{
    std::shared_ptr<UdpClient> client;
    if (argc == 1)
    {
        // 创建客户端对象——使用默认端口和IP地址
        client = std::make_shared<UdpClient>();
    }
    else if (argc == 3)
    {
        // 获取到用户输入的端口和IP地址
        std::string ip = argv[1];
        uint16_t port = std::stoi(argv[2]);

        // 创建客户端对象——用户自定义端口和IP地址
        client = std::make_shared<UdpClient>(ip, port);
    }
    else
    {
        LOG(LogLevel::ERROR) << "错误使用，正确使用为：" << argv[0] << " IP地址 端口号（或者二者都不存在）";
        exit(3);
    }

    // 启动客户端
    client->start();

    return 0;
}