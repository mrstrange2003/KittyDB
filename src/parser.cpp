// parser.cpp

#include "parser.h"
#include "where.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

static string trim(string s)
{
    while (!s.empty() && s.front() == ' ')
        s.erase(s.begin());
    while (!s.empty() && s.back() == ' ')
        s.pop_back();
    return s;
}

static std::string stripQuotes(std::string v)
{
    if (v.size() >= 2 && v.front() == '"' && v.back() == '"')
        return v.substr(1, v.size() - 2);
    return v;
}

// Parse aggregate functions from column specification
static void parseAggregates(const std::string &colPart, std::vector<std::string> &columns, std::vector<AggregateFunction> &aggregates)
{
    std::stringstream ss(colPart);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        token = trim(token);
        std::string upper = token;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

        if (upper.rfind("COUNT(", 0) == 0)
        {
            size_t closePos = upper.find(')');
            if (closePos != std::string::npos)
            {
                std::string colName = trim(token.substr(6, closePos - 6));
                AggregateFunction agg;
                agg.type = AggregateType::COUNT;
                agg.column = colName;
                aggregates.push_back(agg);
            }
        }
        else if (upper.rfind("SUM(", 0) == 0)
        {
            size_t closePos = upper.find(')');
            if (closePos != std::string::npos)
            {
                std::string colName = trim(token.substr(4, closePos - 4));
                AggregateFunction agg;
                agg.type = AggregateType::SUM;
                agg.column = colName;
                aggregates.push_back(agg);
            }
        }
        else if (upper.rfind("AVG(", 0) == 0)
        {
            size_t closePos = upper.find(')');
            if (closePos != std::string::npos)
            {
                std::string colName = trim(token.substr(4, closePos - 4));
                AggregateFunction agg;
                agg.type = AggregateType::AVG;
                agg.column = colName;
                aggregates.push_back(agg);
            }
        }
        else if (upper.rfind("MIN(", 0) == 0)
        {
            size_t closePos = upper.find(')');
            if (closePos != std::string::npos)
            {
                std::string colName = trim(token.substr(4, closePos - 4));
                AggregateFunction agg;
                agg.type = AggregateType::MIN;
                agg.column = colName;
                aggregates.push_back(agg);
            }
        }
        else if (upper.rfind("MAX(", 0) == 0)
        {
            size_t closePos = upper.find(')');
            if (closePos != std::string::npos)
            {
                std::string colName = trim(token.substr(4, closePos - 4));
                AggregateFunction agg;
                agg.type = AggregateType::MAX;
                agg.column = colName;
                aggregates.push_back(agg);
            }
        }
        else
        {
            columns.push_back(token);
        }
    }
}

// Parse complex WHERE clause with AND/OR
static WhereCondition *parseWhereClause(const std::string &whereStr)
{
    WhereCondition *result = new WhereCondition();

    std::string upper = whereStr;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    std::vector<std::string> parts;
    std::vector<std::string> ops;

    size_t lastPos = 0;
    size_t pos = 0;

    while (pos < upper.size())
    {
        bool isBetweenAnd = false;

        if (pos + 4 <= upper.size() && upper.substr(pos, 4) == " AND")
        {
            std::string before = upper.substr(lastPos, pos - lastPos);
            if (before.find("BETWEEN") != std::string::npos)
            {
                isBetweenAnd = true;
            }
        }

        if (!isBetweenAnd && pos + 4 <= upper.size() && upper.substr(pos, 4) == " AND")
        {
            parts.push_back(whereStr.substr(lastPos, pos - lastPos));
            ops.push_back("AND");
            lastPos = pos + 5;
            pos = lastPos;
        }
        else if (pos + 3 <= upper.size() && upper.substr(pos, 3) == " OR")
        {
            parts.push_back(whereStr.substr(lastPos, pos - lastPos));
            ops.push_back("OR");
            lastPos = pos + 4;
            pos = lastPos;
        }
        else
        {
            pos++;
        }
    }
    parts.push_back(whereStr.substr(lastPos));

    for (const auto &part : parts)
    {
        std::string cond = trim(part);
        SimpleCondition sc;

        std::string upperCond = cond;
        std::transform(upperCond.begin(), upperCond.end(), upperCond.begin(), ::toupper);

        if (upperCond.find(" IS NOT NULL") != std::string::npos)
        {
            sc.column = trim(cond.substr(0, upperCond.find(" IS")));
            sc.isNotNullCheck = true;
            result->conditions.push_back(sc);
            continue;
        }

        if (upperCond.find(" IS NULL") != std::string::npos)
        {
            sc.column = trim(cond.substr(0, upperCond.find(" IS")));
            sc.isNullCheck = true;
            result->conditions.push_back(sc);
            continue;
        }

        const std::vector<std::string> opList = {"BETWEEN", "LIKE", "<=", ">=", "!=", "IN", "=", "<", ">"};

        bool found = false;
        for (const auto &op : opList)
        {
            std::string upperCond = cond;
            std::transform(upperCond.begin(), upperCond.end(), upperCond.begin(), ::toupper);

            size_t opPos = upperCond.find(op);
            if (opPos != std::string::npos)
            {
                sc.column = trim(cond.substr(0, opPos));
                sc.op = op;

                std::string remainder = trim(cond.substr(opPos + op.size()));

                if (op == "BETWEEN")
                {
                    std::string upperRem = remainder;
                    std::transform(upperRem.begin(), upperRem.end(), upperRem.begin(), ::toupper);
                    size_t andPos = upperRem.find(" AND");

                    if (andPos != std::string::npos)
                    {
                        sc.value = stripQuotes(trim(remainder.substr(0, andPos)));
                        sc.value2 = stripQuotes(trim(remainder.substr(andPos + 5)));
                        found = true;
                        break;
                    }
                }
                else if (op == "IN")
                {
                    if (!remainder.empty() && remainder.front() == '(' && remainder.back() == ')')
                    {
                        sc.value = remainder.substr(1, remainder.size() - 2);
                        found = true;
                        break;
                    }
                }
                else
                {
                    sc.value = stripQuotes(trim(remainder));
                    found = true;
                    break;
                }
            }
        }

        if (found)
        {
            result->conditions.push_back(sc);
        }
    }

    result->logicalOps = ops;
    return result;
}

ParsedCommand parseCommand(const string &command)
{
    std::string cleaned = command;

    while (!cleaned.empty() && isspace(cleaned.front()))
        cleaned.erase(cleaned.begin());
    while (!cleaned.empty() && isspace(cleaned.back()))
        cleaned.pop_back();
    while (!cleaned.empty() && cleaned.back() == ';')
        cleaned.pop_back();

    ParsedCommand result;
    result.type = CommandType::UNKNOWN;

    string cmd = cleaned;
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    // CREATE DATABASE
    if (cmd.rfind("CREATE DATABASE", 0) == 0)
    {
        result.type = CommandType::CREATE_DATABASE;
        // Find "DATABASE" in UPPERCASE cmd, use that position in cleaned
        size_t dbPos = cmd.find("DATABASE");
        if (dbPos != std::string::npos)
        {
            size_t namePos = dbPos + 8; // "DATABASE" is 8 chars
            while (namePos < cleaned.size() && isspace(cleaned[namePos]))
                namePos++;
            result.databaseName = trim(cleaned.substr(namePos));
        }
        return result;
    }

    // USE DATABASE
    else if (cmd.rfind("USE", 0) == 0)
    {
        result.type = CommandType::USE_DATABASE;
        size_t namePos = 3; // "USE" is 3 chars
        while (namePos < cleaned.size() && isspace(cleaned[namePos]))
            namePos++;
        result.databaseName = trim(cleaned.substr(namePos));
        return result;
    }

    // CREATE TABLE
    else if (cmd.rfind("CREATE TABLE", 0) == 0)
    {
        result.type = CommandType::CREATE;
        size_t tablePos = cmd.find("TABLE");
        if (tablePos != std::string::npos)
        {
            tablePos += 5; // "TABLE" is 5 chars
            while (tablePos < cleaned.size() && isspace(cleaned[tablePos]))
                tablePos++;
            size_t parenPos = cleaned.find('(', tablePos);
            result.tableName = trim(cleaned.substr(tablePos, parenPos - tablePos));
            size_t closeParen = cleaned.find(')', parenPos);
            result.schema = cleaned.substr(parenPos + 1, closeParen - parenPos - 1);
        }
        return result;
    }

    // INSERT
    else if (cmd.rfind("INSERT INTO", 0) == 0)
    {
        result.type = CommandType::INSERT;
        size_t intoPos = cmd.find("INTO");
        if (intoPos != std::string::npos)
        {
            intoPos += 4; // "INTO" is 4 chars
            while (intoPos < cleaned.size() && isspace(cleaned[intoPos]))
                intoPos++;
            size_t valuesPos = cmd.find("VALUES");
            result.tableName = trim(cleaned.substr(intoPos, valuesPos - intoPos));
            size_t openParen = cleaned.find('(', valuesPos);
            size_t closeParen = cleaned.find(')', openParen);
            result.values = cleaned.substr(openParen + 1, closeParen - openParen - 1);
        }
        return result;
    }

    // DESCRIBE TABLE
    else if (cmd.rfind("DESCRIBE", 0) == 0)
    {
        result.type = CommandType::DESCRIBE;
        size_t namePos = 8; // "DESCRIBE" is 8 chars
        while (namePos < cleaned.size() && isspace(cleaned[namePos]))
            namePos++;
        result.tableName = trim(cleaned.substr(namePos));
        return result;
    }

    // SHOW TABLES
    else if (cmd == "SHOW TABLES")
    {
        result.type = CommandType::SHOW_TABLES;
        return result;
    }

    // TRUNCATE TABLE
    else if (cmd.rfind("TRUNCATE", 0) == 0)
    {
        result.type = CommandType::TRUNCATE;
        size_t namePos = 8; // "TRUNCATE" is 8 chars
        while (namePos < cleaned.size() && isspace(cleaned[namePos]))
            namePos++;
        result.tableName = trim(cleaned.substr(namePos));
        return result;
    }

    // SELECT
    else if (cmd.rfind("SELECT", 0) == 0)
    {
        result.type = CommandType::SELECT;

        size_t selectPos = 6; // "SELECT" is 6 chars
        while (selectPos < cleaned.size() && isspace(cleaned[selectPos]))
            selectPos++;

        std::string afterSelect = trim(cleaned.substr(selectPos));
        std::string afterSelectUpper = afterSelect;
        std::transform(afterSelectUpper.begin(), afterSelectUpper.end(),
                       afterSelectUpper.begin(), ::toupper);

        if (afterSelectUpper.rfind("DISTINCT", 0) == 0)
        {
            result.distinct = true;
            selectPos += 8; // "DISTINCT" is 8 chars
            while (selectPos < cleaned.size() && isspace(cleaned[selectPos]))
                selectPos++;
        }

        size_t fromPos = cmd.find("FROM");
        if (fromPos == std::string::npos)
            return result;

        std::string colPart = trim(cleaned.substr(selectPos, fromPos - selectPos));
        if (colPart != "*")
        {
            std::string upperColPart = colPart;
            std::transform(upperColPart.begin(), upperColPart.end(), upperColPart.begin(), ::toupper);

            if (upperColPart.find("COUNT(") != std::string::npos ||
                upperColPart.find("SUM(") != std::string::npos ||
                upperColPart.find("AVG(") != std::string::npos ||
                upperColPart.find("MIN(") != std::string::npos ||
                upperColPart.find("MAX(") != std::string::npos)
            {
                result.hasAggregates = true;
                parseAggregates(colPart, result.selectedColumns, result.aggregateFunctions);
            }
            else
            {
                std::stringstream ss(colPart);
                std::string col;
                while (std::getline(ss, col, ','))
                {
                    result.selectedColumns.push_back(trim(col));
                }
            }
        }

        size_t wherePos = cmd.find("WHERE");
        size_t orderPos = cmd.find("ORDER BY");
        size_t limitPos = cmd.find("LIMIT");

        size_t tableStart = fromPos + 4; // "FROM" is 4 chars
        while (tableStart < cleaned.size() && isspace(cleaned[tableStart]))
            tableStart++;
        size_t tableEnd = cleaned.size();

        if (wherePos != std::string::npos)
            tableEnd = std::min(tableEnd, wherePos);
        if (orderPos != std::string::npos)
            tableEnd = std::min(tableEnd, orderPos);
        if (limitPos != std::string::npos)
            tableEnd = std::min(tableEnd, limitPos);

        result.tableName = trim(cleaned.substr(tableStart, tableEnd - tableStart));

        // Parse WHERE
        if (wherePos != std::string::npos)
        {
            result.hasWhere = true;
            size_t whereEndPos = cleaned.size();
            if (orderPos != std::string::npos)
                whereEndPos = std::min(whereEndPos, orderPos);
            if (limitPos != std::string::npos)
                whereEndPos = std::min(whereEndPos, limitPos);

            size_t whereDataPos = wherePos + 5; // "WHERE" is 5 chars
            while (whereDataPos < cleaned.size() && isspace(cleaned[whereDataPos]))
                whereDataPos++;

            std::string whereClause = trim(cleaned.substr(whereDataPos, whereEndPos - whereDataPos));
            result.where = parseWhereClause(whereClause);
        }

        // Parse ORDER BY
        if (orderPos != std::string::npos)
        {
            result.hasOrderBy = true;
            size_t orderEndPos = cleaned.size();
            if (limitPos != std::string::npos)
                orderEndPos = std::min(orderEndPos, limitPos);

            size_t orderDataPos = orderPos + 8; // "ORDER BY" is 8 chars
            while (orderDataPos < cleaned.size() && isspace(cleaned[orderDataPos]))
                orderDataPos++;

            std::string orderClause = trim(cleaned.substr(orderDataPos, orderEndPos - orderDataPos));

            std::string orderUpper = orderClause;
            std::transform(orderUpper.begin(), orderUpper.end(), orderUpper.begin(), ::toupper);

            size_t descPos = orderUpper.find(" DESC");
            size_t ascPos = orderUpper.find(" ASC");

            if (descPos != std::string::npos)
            {
                result.orderBy.column = trim(orderClause.substr(0, descPos));
                result.orderBy.ascending = false;
            }
            else if (ascPos != std::string::npos)
            {
                result.orderBy.column = trim(orderClause.substr(0, ascPos));
                result.orderBy.ascending = true;
            }
            else
            {
                result.orderBy.column = trim(orderClause);
                result.orderBy.ascending = true;
            }
        }

        // Parse LIMIT
        if (limitPos != std::string::npos)
        {
            result.hasLimit = true;
            size_t limitDataPos = limitPos + 5; // "LIMIT" is 5 chars
            while (limitDataPos < cleaned.size() && isspace(cleaned[limitDataPos]))
                limitDataPos++;

            std::string limitClause = trim(cleaned.substr(limitDataPos));

            // FIX: Search for OFFSET in uppercase version
            std::string limitClauseUpper = limitClause;
            std::transform(limitClauseUpper.begin(), limitClauseUpper.end(),
                           limitClauseUpper.begin(), ::toupper);

            size_t offsetKeywordPos = limitClauseUpper.find("OFFSET");
            if (offsetKeywordPos != std::string::npos)
            {
                result.limitCount = std::stoi(trim(limitClause.substr(0, offsetKeywordPos)));
                result.offsetCount = std::stoi(trim(limitClause.substr(offsetKeywordPos + 6)));
            }
            else
            {
                result.limitCount = std::stoi(trim(limitClause));
                result.offsetCount = 0;
            }
        }
    }

    // DELETE
    else if (cmd.rfind("DELETE FROM", 0) == 0)
    {
        result.type = CommandType::DELETE_CMD;
        size_t fromPos = cmd.find("FROM");
        if (fromPos != std::string::npos)
        {
            fromPos += 4; // "FROM" is 4 chars
            while (fromPos < cleaned.size() && isspace(cleaned[fromPos]))
                fromPos++;
            size_t wherePos = cmd.find("WHERE");

            if (wherePos == std::string::npos)
            {
                result.tableName = trim(cleaned.substr(fromPos));
                result.hasWhere = false;
            }
            else
            {
                result.tableName = trim(cleaned.substr(fromPos, wherePos - fromPos));
                result.hasWhere = true;
                size_t whereDataPos = wherePos + 5; // "WHERE" is 5 chars
                while (whereDataPos < cleaned.size() && isspace(cleaned[whereDataPos]))
                    whereDataPos++;
                std::string cond = trim(cleaned.substr(whereDataPos));
                result.where = parseWhereClause(cond);
            }
        }
        return result;
    }

    // UPDATE
    else if (cmd.rfind("UPDATE", 0) == 0)
    {
        result.type = CommandType::UPDATE;
        size_t updatePos = 6; // "UPDATE" is 6 chars
        while (updatePos < cleaned.size() && isspace(cleaned[updatePos]))
            updatePos++;

        size_t setPos = cmd.find("SET", updatePos);
        size_t wherePos = cmd.find("WHERE");

        if (setPos != std::string::npos)
        {
            result.tableName = trim(cleaned.substr(updatePos, setPos - updatePos));

            size_t setDataPos = setPos + 3; // "SET" is 3 chars
            while (setDataPos < cleaned.size() && isspace(cleaned[setDataPos]))
                setDataPos++;

            result.setClause = trim(cleaned.substr(setDataPos, wherePos - setDataPos));

            if (wherePos != std::string::npos)
            {
                result.hasWhere = true;
                size_t whereDataPos = wherePos + 5; // "WHERE" is 5 chars
                while (whereDataPos < cleaned.size() && isspace(cleaned[whereDataPos]))
                    whereDataPos++;
                std::string cond = trim(cleaned.substr(whereDataPos));
                result.where = parseWhereClause(cond);
            }
        }
        return result;
    }

    return result;
}