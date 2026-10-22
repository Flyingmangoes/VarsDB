#include "RESP.hpp"
using namespace std;

#define PARSER_BUFFER 4096

CmdLists cmd_lists[] = {
    {"server", CMD_SERVER},
    {"client", CMD_CLIENT},
    {NULL, CMD_UNKNOWN} 
};

struct StorageList storage_lists[] = {
    {"standard", STANDARD},
    {"temporary", TEMPORARY},
    {"list", LIST},
    {"hash", HASH},
    {NULL, UNKNOWN}
};

CmdType RESP_Parser::parse_command(const char* input) {
    if(input == NULL) {
        vars::MessageFactory::error("No command provided");
        return CMD_UNKNOWN;
    }

    CmdType result = parse_table(input, cmd_lists, CMD_UNKNOWN);
    return result;
};

StorageType RESP_Parser::parse_storage(const char* input) {
    if (input == NULL) {
        vars::MessageFactory::error("No destination provided");
        return UNKNOWN;
    }

    StorageType result = parse_table(input, storage_lists, UNKNOWN);
    if (result == UNKNOWN) {
        vars::MessageFactory::error("Unknown destination provided");
    }

    return result;
};

RespType RESP_Parser::detect_type(char first) {
    switch (first) {
        case '+': return RespType::SimpleString;
        case '-': return RespType::Error;
        case ':': return RespType::Integer;
        case '$': return RespType::BulkString;
        case '*': return RespType::Array;
        default:  return RespType::Unknown;
    }
};

ParseResult RESP_Parser::parse(const char* raw_bytes) {
    RespType types = detect_type(raw_bytes[0]);
    switch (types){
    case RespType::SimpleString:
        
        break;
    case RespType::BulkString:
        break;
    case RespType::Integer:
        break;
    case RespType::Array:
        break;
    case RespType::Unknown:
    default:
        break;
    }
};

