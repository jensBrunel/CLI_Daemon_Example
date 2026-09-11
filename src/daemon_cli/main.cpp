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
 * @brief Program entry point.
 *
 * Supported command-line options:
 * - `--socket PATH` : path to the daemon unix-domain socket (default `/var/run/Daemon_Socket`)
 * - `-h`, `--help`   : print this help message
 */
int main(int iArgc, char **ppszArgv) {
    CommandParser commandParser(iArgc, ppszArgv);
    const int ec = commandParser.ExitCode();
    if (ec == 0) {
        return 0; // help printed
    }
    if (ec == 1) {
        std::cout << "Invalid command-line arguments." << std::endl;
        return 1;
    }

    std::string strErr;

    // interactive mode
    std::string strLine;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, strLine)) break;

        commandParser.SetInput(strLine);
        if (commandParser.IsLocalOnlyCommand()) {
            if (!commandParser.HandleInput(strErr, std::cout)) {
                break;
            }
            continue;
        }

        if (!commandParser.Connect(strErr)) {
            std::cerr << "Failed to connect to " << commandParser.SocketPath() << ": " << strErr << "\n";
            return 2;
        }

        if (!commandParser.HandleInput(strErr, std::cout)) {
            break;
        }
    }

    return 0;
}
