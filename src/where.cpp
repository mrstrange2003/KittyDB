#include "where.h"
#include <cstdlib>

static int getColumnIndex(
    const std::vector<Column>& columns,
    const std::string& name
) {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == name)
            return (int)i;
    }
    return -1;
}

bool evaluateWhere(
    const std::vector<Column>& columns,
    const std::vector<std::string>& rowValues,
    const std::string& column,
    const std::string& op,
    const std::string& value
) {
    int idx = getColumnIndex(columns, column);
    if (idx == -1 || idx >= (int)rowValues.size())
        return false;

    std::string cell = rowValues[idx];

    // numeric comparison
    if (columns[idx].type == "INT") {
        int a = std::stoi(cell);
        int b = std::stoi(value);

        if (op == "=")  return a == b;
        if (op == "!=") return a != b;
        if (op == "<")  return a < b;
        if (op == ">")  return a > b;
        if (op == "<=") return a <= b;
        if (op == ">=") return a >= b;
    }
    else {
        // string comparison
        if (op == "=")  return cell == value;
        if (op == "!=") return cell != value;
    }

    return false;
}
