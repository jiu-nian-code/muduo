#include"channel.hpp"

#include"log.hpp"

#define NEW_LV 0

#include<unordered_map>

#include<string.h>

#define MAX_READYEVENTS 1024
class Poller
{
    int _epfd;
    std::unordered_map<int, Channel*> _mp;
    struct epoll_event _revs[MAX_READYEVENTS];
public:
    Poller()
    {
        _epfd = epoll_create(1);
        if(_epfd < 0)
        {
            ERR_LOG("epoll_create: %s", strerror(errno));
            return;
        }
    }

    bool Add_Modify_Event(int fd, uint32_t events)
    {
        struct epoll_event ee;
        ee.events = events;
        ee.data.fd = fd;
        if(_mp.count(fd) == 0) { return epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ee) < 0 ? false : true; }
        else { return epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ee) < 0 ? false : true; }
    }

    bool Del_Event(int fd) { return epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr) < 0 ? false : true;}
};