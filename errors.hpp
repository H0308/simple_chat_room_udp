#pragma once

// 错误码枚举类
enum class ErrorNumber
{
    SocketFail = 1, // 创建套接字失败
    BindSocketFail, // 绑定失败
};