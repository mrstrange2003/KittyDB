#include "db.h"

#include <direct.h> // _mkdir
#include <string>

bool createDatabase(const std::string &dbName, std::string &error)
{
    if (dbName.empty())
    {
        error = "Database name can't be empty";
        return false;
    }

    std::string path = "..\\databases\\" + dbName;

    // Try creating directory
    int result = _mkdir(path.c_str());

    if (result == 0)
    {
        return true; // success
    }
    else
    {
        error = "Database already exists and cannot be created";
        return false;
    }
}
