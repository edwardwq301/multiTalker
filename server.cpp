#include "./filelogger.h"
#include "./util.h"
#include <arpa/inet.h>
#include <list>
#include <netinet/in.h>
#include <sstream>
using namespace std;

void test_FileLogger() {
    auto ptr = FileLogger::GetInstance();
    ptr->Write("hello");
    ptr->Write("world");
    ptr->Flush();
}



class Server {
private:
    int fd_server;
    struct sockaddr_in addr_server;
    const int addrlen = sizeof(addr_server);
    Epoller eper;
    unique_ptr<FileLogger> logger;
    list<int> client_list;

    char recv_buffer[SZ];

public:
    Server(string addr = "127.0.0.1", uint16_t port = 8080) : eper() {
        addr_server.sin_family = AF_INET;
        addr_server.sin_port = htons(port);
        addr_server.sin_addr.s_addr = inet_addr(addr.c_str());
        fd_server = socket(AF_INET, SOCK_STREAM, 0);

        logger = FileLogger::GetInstance();
    }

    bool Bind_Listen() {
        if (bind(fd_server, (struct sockaddr *)(&addr_server), addrlen) < 0) logger->Write("server bind failed");
        if (listen(fd_server, 5) < 0) logger->Write("server listen failed");
        logger->Flush();
    }

    void Process() {
        eper.Addfd(fd_server, true);
        while (true) {
            int todo_cnt = eper.Wait();
            if (todo_cnt < 0) {
                logger->Write("eper wait failed");
                logger->Flush();
                exit(-1);
            }
            for (int x = 0; x < todo_cnt; x++) {
                int temfd = eper.todos[x].data.fd;
                if (temfd == fd_server) { ProcessConnect(); }
                else { ProcessClient(temfd); }
            }
        }
    }

    void ProcessConnect() {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int fd_client = accept(fd_server, (struct sockaddr *)(&client_address), &client_address_len);

        // format string
        stringstream ss;
        ss << "accept conn: " << inet_ntoa(client_address.sin_addr) << ':' << ntohs(client_address.sin_port);
        logger->Write(ss.str());


        eper.Addfd(fd_client, true);
        client_list.push_back(fd_client);
        string nowHave = to_string(client_list.size()).append("in room");
        logger->Write(nowHave);
        logger->Flush();

        // show welcome
        string welcome = "welcome " + nowHave;
        int ret = send(fd_client, welcome.c_str(), welcome.size(), 0);
        if (ret < 0) {
            logger->Write("server send welcome fail");
            logger->Flush();
            exit(-1);
        }
    }

    void ProcessClient(int temfd) {
        string client_say = GetClientSay(temfd);
        Broadcast(temfd, client_say);
    }

    void Broadcast(int temfd, const string & message) {
        if (client_list.size() == 1) { send(temfd, ONLYYOU.c_str(), ONLYYOU.size(), 0); }
        else {
            for (int client : client_list)
                if (client != temfd) { send(client, message.c_str(), message.size(), 0); }
        }
    }

    string GetClientSay(int temfd) {
        fill(recv_buffer, recv_buffer + SZ, 0);
        int bytesReceived = recv(temfd, recv_buffer, sizeof(recv_buffer), 0);
        if (bytesReceived == -1) {
            logger->Write("Error receiving data from client");
            logger->Flush();
            exit(-1);
        }

        std::string receivedString(recv_buffer, bytesReceived);

        if (receivedString == "exit") {
            client_list.remove(temfd);
            eper.Delfd(temfd);
            logger->Write("client leave");
        }
        logger->Write("Received string from client: " + receivedString);
        logger->Flush();
        return receivedString;
    }
};

int main() {
    Server s;
    s.Bind_Listen();
    s.Process();
    return 0;
}