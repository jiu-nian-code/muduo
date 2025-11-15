#include<iostream>

#include<functional>

#include <sys/epoll.h>

class EventLoop;

class Channel
{
    int _fd;
    uint32_t _events; // 监控的事件
    uint32_t _revents; // 当前触发的事件
    EventLoop* _el;
    using Event_Callback = std::function<void()>;
    Event_Callback _read_callback;
    Event_Callback _write_callback;
    Event_Callback _error_callback;
    Event_Callback _close_callback;
    Event_Callback _event_callback;
public:
    Channel(int fd, EventLoop* el) : _fd(fd), _el(el)
    {}

    void Set_Read_Callback(Event_Callback& cb) { _read_callback = cb; }

    void Set_Write_Callback(Event_Callback& cb) { _write_callback = cb; }

    void Set_Error_Callback(Event_Callback& cb) { _error_callback = cb; }

    void Set_Close_Callback(Event_Callback& cb) { _close_callback = cb; }

    void Set_Event_Callback(Event_Callback& cb) { _event_callback = cb; }

    bool Read_Able() {return (_events & EPOLLIN) ? true : false;}

    bool Write_Able() {return (_events & EPOLLOUT) ? true : false;}

    void Set_Read_Able() { _events |= EPOLLIN; }

    void Set_Write_Able() { _events |= EPOLLOUT; }

    void Reset_Read_Able() { _events &= ~(EPOLLIN); }

    void Reset_Write_Able() { _events |= ~(EPOLLOUT); }

    void Set_Revents(uint32_t event) { _revents = event; }

    void Disable_All() { _events = 0; }

    //移除监控
    void Remove(); // TODO

    //事件处理，一旦连接触发了事件，就调用这个函数，自己触发了什么事件如何处理自己决定
    void Update(); // TODO

    void Handle_Event()
    {
        if((_revents & EPOLLIN) || (_revents & EPOLLRDHUP) || (_revents & EPOLLPRI))
        {
            if(_read_callback) _read_callback();
            if(_event_callback) _event_callback();
        }

        if(_revents & EPOLLOUT)
        {
            if(_write_callback) _write_callback();
            if(_event_callback) _event_callback();
        }
        else if(_revents & EPOLLERR)
        {
            if(_error_callback) _error_callback();
        }
        else if(_revents & EPOLLHUP)
        {
            if(_close_callback) _close_callback();
        }
    }
};