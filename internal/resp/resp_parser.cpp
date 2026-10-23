#include "RESP.hpp"
#include <charconv>
using namespace std;

StartupList startup_list[] = {
    {"server", CMD_SERVER},
    {"client", CMD_CLIENT},
    {NULL, CMD_UNKNOWN}
};

StorageList storage_list[] = {           
    {"standard",  STANDARD},
    {"temporary", TEMPORARY},
    {"list",      LIST},
    {"hash",      HASH},
    {NULL, UNKNOWN}
};

StartupType RESP_Parser::parse_startup_mode(std::string_view raw) {
    if (raw.empty()) {
        vars::MessageFactory::error("No command provided");
        return CMD_UNKNOWN;
    }
    return parse_table(raw, startup_list, CMD_UNKNOWN);
}

StorageType RESP_Parser::parse_storage(std::string_view raw) {
    if (raw.empty()) {
        vars::MessageFactory::error("No destination provided");
        return UNKNOWN;
    }
    StorageType result = parse_table(raw, storage_list, UNKNOWN);
    if (result == UNKNOWN) {
        vars::MessageFactory::error("Unknown destination provided");
    }
    return result;
}

RespType RESP_Parser::detect_type(char first) {
    switch (first) {
        case '+': return RespType::SimpleString;
        case '-': return RespType::Error;
        case ':': return RespType::Integer;
        case '$': return RespType::BulkString;
        case '*': return RespType::Array;
        default:  return RespType::Unknown;
    }
}

static int read_int_line(std::string_view buf, size_t& pos, long long& out) {
    size_t end = buf.find("\r\n", pos);
    if (end == std::string_view::npos) return 0;
    auto [p, ec] = std::from_chars(buf.data() + pos, buf.data() + end, out);
    if (ec != std::errc() || p != buf.data() + end) return -1;
    pos = end + 2;
    return 1;
}

ParseResult RESP_Parser::parse(std::string_view raw) {
    ParseResult r;                                   
    if (raw.empty()) return r;

    auto fail = [&](const char* msg) {
        r.status = ParseStatus::Error;
        r.error  = msg;
        return r;
    };

    if (detect_type(raw[0]) != RespType::Array) return fail("expected '*'");

    size_t pos = 1;
    long long n = 0;
    int rc = read_int_line(raw, pos, n);
    if (rc == 0) return r;
    if (rc < 0 || n > 1024 * 1024) return fail("invalid multibulk length");
    if (n <= 0) { r.status = ParseStatus::Ok; r.consumed = pos; return r; }

    std::vector<std::string> args;
    for (long long i = 0; i < n; i++) {
        if (pos >= raw.size()) return r;
        if (raw[pos] != '$') return fail("expected '$'");
        pos++;

        long long len = 0;
        rc = read_int_line(raw, pos, len);
        if (rc == 0) return r;
        if (rc < 0 || len < 0 || len > 512LL * 1024 * 1024) return fail("invalid bulk length");

        if (raw.size() - pos < static_cast<size_t>(len) + 2) return r;
        if (raw.substr(pos + len, 2) != "\r\n") return fail("missing CRLF");

        args.emplace_back(raw.substr(pos, len));
        pos += len + 2;
    }

    r.status   = ParseStatus::Ok;
    r.args     = std::move(args);
    r.consumed = pos;
    return r;
}