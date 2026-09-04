#include "CommandParser.h"

#include <iostream>
#include <sstream>
#include <utility>

void CommandParser::PrintUsage(const char *pszProg) {
    std::cerr << "Usage: " << pszProg << " [--socket PATH]\n";
    std::cerr << "Starts interactive command mode. Type 'quit' or 'exit' to leave.\n";
}

int CommandParser::ParseArgs(int iArgc, char **ppszArgv, std::string &strSocketPath) {
    strSocketPath = "/var/run/Daemon_Socket";

    for (int iIndex = 1; iIndex < iArgc; ++iIndex) {
        std::string strArg = ppszArgv[iIndex];
        if (strArg == "--socket" && iIndex + 1 < iArgc) {
            strSocketPath = ppszArgv[++iIndex];
        } else if (strArg == "-h" || strArg == "--help") {
            PrintUsage(ppszArgv[0]);
            return 0;
        } else {
            PrintUsage(ppszArgv[0]);
            return 1;
        }
    }

    return -1;
}

CommandParser::CommandParser(std::string strSocketPath)
    : m_strInput(), m_socket(std::move(strSocketPath)) {}

void CommandParser::SetInput(std::string strInput) {
    m_strInput = std::move(strInput);
}

bool CommandParser::Connect(std::string &strErr) {
    return m_socket.connect_socket(strErr);
}

std::vector<std::string> CommandParser::Parse() const {
    std::istringstream stream(m_strInput);
    std::vector<std::string> vecTokens;
    std::string strToken;

    while (stream >> strToken) {
        vecTokens.push_back(strToken);
    }

    return vecTokens;
}

std::string CommandParser::Raw() const {
    return m_strInput;
}

std::optional<std::string> CommandParser::Execute(std::string &strErr) {
    if (!m_socket.send_message(m_strInput, strErr)) {
        return std::nullopt;
    }

    return m_socket.receive_response(strErr);
}

bool CommandParser::HandleInput(std::string &strErr, std::ostream &out) {
    const std::vector<std::string> vecTokens = Parse();

    if (vecTokens.empty()) {
        return true;
    }

    const std::string strCommand = vecTokens.front();
    if (strCommand == "quit" || strCommand == "exit") {
        return false;
    }

    auto optStrResp = Execute(strErr);
    if (!optStrResp) {
        out << "Command execution failed: " << strErr << '\n';
        return true;
    }

    out << *optStrResp << std::endl;
    return true;
}
