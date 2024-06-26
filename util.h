#ifndef UTIL_H
#define UTIL_H
#include <fcntl.h>
#include <string>
#include <sys/epoll.h>
#include <vector>

const int SZ = 1024;
const std::string ONLYYOU = "only you in room";

int SetNonBlock(int fd) {
    int old_commands = fcntl(fd, F_GETFL);
    return fcntl(fd, O_NONBLOCK | old_commands);
}

class Epoller {
    int max_todo;
    int fd_epoller;

public:
    std::vector<struct epoll_event> todos;
    explicit Epoller(int sz = 256) {
        fd_epoller = epoll_create(sz);
        max_todo = sz;
        todos.assign(sz, {});
    }

    int Wait() {
        return epoll_wait(fd_epoller, &todos[0], max_todo, -1);
    }

    void Addfd(int fd, bool edge_triger = false) {
        struct epoll_event ev {
            0
        };
        ev.data.fd = fd;
        ev.events = EPOLLIN;
        if (edge_triger) {
            ev.events |= EPOLLET;
            SetNonBlock(fd);
        }
        epoll_ctl(fd_epoller, EPOLL_CTL_ADD, fd, &ev);
    }

    void Modfd(int fd, int flag, bool edge_triger = false) {
        struct epoll_event ev {
            0
        };
        ev.events = flag;
        ev.data.fd = fd;
        if (edge_triger) ev.events |= EPOLLET;
        epoll_ctl(fd_epoller, EPOLL_CTL_MOD, fd, &ev);
    }

    void Delfd(int fd) {
        epoll_ctl(fd_epoller, EPOLL_CTL_DEL, fd, nullptr);
    }
};

#endif