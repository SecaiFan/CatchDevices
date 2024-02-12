#include "Sql.h"

void Sql::Query(char* q) {
    if (mysql_query(&con, q)) {
        OutputDebugString(L"Query Failed\n");
    }
}

Sql::Sql(char* db_name) {
    if(!mysql_init(&con)) OutputDebugString(L"Init Failed\n");
    if(!mysql_real_connect(&con,
        "localhost",
        "root",
        "1234",
        db_name,
        0,
        NULL,
        0)) OutputDebugString(L"Attach Failed\n");
}

Sql::~Sql() {

}