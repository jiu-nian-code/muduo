#include"buffer.hpp"

#include"socket.hpp"

#include"log.hpp"

#include"channel.hpp"

#include"poller.hpp"

#include"eventloop.hpp"

#include<iostream>

#include<time.h>

#include<unistd.h>

#include<functional>

void myclose(Channel* cl)
{
    INF_LOG("do close.");
    cl->Remove();
}

void myerror(Channel* cl)
{
    INF_LOG("do error.");
    std::cout << "error" << std::endl;
    myclose(cl);
}

void myevent(Channel* cl)
{
    INF_LOG("do event.");
    std::cout << cl->FD() << std::endl;
}

void myread(Channel* cl)
{
    INF_LOG("do read.");
    char buffer[1024] = {0};
    ssize_t ret = read(cl->FD(), buffer, 1024);
    if(ret <= 0)
    {
        myclose(cl);
        return;
    }
    buffer[ret] = 0;
    std::cout << buffer << std::endl;
    cl->Set_Write_Able();
}

void mywrite(Channel* cl)
{
    INF_LOG("do write.");
    ssize_t ret = write(cl->FD(), "cnm!", 1024);
    if(ret <= 0)
    {
        myclose(cl);
        return;
    }
    cl->Reset_Write_Able();
}

void Accepter(Channel* cl, Eventloop* el)
{
    INF_LOG("accept a link.");
    int fd = cl->FD();
    int newfd = accept(fd, nullptr, nullptr);
    Channel* newcl = new Channel(newfd, el);
    newcl->Set_Close_Callback(std::bind(myclose, newcl));
    newcl->Set_Error_Callback(std::bind(myerror, newcl));
    newcl->Set_Event_Callback(std::bind(myevent, newcl));
    newcl->Set_Read_Callback(std::bind(myread, newcl));
    newcl->Set_Write_Callback(std::bind(mywrite, newcl));

    newcl->Set_Read_Able();
}

int main()
{
    Socket sk;
    Eventloop el;
    sk.create_listen_link();
    sk.reuse_address_port();
    Channel* cl = new Channel(sk.FD(), &el);
    cl->Set_Read_Callback(std::bind(Accepter, cl, &el));
    cl->Set_Read_Able();

    el.Start();

    return 0;
}

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