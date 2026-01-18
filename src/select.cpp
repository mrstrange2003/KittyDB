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

// Comparator for sorting rows (STRICT WEAK ORDERING SAFE)
struct RowComparator
{
    int colIdx;
    bool ascending;
    std::string colType;

    bool operator()(const std::vector<std::string> &a,
                    const std::vector<std::string> &b) const
    {

        bool aValid = colIdx >= 0 && colIdx < (int)a.size();
        bool bValid = colIdx >= 0 && colIdx < (int)b.size();

        // Handle missing values deterministically
        if (!aValid && !bValid)
            return false; // equal
        if (!aValid)
            return false; // a > b
        if (!bValid)
            return true; // a < b

        const std::string &aVal = a[colIdx];
        const std::string &bVal = b[colIdx];

        int cmp = 0;

        if (colType == "INT")
        {
            try
            {
                int ai = std::stoi(aVal);
                int bi = std::stoi(bVal);
                cmp = (ai < bi) ? -1 : (ai > bi) ? 1
                                                 : 0;
            }
            catch (...)
            {
                cmp = aVal.compare(bVal);
            }
        }
        else if (colType == "FLOAT")
        {
            try
            {
                double af = std::stod(aVal);
                double bf = std::stod(bVal);
                cmp = (af < bf) ? -1 : (af > bf) ? 1
                                                 : 0;
            }
            catch (...)
            {
                cmp = aVal.compare(bVal);
            }
        }
        else
        {
            cmp = aVal.compare(bVal);
        }

        return ascending ? (cmp < 0) : (cmp > 0);
    }
};

bool selectColumns(
    const std::string &databaseName,
    const std::string &tableName,
    const std::vector<std::string> &selectedColumns,
    bool hasWhere,
    const WhereCondition &where,
    bool distinct,
    bool hasOrderBy,
    const OrderByClause &orderBy,
    bool hasLimit,
    int limitCount,
    int offsetCount,
    bool hasAggregates,
    const std::vector<AggregateFunction> &aggregates,
    std::string &error)
{
    if (databaseName.empty())
    {
        error = "No database selected";
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

    // resolve selected column indexes
    std::vector<int> colIndexes;

    if (selectedColumns.empty())
    {
        for (size_t i = 0; i < columns.size(); i++)
        {
            if (columns[i].name == "__id")
                continue;
            colIndexes.push_back((int)i);
        }
    }
    else
    {
        for (const auto &name : selectedColumns)
        {
            bool found = false;
            for (size_t i = 0; i < columns.size(); i++)
            {
                if (columns[i].name == name)
                {
                    colIndexes.push_back((int)i);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                error = "Unknown column '" + name + "'";
                return false;
            }
        }
    }

    // resolve ORDER BY column
    int orderByColIdx = -1;
    std::string orderByColType = "TEXT";

    if (hasOrderBy)
    {
        for (size_t i = 0; i < columns.size(); i++)
        {
            if (columns[i].name == orderBy.column)
            {
                orderByColIdx = (int)i;
                orderByColType = columns[i].type;
                break;
            }
        }
        if (orderByColIdx == -1)
        {
            error = "Unknown column '" + orderBy.column + "' in ORDER BY";
            return false;
        }
    }

    // open table
    std::ifstream tblFile(tblPath);
    if (!tblFile)
    {
        error = "Failed to open table data";
        return false;
    }

    // read rows + WHERE filtering
    std::vector<std::vector<std::string>> filteredRows;
    std::string line;

    while (std::getline(tblFile, line))
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

        if (hasWhere && !evaluateWhere(columns, fields, where))
            continue;

        filteredRows.push_back(fields);
    }

    tblFile.close();

    // ORDER BY (non-aggregate only)
    if (hasOrderBy && !hasAggregates)
    {
        RowComparator comp{orderByColIdx, orderBy.ascending, orderByColType};
        std::sort(filteredRows.begin(), filteredRows.end(), comp);
    }

    // AGGREGATE path
    if (hasAggregates)
    {
        for (size_t i = 0; i < aggregates.size(); i++)
        {
            std::string label;
            switch (aggregates[i].type)
            {
            case AggregateType::COUNT:
                label = "COUNT(" + aggregates[i].column + ")";
                break;
            case AggregateType::SUM:
                label = "SUM(" + aggregates[i].column + ")";
                break;
            case AggregateType::AVG:
                label = "AVG(" + aggregates[i].column + ")";
                break;
            case AggregateType::MIN:
                label = "MIN(" + aggregates[i].column + ")";
                break;
            case AggregateType::MAX:
                label = "MAX(" + aggregates[i].column + ")";
                break;
            default:
                label = "?";
            }
            std::cout << label;
            if (i + 1 < aggregates.size())
                std::cout << " | ";
        }
        std::cout << "\n";

        std::vector<std::string> aggResults;
        evaluateAggregate(columns, filteredRows, aggregates, aggResults);

        for (size_t i = 0; i < aggResults.size(); i++)
        {
            std::cout << aggResults[i];
            if (i + 1 < aggResults.size())
                std::cout << " | ";
        }
        std::cout << "\n";

        return true;
    }

    // print header (regular SELECT)
    for (size_t i = 0; i < colIndexes.size(); i++)
    {
        std::cout << columns[colIndexes[i]].name;
        if (i + 1 < colIndexes.size())
            std::cout << " | ";
    }
    std::cout << "\n";

    // DISTINCT
    if (distinct)
    {
        std::set<std::vector<std::string>> seen;
        std::vector<std::vector<std::string>> uniqueRows;

        for (const auto &row : filteredRows)
        {
            std::vector<std::string> key;
            for (int idx : colIndexes)
                if (idx < (int)row.size())
                    key.push_back(row[idx]);

            if (seen.insert(key).second)
                uniqueRows.push_back(row);
        }

        filteredRows = uniqueRows;
    }

    // LIMIT / OFFSET
    int startIdx = offsetCount;
    int endIdx = (int)filteredRows.size();

    if (hasLimit)
        endIdx = std::min(endIdx, startIdx + limitCount);

    for (int i = startIdx; i < endIdx; i++)
    {
        if (i < 0)
            continue;
        const auto &row = filteredRows[i];

        for (size_t j = 0; j < colIndexes.size(); j++)
        {
            int idx = colIndexes[j];
            if (idx < (int)row.size())
                std::cout << row[idx];
            if (j + 1 < colIndexes.size())
                std::cout << " | ";
        }
        std::cout << "\n";
    }

    return true;
}
