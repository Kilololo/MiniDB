#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include "../include/DBManager.h"
#include "../include/SQLParser.h"

using namespace minidb;

void printWelcome() {
    std::cout << "欢迎使用MiniDB数据库管理系统" << std::endl;
    std::cout << "输入SQL语句执行操作，输入exit退出系统" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

void printPrompt() {
    std::string currentDb = DBManager::getInstance().getCurrentDatabaseName();
    if (currentDb.empty()) {
        std::cout << "MiniDB> ";
    } else {
        std::cout << "MiniDB [" << currentDb << "]> ";
    }
}

int main() {
    // 初始化数据库管理器
    if (!DBManager::getInstance().initDataDirectory()) {
        std::cerr << "无法初始化数据目录，程序退出" << std::endl;
        return 1;
    }
    
    // 加载现有数据库
    if (!DBManager::getInstance().loadDatabases()) {
        std::cerr << "加载数据库失败，程序退出" << std::endl;
        return 1;
    }
    
    // 打印欢迎信息
    printWelcome();
    
    std::string line;
    std::string sql;
    
    // 主循环
    while (true) {
        printPrompt();
        std::getline(std::cin, line);
        
        // 去除前后空格
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty()) {
            continue;
        }
        
        // 退出命令
        if (line == "exit" || line == "quit") {
            std::cout << "再见！" << std::endl;
            break;
        }
        
        // 清空命令
        if (line == "clear") {
            sql.clear();
            continue;
        }
        
        // 累加SQL语句
        sql += line;
        
        // 如果不以分号结尾，继续读取下一行
        if (sql.back() != ';') {
            sql += ' ';
            continue;
        }
        
        // 移除结尾分号
        sql.pop_back();
        
        // 执行SQL语句
        SQLResult result = SQLParser::execute(sql);
        
        // 显示执行结果
        std::cout << result.message << std::endl;
        
        // 清空SQL缓存
        sql.clear();
    }
    
    return 0;
} 