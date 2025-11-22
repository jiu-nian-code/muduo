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

    int create_server(const std::string& ip = default_ip, int16_t port = DEFAULT_PORT)
    {
        if(!_sk.create_listen_link(ip, port)) return -1;
        _sk.reuse_address_port();
        return _sk.FD();
    }
public:
    Accept(Eventloop* el, const std::string& ip = default_ip, int16_t port = DEFAULT_PORT):
        _el(el),
        _cl(create_server(ip, port), _el)
    {
        _cl.Set_Read_Callback(std::bind(&Accept::handle_accept, this));
    }

    void Set_Accept_Callback(const accept_callback& cb) { _accept_callback = cb; }

    void Start_Listen() { _cl.Set_Read_Able(); }
};