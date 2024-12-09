#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // for std::find_if, std::remove_if
#include <sstream> // for std::stringstream
#include "db.h"
using namespace std;

static vector<Database> databases; // 存储所有数据库
static Database* currentDatabase = nullptr; // 当前数据库指针


// 创建数据库
void createDatabase(const string& databaseName) {
    // 检查数据库是否已存在
    if (find_if(databases.begin(), databases.end(), [&](const Database& db) {
        return db.name == databaseName;
        }) != databases.end()) {
        cerr << "Database " << databaseName << " already exists." << endl;
        return;
    }

    Database newDB;
    newDB.name = databaseName;
    databases.push_back(newDB);
}

// 使用数据库
void useDatabase(const string& databaseName) {
    auto it = find_if(databases.begin(), databases.end(), [&](const Database& db) {
        return db.name == databaseName;
        });

    if (it != databases.end()) {
        currentDatabase = &(*it);
    }
    else {
        cerr << "Database " << databaseName << " does not exist." << endl;
    }
}

// 创建表
void createTable(const string& tableName, const vector<pair<string, string>>& columns) {
    if (!currentDatabase) {
        cerr << "No database selected." << endl;
        return;
    }

    // 检查表是否已存在
    if (find_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == tableName;
        }) != currentDatabase->tables.end()) {
        cerr << "Table " << tableName << " already exists." << endl;
        return;
    }

    Table newTable;
    newTable.name = tableName;

    // 添加列名
    for (const auto& column : columns) {
        newTable.columns.push_back(column.first); // 只保存列名
    }

    currentDatabase->tables.push_back(newTable);
}

// 插入数据
void insertIntoTable(const string& tableName, const Record& record) {
    if (!currentDatabase) {
        cerr << "No database selected." << endl;
        return;
    }

    auto it = find_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == tableName;
        });

    if (it != currentDatabase->tables.end()) {
        // 检查记录字段数量是否与表列数匹配
        if (record.fields.size() != it->columns.size()) {
            cerr << "Record field count does not match table column count." << endl;
            return;
        }

        it->records.push_back(record);
    }
    else {
        cerr << "Table " << tableName << " does not exist." << endl;
    }
}

// 查询数据
// selectFromTable 函数中的修改
void selectFromTable(const string& tableName, const vector<string>& columns, const string& condition) {
    if (!currentDatabase) {
        cerr << "No database selected." << endl;
        return;
    }

    auto it = find_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == tableName;
        });

    if (it != currentDatabase->tables.end()) {
        // 检查是否查询所有列
        bool selectAll = (columns.size() == 1 && columns[0] == "*");

        // 打印表头
        if (selectAll) {
            for (size_t i = 0; i < it->columns.size(); ++i) {
                cout << it->columns[i];
                if (i < it->columns.size() - 1) cout << ",";
            }
        }
        else {
            for (size_t i = 0; i < columns.size(); ++i) {
                cout << columns[i];
                if (i < columns.size() - 1) cout << ",";
            }
        }
        cout << endl;

        // 解析条件
        vector<pair<string, pair<string, string>>> conditions;
        stringstream ss(condition);
        string line;
        getline(ss, line);
        vector<string> result;
        stringstream ss1(line);
        string token;
        while (getline(ss1, token, ' ')) {
            if (token == "AND" || token == "OR") {
                result.push_back(token);
            }
            else {
                if (!result.empty() && result.back() != "AND" && result.back() != "OR") {
                    result.back() += " " + token;
                }
                else {
                    result.push_back(token);
                }
            }
        }

        for (auto token : result) {
            if (token == "AND" || token == "OR") {
                conditions.push_back({ token, {"", ""} });
            }
            else {
                size_t pos = token.find_first_of("><=!");

                if (pos != string::npos) {
                    string column = token.substr(0, pos);
                    string op;
                    if (token[pos] == '>' || token[pos] == '<') {
                        if (token[pos + 1] == '=') {
                            op = token.substr(pos, 2);
                            pos++;
                        }
                        else {
                            op = token[pos];
                        }
                    }
                    else if (token[pos] == '!') {
                        if (token[pos + 1] == '=') {
                            op = token.substr(pos, 2);
                            pos++;
                        }
                    }
                    else {
                        op = token[pos];
                    }
                    string value = token.substr(pos + 1);
                    column.erase(remove_if(column.begin(), column.end(), ::isspace), column.end());
                    value.erase(remove_if(value.begin(), value.end(), ::isspace), value.end());
                    // 去掉分号
                    if (!value.empty() && value.back() == ';') {
                        value.pop_back();
                    }
                    conditions.push_back({ column, {op, value} });
                }
            }
        }

        // 打印记录
        for (const auto& record : it->records) {
            bool printRecord = true;
            bool lastConditionResult = true;
            for (size_t i = 0; i < conditions.size(); ++i) {
                if (conditions[i].first == "AND") {
                    if (!lastConditionResult) {
                        printRecord = false;
                        break;
                    }
                }
                else if (conditions[i].first == "OR") {
                    if (lastConditionResult) {
                        printRecord = true;
                        break;
                    }
                }
                else {
                    auto colIt = find(it->columns.begin(), it->columns.end(), conditions[i].first);
                    if (colIt != it->columns.end()) {
                        size_t index = distance(it->columns.begin(), colIt);
                        string recordValue = record.fields[index];
                        string conditionValue = conditions[i].second.second;
                        try {
                            double recordValueNum = stod(recordValue);
                            double conditionValueNum = stod(conditionValue);
                            if (conditions[i].second.first == ">") {
                                lastConditionResult = (recordValueNum > conditionValueNum);
                            }
                            else if (conditions[i].second.first == "<") {
                                lastConditionResult = (recordValueNum < conditionValueNum);
                            }
                            else if (conditions[i].second.first == "=") {
                                lastConditionResult = (recordValueNum == conditionValueNum);
                            }
                            else if (conditions[i].second.first == "!=") {
                                lastConditionResult = (recordValueNum != conditionValueNum);
                            }
                        }
                        catch (const invalid_argument&) {
                            if (conditions[i].second.first == ">") {
                                lastConditionResult = (recordValue > conditionValue);
                            }
                            else if (conditions[i].second.first == "<") {
                                lastConditionResult = (recordValue < conditionValue);
                            }
                            else if (conditions[i].second.first == "=") {
                                lastConditionResult = (recordValue == conditionValue);
                            }
                            else if (conditions[i].second.first == "!=") {
                                lastConditionResult = (recordValue != conditionValue);
                            }
                        }
                    }
                }
            }
            if (printRecord && lastConditionResult) {
                if (selectAll) {
                    for (size_t i = 0; i < record.fields.size(); ++i) {
                        cout << record.fields[i];
                        if (i < record.fields.size() - 1) cout << ",";
                    }
                }
                else {
                    for (size_t i = 0; i < columns.size(); ++i) {
                        auto colIt = find(it->columns.begin(), it->columns.end(), columns[i]);
                        if (colIt != it->columns.end()) {
                            size_t index = distance(it->columns.begin(), colIt);
                            cout << record.fields[index];
                        }
                        else {
                            cout << "NULL";
                        }
                        if (i < columns.size() - 1) cout << ",";
                    }
                }
                cout << endl;
            }
        }
        cout << "---------------------" << endl;
    }
    else {
        cerr << "Table " << tableName << " does not exist." << endl;
    }
}

// updateTable 函数中的修改
void updateTable(const string& tableName, const vector<pair<string, string>>& updateFields, const string& whereClause) {
    if (!currentDatabase) {
        cerr << "No database selected." << endl;
        return;
    }

    auto it = find_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == tableName;
        });

    if (it != currentDatabase->tables.end()) {
        for (auto& record : it->records) {
            size_t pos = whereClause.find('=');
            if (pos != string::npos) {
                string column = whereClause.substr(0, pos);
                string value = whereClause.substr(pos + 1);
                column.erase(remove_if(column.begin(), column.end(), ::isspace), column.end());
                value.erase(0, 1);

                auto colIt = find(it->columns.begin(), it->columns.end(), column);
                if (colIt != it->columns.end()) {
                    size_t index = distance(it->columns.begin(), colIt);
                    if (record.fields[index] == value) {
                        for (const auto& field : updateFields) {
                            auto updateColIt = find(it->columns.begin(), it->columns.end(), field.first);
                            if (updateColIt != it->columns.end()) {
                                size_t updateIndex = distance(it->columns.begin(), updateColIt);
                                record.fields[updateIndex] = field.second;
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        cerr << "Table " << tableName << " does not exist." << endl;
    }
}

// selectWithJoin 函数中的修改
void selectWithJoin(const string& tableName, const string& joinTable, const string& joinCondition, const vector<string>& columns, const string& condition) {
    if (!currentDatabase) {
        cerr << "No database selected." << endl;
        return;
    }

    auto it1 = find_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == tableName;
        });

    auto it2 = find_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == joinTable;
        });

    if (it1 == currentDatabase->tables.end()) {
        cerr << "Table " << tableName << " does not exist." << endl;
        return;
    }

    if (it2 == currentDatabase->tables.end()) {
        cerr << "Table " << joinTable << " does not exist." << endl;
        return;
    }

    // 解析JOIN条件
    size_t pos = joinCondition.find('=');
    if (pos == string::npos) {
        cerr << "Invalid JOIN condition." << endl;
        return;
    }

    string column1 = joinCondition.substr(0, pos);
    string column2 = joinCondition.substr(pos + 1);

    column1.erase(remove_if(column1.begin(), column1.end(), ::isspace), column1.end());
    column2.erase(remove_if(column2.begin(), column2.end(), ::isspace), column2.end());

    // 提取表名和列名
    size_t dotPos1 = column1.find('.');
    size_t dotPos2 = column2.find('.');

    if (dotPos1 == string::npos || dotPos2 == string::npos) {
        cerr << "Invalid JOIN columns format." << endl;
        return;
    }

    string table1 = column1.substr(0, dotPos1);
    string col1 = column1.substr(dotPos1 + 1);
    string table2 = column2.substr(0, dotPos2);
    string col2 = column2.substr(dotPos2 + 1);

    auto colIt1 = find(it1->columns.begin(), it1->columns.end(), col1);
    auto colIt2 = find(it2->columns.begin(), it2->columns.end(), col2);

    if (colIt1 == it1->columns.end() || colIt2 == it2->columns.end()) {
        cerr << "Invalid JOIN columns." << endl;
        return;
    }

    size_t index1 = distance(it1->columns.begin(), colIt1);
    size_t index2 = distance(it2->columns.begin(), colIt2);

    // 解析WHERE条件
    vector<pair<string, pair<string, string>>> conditions;
    stringstream ss(condition);
    string line;
    getline(ss, line);
    vector<string> result;
    stringstream ss1(line);
    string token;
    while (getline(ss1, token, ' ')) {
        if (token == "AND" || token == "OR") {
            result.push_back(token);
        }
        else {
            if (!result.empty() && result.back() != "AND" && result.back() != "OR") {
                result.back() += " " + token;
            }
            else {
                result.push_back(token);
            }
        }
    }

    for (auto token : result) {
        if (token == "AND" || token == "OR") {
            conditions.push_back({ token, {"", ""} });
        }
        else {
            size_t pos = token.find_first_of("><=!");

            if (pos != string::npos) {
                string column = token.substr(0, pos);
                string op;
                if (token[pos] == '>' || token[pos] == '<') {
                    if (token[pos + 1] == '=') {
                        op = token.substr(pos, 2);
                        pos++;
                    }
                    else {
                        op = token[pos];
                    }
                }
                else if (token[pos] == '!') {
                    if (token[pos + 1] == '=') {
                        op = token.substr(pos, 2);
                        pos++;
                    }
                }
                else {
                    op = token[pos];
                }
                string value = token.substr(pos + 1);
                column.erase(remove_if(column.begin(), column.end(), ::isspace), column.end());
                value.erase(0,1);
                // 去掉分号
                if (!value.empty() && value.back() == ';') {
                    value.pop_back();
                }
                conditions.push_back({ column, {op, value} });
            }
        }
    }

    // 打印表头
    for (size_t i = 0; i < columns.size(); ++i) {
        cout << columns[i];
        if (i < columns.size() - 1) cout << ",";
    }
    cout << endl;

    // 执行JOIN操作并打印结果
    for (const auto& record1 : it1->records) {
        for (const auto& record2 : it2->records) {
            if (record1.fields[index1] == record2.fields[index2]) {
                bool printRecord = true;
                bool lastConditionResult = true;
                for (size_t i = 0; i < conditions.size(); ++i) {
                    if (conditions[i].first == "AND") {
                        if (!lastConditionResult) {
                            printRecord = false;
                            break;
                        }
                    }
                    else if (conditions[i].first == "OR") {
                        if (lastConditionResult) {
                            printRecord = true;
                            break;
                        }
                    }
                    else {
                        auto tableColumn = conditions[i].first;
                        size_t dotPos = tableColumn.find('.');
                        if (dotPos != string::npos) {
                            string tableName = tableColumn.substr(0, dotPos);
                            string columnName = tableColumn.substr(dotPos + 1);

                            auto colIt1 = (tableName == it1->name) ? find(it1->columns.begin(), it1->columns.end(), columnName) : it1->columns.end();
                            auto colIt2 = (tableName == it2->name) ? find(it2->columns.begin(), it2->columns.end(), columnName) : it2->columns.end();

                            if (colIt1 != it1->columns.end() || colIt2 != it2->columns.end()) {
                                size_t index;
                                string recordValue;
                                if (colIt1 != it1->columns.end()) {
                                    index = distance(it1->columns.begin(), colIt1);
                                    recordValue = record1.fields[index];
                                }
                                else {
                                    index = distance(it2->columns.begin(), colIt2);
                                    recordValue = record2.fields[index];
                                }

                                string conditionValue = conditions[i].second.second;
                                try {
                                    double recordValueNum = stod(recordValue);
                                    double conditionValueNum = stod(conditionValue);
                                    if (conditions[i].second.first == ">") {
                                        lastConditionResult = (recordValueNum > conditionValueNum);
                                    }
                                    else if (conditions[i].second.first == "<") {
                                        lastConditionResult = (recordValueNum < conditionValueNum);
                                    }
                                    else if (conditions[i].second.first == "=") {
                                        lastConditionResult = (recordValueNum == conditionValueNum);
                                    }
                                    else if (conditions[i].second.first == "!=") {
                                        lastConditionResult = (recordValueNum != conditionValueNum);
                                    }
                                }
                                catch (const invalid_argument&) {
                                    if (conditions[i].second.first == ">") {
                                        lastConditionResult = (recordValue > conditionValue);
                                    }
                                    else if (conditions[i].second.first == "<") {
                                        lastConditionResult = (recordValue < conditionValue);
                                    }
                                    else if (conditions[i].second.first == "=") {
                                        lastConditionResult = (recordValue == conditionValue);
                                    }
                                    else if (conditions[i].second.first == "!=") {
                                        lastConditionResult = (recordValue != conditionValue);
                                    }
                                }
                            }
                        }
                    }
                }
                if (printRecord && lastConditionResult) {
                    for (size_t i = 0; i < columns.size(); ++i) {
                        string col = columns[i];
                        size_t dotPos = col.find('.');
                        if (dotPos != string::npos) {
                            string tableName = col.substr(0, dotPos);
                            string columnName = col.substr(dotPos + 1);

                            if (tableName == it1->name) {
                                auto colIt = find(it1->columns.begin(), it1->columns.end(), columnName);
                                if (colIt != it1->columns.end()) {
                                    size_t index = distance(it1->columns.begin(), colIt);
                                    cout << record1.fields[index];
                                }
                                else {
                                    cout << "NULL";
                                }
                            }
                            else if (tableName == it2->name) {
                                auto colIt = find(it2->columns.begin(), it2->columns.end(), columnName);
                                if (colIt != it2->columns.end()) {
                                    size_t index = distance(it2->columns.begin(), colIt);
                                    cout << record2.fields[index];
                                }
                                else {
                                    cout << "NULL";
                                }
                            }
                        }
                        if (i < columns.size() - 1) cout << ",";
                    }
                    cout << endl;

                }
            }
        }
    }
    cout << "---------------------" << endl;
}






// deleteFromTable 函数中的修改
void deleteFromTable(const string& tableName, const string& whereClause) {
    if (!currentDatabase) {
        cerr << "No database selected." << endl;
        return;
    }

    auto it = find_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == tableName;
        });

    if (it != currentDatabase->tables.end()) {
        auto& records = it->records;
        records.erase(remove_if(records.begin(), records.end(), [&](const Record& record) {
            // 假设 whereClause 是 "Name = 'Dave Brown'" 这样的格式
            size_t pos = whereClause.find('=');
            if (pos != string::npos) {
                string column = whereClause.substr(0, pos);
                string value = whereClause.substr(pos + 1);
                column.erase(remove_if(column.begin(), column.end(), ::isspace), column.end());
                value.erase(0, 1);
                // 不去掉引号

                auto colIt = find(it->columns.begin(), it->columns.end(), column);
                if (colIt != it->columns.end()) {
                    size_t index = distance(it->columns.begin(), colIt);
                    return record.fields[index] == value;
                }
            }
            return false;
            }), records.end());
    }
    else {
        cerr << "Table " << tableName << " does not exist." << endl;
    }
}

// dropTable 函数中的修改
void dropTable(const string& tableName) {
    if (!currentDatabase) {
        cerr << "No database selected." << endl;
        return;
    }

    auto it = remove_if(currentDatabase->tables.begin(), currentDatabase->tables.end(), [&](const Table& tbl) {
        return tbl.name == tableName;
        });

    if (it != currentDatabase->tables.end()) {
        currentDatabase->tables.erase(it, currentDatabase->tables.end());
    }
    else {
        cerr << "Table " << tableName << " does not exist." << endl;
    }
}


// 清理数据库
void cleanup() {
    databases.clear();
    currentDatabase = nullptr;
}




