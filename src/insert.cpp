#include "insert.h"
#include <fstream>
#include <string>
#include <sys/stat.h>

// check directory exists
static bool directoryExists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

// check file exists
static bool fileExists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

// remove quotes + spaces
static std::string cleanValue(std::string v) {
    while (!v.empty() && v.front() == ' ') v.erase(v.begin());
    while (!v.empty() && v.back() == ' ') v.pop_back();

    if (v.size() >= 2 && v.front() == '"' && v.back() == '"') {
        v = v.substr(1, v.size() - 2);
    }
    return v;
}

bool insertRow(
    const std::string& databaseName,
    const std::string& tableName,
    const std::string& values,
    std::string& error
) {
    if (databaseName.empty()) {
        error = "No database selected";
        return false;
    }

    std::string basePath = "..\\databases\\" + databaseName + "\\";

    if (!directoryExists(basePath)) {
        error = "Database does not exist";
        return false;
    }

    std::string tblPath  = basePath + tableName + ".tbl";
    std::string metaPath = basePath + tableName + ".meta";

    if (!fileExists(metaPath) || !fileExists(tblPath)) {
        error = "Table does not exist";
        return false;
    }

    // Parse VALUES (comma separated) - values string is already without outer parens
    std::string row;
    std::string temp;
    std::string inside = values;

    for (size_t i = 0; i < inside.size(); i++) {
        if (inside[i] == ',') {
            row += cleanValue(temp) + "|";
            temp.clear();
        } else {
            temp += inside[i];
        }
    }
    row += cleanValue(temp);

    // Append row to table
    std::ofstream tblFile(tblPath, std::ios::app);
    if (!tblFile) {
        error = "Failed to write to table";
        return false;
    }

    tblFile << row << "\n";
    tblFile.close();

    return true;
}