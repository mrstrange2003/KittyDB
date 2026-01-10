#ifndef WHERE_H
#define WHERE_H
#include<string>
#include<vector>
#include "schema.h"

bool evaluateWhere(
    const std::vector<Column>& columns,
    const std::vector<std::string>& rowValues,
    const std::string& column,
    const std::string& op,
    const std::string& value
);

#endif