// aggregate.cpp

#include "aggregate.h"
#include "schema.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>

static int getColumnIndex(
    const std::vector<Column>& columns,
    const std::string& name) {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == name)
            return (int)i;
    }
    return -1;
}

void evaluateAggregate(
    const std::vector<Column>& columns,
    const std::vector<std::vector<std::string>>& rows,
    const std::vector<AggregateFunction>& functions,
    std::vector<std::string>& results) {
    
    for (const auto& func : functions) {
        std::string result;

        if (func.type == AggregateType::COUNT) {
            if (func.column == "*") {
                // COUNT(*)
                result = std::to_string(rows.size());
            } else {
                // COUNT(column) - count non-null values
                int colIdx = getColumnIndex(columns, func.column);
                int count = 0;
                if (colIdx != -1) {
                    for (const auto& row : rows) {
                        if (colIdx < (int)row.size() && !row[colIdx].empty()) {
                            count++;
                        }
                    }
                }
                result = std::to_string(count);
            }
        }
        else if (func.type == AggregateType::SUM) {
            int colIdx = getColumnIndex(columns, func.column);
            double sum = 0.0;
            int count = 0;
            
            if (colIdx != -1) {
                for (const auto& row : rows) {
                    if (colIdx < (int)row.size() && !row[colIdx].empty()) {
                        try {
                            if (columns[colIdx].type == "FLOAT") {
                                sum += std::stod(row[colIdx]);
                            } else if (columns[colIdx].type == "INT") {
                                sum += std::stoi(row[colIdx]);
                            }
                            count++;
                        } catch (...) {
                            // Skip invalid values
                        }
                    }
                }
            }
            
            // Format output
            if (count > 0) {
                if (columns[colIdx].type == "INT") {
                    result = std::to_string((int)sum);
                } else {
                    result = std::to_string(sum);
                }
            } else {
                result = "0";
            }
        }
        else if (func.type == AggregateType::AVG) {
            int colIdx = getColumnIndex(columns, func.column);
            double sum = 0.0;
            int count = 0;
            
            if (colIdx != -1) {
                for (const auto& row : rows) {
                    if (colIdx < (int)row.size() && !row[colIdx].empty()) {
                        try {
                            if (columns[colIdx].type == "FLOAT") {
                                sum += std::stod(row[colIdx]);
                            } else if (columns[colIdx].type == "INT") {
                                sum += std::stoi(row[colIdx]);
                            }
                            count++;
                        } catch (...) {
                            // Skip invalid values
                        }
                    }
                }
            }
            
            if (count > 0) {
                double avg = sum / count;
                result = std::to_string(avg);
            } else {
                result = "0";
            }
        }
        else if (func.type == AggregateType::MIN) {
            int colIdx = getColumnIndex(columns, func.column);
            
            if (colIdx != -1 && !rows.empty()) {
                std::string minVal = "";
                bool found = false;
                
                for (const auto& row : rows) {
                    if (colIdx < (int)row.size() && !row[colIdx].empty()) {
                        if (!found) {
                            minVal = row[colIdx];
                            found = true;
                        } else {
                            try {
                                if (columns[colIdx].type == "INT") {
                                    int a = std::stoi(minVal);
                                    int b = std::stoi(row[colIdx]);
                                    if (b < a) minVal = row[colIdx];
                                } else if (columns[colIdx].type == "FLOAT") {
                                    double a = std::stod(minVal);
                                    double b = std::stod(row[colIdx]);
                                    if (b < a) minVal = row[colIdx];
                                } else {
                                    if (row[colIdx] < minVal) minVal = row[colIdx];
                                }
                            } catch (...) {
                                // Skip invalid values
                            }
                        }
                    }
                }
                result = found ? minVal : "NULL";
            } else {
                result = "NULL";
            }
        }
        else if (func.type == AggregateType::MAX) {
            int colIdx = getColumnIndex(columns, func.column);
            
            if (colIdx != -1 && !rows.empty()) {
                std::string maxVal = "";
                bool found = false;
                
                for (const auto& row : rows) {
                    if (colIdx < (int)row.size() && !row[colIdx].empty()) {
                        if (!found) {
                            maxVal = row[colIdx];
                            found = true;
                        } else {
                            try {
                                if (columns[colIdx].type == "INT") {
                                    int a = std::stoi(maxVal);
                                    int b = std::stoi(row[colIdx]);
                                    if (b > a) maxVal = row[colIdx];
                                } else if (columns[colIdx].type == "FLOAT") {
                                    double a = std::stod(maxVal);
                                    double b = std::stod(row[colIdx]);
                                    if (b > a) maxVal = row[colIdx];
                                } else {
                                    if (row[colIdx] > maxVal) maxVal = row[colIdx];
                                }
                            } catch (...) {
                                // Skip invalid values
                            }
                        }
                    }
                }
                result = found ? maxVal : "NULL";
            } else {
                result = "NULL";
            }
        }

        results.push_back(result);
    }
}