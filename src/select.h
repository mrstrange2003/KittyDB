// select.h
#ifndef SELECT_H
#define SELECT_H

#include <string>
#include <vector>
#include "parser.h"
#include "where.h"

bool selectColumns(
    const std::string& databaseName,
    const std::string& tableName,
    const std::vector<std::string>& selectedColumns,
    bool hasWhere,
    const WhereCondition& where,
    bool distinct,
    bool hasOrderBy,
    const OrderByClause& orderBy,
    bool hasLimit,
    int limitCount,
    int offsetCount,
    std::string& error
);

#endif