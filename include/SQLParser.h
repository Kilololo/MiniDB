#pragma once

#include <string>
#include <vector>
#include <optional>
#include <tuple>
#include "Types.h"

namespace minidb {

// SQL语句类型
enum class SQLType {
    CREATE_DATABASE,
    DROP_DATABASE,
    USE_DATABASE,
    CREATE_TABLE,
    DROP_TABLE,
    INSERT,
    DELETE,
    UPDATE,
    SELECT,
    UNKNOWN
};

// 解析结果
struct SQLResult {
    SQLType type = SQLType::UNKNOWN;
    std::string message;
    bool success = false;
};

// 创建表的列定义
struct CreateTableColumn {
    std::string name;
    std::string type;
    bool isPrimary = false;
};

class SQLParser {
public:
    // 解析并执行SQL语句
    static SQLResult execute(const std::string& sql);
    
private:
    // 解析CREATE DATABASE语句
    static SQLResult parseCreateDatabase(const std::string& sql);
    
    // 解析DROP DATABASE语句
    static SQLResult parseDropDatabase(const std::string& sql);
    
    // 解析USE语句
    static SQLResult parseUse(const std::string& sql);
    
    // 解析CREATE TABLE语句
    static SQLResult parseCreateTable(const std::string& sql);
    
    // 解析DROP TABLE语句
    static SQLResult parseDropTable(const std::string& sql);
    
    // 解析INSERT语句
    static SQLResult parseInsert(const std::string& sql);
    
    // 解析DELETE语句
    static SQLResult parseDelete(const std::string& sql);
    
    // 解析UPDATE语句
    static SQLResult parseUpdate(const std::string& sql);
    
    // 解析SELECT语句
    static SQLResult parseSelect(const std::string& sql);
    
    // 解析WHERE子句
    static std::optional<std::tuple<std::string, Operator, std::string>> parseWhereClause(const std::string& whereClause);
    
    // 解析值列表
    static std::vector<std::string> parseValues(const std::string& values);
    
    // 解析列定义
    static std::vector<CreateTableColumn> parseColumnDefs(const std::string& columnDefs);
    
    // 将字符串操作符转换为枚举
    static Operator stringToOperator(const std::string& op);
    
    // 将字符串类型转换为枚举
    static DataType stringToDataType(const std::string& type);
    
    // 从字符串中提取引号内的内容
    static std::string extractQuotedString(const std::string& str);
    
    // 检查标识符是否有效
    static bool isValidIdentifier(const std::string& identifier);
};

} // namespace minidb 