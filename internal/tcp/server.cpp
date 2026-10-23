#include "TCP.hpp"
#include "Command.hpp"
#include "RESP.hpp"

#define SERVER_EXIT "EXIT"
#define SERVER_START "START"
#define RETRY 2

#define RESPONSE_BUFFER 4096 
#define ARGUMENTS_BUFFER 2048
#define WAIT_BUFFER 4096

using namespace std;

Server::Server(int domain, int type, int protocol) {    
    this->domain   = domain;
    this->type     = type;
    this->protocol = protocol;
    this->socket_fd = -1;   
    this->port      = -1;    
}

int Server::setup_server() {
    this->socket_fd = socket(this->domain, this->type, this->protocol);
    if (this->socket_fd < 0) {
        vars::MessageFactory::error("Failed to set socket", errno);
        return -1;
    }

    int reuse = 1;

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        vars::MessageFactory::error("Failed to set socket option", errno);
        return -1;
    }

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        vars::MessageFactory::error("Failed to set socket option", errno);
        return -1;
    }

    if (SetFlag::set_socket_non_block(this->socket_fd) < 0) {
        vars::MessageFactory::error("Failed to set socket non block", errno);
        return -1;
    }

    if (bind(this->socket_fd, reinterpret_cast<sockaddr*>(&this->address), sizeof(this->address)) < 0) {
        vars::MessageFactory::error("Failed to bind socket", errno);
        return -1;
    }

    vars::MessageFactory::info("Server Setup Complete");
    return 0;
};

int Server::wait_client() {  
    struct epoll_event ev, events[MAX_EVENT];
    int nfds, epollfd, s;
    bool running = false;
    std::unordered_map<int, std::string> inbuf;

    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        vars::MessageFactory::error("Failed to create epoll fd", errno);
        close(epollfd);
        return -1;
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = this->socket_fd;
    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, this->socket_fd, &ev);
    if (s == -1) {
        vars::MessageFactory::error("Failed to add listen socket to epoll", errno);
        close(epollfd);
        return -1;
    }

    if (isatty(STDIN_FILENO)) {
        ev.events = EPOLLIN;
        ev.data.fd = STDIN_FILENO;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
            vars::MessageFactory::error("Failed to add stdin to epoll", errno);
            close(epollfd);
            return -1;
        }

        vars::MessageFactory::info("Type START to accept connections, EXIT to quit");
    } else {
        running = true;    
        vars::MessageFactory::info("No console attached, accepting connections on port " + this->port);
    }

    while(1) {
        nfds = epoll_wait(epollfd, events, MAX_EVENT, -1);
        if (nfds < 0) {
            vars::MessageFactory::error("epoll_wait failed", errno);
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                char cmd_buf[RESPONSE_BUFFER] = {0};
                if (fgets(cmd_buf, sizeof(cmd_buf), stdin) == NULL) {
                    vars::MessageFactory::info("Shutting down server");
                    return shutdown(epollfd);
                }

                cmd_buf[strcspn(cmd_buf, "\r\n")] = '\0';
                if (strlen(cmd_buf) == 0) continue;

                if (strcmp(cmd_buf, SERVER_EXIT) == 0) {
                    vars::MessageFactory::info("Shutting down server");
                    return shutdown(epollfd);
                }
                else if (strcmp(cmd_buf, SERVER_START) == 0) {
                    running = true;
                    char msg[RESPONSE_BUFFER];
                    snprintf(msg, sizeof(msg), "Connections on port %d", this->port);
                    vars::MessageFactory::info(msg);
                }
                else {
                    char msg[RESPONSE_BUFFER];
                    snprintf(msg, sizeof(msg), "Unknown command: %.200s — use START or EXIT", cmd_buf);
                    vars::MessageFactory::warn(msg);
                }
                continue;
            }

            if (!running) continue;

            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                !(events[i].events & EPOLLIN))
            {
                vars::MessageFactory::error("Wrong epoll events", errno);

                epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(events[i].data.fd);
                continue;
            }

            else if (events[i].data.fd == this->socket_fd) {
                while(1) {
                    struct sockaddr in_addr;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                    socklen_t in_len = sizeof(in_addr);

                    int infd = accept(this->socket_fd, &in_addr, &in_len);
                    if (infd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) break;
                        vars::MessageFactory::error("accept failed", errno);
                        break;
                    }
                    
                    s = getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        vars::MessageFactory::print_client(vars::MessageFactory::INFO, const_cast <int *> (&infd), hbuf, sbuf);
                    }

                    s = SetFlag::set_socket_non_block(infd);
                    if (s == -1) {
                        vars::MessageFactory::error("Failed to turn socket non blocking", errno);
                        close(infd);
                        continue;
                    }

                    ev.data.fd = infd;
                    ev.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, infd, &ev);
                    if (s < 0) {
                        vars::MessageFactory::error("Failed to add client to epoll", errno);
                        close(infd);
                    }
                }

                continue;
            } else {
                const int fd = events[i].data.fd;
                bool closed = false;
                char buf[WAIT_BUFFER];

                while(1) {
                    ssize_t count = recv(fd, buf, sizeof(buf), 0);
                    if (count > 0) { inbuf[fd].append(buf, count); continue; }
                    if (count == 0) { closed = true; break; }
                    if (errno == EINTR) continue;
                    if (errno != EAGAIN && errno != EWOULDBLOCK) closed = true;
                    break;
                }

                std::string& in = inbuf[fd];
                while(!in.empty()) {
                    ParseResult r = RESP_Parser::parse(in);
                    if (r.status == ParseStatus::Incomplete) break;

                    if (r.status == ParseStatus::Error) {
                        error_cleanup(fd, ("ERR Protocol error: " + r.error).c_str());
                        closed = true;
                        break;
                    }

                    in.erase(0, r.consumed);
                    if (r.args.empty()) continue;
                    handle_command(fd, r.args);
                }

                if(closed) {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    inbuf.erase(fd);
                    vars::MessageFactory::info("Client disconnected");
                }
            }
        }
    }

    return -1;
};

int Server::shutdown(int& epoll_fd) {
    if (this->storage) {
        this->storage->db.clear();
        vars::MemoryStorage::deallocate(this->storage);
        this->storage = nullptr;
    }
    close(epoll_fd);
    close(this->socket_fd);
    return 0;
};

int Server::start() {
    char msg[RESPONSE_BUFFER];

    if (listen(this->socket_fd, MAX_CONNECTION) < 0) {
        vars::MessageFactory::error("Socket failed to listen for connection", errno);
        return -1;
    }

    setStorage(vars::MemoryStorage::allocate());
    snprintf(msg, sizeof(msg),"Storage allocated: %p", (void*)this->storage);
    vars::MessageFactory::info(msg);

    while(1) {
        int s = wait_client();
        if (s == RETRY) continue;
        if (s < 0)      break;
        break;
    }

    if (this->storage) {
        vars::MemoryStorage::deallocate(this->storage);
        this->storage = nullptr;
    }

    return 0;   
};

int Server::handle_command(const int client_fd, const std::vector<std::string>& args) {
    GeneralCommand general{ this->storage };
    const size_t argc = args.size();

    std::string cmd = args[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    if (cmd == "PING") {
        general.PING(client_fd);
    }
    else if (cmd == "SET") {
        if (argc != 3 && argc != 5)
            return error_cleanup(client_fd, "ERR wrong number of arguments for 'set' command");
        std::string opt = (argc == 5) ? args[3] : "";
        int n = (argc == 5) ? atoi(args[4].c_str()) : 0;
        if (argc == 5 && n <= 0) return error_cleanup(client_fd, "ERR invalid expire time");
        general.SET(client_fd, args[1], args[2], opt, n);
    }
    else if (cmd == "GET") {
        if (argc != 2) return error_cleanup(client_fd, "ERR wrong number of arguments for 'get' command");
        general.GET(client_fd, args[1]);
    }
    else if (cmd == "ECHO") {
        if (argc != 2) return error_cleanup(client_fd, "ERR wrong number of arguments for 'echo' command");
        general.ECHO(client_fd, args[1]);
    }
    else if (cmd == "DEL") {
        if (argc < 2) return error_cleanup(client_fd, "ERR wrong number of arguments for 'del' command");
        general.DEL(client_fd, std::vector<std::string>(args.begin() + 1, args.end()));
    }
    else if (cmd == "EXISTS") {
        if (argc < 2) return error_cleanup(client_fd, "ERR wrong number of arguments for 'exists' command");
        general.EXISTS(client_fd, std::vector<std::string>(args.begin() + 1, args.end()));
    }
    else if (cmd == "MOVE") {
        if (argc != 3 && argc != 5) return error_cleanup(client_fd, "ERR wrong number of arguments for 'move' command");
        std::string opt = (argc == 5) ? args[3] : "";
        int n = (argc == 5) ? atoi(args[4].c_str()) : 0;
        if (argc == 5 && n <= 0) return error_cleanup(client_fd, "ERR invalid expire time");
        general.MOVE(client_fd, args[1], args[2], opt, n);
    }
    else {
        return error_cleanup(client_fd, "ERR unknown command '" + args[0] + "'");   
    }
    return 0;
}