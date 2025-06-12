-- 测试脚本
-- 创建数据库
create database testdb;

-- 使用数据库
use testdb;

-- 创建表
create table person (
    id int primary,
    name string
);

-- 插入数据
insert person values(1001, "张三");
insert person values(1002, "李四");
insert person values(1003, "王五");

-- 查询数据
select * from person;
select name from person where id = 1001;

-- 更新数据
update person set name = "张三丰" where id = 1001;
select * from person;

-- 删除数据
delete person where id = 1003;
select * from person;

-- 创建另一个表
create table student (
    studentid int primary,
    studentname string,
    age int
);

-- 插入数据
insert student values(2001, "赵六", 20);
insert student values(2002, "钱七", 21);
insert student values(2003, "孙八", 22);

-- 查询数据
select * from student;
select studentname from student where age > 20;

-- 删除表
drop table student;

-- 删除数据库
drop database testdb;

-- 退出
exit 