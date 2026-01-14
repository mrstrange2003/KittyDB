// aggregate.h
#ifndef AGGREGATE_H
#define AGGREGATE_H

#include <string>
#include <vector>

struct Column;

enum class AggregateType {
    NONE = 0,
    COUNT = 1,
    SUM = 2,
    AVG = 3,
    MIN = 4,
    MAX = 5
};

struct AggregateFunction {
    AggregateType type;
    std::string column; // column name, or "*" for COUNT(*)
};

// Process aggregate functions
struct AggregateResult {
    bool hasAggregate;
    std::vector<AggregateFunction> functions;
    std::vector<std::string> resultLabels; // Labels for output
};

// Evaluate aggregate on a set of rows
void evaluateAggregate(
    const std::vector<Column>& columns,
    const std::vector<std::vector<std::string>>& rows,
    const std::vector<AggregateFunction>& functions,
    std::vector<std::string>& results
);

#endif
