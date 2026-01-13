// where.cpp

#include "where.h"
#include <cstdlib>
#include <algorithm>
#include <cctype>

static int getColumnIndex(
    const std::vector<Column>& columns,
    const std::string& name) {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == name)
            return (int)i;
    }
    return -1;
}

static std::string trim(std::string s) {
    while (!s.empty() && s.front() == ' ')
        s.erase(s.begin());
    while (!s.empty() && s.back() == ' ')
        s.pop_back();
    return s;
}

// LIKE pattern matching: % = any chars, _ = single char
static bool matchLike(const std::string& text, const std::string& pattern) {
    size_t textIdx = 0, patIdx = 0;
    size_t starIdx = (size_t)-1;
    int matchIdx = 0;

    while (textIdx < text.size()) {
        if (patIdx < pattern.size() && pattern[patIdx] == '%') {
            starIdx = patIdx;
            matchIdx = (int)textIdx;
            patIdx++;
        } else if (patIdx < pattern.size() && 
                   (pattern[patIdx] == '_' || pattern[patIdx] == text[textIdx])) {
            patIdx++;
            textIdx++;
        } else if (starIdx != (size_t)-1) {
            patIdx = starIdx + 1;
            matchIdx++;
            textIdx = (size_t)matchIdx;
        } else {
            return false;
        }
    }

    while (patIdx < pattern.size() && pattern[patIdx] == '%') {
        patIdx++;
    }

    return patIdx == pattern.size();
}

// Evaluate IN operator: "val1,val2,val3"
static bool evaluateIn(const std::string& value, const std::string& inList) {
    std::stringstream ss(inList);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        // Remove quotes if present
        if (item.size() >= 2 && item.front() == '\'' && item.back() == '\'') {
            item = item.substr(1, item.size() - 2);
        }
        if (item == value) {
            return true;
        }
    }
    return false;
}

// Evaluate BETWEEN operator
static bool evaluateBetween(const std::string& value, const std::string& val1, 
                           const std::string& val2, const std::string& type) {
    if (type == "INT") {
        int v = std::stoi(value);
        int v1 = std::stoi(val1);
        int v2 = std::stoi(val2);
        return v >= v1 && v <= v2;
    } else if (type == "FLOAT") {
        double v = std::stod(value);
        double v1 = std::stod(val1);
        double v2 = std::stod(val2);
        return v >= v1 && v <= v2;
    } else {
        // String comparison
        return value >= val1 && value <= val2;
    }
}

// Evaluate a single condition
static bool evaluateCondition(
    const std::vector<Column>& columns,
    const std::vector<std::string>& rowValues,
    const SimpleCondition& cond) {
    
    int idx = getColumnIndex(columns, cond.column);
    if (idx == -1 || idx >= (int)rowValues.size())
        return false;

    std::string cell = rowValues[idx];
    std::string type = columns[idx].type;

    // Numeric comparison
    if (type == "INT") {
        try {
            int a = std::stoi(cell);
            int b = std::stoi(cond.value);

            if (cond.op == "=")  return a == b;
            if (cond.op == "!=") return a != b;
            if (cond.op == "<")  return a < b;
            if (cond.op == ">")  return a > b;
            if (cond.op == "<=") return a <= b;
            if (cond.op == ">=") return a >= b;
            if (cond.op == "BETWEEN") return evaluateBetween(cell, cond.value, cond.value2, type);
        } catch (...) {
            return false;
        }
    }
    else if (type == "FLOAT") {
        try {
            double a = std::stod(cell);
            double b = std::stod(cond.value);

            if (cond.op == "=")  return a == b;
            if (cond.op == "!=") return a != b;
            if (cond.op == "<")  return a < b;
            if (cond.op == ">")  return a > b;
            if (cond.op == "<=") return a <= b;
            if (cond.op == ">=") return a >= b;
            if (cond.op == "BETWEEN") return evaluateBetween(cell, cond.value, cond.value2, type);
        } catch (...) {
            return false;
        }
    }
    else {
        // String comparison
        if (cond.op == "=")  return cell == cond.value;
        if (cond.op == "!=") return cell != cond.value;
        if (cond.op == "LIKE") return matchLike(cell, cond.value);
        if (cond.op == "IN") return evaluateIn(cell, cond.value);
        if (cond.op == "BETWEEN") return evaluateBetween(cell, cond.value, cond.value2, type);
    }

    return false;
}

// Evaluate entire WHERE clause with AND/OR support
bool evaluateWhere(
    const std::vector<Column>& columns,
    const std::vector<std::string>& rowValues,
    const WhereCondition& where) {
    
    if (where.conditions.empty())
        return true;

    if (where.conditions.size() == 1) {
        return evaluateCondition(columns, rowValues, where.conditions[0]);
    }

    // Multiple conditions with AND/OR
    bool result = evaluateCondition(columns, rowValues, where.conditions[0]);

    for (size_t i = 0; i < where.logicalOps.size(); i++) {
        bool nextResult = evaluateCondition(columns, rowValues, where.conditions[i + 1]);
        
        if (where.logicalOps[i] == "AND") {
            result = result && nextResult;
        } else if (where.logicalOps[i] == "OR") {
            result = result || nextResult;
        }
    }

    return result;
}