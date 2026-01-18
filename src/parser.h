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
    DESCRIBE,
    UNKNOWN
};

// Forward declare structures from where.h
struct SimpleCondition;
struct WhereCondition;

// We need to include aggregate.h for AggregateFunction
#include "aggregate.h"

struct OrderByClause {
    std::string column;
    bool ascending = true;
};

struct ParsedCommand {
    CommandType type;

    std::string databaseName;
    std::string tableName;

    std::vector<std::string> selectedColumns;
    std::vector<AggregateFunction> aggregateFunctions;
    bool hasAggregates = false;
    bool distinct = false;
    
    std::string values;
    std::string schema;
    
    bool hasWhere = false;
    WhereCondition* where = nullptr;

    bool hasOrderBy = false;
    OrderByClause orderBy;

    bool hasLimit = false;
    int limitCount = 0;
    int offsetCount = 0;

    std::string setClause;
};

ParsedCommand parseCommand(const std::string& command);

#endif
