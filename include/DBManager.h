#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include "Database.h"

namespace minidb {

class DBManager {
public:
    static DBManager& getInstance();
    
    // 创建数据库
    bool createDatabase(const std::string& dbName);
    
    // 删除数据库
    bool dropDatabase(const std::string& dbName);
    
    // 使用数据库
    bool useDatabase(const std::string& dbName);
    
    // 获取当前数据库
    std::shared_ptr<Database> getCurrentDatabase() const;
    
    // 获取当前数据库名称
    std::string getCurrentDatabaseName() const;
    
    // 初始化数据目录
    bool initDataDirectory();
    
    // 加载所有数据库
    bool loadDatabases();

private:
    DBManager();
    ~DBManager();
    
    // 禁止拷贝和移动
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;
    DBManager(DBManager&&) = delete;
    DBManager& operator=(DBManager&&) = delete;
    
    std::filesystem::path dataPath_;
    std::unordered_map<std::string, std::shared_ptr<Database>> databases_;
    std::string currentDbName_;
};

} // namespace minidb 