#include "table.h"
#include <fstream>
#include <string>
#include <sys/stat.h>

static bool directoryExists(const std::string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

// check if table files exist
static bool fileExists(const std::string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

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

    // Check if table already exists
    if (fileExists(metaPath))
    {
        error = "Table already exists";
        return false;
    }

    // Create .tbl (empty)
    std::ofstream tblFile(tblPath);
    if (!tblFile)
    {
        error = "Failed to create table data file";
        return false;
    }
    tblFile.close();

    // Create .meta (write schema)
    std::ofstream metaFile(metaPath);
    if (!metaFile)
    {
        error = "Failed to create table metadata file";
        return false;
    }
    metaFile << schema;
    metaFile.close();

    // Create .idx (empty placeholder)
    std::ofstream idxFile(idxPath);
    if (!idxFile)
    {
        error = "Failed to create table index file";
        return false;
    }
    idxFile.close();

    return true;
}
