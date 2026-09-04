#include "CommandParser.h"

#include <sstream>
#include <utility>

CommandParser::CommandParser(std::string strSocketPath)
    : m_strInput(), m_socket(std::move(strSocketPath)) {}

void CommandParser::set_input(std::string strInput) {
    m_strInput = std::move(strInput);
}

bool CommandParser::connect(std::string &strErr) {
    return m_socket.connect_socket(strErr);
}

std::vector<std::string> CommandParser::parse() const {
    std::istringstream stream(m_strInput);
    std::vector<std::string> vecTokens;
    std::string strToken;

    while (stream >> strToken) {
        vecTokens.push_back(strToken);
    }

    return vecTokens;
}

std::string CommandParser::raw() const {
    return m_strInput;
}

std::optional<std::string> CommandParser::execute(std::string &strErr) {
    if (!m_socket.send_message(m_strInput, strErr)) {
        return std::nullopt;
    }

    return m_socket.receive_response(strErr);
}
