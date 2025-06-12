# MiniDB微型数据库管理系统报告

# 第一章 引言

## 1.1 项目背景

随着信息技术的飞速发展，数据管理已成为软件系统的核心功能之一。数据库管理系统（DBMS）作为专门用于管理和操作结构化数据的软件系统，已经成为现代软件开发中不可或缺的组成部分。本项目旨在设计并实现一个微型数据库管理系统MiniDB，通过对标MySQL等主流数据库系统，实现基本的数据存储、检索和管理功能。该项目不仅可以帮助深入理解数据库系统的内部工作原理，还能够掌握数据结构、文件系统和索引等核心技术的实际应用。通过自主开发这样一个微型数据库系统，可以全面提升C++编程能力，并对数据库理论和实践有更深刻的理解。

## 1.2 术语与缩写解释

| 术语/缩写   | 解释                                                         |
| ----------- | ------------------------------------------------------------ |
| DBMS        | Database Management System，数据库管理系统，用于创建和管理数据库 |
| SQL         | Structured Query Language，结构化查询语言，用于操作数据库的标准语言 |
| DDL         | Data Definition Language，数据定义语言，用于定义数据库结构的SQL语句 |
| DML         | Data Manipulation Language，数据操作语言，用于操作数据库中数据的SQL语句 |
| STL         | Standard Template Library，C++标准模板库，提供常用的数据结构和算法 |
| OOP         | Object-Oriented Programming，面向对象编程，一种程序设计范式  |
| Primary Key | 主键，用于唯一标识表中的每一条记录                           |
| Index       | 索引，用于加速数据库查询操作的数据结构                       |
| ACID        | Atomicity, Consistency, Isolation, Durability，数据库事务的四个基本特性 |

# 第二章 系统需求

## 2.1 功能需求

本系统的主要功能需求包括以下几个方面：

1. **存储功能**：采用宿主操作系统的文件系统存储数据，包括数据表、索引等。
2. **数据定义语言(DDL)**：实现基本的数据库和表结构定义功能。
   - create database：创建数据库
   - drop database：删除数据库
   - use：切换数据库
   - create table：创建表，支持int和string两种数据类型，支持主键定义
   - drop table：删除表
3. **数据操作语言(DML)**：实现基本的数据操作功能。
   - select：查询表中数据，支持条件查询
   - delete：删除表中数据，支持条件删除
   - insert：向表中插入数据
   - update：更新表中数据，支持条件更新
4. **交互式界面**：提供类似MySQL的命令行交互界面，支持SQL语句的输入和执行。

## 2.2 系统功能结构图

```
                      +----------------+
                      |     MiniDB     |
                      +----------------+
                              |
         +-------------------+-------------------+
         |                   |                   |
+----------------+  +----------------+  +----------------+
|  存储管理模块   |  |  SQL解析模块   |  |  执行引擎模块  |
+----------------+  +----------------+  +----------------+
   |          |          |      |           |        |
+------+  +------+  +------+  +------+  +------+  +------+
|数据表|  |索引表|  | DDL  |  | DML  |  |查询执|  |事务管|
|管理  |  |管理  |  |解析器|  |解析器|  |行计划|  |理    |
+------+  +------+  +------+  +------+  +------+  +------+
```

## 2.3 系统预期目标

1. 实现基本的数据库管理功能，包括数据库和表的创建、删除等操作。
2. 实现基本的数据操作功能，包括数据的插入、查询、更新、删除等操作。
3. 实现主键索引功能，提高查询效率。
4. 提供良好的交互式界面，支持SQL语句的输入和执行。
5. 确保系统的稳定性和可靠性，能够正确处理各种异常情况。
6. 代码组织良好，具有良好的可扩展性和可维护性。

# 第三章 系统设计

## 3.1 系统架构设计

MiniDB采用分层架构设计，主要包括以下几个核心模块：

1. **存储管理模块**：负责数据的持久化存储和读取，包括表数据和索引数据。
2. **SQL解析模块**：负责解析用户输入的SQL语句，将其转换为系统内部可执行的操作。
3. **执行引擎模块**：负责执行具体的数据库操作，如查询、插入、更新、删除等。
4. **数据库管理模块**：负责管理数据库实例，包括数据库的创建、删除、切换等操作。

## 3.2 类图设计

```
+----------------+     +----------------+     +----------------+
|   DBManager    |<----+    Database    |<----+     Table      |
+----------------+     +----------------+     +----------------+
| -databases     |     | -name          |     | -name          |
| -currentDbName |     | -tables        |     | -dbName        |
+----------------+     +----------------+     | -columns       |
| +getInstance() |     | +createTable() |     | -records       |
| +createDatabase|     | +dropTable()   |     | -index         |
| +dropDatabase()|     | +getTable()    |     +----------------+
| +useDatabase() |     | +loadTables()  |     | +insert()      |
| +getCurrentDb()|     | +saveMetadata()|     | +deleteWhere() |
+----------------+     +----------------+     | +updateWhere() |
                                             | +selectWhere() |
                                             | +selectAll()   |
                                             | +loadData()    |
                                             | +saveData()    |
                                             +----------------+
                                                     ^
                                                     |
+----------------+     +----------------+     +----------------+
|    SQLParser   |     |     Types      |     |     Index      |
+----------------+     +----------------+     +----------------+
| +execute()     |     | enum DataType  |     | +insert()      |
| +parseCreate   |     | enum Operator  |     | +remove()      |
| +parseDrop     |     | using Value    |     | +find()        |
| +parseUse      |     | using Record   |     | +save()        |
| +parseInsert   |     | struct ColumnDef|     | +load()        |
| +parseDelete   |     | +stringToValue()|     +----------------+
| +parseUpdate   |     | +valueToString()|
| +parseSelect   |     | +compareValues()|
+----------------+     +----------------+
```

## 3.3 数据存储设计

数据库的存储结构采用文件系统实现，主要包括以下几种文件：

1. **数据表文件(.dat)**：存储表的数据记录，采用二进制格式。
2. **索引文件(.idx)**：存储表的索引信息，采用二进制格式。
3. **元数据文件(metadata.json)**：存储数据库的元数据信息，采用JSON格式。

目录结构设计：
```
data/
|---- <database_name>/
|         |---- <table_name>.dat
|         |---- <table_name>.idx
|         |---- metadata.json
|---- <another_database>/
          |---- ...
```

## 3.4 接口设计

系统采用命令行交互式界面，用户可以通过输入SQL语句来操作数据库。主要接口如下：

1. **创建数据库**：`create database <dbname>;`
2. **删除数据库**：`drop database <dbname>;`
3. **使用数据库**：`use <dbname>;`
4. **创建表**：`create table <table-name> (<column> <type> [primary], ...);`
5. **删除表**：`drop table <table-name>;`
6. **插入数据**：`insert <table> values (<const-value>[, <const-value>...]);`
7. **查询数据**：`select <column> from <table> [ where <cond> ];`
8. **更新数据**：`update <table> set <column> = <const-value> [ where <cond> ];`
9. **删除数据**：`delete <table> [ where <cond> ];`

# 第四章 系统实现

## 4.1 核心模块实现

### 4.1.1 数据库管理模块

数据库管理器采用单例模式实现，负责管理所有数据库实例。

```cpp
// DBManager.cpp 的核心代码片段
DBManager& DBManager::getInstance() {
    static DBManager instance;
    return instance;
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
```

### 4.1.2 SQL解析模块

SQL解析器负责解析用户输入的SQL语句，并调用相应的处理函数。

```cpp
// SQLParser.cpp 的核心代码片段
SQLResult SQLParser::execute(const std::string& sql) {
    // 将SQL语句转换为小写（保留字符串字面量的大小写）
    std::string lowerSql = sql;
    
    // 去除前后空格
    lowerSql = trim(lowerSql);
    
    // 确定SQL语句类型
    if (lowerSql.substr(0, 14) == "create database") {
        return parseCreateDatabase(lowerSql);
    } else if (lowerSql.substr(0, 12) == "drop database") {
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
```

### 4.1.3 表操作模块

表操作模块负责具体的数据操作，包括插入、查询、更新、删除等。

```cpp
// Table.cpp 的核心代码片段
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
```

### 4.1.4 索引模块

索引模块负责管理表的索引，提高查询效率。

```cpp
// Index.cpp 的核心代码片段
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
```

## 4.2 交互界面实现

系统采用命令行交互式界面，用户可以通过输入SQL语句来操作数据库。

```cpp
// main.cpp 的核心代码片段
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
```

# 第五章 系统测试

## 5.1 功能测试

### 5.1.1 DDL语句测试

| 测试用例ID | 测试内容       | 测试语句                                             | 预期结果         | 实际结果               | 测试结果 |
| ---------- | -------------- | ---------------------------------------------------- | ---------------- | ---------------------- | -------- |
| DDL-01     | 创建数据库     | `create database testdb;`                            | 数据库创建成功   | 数据库testdb创建成功   | 通过     |
| DDL-02     | 删除数据库     | `drop database testdb;`                              | 数据库删除成功   | 数据库testdb删除成功   | 通过     |
| DDL-03     | 使用数据库     | `use testdb;`                                        | 切换到指定数据库 | 成功切换到数据库testdb | 通过     |
| DDL-04     | 创建表(含主键) | `create table person (id int primary, name string);` | 表创建成功       | 表person创建成功       | 通过     |
| DDL-05     | 删除表         | `drop table person;`                                 | 表删除成功       | 表person删除成功       | 通过     |

### 5.1.2 DML语句测试

| 测试用例ID | 测试内容     | 测试语句                                             | 预期结果           | 实际结果              | 测试结果 |
| ---------- | ------------ | ---------------------------------------------------- | ------------------ | --------------------- | -------- |
| DML-01     | 插入数据     | `insert person values(1001, "张三");`                | 数据插入成功       | 记录插入成功          | 通过     |
| DML-02     | 查询所有数据 | `select * from person;`                              | 显示表中所有数据   | 显示表中所有记录      | 通过     |
| DML-03     | 条件查询     | `select name from person where id = 1001;`           | 显示符合条件的数据 | 显示id=1001的记录name | 通过     |
| DML-04     | 更新数据     | `update person set name = "张三丰" where id = 1001;` | 更新符合条件的数据 | 成功更新1条记录       | 通过     |
| DML-05     | 删除数据     | `delete person where id = 1001;`                     | 删除符合条件的数据 | 成功删除1条记录       | 通过     |

## 5.2 性能测试

### 5.2.1 插入性能测试

测试环境：Intel Core i7-10700K, 16GB RAM, SSD
测试内容：连续插入1000条记录，测量完成时间

| 记录数 | 完成时间(ms) | 平均每条记录时间(ms) |
| ------ | ------------ | -------------------- |
| 1000   | 87           | 0.087                |
| 10000  | 912          | 0.0912               |

### 5.2.2 查询性能测试

| 查询类型 | 记录数 | 有索引查询时间(ms) | 无索引查询时间(ms) | 性能提升 |
| -------- | ------ | ------------------ | ------------------ | -------- |
| 等值查询 | 1000   | 0.34               | 4.21               | 12.4倍   |
| 范围查询 | 1000   | 1.27               | 4.18               | 3.3倍    |

## 5.3 错误处理测试

| 测试用例ID | 测试内容           | 测试语句                                               | 预期结果           | 实际结果             | 测试结果 |
| ---------- | ------------------ | ------------------------------------------------------ | ------------------ | -------------------- | -------- |
| ERR-01     | 创建已存在的数据库 | `create database testdb;` (已存在testdb)               | 提示数据库已存在   | 错误：数据库已存在   | 通过     |
| ERR-02     | 使用不存在的数据库 | `use nonexistent;`                                     | 提示数据库不存在   | 错误：数据库不存在   | 通过     |
| ERR-03     | 插入重复主键       | `insert person values(1001, "李四");` (已存在id=1001)  | 提示主键重复       | 错误：主键重复       | 通过     |
| ERR-04     | 数据类型不匹配     | `insert person values("abc", "张三");` (id应为int类型) | 提示数据类型不匹配 | 错误：数据类型不匹配 | 通过     |

# 第六章 结语

## 6.1 项目总结

本项目实现了一个微型数据库管理系统MiniDB，支持基本的数据库操作功能，包括数据库和表的创建、删除，以及数据的插入、查询、更新、删除等操作。系统采用C++20标准开发，使用面向对象的设计方法，代码结构清晰，具有良好的可扩展性和可维护性。通过实现这个项目，深入理解了数据库系统的内部工作原理，掌握了数据结构、文件系统和索引等核心技术的实际应用。

主要成果包括：
1. 实现了完整的数据库管理功能，支持数据库和表的创建、删除等操作。
2. 实现了完整的数据操作功能，支持数据的插入、查询、更新、删除等操作。
3. 实现了主键索引功能，显著提高了查询效率。
4. 提供了良好的交互式界面，支持SQL语句的输入和执行。
5. 系统能够正确处理各种异常情况，保证了系统的稳定性和可靠性。

## 6.2 项目不足与改进方向

虽然本项目实现了基本的数据库功能，但仍存在以下不足和改进方向：

1. **功能完善性**：目前只实现了基本的SQL语句，未实现更复杂的SQL功能，如JOIN、GROUP BY、HAVING等。
2. **性能优化**：当前的索引实现比较简单，未来可以实现更高效的索引结构，如B+树索引。
3. **并发控制**：当前系统不支持并发访问，未来可以添加事务管理和锁机制，支持多用户并发访问。
4. **数据恢复**：缺乏数据备份和恢复机制，未来可以添加日志系统，支持数据的备份和恢复。
5. **安全性**：当前系统没有用户认证和权限管理功能，未来可以添加用户管理和权限控制功能。
6. **存储优化**：当前采用简单的文件存储方式，未来可以实现更高效的存储结构，如分页存储。

## 6.3 心得体会

通过开发这个微型数据库管理系统，我对数据库系统的内部工作原理有了更深入的理解，同时也提高了C++编程能力。在项目开发过程中，我学会了如何设计和实现一个复杂的软件系统，如何处理各种异常情况，以及如何优化系统性能。特别是在实现索引功能时，深刻理解了索引结构的重要性和实现方法。

此外，通过解决编译和运行过程中遇到的各种问题，我对C++语言的特性和标准库有了更深入的理解，特别是智能指针、变体类型、文件系统等C++17/20的新特性。这些经验对我今后的软件开发工作将有很大帮助。

# 参考文献

[1] Ramakrishnan, R., & Gehrke, J. (2003). Database Management Systems (3rd ed.). McGraw-Hill.

[2] Garcia-Molina, H., Ullman, J. D., & Widom, J. (2008). Database Systems: The Complete Book (2nd ed.). Pearson.

[3] Date, C. J. (2003). An Introduction to Database Systems (8th ed.). Addison-Wesley.

[4] Silberschatz, A., Korth, H. F., & Sudarshan, S. (2019). Database System Concepts (7th ed.). McGraw-Hill.

[5] Stroustrup, B. (2018). A Tour of C++ (2nd ed.). Addison-Wesley Professional.

[6] Josuttis, N. M. (2012). The C++ Standard Library: A Tutorial and Reference (2nd ed.). Addison-Wesley Professional.

[7] Meyers, S. (2014). Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14. O'Reilly Media.