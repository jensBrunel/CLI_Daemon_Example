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
 * @param pszProg Program name (argv[0]).
 */
static void print_usage(const char *pszProg) {
    std::cerr << "Usage: " << pszProg << " [--socket PATH]\n";
    std::cerr << "Starts interactive command mode. Type 'quit' or 'exit' to leave.\n";
}

/**
 * @brief Program entry point.
 *
 * Supported command-line options:
 * - `--socket PATH` : path to the daemon unix-domain socket (default `/var/run/Daemon_Socket`)
 * - `-h`, `--help`   : print this help message
 */
int main(int iArgc, char **ppszArgv) {
    std::string strSocketPath = "/var/run/Daemon_Socket"; // default path

    for (int iIndex = 1; iIndex < iArgc; ++iIndex) {
        std::string strArg = ppszArgv[iIndex];
        if (strArg == "--socket" && iIndex + 1 < iArgc) {
            strSocketPath = ppszArgv[++iIndex];
        } else if (strArg == "-h" || strArg == "--help") {
            print_usage(ppszArgv[0]);
            return 0;
        } else {
            print_usage(ppszArgv[0]);
            return 1;
        }
    }

    DaemonSocket client(strSocketPath);
    std::string strErr;
    if (!client.connect_socket(strErr)) {
        std::cerr << "Failed to connect to " << strSocketPath << ": " << strErr << "\n";
        return 2;
    }

    // interactive mode
    std::string strLine;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, strLine)) break;

        CommandParser commandParser(strLine);
        const std::vector<std::string> vecTokens = commandParser.parse();

        if (vecTokens.empty()) {
            continue;
        }

        const std::string strCommand = vecTokens.front();
        if (strCommand == "quit" || strCommand == "exit") break;

        if (!client.send_message(commandParser.raw(), strErr)) {
            std::cerr << "Send failed: " << strErr << "\n";
            continue;
        }
        auto optStrResp = client.receive_response(strErr);
        if (!optStrResp) {
            std::cerr << "Receive failed: " << strErr << "\n";
            continue;
        }
        std::cout << *optStrResp << std::endl;
    }

    return 0;
}
