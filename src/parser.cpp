#include "parser.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

// trim spaces
static string trim(string s)
{
    while (!s.empty() && s.front() == ' ')
        s.erase(s.begin());
    while (!s.empty() && s.back() == ' ')
        s.pop_back();
    return s;
}

// helper
static std::string stripQuotes(std::string v)
{
    if (v.size() >= 2 && v.front() == '"' && v.back() == '"')
        return v.substr(1, v.size() - 2);
    return v;
}

ParsedCommand parseCommand(const string &command)
{

    std::string cleaned = command;

    // trim leading/trailing spaces
    while (!cleaned.empty() && isspace(cleaned.front()))
        cleaned.erase(cleaned.begin());

    while (!cleaned.empty() && isspace(cleaned.back()))
        cleaned.pop_back();

    // remove trailing semicolon(s)
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

        size_t namePos = cmd.find("DATABASE") + 9;
        result.databaseName = trim(cleaned.substr(namePos));

        return result;
    }

    // USE DATABASE
    else if (cmd.rfind("USE", 0) == 0)
    {
        result.type = CommandType::USE_DATABASE;

        size_t namePos = 4; // after "USE "
        result.databaseName = trim(cleaned.substr(namePos));

        return result;
    }

    // CREATE TABLE
    else if (cmd.rfind("CREATE TABLE", 0) == 0)
    {
        result.type = CommandType::CREATE;

        size_t tablePos = cmd.find("TABLE") + 6;
        size_t parenPos = cleaned.find('(', tablePos);

        result.tableName = trim(cleaned.substr(tablePos, parenPos - tablePos));

        size_t closeParen = cleaned.find(')', parenPos);
        result.schema = cleaned.substr(parenPos + 1,
                                       closeParen - parenPos - 1);
    }

    // INSERT
    else if (cmd.rfind("INSERT INTO", 0) == 0)
    {
        result.type = CommandType::INSERT;

        size_t intoPos = cmd.find("INTO") + 5;
        size_t valuesPos = cmd.find("VALUES");

        result.tableName = trim(cleaned.substr(intoPos,
                                               valuesPos - intoPos));

        size_t openParen = cleaned.find('(', valuesPos);
        size_t closeParen = cleaned.find(')', openParen);

        result.values = cleaned.substr(openParen + 1,
                                       closeParen - openParen - 1);
    }

    // SELECT
    // SELECT
    else if (cmd.rfind("SELECT", 0) == 0)
    {
        result.type = CommandType::SELECT;

        size_t fromPos = cmd.find("FROM");
        if (fromPos == std::string::npos)
            return result;

        // columns
        std::string colPart = trim(cleaned.substr(6, fromPos - 6));
        if (colPart != "*")
        {
            std::stringstream ss(colPart);
            std::string col;
            while (std::getline(ss, col, ','))
            {
                result.selectedColumns.push_back(trim(col));
            }
        }

        // table + optional WHERE
        size_t wherePos = cmd.find("WHERE");
        if (wherePos == std::string::npos)
        {
            result.tableName = trim(cleaned.substr(fromPos + 5));
        }
        else
        {
            result.tableName = trim(cleaned.substr(fromPos + 5,
                                                   wherePos - (fromPos + 5)));

            result.hasWhere = true;

            std::string cond = trim(cleaned.substr(wherePos + 6));

            // detect operator
            const std::vector<std::string> ops = {"<=", ">=", "!=", "=", "<", ">"};
            for (const auto &op : ops)
            {
                size_t pos = cond.find(op);
                if (pos != std::string::npos)
                {
                    result.where.column = trim(cond.substr(0, pos));
                    result.where.op = op;
                    result.where.value = stripQuotes(
                        trim(cond.substr(pos + op.size())));
                    break;
                }
            }
        }
    }

    // DELETE
    else if (cmd.rfind("DELETE FROM", 0) == 0)
    {
        result.type = CommandType::DELETE_CMD;

        size_t fromPos = cmd.find("FROM") + 5;
        size_t wherePos = cmd.find("WHERE");

        if (wherePos == std::string::npos)
        {
            result.tableName = trim(cleaned.substr(fromPos));
            result.hasWhere = false;
        }
        else
        {
            result.tableName = trim(cleaned.substr(fromPos,
                                                   wherePos - fromPos));

            result.hasWhere = true;
            std::string cond = trim(cleaned.substr(wherePos + 6));

            const std::vector<std::string> ops = {"<=", ">=", "!=", "=", "<", ">"};
            for (const auto &op : ops)
            {
                size_t pos = cond.find(op);
                if (pos != std::string::npos)
                {
                    result.where.column = trim(cond.substr(0, pos));
                    result.where.op = op;
                    result.where.value = stripQuotes(
                        trim(cond.substr(pos + op.size())));
                    break;
                }
            }
        }
    }

    // UPDATE
    // UPDATE
    else if (cmd.rfind("UPDATE", 0) == 0)
    {
        result.type = CommandType::UPDATE;

        size_t setPos = cmd.find("SET");
        size_t wherePos = cmd.find("WHERE");

        result.tableName = trim(cleaned.substr(7, setPos - 7));
        result.setClause = trim(cleaned.substr(setPos + 4,
                                               wherePos - setPos - 4));

        result.hasWhere = true;
        std::string cond = trim(cleaned.substr(wherePos + 6));

        const std::vector<std::string> ops = {"<=", ">=", "!=", "=", "<", ">"};
        for (const auto &op : ops)
        {
            size_t pos = cond.find(op);
            if (pos != std::string::npos)
            {
                result.where.column = trim(cond.substr(0, pos));
                result.where.op = op;
                result.where.value = stripQuotes(
                    trim(cond.substr(pos + op.size())));
                break;
            }
        }
    }

    return result;
}
