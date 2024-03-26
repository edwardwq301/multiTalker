#include "util.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <sstream>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <list>
#include <csignal>

class Server {
private:
    struct sockaddr_in serverAddr;
    Epoller epollID;
    std::string serverIP = "127.0.0.1";
    u_int32_t serverPort = 8080;
    int listener;
    std::list<int> clients_list;

    char recvMessage[BUF_SIZE];
    char sendMessage[BUF_SIZE];

public:
    Server() :
        epollID() {
        serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
    }

    int createSocketAndListen() {
        listener = socket(AF_INET, SOCK_STREAM, 0);
        if (listener == -1) {
            perror("server: create socket failed\n");
            exit(-1);
        }

        // 设置地址可重用
        int reuseAddr = 1;
        int ret = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr));
        if (ret != 0) {
            std::cout << "套接字设置地址重用失败" << std::endl;
            return -2;
        }

        if (bind(listener, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("server: bind socket failed\n");
            exit(-1);
        }
        ret = listen(listener, 5);
        if (ret < 0) {
            perror("server: listen socket failed\n");
            exit(-1);
        }
        return ret;
    }


    void processConnect(int fd) {
        struct sockaddr_in client_address;
        socklen_t client_addrLength = sizeof(struct sockaddr_in);
        int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);

        printf("client connection from: %s: %d, clientfd = %d\n",
               inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port),
               clientfd);

        epollID.addfd(clientfd, EPOLLIN, true);

        // 服务端保存用户连接
        clients_list.push_back(clientfd);
        printf("Now there are %d clients in  chat room\n", (int)clients_list.size());

        // show welcome
        bzero(sendMessage, BUF_SIZE);
        sprintf(sendMessage, SHOW_WELCOME_ID.c_str(), clientfd);
        int ret = send(clientfd, sendMessage, BUF_SIZE, 0);
        if (ret < 0) {
            perror("send error");
            exit(-1);
        }
    }

    void processSend(int temfd) {
        bzero(recvMessage, BUF_SIZE);
        // bzero must be before recv
        int ret = recv(temfd, recvMessage, BUF_SIZE, 0);

        if (ret <= 0 || !strcmp(recvMessage, EXIT.c_str())) {
            epollID.delfd(temfd);
            clients_list.remove(temfd);
            printf("client %d log out\n", temfd);
            printf("Now there are %d clients in  chat room\n", (int)clients_list.size());
        }
        else if (clients_list.size() == 1) {
            send(temfd, ONLY_USER.c_str(), BUF_SIZE, 0);
            return;
        }
        else {
            for (int k : clients_list)
                if (k != temfd) {
                    sprintf(sendMessage, CLIENT_RECV.c_str(), temfd, recvMessage);
                    send(k, sendMessage, BUF_SIZE, 0);
                }
        }
    }

    void process() {
        epollID.addfd(listener, EPOLLIN, true);

        while (true) {
            int eventCnt = epollID.wait();

            if (eventCnt < 0) {
                perror("epollID wait failed\n");
                exit(-1);
            }
            for (int i = 0; i < eventCnt; ++i) {
                int temfd = epollID.getfd(i);
                // server get connetction
                if (temfd == listener)
                    processConnect(temfd);
                else {
                    processSend(temfd);
                }
            }
        }
        printf("should not be here\n");
    }
};



int main() {
    Server ser;
    ser.createSocketAndListen();
    ser.process();
    return 0;
}