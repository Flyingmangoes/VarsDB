#include <iostream>
#include "server.hpp"
#include "utilities.hpp"
#include "resp.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        MessageManager::error("Usage: ./redis [server|client]");
        return EXIT_FAILURE;
    }

    Parser parser(argc, argv);
    cmd_target cmd = parser.parse();

    switch(cmd) {
        case CMD_SERVER: {
            MessageManager::info("Starting server");
            Server s = Server();
            s.setAddress(DEFAULT_PORT);
            if (s.setup_server() < 0) return EXIT_FAILURE;
            return s.start();
        }

        case CMD_CLIENT: {
            MessageManager::info("Starting client");
            Client c = Client();
            c.setAddress(DEFAULT_PORT);
            if(c.setup_client() < 0) return EXIT_FAILURE;
            if (c.connect_to_server()  < 0) return EXIT_FAILURE;

            CLI_Manager cli = CLI_Manager(c.getSocket());
            cli.loop();
            return EXIT_SUCCESS;
        }

        case CMD_UNKNOWN:
        default:
            MessageManager::error("Unknown command - use: server | client");
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}