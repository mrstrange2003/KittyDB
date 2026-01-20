/*
// select.cpp

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <set>
#include <algorithm>
#include <iomanip>

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

void printPrettyTable(
    const std::vector<std::string>& headers,
    const std::vector<std::vector<std::string>>& rows
);

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

    // DISTINCT (after ORDER BY, before LIMIT)
    if (distinct)
    {
        std::set<std::vector<std::string>> seen;
        std::vector<std::vector<std::string>> uniqueRows;

        for (const auto &row : filteredRows)
        {
            std::vector<std::string> key;
            for (int idx : colIndexes)
            {
                if (idx < (int)row.size())
                    key.push_back(row[idx]);
                else
                    key.push_back("");
            }

            if (seen.insert(key).second)
            {
                uniqueRows.push_back(row);
            }
        }

        filteredRows = uniqueRows;
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

    // LIMIT / OFFSET  <-- ADD HERE
    int startIdx = offsetCount;
    int endIdx = (int)filteredRows.size();

    if (hasLimit)
    {
        endIdx = std::min(endIdx, startIdx + limitCount);
    }

    // Pretty table printing

    // collect rows to print
    std::vector<std::vector<std::string>> outputRows;
    for (int i = startIdx; i < endIdx; i++)
    {
        if (i < 0)
            continue;
        std::vector<std::string> row;
        for (int idx : colIndexes)
        {
            if (idx < (int)filteredRows[i].size())
                row.push_back(filteredRows[i][idx]);
            else
                row.push_back("");
        }
        outputRows.push_back(row);
    }

    // compute column widths
    std::vector<size_t> colWidths(colIndexes.size(), 0);

    // header widths
    for (size_t i = 0; i < colIndexes.size(); i++)
    {
        colWidths[i] = columns[colIndexes[i]].name.size();
    }

    // data widths
    for (const auto &row : outputRows)
    {
        for (size_t i = 0; i < row.size(); i++)
        {
            colWidths[i] = std::max(colWidths[i], row[i].size());
        }
    }

    // helper to print border
    auto printBorder = [&]()
    {
        std::cout << "+";
        for (size_t w : colWidths)
        {
            std::cout << std::string(w + 2, '-') << "+";
        }
        std::cout << "\n";
    };

    // top border
    printBorder();

    // header row
    std::cout << "|";
    for (size_t i = 0; i < colIndexes.size(); i++)
    {
        std::cout << " "
                  << std::left << std::setw(colWidths[i])
                  << columns[colIndexes[i]].name
                  << " |";
    }
    std::cout << "\n";

    // header separator
    printBorder();

    // data rows
    for (const auto &row : outputRows)
    {
        std::cout << "|";
        for (size_t i = 0; i < row.size(); i++)
        {
            std::cout << " "
                      << std::left << std::setw(colWidths[i])
                      << row[i]
                      << " |";
        }
        std::cout << "\n";
    }

    // bottom border
    printBorder();

    return true;
}
*/

// select.cpp

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <set>
#include <algorithm>
#include <iomanip>

#include "select.h"
#include "schema.h"
#include "where.h"

/* -------------------------------------------------
   Helpers
------------------------------------------------- */

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

/* -------------------------------------------------
   Pretty table printer (USED BY SELECT)
------------------------------------------------- */
static void printPrettyTable(
    const std::vector<std::string>& headers,
    const std::vector<std::vector<std::string>>& rows)
{
    size_t cols = headers.size();
    std::vector<size_t> widths(cols, 0);

    for (size_t i = 0; i < cols; i++)
        widths[i] = headers[i].size();

    for (const auto& row : rows)
        for (size_t i = 0; i < cols; i++)
            widths[i] = std::max(widths[i], row[i].size());

    auto border = [&]() {
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

    for (const auto& row : rows)
    {
        std::cout << "|";
        for (size_t i = 0; i < cols; i++)
            std::cout << " " << std::left << std::setw(widths[i])
                      << row[i] << " |";
        std::cout << "\n";
    }

    border();
}

/* -------------------------------------------------
   ORDER BY comparator
------------------------------------------------- */
struct RowComparator
{
    int colIdx;
    bool ascending;
    std::string colType;

    bool operator()(const std::vector<std::string>& a,
                    const std::vector<std::string>& b) const
    {
        if (colIdx < 0 || colIdx >= (int)a.size() || colIdx >= (int)b.size())
            return false;

        const std::string& av = a[colIdx];
        const std::string& bv = b[colIdx];

        int cmp = 0;

        try
        {
            if (colType == "INT")
                cmp = std::stoi(av) - std::stoi(bv);
            else if (colType == "FLOAT")
                cmp = (std::stod(av) < std::stod(bv)) ? -1 :
                      (std::stod(av) > std::stod(bv)) ? 1 : 0;
            else
                cmp = av.compare(bv);
        }
        catch (...)
        {
            cmp = av.compare(bv);
        }

        return ascending ? (cmp < 0) : (cmp > 0);
    }
};

/* -------------------------------------------------
   SELECT implementation
------------------------------------------------- */
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
    std::string tblPath  = basePath + tableName + ".tbl";
    std::string metaPath = basePath + tableName + ".meta";

    if (!directoryExists(basePath) || !fileExists(tblPath) || !fileExists(metaPath))
    {
        error = "Table does not exist";
        return false;
    }

    /* ---------- schema ---------- */
    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error))
        return false;

    /* ---------- resolve columns ---------- */
    std::vector<int> colIdxs;

    if (selectedColumns.empty())
    {
        for (size_t i = 0; i < columns.size(); i++)
            if (columns[i].name != "__id")
                colIdxs.push_back((int)i);
    }
    else
    {
        for (const auto& name : selectedColumns)
        {
            bool found = false;
            for (size_t i = 0; i < columns.size(); i++)
            {
                if (columns[i].name == name)
                {
                    colIdxs.push_back((int)i);
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

    /* ---------- read rows ---------- */
    std::ifstream in(tblPath);
    std::vector<std::vector<std::string>> rows;
    std::string line;

    while (std::getline(in, line))
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
            else temp += c;
        }
        fields.push_back(temp);

        if (hasWhere && !evaluateWhere(columns, fields, where))
            continue;

        rows.push_back(fields);
    }

    /* ---------- ORDER BY ---------- */
    if (hasOrderBy && !hasAggregates)
    {
        int idx = -1;
        std::string type;

        for (size_t i = 0; i < columns.size(); i++)
            if (columns[i].name == orderBy.column)
            {
                idx = (int)i;
                type = columns[i].type;
                break;
            }

        if (idx == -1)
        {
            error = "Unknown column in ORDER BY";
            return false;
        }

        std::sort(rows.begin(), rows.end(),
                  RowComparator{idx, orderBy.ascending, type});
    }

    /* ---------- DISTINCT ---------- */
    if (distinct)
    {
        std::set<std::vector<std::string>> seen;
        std::vector<std::vector<std::string>> unique;

        for (const auto& r : rows)
        {
            std::vector<std::string> key;
            for (int i : colIdxs)
                key.push_back(i < (int)r.size() ? r[i] : "");

            if (seen.insert(key).second)
                unique.push_back(r);
        }
        rows = unique;
    }

    /* ---------- AGGREGATES ---------- */
    if (hasAggregates)
    {
        std::vector<std::string> results;
        evaluateAggregate(columns, rows, aggregates, results);

        for (size_t i = 0; i < aggregates.size(); i++)
        {
            if (i) std::cout << " | ";
            std::cout << results[i];
        }
        std::cout << "\n";
        return true;
    }

    /* ---------- LIMIT / OFFSET ---------- */
    int start = std::max(0, offsetCount);
    int end   = hasLimit ? std::min((int)rows.size(), start + limitCount)
                         : (int)rows.size();

    /* ---------- build output ---------- */
    std::vector<std::string> headers;
    for (int i : colIdxs)
        headers.push_back(columns[i].name);

    std::vector<std::vector<std::string>> outRows;

    for (int i = start; i < end; i++)
    {
        std::vector<std::string> r;
        for (int c : colIdxs)
            r.push_back(c < (int)rows[i].size() ? rows[i][c] : "");
        outRows.push_back(r);
    }

    printPrettyTable(headers, outRows);
    return true;
}
