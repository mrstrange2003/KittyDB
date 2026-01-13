// where.h
#ifndef WHERE_H
#define WHERE_H

#include <string>
#include <vector>
#include <sstream>

// Forward declare Column - we need schema.h later
struct Column;

// SimpleCondition and WhereCondition are defined here
struct SimpleCondition {
    std::string column;
    std::string op;
    std::string value;
    std::string value2;
};

struct WhereCondition {
    std::vector<SimpleCondition> conditions;
    std::vector<std::string> logicalOps;
};

// Now include schema.h for Column definition
#include "schema.h"

bool evaluateWhere(
    const std::vector<Column>& columns,
    const std::vector<std::string>& rowValues,
    const WhereCondition& where
);

#endif

