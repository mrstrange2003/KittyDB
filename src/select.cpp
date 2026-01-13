// select.cpp

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <set>
#include <algorithm>

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

// Comparator for sorting rows
struct RowComparator {
    int colIdx;
    bool ascending;
    std::string colType;

    bool operator()(const std::vector<std::string>& a, 
                   const std::vector<std::string>& b) const {
        if (colIdx < 0 || colIdx >= (int)a.size() || colIdx >= (int)b.size())
            return false;

        const std::string& aVal = a[colIdx];
        const std::string& bVal = b[colIdx];

        int cmp = 0;

        if (colType == "INT") {
            try {
                int aInt = std::stoi(aVal);
                int bInt = std::stoi(bVal);
                cmp = (aInt < bInt) ? -1 : (aInt > bInt) ? 1 : 0;
            } catch (...) {
                cmp = aVal.compare(bVal);
            }
        } else if (colType == "FLOAT") {
            try {
                double aFloat = std::stod(aVal);
                double bFloat = std::stod(bVal);
                cmp = (aFloat < bFloat) ? -1 : (aFloat > bFloat) ? 1 : 0;
            } catch (...) {
                cmp = aVal.compare(bVal);
            }
        } else {
            cmp = aVal.compare(bVal);
        }

        return ascending ? (cmp < 0) : (cmp > 0);
    }
};

bool selectColumns(
    const std::string& databaseName,
    const std::string& tableName,
    const std::vector<std::string>& selectedColumns,
    bool hasWhere,
    const WhereCondition& where,
    bool distinct,
    bool hasOrderBy,
    const OrderByClause& orderBy,
    bool hasLimit,
    int limitCount,
    int offsetCount,
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

    //  parse schema
    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error)) {
        return false;
    }

    //  resolve column indexes
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

    //  Find ORDER BY column index
    int orderByColIdx = -1;
    std::string orderByColType = "TEXT";
    if (hasOrderBy) {
        for (size_t i = 0; i < columns.size(); i++) {
            if (columns[i].name == orderBy.column) {
                orderByColIdx = (int)i;
                orderByColType = columns[i].type;
                break;
            }
        }
        if (orderByColIdx == -1) {
            error = "Unknown column '" + orderBy.column + "' in ORDER BY";
            return false;
        }
    }

    //  print header
    for (size_t i = 0; i < colIndexes.size(); i++) {
        std::cout << columns[colIndexes[i]].name;
        if (i + 1 < colIndexes.size()) std::cout << " | ";
    }
    std::cout << "\n";

    //  open table
    std::ifstream tblFile(tblPath);
    if (!tblFile) {
        error = "Failed to open table data";
        return false;
    }

    //  read rows and apply WHERE filter
    std::vector<std::vector<std::string>> filteredRows;
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

        //  APPLY WHERE FILTER 
        if (hasWhere) {
            if (!evaluateWhere(columns, fields, where)) {
                continue; // skip this row
            }
        }

        filteredRows.push_back(fields);
    }

    tblFile.close();

    //  Apply ORDER BY
    if (hasOrderBy) {
        RowComparator comp{orderByColIdx, orderBy.ascending, orderByColType};
        std::sort(filteredRows.begin(), filteredRows.end(), comp);
    }

    //  Apply DISTINCT
    if (distinct) {
        std::set<std::vector<std::string>> uniqueRows;
        std::vector<std::vector<std::string>> tempRows;

        for (const auto& row : filteredRows) {
            // Create a row with only selected columns
            std::vector<std::string> selectedRow;
            for (int idx : colIndexes) {
                if (idx < (int)row.size()) {
                    selectedRow.push_back(row[idx]);
                }
            }

            if (uniqueRows.find(selectedRow) == uniqueRows.end()) {
                uniqueRows.insert(selectedRow);
                tempRows.push_back(row);
            }
        }

        filteredRows = tempRows;
    }

    //  Apply LIMIT and OFFSET
    std::vector<std::vector<std::string>> finalRows;

    int startIdx = offsetCount;
    int endIdx = (int)filteredRows.size();

    if (hasLimit) {
        endIdx = std::min((int)filteredRows.size(), startIdx + limitCount);
    }

    for (int i = startIdx; i < endIdx; i++) {
        if (i >= 0) {
            finalRows.push_back(filteredRows[i]);
        }
    }

    //  print selected columns from final rows
    for (const auto& row : finalRows) {
        for (size_t i = 0; i < colIndexes.size(); i++) {
            int idx = colIndexes[i];
            if (idx < (int)row.size())
                std::cout << row[idx];
            if (i + 1 < colIndexes.size()) std::cout << " | ";
        }
        std::cout << "\n";
    }

    return true;
}