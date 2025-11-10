#include<iostream>

#include<sys/timerfd.h>

#include<string.h>

#include<unistd.h>

#include<time.h>

int main()
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);

    itimerspec newspec;
    memset((void*)&newspec, 1, sizeof(itimerspec));
    newspec.it_value.tv_sec = 3;
    newspec.it_value.tv_nsec = 0;
    newspec.it_interval.tv_sec = 1;
    newspec.it_interval.tv_nsec = 0;
    timerfd_settime(tfd, 0, &newspec, nullptr);

    time_t start = time(nullptr);
    while(1)
    {
        uint64_t tmp;
        ssize_t ret = read(tfd, &tmp, sizeof(tmp));

        std::cout << tmp << " " << time(nullptr) - start << std::endl;
    }

    close(tfd);
    return 0;
}