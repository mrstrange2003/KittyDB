// update.h
#ifndef UPDATE_H
#define UPDATE_H

#include <string>
#include "parser.h"

bool updateWhere(
    const std::string& databaseName,
    const std::string& tableName,
    const std::string& setClause,
    bool hasWhere,
    const WhereCondition& where,
    std::string& error
);

#endif
