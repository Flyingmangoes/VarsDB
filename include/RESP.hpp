#ifndef RESP_H
#define RESP_H

#include "TCP.hpp"
#include "Command.hpp"
#include "Message.hpp"
#include <cstring>
#include <vector>

class RESP {
private:
    SOCKET client_fd;
    ::Storage* storage;

    const char* encode_resp(const char* raw);
    int show_value_in_memory(); // print table and etc need Storage
    int send_request(const char* encoded);
    int read_response();
    int add_storage(::Storage* storage);
public:
    RESP(SOCKET client_fd);
    void loop();
};

enum class RespType { Unknown, SimpleString, Error, Integer, BulkString, Array };
enum class ParseStatus {Ok, Incomplete, Error };

typedef struct ParseResult {
    ParseStatus status;
    std::vector<std::string> args;
    size_t consumed = 0;
    std::string error;
};

namespace RESP_Parser {
    RespType detect_type(char first);
    ParseResult parse(const char* raw_bytes);
    CmdType parse_command(const char* input);
    StorageType parse_storage(const char* input);
    template <typename TList, typename TResult> static TResult parse_table(const char* input, TList* table, TResult unknown){
        if(input == NULL) return unknown;

        char lower[32] = {0};
        size_t len = strlen(input);
        if(len >= sizeof(lower)) len = sizeof(lower) - 1;
    
        for (size_t i=0; i < len;i++) {
            lower[i] = tolower(input[i]);
        }
        lower[len] = '\0';

        for(int i=0;table[i].name != NULL; i++) {
            if(strncmp(lower, table[i].name, strlen(table[i].name)) == 0) {
                return table[i].target;
            }
        }

        return unknown;
    };

    std::string inline simple_string(const std::string& s) { return "+" + s + "\r\n"; };
    std::string inline error(const std::string& s)         { return "-" + s + "\r\n"; };
    std::string inline null_simple()                       { return "_\r\n"; };
    std::string inline integer(long long n)                { return ":" + std::to_string(n) + "\r\n"; };
    std::string inline bulk_string(const std::string& s)   { return "$" + std::to_string(s.size()) + "\r\n" + s + "\r\n"; }
    std::string inline null_bulk()                         { return "$-1\r\n"; };
};

#endif