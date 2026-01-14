#include <iostream>
#include <string>
#include <algorithm>
#include <sys/stat.h>
#include "parser.h"
#include "where.h"
#include "aggregate.h"
#include "db.h"
#include "table.h"
#include "insert.h"
#include "select.h"
#include "delete.h"
#include "update.h"

using namespace std;

bool directoryExists(const string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

int main()
{
    cout << "Welcome to KittyDB" << endl;
    cout << "Type 'exit' to quit" << endl;

    string currentDatabase = "";

    while (true)
    {
        cout << "KittyDB > ";
        string command;
        getline(cin, command);

        if (command.empty())
        {
            continue;
        }

        string cmdUpper = command;
        transform(cmdUpper.begin(), cmdUpper.end(), cmdUpper.begin(), ::toupper);

        if (cmdUpper == "EXIT")
        {
            break;
        }

        ParsedCommand pc = parseCommand(command);

        switch (pc.type)
        {
        case CommandType::CREATE_DATABASE:
        {
            std::string error;
            if (createDatabase(pc.databaseName, error))
            {
                cout << "Database '" << pc.databaseName << "' created successfully." << endl;
            }
            else
            {
                cout << "Error: " << error << endl;
            }
            break;
        }

        case CommandType::USE_DATABASE:
        {
            string dbPath = "..\\databases\\" + pc.databaseName;

            if (!directoryExists(dbPath))
            {
                cout << "Error: database '" << pc.databaseName << "' does not exist." << endl;
            }
            else
            {
                currentDatabase = pc.databaseName;
                cout << "Using database '" << currentDatabase << "'" << endl;
            }
            break;
        }

        case CommandType::CREATE:
        {
            if (currentDatabase.empty())
            {
                cout << "Error: no database selected. Use USE <database> first." << endl;
                break;
            }

            string error;
            if (createTable(currentDatabase, pc.tableName, pc.schema, error))
            {
                cout << "Table '" << pc.tableName
                     << "' created in database '" << currentDatabase << "'." << endl;
            }
            else
            {
                cout << "Error: " << error << endl;
            }
            break;
        }

        case CommandType::INSERT:
        {
            if (currentDatabase.empty())
            {
                cout << "Error: no database selected." << endl;
                break;
            }

            string error;
            if (insertRow(currentDatabase, pc.tableName, pc.values, error))
            {
                cout << "1 row inserted into '" << pc.tableName << "'." << endl;
            }
            else
            {
                cout << "Error: " << error << endl;
            }
            break;
        }

        case CommandType::SELECT:
        {
            std::string error;
            if (pc.where == nullptr) {
                WhereCondition emptyWhere;
                if (!selectColumns(
                        currentDatabase,
                        pc.tableName,
                        pc.selectedColumns,
                        false,
                        emptyWhere,
                        pc.distinct,
                        pc.hasOrderBy,
                        pc.orderBy,
                        pc.hasLimit,
                        pc.limitCount,
                        pc.offsetCount,
                        pc.hasAggregates,
                        pc.aggregateFunctions,
                        error))
                {
                    std::cout << "Error: " << error << std::endl;
                }
            } else {
                if (!selectColumns(
                        currentDatabase,
                        pc.tableName,
                        pc.selectedColumns,
                        pc.hasWhere,
                        *pc.where,
                        pc.distinct,
                        pc.hasOrderBy,
                        pc.orderBy,
                        pc.hasLimit,
                        pc.limitCount,
                        pc.offsetCount,
                        pc.hasAggregates,
                        pc.aggregateFunctions,
                        error))
                {
                    std::cout << "Error: " << error << std::endl;
                }
                delete pc.where;
            }
            break;
        }

        case CommandType::DELETE_CMD:
        {
            if (!pc.hasWhere)
            {
                std::cout << "Error: DELETE without WHERE is not allowed.\n";
                break;
            }

            std::string error;
            if (pc.where != nullptr) {
                if (!deleteWhere(
                        currentDatabase,
                        pc.tableName,
                        pc.hasWhere,
                        *pc.where,
                        error))
                {
                    std::cout << "Error: " << error << std::endl;
                }
                else
                {
                    std::cout << "Rows deleted successfully.\n";
                }
                delete pc.where;
            }
            break;
        }

        case CommandType::UPDATE:
        {
            std::string error;
            if (pc.where != nullptr) {
                if (!updateWhere(
                        currentDatabase,
                        pc.tableName,
                        pc.setClause,
                        pc.hasWhere,
                        *pc.where,
                        error))
                {
                    std::cout << "Error: " << error << std::endl;
                }
                else
                {
                    std::cout << "Rows updated successfully.\n";
                }
                delete pc.where;
            }
            break;
        }

        default:
            cout << "Unknown command" << endl;
        }
    }
    cout << "Exiting KittyDB..." << endl;
    return 0;
}