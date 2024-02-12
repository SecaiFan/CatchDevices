#include "Pipe.h"
#include <stdio.h>

int main() {
	BOOL fSuccess = FALSE;
	Pipe mypipe = Pipe(L"\\\\.\\pipe\\userpipe");
	//Pipe pipe = Pipe(L"\\\\.\\pipe\\userpipe");
	mypipe.getName();
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, mypipe.getSize() * sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, mypipe.getSize() * sizeof(TCHAR));
	//mypipe.create();
	mypipe.connect();
    LPCWSTR lpvMessage = TEXT("Default message from client.");
	mypipe.writeG(lpvMessage, TRUE);
	do {
		fSuccess = mypipe.readG(pchRequest, TRUE);
	} while (!fSuccess);
	return 0;
}