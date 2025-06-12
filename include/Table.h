#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <variant>
#include <optional>
#include "Types.h"
#include "Index.h"

namespace minidb {

// 表结构
class Table {
public:
    Table(const std::string& name, const std::string& dbName, 
          const std::vector<ColumnDef>& columns);
    ~Table();

    // 获取表名
    std::string getName() const { return name_; }
    
    // 获取表所在数据库名
    std::string getDbName() const { return dbName_; }
    
    // 获取列定义
    const std::vector<ColumnDef>& getColumns() const { return columns_; }
    
    // 获取主键列索引
    std::optional<size_t> getPrimaryKeyColumn() const { return primaryKeyCol_; }
    
    // 插入记录
    bool insert(const std::vector<Value>& values);
    
    // 根据条件删除记录
    int deleteWhere(const std::string& colName, Operator op, const Value& value);
    
    // 根据条件更新记录
    int updateWhere(const std::string& setColName, const Value& setValue, 
                    const std::string& whereColName, Operator op, const Value& whereValue);
    
    // 根据条件查询记录
    std::vector<Record> selectWhere(const std::string& colName, 
                                     Operator op, const Value& value, 
                                     const std::string& selectCol);
    
    // 查询所有记录
    std::vector<Record> selectAll(const std::string& selectCol);
    
    // 加载表数据
    bool loadData();
    
    // 保存表数据
    bool saveData() const;
    
    // 创建索引
    bool createIndex();
    
    // 获取列索引
    std::optional<size_t> getColumnIndex(const std::string& colName) const;

private:
    std::string name_;
    std::string dbName_;
    std::vector<ColumnDef> columns_;
    std::optional<size_t> primaryKeyCol_;
    std::vector<Record> records_;
    std::unique_ptr<Index> index_;
    std::filesystem::path tablePath_;
    
    // 检查记录是否符合条件
    bool matchCondition(const Record& record, size_t colIndex, Operator op, const Value& value) const;
};

} // namespace minidb 