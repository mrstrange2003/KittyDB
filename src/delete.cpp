// delete.cpp

#include "delete.h"
#include "schema.h"
#include "where.h"

#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>

// helpers
static bool directoryExists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

static bool fileExists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

bool deleteWhere(
    const std::string& databaseName,
    const std::string& tableName,
    bool hasWhere,
    const WhereCondition& where,
    std::string& error
) {
    if (databaseName.empty()) {
        error = "No database selected";
        return false;
    }

    std::string basePath = "..\\databases\\" + databaseName + "\\";
    std::string tblPath  = basePath + tableName + ".tbl";
    std::string metaPath = basePath + tableName + ".meta";

    if (!directoryExists(basePath)) {
        error = "Database does not exist";
        return false;
    }

    if (!fileExists(tblPath) || !fileExists(metaPath)) {
        error = "Table does not exist";
        return false;
    }

    // parse schema
    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error)) {
        return false;
    }

    // read all rows
    std::ifstream in(tblPath);
    if (!in) {
        error = "Failed to open table";
        return false;
    }

    std::vector<std::string> keptRows;
    std::string line;

    while (std::getline(in, line)) {
        // split row into fields
        std::vector<std::string> fields;
        std::string temp;

        for (char c : line) {
            if (c == '|') {
                fields.push_back(temp);
                temp.clear();
            } else {
                temp += c;
            }
        }
        fields.push_back(temp);

        // apply WHERE
        if (hasWhere) {
            if (evaluateWhere(columns, fields, where)) {
                // row matches WHERE → delete it
                continue;
            }
        }

        // keep row
        keptRows.push_back(line);
    }
    in.close();

    // rewrite table file
    std::ofstream out(tblPath, std::ios::trunc);
    if (!out) {
        error = "Failed to rewrite table";
        return false;
    }

    for (const auto& r : keptRows) {
        out << r << "\n";
    }
    out.close();

    return true;
}