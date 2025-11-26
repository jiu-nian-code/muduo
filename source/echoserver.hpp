#pragma once

#include"tcpserver.hpp"


class Echo_server
{
    Tcpserver _server;

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
    
public:
    Echo_server()
    {
        _server.Set_Connected_Callback(std::bind(&Echo_server::Connect, this, std::placeholders::_1));
        _server.Set_Message_Callback(std::bind(&Echo_server::Message, this, std::placeholders::_1, std::placeholders::_2));
        _server.Set_Closed_Callback(std::bind(&Echo_server::Close, this, std::placeholders::_1));
        // _server.Set_Anyevent_Callback(std::bind(&Echo_server::Event, this, std::placeholders::_1));
        _server.Set_Threadloop_Num(2);
        _server.Start_Inactive_Destruction(5);
    }

    void start() { _server.Start(); }
};