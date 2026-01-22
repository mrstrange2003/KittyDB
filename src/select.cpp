// select.cpp

#include "select.h"
#include "schema.h"
#include "where.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <set>
#include <algorithm>
#include <iomanip>

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

//   Pretty table printer (USED BY SELECT)
static void printPrettyTable(
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

// ORDER BY comparator
struct RowComparator
{
    int colIdx;
    bool ascending;
    std::string colType;

    bool operator()(const std::vector<std::string> &a,
                    const std::vector<std::string> &b) const
    {
        if (colIdx < 0 || colIdx >= (int)a.size() || colIdx >= (int)b.size())
            return false;

        const std::string &av = a[colIdx];
        const std::string &bv = b[colIdx];

        int cmp = 0;

        try
        {
            if (colType == "INT")
                cmp = std::stoi(av) - std::stoi(bv);
            else if (colType == "FLOAT")
                cmp = (std::stod(av) < std::stod(bv)) ? -1 : (std::stod(av) > std::stod(bv)) ? 1
                                                                                             : 0;
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

// SELECT implementation
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

    if (!directoryExists(basePath) || !fileExists(tblPath) || !fileExists(metaPath))
    {
        error = "Table does not exist";
        return false;
    }

    // schema
    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error))
        return false;

    // resolve columns
    std::vector<int> colIdxs;

    if (selectedColumns.empty())
    {
        for (size_t i = 0; i < columns.size(); i++)
            if (columns[i].name != "__id")
                colIdxs.push_back((int)i);
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

    // read rows
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
            else
                temp += c;
        }
        fields.push_back(temp);

        if (hasWhere && !evaluateWhere(columns, fields, where))
            continue;

        rows.push_back(fields);
    }

    // ORDER BY
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

    // DISTINCT
    if (distinct)
    {
        std::set<std::vector<std::string>> seen;
        std::vector<std::vector<std::string>> unique;

        for (const auto &r : rows)
        {
            std::vector<std::string> key;
            for (int i : colIdxs)
                key.push_back(i < (int)r.size() ? r[i] : "");

            if (seen.insert(key).second)
                unique.push_back(r);
        }
        rows = unique;
    }

    // AGGREGATES
    if (hasAggregates)
    {
        std::vector<std::string> results;
        evaluateAggregate(columns, rows, aggregates, results);

        for (size_t i = 0; i < aggregates.size(); i++)
        {
            if (i)
                std::cout << " | ";
            std::cout << results[i];
        }
        std::cout << "\n";
        return true;
    }

    // LIMIT / OFFSET
    int start = offsetCount; // Start from offset position
    if (start < 0)
        start = 0;
    if (start >= (int)rows.size())
        start = (int)rows.size();

    int end = (int)rows.size(); // Default: go to end
    if (hasLimit)
    {
        end = start + limitCount; // Start + how many to return
        if (end > (int)rows.size())
            end = (int)rows.size();
    }

    // build output
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
