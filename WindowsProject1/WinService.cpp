#include <Windows.h>
#include <WinBase.h>
#include <debugapi.h>
#include <strsafe.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <tchar.h>
#include <fltUser.h>
#include <Pipe.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "NtDll.lib")
#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "FltLib.lib")

#define SVCNAME TEXT("SvcName")
#define PORT_NAME L"\\FltPort"

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent = NULL;
CM_NOTIFY_FILTER notFilter = { 0 };
Pipe pipe = Pipe(L"\\\\.\\pipe\\userpipe");

__callback VOID eventAction(
    _In_ HCMNOTIFICATION       hNotify,
    _In_opt_ PVOID             Context,
    _In_ CM_NOTIFY_ACTION      Action,
    _In_reads_bytes_(EventDataSize) PCM_NOTIFY_EVENT_DATA EventData,
    _In_ DWORD                 EventDataSize
    );

VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR*);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR*);
DWORD WINAPI threadFunc(LPVOID);


VOID eventAction(
    HCMNOTIFICATION hNotify, 
    PVOID Context,
    CM_NOTIFY_ACTION Action, 
    PCM_NOTIFY_EVENT_DATA EventData,
    DWORD EventDataSize) 
{
    WCHAR fsName[MAX_PATH] = L"";
    
    /*FILE_FULL_DIR_INFO fInfo = { 0 };
    WCHAR volName[MAX_PATH + 1] = L"";
    PSTORAGE_DEVICE_DESCRIPTOR pDevDesc = { 0 };

    DWORD cbBytesReturned, dwInBuffer = 0, dwOutBuffer = 0;
    
    BYTE byBuffer[1024];
    BOOL bSuccess = FALSE;
    PBYTE pbyInBuffer = { 0 }, pbyOutBuffer = { 0 };
    LPTSTR pszLogicalDrives, pszDriveRoot;*/

    switch (Action) {
    case CM_NOTIFY_ACTION_DEVICEINTERFACEARRIVAL:

        swprintf_s(fsName, L"Service: DeviceIsArrived: %s\n", EventData->u.DeviceInterface.SymbolicLink);
        OutputDebugStringW((LPWSTR)fsName);

        break;

    default:
        break;
    }

}

VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code. 

    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.

        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }

}

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler(
        SVCNAME,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        OutputDebugStringW(L"RegisterServiceCtrlHandler");
        return;
    }

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.


    CONFIGRET regRet = CR_DEFAULT;
    HCMNOTIFICATION hcm;
    char* context = new char[1024]();
    notFilter.cbSize = sizeof(CM_NOTIFY_FILTER);
    notFilter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
    notFilter.u.DeviceInterface.ClassGuid = GUID_DEVINTERFACE_USB_DEVICE;
    regRet = CM_Register_Notification(&notFilter, (PVOID)context, (PCM_NOTIFY_CALLBACK)eventAction, &hcm);
    SvcInit(dwArgc, lpszArgv);
    CM_Unregister_Notification(hcm);
}


VOID SvcInit(DWORD dwArgc, LPTSTR* lpszArgv)
{
    HANDLE port = INVALID_HANDLE_VALUE, hThread = NULL;
    HRESULT hResult = S_OK;
    BOOL fSuccess = FALSE;
    ghSvcStopEvent = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    // Report running status when initialization is complete.

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
    OutputDebugStringW(L"Service: is running");
    // TO_DO: Perform work until service stops.
    /*hResult = FilterConnectCommunicationPort(PORT_NAME,
            0,
            NULL,
            0,
            NULL,
            &port);
    if (IS_ERROR(hResult)) {
        OutputDebugStringW(L"Service: Could not connect to filter\n");
    }*/
    pipe = Pipe(L"\\\\.\\pipe\\userpipe");
    pipe.getName();
    HANDLE hHeap = GetProcessHeap();
    TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, pipe.getSize() * sizeof(TCHAR));
    TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, pipe.getSize() * sizeof(TCHAR));
    DWORD  dwThreadId = 0;

    
    hThread = CreateThread(
        NULL,              // no security attribute 
        0,                 // default stack size 
        threadFunc,    // thread proc
        NULL,    // thread parameter 
        0,                 // not suspended 
        &dwThreadId);      // returns thread ID 

    if (hThread == NULL)
    {
        OutputDebugString(L"CreateThread failed.\n");
    }
    //else CloseHandle(hThread);
    OutputDebugString(L"CreateThread success.\n");
    
   

    while (1)
    {
        // Check whether to stop the service.
        WaitForSingleObject(ghSvcStopEvent, INFINITE);

        OutputDebugStringW(L"Service: is stopped.");
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }
}


VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

int __cdecl _tmain(int argc, TCHAR* argv[])
{
    
    OutputDebugStringW(L"Service just start");
    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] =
    {
        { (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
        { NULL, NULL }
    };

    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcherW(DispatchTable))
    {
        OutputDebugStringW(L"StartServiceCtrlDispatcher");
    }
    //return 0;
}

DWORD WINAPI threadFunc(LPVOID lpParam)
{
    BOOL fSuccess = FALSE;
    HANDLE hHeap = GetProcessHeap();
    TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, pipe.getSize() * sizeof(TCHAR));
    TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, pipe.getSize() * sizeof(TCHAR));
    OutputDebugString(L"InstanceThread: pipe creating.\n");
    fSuccess = pipe.create();
    OutputDebugString(L"InstanceThread: client connected.\n");
    if (!fSuccess) return 1;
    while (1) {
        fSuccess = pipe.readG(pchRequest, FALSE);
        if (!fSuccess)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                OutputDebugString(L"InstanceThread: client disconnected.\n");
            }
            else
            {
                OutputDebugString(TEXT("InstanceThread ReadFile failed, GLE=%d.\n", GetLastError()));
            }
            break;
        }

        OutputDebugString(pchRequest);
        if (!FAILED(StringCchCopy(pchReply, pipe.getSize(), TEXT("Default answer from server")))) fSuccess = pipe.writeG(pchReply, FALSE);
    }
    DisconnectNamedPipe(pipe.getHandle());
    return 0;
}

