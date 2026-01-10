#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>

#include "select.h"
#include "schema.h"
#include "where.h"

// helpers
static bool directoryExists(const std::string& path){
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

static bool fileExists(const std::string& path){
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}

bool selectColumns(
    const std::string& databaseName,
    const std::string& tableName,
    const std::vector<std::string>& selectedColumns,
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

    // 1️⃣ parse schema
    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error)) {
        return false;
    }

    // 2️⃣ resolve column indexes
    std::vector<int> colIndexes;

    if (selectedColumns.empty()) {
        // SELECT *
        for (size_t i = 0; i < columns.size(); i++)
            colIndexes.push_back((int)i);
    } else {
        for (const auto& name : selectedColumns) {
            bool found = false;
            for (size_t i = 0; i < columns.size(); i++) {
                if (columns[i].name == name) {
                    colIndexes.push_back((int)i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                error = "Unknown column '" + name + "'";
                return false;
            }
        }
    }

    // 3️⃣ print header
    for (size_t i = 0; i < colIndexes.size(); i++) {
        std::cout << columns[colIndexes[i]].name;
        if (i + 1 < colIndexes.size()) std::cout << " | ";
    }
    std::cout << "\n";

    // 4️⃣ open table
    std::ifstream tblFile(tblPath);
    if (!tblFile) {
        error = "Failed to open table data";
        return false;
    }

    // 5️⃣ read rows
    std::string line;
    while (std::getline(tblFile, line)) {

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

        // ⭐ APPLY WHERE FILTER ⭐
        if (hasWhere) {
            if (!evaluateWhere(columns,
                               fields,
                               where.column,
                               where.op,
                               where.value)) {
                continue; // skip this row
            }
        }

        // 6️⃣ print selected columns
        for (size_t i = 0; i < colIndexes.size(); i++) {
            int idx = colIndexes[i];
            if (idx < (int)fields.size())
                std::cout << fields[idx];
            if (i + 1 < colIndexes.size()) std::cout << " | ";
        }
        std::cout << "\n";
    }

    tblFile.close();
    return true;
}
