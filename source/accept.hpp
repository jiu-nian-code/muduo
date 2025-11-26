#pragma once

#include"connection.hpp"

class Accept
{
    Eventloop* _el;
    Socket _sk;
    Channel _cl;
    using accept_callback = std::function<void(int)>;
    accept_callback _accept_callback;

    void handle_accept()
    {
        int newfd = _sk.my_accept();
        if(newfd < 0) return;
        if(_accept_callback) _accept_callback(newfd);
    }

    int create_server(uint16_t port = DEFAULT_PORT, const std::string& ip = default_ip)
    {
        if(!_sk.create_listen_link(port, ip)) return -1;
        return _sk.FD();
    }
public:
    Accept(Eventloop* el, uint16_t port = DEFAULT_PORT, const std::string& ip = default_ip):
        _el(el),
        _cl(create_server(port, ip), _el)
    {
        _cl.Set_Read_Callback(std::bind(&Accept::handle_accept, this));
    }

    void Set_Accept_Callback(const accept_callback& cb) { _accept_callback = cb; }

    void Start_Listen() { _cl.Set_Read_Able(); }
};