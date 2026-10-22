#include <iostream>
#include <string.h>
#include <chrono>
#include <cstdio>
#include "TCP.hpp"

#define SERVER_EXIT "EXIT"
#define SERVER_START "START"
#define RETRY 2

#define RESPONSE_BUFFER 512 
#define ARGUMENTS_BUFFER 256
#define WAIT_BUFFER 256

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

    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        vars::MessageFactory::error("Failed to create epoll fd", errno);
        return -1;
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = this->socket_fd;
    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, this->socket_fd, &ev);
    if (s == -1) {
        vars::MessageFactory::error("Failed to add epoll opt", errno);
        return -1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
        vars::MessageFactory::error("Failed to add epoll opt", errno);
        return -1;
    }

    vars::MessageFactory::info("Type START to accept connections, EXIT to quit");

    while(1) {
        
        nfds = epoll_wait(epollfd, events, MAX_EVENT, -1);
        if (nfds < 0) {
            vars::MessageFactory::error("epoll_wait failed", errno);
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                char cmdbuffer[RESPONSE_BUFFER] = {0};
                if (fgets(cmdbuffer, sizeof(cmdbuffer), stdin) == NULL) {
                    vars::MessageFactory::info("Shutting down server");
                    return shutdown(epollfd);
                }

                cmdbuffer[strcspn(cmdbuffer, "\r\n")] = '\0';
                if (strlen(cmdbuffer) == 0) continue;

                if (strncmp(cmdbuffer, SERVER_EXIT, strlen(SERVER_EXIT)) == 0) {
                    vars::MessageFactory::info("Shutting down server");
                    return shutdown(epollfd);
                }
                else if (strncmp(cmdbuffer, SERVER_START, strlen(SERVER_START)) == 0) {
                    running = true;
                    char msg[RESPONSE_BUFFER];
                    snprintf(msg, sizeof(msg), "Connections on port %d", this->port);
                    vars::MessageFactory::info(msg);
                }
                else {
                    char msg[RESPONSE_BUFFER];
                    snprintf(msg, sizeof(msg), "Unknown command: %s — use START or EXIT", cmdbuffer);
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
                while(1) {
                    char buf[WAIT_BUFFER];
                    ssize_t count = recv(events[i].data.fd, buf, sizeof(buf) - 1, 0);

                    if (count < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break; 
                        vars::MessageFactory::error("recv failed", errno);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        close(events[i].data.fd);
                        break;
                    }

                    if (count == 0) {
                        vars::MessageFactory::info("Client disconnected");
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
};

int Server::shutdown(int& epoll_fd) {
    this->storage->db.clear(); 
    vars::MemoryStorage::deallocate(this->storage);
    close(epoll_fd);
    close(this->socket_fd);
    return 0;
};

int Server::start() {
    char msg[RESPONSE_BUFFER];
    Storage* storage;

    if (listen(this->socket_fd, MAX_CONNECTION) < 0) {
        vars::MessageFactory::error("Socket failed to listen for connection", errno);
        return -1;
    }

    storage = vars::MemoryStorage::allocate();
    setStorage(storage);
    snprintf(msg, sizeof(msg),"Storage allocated: %p", (void*)&storage);
    vars::MessageFactory::info(msg);

    while(1) {
    int s = wait_client();
    if (s == RETRY) continue;
    if (s < 0)      break;
    break;
}
    return 0;   
};

int Server::handle_command() {
    char copy_buf[ARGUMENTS_BUFFER] = {0};
    strncpy(copy_buf, this->arguments, sizeof(copy_buf) - 1);
    copy_buf[sizeof(copy_buf) - 1] = '\0';

    GeneralCommand general = GeneralCommand(this->storage);
    ListCommand list = ListCommand(this->storage);
    StreamCommand stream = StreamCommand(this->storage);

    char* tokens[16];
    int argc = 0;
    
    char* token = strtok(copy_buf, " \r\n");
    while(token != NULL && argc < 16) {
        tokens[argc++] = token;
        token = strtok(NULL, " \r\n");
    }

    if (argc == 0) return -1;
    if(strncmp(tokens[0], "PING", 4) == 0) {
        general.PING(this->client_fd);
        return 0;
    }
    else if (strncmp(tokens[0], "SET", 3) == 0) 
    {
        if (argc < 3) {
           return this->error_cleanup("Insufficient arguments");
        }

        const char * opt = (argc >= 5) ? tokens[3] : NULL;
        int n = (argc >= 5 && tokens[4] != NULL) ? atoi(tokens[4]) : 0;

        if (argc >= 5 && n <= 0) {
            return this->error_cleanup("Invalid expire time");
        }

        general.SET(this->client_fd, tokens[1], tokens[2], opt, n);
        return 0;
    }
    else if (strncmp(tokens[0], "GET", 3) == 0) {
        if (argc < 2) {
            return this->error_cleanup("Insufficient arguments");
        }

        general.GET(this->client_fd, tokens[1]);
        return 0;
    }
    else if (strncmp(tokens[0], "ECHO", 4) == 0) 
    {
        if (argc < 2) {
            return this->error_cleanup("Insufficient arguments");
        }

        general.ECHO(this->client_fd, tokens[1]);
        return 0;
    }
    else if(strncmp(tokens[0], "DEL", 3) == 0) {
        if(argc < 2) {
            return this->error_cleanup("Insufficient arguments");
        }

        general.DEL(this->client_fd, argc - 1, const_cast<const char**>(tokens + 1));
        return 0;
    }
    else if(strncmp(tokens[0], "EXISTS", 6) == 0) {
        if(argc < 2) {
            return this->error_cleanup("Insufficient arguments");
        }

        general.EXISTS(this->client_fd, argc - 1, const_cast<const char**>(tokens + 1));
        return 0;
    } 

    else if(strncmp(tokens[0], "MOVE", 4) == 0) {
        if(argc < 2) {
            return this->error_cleanup("Insufficient arguments");
        }

        const char* key = tokens[1];
        const char* value = tokens[2];

        const char* opt = (argc >= 5) ? tokens[3] : NULL;
        int n = (argc >= 5) ? atoi(tokens[5]) : 0;

        if(argc >=5 && n <= 0) {
            this->error_cleanup("Invalid expire time");
        }

        general.MOVE(this->client_fd, key, value, opt, n);
        return 0;
    }

    /* LIST command */
    else if(strncmp(tokens[0], "RPUSH", 5) == 0) {
        if(argc < 2) {
            return this->error_cleanup("Insufficient arguments");
        }
    }
    else if(strncmp(tokens[0], "LRANGE", 6) == 0) {
        if(argc < 2) {
            return this->error_cleanup("Insufficient arguments");
        }
    }

    /* HASH command */
    else {
        return this->error_cleanup("Unknown command");
    }
};

int Server::error_cleanup(const char* error_message){
    std::string response = RESP_Parser::error(std::string(error_message));
    send(this->client_fd, response.c_str(), strlen(response.c_str()), 0);
    return -1;
};