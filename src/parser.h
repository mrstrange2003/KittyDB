#ifndef PARSER_H
#define PARSER_H
#include<string>
#include<vector>

enum class CommandType{
    CREATE_DATABASE,
    USE_DATABASE,
    CREATE,
    INSERT,
    SELECT,
    DELETE_CMD,
    UPDATE,
    UNKNOWN
};

struct WhereCondition {
    std::string column;
    std::string op;     // =, !=, <, >, <=, >=
    std::string value;
};


struct  ParsedCommand
{
    CommandType type;

    std::string databaseName;
    std::string tableName;

    std::vector<std::string> selectedColumns;
    std::string values; //INSERT
    std::string schema; //CREATE
    //std::string whereClause; //SELECT/DELETE/UPDATE
    
    bool hasWhere = false;
    WhereCondition where;

    std::string setClause; //UPDATE
};

ParsedCommand parseCommand(const std::string& command);

#endif