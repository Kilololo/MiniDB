#include "../include/Index.h"
#include <iostream>

namespace minidb {

bool BTreeIndex::insert(const Value& key, size_t rowId) {
    try {
        // 简化实现，使用map代替B树
        indexMap_[key] = rowId;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "插入索引失败: " << e.what() << std::endl;
        return false;
    }
}

bool BTreeIndex::remove(const Value& key) {
    try {
        // 从索引中删除键
        auto it = indexMap_.find(key);
        if (it != indexMap_.end()) {
            indexMap_.erase(it);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "删除索引失败: " << e.what() << std::endl;
        return false;
    }
}

std::vector<size_t> BTreeIndex::find(const Value& key, Operator op) {
    try {
        std::vector<size_t> result;
        
        switch (op) {
            case Operator::EQUAL: {
                // 查找等于key的记录
                auto it = indexMap_.find(key);
                if (it != indexMap_.end()) {
                    result.push_back(it->second);
                }
                break;
            }
            case Operator::LESS_THAN: {
                // 查找小于key的记录
                for (auto it = indexMap_.begin(); it != indexMap_.end(); ++it) {
                    if (compareValues(it->first, key, Operator::LESS_THAN)) {
                        result.push_back(it->second);
                    }
                }
                break;
            }
            case Operator::GREATER_THAN: {
                // 查找大于key的记录
                for (auto it = indexMap_.begin(); it != indexMap_.end(); ++it) {
                    if (compareValues(it->first, key, Operator::GREATER_THAN)) {
                        result.push_back(it->second);
                    }
                }
                break;
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "查找索引失败: " << e.what() << std::endl;
        return {};
    }
}

bool BTreeIndex::save(const std::filesystem::path& indexPath) const {
    try {
        // 打开索引文件
        std::ofstream indexFile(indexPath, std::ios::binary | std::ios::trunc);
        if (!indexFile.is_open()) {
            return false;
        }
        
        // 写入索引项数量
        size_t indexCount = indexMap_.size();
        indexFile.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
        
        // 写入索引项
        for (const auto& [key, rowId] : indexMap_) {
            // 写入键类型（0表示int，1表示string）
            bool isString = std::holds_alternative<std::string>(key);
            indexFile.write(reinterpret_cast<const char*>(&isString), sizeof(isString));
            
            if (isString) {
                // 写入字符串键
                std::string strKey = std::get<std::string>(key);
                size_t keyLength = strKey.length();
                indexFile.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
                indexFile.write(strKey.c_str(), keyLength);
            } else {
                // 写入整数键
                int intKey = std::get<int>(key);
                indexFile.write(reinterpret_cast<const char*>(&intKey), sizeof(intKey));
            }
            
            // 写入行ID
            indexFile.write(reinterpret_cast<const char*>(&rowId), sizeof(rowId));
        }
        
        // 关闭文件
        indexFile.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "保存索引失败: " << e.what() << std::endl;
        return false;
    }
}

bool BTreeIndex::load(const std::filesystem::path& indexPath) {
    try {
        // 如果索引文件不存在，返回false
        if (!std::filesystem::exists(indexPath)) {
            return false;
        }
        
        // 打开索引文件
        std::ifstream indexFile(indexPath, std::ios::binary);
        if (!indexFile.is_open()) {
            return false;
        }
        
        // 清空现有索引
        indexMap_.clear();
        
        // 读取索引项数量
        size_t indexCount;
        indexFile.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
        
        // 读取索引项
        for (size_t i = 0; i < indexCount; ++i) {
            // 读取键类型
            bool isString;
            indexFile.read(reinterpret_cast<char*>(&isString), sizeof(isString));
            
            Value key;
            if (isString) {
                // 读取字符串键
                size_t keyLength;
                indexFile.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
                
                std::string strKey(keyLength, '\0');
                indexFile.read(&strKey[0], keyLength);
                key = strKey;
            } else {
                // 读取整数键
                int intKey;
                indexFile.read(reinterpret_cast<char*>(&intKey), sizeof(intKey));
                key = intKey;
            }
            
            // 读取行ID
            size_t rowId;
            indexFile.read(reinterpret_cast<char*>(&rowId), sizeof(rowId));
            
            // 添加到索引
            indexMap_[key] = rowId;
        }
        
        // 关闭文件
        indexFile.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载索引失败: " << e.what() << std::endl;
        return false;
    }
}

} // namespace minidb 