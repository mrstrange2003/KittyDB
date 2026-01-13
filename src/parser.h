// parser.h
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

// Represents a single condition (column op value)
struct SimpleCondition {
    std::string column;
    std::string op;     // =, !=, <, >, <=, >=, LIKE, BETWEEN, IN
    std::string value;  // value or "val1,val2" for IN
    std::string value2; // for BETWEEN (end value)
};

// Represents AND/OR combinations
struct WhereCondition {
    std::vector<SimpleCondition> conditions;
    std::vector<std::string> logicalOps; // "AND" or "OR" between conditions
};

struct OrderByClause {
    std::string column;
    bool ascending = true; // true = ASC, false = DESC
};

struct ParsedCommand {
    CommandType type;

    std::string databaseName;
    std::string tableName;

    std::vector<std::string> selectedColumns;
    bool distinct = false;
    
    std::string values; //INSERT
    std::string schema; //CREATE
    
    bool hasWhere = false;
    WhereCondition where;

    bool hasOrderBy = false;
    OrderByClause orderBy;

    bool hasLimit = false;
    int limitCount = 0;
    int offsetCount = 0;

    std::string setClause; //UPDATE
};

ParsedCommand parseCommand(const std::string& command);

#endif