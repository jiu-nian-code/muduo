#pragma once

#include<iostream>

#include"log.hpp"

#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include<errno.h>

#include<string.h>

#include<string>

#include<unistd.h>

#include<fcntl.h>

#define DEFAULT_PORT 9091

std::string default_ip("0.0.0.0");

class Socket
{
    int _skfd;
public:
    // 创建套接字
    Socket():
        _skfd(-1)
    {
    }

    Socket(int skfd):
    _skfd(skfd)
    {
    }

    bool my_socket()
    {
        // int socket(int domain, int type, int protocol);
        _skfd = socket(AF_INET, SOCK_STREAM, 0);
        if(_skfd < 0)
        {
            ERR_LOG("socket: %s", strerror(errno));
            return false;
        }
        INF_LOG("socketfd create successful.");
        return true;
    }

    // 绑定IP端口
    bool my_bind(const std::string& ip = default_ip, int16_t port = DEFAULT_PORT)
    {
        struct sockaddr_in in;
        memset((void*)&in, 0, sizeof(in));
        in.sin_family = AF_INET;
        // int inet_pton(int af, const char *src, void *dst);
        inet_pton(AF_INET, ip.c_str(), (void*)&in.sin_addr);
        in.sin_port = htons(port);

        // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        if(bind(_skfd, (const struct sockaddr*)&in, sizeof(in)) < 0)
        {
            ERR_LOG("bind: %s", strerror(errno));
            return false;
        }
        INF_LOG("socketfd bind successful.");
        return true;
    }

    // 开始监听
    bool my_listen()
    {
        // int listen(int sockfd, int backlog);
        if(listen(_skfd, 1024) < 0)
        {
            ERR_LOG("listen: %s", strerror(errno));
            return false;
        }
        INF_LOG("socketfd start listen.");
        return true;
    }

    // 向服务器发起连接
    bool my_connect(const std::string& ip, int16_t port)
    {
        struct sockaddr_in in;
        in.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), (void*)&in.sin_addr);
        in.sin_port = htons(port);
        // int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        if(connect(_skfd, (const struct sockaddr*)&in, sizeof(in)) < 0)
        {
            ERR_LOG("connect: %s", strerror(errno));
            return false;
        }
        INF_LOG("socketfd connect successful.");
        return true;
    }

    // 接受新链接
    int my_accept()
    {
        // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        int ac_fd = accept(_skfd, nullptr, nullptr);
        if(ac_fd < 0)
        {
            ERR_LOG("accept: %s", strerror(errno));
            return -1;
        }
        INF_LOG("listenfd accept a new socketfd.");
        return ac_fd;
    }

    // 发送数据
    ssize_t Send_Block(void* buf, size_t len)
    {
        // ssize_t send(int sockfd, const void *buf, size_t len, int flags);
        ssize_t ret = send(_skfd, buf, len, 0);
        if(ret < 0)
        {
            // EINTR 表示当前的socket的阻塞等待被信号打断了
            // EAGAIN 表示当前socket的接收缓冲区没有数据了，非阻塞返回会有这个情况
            if(errno == EINTR || errno == EAGAIN) return 0;
            ERR_LOG("send: %s", strerror(errno));
        }
        INF_LOG("send successful.");
        return ret;
    }

    ssize_t Send_NoBlock(void* buf, size_t len)
    {
        // ssize_t send(int sockfd, const void *buf, size_t len, int flags);
        ssize_t ret = send(_skfd, buf, len, MSG_DONTWAIT);
        if(ret < 0)
        {
            // EINTR 表示当前的socket的阻塞等待被信号打断了
            // EAGAIN 表示当前socket的接收缓冲区没有数据了，非阻塞返回会有这个情况
            if(errno == EINTR || errno == EAGAIN) return 0;
            ERR_LOG("send: %s", strerror(errno));
        }
        INF_LOG("send successful.");
        return ret;
    }

    // 接收数据
    ssize_t Recv_Block(void* buf, size_t len)
    {
        // ssize_t recv(int sockfd, void *buf, size_t len, int flags);
        ssize_t ret = recv(_skfd, buf, len, 0);
        if(ret < 0)
        {
            if(errno == EINTR || errno == EAGAIN) return 0;
            ERR_LOG("recv: %s", strerror(errno));
        }
        INF_LOG("recv successful.");
        return ret;
    }

    ssize_t Recv_NoBlock(void* buf, size_t len)
    {
        // ssize_t recv(int sockfd, void *buf, size_t len, int flags);
        ssize_t ret = recv(_skfd, buf, len, MSG_DONTWAIT);
        if(ret < 0)
        {
            if(errno == EINTR || errno == EAGAIN) return 0;
            ERR_LOG("recv: %s", strerror(errno));
        }
        INF_LOG("recv successful.");
        return ret;
    }

    // 关闭套接字
    ~Socket()
    {
        close(_skfd);
        INF_LOG("socketfd close.");
    }

    // 开启地址端口复用
    bool reuse_address_port()
    {
        int opt = 1;
        if(setsockopt(_skfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        {
            ERR_LOG("setsockopt: %s", strerror(errno));
            return false;
        }
        INF_LOG("reuse address and port successful.");
        return true;
    }

    // 开启非阻塞
    bool enable_NONBLOCK()
    {
        // int fcntl(int fd, int cmd, ... /* arg */ );
        int fl = fcntl(_skfd, F_GETFL);
        if(fl < 0)
        {
            ERR_LOG("fcntl: F_GETFL: %s", strerror(errno));
            return false;
        }
        if(fcntl(_skfd, F_SETFL, fl | O_NONBLOCK) < 0)
        {
            ERR_LOG("fcntl: F_SETFL: %s", strerror(errno));
            return false;
        }
        INF_LOG("enable socketfd nonblock successful.");
        return true;
    }

    // 创建一个监听连接
    bool create_listen_link(const std::string& ip = default_ip, int16_t port = DEFAULT_PORT)
    {
        my_socket();
        my_bind(ip, port);
        my_listen();
        return true;
    }
    
    // 创建一个服务端连接
    bool create_client_link(const std::string& ip, int16_t port)
    {
        return my_socket() && my_connect(ip, port);
    }

    int FD() { return _skfd; }
};