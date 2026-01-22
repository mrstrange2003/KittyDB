#ifndef DB_H
#define DB_H

#include <string>

// creates database  dorectory
bool createDatabase(const std::string &dbName, std::string &error);

#endif