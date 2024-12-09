#ifndef SQL_PARSER_H
#define SQL_PARSER_H

#include <string>
#include <vector>
#include <utility> // for std::pair
#include "db.h"
using namespace std;

enum CommandType {
    CREATE_DATABASE,
    USE_DATABASE,
    CREATE_TABLE,
    INSERT,
    SELECT,
    UPDATE,
    DELETE,
    DROP_TABLE,
    UNKNOWN
};

struct Command {
    CommandType type;
    string databaseName;
    string tableName;
    vector<pair<string, string>> columns;
    vector<string> selectedColumns;
    Record record;
    string error;
    int line;
    int column;
    string whereClause;
    string joinTable; // 新增字段
    string joinCondition; // 新增字段
    vector<pair<string, string>> updateFields;
};

Command parseSQL(const string& sql);

#endif // SQL_PARSER_H




