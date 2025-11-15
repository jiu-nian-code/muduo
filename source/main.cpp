#include"buffer.hpp"

#include"socket.hpp"

#include"log.hpp"

#include"channel.hpp"

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

    // Socket sk;
    // sk.create_listen_link();
    // Socket newlink(sk.my_accept());
    // char buf[1024];
    // while(1)
    // {
    //     ssize_t ret = newlink.Recv(buf, 1024);
    //     buf[ret] = 0;
    //     std::cout << buf << std::endl;
    //     newlink.Send(buf, 1024);
    // }
    // return 0;

    // buffer buf;
    // std::string str("hello world\n");
    // buf.write_string(str);
    // buf.write_string(str);
    // buf.write_string(str);
    // buf.write_string(str);
    // buf.write_string(str);
    // buf.write_string(str);

    // buffer buf1;
    // buf1.write_buffer(buf);
    // buf1.Clear();
    // buf1.PRINT();

    // std::cout << "------------------------------------------" << std::endl;

    // std::string ret = buf1.read_as_string(1024);
    // std::cout << ret << std::endl;
    // char arry[1024] = {0};
    // int n = 6;
    // while(n--)
    // {
    //     ssize_t ret = buf.GetLine(arry);
    //     arry[ret] = 0;
    //     std::cout << arry << "***" << std::endl;
    // }

    Channel cl(1, nullptr);
    return 0;
}