#ifndef UTILS_H
#define UTILS_H

#include <errno.h>
#include <assert.h>
#include <string.h>

#define MAX_MESSAGE_BUFFER 512
#define MAX_COMMAND_BUFFER 128

class MessageManager {
public:
    typedef enum message_type {
        DEBUG = 0,
        INFO  = 1,
        WARN  = 2,
        ERROR = 3,
    } message_type;

    static void print(message_type type, const char* msg, int err_code = 0);
    static void print_client(message_type type, const int* fd, char* hbuf, char* sbuf);

    static void debug(const char* msg, int err_code = 0);
    static void info(const char* msg, int err_code = 0);
    static void warn(const char* msg, int err_code = 0);
    static void error(const char* msg, int err_code = 0);

private:
    static const char* type_to_string(message_type type);
    static void get_timestamp(char* buffer, size_t size);
};

#endif