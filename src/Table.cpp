#include "../include/Table.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

namespace minidb {

Table::Table(const std::string& name, const std::string& dbName, 
             const std::vector<ColumnDef>& columns)
    : name_(name), dbName_(dbName), columns_(columns), 
      tablePath_("./data/" + dbName + "/" + name + ".dat") {
    
    // 查找主键列
    for (size_t i = 0; i < columns_.size(); ++i) {
        if (columns_[i].isPrimary) {
            primaryKeyCol_ = i;
            break;
        }
    }
}

Table::~Table() {
    // 保存表数据
    saveData();
}

bool Table::insert(const std::vector<Value>& values) {
    try {
        // 检查值的数量是否与列数量匹配
        if (values.size() != columns_.size()) {
            return false;
        }
        
        // 检查值的类型是否与列类型匹配
        for (size_t i = 0; i < values.size(); ++i) {
            bool isInt = std::holds_alternative<int>(values[i]);
            bool isString = std::holds_alternative<std::string>(values[i]);
            
            if ((columns_[i].type == DataType::INT && !isInt) ||
                (columns_[i].type == DataType::STRING && !isString)) {
                return false;
            }
        }
        
        // 检查主键唯一性（如果有主键）
        if (primaryKeyCol_.has_value()) {
            const Value& pkValue = values[primaryKeyCol_.value()];
            
            // 使用索引查找（如果有索引）
            if (index_) {
                std::vector<size_t> result = index_->find(pkValue, Operator::EQUAL);
                if (!result.empty()) {
                    return false;  // 主键已存在
                }
            } else {
                // 没有索引，线性扫描
                for (size_t i = 0; i < records_.size(); ++i) {
                    if (compareValues(records_[i][primaryKeyCol_.value()], pkValue, Operator::EQUAL)) {
                        return false;  // 主键已存在
                    }
                }
            }
        }
        
        // 添加记录
        size_t rowId = records_.size();
        records_.push_back(values);
        
        // 更新索引（如果有主键）
        if (primaryKeyCol_.has_value() && index_) {
            index_->insert(values[primaryKeyCol_.value()], rowId);
        }
        
        // 保存表数据
        saveData();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "插入记录失败: " << e.what() << std::endl;
        return false;
    }
}

int Table::deleteWhere(const std::string& colName, Operator op, const Value& value) {
    try {
        // 获取列索引
        auto colIndex = getColumnIndex(colName);
        if (!colIndex.has_value()) {
            return 0;
        }
        
        // 查找要删除的记录
        std::vector<size_t> deleteIndices;
        
        // 使用索引查找（如果可以）
        if (colIndex == primaryKeyCol_ && index_ && op == Operator::EQUAL) {
            deleteIndices = index_->find(value, op);
        } else {
            // 线性扫描
            for (size_t i = 0; i < records_.size(); ++i) {
                if (matchCondition(records_[i], colIndex.value(), op, value)) {
                    deleteIndices.push_back(i);
                }
            }
        }
        
        // 如果没有找到匹配的记录，返回0
        if (deleteIndices.empty()) {
            return 0;
        }
        
        // 从大到小排序索引，以便正确删除
        std::sort(deleteIndices.begin(), deleteIndices.end(), std::greater<size_t>());
        
        // 删除记录
        for (size_t idx : deleteIndices) {
            // 如果有索引，先从索引中删除
            if (primaryKeyCol_.has_value() && index_) {
                index_->remove(records_[idx][primaryKeyCol_.value()]);
            }
            
            // 删除记录
            records_.erase(records_.begin() + idx);
        }
        
        // 保存表数据
        saveData();
        
        // 返回删除的记录数
        return static_cast<int>(deleteIndices.size());
    } catch (const std::exception& e) {
        std::cerr << "删除记录失败: " << e.what() << std::endl;
        return 0;
    }
}

int Table::updateWhere(const std::string& setColName, const Value& setValue, 
                        const std::string& whereColName, Operator op, const Value& whereValue) {
    try {
        // 获取列索引
        auto setColIndex = getColumnIndex(setColName);
        auto whereColIndex = getColumnIndex(whereColName);
        
        if (!setColIndex.has_value() || !whereColIndex.has_value()) {
            return 0;
        }
        
        // 检查设置值的类型是否与列类型匹配
        bool isInt = std::holds_alternative<int>(setValue);
        bool isString = std::holds_alternative<std::string>(setValue);
        
        if ((columns_[setColIndex.value()].type == DataType::INT && !isInt) ||
            (columns_[setColIndex.value()].type == DataType::STRING && !isString)) {
            return 0;
        }
        
        // 查找要更新的记录
        std::vector<size_t> updateIndices;
        
        // 使用索引查找（如果可以）
        if (whereColIndex == primaryKeyCol_ && index_ && op == Operator::EQUAL) {
            updateIndices = index_->find(whereValue, op);
        } else {
            // 线性扫描
            for (size_t i = 0; i < records_.size(); ++i) {
                if (matchCondition(records_[i], whereColIndex.value(), op, whereValue)) {
                    updateIndices.push_back(i);
                }
            }
        }
        
        // 如果没有找到匹配的记录，返回0
        if (updateIndices.empty()) {
            return 0;
        }
        
        // 更新记录
        for (size_t idx : updateIndices) {
            // 如果更新的是主键列，需要先从索引中删除旧值
            if (setColIndex == primaryKeyCol_ && index_) {
                index_->remove(records_[idx][setColIndex.value()]);
            }
            
            // 更新记录
            records_[idx][setColIndex.value()] = setValue;
            
            // 如果更新的是主键列，需要重新添加到索引
            if (setColIndex == primaryKeyCol_ && index_) {
                index_->insert(setValue, idx);
            }
        }
        
        // 保存表数据
        saveData();
        
        // 返回更新的记录数
        return static_cast<int>(updateIndices.size());
    } catch (const std::exception& e) {
        std::cerr << "更新记录失败: " << e.what() << std::endl;
        return 0;
    }
}

std::vector<Record> Table::selectWhere(const std::string& colName, 
                                     Operator op, const Value& value, 
                                     const std::string& selectCol) {
    try {
        // 获取列索引
        auto colIndex = getColumnIndex(colName);
        auto selectColIndex = getColumnIndex(selectCol);
        
        if (!colIndex.has_value() || (!selectColIndex.has_value() && selectCol != "*")) {
            return {};
        }
        
        // 查找匹配的记录
        std::vector<size_t> matchIndices;
        
        // 使用索引查找（如果可以）
        if (colIndex == primaryKeyCol_ && index_ && op == Operator::EQUAL) {
            matchIndices = index_->find(value, op);
        } else {
            // 线性扫描
            for (size_t i = 0; i < records_.size(); ++i) {
                if (matchCondition(records_[i], colIndex.value(), op, value)) {
                    matchIndices.push_back(i);
                }
            }
        }
        
        // 提取结果
        std::vector<Record> result;
        for (size_t idx : matchIndices) {
            if (selectCol == "*") {
                // 返回所有列
                result.push_back(records_[idx]);
            } else {
                // 返回指定列
                result.push_back({records_[idx][selectColIndex.value()]});
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "查询记录失败: " << e.what() << std::endl;
        return {};
    }
}

std::vector<Record> Table::selectAll(const std::string& selectCol) {
    try {
        // 获取列索引
        std::optional<size_t> selectColIndex;
        if (selectCol != "*") {
            selectColIndex = getColumnIndex(selectCol);
            if (!selectColIndex.has_value()) {
                return {};
            }
        }
        
        // 提取结果
        std::vector<Record> result;
        for (const auto& record : records_) {
            if (selectCol == "*") {
                // 返回所有列
                result.push_back(record);
            } else {
                // 返回指定列
                result.push_back({record[selectColIndex.value()]});
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "查询记录失败: " << e.what() << std::endl;
        return {};
    }
}

bool Table::loadData() {
    try {
        // 如果表文件不存在，返回false
        if (!std::filesystem::exists(tablePath_)) {
            return false;
        }
        
        // 打开表文件
        std::ifstream tableFile(tablePath_, std::ios::binary);
        if (!tableFile.is_open()) {
            return false;
        }
        
        // 读取列定义数量
        size_t columnCount;
        tableFile.read(reinterpret_cast<char*>(&columnCount), sizeof(columnCount));
        
        // 读取列定义
        columns_.clear();
        for (size_t i = 0; i < columnCount; ++i) {
            // 读取列名长度
            size_t nameLength;
            tableFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
            
            // 读取列名
            std::string name(nameLength, '\0');
            tableFile.read(&name[0], nameLength);
            
            // 读取列类型
            int typeValue;
            tableFile.read(reinterpret_cast<char*>(&typeValue), sizeof(typeValue));
            DataType type = static_cast<DataType>(typeValue);
            
            // 读取是否为主键
            bool isPrimary;
            tableFile.read(reinterpret_cast<char*>(&isPrimary), sizeof(isPrimary));
            
            // 添加列定义
            columns_.push_back({name, type, isPrimary});
            
            // 如果是主键，设置主键列索引
            if (isPrimary) {
                primaryKeyCol_ = i;
            }
        }
        
        // 读取记录数量
        size_t recordCount;
        tableFile.read(reinterpret_cast<char*>(&recordCount), sizeof(recordCount));
        
        // 读取记录
        records_.clear();
        for (size_t i = 0; i < recordCount; ++i) {
            Record record;
            
            // 读取每列的值
            for (size_t j = 0; j < columnCount; ++j) {
                if (columns_[j].type == DataType::INT) {
                    // 读取整数值
                    int value;
                    tableFile.read(reinterpret_cast<char*>(&value), sizeof(value));
                    record.push_back(value);
                } else {
                    // 读取字符串值
                    size_t strLength;
                    tableFile.read(reinterpret_cast<char*>(&strLength), sizeof(strLength));
                    
                    std::string value(strLength, '\0');
                    tableFile.read(&value[0], strLength);
                    record.push_back(value);
                }
            }
            
            records_.push_back(record);
        }
        
        // 关闭文件
        tableFile.close();
        
        // 加载索引（如果有主键）
        if (primaryKeyCol_.has_value()) {
            createIndex();
            
            std::filesystem::path indexPath("./data/" + dbName_ + "/" + name_ + ".idx");
            if (std::filesystem::exists(indexPath) && index_) {
                index_->load(indexPath);
            } else {
                // 重建索引
                for (size_t i = 0; i < records_.size(); ++i) {
                    index_->insert(records_[i][primaryKeyCol_.value()], i);
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载表数据失败: " << e.what() << std::endl;
        return false;
    }
}

bool Table::saveData() const {
    try {
        // 确保表目录存在
        std::filesystem::path dbPath("./data/" + dbName_);
        if (!std::filesystem::exists(dbPath)) {
            std::filesystem::create_directory(dbPath);
        }
        
        // 打开表文件
        std::ofstream tableFile(tablePath_, std::ios::binary | std::ios::trunc);
        if (!tableFile.is_open()) {
            return false;
        }
        
        // 写入列定义数量
        size_t columnCount = columns_.size();
        tableFile.write(reinterpret_cast<const char*>(&columnCount), sizeof(columnCount));
        
        // 写入列定义
        for (const auto& column : columns_) {
            // 写入列名长度
            size_t nameLength = column.name.length();
            tableFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
            
            // 写入列名
            tableFile.write(column.name.c_str(), nameLength);
            
            // 写入列类型
            int typeValue = static_cast<int>(column.type);
            tableFile.write(reinterpret_cast<const char*>(&typeValue), sizeof(typeValue));
            
            // 写入是否为主键
            tableFile.write(reinterpret_cast<const char*>(&column.isPrimary), sizeof(column.isPrimary));
        }
        
        // 写入记录数量
        size_t recordCount = records_.size();
        tableFile.write(reinterpret_cast<const char*>(&recordCount), sizeof(recordCount));
        
        // 写入记录
        for (const auto& record : records_) {
            // 写入每列的值
            for (size_t i = 0; i < columnCount; ++i) {
                if (columns_[i].type == DataType::INT) {
                    // 写入整数值
                    int value = std::get<int>(record[i]);
                    tableFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
                } else {
                    // 写入字符串值
                    std::string value = std::get<std::string>(record[i]);
                    size_t strLength = value.length();
                    tableFile.write(reinterpret_cast<const char*>(&strLength), sizeof(strLength));
                    tableFile.write(value.c_str(), strLength);
                }
            }
        }
        
        // 关闭文件
        tableFile.close();
        
        // 保存索引（如果有）
        if (primaryKeyCol_.has_value() && index_) {
            std::filesystem::path indexPath("./data/" + dbName_ + "/" + name_ + ".idx");
            index_->save(indexPath);
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "保存表数据失败: " << e.what() << std::endl;
        return false;
    }
}

bool Table::createIndex() {
    if (primaryKeyCol_.has_value() && !index_) {
        index_ = std::make_unique<BTreeIndex>();
        
        // 为现有记录创建索引
        for (size_t i = 0; i < records_.size(); ++i) {
            index_->insert(records_[i][primaryKeyCol_.value()], i);
        }
        
        return true;
    }
    return false;
}

std::optional<size_t> Table::getColumnIndex(const std::string& colName) const {
    for (size_t i = 0; i < columns_.size(); ++i) {
        if (columns_[i].name == colName) {
            return i;
        }
    }
    return std::nullopt;
}

bool Table::matchCondition(const Record& record, size_t colIndex, Operator op, const Value& value) const {
    return compareValues(record[colIndex], value, op);
}

} // namespace minidb 