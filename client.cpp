#include "util.h"
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <iostream>

using namespace std;
class client {
    struct sockaddr_in serverAddre;
    int clientSockFd;
    string words;
    char buf[BUF_SIZE];

public:
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

        if (recv(clientSockFd, buf, BUF_SIZE, 0) > 0)
            printf("%s", buf);
        return ret;
    }
    int send() {
        string text;
        cin >> text;
        return ::send(clientSockFd, text.c_str(), text.size(), 0);
    }
};

int main() {
    client c1;
    c1.init("127.0.0.1", 8080);
    c1.connectServer();
    c1.send();
}