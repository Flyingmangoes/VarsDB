#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <chrono>

#include "command.hpp"
#include "server.hpp"
#include "utilities.hpp"

#define DEFAULT_PORT 6479 
#define MAX_EVENT 32  
#define MAX_CONNECTION 5
#define SOCKET int

class Server {
private:
    sockaddr_in address;
    SOCKET socket_fd;
    SOCKET client_fd;

    int port;
    int type;
    int protocol;
    int domain;

    const char* arguments;
    struct storage_holder *storages;

    int handle_command();
    int wait_client();

public: 
    Server(int domain = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);

    void setStorage(struct storage_holder* __instances);
    void setClient(SOCKET client_fd, char* argv);
    void setAddress(int port);
    int setup_server();
    int start();
};

class Client {
private:
    sockaddr_in address;
    SOCKET socket_fd;
    int port;
    int domain;
    int type;
    int protocol;

public:
    Client(int domain = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);

    SOCKET getSocket();
    void setAddress(int port);
    int setup_client();
    int connect_to_server();
};

class SetFlag {
public:
    static int set_socket_non_block(SOCKET _fd);
};

#endif