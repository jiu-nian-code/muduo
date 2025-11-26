#pragma once

#include<mutex>

#include <condition_variable>

#include<thread>

#include"eventloop.hpp"

class Threadloop
{
    Eventloop* _el;
    std::mutex _mt;
    std::condition_variable _cv;
    std::thread _th;

    void handle_thread()
    {
        Eventloop el;
        {
            std::unique_lock<std::mutex> lock(_mt);
            _el = &el;
            _cv.notify_all();
        }
        _el->Start();
    }

public:
    Threadloop() : _el(nullptr), _th(&Threadloop::handle_thread, this) {}

    Eventloop* GetEventloop()
    {
        Eventloop* tmpel = nullptr;
        {
            std::unique_lock<std::mutex> lock(_mt);
            _cv.wait(lock, [&](){ return _el != nullptr; });
            tmpel = _el;
        }
        return tmpel;
    }
};

class ThreadloopPool
{
    int _thread_num = 0;
    Eventloop* _mainloop;
    std::vector<Threadloop*> _thread_arr;
    std::vector<Eventloop*> _loop_arr;
    int _next = 0;
public:
    ThreadloopPool(Eventloop* ml) : _mainloop(ml) {  }

    void Set_Thread_Num(int num) {  _thread_num = num; }

    void Init()
    {
        if(_thread_num == 0) return;
        _thread_arr.resize(_thread_num);
        _loop_arr.resize(_thread_num);
        for(int i = 0; i < _thread_num; ++i)
        {
            _thread_arr[i] = new Threadloop();
            _loop_arr[i] = _thread_arr[i]->GetEventloop();
        }
    }

    Eventloop* NextLoop()
    {
        if(_thread_num == 0) return _mainloop;
        int ret = _next;
        (++_next) %= _thread_num;
        return _loop_arr[ret];
    }
};