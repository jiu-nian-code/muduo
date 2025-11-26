#pragma once

#include<iostream>

#include<functional>

#include<vector>

#include<memory>

#include<unordered_map>

#include<unistd.h>

#include<sys/timerfd.h>

#include<string.h>

#include"log.hpp"

#include"channel.hpp"

using TimerCallback = std::function<void()>;
using ReleaseCallback = std::function<void()>;

class Timer
{
    uint64_t _timer_no; // 任务编号
    int _timeout; // 任务超时时间
    TimerCallback _timer_callback; // 任务执行回调
    ReleaseCallback _release_callback; // 移除timerwheel中的任务
    bool _isconcel; // 任务是否被取消
public:
    Timer(uint64_t time_no, int timeout, TimerCallback tc): 
        _timer_no(time_no), 
        _timeout(timeout),
        _timer_callback(tc),
        _isconcel(false)
    {}

    void Set_Release(const ReleaseCallback& rc) {  _release_callback = rc; }

    int Get_Timeout() {  return _timeout; }

    void Cancel() {  _isconcel = true; }

    ~Timer()
    {
         
        if(_release_callback) _release_callback(); 
        // 一定要先进行relase，将任务记录消除，因为已经进入析构中途，此时认为任务已经没有了，智能指针已经销毁，
        // 如果任务callback中再次对任务记录进行访问，可能会访问一个正在析构的对象，会产生未定义行为
        if(_timer_callback && !_isconcel) _timer_callback();
    }
};

class Eventloop;

class Channel;

#define TW_CAP 60

class TimerWheel
{
    using WeakTimer = std::weak_ptr<Timer>;
    using SharedTimer = std::shared_ptr<Timer>;
    using Bucket = std::vector<SharedTimer>;
    using BucketList = std::vector<Bucket>;
    int _tick; // 时间轮指针
    size_t _capcity; // 时间轮数组尺寸
    std::unordered_map<uint64_t, WeakTimer> _timer_um; // 任务记录，用于查询任务是否存在
    BucketList _timer_arr; // 时间轮数组
    Eventloop* _el;
    int _timerfd;
    std::unique_ptr<Channel> _timerfd_cl;

    void Remove_Timer(uint64_t timer_no) // 移除任务记录
    {
         
        auto pos = _timer_um.find(timer_no);
        if(pos != _timer_um.end()){ _timer_um.erase(pos); }
    }

    int Create_Timerfd(int timeout)
    {
         
        int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        // int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
        if(timerfd < 0)
        {
            ERR_LOG("%s", strerror(errno));
            return -1;
        }
        struct itimerspec newspec;
        memset(&newspec, 0, sizeof(newspec));
        newspec.it_value.tv_sec = timeout;
        newspec.it_value.tv_nsec = 0;
        newspec.it_interval.tv_sec = timeout;
        newspec.it_interval.tv_nsec = 0;
        timerfd_settime(timerfd, 0, &newspec, nullptr);
        return timerfd;
    }

    void Read_Timerfd()
    {
         
        uint64_t tmp = 0;
        ssize_t ret = read(_timerfd, &tmp, sizeof(tmp));
        if(ret < 0)
        {
            ERR_LOG("%s", strerror(errno));
            return;
        }
        run_ontime_task();
    }

    void AddTimerInLoop(uint64_t timer_no, int timeout, TimerCallback tc)
    {
         
        if(timeout == 0)
        {
            tc();
            return;
        }

        SharedTimer timer(new Timer(timer_no, timeout, tc)); // 利用智能指针的引用计数自动析构来自动调用任务处理
        if(!timer)
        {
            perror("AddTimer: shared_ptr error");
            return;
        }

        _timer_arr[(_tick + timeout) % _capcity].push_back(timer);
        timer->Set_Release(std::bind(&TimerWheel::Remove_Timer, this, timer_no));

        _timer_um[timer_no] = WeakTimer(timer); // um中不能存shared_ptr，这会导致引用计数 +1，这样shared_ptr永远不能销毁
    }

    void RefreshTimerInLoop(uint64_t timer_no)
    {
         
        auto pos = _timer_um.find(timer_no);
        if(pos == _timer_um.end()){ return; } // 没找到要刷新的事件

        int to = pos->second.lock()->Get_Timeout();
        _timer_arr[(_tick + to) % _capcity].push_back(SharedTimer(pos->second)); // 记得取模
    }

    void CancelTimerInLoop(uint64_t timer_no)
    {
         
        auto pos = _timer_um.find(timer_no);
        if(pos == _timer_um.end())
        {
            perror("Cancel_Timer: unordered_map: find error");
            return;
        }
        SharedTimer st = pos->second.lock();
        if(st) st->Cancel();
    }
public:
    TimerWheel(Eventloop* el, int timeout) :
        _tick(0),
        _capcity(TW_CAP),
        _timer_arr(_capcity),
        _el(el),
        _timerfd(Create_Timerfd(timeout)),
        _timerfd_cl(new Channel(_timerfd, _el))
    {
         
        _timerfd_cl->Set_Read_Callback(std::bind(&TimerWheel::Read_Timerfd, this));
        _timerfd_cl->Set_Read_Able();
    }

    void AddTimer(uint64_t timer_no, int timeout, TimerCallback tc);

    void RefreshTimer(uint64_t timer_no);

    void CancelTimer(uint64_t timer_no);

    void run_ontime_task()
    {
         
        _tick = (_tick + 1) % _capcity; // 取模
        _timer_arr[_tick].clear(); // 指针走到哪清空哪的数组
    }

    // !!!线程不安全
    bool HasTimer(uint64_t timer_no)
    {
         
        auto it = _timer_um.find(timer_no);
        if(it != _timer_um.end()) return true;
        else return false;
    }
};

// void print()
// {
//     std::cout << "hello linux" << std::endl;
// }

// int main()
// {
//     TimerWheel tw;
//     tw.AddTimer(888, 0, &print);

//     int sec = 0;
//     while(sec != 5)
//     {
//         sleep(1);
//         sec += 1;
//         std::cout << sec << "秒过去了" << std::endl;
//         tw.run_ontime_task();
//         tw.RefreshTimer(888);
//         std::cout << "888任务刷新" << std::endl;
//     }

//     tw.Cancel_Timer(888);
//     for(int i = 0; i < 5; ++i)
//     {
//         sleep(1);
//         std::cout << "倒计时" << i + 1 << std::endl;
//         tw.run_ontime_task();
//     }

//     return 0;
// }