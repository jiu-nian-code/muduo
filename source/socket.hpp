#include<iostream>

#include"log.hpp"

#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include<errno.h>

#include<string.h>

#include<string>

std::string default_ip("0.0.0.0");

class Socket
{
    int _skfd;
public:
    // 创建套接字
    Socket()
    {
        // int socket(int domain, int type, int protocol);
        _skfd = socket(AF_INET, SOCK_STREAM, 0);
        if(_skfd < 0)
        {
            ERR_LOG("%s", strerror(errno));
            return;
        }
    }
    // 绑定IP端口
    bool bind(const std::string& ip = default_ip, int16_t port = 9090)
    {
        struct sockaddr_in in;
        memset((void*)&in, 0, sizeof(in));
        in.sin_family = AF_INET;
        // int inet_pton(int af, const char *src, void *dst);
        inet_pton(AF_INET, "0.0.0.0", (void*)&in.sin_addr);
        in.sin_port = htons(port);

        // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        if(bind(_skfd, (const struct sockaddr*)&in, sizeof(in)) < 0)
        {
            ERR_LOG("%s", strerror(errno));
        }
    }
    // 开始监听
    // 向服务器发起连接
    // 接受新链接
    // 发送数据
    // 接收数据
    // 关闭套接字
    // 创建一个监听链接
    // 创建一个服务端连接
};