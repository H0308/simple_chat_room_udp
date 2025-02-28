#include "udp_server.hpp"
#include "dictionary.hpp"
#include <memory>

using namespace UdpServerModule;
using namespace DictionaryModule;

int main(int argc, char *argv[])
{
    // 创建字典类对象
    std::shared_ptr<Dictionary> dict = std::make_shared<Dictionary>();
    dict->printDic();
    // 创建UdpServerModule对象
    std::shared_ptr<UdpServer> udp_server;
    if (argc == 1)
    {
        // 绑定
        udp_server = std::make_shared<UdpServer>(std::bind(&Dictionary::translate, dict.get(), std::placeholders::_1));
    }
    else if (argc == 2)
    {
        uint16_t port = std::stoi(argv[1]);
        udp_server = std::make_shared<UdpServer>(std::bind(&Dictionary::translate, dict.get(), std::placeholders::_1), port);
    }
    else
    {
        LOG(LogLevel::ERROR) << "错误使用，正确使用：" << argv[0] << " 端口（或者不写）";
        exit(4);
    }

    udp_server->start();

    return 0;
}