#include<iostream>

#include<functional>

#include<memory>

#include"channel.hpp"

#include"socket.hpp"

#include"log.hpp"

#include"buffer.hpp"

#include"any.hpp"

typedef enum
{
    CONNECTING,
    CONNECTED,
    CANCELCONNECTING,
    CANCELCONNECTED
} Con_Sta;

class Eventloop;

class Connection
{
    // Socket _sk; // 套接字类
    uint64_t _con_id; // Connection类的id
    int _skfd; // 连接套接字fd
    // uint64_t timer_id; // 定时器id

    bool _enable_inactive_destruction;
    Con_Sta _cs;
    Eventloop* _el;
    Channel _cl;
    Buffer _inbuf;
    Buffer _outbuf;
    Any _context;

    using connect_ptr = std::shared_ptr<Channel>;
    using connected_callback = std::function<void(connect_ptr&)>;
    using message_callback = std::function<void(connect_ptr&, Buffer&)>;
    using closed_callback = std::function<void(connect_ptr&)>;
    using anyevent_callback = std::function<void(connect_ptr&)>;

    void 
public:
    Connection(uint64_t con_id, int skfd, Eventloop* el):
        _con_id(con_id),
        _skfd(skfd),
        _el(el),
        _cl(skfd, _el)
    {
    }


};