// insert.cpp

#include "insert.h"
#include "schema.h"
#include "types.h"

#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>

// ---------- helpers ----------

// check directory exists
static bool directoryExists(const std::string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

// check file exists
static bool fileExists(const std::string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

// trim spaces + remove surrounding quotes
static std::string cleanValue(std::string v)
{
    while (!v.empty() && v.front() == ' ')
        v.erase(v.begin());
    while (!v.empty() && v.back() == ' ')
        v.pop_back();

    if (v.size() >= 2 && v.front() == '"' && v.back() == '"')
    {
        v = v.substr(1, v.size() - 2);
    }
    return v;
}

// ---------- main logic ----------

bool insertRow(
    const std::string &databaseName,
    const std::string &tableName,
    const std::string &values,
    std::string &error)
{
    //  basic checks
    if (databaseName.empty())
    {
        error = "No database selected";
        return false;
    }

    std::string basePath = "..\\databases\\" + databaseName + "\\";

    if (!directoryExists(basePath))
    {
        error = "Database does not exist";
        return false;
    }

    std::string tblPath = basePath + tableName + ".tbl";
    std::string metaPath = basePath + tableName + ".meta";

    if (!fileExists(metaPath) || !fileExists(tblPath))
    {
        error = "Table does not exist";
        return false;
    }

    // parse schema
    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error))
    {
        return false;
    }

    //  parse VALUES into vector
    std::vector<std::string> parsedValues;
    std::string temp;

    for (size_t i = 0; i < values.size(); i++)
    {
        if (values[i] == ',')
        {
            parsedValues.push_back(cleanValue(temp));
            temp.clear();
        }
        else
        {
            temp += values[i];
        }
    }
    parsedValues.push_back(cleanValue(temp));

    //  enforce column count
    if (parsedValues.size() != columns.size())
    {
        error = "Column count mismatch";
        return false;
    }

    for (size_t i = 0; i < columns.size(); i++)
    {
        if (!validateValueForType(parsedValues[i], columns[i].type))
        {
            error = "Type mismatch for column '" + columns[i].name +
                    "' (expected " + columns[i].type + ")";
            return false;
        }
    }

    //  build row string (pipe-separated)
    std::string row;
    for (size_t i = 0; i < parsedValues.size(); i++)
    {
        row += parsedValues[i];
        if (i != parsedValues.size() - 1)
            row += "|";
    }

    //  append to .tbl
    std::ofstream tblFile(tblPath, std::ios::app);
    if (!tblFile)
    {
        error = "Failed to write to table";
        return false;
    }

    tblFile << row << "\n";
    tblFile.close();

    return true;
}