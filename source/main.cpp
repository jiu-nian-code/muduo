#include"buffer.hpp"

#include"socket.hpp"

#include"log.hpp"

#include"channel.hpp"

#include"poller.hpp"

#include"eventloop.hpp"

#include"connection.hpp"

#include"accept.hpp"

#include<iostream>

#include<time.h>

#include<unistd.h>

#include<functional>

#include<unordered_map>

// void myclose(Channel* cl)
// {
//     INF_LOG("do close.");
//     cl->Remove();
//     // delete cl; 暂时不能删
// }

// void myerror(Channel* cl)
// {
//     INF_LOG("do error.");
//     std::cout << "error" << std::endl;
//     myclose(cl);
// }

// void myevent(Channel* cl, Eventloop* el, uint64_t timer_no)
// {
//     INF_LOG("do event.");
//     el->TimerRefresh(timer_no);
//     // std::cout << cl->FD() << std::endl;
// }

// void myread(Channel* cl)
// {
//     INF_LOG("do read.");
//     char buffer[1024] = {0};
//     ssize_t ret = read(cl->FD(), buffer, 1024);
//     if(ret <= 0)
//     {
//         myclose(cl);
//         return;
//     }
//     buffer[ret] = 0;
//     std::cout << buffer << std::endl;
//     cl->Set_Write_Able();
// }

// void mywrite(Channel* cl)
// {
//     INF_LOG("do write.");
//     ssize_t ret = write(cl->FD(), "cnm!", 1024);
//     if(ret <= 0)
//     {
//         myclose(cl);
//         return;
//     }
//     cl->Reset_Write_Able();
// }

std::unordered_map<uint64_t, connect_ptr> arr;

void Connect(const connect_ptr& con_ptr)
{
    std::cout << "get a new link" << std::endl;
}

void message(const connect_ptr& con_ptr, Buffer* bf)
{
    char tmpbuf[65536];
    ssize_t ret = bf->read(tmpbuf, 65535);
    std::cout << tmpbuf << std::endl;
    con_ptr->Send(tmpbuf, ret);
}

void Close(const connect_ptr& con_ptr)
{
    arr.erase(con_ptr->ID());
    std::cout << "close" << std::endl;
}

void Event(const connect_ptr& con_ptr)
{
    std::cout << "event" << std::endl;
}

uint64_t timer_no = 0;

void newconnection(Eventloop* el, int fd)
{
    INF_LOG("accept a link.");

    connect_ptr con(new Connection(timer_no++, fd, el));
    con->Set_Connected_Callback(Connect);
    con->Set_Message_Callback(message);
    con->Set_Server_Closed_Callback(Close);
    con->Set_Anyevent_Callback(Event);
    con->Start_Inactive_Destruction(10);
    con->Stablish();
    arr.insert(make_pair(timer_no, con));
}

int main()
{
    Eventloop el;
    Accept _ap(&el);
    _ap.Set_Accept_Callback(std::bind(&newconnection, &el, std::placeholders::_1));
    _ap.Start_Listen();
    std::cout << 1 << std::endl;
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