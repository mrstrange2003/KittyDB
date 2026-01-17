// insert.cpp

#include "insert.h"
#include "schema.h"
#include "types.h"

#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>

// helpers

static int getNextId(
    const std::string &databaseName,
    const std::string &tableName,
    std::string &error)
{
    std::string path = "..\\databases\\" + databaseName + "\\" + tableName + ".seq";

    std::ifstream in(path);
    if (!in)
    {
        error = "Failed to read sequence file";
        return -1;
    }

    int lastId = 0;
    in >> lastId;
    in.close();

    int nextId = lastId + 1;

    std::ofstream out(path, std::ios::trunc);
    if (!out)
    {
        error = "Failed to update sequence file";
        return -1;
    }

    out << nextId;
    out.close();

    return nextId;
}

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

// main logic

bool insertRow(
    const std::string &databaseName,
    const std::string &tableName,
    const std::string &values,
    std::string &error)
{
    // basic checks
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

    // parse VALUES
    std::vector<std::string> parsedValues;
    std::string temp;

    for (char c : values)
    {
        if (c == ',')
        {
            parsedValues.push_back(cleanValue(temp));
            temp.clear();
        }
        else
        {
            temp += c;
        }
    }
    parsedValues.push_back(cleanValue(temp));

    // user provides values for all columns EXCEPT __id
    if (parsedValues.size() != columns.size() - 1)
    {
        error = "Column count mismatch";
        return false;
    }

    // validate only user columns
    for (size_t i = 1; i < columns.size(); i++)
    {
        if (!validateValueForType(parsedValues[i - 1], columns[i].type))
        {
            error = "Type mismatch for column '" + columns[i].name +
                    "' (expected " + columns[i].type + ")";
            return false;
        }
    }

    // get auto-increment id
    int id = getNextId(databaseName, tableName, error);
    if (id == -1)
    {
        return false;
    }

    // build row string
    std::string row = std::to_string(id) + "|";

    for (size_t i = 0; i < parsedValues.size(); i++)
    {
        row += parsedValues[i];
        if (i + 1 < parsedValues.size())
            row += "|";
    }

    // append to table
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
