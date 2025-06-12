#include "../include/Database.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace minidb {

Database::Database(const std::string& name) 
    : name_(name), dbPath_("./data/" + name) {
}

Database::~Database() {
    // 保存数据库元数据
    saveMetadata();
}

bool Database::createTable(const std::string& tableName, const std::vector<ColumnDef>& columns) {
    try {
        // 检查表名是否有效
        if (tableName.empty() || tableName.find(' ') != std::string::npos) {
            return false;
        }
        
        // 检查表是否已存在
        if (tables_.find(tableName) != tables_.end()) {
            return false;
        }
        
        // 检查列定义是否有效
        if (columns.empty()) {
            return false;
        }
        
        // 创建表对象
        auto table = std::make_shared<Table>(tableName, name_, columns);
        tables_[tableName] = table;
        
        // 创建索引（如果有主键）
        table->createIndex();
        
        // 保存表元数据
        return table->saveData();
    } catch (const std::exception& e) {
        std::cerr << "创建表失败: " << e.what() << std::endl;
        return false;
    }
}

bool Database::dropTable(const std::string& tableName) {
    try {
        // 检查表是否存在
        auto it = tables_.find(tableName);
        if (it == tables_.end()) {
            return false;
        }
        
        // 获取表路径
        std::filesystem::path tablePath = dbPath_ / (tableName + ".dat");
        std::filesystem::path indexPath = dbPath_ / (tableName + ".idx");
        
        // 删除表文件
        if (std::filesystem::exists(tablePath)) {
            std::filesystem::remove(tablePath);
        }
        
        // 删除索引文件
        if (std::filesystem::exists(indexPath)) {
            std::filesystem::remove(indexPath);
        }
        
        // 从数据库中移除表
        tables_.erase(it);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "删除表失败: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<Table> Database::getTable(const std::string& tableName) {
    auto it = tables_.find(tableName);
    if (it == tables_.end()) {
        return nullptr;
    }
    return it->second;
}

bool Database::loadTables() {
    try {
        // 确保数据库目录存在
        if (!std::filesystem::exists(dbPath_)) {
            return false;
        }
        
        // 遍历数据库目录下的所有.dat文件
        for (const auto& entry : std::filesystem::directory_iterator(dbPath_)) {
            if (entry.is_regular_file() && entry.path().extension() == ".dat") {
                std::string tableName = entry.path().stem().string();
                
                // 创建表对象并加载数据
                auto table = std::make_shared<Table>(tableName, name_, std::vector<ColumnDef>());
                if (table->loadData()) {
                    tables_[tableName] = table;
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载表失败: " << e.what() << std::endl;
        return false;
    }
}

bool Database::saveMetadata() const {
    try {
        // 保存数据库元数据（可以扩展添加更多元数据）
        std::filesystem::path metadataPath = dbPath_ / "metadata.json";
        std::ofstream metadataFile(metadataPath);
        if (!metadataFile.is_open()) {
            return false;
        }
        
        metadataFile << "{\"name\":\"" << name_ << "\",\"tables\":[";
        
        bool first = true;
        for (const auto& [tableName, _] : tables_) {
            if (!first) {
                metadataFile << ",";
            }
            metadataFile << "\"" << tableName << "\"";
            first = false;
        }
        
        metadataFile << "]}";
        metadataFile.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "保存数据库元数据失败: " << e.what() << std::endl;
        return false;
    }
}

} // namespace minidb 