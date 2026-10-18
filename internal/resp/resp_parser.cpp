#include <iostream>
#include "RESP.hpp"

struct cmd_list cmd_lists[] = {
    {"server", CMD_SERVER},
    {"client", CMD_CLIENT},
    {NULL, CMD_UNKNOWN} 
};

struct storage_list storage_lists[] = {
    {"standard", STANDARD},
    {"temporary", TEMPORARY},
    {"list", LIST},
    {"hash", HASH},
    {NULL, UNKNOWN}
};

cmd_type RESP_Parser::parse_cmd(const char* input) {
    if(input == NULL) {
        vars::MessageFactory::error("No command provided");
        return CMD_UNKNOWN;
    }

    cmd_type result = parse_table(input, cmd_lists, CMD_UNKNOWN);
    return result;
};

storage_type RESP_Parser::parse_storage(const char* input) {
    if (input == NULL) {
        vars::MessageFactory::error("No destination provided");
        return UNKNOWN;
    }

    storage_type result = parse_table(input, storage_lists, UNKNOWN);
    if (result == UNKNOWN) {
        vars::MessageFactory::error("Unknown destination provided");
    }

    return result;
};