#include "../include/SQLParser.h"
#include "../include/DBManager.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>

namespace minidb {

// 辅助函数：将字符串转换为小写
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// 辅助函数：去除字符串前后空格
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

// 辅助函数：分割字符串
std::vector<std::string> split(const std::string& str, const std::string& delim) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) {
            pos = str.length();
        }
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) {
            tokens.push_back(token);
        }
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

SQLResult SQLParser::execute(const std::string& sql) {
    // 将SQL语句转换为小写（保留字符串字面量的大小写）
    std::string lowerSql = sql;
    
    // 去除前后空格
    lowerSql = trim(lowerSql);
    
    // 确定SQL语句类型
    if (lowerSql.substr(0, 15) == "create database") {//substr的第二个参数是提取长度
        return parseCreateDatabase(lowerSql);
    } else if (lowerSql.substr(0, 13) == "drop database") {
        return parseDropDatabase(lowerSql);
    } else if (lowerSql.substr(0, 3) == "use") {
        return parseUse(lowerSql);
    } else if (lowerSql.substr(0, 12) == "create table") {
        return parseCreateTable(lowerSql);
    } else if (lowerSql.substr(0, 10) == "drop table") {
        return parseDropTable(lowerSql);
    } else if (lowerSql.substr(0, 6) == "insert") {
        return parseInsert(lowerSql);
    } else if (lowerSql.substr(0, 6) == "delete") {
        return parseDelete(lowerSql);
    } else if (lowerSql.substr(0, 6) == "update") {
        return parseUpdate(lowerSql);
    } else if (lowerSql.substr(0, 6) == "select") {
        return parseSelect(lowerSql);
    } else {
        return {SQLType::UNKNOWN, "错误：未知的SQL语句", false};
    }
}

SQLResult SQLParser::parseCreateDatabase(const std::string& sql) {
    std::regex pattern(R"(create\s+database\s+(\w+))");
    std::smatch matches;
    
    if (std::regex_search(sql, matches, pattern) && matches.size() > 1) {
        std::string dbName = matches[1].str();
        
        if (!isValidIdentifier(dbName)) {
            return {SQLType::CREATE_DATABASE, "错误：无效的数据库名", false};
        }
        
        if (DBManager::getInstance().createDatabase(dbName)) {
            return {SQLType::CREATE_DATABASE, "数据库 " + dbName + " 创建成功", true};
        } else {
            return {SQLType::CREATE_DATABASE, "错误：创建数据库失败，数据库可能已存在", false};
        }
    } else {
        return {SQLType::CREATE_DATABASE, "错误：CREATE DATABASE 语法错误", false};
    }
}

SQLResult SQLParser::parseDropDatabase(const std::string& sql) {
    std::regex pattern(R"(drop\s+database\s+(\w+))");
    std::smatch matches;
    
    if (std::regex_search(sql, matches, pattern) && matches.size() > 1) {
        std::string dbName = matches[1].str();
        
        if (DBManager::getInstance().dropDatabase(dbName)) {
            return {SQLType::DROP_DATABASE, "数据库 " + dbName + " 删除成功", true};
        } else {
            return {SQLType::DROP_DATABASE, "错误：删除数据库失败，数据库可能不存在", false};
        }
    } else {
        return {SQLType::DROP_DATABASE, "错误：DROP DATABASE 语法错误", false};
    }
}

SQLResult SQLParser::parseUse(const std::string& sql) {
    std::regex pattern(R"(use\s+(\w+))");
    std::smatch matches;
    
    if (std::regex_search(sql, matches, pattern) && matches.size() > 1) {
        std::string dbName = matches[1].str();
        
        if (DBManager::getInstance().useDatabase(dbName)) {
            return {SQLType::USE_DATABASE, "数据库 " + dbName + " 切换成功", true};
        } else {
            return {SQLType::USE_DATABASE, "错误：切换数据库失败，数据库可能不存在", false};
        }
    } else {
        return {SQLType::USE_DATABASE, "错误：USE 语法错误", false};
    }
}

SQLResult SQLParser::parseCreateTable(const std::string& sql) {
    std::regex pattern(R"(create\s+table\s+(\w+)\s*\((.*)\))");
    std::smatch matches;
    
    if (std::regex_search(sql, matches, pattern) && matches.size() > 2) {
        std::string tableName = matches[1].str();
        std::string columnDefsStr = matches[2].str();
        
        if (!isValidIdentifier(tableName)) {
            return {SQLType::CREATE_TABLE, "错误：无效的表名", false};
        }
        
        // 获取当前数据库
        auto db = DBManager::getInstance().getCurrentDatabase();
        if (!db) {
            return {SQLType::CREATE_TABLE, "错误：未选择数据库", false};
        }
        
        // 解析列定义
        auto columns = parseColumnDefs(columnDefsStr);
        if (columns.empty()) {
            return {SQLType::CREATE_TABLE, "错误：无效的列定义", false};
        }
        
        // 转换列定义
        std::vector<ColumnDef> columnDefs;
        for (const auto& col : columns) {
            DataType type;
            try {
                type = stringToDataType(col.type);
            } catch (const std::exception& e) {
                return {SQLType::CREATE_TABLE, "错误：无效的列类型：" + col.type, false};
            }
            
            if (!isValidIdentifier(col.name)) {
                return {SQLType::CREATE_TABLE, "错误：无效的列名：" + col.name, false};
            }
            
            columnDefs.push_back({col.name, type, col.isPrimary});
        }
        
        // 创建表
        if (db->createTable(tableName, columnDefs)) {
            return {SQLType::CREATE_TABLE, "表 " + tableName + " 创建成功", true};
        } else {
            return {SQLType::CREATE_TABLE, "错误：创建表失败，表可能已存在", false};
        }
    } else {
        return {SQLType::CREATE_TABLE, "错误：CREATE TABLE 语法错误", false};
    }
}

SQLResult SQLParser::parseDropTable(const std::string& sql) {
    std::regex pattern(R"(drop\s+table\s+(\w+))");
    std::smatch matches;
    
    if (std::regex_search(sql, matches, pattern) && matches.size() > 1) {
        std::string tableName = matches[1].str();
        
        // 获取当前数据库
        auto db = DBManager::getInstance().getCurrentDatabase();
        if (!db) {
            return {SQLType::DROP_TABLE, "错误：未选择数据库", false};
        }
        
        // 删除表
        if (db->dropTable(tableName)) {
            return {SQLType::DROP_TABLE, "表 " + tableName + " 删除成功", true};
        } else {
            return {SQLType::DROP_TABLE, "错误：删除表失败，表可能不存在", false};
        }
    } else {
        return {SQLType::DROP_TABLE, "错误：DROP TABLE 语法错误", false};
    }
}

SQLResult SQLParser::parseInsert(const std::string& sql) {
    std::regex pattern(R"(insert\s+(\w+)\s+values\s*\((.*)\))");
    std::smatch matches;
    
    if (std::regex_search(sql, matches, pattern) && matches.size() > 2) {
        std::string tableName = matches[1].str();
        std::string valueStr = matches[2].str();
        
        // 获取当前数据库
        auto db = DBManager::getInstance().getCurrentDatabase();
        if (!db) {
            return {SQLType::INSERT, "错误：未选择数据库", false};
        }
        
        // 获取表
        auto table = db->getTable(tableName);
        if (!table) {
            return {SQLType::INSERT, "错误：表 " + tableName + " 不存在", false};
        }
        
        // 解析值列表
        auto valueList = parseValues(valueStr);
        if (valueList.empty()) {
            return {SQLType::INSERT, "错误：无效的值列表", false};
        }
        
        // 获取表的列定义
        const auto& columns = table->getColumns();
        if (valueList.size() != columns.size()) {
            return {SQLType::INSERT, "错误：值的数量与列的数量不匹配", false};
        }
        
        // 转换值类型
        std::vector<Value> values;
        for (size_t i = 0; i < valueList.size(); ++i) {
            try {
                values.push_back(stringToValue(valueList[i], columns[i].type));
            } catch (const std::exception& e) {
                return {SQLType::INSERT, "错误：无效的值：" + valueList[i], false};
            }
        }
        
        // 插入记录
        if (table->insert(values)) {
            return {SQLType::INSERT, "记录插入成功", true};
        } else {
            return {SQLType::INSERT, "错误：插入记录失败，主键可能重复", false};
        }
    } else {
        return {SQLType::INSERT, "错误：INSERT 语法错误", false};
    }
}

SQLResult SQLParser::parseDelete(const std::string& sql) {
    std::regex patternWithWhere(R"(delete\s+(\w+)\s+where\s+(.*))");
    std::regex patternWithoutWhere(R"(delete\s+(\w+))");
    std::smatch matches;
    
    std::string tableName;
    std::string whereClause;
    bool hasWhere = false;
    
    if (std::regex_search(sql, matches, patternWithWhere) && matches.size() > 2) {
        tableName = matches[1].str();
        whereClause = matches[2].str();
        hasWhere = true;
    } else if (std::regex_search(sql, matches, patternWithoutWhere) && matches.size() > 1) {
        tableName = matches[1].str();
        hasWhere = false;
    } else {
        return {SQLType::DELETE, "错误：DELETE 语法错误", false};
    }
    
    // 获取当前数据库
    auto db = DBManager::getInstance().getCurrentDatabase();
    if (!db) {
        return {SQLType::DELETE, "错误：未选择数据库", false};
    }
    
    // 获取表
    auto table = db->getTable(tableName);
    if (!table) {
        return {SQLType::DELETE, "错误：表 " + tableName + " 不存在", false};
    }
    
    int count = 0;
    if (hasWhere) {
        // 解析WHERE子句
        auto whereResult = parseWhereClause(whereClause);
        if (!whereResult.has_value()) {
            return {SQLType::DELETE, "错误：无效的 WHERE 子句", false};
        }
        
        auto [colName, op, valueStr] = whereResult.value();
        
        // 获取列类型
        auto columns = table->getColumns();
        int colIndex = -1;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == colName) {
                colIndex = static_cast<int>(i);
                break;
            }
        }
        
        if (colIndex == -1) {
            return {SQLType::DELETE, "错误：列 " + colName + " 不存在", false};
        }
        
        DataType colType = columns[colIndex].type;
        
        // 转换值类型
        Value value;
        try {
            value = stringToValue(valueStr, colType);
        } catch (const std::exception& e) {
            return {SQLType::DELETE, "错误：无效的值：" + valueStr, false};
        }
        
        // 删除记录
        count = table->deleteWhere(colName, op, value);
    } else {
        // 删除所有记录
        count = table->deleteWhere("", Operator::EQUAL, 0);
    }
    
    return {SQLType::DELETE, "已删除 " + std::to_string(count) + " 条记录", true};
}

SQLResult SQLParser::parseUpdate(const std::string& sql) {
    std::regex pattern(R"(update\s+(\w+)\s+set\s+(\w+)\s*=\s*([^,\s]+)(?:\s+where\s+(.*))?)", std::regex::icase);
    std::smatch matches;
    
    if (std::regex_search(sql, matches, pattern) && matches.size() > 3) {
        std::string tableName = matches[1].str();
        std::string setColName = matches[2].str();
        std::string setValueStr = matches[3].str();
        std::string whereClause = matches.size() > 4 ? matches[4].str() : "";
        bool hasWhere = !whereClause.empty();
        
        // 获取当前数据库
        auto db = DBManager::getInstance().getCurrentDatabase();
        if (!db) {
            return {SQLType::UPDATE, "错误：未选择数据库", false};
        }
        
        // 获取表
        auto table = db->getTable(tableName);
        if (!table) {
            return {SQLType::UPDATE, "错误：表 " + tableName + " 不存在", false};
        }
        
        // 获取设置列的类型
        auto columns = table->getColumns();
        int setColIndex = -1;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == setColName) {
                setColIndex = static_cast<int>(i);
                break;
            }
        }
        
        if (setColIndex == -1) {
            return {SQLType::UPDATE, "错误：列 " + setColName + " 不存在", false};
        }
        
        DataType setColType = columns[setColIndex].type;
        
        // 转换设置值类型
        Value setValue;
        try {
            setValue = stringToValue(setValueStr, setColType);
        } catch (const std::exception& e) {
            return {SQLType::UPDATE, "错误：无效的值：" + setValueStr, false};
        }
        
        int count = 0;
        if (hasWhere) {
            // 解析WHERE子句
            auto whereResult = parseWhereClause(whereClause);
            if (!whereResult.has_value()) {
                return {SQLType::UPDATE, "错误：无效的 WHERE 子句", false};
            }
            
            auto [whereColName, op, whereValueStr] = whereResult.value();
            
            // 获取WHERE列的类型
            int whereColIndex = -1;
            for (size_t i = 0; i < columns.size(); ++i) {
                if (columns[i].name == whereColName) {
                    whereColIndex = static_cast<int>(i);
                    break;
                }
            }
            
            if (whereColIndex == -1) {
                return {SQLType::UPDATE, "错误：列 " + whereColName + " 不存在", false};
            }
            
            DataType whereColType = columns[whereColIndex].type;
            
            // 转换WHERE值类型
            Value whereValue;
            try {
                whereValue = stringToValue(whereValueStr, whereColType);
            } catch (const std::exception& e) {
                return {SQLType::UPDATE, "错误：无效的值：" + whereValueStr, false};
            }
            
            // 更新记录
            count = table->updateWhere(setColName, setValue, whereColName, op, whereValue);
        } else {
            // 更新所有记录
            count = table->updateWhere(setColName, setValue, "", Operator::EQUAL, 0);
        }
        
        return {SQLType::UPDATE, "已更新 " + std::to_string(count) + " 条记录", true};
    } else {
        return {SQLType::UPDATE, "错误：UPDATE 语法错误", false};
    }
}

SQLResult SQLParser::parseSelect(const std::string& sql) {
    std::regex patternWithWhere(R"(select\s+(\S+)\s+from\s+(\w+)\s+where\s+(.*))", std::regex::icase);
    std::regex patternWithoutWhere(R"(select\s+(\S+)\s+from\s+(\w+))", std::regex::icase);
    std::smatch matches;
    
    std::string selectCol;
    std::string tableName;
    std::string whereClause;
    bool hasWhere = false;
    
    if (std::regex_search(sql, matches, patternWithWhere) && matches.size() > 3) {
        selectCol = matches[1].str();
        tableName = matches[2].str();
        whereClause = matches[3].str();
        hasWhere = true;
    } else if (std::regex_search(sql, matches, patternWithoutWhere) && matches.size() > 2) {
        selectCol = matches[1].str();
        tableName = matches[2].str();
        hasWhere = false;
    } else {
        return {SQLType::SELECT, "错误：SELECT 语法错误", false};
    }
    
    // 获取当前数据库
    auto db = DBManager::getInstance().getCurrentDatabase();
    if (!db) {
        return {SQLType::SELECT, "错误：未选择数据库", false};
    }
    
    // 获取表
    auto table = db->getTable(tableName);
    if (!table) {
        return {SQLType::SELECT, "错误：表 " + tableName + " 不存在", false};
    }
    
    // 获取表的列定义
    const auto& columns = table->getColumns();
    
    std::vector<Record> result;
    if (hasWhere) {
        // 解析WHERE子句
        auto whereResult = parseWhereClause(whereClause);
        if (!whereResult.has_value()) {
            return {SQLType::SELECT, "错误：无效的 WHERE 子句", false};
        }
        
        auto [colName, op, valueStr] = whereResult.value();
        
        // 获取列类型
        int colIndex = -1;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i].name == colName) {
                colIndex = static_cast<int>(i);
                break;
            }
        }
        
        if (colIndex == -1) {
            return {SQLType::SELECT, "错误：列 " + colName + " 不存在", false};
        }
        
        DataType colType = columns[colIndex].type;
        
        // 转换值类型
        Value value;
        try {
            value = stringToValue(valueStr, colType);
        } catch (const std::exception& e) {
            return {SQLType::SELECT, "错误：无效的值：" + valueStr, false};
        }
        
        // 查询记录
        result = table->selectWhere(colName, op, value, selectCol);
    } else {
        // 查询所有记录
        result = table->selectAll(selectCol);
    }
    
    // 构建结果消息
    std::stringstream ss;
    ss << "查询结果：" << result.size() << " 条记录" << std::endl;
    
    // 如果有结果，显示结果
    if (!result.empty()) {
        // 显示列名
        if (selectCol == "*") {
            for (size_t i = 0; i < columns.size(); ++i) {
                ss << columns[i].name;
                if (i < columns.size() - 1) {
                    ss << "\t";
                }
            }
        } else {
            ss << selectCol;
        }
        ss << std::endl;
        
        // 显示分隔线
        if (selectCol == "*") {
            for (size_t i = 0; i < columns.size(); ++i) {
                ss << "--------";
                if (i < columns.size() - 1) {
                    ss << "\t";
                }
            }
        } else {
            ss << "--------";
        }
        ss << std::endl;
        
        // 显示记录
        for (const auto& record : result) {
            for (size_t i = 0; i < record.size(); ++i) {
                if (std::holds_alternative<int>(record[i])) {
                    ss << std::get<int>(record[i]);
                } else {
                    ss << std::get<std::string>(record[i]);
                }
                if (i < record.size() - 1) {
                    ss << "\t";
                }
            }
            ss << std::endl;
        }
    }
    
    return {SQLType::SELECT, ss.str(), true};
}

std::optional<std::tuple<std::string, Operator, std::string>> SQLParser::parseWhereClause(const std::string& whereClause) {
    std::regex pattern(R"((\w+)\s*([=<>])\s*([^,\s]+))");
    std::smatch matches;
    
    if (std::regex_search(whereClause, matches, pattern) && matches.size() > 3) {
        std::string colName = matches[1].str();
        std::string op = matches[2].str();
        std::string value = matches[3].str();
        
        return std::make_tuple(colName, stringToOperator(op), value);
    }
    
    return std::nullopt;
}

std::vector<std::string> SQLParser::parseValues(const std::string& values) {
    std::vector<std::string> result;
    std::string current;
    bool inQuotes = false;
    
    for (size_t i = 0; i < values.length(); ++i) {
        char c = values[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
            if (!inQuotes) {
                // 引号结束，添加当前字符串到结果
                result.push_back(current);
                current.clear();
            }
        } else if (c == ',' && !inQuotes) {
            // 逗号分隔符，如果不在引号内，则添加当前字符串到结果
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else if (!inQuotes && (c == ' ' || c == '\t')) {
            // 忽略引号外的空白字符
            continue;
        } else {
            // 添加字符到当前字符串
            current += c;
        }
    }
    
    // 添加最后一个字符串（如果有）
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}

std::vector<CreateTableColumn> SQLParser::parseColumnDefs(const std::string& columnDefs) {
    std::vector<CreateTableColumn> result;
    std::regex pattern(R"((\w+)\s+(\w+)(?:\s+primary)?)");
    std::sregex_iterator it(columnDefs.begin(), columnDefs.end(), pattern);
    std::sregex_iterator end;
    
    while (it != end) {
        std::smatch match = *it;
        if (match.size() > 2) {
            std::string name = match[1].str();
            std::string type = match[2].str();
            bool isPrimary = match.str().find("primary") != std::string::npos;
            
            result.push_back({name, type, isPrimary});
        }
        ++it;
    }
    
    return result;
}

Operator SQLParser::stringToOperator(const std::string& op) {
    if (op == "=") {
        return Operator::EQUAL;
    } else if (op == "<") {
        return Operator::LESS_THAN;
    } else if (op == ">") {
        return Operator::GREATER_THAN;
    } else {
        throw std::invalid_argument("无效的操作符：" + op);
    }
}

DataType SQLParser::stringToDataType(const std::string& type) {
    std::string lowerType = toLower(type);
    if (lowerType == "int") {
        return DataType::INT;
    } else if (lowerType == "string") {
        return DataType::STRING;
    } else {
        throw std::invalid_argument("无效的数据类型：" + type);
    }
}

std::string SQLParser::extractQuotedString(const std::string& str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

bool SQLParser::isValidIdentifier(const std::string& identifier) {
    if (identifier.empty()) {
        return false;
    }
    
    // 标识符只能包含字母、数字和下划线，且不能以数字开头
    if (!std::isalpha(identifier[0])) {
        return false;
    }
    
    for (char c : identifier) {
        if (!std::isalnum(c) && c != '_') {
            return false;
        }
    }
    
    return true;
}

} // namespace minidb 