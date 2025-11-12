#include"buffer.hpp"

#include"socket.hpp"

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
        std::string str("hello world");
    //     ERR_LOG("%s", str.c_str());
    //     sleep(1);
    // }

    Socket sk;
    sk.bind();
    return 0;
}