#include "db.h"
#include<filesystem>

namespace fs= std::filesystem;

bool createDatabase(const std::string& dbName, std::string& error){
    if(dbName.empty()){
        error= "Database name can't be empty";
        return false;
    }

    fs::path dbPath= fs::path("../databases") / dbName;

    if(fs::exists(dbPath)){
        error = "Database already exists";
        return false;
    }

    try{
        fs::create_directories(dbPath);
    } catch(...){
        error="Failed to create database directory";
        return false;
    }

    return true;
}