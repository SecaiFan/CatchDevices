#include "CppStudio.h"
#include <iostream>
#include <windows.h>
#include <mysql.h>
#include <string.h>

using namespace std;

//const SQLString host = "tcp://localhost:3306";
//const SQLString uname = "root@localhost";
//const SQLString pwd = "1234";

int main() {
    MYSQL conn;
    MYSQL_RES* res;
    MYSQL_ROW row;

    try {
        if (!mysql_init(&conn)) cout << "Init Failed" << endl;
        if (!mysql_real_connect(&conn,
            "localhost",
            "root",
            "1234",
            "test",
            0,
            NULL,
            0)) cout << "Attach Failed" << endl;
        else {
            cout << "Attach Success" << endl;
        }
        if(mysql_query(&conn, "SELECT * FROM test")) cout << "Query Failed" << endl;
        res = mysql_store_result(&conn);
        unsigned int fld_count = mysql_num_fields(res);
        for (int i = 0; i < res->row_count; i++) {
            row = mysql_fetch_row(res);
            for (int j = 0; j < fld_count; j++) {
                cout << row[j] << " | ";
            }
            cout << endl;
        }
        mysql_close(&conn);
    }
    catch(const exception& e) {
        cout << e.what() << endl;
    }
    return 0;
}