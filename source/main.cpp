#include"buffer.hpp"

#include"socket.hpp"

#include"log.hpp"

#include"channel.hpp"

#include"poller.hpp"

#include"eventloop.hpp"

#include"connection.hpp"

#include"accept.hpp"

#include"threadloop.hpp"

#include"tcpserver.hpp"

#include<iostream>

#include<time.h>

#include<unistd.h>

#include<functional>

#include<unordered_map>

// std::unordered_map<uint64_t, connect_ptr> um;
// ThreadloopPool* tlp;

void Connect(const connect_ptr& con_ptr)
{
    std::cout << "get a new link" << std::endl;
}

void Message(const connect_ptr& con_ptr, Buffer* bf)
{
    char tmpbuf[65536];
    ssize_t ret = bf->read(tmpbuf, 65535);
    std::cout << tmpbuf << std::endl;
    con_ptr->Send(tmpbuf, ret);
}

void Close(const connect_ptr& con_ptr)
{
    std::cout << "close" << std::endl;
}

void Event(const connect_ptr& con_ptr)
{
    std::cout << "event" << std::endl;
}

// uint64_t timer_no = 0;

// void newconnection(int fd)
// {
//     INF_LOG("accept a link.");

//     connect_ptr con(new Connection(timer_no, fd, tlp->NextLoop()));
//     con->Set_Connected_Callback(Connect);
//     con->Set_Message_Callback(message);
//     con->Set_Closed_Callback(Close);
//     con->Set_Anyevent_Callback(Event);
//     con->Start_Inactive_Destruction(10);
//     con->Stablish();
//     um.insert(make_pair(timer_no, con));
//     ++timer_no;
// }

// int main()
// {
//     Eventloop mel;
//     tlp = new ThreadloopPool(&mel);
//     tlp->Set_Thread_Num(0);
//     // std::cout << 1 << std::endl;
//     tlp->Init();
//     // std::cout << 1 << std::endl;
//     Accept _ap(&mel);
//     _ap.Set_Accept_Callback(std::bind(&newconnection, std::placeholders::_1));
//     _ap.Start_Listen();
//     // std::cout << 1 << std::endl;
//     mel.Start();
//     return 0;
// }

int main()
{
    Tcpserver ts;
    ts.Set_Connected_Callback(Connect);
    ts.Set_Message_Callback(Message);
    ts.Set_Closed_Callback(Close);
    ts.Set_Anyevent_Callback(Event);
    ts.Set_Threadloop_Num(10);
    ts.Start_Inactive_Destruction(3);
    ts.Start();
}