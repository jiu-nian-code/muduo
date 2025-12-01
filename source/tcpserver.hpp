#pragma once

#include<signal.h>

#include"connection.hpp"

#include"threadloop.hpp"

#include"accept.hpp"

class Tcpserver
{
    Eventloop _mainloop;
    Accept _ap;
    ThreadloopPool _tlp;
    int _tlpNum;
    bool _enable_inactive_destruction;
    int _timeout;
    uint64_t _con_id;
    std::unordered_map<uint64_t, connect_ptr> _um;

    using connected_callback = std::function<void(const connect_ptr&)>;
    using message_callback = std::function<void(const connect_ptr&, Buffer*)>;
    using closed_callback = std::function<void(const connect_ptr&)>;
    using anyevent_callback = std::function<void(const connect_ptr&)>;

    connected_callback _connected_callback;
    message_callback _message_callback;
    closed_callback _closed_callback;
    anyevent_callback _anyevent_callback;

    void Destory_Connection_In_Loop(const connect_ptr& con_ptr)
    {
        std::cout << "erase" << std::endl;
        int id = con_ptr->ID();
        auto it = _um.find(id);
        if(it != _um.end()) _um.erase(it);
    }

    void Destory_Connection(const connect_ptr& con_ptr)
    {
        _mainloop.Runinloop(std::bind(&Tcpserver::Destory_Connection_In_Loop, this, con_ptr));
    }

    void NewConnection(int fd)
    {
        DBG_LOG("accept a link.");
    
        connect_ptr con(new Connection(_con_id, fd, _tlp.NextLoop()));
        con->Set_Connected_Callback(_connected_callback);
        con->Set_Message_Callback(_message_callback);
        con->Set_Closed_Callback(_closed_callback);
        // con->Set_Anyevent_Callback(_anyevent_callback);
        con->Set_Server_Closed_Callback(std::bind(&Tcpserver::Destory_Connection, this, std::placeholders::_1));
        if(_enable_inactive_destruction)
        {
            con->Start_Inactive_Destruction(_timeout);
        }
        con->Establish();
        _um.insert(make_pair(_con_id, con));
        ++_con_id;
    }

    // void NewConnection(int fd)
    // {
    //     _mainloop.Runinloop(std::bind(&Tcpserver::NewConnection_In_Loop, this, fd));
    // }

    void Add_Timeout_Task_In_Loop(int timeout, const TimerCallback& tc)
    {
        _mainloop.TimerAdd(_con_id++, timeout, tc);
    }

public:
    Tcpserver(uint16_t port = DEFAULT_PORT, const std::string& ip = default_ip) :
        _ap(&_mainloop, DEFAULT_PORT),
        _tlp(&_mainloop),
        _tlpNum(0),
        _enable_inactive_destruction(false),
        _timeout(0),
        _con_id(0)
    {
        _ap.Set_Accept_Callback(std::bind(&Tcpserver::NewConnection, this, std::placeholders::_1));
        _ap.Start_Listen();
    }

    // 设置线程池数量
    void Set_Threadloop_Num(int num) {  _tlp.Set_Thread_Num(num); }
    // 设置各个回调
    void Set_Connected_Callback(const connected_callback& cb) {  _connected_callback = cb; }

    void Set_Message_Callback(const message_callback& cb) {  _message_callback = cb; }

    void Set_Closed_Callback(const closed_callback& cb) {  _closed_callback = cb; }

    // void Set_Anyevent_Callback(const anyevent_callback& cb) {  _anyevent_callback = cb; }
    // 设置非活跃链接销毁
    void Start_Inactive_Destruction(int timeout) {  _enable_inactive_destruction = true; _timeout = timeout; }
    // 启动线程池
    void Start()
    {
        _tlp.Init();
        _mainloop.Start();
    }
    // 添加定时任务
    void Add_Timeout_Task(int timeout, const TimerCallback& tc)
    {
        _mainloop.Runinloop(std::bind(&Tcpserver::Add_Timeout_Task_In_Loop, this, timeout, tc));
    }
};

struct Network // 防止连接断开被发送SIGPIPE信号终止
{
    Network()
    {
        signal(SIGPIPE, SIG_IGN);
        DBG_LOG("SIGPIPE INIT");
    }
};

static Network net;