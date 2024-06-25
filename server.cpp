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
        eper.Addfd(fd_server, false);
        while (true) {
            int todo_cnt = eper.Wait();
            if (todo_cnt < 0) {
                logger->Write("eper wait failed");
                logger->Flush();
                exit(-1);
            }
            for (auto x : eper.todos) {
                int temfd = x.data.fd;
                if (temfd == fd_server)
                    ProcessConnect();
                else
                    ProcessSend(temfd);
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


        eper.Addfd(fd_client, false);
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

    void ProcessSend(int temfd) {
        char buffer[1024];
        int bytesReceived = recv(temfd, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1) {
            logger->Write("Error receiving data from client");
            exit(-1);
        }

        std::string receivedString(buffer, bytesReceived);
        logger->Write("Received string from client: " + receivedString);
        logger->Flush();
    }
    bool listenandacc() {
        int new_session = accept(fd_server, (struct sockaddr *)(&addr_server), (socklen_t *)(&addrlen));
        string anw = "hello form server";
        cout << send(new_session, anw.c_str(), anw.length(), 0);
    }
};

int main() {
    Server s;
    s.Bind_Listen();
    s.Process();
    return 0;
}