#include <stdio.h>
#include <ctime>
#include "utilities.hpp"

const char* utils::MessageManager::type_to_string(message_type type) {
    switch (type) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        default:    return "UNKNOWN";
    }
}

void utils::MessageManager::get_timestamp(char* buffer, size_t size) {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void utils::MessageManager::print_client(message_type type, const int* fd, char* hbuf, char *sbuf) {
    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));

    printf("> [%s] [%s] Accepted connection on descriptor %d (host=%s, port=%s)\n", timestamp, type_to_string(type), *fd, hbuf, sbuf);
}

void utils::MessageManager::print(message_type type, const char* msg, int err_code) {
    char timestamp[20];
    get_timestamp(timestamp, sizeof(timestamp));
    if (err_code != 0) {
        printf("> [%s] [%s] %s — %s\n", timestamp, type_to_string(type), msg, strerror(err_code));
        fflush(stdout);
    }

    printf("> [%s] [%s] %s\n", timestamp, type_to_string(type), msg);
}

void utils::MessageManager::debug(const char* msg, int err_code) { print(DEBUG, msg, err_code); }
void utils::MessageManager::info(const char* msg,  int err_code) { print(INFO,  msg, err_code); }
void utils::MessageManager::warn(const char* msg,  int err_code) { print(WARN,  msg, err_code); }
void utils::MessageManager::error(const char* msg, int err_code) { print(ERROR, msg, err_code); }