#include "TCP.hpp"
#include "RESP.hpp"
#include "Message.hpp"
#include "Fork.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        vars::MessageFactory::error("Usage: ./redis [server|client]");
        return EXIT_FAILURE;
    }

    StartupType startup = RESP_Parser::parse_startup_mode(argv[1]);

    switch (startup) {
        case CMD_SERVER: {
            vars::MessageFactory::info("Starting server");
            Server s;
            s.setAddress(DEFAULT_PORT);
            if (s.setup_server() < 0) return EXIT_FAILURE;   
            if (!daemonize()) {
                vars::MessageFactory::error("Failed to daemonize");
                return EXIT_FAILURE;
            }
            write_pidfile("/tmp/varsdb.pid");                  
            return s.start() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        case CMD_CLIENT: {
            vars::MessageFactory::info("Starting client");
            Client c;
            c.setAddress(DEFAULT_PORT);
            if (c.setup_client() < 0) return EXIT_FAILURE;
            if (c.connect_to_server() < 0) {
                vars::MessageFactory::error("Could not connect. Is the server running?");
                return EXIT_FAILURE;
            }
            RESP resp(c.getSocket());
            resp.loop();
            return EXIT_SUCCESS;
        }
        default:
            vars::MessageFactory::error("Unknown command - Usage: ./redis [server|client]");
            return EXIT_FAILURE;
    }
}