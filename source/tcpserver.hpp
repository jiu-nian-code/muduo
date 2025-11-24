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
    std::unordered_map<uint64_t, const connect_ptr&> _um;

    using connected_callback = std::function<void(const connect_ptr&)>;
    using message_callback = std::function<void(const connect_ptr&, Buffer*)>;
    using closed_callback = std::function<void(const connect_ptr&)>;
    using anyevent_callback = std::function<void(const connect_ptr&)>;

    connected_callback _connected_callback;
    message_callback _message_callback;
    closed_callback _closed_callback;
    anyevent_callback _anyevent_callback;

    void Destory_Connection(const connect_ptr& con_ptr)
    {
        int fd = con_ptr->FD();
        auto it = _um.find(fd);
        if(it != _um.end()) _um.erase(it);
    }

    void NewConnection(int fd)
    {
        INF_LOG("accept a link.");
    
        connect_ptr con(new Connection(_con_id, fd, _tlp.NextLoop()));
        con->Set_Connected_Callback(_connected_callback);
        con->Set_Message_Callback(_message_callback);
        con->Set_Closed_Callback(_closed_callback);
        con->Set_Anyevent_Callback(_anyevent_callback);
        con->Set_Server_Closed_Callback(std::bind(&Tcpserver::Destory_Connection, this, std::placeholders::_1));
        con->Start_Inactive_Destruction(10);
        con->Stablish();
        _um.insert(make_pair(_con_id, con));
        ++_con_id;
    }
public:
    Tcpserver(const std::string& ip = default_ip, int16_t port = DEFAULT_PORT) :
        _ap(&_mainloop, ip, port),
        _tlp(&_mainloop),
        _tlpNum(0),
        _enable_inactive_destruction(false),
        _timeout(0),
        _con_id(0)
    {
        _ap.Set_Accept_Callback(std::bind(&Tcpserver::NewConnection, this, std::placeholders::_1));
    }

    // 设置线程池数量
    void Set_Threadloop_Num(int num) { _tlpNum = num; }
    // 设置各个回调
    void Set_Connected_Callback(const connected_callback& cb) { _connected_callback = cb; }

    void Set_Message_Callback(const message_callback& cb) { _message_callback = cb; }

    void Set_Closed_Callback(const closed_callback& cb) { _closed_callback = cb; }

    void Set_Anyevent_Callback(const anyevent_callback& cb) { _anyevent_callback = cb; }
    // 设置非活跃链接销毁
    void Start_Inactive_Destruction(int timeout) { _enable_inactive_destruction = true; _timeout = timeout; }
    // 启动线程池
    void Start()
    {
        _ap.Start_Listen();
        _tlp.Set_Thread_Num(_tlpNum);
        _tlp.Init();
        _mainloop.Start();
    }
    // 添加定时任务
    void Add_Timeout_Task(int timeout, const TimerCallback& tc)
    {
        
    }
};