#include "CommandParser.h"

#include <sstream>
#include <utility>

CommandParser::CommandParser(std::string strInput) : m_strInput(std::move(strInput)) {}

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
