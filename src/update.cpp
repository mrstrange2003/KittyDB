#include "update.h"
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

// split "col=value"
static bool parseSetClause(
    const std::string& setClause,
    std::string& column,
    std::string& value
) {
    size_t pos = setClause.find('=');
    if (pos == std::string::npos)
        return false;

    column = setClause.substr(0, pos);
    value  = setClause.substr(pos + 1);

    // trim spaces
    auto trim = [](std::string& s) {
        while (!s.empty() && s.front() == ' ') s.erase(s.begin());
        while (!s.empty() && s.back() == ' ') s.pop_back();
    };

    trim(column);
    trim(value);

    // remove quotes
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }

    return true;
}

bool updateWhere(
    const std::string& databaseName,
    const std::string& tableName,
    const std::string& setClause,
    bool hasWhere,
    const WhereCondition& where,
    std::string& error
) {
    if (databaseName.empty()) {
        error = "No database selected";
        return false;
    }

    if (!hasWhere) {
        error = "UPDATE without WHERE is not allowed";
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

    // parse SET clause
    std::string setColumn, setValue;
    if (!parseSetClause(setClause, setColumn, setValue)) {
        error = "Invalid SET clause";
        return false;
    }

    int setIdx = -1;
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == setColumn) {
            setIdx = (int)i;
            break;
        }
    }

    if (setIdx == -1) {
        error = "Unknown column '" + setColumn + "'";
        return false;
    }

    // read rows
    std::ifstream in(tblPath);
    if (!in) {
        error = "Failed to open table";
        return false;
    }

    std::vector<std::string> updatedRows;
    std::string line;

    while (std::getline(in, line)) {
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
        if (evaluateWhere(columns,
                          fields,
                          where.column,
                          where.op,
                          where.value)) {
            // update value
            if (setIdx < (int)fields.size()) {
                fields[setIdx] = setValue;
            }
        }

        // rebuild row
        std::string newRow;
        for (size_t i = 0; i < fields.size(); i++) {
            newRow += fields[i];
            if (i + 1 < fields.size())
                newRow += "|";
        }

        updatedRows.push_back(newRow);
    }
    in.close();

    // rewrite table
    std::ofstream out(tblPath, std::ios::trunc);
    if (!out) {
        error = "Failed to rewrite table";
        return false;
    }

    for (const auto& r : updatedRows) {
        out << r << "\n";
    }
    out.close();

    return true;
}
