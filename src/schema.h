// schema.h

#ifndef SCHEMA_H
#define SCHEMA_H

#include <string>
#include <vector>

struct Column
{
    std::string name;
    std::string type;
    bool notNull = false;
};

// parse schema from .meta file
bool parseSchema(
    const std::string &metaPath,
    std::vector<Column> &columns,
    std::string &error);

#endif