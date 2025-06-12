# MiniDB - 微型数据库管理系统

这是一个基于C++实现的微型数据库管理系统，支持基本的SQL语句操作。

## 功能特性

- 数据存储：使用文件系统存储数据表和索引
- DDL支持：create/drop database, use, create/drop table
- DML支持：select, delete, insert, update
- 索引支持：自动为主键创建索引

## 编译运行

```bash
make
./minidb
```

## 项目结构

- `src/` - 源代码目录
- `include/` - 头文件目录
- `data/` - 数据存储目录
- `test/` - 测试代码目录
- `Makefile` - 编译配置文件 