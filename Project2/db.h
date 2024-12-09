#ifndef DB_H
#define DB_H

#include <string>
#include <vector>
#include <utility> // for std::pair
using namespace std;

// 记录结构体
struct Record {
    vector<string> fields; // 存储记录的字段
};

// 表结构体
struct Table {
    string name;                  // 表名
    vector<string> columns;       // 列名
    vector<Record> records;       // 存储记录
};

// 数据库结构体
struct Database {
    string name;                  // 数据库名
    vector<Table> tables;         // 表集合
};

// 创建数据库
void createDatabase(const string& databaseName);

// 使用数据库
void useDatabase(const string& databaseName);

// 创建表
void createTable(const string& tableName, const vector<pair<string, string>>& columns);

// 插入数据
void insertIntoTable(const string& tableName, const Record& record);

// 查询数据
void selectFromTable(const string& tableName, const vector<string>& columns, const string& condition);

// 执行JOIN查询
void selectWithJoin(const string& tableName, const string& joinTable, const string& joinCondition, const vector<string>& columns, const string& condition);

// 更新数据
void updateTable(const string& tableName, const vector<pair<string, string>>& updateFields, const string& whereClause);

// 删除数据
void deleteFromTable(const string& tableName, const string& whereClause);

// 删除表
void dropTable(const string& tableName);

// 清理数据库
void cleanup();

#endif // DB_H




