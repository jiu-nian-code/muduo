#pragma once

#include<iostream>

#include<vector>

#include<thread>

#include<functional>

#include<mutex>

#include<memory>

#include<unistd.h>

#include <sys/eventfd.h>

#include"poller.hpp"

#include"channel.hpp"

#include"timewheel.hpp"

#include"log.hpp"

class TimerWheel;

class Eventloop
{
    using functor = std::function<void()>;
    std::thread::id thread_id;
    Poller _pl;
    std::vector<functor> _tasks;
    int _eventfd;
    std::unique_ptr<Channel> _event_channel_ptr;
    // Channel* _event_channel_ptr;
    std::mutex _mt;
    TimerWheel _tw;

    void Run_All_Task()
    {
        std::vector<functor> sp;
        {
            std::unique_lock<std::mutex> lock(_mt);
            sp.swap(_tasks);
        }
        for(auto& e : sp) e();
    }

    static int Create_Eventfd()
    {
        int tmp = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if(tmp < 0)
        {
            ERR_LOG("eventfd: %s", strerror(errno));
            return -1;
        }
        return tmp;
    }

    void Read_Eventfd()
    {
        uint64_t get;
        ssize_t ret = read(_eventfd, &get, sizeof(get));
        if(ret < 0)
        {
            ERR_LOG("read: %s", strerror(errno));
            if(errno == EAGAIN || errno == EINTR) return;
            else abort();
        }
    }

    void Weakup_Eventfd()
    {
        uint64_t in = 1;
        ssize_t ret = write(_eventfd, &in, sizeof(in));
        if(ret < 0)
        {
            ERR_LOG("write: %s", strerror(errno));
            if(errno == EINTR) return;
            else abort();
        }
    }
public:
    // int eventfd(unsigned int initval, int flags);
    Eventloop():
        thread_id(std::this_thread::get_id()),
        _eventfd(Create_Eventfd()),
        _event_channel_ptr(new Channel(_eventfd, this)),
        _tw(this, 1)

    {
        _event_channel_ptr->Set_Read_Callback(std::bind(&Eventloop::Read_Eventfd, this));
        _event_channel_ptr->Set_Read_Able();
    }
    
    void Start() // 三板斧
    {
        while(1)
        {
            std::vector<Channel *> arr;
            _pl.Poller_Wait(arr);
    
            for(auto& e : arr)
                e->Handle_Event();
    
            Run_All_Task();
        }
    }

    bool IsInloop() // 判断当前想要执行任务的线程是不是当前的线程
    {
        return thread_id == std::this_thread::get_id();
    }

    void QueueInLoop(const functor& fc) // 压进对应线程的eventloop的任务队列
    {
        {
            std::unique_lock<std::mutex> lock(_mt);
            _tasks.push_back(fc);
        }
        Weakup_Eventfd();
    }

    void Runinloop(const functor& fc)
    {
        if(IsInloop()) fc();
        else QueueInLoop(fc);
    }

    void EL_Del_Event(Channel *cl)
    {
        return _pl.Del_Event(cl);
    }

    void EL_Update(Channel *cl)
    { 
        return _pl.Add_Modify_Event(cl);
    }

    void TimerAdd(uint64_t timer_no, int timeout, TimerCallback tc)
    {
        return _tw.AddTimer(timer_no, timeout, tc);
    }

    void TimerRefresh(uint64_t timer_no)
    {
        return _tw.RefreshTimer(timer_no);
    }

    void TimerCancel(uint64_t timer_no)
    {
        return _tw.CancelTimer(timer_no);
    }

    bool HasTimer(uint64_t timer_no)
    { 
        return _tw.HasTimer(timer_no);
    }
};

//移除监控
void Channel::Remove()
{
    return _el->EL_Del_Event(this);
}

//事件处理，一旦连接触发了事件，就调用这个函数，自己触发了什么事件如何处理自己决定
void Channel::Update()
{
    return _el->EL_Update(this);
}

void TimerWheel::AddTimer(uint64_t timer_no, int timeout, TimerCallback tc)
{
    // void AddTimerInLoop(uint64_t timer_no, int timeout, TimerCallback tc)
    return _el->Runinloop(std::bind(&TimerWheel::AddTimerInLoop, this, timer_no, timeout, tc));
}

void TimerWheel::RefreshTimer(uint64_t timer_no)
{
    return _el->Runinloop(std::bind(&TimerWheel::RefreshTimerInLoop, this, timer_no));
}

void TimerWheel::CancelTimer(uint64_t timer_no)
{
    return _el->Runinloop(std::bind(&TimerWheel::CancelTimerInLoop, this, timer_no));
}