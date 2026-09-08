#include "CommandParser.h"

#include <iostream>
#include <sstream>
#include <utility>

void CommandParser::PrintUsage(const char *pszProg) {
    std::cerr << "Usage: " << pszProg << " [--socket PATH]\n";
    std::cerr << "Starts interactive command mode. Type 'quit' or 'exit' to leave.\n";
}

int CommandParser::ParseArgs(int iArgc, char **ppszArgv) {
    m_strSocketPath = "/var/run/Daemon_Socket";
    m_strIniPath.clear();

    for (int iIndex = 1; iIndex < iArgc; ++iIndex) {
        std::string strArg = ppszArgv[iIndex];
        if (strArg == "--socket" && iIndex + 1 < iArgc) {
            m_strSocketPath = ppszArgv[++iIndex];
        } else if (strArg == "--inifile" && iIndex + 1 < iArgc) {
            m_strIniPath = ppszArgv[++iIndex];
        } else if (strArg == "--configfile" && iIndex + 1 < iArgc) {
            m_configJson = ppszArgv[++iIndex];
            std::cout<< "Config file path: " << m_configJson << std::endl;
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

std::string CommandParser::LoadSocketPathFromIni() const {
    if (m_strIniPath.empty()) {
        return std::string();
    }

    IniConfig iniConfig(m_strIniPath);
    return iniConfig.get("SOCKET_PATH");
}

std::string CommandParser::ResolveSocketPath() {
    const std::string strIniSocketPath = LoadSocketPathFromIni();
    return strIniSocketPath.empty() ? m_strSocketPath : strIniSocketPath;
}

CommandParser::CommandParser(int iArgc, char **ppszArgv)
    : m_strInput(), m_strSocketPath(), m_socket(""), m_bValid(true), m_iExitCode(-1) {
    m_iExitCode = ParseArgs(iArgc, ppszArgv);
    if (m_iExitCode == 0 || m_iExitCode == 1) {
        m_bValid = false;
        return;
    }
    // If a JSON config file path was provided, try to open it and allow it
    // to override the socket path via the key "SOCKET_PATH".
    if (!m_configJson.empty()) {
        if (m_configParser.Open(m_configJson)) {
            if (m_configParser.HasKey("SOCKET_PATH")) {
                m_strSocketPath = m_configParser.GetValue("SOCKET_PATH");
            }
        } else {
            std::cerr << "Warning: failed to open config file: " << m_configJson << std::endl;
        }
    }

    m_strSocketPath = ResolveSocketPath();
    m_socket = DaemonSocket(m_strSocketPath);
}

CommandParser::CommandParser(std::string strSocketPath)
    : m_strInput(), m_strSocketPath(std::move(strSocketPath)), m_socket(m_strSocketPath), m_bValid(true), m_iExitCode(-1) {}

bool CommandParser::IsValid() const {
    return m_bValid;
}

int CommandParser::ExitCode() const {
    return m_iExitCode;
}

const std::string &CommandParser::SocketPath() const {
    return m_strSocketPath;
}

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
