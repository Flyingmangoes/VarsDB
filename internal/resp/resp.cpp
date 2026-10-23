#include <iostream>
#include "RESP.hpp"

#define CLI_PROMPT "VARS >"
#define CLI_EXIT "EXIT"
#define RESP_MESSAGE_BUFFER 4096

RESP::RESP(SOCKET client_fd) {
    this->client_fd = client_fd;
};

int RESP::read_response() {
    char raw_buf[RESP_MESSAGE_BUFFER] = {0};
    ssize_t count = recv(this->client_fd, raw_buf, sizeof(raw_buf) - 1, 0);

    if (count < 0) {
        vars::MessageFactory::error("Failed to read response", errno);
        return -1;
    }

    if (count == 0) {
        vars::MessageFactory::warn("Disconnected from server");
        return -1;
    }

    raw_buf[count] = '\0';
    std::cout<<raw_buf<<std::endl;
    return 0;
};

void RESP::loop() {
    assert(this->client_fd != -1);

    char raw_buf[RESP_MESSAGE_BUFFER];
    while(1) {
        std::cout<<CLI_PROMPT<<" ";
        std::cin.getline(raw_buf, sizeof(raw_buf));

        if (std::cin.eof()) {
            std::cout << "Exiting environment" << std::endl;
            break;
        }

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore();
            continue;
        }

        if (strlen(raw_buf) == 0) continue;

        if (strncmp(raw_buf, CLI_EXIT, strlen(CLI_EXIT)) == 0) {
            std::cout<<"Exiting environment"<<std::endl;
            break;
        }

        int rc = send_request(raw_buf);
        if (rc < 0)  break;
        if (rc == 1) continue;
        if (read_response() != 0) break;
    }   
};

bool RESP::tokenize(const std::string& line, std::vector<std::string>& out) {
    out.clear();
    std::string cur;
    bool in_token = false;
    char quote = 0;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (quote) {
            if (c == '\\' && quote == '"' && i + 1 < line.size()) {
                char n = line[++i];
                switch (n) {
                    case 'n': cur += '\n'; break;
                    case 'r': cur += '\r'; break;
                    case 't': cur += '\t'; break;
                    default:  cur += n;    break;
                }
            } else if (c == quote) {
                quote = 0;
            } else {
                cur += c;
            }
        }
        else if (c == '"' || c == '\'') {
            quote = c;
            in_token = true;              
        }
        else if (std::isspace(static_cast<unsigned char>(c))) {
            if (in_token) { out.push_back(cur); cur.clear(); in_token = false; }
        }
        else {
            cur += c;
            in_token = true;
        }
    }

    if (quote) return false;              
    if (in_token) out.push_back(cur);
    return true;
}

std::string RESP::encode_command(const std::vector<std::string>& args) {
    std::string out = "*" + std::to_string(args.size()) + "\r\n";
    for (const auto& a : args)
        out += "$" + std::to_string(a.size()) + "\r\n" + a + "\r\n";
    return out;
}

int RESP::send_request(const std::string& raw) {
    std::vector<std::string> args;
    if (!this->tokenize(raw, args)) {
        std::cout << "(error) unbalanced quotes" << std::endl;
        return 1;                              // nothing sent
    }
    if (args.empty()) return 1;                // blank line

    std::string encoded = encode_command(args);

    size_t sent = 0;
    while (sent < encoded.size()) {
        ssize_t n = send(this->client_fd, encoded.data() + sent,
                         encoded.size() - sent, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            vars::MessageFactory::error("Failed to send command", errno);
            return -1;
        }
        sent += static_cast<size_t>(n);
    }
    return 0;
}