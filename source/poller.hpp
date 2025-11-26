#pragma once

#include"channel.hpp"

#include"log.hpp"

#define NEW_LV 0

#include<unordered_map>

#include<string.h>

#include<vector>

#define MAX_READYEVENTS 1024

#define DEFAULT_TIMEOUT -1

class Poller
{
    int _epfd;
    std::unordered_map<int, Channel*> _mp;
    struct epoll_event _revs[MAX_READYEVENTS];

    void Update(Channel* cl, int op)
    {
        int fd = cl->FD();
        struct epoll_event ee;
        ee.events = cl->EVENTS();
        ee.data.fd = fd;
        if(epoll_ctl(_epfd, op, fd, &ee) < 0)
        {
            // std::cout << "出错了: " << _epfd << " " << op << " " << fd << " " << ee.data.fd << std::endl;
            ERR_LOG("epoll_ctl: %s", strerror(errno));
        }
    }

public:
    Poller()
    {
        _epfd = epoll_create(1);
        if(_epfd < 0)
        {
            ERR_LOG("epoll_create: epoll_create: %s", strerror(errno));
            return;
        }
    }

    void Add_Modify_Event(Channel* cl)
    {
        int fd = cl->FD();
        if(_mp.count(fd) == 0)
        {
            _mp[fd] = cl;
            Update(cl, EPOLL_CTL_ADD);
        }
        else Update(cl, EPOLL_CTL_MOD);
    }

    void Del_Event(Channel* cl)
    {
        int fd = cl->FD();
        auto it = _mp.find(fd);
        if(it != _mp.end())
        {
            _mp.erase(it);
            Update(cl, EPOLL_CTL_DEL);
        }
    }

    void Poller_Wait(std::vector<Channel*>& active)
    {
        // int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
        int num = epoll_wait(_epfd, _revs, MAX_READYEVENTS, DEFAULT_TIMEOUT);

        for(int i = 0; i < num; ++i)
        {
            auto it = _mp.find(_revs[i].data.fd);
            if(it == _mp.end())
            {
                ERR_LOG("find error");
                return;
            }
            it->second->Set_Revents(_revs[i].events);
            active.push_back(it->second);
        }
    }

    ~Poller()
    {
        std::cout << "poller distory" << std::endl;
    }
};