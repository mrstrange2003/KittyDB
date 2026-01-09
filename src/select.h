#ifndef SELECT_H
#define SELECT_H
#include<string>
#include<vector>

bool selectRows(
    const std::string& databaseName,
    const std::string& tableName,
    const std::vector<std::string>& selectedColumns,
    std::string& error
);


#endif