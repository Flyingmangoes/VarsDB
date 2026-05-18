#include <iostream>
#include <string.h>
#include <chrono>
#include "network.hpp"

#define SERVER_EXIT "EXIT"
#define SERVER_START "START"
#define RETRY 2

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
        utils::MessageManager::error("Failed to set socket", errno);
        return -1;
    }

    int reuse = 1;

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        utils::MessageManager::error("Failed to set socket option", errno);
        return -1;
    }

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        utils::MessageManager::error("Failed to set socket option", errno);
        return -1;
    }

    if (SetFlag::set_socket_non_block(this->socket_fd) < 0) {
        utils::MessageManager::error("Failed to set socket non block", errno);
        return -1;
    }

    if (bind(this->socket_fd, reinterpret_cast<sockaddr*>(&this->address), sizeof(this->address)) < 0) {
        utils::MessageManager::error("Failed to bind socket", errno);
        return -1;
    }

    utils::MessageManager::info("Server Setup Complete");
    return 0;
};

int Server::wait_client() {  
    struct epoll_event ev, events[MAX_EVENT];
    int nfds, epollfd, s;
    bool running = false;

    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        utils::MessageManager::error("Failed to create epoll fd", errno);
        return -1;
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = this->socket_fd;
    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, this->socket_fd, &ev);
    if (s == -1) {
        utils::MessageManager::error("Failed to add epoll opt", errno);
        return -1;
    }

    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
        utils::MessageManager::error("Failed to add epoll opt", errno);
        return -1;
    }

    utils::MessageManager::info("Type START to accept connections, EXIT to quit");

    while(1) {
        
        nfds = epoll_wait(epollfd, events, MAX_EVENT, -1);
        if (nfds < 0) {
            utils::MessageManager::error("epoll_wait failed", errno);
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                char cmdbuffer[64] = {0};
                if (fgets(cmdbuffer, sizeof(cmdbuffer), stdin) == NULL) {
                    utils::MessageManager::info("Shutting down server");
                    goto shutdown;
                }

                cmdbuffer[strcspn(cmdbuffer, "\r\n")] = '\0';
                if (strlen(cmdbuffer) == 0) continue;

                if (strncmp(cmdbuffer, SERVER_EXIT, strlen(SERVER_EXIT)) == 0) {
                    utils::MessageManager::info("Shutting down server");
                    goto shutdown;
                }
                else if (strncmp(cmdbuffer, SERVER_START, strlen(SERVER_START)) == 0) {
                    running = true;
                    char msg[64];
                    snprintf(msg, sizeof(msg), "Connections on port %d", this->port);
                    utils::MessageManager::info(msg);
                }
                else {
                    char msg[MAX_MESSAGE_BUFFER];
                    snprintf(msg, sizeof(msg), "Unknown command: %s — use START or EXIT", cmdbuffer);
                    utils::MessageManager::warn(msg);
                }
                continue;
            }

            if (!running) continue;

            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                !(events[i].events & EPOLLIN))
            {
                utils::MessageManager::error("Wrong epoll events", errno);

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
                        utils::MessageManager::error("accept failed", errno);
                        break;
                    }
                    
                    s = getnameinfo(&in_addr, in_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        utils::MessageManager::print_client(utils::MessageManager::INFO, const_cast <int *> (&infd), hbuf, sbuf);
                    }

                    s = SetFlag::set_socket_non_block(infd);
                    if (s == -1) {
                        utils::MessageManager::error("Failed to turn socket non blocking", errno);
                        close(infd);
                        continue;
                    }

                    ev.data.fd = infd;
                    ev.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(epollfd, EPOLL_CTL_ADD, infd, &ev);
                    if (s < 0) {
                        utils::MessageManager::error("Failed to add client to epoll", errno);
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
                        utils::MessageManager::error("recv failed", errno);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        close(events[i].data.fd);
                        break;
                    }

                    if (count == 0) {
                        utils::MessageManager::info("Client disconnected");
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

shutdown:
    strg::deallocator(this->storages);
    close(epollfd);
    close(this->socket_fd);
    return 0;
};

int Server::start() {
    char msg[MAX_MESSAGE_BUFFER];
    struct storage_holder* storage;

    if (listen(this->socket_fd, MAX_CONNECTION) < 0) {
        utils::MessageManager::error("Socket failed to listen for connection", errno);
        return -1;
    }

    storage = strg::allocator();
    setStorage(storage);
    snprintf(msg, sizeof(msg),"Storage allocated: %p", &storage);
    utils::MessageManager::info(msg);

    while(1) {
    int s = wait_client();
    if (s == RETRY) continue;
    if (s < 0)      break;
    break;
}
    return 0;   
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
        GeneralCommand::PING(this->client_fd);
        return 0;
    }
    else if (strncmp(tokens[0], "SET", 3) == 0) 
    {
        if (argc < 3) {
           goto insufficient_argument_cleanup;
        }

        const char * opt = (argc >= 5) ? tokens[3] : NULL;
        int n = (argc >= 5 && tokens[4] != NULL) ? atoi(tokens[4]) : 0;

        if (argc >= 5 && n <= 0) {
            response = "-ERR invalid expire time\r\n";
            send(this->client_fd, response, strlen(response), 0);
            return -1;
        }

        GeneralCommand::SET(this->client_fd, tokens[1], tokens[2], opt, n, this->storages);
        return 0;
    }
    else if (strncmp(tokens[0], "GET", 3) == 0) {
        if (argc < 2) {
            goto insufficient_argument_cleanup;
        }

        GeneralCommand::GET(this->client_fd, tokens[1], this->storages);
        return 0;
    }
    else if (strncmp(tokens[0], "ECHO", 4) == 0) 
    {
        if (argc < 2) {
            goto insufficient_argument_cleanup;
        }

        GeneralCommand::ECHO(this->client_fd, tokens[1]);
        return 0;
    }
    else if(strncmp(tokens[0], "DEL", 3) == 0) {
        if(argc < 2) {
            goto insufficient_argument_cleanup;
        }

        GeneralCommand::DEL(this->client_fd, argc - 1, const_cast<const char**>(tokens + 1), this->storages);
        return 0;
    }
    else if(strncmp(tokens[0], "EXISTS", 6) == 0) {
        if(argc < 2) {
            goto insufficient_argument_cleanup;
        }

        GeneralCommand::EXISTS(this->client_fd, argc - 1, const_cast<const char**>(tokens + 1), this->storages);
        return 0;
    } 

    /* LIST command */
    else if(strncmp(tokens[0], "RPUSH", 5) == 0) {
        if(argc < 2) {
            goto insufficient_argument_cleanup;
        }
    }
    else if(strncmp(tokens[0], "LRANGE", 6) == 0) {
        if(argc < 2) {
            goto insufficient_argument_cleanup;
        }
    }

    /* HASH command */
    else {
        response = "-ERR unknown command\r\n";
        send(client_fd, response, strlen(response), 0);
        return -1;
    }

insufficient_argument_cleanup:
    response = "-ERR insufficient number of arguments given\r\n";
    send(this->client_fd, response, strlen(response), 0);
    return -1;
};