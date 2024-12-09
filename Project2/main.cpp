#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "db.h"  // �������ݿ������ص�ͷ�ļ�
#include "sql_parser.h"  // ����SQL������ص�ͷ�ļ�

using namespace std;

void executeSQL(const string& sql) {
    // ����SQL����
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

    // ������SQL�ļ�
    ifstream input(inputFile);
    if (!input) {
        cerr << "Error opening input file: " << inputFile << endl;
        return 1;
    }

    // ������ļ�
    ofstream output(outputFile);
    if (!output) {
        cerr << "Error opening output file: " << outputFile << endl;
        return 1;
    }

    // �ض����׼������ļ�
    streambuf* coutBuf = cout.rdbuf();
    cout.rdbuf(output.rdbuf());

    // ��ȡ�ļ��е�ÿһ�в�ִ��SQL����
    string line;
    while (getline(input, line)) {
        // ���Կ���
        if (line.empty()) continue;
        // ִ��SQL����
        executeSQL(line);
    }

    // �ָ���׼���
    cout.rdbuf(coutBuf);

    input.close();
    output.close();

    // �� output.txt ת��Ϊ output.csv
    convertTxtToCsv(outputFile, csvFile);

    return 0;
}



//source\repos\Project2\x64\Debug\minidb.exe C:\Users\91755\source\repos\Project2\x64\Debug\input.sql C:\Users\91755\source\repos\Project2\x64\Debug\output.txt
//�����Ҫ�Ӷ���
//txtתcsv
//�ļ����