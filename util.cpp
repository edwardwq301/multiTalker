#include "util.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

int SetNonBlock(int fd) {
    int oldControlFlag = fcntl(fd, F_GETFL);
    int newControlFlag = oldControlFlag | O_NONBLOCK;
    return fcntl(fd, newControlFlag);
}

Epoller::Epoller(int sz) :
    epollfd(epoll_create(sz)), events(512) {
    // std::cout << epollfd;
}


int Epoller::Addfd(int fd, int newControlFlag, bool ET) {
    struct epoll_event ev = {0};
    ev.events = newControlFlag;
    ev.data.fd = fd;
    if (ET)
        newControlFlag |= EPOLLET;

    epoll_ctl(this->epollfd, EPOLL_CTL_ADD, fd, &ev);
    return SetNonBlock(fd);
}

int Epoller::Modfd(int fd, int newControlFlag, bool ET) {
    struct epoll_event ev = {0};
    ev.events = newControlFlag;
    ev.data.fd = fd;
    if (ET)
        newControlFlag |= EPOLLET;
    return epoll_ctl(this->epollfd, EPOLL_CTL_MOD, fd, &ev);
}

int Epoller::Delfd(int fd) {
    return epoll_ctl(this->epollfd, EPOLL_CTL_DEL, fd, nullptr);
}

int Epoller::Wait() {
    int ret = epoll_wait(epollfd, &events[0], static_cast<int>(events.size()), -1);
    // std::cout << "epoll wait" << ret << std::endl;
    return ret;
}

u_int32_t Epoller::GetEvent(size_t i) {
    return events[i].events;
}

int Epoller::Getfd(size_t i) {
    return events[i].data.fd;
}