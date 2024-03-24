
#include "util.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
class Server {
private:
    struct sockaddr_in serverAddr;
    Epoller epollID;
    std::string serverIP = "127.0.0.1";
    u_int32_t serverPort = 8080;
    int listener;
    std::vector<int> clients_list;


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
                if (temfd == listener) {
                    struct sockaddr_in client_address;
                    socklen_t client_addrLength = sizeof(struct sockaddr_in);
                    int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);

                    printf("client connection from: %s: %d, clientfd = %d\n",
                           inet_ntoa(client_address.sin_addr),
                           ntohs(client_address.sin_port),
                           clientfd);

                    epollID.addfd(clientfd, EPOLLIN, true);

                    // 服务端用list保存用户连接
                    clients_list.push_back(clientfd);
                    printf("Now there are %d clients int the chat room\n", (int)clients_list.size());


                    char message[BUF_SIZE];
                    bzero(message, BUF_SIZE);
                    sprintf(message, SHOW_WELCOME_ID.c_str(), clientfd);
                    int ret = send(clientfd, message, BUF_SIZE, 0);
                    if (ret < 0) {
                        perror("send error");
                        exit(-1);
                    }
                }
            }
        }
    }

    void test() {
        struct sockaddr_in client_address;
        socklen_t client_addrLength = sizeof(struct sockaddr_in);
        int clientfd =
            accept(listener, (struct sockaddr *)&client_address, &client_addrLength);

        printf("client connection from: %s : % d(IP : port), clientfd = %d \n",
               inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port),
               clientfd);
    }
};


int main() {
    Server ser;
    ser.createSocketAndListen();
    ser.process();
}