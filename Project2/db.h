#ifndef DB_H
#define DB_H

#include <string>
#include <vector>
#include <utility> // for std::pair
using namespace std;

// ��¼�ṹ��
struct Record {
    vector<string> fields; // �洢��¼���ֶ�
};

// ��ṹ��
struct Table {
    string name;                  // ����
    vector<string> columns;       // ����
    vector<Record> records;       // �洢��¼
};

// ���ݿ�ṹ��
struct Database {
    string name;                  // ���ݿ���
    vector<Table> tables;         // ����
};

// �������ݿ�
void createDatabase(const string& databaseName);

// ʹ�����ݿ�
void useDatabase(const string& databaseName);

// ������
void createTable(const string& tableName, const vector<pair<string, string>>& columns);

// ��������
void insertIntoTable(const string& tableName, const Record& record);

// ��ѯ����
void selectFromTable(const string& tableName, const vector<string>& columns, const string& condition);

// ִ��JOIN��ѯ
void selectWithJoin(const string& tableName, const string& joinTable, const string& joinCondition, const vector<string>& columns, const string& condition);

// ��������
void updateTable(const string& tableName, const vector<pair<string, string>>& updateFields, const string& whereClause);

// ɾ������
void deleteFromTable(const string& tableName, const string& whereClause);

// ɾ����
void dropTable(const string& tableName);

// �������ݿ�
void cleanup();

#endif // DB_H




