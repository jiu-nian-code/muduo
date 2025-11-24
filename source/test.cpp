#include"server.hpp"

std::unordered_map<uint64_t, PtrConnection> um;
LoopThreadPool* tlp;

void Connect(const PtrConnection& con_ptr)
{
    std::cout << "get a new link" << std::endl;
}

void message(const PtrConnection& con_ptr, Buffer* bf)
{
    char tmpbuf[65536];
    bf->ReadAndPop(tmpbuf, bf->ReadAbleSize());
    std::cout << tmpbuf << std::endl;
}

void Close(const PtrConnection& con_ptr)
{
    um.erase(con_ptr->Id());
    std::cout << "close" << std::endl;
}

void Event(const PtrConnection& con_ptr)
{
    std::cout << "event" << std::endl;
}

uint64_t timer_no = 0;

void newconnection(int fd)
{
    INF_LOG("accept a link.");

    PtrConnection con(new Connection(tlp->NextLoop(), timer_no, fd));
    con->SetConnectedCallback(Connect);
    con->SetMessageCallback(message);
    con->SetSrvClosedCallback(Close);
    con->SetAnyEventCallback(Event);
    con->EnableInactiveRelease(10);
    con->Established();
    um.insert(make_pair(timer_no, con));
    ++timer_no;
}

int main()
{
    EventLoop mel;
    tlp = new LoopThreadPool(&mel);
    tlp->SetThreadCount(0);
    // std::cout << 1 << std::endl;
    tlp->Create();
    // std::cout << 1 << std::endl;
    Acceptor _ap(&mel, (uint16_t)9091);
    _ap.SetAcceptCallback(std::bind(&newconnection, std::placeholders::_1));
    _ap.Listen();
    // std::cout << 1 << std::endl;
    mel.Start();
    return 0;
}