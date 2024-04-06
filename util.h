#ifndef UTIL_H
#define UTIL_H
#include <cstddef>
#include <string>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <vector>

const std::string EXIT = "EXIT";
const std::string SHOW_WELCOME_ID = "Welcome join the room, ID is %d\n";
const std::string CLIENT_SAY = "you <- %s\n";
const std::string CLIENT_RECV = "from %d -> %s\n";
const std::string ONLY_USER = "you are the only user\n";
const int BUF_SIZE = 1024;


int SetNonBlock(int fd);

class Epoller {
private:
    int epollfd;
    std::vector<struct epoll_event> events;

public:
    explicit Epoller(int sz = 512);

    int Addfd(int fd, int newControlFlag, bool ET);

    int Delfd(int fd);

    int Modfd(int fd, int newControlFlag, bool ET);

    int Wait();

    int Getfd(size_t i);
    u_int32_t GetEvent(size_t i);
};


#endif