#include<iostream>

#include<functional>

#include<vector>

#include<memory>

#include<unordered_map>

#include<unistd.h>

using TimerCallback = std::function<void()>;
using ReleaseCallback = std::function<void()>;

class Timer
{
    uint64_t _timer_no; // 任务编号
    int _timeout; // 任务超时时间
    TimerCallback _timer_callback; // 任务执行回调
    ReleaseCallback _release_callback; // 移除timerwheel中的任务
    bool isconcel; // 任务是否被取消
public:
    Timer(uint64_t time_no, int timeout, TimerCallback tc): 
    _timer_no(time_no), 
    _timeout(timeout),
    _timer_callback(tc),
    isconcel(false)
    {}

    void Set_Release(const ReleaseCallback& rc) { _release_callback = rc; }

    int Get_Timeout() { return _timeout; }

    void Cancel() { isconcel = true; }

    ~Timer()
    {
        if(_release_callback) _release_callback(); 
        // 一定要先进行relase，将任务记录消除，因为已经进入析构中途，此时认为任务已经没有了，智能指针已经销毁，
        // 如果任务callback中再次对任务记录进行访问，可能会访问一个正在析构的对象，会产生未定义行为
        if(_timer_callback && !isconcel) _timer_callback();
    }
};

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

    void remove_timer(uint64_t timer_no) // 移除任务记录
    {
        auto pos = _timer_um.find(timer_no);
        if(pos != _timer_um.end()){ _timer_um.erase(pos); }
    }
public:
    TimerWheel() :_tick(0), _capcity(TW_CAP), _timer_arr(_capcity){}

    void AddTimer(uint64_t timer_no, int timeout, TimerCallback tc)
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
        timer->Set_Release(std::bind(&TimerWheel::remove_timer, this, timer_no));

        _timer_um[timer_no] = WeakTimer(timer); // um中不能存shared_ptr，这会导致引用计数 +1，这样shared_ptr永远不能销毁
    }

    void RefreshTimer(uint64_t timer_no)
    {
        auto pos = _timer_um.find(timer_no);
        if(pos == _timer_um.end()){ return; } // 没找到要刷新的事件

        int to = pos->second.lock()->Get_Timeout();
        _timer_arr[(_tick + to) % _capcity].push_back(SharedTimer(pos->second)); // 记得取模
    }

    void Cancel_Timer(uint64_t timer_no)
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

    void run_ontime_task()
    {
        _tick = (_tick + 1) % _capcity; // 取模
        _timer_arr[_tick].clear(); // 指针走到哪清空哪的数组
    }
};

void print()
{
    std::cout << "hello linux" << std::endl;
}

int main()
{
    TimerWheel tw;
    tw.AddTimer(888, 0, &print);

    int sec = 0;
    while(sec != 5)
    {
        sleep(1);
        sec += 1;
        std::cout << sec << "秒过去了" << std::endl;
        tw.run_ontime_task();
        tw.RefreshTimer(888);
        std::cout << "888任务刷新" << std::endl;
    }

    tw.Cancel_Timer(888);
    for(int i = 0; i < 5; ++i)
    {
        sleep(1);
        std::cout << "倒计时" << i + 1 << std::endl;
        tw.run_ontime_task();
    }

    return 0;
}