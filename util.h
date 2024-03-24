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
const std::string CLIENT_SAY = "you > %s\n";

const int BUF_SIZE = 1024;
int setNonBlock(int fd);

class Epoller {
private:
    int epollfd;
    std::vector<struct epoll_event> events;

public:
    explicit Epoller(int sz = 512);

    int addfd(int fd, int newControlFlag, bool ET);

    int delfd(int fd);

    int modfd(int fd, int newControlFlag, bool ET);

    int wait();

    int getfd(size_t i);
    u_int32_t getEvent(size_t i);
};


#endif