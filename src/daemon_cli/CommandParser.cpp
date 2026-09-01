#include "CommandParser.h"

#include <sstream>

CommandParser::CommandParser(std::string input) : input_(std::move(input)) {}

std::vector<std::string> CommandParser::parse() const {
    std::istringstream stream(input_);
    std::vector<std::string> tokens;
    std::string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string CommandParser::raw() const {
    return input_;
}
