/**
 * @file main.cpp
 * @brief Simple CLI client for Daemon_Socket using Unix domain sockets.
 *
 * This executable connects to a Unix-domain socket (default: /var/run/Daemon_Socket)
 * and sends/receives newline-terminated messages. It supports one-shot sends via
 * `--send` and an interactive REPL mode.
 */

#include <iostream>
#include <optional>
#include <string>

#include "DaemonSocket.h"



/**
 * @brief Print usage information for the CLI.
 * @param prog Program name (argv[0]).
 */
static void print_usage(const char *prog) {
    std::cerr << "Usage: " << prog << " [--socket PATH] [--send MESSAGE]\n";
    std::cerr << "If --send is omitted, enters interactive mode where each line is sent.\n";
}

/**
 * @brief Program entry point.
 *
 * Supported command-line options:
 * - `--socket PATH` : path to the daemon unix-domain socket (default `/var/run/Daemon_Socket`)
 * - `--send MESSAGE`: send a single MESSAGE and exit (one-shot mode)
 * - `-h`, `--help`   : print this help message
 */
int main(int argc, char **argv) {
    std::string socket_path = "/var/run/Daemon_Socket"; // default path
    std::optional<std::string> one_shot_msg;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--socket" && i + 1 < argc) {
            socket_path = argv[++i];
        } else if (a == "--send" && i + 1 < argc) {
            one_shot_msg = argv[++i];
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

    if (one_shot_msg.has_value()) {
        if (!client.send_message(*one_shot_msg, err)) {
            std::cerr << "Send failed: " << err << "\n";
            return 3;
        }
        auto resp = client.receive_response(err);
        if (!resp) {
            std::cerr << "Receive failed: " << err << "\n";
            return 4;
        }
        std::cout << *resp << std::endl;
        return 0;
    }

    // interactive mode
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        if (!client.send_message(line, err)) {
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
