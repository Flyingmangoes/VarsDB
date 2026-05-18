#ifndef REDIS_UTILS_H
#define REDIS_UTILS_H

#include <errno.h>
#include <assert.h>
#include <string.h>

#define MAX_MESSAGE_BUFFER 512
#define MAX_ARGUMENTS_BUFFER 256
#define MAX_COMMAND_BUFFER 128
#define MAX_RESPONSE_BUFFER 64

typedef enum {
    CMD_SERVER  = 1,
    CMD_CLIENT  = 2,
    CMD_UNKNOWN = 0
} cmd_target;

typedef enum {
    DEST_TEMPORARY = 1,
    DEST_STANDARD = 2,
    DEST_HASH = 3,
    DEST_LIST = 4,
    DEST_UNKNOWN = 0
} storage_target;

struct cmd_target_list {
    const char* name;
    cmd_target  target;    
};

struct storage_target_list {
    const char* name;
    storage_target target;
};

extern struct cmd_target_list cmd_targets[];
extern struct storage_target_list storage_targets[];

namespace utils {
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

    class Parser {
    public:
        static cmd_target parse_cmd(const char* input);
        static storage_target parse_storage(const char* input);
        template <typename TList, typename TResult> static TResult parse_table(const char* input, TList* table, TResult unknown);
    };
};

#endif