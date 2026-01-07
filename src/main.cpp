#include <iostream>
#include <string>
#include <algorithm>
#include "parser.h"

using namespace std;
int main()
{
    cout << "Welcome to KittyDB" << endl;
    cout << "Type 'exit' to quit" << endl;

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
            cout << "CREATE DATABASE\n";
            cout << "Database: " << pc.databaseName << endl;
            break;

        case CommandType::USE_DATABASE:
            cout << "USE DATABASE\n";
            cout << "Database: " << pc.databaseName << endl;
            break;

        case CommandType::CREATE:
            cout << "CREATE TABLE\n";
            cout << "Table: " << pc.tableName << endl;
            cout << "Schema: " << pc.schema << endl;
            break;

        case CommandType::INSERT:
            cout << "INSERT\n";
            cout << "Table: " << pc.tableName << endl;
            cout << "Values: " << pc.values << endl;
            break;

        case CommandType::SELECT:
            cout << "SELECT\n";
            cout << "Table: " << pc.tableName << endl;
            cout << "Where: " << pc.whereClause << endl;
            break;

        case CommandType::DELETE_CMD:
            cout << "DELETE\n";
            cout << "Table: " << pc.tableName << endl;
            cout << "Where: " << pc.whereClause << endl;
            break;

        case CommandType::UPDATE:
            cout << "UPDATE\n";
            cout << "Table: " << pc.tableName << endl;
            cout << "Set: " << pc.setClause << endl;
            cout << "Where: " << pc.whereClause << endl;
            break;

        default:
            cout << "Unknown command" << endl;
        }
    }
    cout << "Exiting KittyDB..." << endl;
}