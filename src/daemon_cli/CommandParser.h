#pragma once

#include <string>
#include <vector>

class CommandParser {
public:
    explicit CommandParser(std::string input);

    std::vector<std::string> parse() const;
    std::string raw() const;

private:
    std::string input_;
};
