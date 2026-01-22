// delete.h

#ifndef DELETE_H
#define DELETE_H

#include "parser.h"

#include <string>

bool deleteWhere(
    const std::string &databaseName,
    const std::string &tableName,
    bool hasWhere,
    const WhereCondition &where,
    std::string &error);

#endif
