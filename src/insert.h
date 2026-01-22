// insert.h

#ifndef INSERT_H
#define INSERT_H

#include <string>

bool insertRow(
    const std::string &databaseName,
    const std::string &tableName,
    const std::string &values,
    std::string &error);

#endif