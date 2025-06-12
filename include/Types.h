#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>

namespace minidb {

// 数据类型枚举
enum class DataType {
    INT,
    STRING
};

// 操作符枚举
enum class Operator {
    EQUAL,
    LESS_THAN,
    GREATER_THAN
};

// 值类型
using Value = std::variant<int, std::string>;

// 记录类型
using Record = std::vector<Value>;

// 列定义
struct ColumnDef {
    std::string name;
    DataType type;
    bool isPrimary;
    
    ColumnDef(const std::string& name, DataType type, bool isPrimary = false)
        : name(name), type(type), isPrimary(isPrimary) {}
};

// 获取Value类型的值
template<typename T>
T getValue(const Value& value) {
    return std::get<T>(value);
}

// 将字符串转换为Value
Value stringToValue(const std::string& str, DataType type);

// 将Value转换为字符串
std::string valueToString(const Value& value);

// 比较两个Value
bool compareValues(const Value& left, const Value& right, Operator op);

} // namespace minidb 