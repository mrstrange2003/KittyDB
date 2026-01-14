// select.h
#ifndef SELECT_H
#define SELECT_H

#include <string>
#include <vector>
#include "parser.h"
#include "where.h"
#include "aggregate.h"

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
    bool hasAggregates,
    const std::vector<AggregateFunction>& aggregates,
    std::string& error
);

#endif
