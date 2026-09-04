#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <optional>
#include <string>
#include <vector>

#include "DaemonSocket.h"

class CommandParser {
public:
    explicit CommandParser(std::string strSocketPath);

    void set_input(std::string strInput);
    bool connect(std::string &strErr);
    std::vector<std::string> parse() const;
    std::string raw() const;
    std::optional<std::string> execute(std::string &strErr);

private:
    std::string m_strInput;
    DaemonSocket m_socket;
};

#endif // COMMANDPARSER_H
