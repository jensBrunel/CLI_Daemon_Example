/**
 * @file main.cpp
 * @brief Simple CLI client for Daemon_Socket using Unix domain sockets.
 *
 * This executable connects to a Unix-domain socket (default: /var/run/Daemon_Socket)
 * and processes interactive command-line input in a REPL loop.
 */

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "CommandParser.h"
#include "DaemonSocket.h"

/**
 * @brief Print usage information for the CLI.
 * @param prog Program name (argv[0]).
 */
static void print_usage(const char *prog) {
    std::cerr << "Usage: " << prog << " [--socket PATH]\n";
    std::cerr << "Starts interactive command mode. Type 'quit' or 'exit' to leave.\n";
}

/**
 * @brief Program entry point.
 *
 * Supported command-line options:
 * - `--socket PATH` : path to the daemon unix-domain socket (default `/var/run/Daemon_Socket`)
 * - `-h`, `--help`   : print this help message
 */
int main(int argc, char **argv) {
    std::string socket_path = "/var/run/Daemon_Socket"; // default path

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--socket" && i + 1 < argc) {
            socket_path = argv[++i];
        } else if (a == "-h" || a == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    DaemonSocket client(socket_path);
    std::string err;
    if (!client.connect_socket(err)) {
        std::cerr << "Failed to connect to " << socket_path << ": " << err << "\n";
        return 2;
    }

    // interactive mode
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;

        CommandParser command_parser(line);
        const std::vector<std::string> tokens = command_parser.parse();

        if (tokens.empty()) {
            continue;
        }

        const std::string command = tokens.front();
        if (command == "quit" || command == "exit") break;

        if (!client.send_message(command_parser.raw(), err)) {
            std::cerr << "Send failed: " << err << "\n";
            continue;
        }
        auto resp = client.receive_response(err);
        if (!resp) {
            std::cerr << "Receive failed: " << err << "\n";
            continue;
        }
        std::cout << *resp << std::endl;
    }

    return 0;
}
