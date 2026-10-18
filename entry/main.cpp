#include "TCP.hpp"
#include "RESP.hpp"
#include "Logger.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        utils::MessageManager::error("Insufficient arguments - Usage: ./redis [server|client]");
        return EXIT_FAILURE;
    }

    cmd_type cmd = utils::Parser::parse_cmd(argv[1]);

    switch(cmd) {
        case CMD_SERVER: {
            utils::MessageManager::info("Starting server");
            Server s = Server();
            s.setAddress(DEFAULT_PORT);
            if (s.setup_server() < 0) return EXIT_FAILURE;
            if(s.start() != 0) return EXIT_FAILURE;
            return EXIT_SUCCESS;
        }

        case CMD_CLIENT: {
            utils::MessageManager::info("Starting client");
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
            utils::MessageManager::error("Unknown command - Usage: ./redis [server | client]");
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}