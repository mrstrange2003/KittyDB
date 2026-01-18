#ifndef TABLE_H
#define TABLE_H
#include<string>

//create table files in database
bool createTable(
    const std::string& databaseName,
    const std::string& tableName,
    const std::string& schema,
    std::string& error 
);

bool describeTable(
    const std::string& databaseName,
    const std::string& tableName,
    std::string& error
);

bool showTables(
    const std::string& databaseName,
    std::string& error
);



#endif