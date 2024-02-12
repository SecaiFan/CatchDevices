#ifndef SQL
#define SQL
#include <Windows.h>
#include <WinBase.h>
#include <debugapi.h>
#include <iostream>
#include <mysql.h>
#include <string.h>
#endif
class Sql
{
private:
	MYSQL con;

public:
	void Query(char*);
	Sql(char*);
	~Sql();
};