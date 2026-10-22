#include "TCP.hpp"
#include "RESP.hpp"
#include "Message.hpp"
#include <cstdio>

void print_help() {
    printf("");
    printf("");
};

int main(int argc, char** argv) {
    if (argc < 2) {
        vars::MessageFactory::error("Insufficient arguments - Usage: ./redis [server|client]");
        return EXIT_FAILURE;
    }

    CmdType cmd = RESP_Parser::parse_command(argv[1]);

    /*For now separate server and client soon server be replace to run in the background*/ 
    switch(cmd) {
        case CMD_SERVER: 
            vars::MessageFactory::info("Starting server");
            Server s = Server();
            s.setAddress(DEFAULT_PORT);
            if (s.setup_server() < 0) return EXIT_FAILURE;
            if(s.start() != 0) return EXIT_FAILURE;
            return EXIT_SUCCESS;
        case CMD_CLIENT: 
            vars::MessageFactory::info("Starting client");
            Client c = Client();
            c.setAddress(DEFAULT_PORT);
            if(c.setup_client() < 0) return EXIT_FAILURE;
            if (c.connect_to_server()  < 0) return EXIT_FAILURE;

            RESP resp = RESP(c.getSocket());
            resp.loop();
            return EXIT_SUCCESS;
        case CMD_HELP: 
            print_help();
            return EXIT_SUCCESS;
        case CMD_UNKNOWN:
        default:
            vars::MessageFactory::error("Unknown command - Usage: ./redis [server | client]");
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}