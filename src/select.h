#ifndef SELECT_H
#define SELECT_H

#include <string>
#include <vector>
#include "parser.h"   // for WhereCondition

bool selectColumns(
    const std::string& databaseName,
    const std::string& tableName,
    const std::vector<std::string>& selectedColumns,
    bool hasWhere,
    const WhereCondition& where,
    std::string& error
);

#endif
