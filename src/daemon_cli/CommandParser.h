#pragma once

#include <string>
#include <vector>
#include "vtss_ocelot_reg.h"

class CommandParser {
public:
    explicit CommandParser(std::string strInput);

    std::vector<std::string> parse() const;
    std::string raw() const;

private:
    std::string m_strInput;
};
