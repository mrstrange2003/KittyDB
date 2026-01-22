// main.cpp

#include "parser.h"
#include "where.h"
#include "aggregate.h"
#include "db.h"
#include "table.h"
#include "insert.h"
#include "select.h"
#include "delete.h"
#include "update.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

bool directoryExists(const string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

int main()
{
    // shell commands
    cout << "Welcome to KittyDB" << endl;
    cout << "Type 'help' to show supported commands" << endl;
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

        if (cmdUpper == "HELP")
        {
            std::cout << "\n=== KittyDB Help ===\n\n";

            std::cout << "Database commands:\n";
            std::cout << "  CREATE DATABASE <name>\n";
            std::cout << "  USE <database>\n\n";

            std::cout << "Table commands:\n";
            std::cout << "  CREATE TABLE <table> (col TYPE, ...)\n";
            std::cout << "  SHOW TABLES\n";
            std::cout << "  DESCRIBE <table>\n";
            std::cout << "  TRUNCATE <table>\n\n";

            std::cout << "Data commands:\n";
            std::cout << "  INSERT INTO <table> VALUES (v1, v2, ...)\n";
            std::cout << "  SELECT [DISTINCT] <cols> FROM <table>\n";
            std::cout << "         [WHERE condition]\n";
            std::cout << "         [ORDER BY col ASC|DESC]\n";
            std::cout << "         [LIMIT n OFFSET m]\n\n";

            std::cout << "  UPDATE <table> SET col=value [, col=value]\n";
            std::cout << "         WHERE condition\n";
            std::cout << "  DELETE FROM <table> WHERE condition\n\n";

            std::cout << "Conditions:\n";
            std::cout << "  =  !=  <  >  <=  >=\n";
            std::cout << "  BETWEEN a AND b\n";
            std::cout << "  IN (a, b, c)\n";
            std::cout << "  IS NULL / IS NOT NULL\n\n";

            std::cout << "System:\n";
            std::cout << "  EXIT\n\n";

            continue;
        }

        ParsedCommand pc = parseCommand(command);

        switch (pc.type)
        {
            // CREATE DATABASE command
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
            // USE DATABASE command
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
            // CREATE TABLE command
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
            // INSERT VALUES into TABLE command
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
            // DESCRIBE TABLE command
        case CommandType::DESCRIBE:
        {
            std::string error;
            if (!describeTable(currentDatabase, pc.tableName, error))
            {
                std::cout << "Error: " << error << "\n";
            }
            break;
        }
            // SHOW TABLES IN DATABASE command
        case CommandType::SHOW_TABLES:
        {
            std::string error;
            if (!showTables(currentDatabase, error))
            {
                std::cout << "Error: " << error << "\n";
            }
            break;
        }
            // TRUNCATE TABLE VALUES command
        case CommandType::TRUNCATE:
        {
            std::string error;
            if (!truncateTable(currentDatabase, pc.tableName, error))
            {
                std::cout << "Error: " << error << "\n";
            }
            else
            {
                std::cout << "Table '" << pc.tableName << "' truncated.\n";
            }
            break;
        }
            // SELECT(display) TABLE VALUES
        case CommandType::SELECT:
        {
            std::string error;
            if (pc.where == nullptr)
            {
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
            }
            else
            {
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
            // DELETE TABLE rows
        case CommandType::DELETE_CMD:
        {
            if (!pc.hasWhere)
            {
                std::cout << "Error: DELETE without WHERE is not allowed.\n";
                break;
            }

            std::string error;
            if (pc.where != nullptr)
            {
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
            // UPDATE TABLE rows
        case CommandType::UPDATE:
        {
            std::string error;
            if (pc.where != nullptr)
            {
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