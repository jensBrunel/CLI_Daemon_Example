#include "ConfigParser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

namespace {
std::string JsonValueToString(const rapidjson::Value &value) {
    if (value.IsString()) {
        return value.GetString();
    }
    if (value.IsBool()) {
        return value.GetBool() ? "true" : "false";
    }
    if (value.IsInt()) {
        return std::to_string(value.GetInt());
    }
    if (value.IsUint()) {
        return std::to_string(value.GetUint());
    }
    if (value.IsInt64()) {
        return std::to_string(value.GetInt64());
    }
    if (value.IsUint64()) {
        return std::to_string(value.GetUint64());
    }
    if (value.IsDouble()) {
        return std::to_string(value.GetDouble());
    }
    if (value.IsNull()) {
        return "";
    }
    if (value.IsArray()) {
        std::ostringstream stream;
        for (rapidjson::SizeType i = 0; i < value.Size(); ++i) {
            if (i > 0) {
                stream << ",";
            }
            stream << JsonValueToString(value[i]);
        }
        return stream.str();
    }
    return "";
}
}

std::string ConfigParser::Uppercase(const std::string &strText) {
    std::string strResult = strText;
    std::transform(strResult.begin(), strResult.end(), strResult.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    return strResult;
}

ConfigParser::ConfigParser() : m_strPath() {
}

ConfigParser::ConfigParser(const std::string &strPath) : m_strPath(strPath) {
    Load();
}

bool ConfigParser::Open(const std::string &strPath) {
    m_strPath = strPath;
    m_mapValues.clear();
    Load();
    return !m_strPath.empty();
}

bool ConfigParser::IsOpen() const {
    return !m_strPath.empty();
}

void ConfigParser::ParseValue(const rapidjson::Value &value, const std::string &strPrefix) {
    if (value.IsObject()) {
        for (auto it = value.MemberBegin(); it != value.MemberEnd(); ++it) {
            const std::string strKey = strPrefix.empty() ? it->name.GetString() : strPrefix + "." + it->name.GetString();
            ParseValue(it->value, strKey);
        }
        return;
    }

    if (strPrefix.empty()) {
        return;
    }

    m_mapValues[Uppercase(strPrefix)] = JsonValueToString(value);
}

void ConfigParser::Load() {
    std::ifstream stream(m_strPath);
    if (!stream.is_open()) {
        return;
    }

    std::string strJson((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    rapidjson::Document document;
    document.Parse(strJson.c_str());
    if (!document.IsObject()) {
        return;
    }

    m_mapValues.clear();
    ParseValue(document, "");
}

std::string ConfigParser::GetValue(const std::string &strKey) const {
    const auto it = m_mapValues.find(Uppercase(strKey));
    if (it == m_mapValues.end()) {
        return "";
    }
    return it->second;
}

bool ConfigParser::HasKey(const std::string &strKey) const {
    return m_mapValues.find(Uppercase(strKey)) != m_mapValues.end();
}
