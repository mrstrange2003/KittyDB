#ifndef SELECT_H
#define SELECT_H
#include<string>

bool selectAll(
    const std::string& databaseName,
    const std::string& tableName,
    std::string& error
);

#endif