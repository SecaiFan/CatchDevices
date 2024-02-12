#include "Pipe.h"
#include <stdio.h>
#include <strsafe.h>
#include <tchar.h>

BOOL Pipe::create() {
    p = CreateNamedPipe(
        p_name,             // pipe name 
        PIPE_ACCESS_DUPLEX,       // read/write access 
        PIPE_TYPE_MESSAGE |       // message type pipe 
        PIPE_READMODE_MESSAGE |   // message-read mode 
        PIPE_WAIT,                // blocking mode 
        PIPE_UNLIMITED_INSTANCES, // max. instances  
        p_size,                  // output buffer size 
        p_size,                  // input buffer size 
        0,                        // client time-out 
        NULL);
    if (p == INVALID_HANDLE_VALUE) {
        OutputDebugStringW(L"Handle Broken!\n");
        return FALSE;
    }
    else {
        OutputDebugStringW(L"Handle Allright!\n");
    }
    BOOL fCon = ConnectNamedPipe(p, NULL) ?
        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (fCon) OutputDebugStringW(L"Connection Succes!\n");
    return fCon;
}

BOOL Pipe::connect() {
    while (1)
    {
        p = CreateFile(
            p_name,   // pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE,
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file 

        // Break if the pipe handle is valid. 

        if (p != INVALID_HANDLE_VALUE) {
            _tprintf(TEXT("Pipe open.\n"));
            OutputDebugStringW(L"Pipe open.\n");
            break;
        }

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
        
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            _tprintf(TEXT("Could not open pipe. GLE=%d\n", GetLastError()));
            OutputDebugStringW(TEXT("Could not open pipe. GLE=%d\n", GetLastError()));
        }

        // All pipe instances are busy, so wait for 20 seconds. 

        if (!WaitNamedPipeW(p_name, 5000))
        {
            _tprintf(TEXT("Could not open pipe: 5 second wait timed out.\n"));
            OutputDebugStringW(L"Could not open pipe: 5 second wait timed out.\n");
            return FALSE;
        }
    }
    DWORD dwMode = PIPE_READMODE_MESSAGE;
    BOOL fSuccess = SetNamedPipeHandleState(
        p,    // pipe handle 
        &dwMode,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL);    // don't set maximum time 
    if (!fSuccess)
    {
        OutputDebugStringW(TEXT("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError()));
        return FALSE;
    }
    return fSuccess;
}

BOOL Pipe::writeG(LPCWSTR msg, BOOL isClient) {
    DWORD cbToWrite = (lstrlen(msg) + 1) * sizeof(TCHAR);
    DWORD cb = 0;
    BOOL fSuccess = WriteFile(
        p,                  // pipe handle 
        msg,             // message 
        cbToWrite,              // message length 
        &cb,             // bytes written 
        NULL);                  // not overlapped 

    if (!fSuccess)
    {
        printf("WriteFile to pipe failed. GLE=%d\n", GetLastError());
    }
    return fSuccess;
}

BOOL Pipe::readG(TCHAR* msg, BOOL isClient) {
    DWORD cb = 0;
    BOOL fSuccess = FALSE;
    TCHAR m[512];
   
    
    // Read from the pipe. 

    fSuccess = ReadFile(
        p,    // pipe handle 
        m,    // buffer to receive reply 
        p_size * sizeof(TCHAR),  // size of buffer 
        &cb,  // number of bytes read 
        NULL);    // not overlapped 
  
    if (!fSuccess || cb == 0)
    {
        if (isClient) printf("ReadFile from pipe failed. GLE=%d\n", GetLastError());
        else OutputDebugString(L"ReadFile from pipe failed.\n");
        return fSuccess;
    }
    if (isClient) _tprintf(TEXT("GUI: %s\n"), m);
    else OutputDebugString(m);
    return fSuccess;
}

//void Pipe::writeS(LPCWSTR msg) {
//
//}
//
//void Pipe::readS(LPCWSTR msg) {
//
//}

HANDLE Pipe::getHandle() {
    return p;
}

int Pipe::getSize() {
    return p_size;
}

Pipe::Pipe(LPCWSTR n) {
    HANDLE hHeap = GetProcessHeap();
    p_name = (TCHAR*)HeapAlloc(hHeap, 0, p_size * sizeof(TCHAR));
    StringCchCopy((LPTSTR)p_name, p_size, n);
	p = NULL;
}

void Pipe::getName() {
    _tprintf(TEXT("Name: %s\n"), p_name);
    OutputDebugString(p_name);
}

Pipe::~Pipe() {
	if(p) CloseHandle(p);
}
