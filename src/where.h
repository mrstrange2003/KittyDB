// where.h

#ifndef WHERE_H
#define WHERE_H

#include "schema.h"

#include <string>
#include <vector>
#include <sstream>

struct Column;

// SimpleCondition and WhereCondition are defined here
struct SimpleCondition
{
    std::string column;
    std::string op;
    std::string value;
    std::string value2;

    bool isNullCheck = false;
    bool isNotNullCheck = false;
};

struct WhereCondition
{
    std::vector<SimpleCondition> conditions;
    std::vector<std::string> logicalOps;
};

bool evaluateWhere(
    const std::vector<Column> &columns,
    const std::vector<std::string> &rowValues,
    const WhereCondition &where);

#endif
