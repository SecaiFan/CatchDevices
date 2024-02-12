#ifndef PIPE
#define PIPE
#include <Windows.h>
#include <WinBase.h>
#include <debugapi.h>
#endif

class Pipe
{
private:
	HANDLE p;
	LPCTSTR p_name;
	int p_size = 512;
public:
	BOOL create();
	BOOL connect();
	BOOL writeG(LPCWSTR msg, BOOL isClient);
	BOOL readG(TCHAR* msg, BOOL isClient);
	/*void writeS(LPCWSTR msg);
	void readS(LPCWSTR msg);*/
	HANDLE getHandle();
	int getSize();
	void getName();
	Pipe(LPCWSTR name);
	~Pipe();
};

