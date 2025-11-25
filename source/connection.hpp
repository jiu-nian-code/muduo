#pragma once

#include<iostream>

#include<functional>

#include<memory>

#include<assert.h>

#include"channel.hpp"

#include"socket.hpp"

#include"log.hpp"

#include"buffer.hpp"

#include"any.hpp"

#include"eventloop.hpp"


typedef enum
{
    CONNECTING,
    CONNECTED,
    CANCELCONNECTING,
    CANCELCONNECTED
} Con_Sta;

// class Eventloop;
class Connection;
using connect_ptr = std::shared_ptr<Connection>;

class Connection : public std::enable_shared_from_this<Connection>
{
    uint64_t _con_id; // Connection类的id
    // uint64_t timer_no; // 定时器任务id
    int _skfd; // 连接套接字fd
    bool _enable_inactive_destruction;
    Eventloop* _el; // 一定得是指针，有大门道
    Con_Sta _cs;
    Socket _sk; // 套接字类，管理上面的套接字
    Channel _cl;
    Buffer _inbuf;
    Buffer _outbuf;
    Any _context;

    using connected_callback = std::function<void(const connect_ptr&)>;
    using message_callback = std::function<void(const connect_ptr&, Buffer*)>;
    using closed_callback = std::function<void(const connect_ptr&)>;
    using anyevent_callback = std::function<void(const connect_ptr&)>;

    connected_callback _connected_callback;
    message_callback _message_callback;
    closed_callback _closed_callback;
    anyevent_callback _anyevent_callback;

    closed_callback _server_closed_callback;

    void Handle_Read()
    {
        char tmp[65536] = {0};
        ssize_t ret = _sk.Recv_NoBlock(tmp, 65535);
        if(ret < 0)
        {
            // std::cout << strerror(errno) << std::endl;
            return Shutdown_In_Loop();
        }
        _inbuf.wirte(tmp, ret);
        // std::cout << "引用次数: " << shared_from_this().use_count() << std::endl;
        if(_inbuf.effective_read_area() > 0) 
            _message_callback(shared_from_this(), &_inbuf);
    }

    void Handle_Write()
    {
        ssize_t ret = _sk.Send_NoBlock(_outbuf.read_position(), _outbuf.effective_read_area());
        if(ret < 0)
        {
            if(_inbuf.effective_read_area() > 0)
            {
                _message_callback(shared_from_this(), &_inbuf);
            }
            return Release();
        }
        _outbuf.Move_Read_Loc(ret);

        if(_outbuf.effective_read_area() == 0)
        {
            _cl.Reset_Write_Able();
            if(_cs == CANCELCONNECTING) return Release();
        }
        return;
    }

    void Handle_Close()
    {
        if(_inbuf.effective_read_area() > 0) 
            _message_callback(shared_from_this(), &_inbuf);
        return Release();
    }

    void Handle_Error()
    {
        return Handle_Close();
    }

    void Handle_Event()
    {
        if(_enable_inactive_destruction && _el->HasTimer(_con_id)) _el->TimerRefresh(_con_id);
        if(_anyevent_callback) _anyevent_callback(shared_from_this());
    }

    void Establish_In_Loop()
    {
        if(_cs != CONNECTING) 
        {
            ERR_LOG("Establish error, connect has not start!");
            return;
        }
        _cs = CONNECTED;
        _cl.Set_Read_Able();
        if(_connected_callback) _connected_callback(shared_from_this());
    }

    void Release_In_Loop()
    {
        _cs = CANCELCONNECTED;
            
        _sk.Close();
        _cl.Remove();
        if(_el->HasTimer(_con_id)) Cancel_Inactive_Destruction_In_Loop();
        if(_closed_callback) _closed_callback(shared_from_this());
        if(_server_closed_callback) _server_closed_callback(shared_from_this());
    }

    void Send_In_Loop(Buffer& buf)
    {
        if(_cs == CANCELCONNECTED)
        {
            ERR_LOG("connect is cancel.");
            return;
        }
        _outbuf.write_buffer(buf);
        if(!_cl.Write_Able()) _cl.Set_Write_Able();
    }

    void Shutdown_In_Loop()
    {
        _cs = CANCELCONNECTING;
        if(_inbuf.effective_read_area() > 0 && _message_callback) 
            _message_callback(shared_from_this(), &_inbuf);
        if(_outbuf.effective_read_area() > 0 && !_cl.Write_Able())
            _cl.Set_Write_Able();
        if(_outbuf.effective_read_area() == 0) Release();
    }

    void Start_Inactive_Destruction_In_Loop(int timeout)
    {
        _enable_inactive_destruction = true;
        if(_el->HasTimer(_con_id)) _el->TimerRefresh(_con_id);
        else _el->TimerAdd(_con_id, timeout, std::bind(&Connection::Release, this));
    }

    void Cancel_Inactive_Destruction_In_Loop()
    {
        _enable_inactive_destruction = false;
        if(_el->HasTimer(_con_id)) _el->TimerCancel(_con_id);
    }

    void UpGrade_In_Loop(const Any& ay, 
            const connected_callback& con_c, 
            const message_callback& mess_c, 
            const closed_callback& clo_c,
            const anyevent_callback& any_c)
    {
        _context = ay;
        _connected_callback = con_c;
        _message_callback = mess_c;
        _closed_callback = clo_c;
        _anyevent_callback = any_c;
    }

public:
    Connection(uint64_t con_id, int skfd, Eventloop* el):
        _con_id(con_id),
        _skfd(skfd),
        _enable_inactive_destruction(false),
        _el(el),
        _cs(CONNECTING),
        _sk(_skfd),
        _cl(_skfd, _el)
    {
        _cl.Set_Read_Callback(std::bind(&Connection::Handle_Read, this));
        _cl.Set_Write_Callback(std::bind(&Connection::Handle_Write, this));
        _cl.Set_Close_Callback(std::bind(&Connection::Handle_Close, this));
        _cl.Set_Error_Callback(std::bind(&Connection::Handle_Error, this));
        _cl.Set_Event_Callback(std::bind(&Connection::Handle_Event, this));
    }

    uint64_t ID() { return _con_id; }

    int FD() { return _skfd; }

    bool IsConnected() { return _cs == CONNECTED; }

    void SetContext(const Any &context) { _context = context; }

    Any *GetContext() { return &_context; }

    void Set_Connected_Callback(const connected_callback& cb) { _connected_callback = cb; }

    void Set_Message_Callback(const message_callback& cb) { _message_callback = cb; }

    void Set_Closed_Callback(const closed_callback& cb) { _closed_callback = cb; }

    void Set_Anyevent_Callback(const anyevent_callback& cb) { _anyevent_callback = cb; }

    void Set_Server_Closed_Callback(const closed_callback& cb) {_server_closed_callback = cb; }

    void Establish()
    {
        _el->Runinloop(std::bind(&Connection::Establish_In_Loop, this));
    }

    void Send(const char* buf, size_t sz)
    {
        Buffer tmp;
        tmp.wirte(buf, sz);
        _el->Runinloop(std::bind(&Connection::Send_In_Loop, this, std::move(tmp)));
    }   

    void Shutdown()
    {
        _el->Runinloop(std::bind(&Connection::Shutdown_In_Loop, this));
    }

    void Release()
    {
        _el->QueueInLoop(std::bind(&Connection::Release_In_Loop, this));
    }

    void UpGrade(const Any& ay, 
        const connected_callback& con_c, 
        const message_callback& mess_c, 
        const closed_callback& clo_c,
        const anyevent_callback& any_c)
    {
        if(!_el->IsInloop())
        {
            ERR_LOG("not in eventloop thread.");
            return;
        }
        _el->Runinloop(std::bind(&Connection::UpGrade_In_Loop, this, ay, con_c, mess_c, clo_c, any_c));
    }

    void Start_Inactive_Destruction(int timeout)
    {
        _el->Runinloop(std::bind(&Connection::Start_Inactive_Destruction_In_Loop, this, timeout));
    }

    void Cancel_Inactive_Destruction()
    {
        _el->Runinloop(std::bind(&Connection::Cancel_Inactive_Destruction_In_Loop, this));
    }

    ~Connection()
    {
        DBG_LOG("release connection: %p", this);
    }
};