#include "./filelogger.h"
#include <arpa/inet.h>
#include <netinet/in.h>
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
    int addrlen = sizeof(addr_server);

public:
    Server(string addr = "127.0.0.1", uint16_t port = 8080) {
        addr_server.sin_family = AF_INET;
        addr_server.sin_port = htons(port);
        addr_server.sin_addr.s_addr = inet_addr(addr.c_str());
        fd_server = socket(AF_INET, SOCK_STREAM, 0);
    }

    bool Bind() {
        if (bind(fd_server, (struct sockaddr *)(&addr_server), addrlen))
            return false;
        else
            return true;
    }

    bool listenandacc() {
        listen(fd_server, 5);
        
        int new_session = accept(fd_server, (struct sockaddr *)(&addr_server), (socklen_t *)(&addrlen));
        cout << "new session " << new_session;
        string anw = "hello form server";
        cout << send(new_session, anw.c_str(), anw.length(), 0);
    }
};
int main() {
    Server s;
    s.Bind();
    s.listenandacc();
    return 0;
}