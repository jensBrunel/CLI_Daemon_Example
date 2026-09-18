#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <string>
#include <unordered_map>

#include "rapidjson/document.h"
#include <fstream>

class ConfigParser {
public:
    /**
     * @brief Construct an empty parser.
     */
    ConfigParser();

    /**
     * @brief Construct a parser for a specific .cfg file.
     * @param strPath Path to the configuration file.
     */
    explicit ConfigParser(const std::string &strPath);

    /**
     * @brief Open and parse a configuration file.
     * @param strPath Path to the .cfg file.
     * @return true if the file was opened and parsed successfully, false otherwise.
     */
    bool Open(const std::string &strPath);

    /**
     * @brief Get the path of the currently loaded config file.
     * @return Path to the config file.
     */
    const std::string &GetPath() const;

private:
    void Load();
    void ParseValue(const rapidjson::Value &value, const std::string &strPrefix);
    static std::string ValueToString(const rapidjson::Value &value);
    static std::string Uppercase(const std::string &strText);

    std::unordered_map<std::string, std::string> m_mapValues;
    std::string m_strPath;
    std::ifstream m_stream;
};

#endif // CONFIGPARSER_H
