#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include <ctype.h>

typedef enum {
    CMD_SERVER  = 1,
    CMD_CLIENT  = 2,
    CMD_UNKNOWN = 0
} cmd_target;

struct target_list {
    const char* name;
    cmd_target  target;
};

class Parser {
private:
    int    argc;
    char** argv;

    static target_list targets[];

public:
    Parser(int argc, char** argv);

    cmd_target parse();
};

#endif