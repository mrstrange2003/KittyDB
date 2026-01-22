// table.cpp

#include "table.h"
#include "schema.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <iomanip>
#include <algorithm>
#include <windows.h>

#undef max
#undef min

// Pretty table printer
void printPrettyTable(
    const std::vector<std::string> &headers,
    const std::vector<std::vector<std::string>> &rows)
{
    size_t cols = headers.size();
    std::vector<size_t> widths(cols, 0);

    for (size_t i = 0; i < cols; i++)
        widths[i] = headers[i].size();

    for (const auto &row : rows)
        for (size_t i = 0; i < cols; i++)
            widths[i] = std::max(widths[i], row[i].size());

    auto border = [&]()
    {
        std::cout << "+";
        for (auto w : widths)
            std::cout << std::string(w + 2, '-') << "+";
        std::cout << "\n";
    };

    border();

    std::cout << "|";
    for (size_t i = 0; i < cols; i++)
        std::cout << " " << std::left << std::setw(widths[i])
                  << headers[i] << " |";
    std::cout << "\n";

    border();

    for (const auto &row : rows)
    {
        std::cout << "|";
        for (size_t i = 0; i < cols; i++)
            std::cout << " " << std::left << std::setw(widths[i])
                      << row[i] << " |";
        std::cout << "\n";
    }

    border();
}

// Helpers
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

// TRUNCATE TABLE
bool truncateTable(
    const std::string &databaseName,
    const std::string &tableName,
    std::string &error)
{
    if (databaseName.empty())
    {
        error = "No database selected";
        return false;
    }

    std::string basePath = "..\\databases\\" + databaseName + "\\";
    std::string tblPath = basePath + tableName + ".tbl";
    std::string seqPath = basePath + tableName + ".seq";
    std::string metaPath = basePath + tableName + ".meta";

    if (!fileExists(metaPath))
    {
        error = "Table does not exist";
        return false;
    }

    std::ofstream tblFile(tblPath, std::ios::trunc);
    if (!tblFile)
    {
        error = "Failed to truncate table";
        return false;
    }

    std::ofstream seqFile(seqPath, std::ios::trunc);
    if (seqFile)
        seqFile << "0";

    return true;
}

// SHOW TABLES
bool showTables(
    const std::string &databaseName,
    std::string &error)
{
    if (databaseName.empty())
    {
        error = "No database selected";
        return false;
    }

    std::string searchPath =
        "..\\databases\\" + databaseName + "\\*.meta";

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    std::vector<std::vector<std::string>> rows;

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::string fileName = findData.cFileName;
            rows.push_back({fileName.substr(0, fileName.size() - 5)});
        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }

    printPrettyTable(
        {"Tables in " + databaseName},
        rows.empty() ? std::vector<std::vector<std::string>>{{"(no tables)"}} : rows);

    return true;
}

// DESCRIBE TABLE
bool describeTable(
    const std::string &databaseName,
    const std::string &tableName,
    std::string &error)
{
    if (databaseName.empty())
    {
        error = "No database selected";
        return false;
    }

    std::string metaPath =
        "..\\databases\\" + databaseName + "\\" + tableName + ".meta";

    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error))
        return false;

    std::vector<std::vector<std::string>> rows;

    for (const auto &col : columns)
        rows.push_back({col.name, col.type});

    printPrettyTable(
        {"Column", "Type"},
        rows);

    return true;
}

// CREATE TABLE
bool createTable(
    const std::string &databaseName,
    const std::string &tableName,
    const std::string &schema,
    std::string &error)
{
    if (databaseName.empty())
    {
        error = "No database selected";
        return false;
    }

    if (tableName.empty())
    {
        error = "Table name cannot be empty";
        return false;
    }

    std::string basePath = "..\\databases\\" + databaseName + "\\";

    if (!directoryExists(basePath))
    {
        error = "Database directory does not exist";
        return false;
    }

    std::string tblPath = basePath + tableName + ".tbl";
    std::string metaPath = basePath + tableName + ".meta";
    std::string idxPath = basePath + tableName + ".idx";
    std::string seqPath = basePath + tableName + ".seq";

    if (fileExists(metaPath))
    {
        error = "Table already exists";
        return false;
    }

    std::ofstream(tblPath).close();
    std::ofstream(metaPath) << schema;
    std::ofstream(idxPath).close();
    std::ofstream(seqPath) << "0";

    return true;
}
