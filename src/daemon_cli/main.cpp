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
    std::string strSocketPath;
    const int iParseStatus = CommandParser::ParseArgs(iArgc, ppszArgv, strSocketPath);
    if (iParseStatus == 0 || iParseStatus == 1) {
        return iParseStatus;
    }

    std::string strErr;
    CommandParser commandParser(strSocketPath);
    if (!commandParser.Connect(strErr)) {
        std::cerr << "Failed to connect to " << strSocketPath << ": " << strErr << "\n";
        return 2;
    }

    // interactive mode
    std::string strLine;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, strLine)) break;

        commandParser.SetInput(strLine);
        if (!commandParser.HandleInput(strErr, std::cout)) {
            break;
        }
    }

    return 0;
}
