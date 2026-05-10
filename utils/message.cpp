#include <stdio.h>
#include <ctime>
#include "message.hpp"

const char* MessageManager::type_to_string(message_type type) {
    switch (type) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        default:    return "UNKNOWN";
    }
}

void MessageManager::get_timestamp(char* buffer, size_t size) {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void MessageManager::print_client(message_type type, const int* fd, char* hbuf, char *sbuf) {
    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));

    printf("> [%s] [%s] Accepted connection on descriptor %d (host=%s, port=%s)\n", timestamp, type_to_string(type), *fd, hbuf, sbuf);
}


void MessageManager::print(message_type type, const char* msg, int err_code) {
    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));
    if (err_code != 0) {
        fprintf(stderr ,"> [%s] [%s] %s — %s\n", timestamp, type_to_string(type), msg, strerror(err_code));
    }

    printf("> [%s] [%s] %s\n", timestamp, type_to_string(type), msg);
}

void MessageManager::debug(const char* msg, int err_code) { print(DEBUG, msg, err_code); }
void MessageManager::info(const char* msg,  int err_code) { print(INFO,  msg, err_code); }
void MessageManager::warn(const char* msg,  int err_code) { print(WARN,  msg, err_code); }
void MessageManager::error(const char* msg, int err_code) { print(ERROR, msg, err_code); }