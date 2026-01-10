#include "parser.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>

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

ParsedCommand parseCommand(const string &command)
{
    ParsedCommand result;
    result.type = CommandType::UNKNOWN;

    string cmd = command;
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    // CREATE DATABASE
    if (cmd.rfind("CREATE DATABASE", 0) == 0)
    {
        result.type = CommandType::CREATE_DATABASE;

        size_t namePos = cmd.find("DATABASE") + 9;
        result.databaseName = trim(command.substr(namePos));

        return result;
    }

    // USE DATABASE
    else if (cmd.rfind("USE", 0) == 0)
    {
        result.type = CommandType::USE_DATABASE;

        size_t namePos = 4; // after "USE "
        result.databaseName = trim(command.substr(namePos));

        return result;
    }

    // CREATE TABLE
    else if (cmd.rfind("CREATE TABLE", 0) == 0)
    {
        result.type = CommandType::CREATE;

        size_t tablePos = cmd.find("TABLE") + 6;
        size_t parenPos = command.find('(', tablePos);

        result.tableName = trim(command.substr(tablePos, parenPos - tablePos));

        size_t closeParen = command.find(')', parenPos);
        result.schema = command.substr(parenPos + 1,
                                       closeParen - parenPos - 1);
    }

    // INSERT
    else if (cmd.rfind("INSERT INTO", 0) == 0)
    {
        result.type = CommandType::INSERT;

        size_t intoPos = cmd.find("INTO") + 5;
        size_t valuesPos = cmd.find("VALUES");

        result.tableName = trim(command.substr(intoPos,
                                               valuesPos - intoPos));

        size_t openParen = command.find('(', valuesPos);
        size_t closeParen = command.find(')', openParen);

        result.values = command.substr(openParen + 1,
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
        std::string colPart = trim(command.substr(6, fromPos - 6));
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
            result.tableName = trim(command.substr(fromPos + 5));
        }
        else
        {
            result.tableName = trim(command.substr(fromPos + 5,
                                                   wherePos - (fromPos + 5)));

            result.hasWhere = true;

            std::string cond = trim(command.substr(wherePos + 6));

            // detect operator
            const std::vector<std::string> ops = {"<=", ">=", "!=", "=", "<", ">"};
            for (const auto &op : ops)
            {
                size_t pos = cond.find(op);
                if (pos != std::string::npos)
                {
                    result.where.column = trim(cond.substr(0, pos));
                    result.where.op = op;
                    result.where.value = trim(cond.substr(pos + op.size()));
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

        result.tableName = trim(command.substr(fromPos,
                                               wherePos - fromPos));
        // result.whereClause = trim(command.substr(wherePos + 6));
    }

    // UPDATE
    else if (cmd.rfind("UPDATE", 0) == 0)
    {
        result.type = CommandType::UPDATE;

        size_t setPos = cmd.find("SET");
        size_t wherePos = cmd.find("WHERE");

        result.tableName = trim(command.substr(7, setPos - 7));
        result.setClause = trim(command.substr(setPos + 4,
                                               wherePos - setPos - 4));
        // result.whereClause = trim(command.substr(wherePos + 6));
    }

    return result;
}
