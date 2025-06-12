#include "../include/Types.h"

namespace minidb {

// 将字符串转换为Value
Value stringToValue(const std::string& str, DataType type) {
    if (type == DataType::INT) {
        return std::stoi(str);
    } else {
        return str;
    }
}

// 将Value转换为字符串
std::string valueToString(const Value& value) {
    if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else {
        return std::get<std::string>(value);
    }
}

// 比较两个Value
bool compareValues(const Value& left, const Value& right, Operator op) {
    if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
        int leftInt = std::get<int>(left);
        int rightInt = std::get<int>(right);
        
        switch (op) {
            case Operator::EQUAL: return leftInt == rightInt;
            case Operator::LESS_THAN: return leftInt < rightInt;
            case Operator::GREATER_THAN: return leftInt > rightInt;
        }
    } else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
        std::string leftStr = std::get<std::string>(left);
        std::string rightStr = std::get<std::string>(right);
        
        switch (op) {
            case Operator::EQUAL: return leftStr == rightStr;
            case Operator::LESS_THAN: return leftStr < rightStr;
            case Operator::GREATER_THAN: return leftStr > rightStr;
        }
    }
    
    return false;
}

} // namespace minidb 