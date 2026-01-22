#include "types.h"

#include <cctype>
#include <regex>

static bool isInt(const std::string &v)
{
    if (v.empty())
        return false;
    size_t i = (v[0] == '-' ? 1 : 0);
    for (; i < v.size(); i++)
        if (!isdigit(v[i]))
            return false;
    return true;
}

static bool isFloat(const std::string &v)
{
    bool dotSeen = false;
    size_t i = (v[0] == '-' ? 1 : 0);

    for (; i < v.size(); i++)
    {
        if (v[i] == '.')
        {
            if (dotSeen)
                return false;
            dotSeen = true;
        }
        else if (!isdigit(v[i]))
        {
            return false;
        }
    }
    return dotSeen;
}

static bool isBool(const std::string &v)
{
    return v == "true" || v == "false" || v == "1" || v == "0";
}

static bool isChar(const std::string &v)
{
    return v.size() == 1;
}

static bool isDate(const std::string &v)
{
    std::regex datePattern(R"(\d{4}-\d{2}-\d{2})");
    return std::regex_match(v, datePattern);
}

bool validateValueForType(
    const std::string &value,
    const std::string &type)
{
    // NULL is allowed for all types
    if (value == "NULL")
        return true;

    if (type == "INT")
        return isInt(value);

    if (type == "FLOAT")
        return isFloat(value);

    if (type == "TEXT" || type == "VARCHAR")
        return true;

    if (type == "BOOL")
        return isBool(value);

    if (type == "CHAR")
        return isChar(value);

    if (type == "DATE")
        return isDate(value);

    // unknown datatype
    return false;
}
