#include <iostream>
#include "utilities.hpp"

struct cmd_target_list cmd_targets[] = {
    {"server", CMD_SERVER},
    {"client", CMD_CLIENT},
    {NULL, CMD_UNKNOWN} 
};

struct storage_target_list storage_targets[] = {
    {"standard", DEST_STANDARD},
    {"temporary", DEST_TEMPORARY},
    {"list", DEST_LIST},
    {"hash", DEST_HASH},
    {NULL, DEST_UNKNOWN}
};

cmd_target utils::Parser::parse_cmd(const char* input) {
    if(input == NULL) {
        utils::MessageManager::error("No command provided");
        return CMD_UNKNOWN;
    }

    cmd_target result = parse_table(input, cmd_targets, CMD_UNKNOWN);
    return result;
};

storage_target utils::Parser::parse_storage(const char* input) {
    if (input == NULL) {
        utils::MessageManager::error("No destination provided");
        return DEST_UNKNOWN;
    }

    storage_target result = parse_table(input, storage_targets, DEST_UNKNOWN);
    if (result == DEST_UNKNOWN) {
        utils::MessageManager::error("Unknown destination provided");
    }

    return result;
};

template <typename TList, typename TResult> TResult utils::Parser::parse_table(const char* input, TList* table, TResult unknown) {
    if(input == NULL) return unknown;

    char lower[32] = {0};
    size_t len = strlen(input);
    if(len < sizeof(lower)) len = sizeof(lower) - 1;
    
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