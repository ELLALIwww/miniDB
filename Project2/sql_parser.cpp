#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "sql_parser.h"
using namespace std;

// �������������ַ���ת��ΪСд
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
        return str; // ����ԭʼ�ַ���
    }
    catch (const std::out_of_range& e) {
        return str; // ����ԭʼ�ַ���
    }
}

// ���� SQL ����
Command parseSQL(const string& sql) {
    Command command;
    command.type = UNKNOWN; // Ĭ������
    istringstream stream(sql);
    string token;
    int columnNumber = 0;

    while (stream >> token) {
        columnNumber += token.length() + 1; // �����кţ�+1��Ϊ�˿��ǿո�

        // ������ת��ΪСд�Ա��ڱȽ�
        //token = toLower(token);

        if (token == "CREATE") {
            stream >> token; // ��ȡ��һ������
            if (token == "DATABASE") {
                command.type = CREATE_DATABASE;
                stream >> command.databaseName; // ��ȡ���ݿ���
                if (command.databaseName.empty()) {
                    command.error = "Database name is missing.";
                    command.column = columnNumber;
                    return command;
                }
            }
            else if (token == "TABLE") {
                command.type = CREATE_TABLE;
                stream >> command.tableName; // ��ȡ����
                if (command.tableName.empty()) {
                    command.error = "Table name is missing.";
                    command.column = columnNumber;
                    return command;
                }

                // ��������������
                string columnDefinition;
                while (stream >> columnDefinition) {
                    string columnName;
                    if (columnDefinition[0] == '(') {
                        columnDefinition.erase(0, 1);
                    }
                    columnName = columnDefinition;
                    string columnType;
                    stream >> columnType; // ��ȡ������
                    int a = columnType.length();
                    if (columnType[a - 1] == ';') {
                        columnType.pop_back();
                        columnType.pop_back();
                        a = columnType.length();
                    }// ������
                    else if (columnType[a - 1] == ',') {
                        columnType.pop_back();
                        a = columnType.length();
                    }
                    if (columnType.empty()) {
                        command.error = "Column type is missing for column: " + columnName;
                        command.column = columnNumber;
                        return command;
                    }
                    command.columns.emplace_back(columnName, columnType); // ��������������
                }
            }
        }
        else if (token == "USE") {
            command.type = USE_DATABASE;
            stream >> command.databaseName;
            stream >> command.databaseName;// ��ȡ���ݿ���
            if (command.databaseName.empty()) {
                command.error = "Database name is missing.";
                command.column = columnNumber;
                return command;
            }
        }
        else if (token == "INSERT") {
            command.type = INSERT;
            stream >> token; // ��ȡ "into"
            if (token != "INTO") {
                command.error = "Expected 'into' after 'insert'.";
                command.column = columnNumber;
                return command;
            }
            stream >> command.tableName; // ��ȡ����
            if (command.tableName.empty()) {
                command.error = "Table name is missing.";
                command.column = columnNumber;
                return command;
            }

            // ������¼
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
                } // ������
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

                if (column == "FROM") break; // ���� "from" �ؼ���
                command.selectedColumns.push_back(column); // �����ѯ����
            }
            stream >> command.tableName; // ��ȡ����
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

            // ���� JOIN �Ӿ�
            string joinToken;
            while (stream >> joinToken) {
                if (joinToken == "INNER" || joinToken == "LEFT" || joinToken == "RIGHT" || joinToken == "FULL") {
                    stream >> joinToken; // ��ȡ "JOIN"
                    if (joinToken == "JOIN") {
                        stream >> command.joinTable; // ��ȡ JOIN ����
                        string onToken;
                        stream >> onToken; // ��ȡ "ON"
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
							//�������Ƿֺţ�ȥ�����ķֺ�
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
                    getline(stream, whereClause, ';'); // ��ȡ WHERE �Ӿ�ֱ���ֺ�

                    // ȥ��ǰ��Ŀո�
                    whereClause.erase(0, whereClause.find_first_not_of(" \t\n\r\f\v"));
                    whereClause.erase(whereClause.find_last_not_of(" \t\n\r\f\v") + 1);
                    command.whereClause = whereClause;
                    break;
                }
            }
        }

        else if (token == "UPDATE") {
            command.type = UPDATE;
            stream >> command.tableName; // ��ȡ����
            if (command.tableName.empty()) {
                command.error = "Table name is missing.";
                command.column = columnNumber;
                return command;
            }

            string setToken;
            stream >> setToken; // ��ȡ "SET"
            if (setToken != "SET") {
                command.error = "Expected 'SET' after table name.";
                command.column = columnNumber;
                return command;
            }

            // ���� SET �ֶ�
            string setFields;
            getline(stream, setFields, 'W'); // ��ȡ SET �ֶ�ֱ�� "WHERE"
            setFields.pop_back(); // ȥ�����Ŀո�
			// ȥ��ǰ�ߵĿո�
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

            // ���� WHERE �Ӿ�
            string whereClause;
            getline(stream, whereClause, ';'); // ��ȡ WHERE �Ӿ�ֱ���ֺ�
            whereClause.erase(0, whereClause.find_first_not_of(" \t\n\r\f\v"));
            whereClause.erase(whereClause.find_last_not_of(" \t\n\r\f\v") + 1);
            whereClause.erase(0, 5);
            command.whereClause = whereClause;
        }
        else if (token == "DELETE") {
            command.type = DELETE;
            stream >> token; // ��ȡ "FROM"
            if (token != "FROM") {
                command.error = "Expected 'FROM' after 'DELETE'.";
                command.column = columnNumber;
                return command;
            }
            stream >> command.tableName; // ��ȡ����
            if (command.tableName.empty()) {
                command.error = "Table name is missing.";
                command.column = columnNumber;
                return command;
            }

            // ���� WHERE �Ӿ�
            string condition;
            while (stream >> condition) {
                if (condition == "WHERE") {
                    string whereClause;
                    getline(stream, whereClause, ';'); // ��ȡ WHERE �Ӿ�ֱ���ֺ�

                    // ȥ��ǰ��Ŀո�
                    whereClause.erase(0, whereClause.find_first_not_of(" \t\n\r\f\v"));
                    whereClause.erase(whereClause.find_last_not_of(" \t\n\r\f\v") + 1);

                    command.whereClause = whereClause;
                    break;
                }
            }
        }
        else if (token == "DROP") {
            stream >> token; // ��ȡ��һ������
            if (token == "TABLE") {
                command.type = DROP_TABLE;
                stream >> command.tableName; // ��ȡ����
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




