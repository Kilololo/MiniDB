#include "../include/DBManager.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace minidb {

DBManager::DBManager() : dataPath_("./data") {
}

DBManager::~DBManager() {
    // 确保所有数据都已保存
    for (auto& [name, db] : databases_) {
        db->saveMetadata();
    }
}

DBManager& DBManager::getInstance() {
    static DBManager instance;
    return instance;
}

bool DBManager::initDataDirectory() {
    try {
        // 如果数据目录不存在，创建它
        if (!std::filesystem::exists(dataPath_)) {
            std::filesystem::create_directory(dataPath_);
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "初始化数据目录失败: " << e.what() << std::endl;
        return false;
    }
}

bool DBManager::loadDatabases() {
    try {
        // 遍历数据目录下的所有子目录，每个子目录代表一个数据库
        for (const auto& entry : std::filesystem::directory_iterator(dataPath_)) {
            if (entry.is_directory()) {
                std::string dbName = entry.path().filename().string();
                databases_[dbName] = std::make_shared<Database>(dbName);
                databases_[dbName]->loadTables();
            }
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载数据库失败: " << e.what() << std::endl;
        return false;
    }
}

bool DBManager::createDatabase(const std::string& dbName) {
    try {
        // 检查数据库名称是否有效
        if (dbName.empty() || dbName.find(' ') != std::string::npos) {
            return false;
        }
        
        // 检查数据库是否已存在
        if (databases_.find(dbName) != databases_.end()) {
            return false;
        }
        
        // 创建数据库目录
        std::filesystem::path dbPath = dataPath_ / dbName;
        if (!std::filesystem::create_directory(dbPath)) {
            return false;
        }
        
        // 创建数据库对象
        databases_[dbName] = std::make_shared<Database>(dbName);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "创建数据库失败: " << e.what() << std::endl;
        return false;
    }
}

bool DBManager::dropDatabase(const std::string& dbName) {
    try {
        // 检查数据库是否存在
        auto it = databases_.find(dbName);
        if (it == databases_.end()) {
            return false;
        }
        
        // 从管理器中移除数据库
        databases_.erase(it);
        
        // 删除数据库目录
        std::filesystem::path dbPath = dataPath_ / dbName;
        std::filesystem::remove_all(dbPath);
        
        // 如果当前数据库是被删除的数据库，清空当前数据库
        if (currentDbName_ == dbName) {
            currentDbName_.clear();
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "删除数据库失败: " << e.what() << std::endl;
        return false;
    }
}

bool DBManager::useDatabase(const std::string& dbName) {
    // 检查数据库是否存在
    if (databases_.find(dbName) == databases_.end()) {
        return false;
    }
    
    // 设置当前数据库
    currentDbName_ = dbName;
    return true;
}

std::shared_ptr<Database> DBManager::getCurrentDatabase() const {
    if (currentDbName_.empty() || databases_.find(currentDbName_) == databases_.end()) {
        return nullptr;
    }
    return databases_.at(currentDbName_);
}

std::string DBManager::getCurrentDatabaseName() const {
    return currentDbName_;
}

} // namespace minidb 