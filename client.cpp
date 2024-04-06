#include "util.h"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <csignal>

using namespace std;



int pipefd[2];

class Client {
    struct sockaddr_in serverAddre;
    int clientSockFd;
    string words;
    bool close_flag = false;
    char message[BUF_SIZE];
    char sendbuf[BUF_SIZE];

public:
    Epoller epollID;
    Client() :
        epollID() {
        socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
        SetNonBlock(pipefd[1]);
        epollID.Addfd(pipefd[0], EPOLLIN, true);
    }

    ~Client() {
        close(pipefd[1]);
        close(pipefd[0]);
    }
    void Init(const string server_address, uint32_t server_port) {
        serverAddre.sin_addr.s_addr = inet_addr(server_address.c_str());
        serverAddre.sin_port = htons(server_port);
        serverAddre.sin_family = AF_INET;
        clientSockFd = socket(AF_INET, SOCK_STREAM, 0);

        if (clientSockFd == -1) {
            perror("clinet: create socket failed\n");
            exit(-1);
        }
    }

    int ConnectServer() {
        int ret = connect(clientSockFd, (sockaddr *)&serverAddre, sizeof(serverAddre));

        if (-1 == ret) {
            perror("client:error\n");
            exit(-1);
        }

        if (recv(clientSockFd, message, BUF_SIZE, 0) > 0)
            printf("%s", message);

        return ret;
    }

    int Send() {
        fgets(sendbuf, BUF_SIZE, stdin);
        sendbuf[strcspn(sendbuf, "\n")] = '\0';
        if (!strcmp(sendbuf, "EXIT")) {
            ::send(clientSockFd, sendbuf, BUF_SIZE, 0);
            close(clientSockFd);
            close_flag = true;
        }

        return ::send(clientSockFd, sendbuf, BUF_SIZE, 0);
    }

    int Receive() {
        int ret = recv(clientSockFd, message, BUF_SIZE, 0);
        if (ret > 0)
            printf("%s", message);
        return ret;
    }

    void ProcessSIG() {
        int sig;
        char signals[1024];
        int ret = recv(pipefd[0], signals, sizeof(signals), 0);
        if (ret == -1 || ret == 0)
            return;
        else {
            for (int i = 0; i < ret; ++i)
                switch (signals[i]) {
                case SIGINT:
                    strcpy(sendbuf, "EXIT");
                    ::send(clientSockFd, sendbuf, BUF_SIZE, 0);
                    close(clientSockFd);
                    this->close_flag = true;
                }
        }
    }

    void ListenStdIn() {
        epollID.Addfd(clientSockFd, EPOLLIN, true);
        epollID.Addfd(0, EPOLLIN, true);
    }


    void Process() {
        while (!IsClose()) {
            int cnt = epollID.Wait();
            // printf("debug:in client, cnt %d\n", cnt);
            for (int i = 0; i < cnt; ++i) {
                int temfd = epollID.Getfd(i);
                if (temfd == 0) {
                    this->Send();
                }
                else if (temfd == pipefd[0]) {
                    this->ProcessSIG();
                }
                else {
                    this->Receive();
                }
            }
        }
    }

    bool IsClose() {
        return close_flag;
    }
};

void SigHandler(int signum) {
    int sig = signum;
    if (sig == SIGINT) {
        send(pipefd[1], (char *)&sig, 1, 0);
    }
}

int main() {
    signal(SIGINT, SigHandler);
    Client c1;
    c1.Init("127.0.0.1", 8080);
    c1.ConnectServer();
    c1.ListenStdIn();
    c1.Process();

    return 0;
}