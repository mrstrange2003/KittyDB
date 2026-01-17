// schema.cpp

#include "schema.h"
#include <fstream>
#include <sstream>
#include <algorithm>

// trim helper
static std::string trim(std::string s)
{
    while (!s.empty() && s.front() == ' ')
        s.erase(s.begin());
    while (!s.empty() && s.back() == ' ')
        s.pop_back();
    return s;
}

static std::string toUpper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

bool parseSchema(
    const std::string &metaPath,
    std::vector<Column> &columns,
    std::string &error)
{
    std::ifstream metaFile(metaPath);
    if (!metaFile)
    {
        error = "Failed to open table schema";
        return false;
    }

    std::string line;
    std::getline(metaFile, line);
    metaFile.close();

    std::stringstream ss(line);
    std::string part;

    while (std::getline(ss, part, ','))
    {
        part = trim(part);

        std::stringstream colStream(part);
        std::string colName, colType;

        colStream >> colName >> colType;

        if (colName.empty() || colType.empty())
        {
            error = "Invalid schema format";
            return false;
        }

        Column c;
        c.name = colName;
        c.type = toUpper(colType); // enforce uppercase

        columns.push_back(c);
    }

    if (columns.empty())
    {
        error = "No columns defined in schema";
        return false;
    }

    // Inject system __id column at the beginning
    Column idCol;
    idCol.name = "__id";
    idCol.type = "INT";
    columns.insert(columns.begin(), idCol);

    return true;
}