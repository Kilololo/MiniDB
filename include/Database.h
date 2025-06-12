#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include "Table.h"

namespace minidb {

class Database {
public:
    explicit Database(const std::string& name);
    ~Database();

    // 获取数据库名称
    std::string getName() const { return name_; }

    // 创建表
    bool createTable(const std::string& tableName, const std::vector<ColumnDef>& columns);
    
    // 删除表
    bool dropTable(const std::string& tableName);
    
    // 获取表
    std::shared_ptr<Table> getTable(const std::string& tableName);
    
    // 加载数据库中的表
    bool loadTables();
    
    // 保存数据库状态
    bool saveMetadata() const;

private:
    std::string name_;
    std::filesystem::path dbPath_;
    std::unordered_map<std::string, std::shared_ptr<Table>> tables_;
};

} // namespace minidb 