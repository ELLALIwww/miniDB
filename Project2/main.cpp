#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "db.h"  // 包含数据库管理相关的头文件
#include "sql_parser.h"  // 包含SQL解析相关的头文件

using namespace std;

void executeSQL(const string& sql) {
    // 解析SQL命令
    Command command = parseSQL(sql);

    switch (command.type) {
    case CREATE_DATABASE:
        createDatabase(command.databaseName);
        break;
    case USE_DATABASE:
        useDatabase(command.databaseName);
        break;
    case CREATE_TABLE:
        createTable(command.tableName, command.columns);
        break;
    case INSERT:
        insertIntoTable(command.tableName, command.record);
        break;
    case SELECT:
        if (!command.joinTable.empty()) {
            selectWithJoin(command.tableName, command.joinTable, command.joinCondition, command.selectedColumns, command.whereClause);
        }
        else {
            selectFromTable(command.tableName, command.selectedColumns, command.whereClause);
        }
        break;
    case UPDATE:
        updateTable(command.tableName, command.updateFields, command.whereClause);
        break;
    case DELETE:
        deleteFromTable(command.tableName, command.whereClause);
        break;
    case DROP_TABLE:
        dropTable(command.tableName);
        break;
    default:
        cerr << "Unknown command: " << sql << endl;
        break;
    }
}


void convertTxtToCsv(const string& txtFile, const string& csvFile) {
    ifstream input(txtFile);
    ofstream output(csvFile);

    if (!input) {
        cerr << "Error opening input file: " << txtFile << endl;
        return;
    }

    if (!output) {
        cerr << "Error opening output file: " << csvFile << endl;
        return;
    }

    string line;
    while (getline(input, line)) {
        output << line << endl;
    }

    input.close();
    output.close();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./miniDB input.sql output.txt" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];
    string csvFile = outputFile.substr(0, outputFile.find_last_of('.')) + ".csv";

    // 打开输入SQL文件
    ifstream input(inputFile);
    if (!input) {
        cerr << "Error opening input file: " << inputFile << endl;
        return 1;
    }

    // 打开输出文件
    ofstream output(outputFile);
    if (!output) {
        cerr << "Error opening output file: " << outputFile << endl;
        return 1;
    }

    // 重定向标准输出到文件
    streambuf* coutBuf = cout.rdbuf();
    cout.rdbuf(output.rdbuf());

    // 读取文件中的每一行并执行SQL命令
    string line;
    while (getline(input, line)) {
        // 忽略空行
        if (line.empty()) continue;
        // 执行SQL命令
        executeSQL(line);
    }

    // 恢复标准输出
    cout.rdbuf(coutBuf);

    input.close();
    output.close();

    // 将 output.txt 转换为 output.csv
    convertTxtToCsv(outputFile, csvFile);

    return 0;
}



//source\repos\Project2\x64\Debug\minidb.exe C:\Users\91755\source\repos\Project2\x64\Debug\input.sql C:\Users\91755\source\repos\Project2\x64\Debug\output.txt
//输出需要加逗号
//txt转csv
//文件输出