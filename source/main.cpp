#include"buffer.hpp"

#include"socket.hpp"

#include"log.hpp"

#include<iostream>

#include<time.h>

#include<unistd.h>

int main()
{
    // int n = 10;
    // while(n--)
    // {
    //     INF_LOG("hello world");
    //     DBG_LOG("hello world");
    //     std::string str("hello world");
    //     ERR_LOG("%s", str.c_str());
    //     sleep(1);
    // }

    Socket sk;
    sk.create_listen_link();
    Socket newlink(sk.my_accept());
    char buf[1024];
    while(1)
    {
        ssize_t ret = newlink.Recv(buf, 1024);
        buf[ret] = 0;
        std::cout << buf << std::endl;
        newlink.Send(buf, 1024);
    }
    return 0;
}