#ifndef SELECT_H
#define SELECT_H
#include<string>
#include<vector>

bool selectColumns(
    const std::string& databaseName,
    const std::string& tableName,
    const std::vector<std::string>& selectedColumns,
    std::string& error
);


#endif