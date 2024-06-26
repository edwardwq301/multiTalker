#include "./filelogger.h"
#include "./util.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <unistd.h>

using namespace std;

class Client {
    int fd_client = 0;
    int recv_cnt;
    char buffer[SZ];
    Epoller eper;
    bool islive = true;

    int Receive() {
        fill(buffer, buffer + SZ, 0);
        recv_cnt = read(fd_client, buffer, SZ);
        if (recv_cnt < 0) {
            cerr << "client read failed";
            return -1;
        }
        std::cout << buffer << std::endl;
        return recv_cnt;
    }

    int Send() {
        string words;
        getline(cin, words);
        if (words == "exit") islive = false;
        return send(fd_client, words.c_str(), words.size(), 0);
    }

    void Conn_server() {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(8080);

        fd_client = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_client < 0) { std::cerr << "Socket creation error" << std::endl; }

        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address/ Address not supported" << std::endl;
        }

        if (connect(fd_client, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Connection failed" << std::endl;
        }
        Receive();
    }

public:
    Client() : eper() {
        Conn_server();
        eper.Addfd(0, true);
        eper.Addfd(fd_client, true);
    }
    void ProcessInputandSend() {
        while (true) {
            if (!islive) break;
            int todo_cnt = eper.Wait();
            for (int x = 0; x < todo_cnt; x++) {
                int temfd = eper.todos[x].data.fd;
                if (temfd == 0) { Send(); }
                else { Receive(); }
            }
        }
    }
};

int main() {
    Client cc;

    cc.ProcessInputandSend();

    return 0;
}
