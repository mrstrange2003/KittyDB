#include<fstream>
#include<iostream>
#include<vector>
#include<string>
#include<sys/stat.h>
#include "select.h"
#include "schema.h"

static bool directoryExists(const std::string& path){
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

static bool fileExists(const std::string& path){
    struct stat info;
    return (stat(path.c_str(), &info)==0);
}

bool selectAll(
    const std::string& databaseName,
    const std::string& tableName,
    std::string& error
){
    if(databaseName.empty()){
        error="No database selected";
        return false;
    }

    std::string basePath= "..\\databases\\"+databaseName+"\\";
    if(!directoryExists(basePath)){
        error="Database doesn not exist";
        return false;
    }

    std::string tblPath  = basePath + tableName + ".tbl";
    std::string metaPath = basePath + tableName + ".meta";

    if (!fileExists(tblPath) || !fileExists(metaPath)) {
        error = "Table does not exist";
        return false;
    }

    //parse schema
    std::vector<Column> columns;
    if (!parseSchema(metaPath, columns, error)) {
        return false;
    }

    //print header
    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << columns[i].name;
        if (i != columns.size() - 1)
            std::cout << " | ";
    }
    std::cout << "\n";

    // separator
    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << "--------";
    }
    std::cout << "\n";

    //read and print rows
    std::ifstream tblFile(tblPath);
    if (!tblFile) {
        error = "Failed to open table data";
        return false;
    }

    std::string line;
    while (std::getline(tblFile, line)) {
        std::string field;
        for (char c : line) {
            if (c == '|') {
                std::cout << field << " | ";
                field.clear();
            } else {
                field += c;
            }
        }
        std::cout << field << "\n";
    }

    tblFile.close();
    return true;

}