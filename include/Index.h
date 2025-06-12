#pragma once

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "Types.h"

namespace minidb {

// 索引基类
class Index {
public:
    virtual ~Index() = default;
    
    // 插入索引
    virtual bool insert(const Value& key, size_t rowId) = 0;
    
    // 删除索引
    virtual bool remove(const Value& key) = 0;
    
    // 查找索引
    virtual std::vector<size_t> find(const Value& key, Operator op) = 0;
    
    // 保存索引
    virtual bool save(const std::filesystem::path& indexPath) const = 0;
    
    // 加载索引
    virtual bool load(const std::filesystem::path& indexPath) = 0;
};

// 二叉树索引实现
class BTreeIndex : public Index {
public:
    BTreeIndex() = default;
    ~BTreeIndex() = default;
    
    // 插入索引
    bool insert(const Value& key, size_t rowId) override;
    
    // 删除索引
    bool remove(const Value& key) override;
    
    // 查找索引
    std::vector<size_t> find(const Value& key, Operator op) override;
    
    // 保存索引
    bool save(const std::filesystem::path& indexPath) const override;
    
    // 加载索引
    bool load(const std::filesystem::path& indexPath) override;

private:
    std::map<Value, size_t> indexMap_; // 简化实现，使用map代替B树
};

} // namespace minidb 