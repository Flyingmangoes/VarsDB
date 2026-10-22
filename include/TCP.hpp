#ifndef TCP_H
#define TCP_H

struct storage_holder;

#include <unordered_map>
#include <chrono>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#else

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <assert.h>
#include <errno.h>

#include "Command.hpp"
#include "RESP.hpp"

#define DEFAULT_PORT 6479
#define MAX_EVENT 32
#define MAX_CONNECTION 5
#if defined(_WIN32)
using SOCKET = UINT_PTR;
#else
using SOCKET = int;
#endif

class Server {
private:
    struct sockaddr_in address;
    SOCKET socket_fd;
    SOCKET client_fd;

    int port;
    int type;
    int protocol;
    int domain;

    const char* arguments;
    struct Storage* storage;

    int handle_command();
    int wait_client();
    int shutdown(int& epoll_fd);
    int error_cleanup(const char* error_message);

public: 
    Server(int domain = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);

    void setStorage(struct Storage* storage);
    void setClient(SOCKET client_fd, char* argv);
    void setAddress(int port);
    int setup_server();
    int start();
};

class Client {
private:
    struct sockaddr_in address;
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