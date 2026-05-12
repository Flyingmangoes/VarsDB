#include <iostream>
#include <string.h>
#include <chrono>
#include "server.hpp"

Server::Server(int domain, int type, int protocol) {    
    this->domain   = domain;
    this->type     = type;
    this->protocol = protocol;
    this->socket_fd = -1;   
    this->port      = -1;    
}

void Server::setAddress(int port) {
    if (port <= 0 || port > 65535 ) {
        MessageManager::warn("Not provided a port using default port");
        port = DEFAULT_PORT;
    }

    if (port < 1024) {
        MessageManager::warn("Port below 1024 required root privileges");
    }

    memset(&this->address, 0, sizeof(this->address));

    this->address.sin_family = this->domain;
    this->address.sin_addr.s_addr = INADDR_ANY;
    this->address.sin_port = htons(port);

    this->port = port;
    MessageManager::info("Address configured");
};

int Server::setup_server() {
    this->socket_fd = socket(this->domain, this->type, this->protocol);
    if (this->socket_fd < 0) {
        MessageManager::error("Failed to set socket", errno);
        return -1;
    }

    int reuse = 1;

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        MessageManager::error("Failed to set socket option", errno);
        return -1;
    }

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        MessageManager::error("Failed to set socket option", errno);
        return -1;
    }

    if (SetFlag::set_socket_non_block(this->socket_fd) < 0) {
        MessageManager::error("Failed to set socket non block", errno);
        return -1;
    }

    if (bind(this->socket_fd, reinterpret_cast<sockaddr*>(&this->address), sizeof(this->address)) < 0) {
        MessageManager::error("Failed to bind socket", errno);
        return -1;
    }

    MessageManager::info("Server Setup Complete");
    return 0;
};

int Server::wait_client() {  
    struct epoll_event ev, events[MAX_EVENT];
    int nfds, epollfd, s;

    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        MessageManager::error("Epoll process failed", errno);
        return 1;
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = this->socket_fd;
    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, this->socket_fd, &ev);
    if (s == -1) {
        MessageManager::error("Epoll process failed", errno);
        return 1;
    }

    while(true) {
        nfds = epoll_wait(epollfd, events, MAX_EVENT, -1);
        for (int i = 0; i < nfds; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                !(events[i].events & EPOLLIN))
            {
                MessageManager::error("Epoll process failed", errno);

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
                        MessageManager::error("accept failed", errno);
                        break;
                    }
                    
                    s = getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        MessageManager::print_client(MessageManager::INFO, const_cast <int *> (&infd), hbuf, sbuf);
                    }

                    s = SetFlag::set_socket_non_block(infd);
                    if (s == -1) {
                        MessageManager::error("Failed to turn socket non blocking", errno);
                        close(infd);
                        continue;
                    }

                    ev.data.fd = infd;
                    ev.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, infd, &ev);
                    if (s < 0) {
                        MessageManager::error("Failed to add client to epoll", errno);
                        close(infd);
                    }
                }

                continue;
            } else {
                while(1) {
                    char buf[MAX_MESSAGE_BUFFER];
                    ssize_t count = recv(events[i].data.fd, buf, sizeof(buf) - 1, 0);

                    if (count < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break; 
                        MessageManager::error("recv failed", errno);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        close(events[i].data.fd);
                        break;
                    }

                    if (count == 0) {
                        MessageManager::info("Client disconnected");
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL); 
                        close(events[i].data.fd);
                        break;
                    }
                    buf[count] = '\0';

                    char *line = strtok(buf, "\n");
                    while(line != NULL) {
                        setClient(events[i].data.fd, line);
                        handle_command();
                        line = strtok(NULL, "\n");
                    }
                }
            }
        }
    }
    
    close(epollfd);
    return EXIT_SUCCESS;
};

int Server::start() {
    if (listen(this->socket_fd, MAX_CONNECTION) < 0) {
        MessageManager::error("Socket failed to listen for connection", errno);
        return -1;
    }

    struct storage_holder* storage = _storage_allocator();
    setStorage(storage);
    MessageManager::info("Storage allocated");

    char msg[MAX_MESSAGE_BUFFER];
    snprintf(msg, sizeof(msg), "Server listening on port %d", this->port);
    MessageManager::info(msg);

    return wait_client();   
};

int SetFlag::set_socket_non_block(SOCKET _fd) {
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::setClient(SOCKET client_fd, char* argv) {
    this->arguments = argv;
    this->client_fd = client_fd;
}

void Server::setStorage(struct storage_holder *__instances) {
    this->storages = __instances;
};

int Server::handle_command() {
    char cmd_copy[MAX_COMMAND_BUFFER] = {0};
    strncpy(cmd_copy, this->arguments, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';

    char* tokens[16];
    int argc = 0;
    
    char* token = strtok(cmd_copy, " \r\n");
    while(token != NULL && argc < 16) {
        tokens[argc++] = token;
        token = strtok(NULL, " \r\n");
    }

    const char* response;

    if (argc == 0) return -1;
    if(strncmp(tokens[0], "PING", 4) == 0) {
        CommandManager::PING(this->client_fd);
        return 0;
    }
    else if (strncmp(tokens[0], "SET", 3) == 0) 
    {
        if (argc < 3) {
            response = "-ERR insufficient number of argument given\r\n";
            send(this->client_fd, response, strlen(response), 0);
            return -1;
        }

        const char * opt = (argc >= 5) ? tokens[3] : NULL;
        int n = (argc >= 5 && tokens[4] != NULL) ? atoi(tokens[4]) : 0;

        if (argc >= 5 && n <= 0) {
            response = "-ERR invalid expire time\r\n";
            send(this->client_fd, response, strlen(response), 0);
            return -1;
        }

        CommandManager::SET(this->client_fd, tokens[1], tokens[2], opt, n, this->storages);
        return 0;
    }
    else if (strncmp(tokens[0], "GET", 3) == 0) {
        if (argc < 2) {
            response = "-ERR insufficient number of arguments given\r\n";
            send(this->client_fd, response, strlen(response), 0);
            return -1;
        }

        CommandManager::GET(this->client_fd, tokens[1], this->storages);
        return 0;
    }
    else if (strncmp(tokens[0], "ECHO", 4) == 0) 
    {
        if (argc < 2) {
            response = "-ERR insufficient number of arguments given\r\n";
            send(this->client_fd, response, strlen(response), 0);
            return -1;
        }

        CommandManager::ECHO(this->client_fd, tokens[1]);
        return 0;
    }
    else {
        response = "-ERR unknown command\r\n";
        send(client_fd, response, strlen(response), 0);
        return -1;
    }
};