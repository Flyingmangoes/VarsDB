#ifndef RESP_H
#define RESP_H

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include "TCP.hpp"
#include "Command.hpp"
#include "Message.hpp"

class RESP {
private:
    SOCKET client_fd;

    bool tokenize(const std::string& line, std::vector<std::string>& out);
    std::string encode_command(const std::vector<std::string>& args);
    int send_request(const std::string& raw);
    int read_response();
public:
    RESP(SOCKET client_fd);
    void loop();
};

enum class RespType { Unknown, SimpleString, Error, Integer, BulkString, Array };
enum class ParseStatus { Ok, Incomplete, Error };

struct ParseResult {
    ParseStatus status = ParseStatus::Incomplete;
    std::vector<std::string> args;
    size_t consumed = 0;
    std::string error;
};

namespace RESP_Parser {
    RespType detect_type(char first);
    ParseResult parse(std::string_view raw);
    StartupType parse_startup_mode(std::string_view raw);
    StorageType parse_storage(std::string_view raw);

    template <typename TList, typename TResult>
    TResult parse_table(std::string_view input, const TList* table, TResult unknown) {
        if (input.empty()) return unknown;

        std::string lower(input);
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

        for (int i = 0; table[i].name != NULL; i++) {
            if (lower == table[i].name) return table[i].target;
        }
        return unknown;
    }

    inline std::string simple_string(const std::string& s) { return "+" + s + "\r\n"; }
    inline std::string error(const std::string& s)         { return "-" + s + "\r\n"; }
    inline std::string null_simple()                       { return "_\r\n"; }
    inline std::string integer(long long n)                { return ":" + std::to_string(n) + "\r\n"; }
    inline std::string bulk_string(const std::string& s)   { return "$" + std::to_string(s.size()) + "\r\n" + s + "\r\n"; }
    inline std::string null_bulk()                         { return "$-1\r\n"; }
}

#endif