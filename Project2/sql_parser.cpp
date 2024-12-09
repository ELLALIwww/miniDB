#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "sql_parser.h"
using namespace std;

// 辅助函数：将字符串转换为小写
string toLower(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

bool isNumber(const string& str) {
    stringstream ss(str);
    double d;
    char c;
    return (ss >> d) && !(ss >> c);
}

string formatToTwoDecimalPlaces(const string& str) {
    try {
        double number = stod(str);
        ostringstream out;
        out << fixed << setprecision(2) << number;
        return out.str();
    }
    catch (const std::invalid_argument& e) {
        return str; // 返回原始字符串
    }
    catch (const std::out_of_range& e) {
        return str; // 返回原始字符串
    }
}

// 解析 SQL 命令
Command parseSQL(const string& sql) {
    Command command;
    command.type = UNKNOWN; // 默认类型
    istringstream stream(sql);
    string token;
    int columnNumber = 0;

    while (stream >> token) {
        columnNumber += token.length() + 1; // 更新列号（+1是为了考虑空格）

        // 将命令转换为小写以便于比较
        //token = toLower(token);

        if (token == "CREATE") {
            stream >> token; // 读取下一个单词
            if (token == "DATABASE") {
                command.type = CREATE_DATABASE;
                stream >> command.databaseName; // 获取数据库名
                if (command.databaseName.empty()) {
                    command.error = "Database name is missing.";
                    command.column = columnNumber;
                    return command;
                }
            }
            else if (token == "TABLE") {
                command.type = CREATE_TABLE;
                stream >> command.tableName; // 获取表名
                if (command.tableName.empty()) {
                    command.error = "Table name is missing.";
                    command.column = columnNumber;
                    return command;
                }

                // 解析列名和类型
                string columnDefinition;
                while (stream >> columnDefinition) {
                    string columnName;
                    if (columnDefinition[0] == '(') {
                        columnDefinition.erase(0, 1);
                    }
                    columnName = columnDefinition;
                    string columnType;
                    stream >> columnType; // 获取列类型
                    int a = columnType.length();
                    if (columnType[a - 1] == ';') {
                        columnType.pop_back();
                        columnType.pop_back();
                        a = columnType.length();
                    }// 结束符
                    else if (columnType[a - 1] == ',') {
                        columnType.pop_back();
                        a = columnType.length();
                    }
                    if (columnType.empty()) {
                        command.error = "Column type is missing for column: " + columnName;
                        command.column = columnNumber;
                        return command;
                    }
                    command.columns.emplace_back(columnName, columnType); // 保存列名和类型
                }
            }
        }
        else if (token == "USE") {
            command.type = USE_DATABASE;
            stream >> command.databaseName;
            stream >> command.databaseName;// 获取数据库名
            if (command.databaseName.empty()) {
                command.error = "Database name is missing.";
                command.column = columnNumber;
                return command;
            }
        }
        else if (token == "INSERT") {
            command.type = INSERT;
            stream >> token; // 读取 "into"
            if (token != "INTO") {
                command.error = "Expected 'into' after 'insert'.";
                command.column = columnNumber;
                return command;
            }
            stream >> command.tableName; // 获取表名
            if (command.tableName.empty()) {
                command.error = "Table name is missing.";
                command.column = columnNumber;
                return command;
            }

            // 解析记录
            string recordStr;
            stream >> recordStr;
            while (stream >> token) {
                int a = token.length();
                if (token[0] == '(') {
                    token.erase(0, 1);
                    a = token.length();
                }
                if (token[0] == '\'') {
                    if (token[a - 1] == ';') {
                        token.pop_back();
                        token.pop_back();
                        a = token.length();
                    }
                    if (token[a - 1] == ',') {
                        token.pop_back();
                        a = token.length();
                    }
                    if (token[a - 1] != '\'') {
                        string TOKENplus;
                        while (1) {
                            stream >> TOKENplus;
                            int a = TOKENplus.length();
                            if (TOKENplus[a - 1] == ';') {
                                TOKENplus.pop_back();
                                TOKENplus.pop_back();
                                a = TOKENplus.length();
                            }
                            if (TOKENplus[a - 1] == ',') {
                                TOKENplus.pop_back();
                                a = TOKENplus.length();
                            }
                            token += ' ';
                            token += TOKENplus;
                            if (TOKENplus[a - 1] == '\'') {
                                break;
                            }
                        }
                    }
                }
                if (token[a - 1] == ';') {
                    token.pop_back();
                    token.pop_back();
                    a = token.length();
                    if (isNumber(token)) {
                        command.record.fields.push_back(formatToTwoDecimalPlaces(token));
                    }
                    else {
                        command.record.fields.push_back(token);
                    }
                    break;
                } // 结束符
                if (token[a - 1] == ',') {
                    token.pop_back();
                    a = token.length();
                }
                if (isNumber(token)) {
                    command.record.fields.push_back(formatToTwoDecimalPlaces(token));
                }
                else {
                    command.record.fields.push_back(token);
                }
            }
        }
        else if (token == "SELECT") {
            command.type = SELECT;
            std::string column;
            while (stream >> column) {
                int a = column.length();
                if (column[a - 1] == ',') {
                    column.pop_back();
                    a = column.length();
                }

                if (column == "FROM") break; // 到达 "from" 关键字
                command.selectedColumns.push_back(column); // 保存查询的列
            }
            stream >> command.tableName; // 获取表名
            int a = command.tableName.length();
            if (command.tableName[a - 1] == ';') {
                command.tableName.pop_back();
                a = command.tableName.length();
            }
            if (command.tableName.empty()) {
                command.error = "Table name is missing.";
                command.column = columnNumber;
                return command;
            }

            // 解析 JOIN 子句
            string joinToken;
            while (stream >> joinToken) {
                if (joinToken == "INNER" || joinToken == "LEFT" || joinToken == "RIGHT" || joinToken == "FULL") {
                    stream >> joinToken; // 读取 "JOIN"
                    if (joinToken == "JOIN") {
                        stream >> command.joinTable; // 获取 JOIN 表名
                        string onToken;
                        stream >> onToken; // 读取 "ON"
                        if (onToken == "ON") {
                            string joinConditionPart;
                            int count = 0;
                            while (count < 3 && stream >> joinConditionPart) {
                                if (!command.joinCondition.empty()) {
                                    command.joinCondition += " ";
                                }
                                command.joinCondition += joinConditionPart;
                                count++;
                            }
							//如果最后是分号，去掉最后的分号
							int a = command.joinCondition.length();
							if (command.joinCondition[a - 1] == ';') {
								command.joinCondition.pop_back();
								a = command.joinCondition.length();
							}
                        }
                    }
                }
                else if (joinToken == "WHERE") {
                    string whereClause;
                    getline(stream, whereClause, ';'); // 读取 WHERE 子句直到分号

                    // 去掉前后的空格
                    whereClause.erase(0, whereClause.find_first_not_of(" \t\n\r\f\v"));
                    whereClause.erase(whereClause.find_last_not_of(" \t\n\r\f\v") + 1);
                    command.whereClause = whereClause;
                    break;
                }
            }
        }

        else if (token == "UPDATE") {
            command.type = UPDATE;
            stream >> command.tableName; // 获取表名
            if (command.tableName.empty()) {
                command.error = "Table name is missing.";
                command.column = columnNumber;
                return command;
            }

            string setToken;
            stream >> setToken; // 读取 "SET"
            if (setToken != "SET") {
                command.error = "Expected 'SET' after table name.";
                command.column = columnNumber;
                return command;
            }

            // 解析 SET 字段
            string setFields;
            getline(stream, setFields, 'W'); // 读取 SET 字段直到 "WHERE"
            setFields.pop_back(); // 去掉最后的空格
			// 去掉前边的空格
            setFields.erase(0, setFields.find_first_not_of(" \t\n\r\f\v"));
            stringstream ss(setFields);
            string field;
            while (getline(ss, field, ',')) {
                size_t equalPos = field.find('=');
                string columnName = field.substr(0, equalPos);
                string value = field.substr(equalPos + 1);
                columnName.erase(remove_if(columnName.begin(), columnName.end(), ::isspace), columnName.end());
                value.erase(remove_if(value.begin(), value.end(), ::isspace), value.end());
				if (isNumber(value)) {
					value = formatToTwoDecimalPlaces(value);
				}
                command.updateFields.push_back(make_pair(columnName, value));
            }

            // 解析 WHERE 子句
            string whereClause;
            getline(stream, whereClause, ';'); // 读取 WHERE 子句直到分号
            whereClause.erase(0, whereClause.find_first_not_of(" \t\n\r\f\v"));
            whereClause.erase(whereClause.find_last_not_of(" \t\n\r\f\v") + 1);
            whereClause.erase(0, 5);
            command.whereClause = whereClause;
        }
        else if (token == "DELETE") {
            command.type = DELETE;
            stream >> token; // 读取 "FROM"
            if (token != "FROM") {
                command.error = "Expected 'FROM' after 'DELETE'.";
                command.column = columnNumber;
                return command;
            }
            stream >> command.tableName; // 获取表名
            if (command.tableName.empty()) {
                command.error = "Table name is missing.";
                command.column = columnNumber;
                return command;
            }

            // 解析 WHERE 子句
            string condition;
            while (stream >> condition) {
                if (condition == "WHERE") {
                    string whereClause;
                    getline(stream, whereClause, ';'); // 读取 WHERE 子句直到分号

                    // 去掉前后的空格
                    whereClause.erase(0, whereClause.find_first_not_of(" \t\n\r\f\v"));
                    whereClause.erase(whereClause.find_last_not_of(" \t\n\r\f\v") + 1);

                    command.whereClause = whereClause;
                    break;
                }
            }
        }
        else if (token == "DROP") {
            stream >> token; // 读取下一个单词
            if (token == "TABLE") {
                command.type = DROP_TABLE;
                stream >> command.tableName; // 获取表名
                if (command.tableName.empty()) {
                    command.error = "Table name is missing.";
                    command.column = columnNumber;
                    return command;
                }
            }
        }
    }
    return command;
}




