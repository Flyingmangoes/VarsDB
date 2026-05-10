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

#include "parser.hpp"
#include "message.hpp"

#define DEFAULT_PORT 6479 
#define MAX_EVENT 32  
#define MAX_CONNECTION 5
#define SOCKET int

struct StoreValue {
    const char* value;
    std::chrono::steady_clock::time_point expires_at;
    bool has_expiry;
};

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

    std::unordered_map<std::string, struct StoreValue> store;
    std::unordered_map<std::string, struct StoreValue> temp_store;

    template<typename Duration> void set_with_expiry(const char* key, const char* value, Duration duration) {
        StoreValue sv;
        sv.value      = value;
        sv.has_expiry = true;
        sv.expires_at = std::chrono::steady_clock::now() + duration;
        this->temp_store[key] = sv;
    }

    inline bool is_expired(const StoreValue& sv) {
        if (!sv.has_expiry) return false;
        return std::chrono::steady_clock::now() > sv.expires_at;
    }

    int handle_command();
    int wait_client();

public: 
    Server(int domain = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);

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