// update.cpp

#include "update.h"
#include "schema.h"
#include "where.h"
#include "types.h"

#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>

// helpers
static bool directoryExists(const std::string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

static bool fileExists(const std::string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

// trim helper
static std::string trim(std::string s)
{
    while (!s.empty() && s.front() == ' ')
        s.erase(s.begin());
    while (!s.empty() && s.back() == ' ')
        s.pop_back();
    return s;
}

// remove quotes
static std::string stripQuotes(std::string v)
{
    if (v.size() >= 2 && v.front() == '"' && v.back() == '"')
        return v.substr(1, v.size() - 2);
    return v;
}

// parse SET clause: col1=val1, col2=val2
static bool parseSetClause(
    const std::string &setClause,
    std::vector<std::pair<std::string, std::string>> &assignments)
{
    std::string temp;

    for (char c : setClause)
    {
        if (c == ',')
        {
            size_t eq = temp.find('=');
            if (eq == std::string::npos)
                return false;

            std::string col = trim(temp.substr(0, eq));
            std::string val = stripQuotes(trim(temp.substr(eq + 1)));

            assignments.push_back({col, val});
            temp.clear();
        }
        else
        {
            temp += c;
        }
    }

    if (!temp.empty())
    {
        size_t eq = temp.find('=');
        if (eq == std::string::npos)
            return false;

        std::string col = trim(temp.substr(0, eq));
        std::string val = stripQuotes(trim(temp.substr(eq + 1)));
        assignments.push_back({col, val});
    }

    return !assignments.empty();
}

bool updateWhere(
    const std::string &databaseName,
    const std::string &tableName,
    const std::string &setClause,
    bool hasWhere,
    const WhereCondition &where,
    std::string &error)
{
    if (databaseName.empty())
    {
        error = "No database selected";
        return false;
    }

    if (!hasWhere)
    {
        error = "UPDATE without WHERE is not allowed";
        return false;
    }

    std::string basePath = "..\\databases\\" + databaseName + "\\";
    std::string tblPath = basePath + tableName + ".tbl";
    std::string metaPath = basePath + tableName + ".meta";

    if (!directoryExists(basePath))
    {
        error = "Database does not exist";
        return false;
    }

    if (!fileExists(tblPath) || !fileExists(metaPath))
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

    // parse SET clause
    std::vector<std::pair<std::string, std::string>> assignments;
    if (!parseSetClause(setClause, assignments))
    {
        error = "Invalid SET clause";
        return false;
    }

    // Check if user is trying to modify __id
    for (const auto &a : assignments)
    {
        if (a.first == "__id")
        {
            error = "Cannot modify system column '__id'";
            return false;
        }
    }

    // map assignments to column indexes
    std::vector<std::pair<int, std::string>> updates;
    for (const auto &a : assignments)
    {
        int idx = -1;
        for (size_t i = 0; i < columns.size(); i++)
        {
            if (columns[i].name == a.first)
            {
                idx = (int)i;
                break;
            }
        }

        if (idx == -1)
        {
            error = "Unknown column '" + a.first + "'";
            return false;
        }

        if (!validateValueForType(a.second, columns[idx].type))
        {
            error = "Invalid value for column '" + columns[idx].name + "'";
            return false;
        }

        updates.push_back({idx, a.second});
    }

    // read rows
    std::ifstream in(tblPath);
    if (!in)
    {
        error = "Failed to open table";
        return false;
    }

    std::vector<std::string> updatedRows;
    std::string line;
    int updatedCount = 0;

    while (std::getline(in, line))
    {
        std::vector<std::string> fields;
        std::string temp;

        for (char c : line)
        {
            if (c == '|')
            {
                fields.push_back(temp);
                temp.clear();
            }
            else
            {
                temp += c;
            }
        }
        fields.push_back(temp);

        if (evaluateWhere(columns, fields, where))
        {
            for (const auto &u : updates)
            {
                if (u.first < (int)fields.size())
                    fields[u.first] = u.second;
            }
            updatedCount++;
        }

        std::string newRow;
        for (size_t i = 0; i < fields.size(); i++)
        {
            newRow += fields[i];
            if (i + 1 < fields.size())
                newRow += "|";
        }

        updatedRows.push_back(newRow);
    }
    in.close();

    if (updatedCount == 0)
    {
        error = "No rows matched WHERE clause";
        return false;
    }

    // rewrite table
    std::ofstream out(tblPath, std::ios::trunc);
    if (!out)
    {
        error = "Failed to rewrite table";
        return false;
    }

    for (const auto &r : updatedRows)
        out << r << "\n";

    out.close();
    return true;
}
