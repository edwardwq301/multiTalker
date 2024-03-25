#include "util.h"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

using namespace std;
class client {
    struct sockaddr_in serverAddre;
    int clientSockFd;
    string words;
    bool close_flag = false;
    char message[BUF_SIZE];
    char sendbuf[BUF_SIZE];

public:
    Epoller epollID;
    client() :
        epollID() {}

    void init(const string server_address, uint32_t server_port) {
        serverAddre.sin_addr.s_addr = inet_addr(server_address.c_str());
        serverAddre.sin_port = htons(server_port);
        serverAddre.sin_family = AF_INET;
        clientSockFd = socket(AF_INET, SOCK_STREAM, 0);

        if (clientSockFd == -1) {
            perror("clinet: create socket failed\n");
            exit(-1);
        }
    }

    int connectServer() {
        int ret = connect(clientSockFd, (sockaddr *)&serverAddre, sizeof(serverAddre));

        if (-1 == ret) {
            perror("client:error\n");
            exit(-1);
        }

        if (recv(clientSockFd, message, BUF_SIZE, 0) > 0)
            printf("%s", message);
        printf("you ->");
        return ret;
    }

    int send() {
        printf("you -> ");

        fgets(sendbuf, BUF_SIZE, stdin);
        sendbuf[strcspn(sendbuf, "\n")] = '\0';
        if (!strcmp(sendbuf, "EXIT")) {
            ::send(clientSockFd, sendbuf, BUF_SIZE, 0);
            close(clientSockFd);
            close_flag = true;
        }

        return ::send(clientSockFd, sendbuf, BUF_SIZE, 0);
    }

    int receive() {
        int ret = recv(clientSockFd, message, BUF_SIZE, 0);
        if (ret > 0)
            printf("%s", message);
        return ret;
    }

    void epollcon() {
        epollID.addfd(clientSockFd, EPOLLIN, true);
        epollID.addfd(0, EPOLLIN, true);
    }

    void process() {
        while (!isClose()) {
            int cnt = epollID.wait();
            // printf("debug:in client, cnt %d\n", cnt);
            for (int i = 0; i < cnt; ++i) {
                int temfd = epollID.getfd(i);
                if (temfd == 0) {
                    this->send();
                }
                else {
                    this->receive();
                }
            }
        }
    }
    bool isClose() {
        return close_flag;
    }
};

int main() {
    client c1;
    c1.init("127.0.0.1", 8080);
    c1.connectServer();
    c1.epollcon();
    c1.process();

    return 0;
}