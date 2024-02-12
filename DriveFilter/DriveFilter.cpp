/*++

Module Name:

    DriveFilter.c

Abstract:

    This is the main module of the DriveFilter miniFilter driver.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>
#include <handleapi.h>

//#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


PFLT_FILTER gFilterHandle;
ULONG_PTR OperationStatusCtx = 1;
//PFLT_PORT port = NULL;
//UNICODE_STRING* msg = NULL;

#define PTDBG_TRACE_ROUTINES 0x00000001
#define PTDBG_TRACE_OPERATION_STATUS 0x00000002
//#define PORT_NAME L"\\FltPort"

ULONG gTraceFlags = 0;


#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
DriveFilterLoad (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
DriveFilterInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
DriveFilterInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
DriveFilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
DriveFilterInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
PreOperationFS(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

FLT_PREOP_CALLBACK_STATUS
PreOperationPnP(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

FLT_PREOP_CALLBACK_STATUS
PreOperationMount(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

VOID
DriveFilterOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    );

FLT_POSTOP_CALLBACK_STATUS
DriveFilterPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
DriveFilterPreOperationNoPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

BOOLEAN
DriveFilterDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    );




EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriveFilterUnload)
#pragma alloc_text(PAGE, DriveFilterInstanceQueryTeardown)
#pragma alloc_text(PAGE, DriveFilterLoad)
#pragma alloc_text(PAGE, DriveFilterInstanceTeardownStart)
#pragma alloc_text(PAGE, DriveFilterInstanceTeardownComplete)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {

    { IRP_MJ_CREATE,
      0,
      PreOperationFS,
      DriveFilterPostOperation },
// TODO - List all of the requests to filter.
    //{ IRP_MJ_CREATE_NAMED_PIPE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_CLOSE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_READ,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_WRITE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_QUERY_INFORMATION,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_SET_INFORMATION,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_QUERY_EA,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_SET_EA,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_FLUSH_BUFFERS,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_QUERY_VOLUME_INFORMATION,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_SET_VOLUME_INFORMATION,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_DIRECTORY_CONTROL,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_FILE_SYSTEM_CONTROL,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_DEVICE_CONTROL,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_INTERNAL_DEVICE_CONTROL,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_SHUTDOWN,
    //  0,
    //  DriveFilterPreOperationNoPostOperation,
    //  NULL },                               //post operations not supported

    //{ IRP_MJ_LOCK_CONTROL,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_CLEANUP,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_CREATE_MAILSLOT,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_QUERY_SECURITY,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_SET_SECURITY,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_QUERY_QUOTA,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_SET_QUOTA,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    { IRP_MJ_PNP,
      0,
      PreOperationPnP,
      DriveFilterPostOperation },

    //{ IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_ACQUIRE_FOR_MOD_WRITE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_RELEASE_FOR_MOD_WRITE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_ACQUIRE_FOR_CC_FLUSH,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_RELEASE_FOR_CC_FLUSH,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_NETWORK_QUERY_OPEN,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_MDL_READ,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_MDL_READ_COMPLETE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_PREPARE_MDL_WRITE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    //{ IRP_MJ_MDL_WRITE_COMPLETE,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

    { IRP_MJ_VOLUME_MOUNT,
      0,
       PreOperationPnP,
      DriveFilterPostOperation },

    //{ IRP_MJ_VOLUME_DISMOUNT,
    //  0,
    //  DriveFilterPreOperation,
    //  DriveFilterPostOperation },

// TODO

    { IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    DriveFilterUnload,                           //  MiniFilterUnload

    DriveFilterLoad,                    //  InstanceSetup
    DriveFilterInstanceQueryTeardown,            //  InstanceQueryTeardown
    DriveFilterInstanceTeardownStart,            //  InstanceTeardownStart
    DriveFilterInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};



NTSTATUS
DriveFilterLoad (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
/*++

Routine Description:

    This routine is called whenever a new instance is created on a volume. This
    gives us a chance to decide if we need to attach to this volume or not.

    If this routine is not defined in the registration structure, automatic
    instances are always created.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Flags describing the reason for this attach request.

Return Value:

    STATUS_SUCCESS - attach
    STATUS_FLT_DO_NOT_ATTACH - do not attach

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriveFilterInstanceSetup: Entered\n") );

    return STATUS_SUCCESS;
}


NTSTATUS
DriveFilterInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This is called when an instance is being manually deleted by a
    call to FltDetachVolume or FilterDetach thereby giving us a
    chance to fail that detach request.

    If this routine is not defined in the registration structure, explicit
    detach requests via FltDetachVolume or FilterDetach will always be
    failed.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Indicating where this detach request came from.

Return Value:

    Returns the status of this operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriveFilterInstanceQueryTeardown: Entered\n") );

    return STATUS_SUCCESS;
}


VOID
DriveFilterInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the start of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriveFilterInstanceTeardownStart: Entered\n") );
}


VOID
DriveFilterInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the end of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriveFilterInstanceTeardownComplete: Entered\n") );
}


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;
    /*PSECURITY_DESCRIPTOR secD;
    OBJECT_ATTRIBUTES objAtr;
    UNICODE_STRING uniString;*/

    /*PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriverEntry: Entered\n") );*/
    DbgPrint(("DRIVER IS LOAD\n"));

    //
    //  Register with FltMgr to tell it our callback routines
    //

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &gFilterHandle );

    //FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) {

        //
        //  Start filtering i/o
        //

        status = FltStartFiltering( gFilterHandle );

        if (!NT_SUCCESS( status )) {
            DbgPrint("DriveFilter!Filtering: Failure\n");
            FltUnregisterFilter( gFilterHandle );
        }
    }

    return status;
}

NTSTATUS
DriveFilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unload indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriveFilterUnload: Entered\n") );

    FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
PreOperationFS (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    )
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The context for the completion routine for this
        operation.

Return Value:

    The return value is the status of the operation.

--*/
{
    
    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION fInfo = { 0 };
    PFLT_VOLUME volume;
    PDEVICE_OBJECT DiskDeviceObject;
    UNICODE_STRING GUID = { 0 };
    UNREFERENCED_PARAMETER(CompletionContext);

    volume = FltObjects->Volume;
    
    status = FltGetDiskDeviceObject(volume, &DiskDeviceObject);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("DriveFilter!Could not create the DiskDeviceObject\n");
    }

    if (NT_SUCCESS(status)) 
    {
        ULONG n = 0;
        status = FltGetVolumeGuidName(volume, &GUID, &n);
        
    }
    
    status = FltGetFileNameInformation(Data,
        FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY, &fInfo);
    if (NT_SUCCESS(status)) {
        
        status = FltParseFileNameInformation(fInfo);

        if (Data->Iopb->TargetFileObject->DeviceObject->Characteristics & FILE_REMOVABLE_MEDIA) {
            DbgPrint("DriveFilter!NAME %wZ GUID %wZ\r\n", &fInfo->Name, GUID);
        }
           
    }

    if (DriveFilterDoRequestOperationStatus( Data )) {

        status = FltRequestOperationStatusCallback( Data,
                                                    DriveFilterOperationStatusCallback,
                                                    (PVOID)(++OperationStatusCtx) );
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                          ("DriveFilter! DriveFilterPreOperation: FltRequestOperationStatusCallback Failed, status=%08x\n",
                           status) );
        }
    }
    ObDereferenceObject(DiskDeviceObject);

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS
PreOperationPnP(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path

--*/
{

    NTSTATUS status;

    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(FltObjects);
    FLT_PREOP_CALLBACK_STATUS callbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

    switch (Data->Iopb->MinorFunction) {
    case IRP_MN_QUERY_REMOVE_DEVICE:
        DbgPrint("DriveFilter: REMOVE QUERY\n");
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        DbgPrint("DriveFilter: REMOVE CANCEL\n");
        callbackStatus = FLT_PREOP_SYNCHRONIZE;
        break;

    case IRP_MN_SURPRISE_REMOVAL:
        DbgPrint("DriveFilter: SURPRISE REMOVE\n");
        status = FltDetachVolume(gFilterHandle, FltObjects->Volume, NULL);

        if (!NT_SUCCESS(status)) {

            DbgPrint(
                "DriveFilter: [Fmm]: Failed to detach instance with status 0x%x after a surprise removal\n",
                status);
        }

        break;
    }

    if (DriveFilterDoRequestOperationStatus(Data)) {

        status = FltRequestOperationStatusCallback(Data,
            DriveFilterOperationStatusCallback,
            (PVOID)(++OperationStatusCtx));
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("DriveFilter: DriveFilterPreOperation: FltRequestOperationStatusCallback Failed, status=%08x\n",
                    status));
        }
    }


    return callbackStatus;
}

FLT_PREOP_CALLBACK_STATUS
PreOperationMount(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION fInfo = { 0 };
    //PFLT_VOLUME volume;
    //PDEVICE_OBJECT DiskDeviceObject;
    //UNICODE_STRING dosName;
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER(CompletionContext);
    FLT_PREOP_CALLBACK_STATUS callbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
    DbgPrint("DriveFilter! MOUNT ENTRY\r\n");

    switch (Data->Iopb->MinorFunction) {

    
    case IRP_MN_MOUNT_VOLUME:
       /* volume = FltObjects->Volume;


        status = FltGetDiskDeviceObject(volume, &DiskDeviceObject);

        if (!NT_SUCCESS(status))
        {
            DbgPrint("Could not create the DiskDeviceObject\n");
        }

        RtlInitUnicodeString(&dosName, NULL);
        if (NT_SUCCESS(status))
        {
            status = IoVolumeDeviceToDosName(DiskDeviceObject, &dosName);
        }*/
        DbgPrint("DriveFilter! MOUNT ENTRY CASE\r\n");
        status = FltGetFileNameInformation(Data,
            FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY, &fInfo);
        if (NT_SUCCESS(status)) {
            status = FltParseFileNameInformation(fInfo);
            DbgPrint("DriveFilter! PnP %wZ\r\n", &fInfo->Volume);
        }
        callbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
        break;
    }

    
    
    if (DriveFilterDoRequestOperationStatus(Data)) {

        status = FltRequestOperationStatusCallback(Data,
            DriveFilterOperationStatusCallback,
            (PVOID)(++OperationStatusCtx));
        if (!NT_SUCCESS(status)) {

            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("DriveFilter!DriveFilterPreOperation: FltRequestOperationStatusCallback Failed, status=%08x\n",
                    status));
        }
    }
    
    return callbackStatus;
}

VOID
DriveFilterOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    )
/*++

Routine Description:

    This routine is called when the given operation returns from the call
    to IoCallDriver.  This is useful for operations where STATUS_PENDING
    means the operation was successfully queued.  This is useful for OpLocks
    and directory change notification operations.

    This callback is called in the context of the originating thread and will
    never be called at DPC level.  The file object has been correctly
    referenced so that you can access it.  It will be automatically
    dereferenced upon return.

    This is non-pageable because it could be called on the paging path

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    RequesterContext - The context for the completion routine for this
        operation.

    OperationStatus -

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriveFilterOperationStatusCallback: Entered\n") );

    PT_DBG_PRINT( PTDBG_TRACE_OPERATION_STATUS,
                  ("DriveFilter!DriveFilterOperationStatusCallback: Status=%08x ctx=%p IrpMj=%02x.%02x \"%s\"\n",
                   OperationStatus,
                   RequesterContext,
                   ParameterSnapshot->MajorFunction,
                   ParameterSnapshot->MinorFunction,
                   FltGetIrpName(ParameterSnapshot->MajorFunction)) );
}


FLT_POSTOP_CALLBACK_STATUS
DriveFilterPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    )
/*++

Routine Description:

    This routine is the post-operation completion routine for this
    miniFilter.

    This is non-pageable because it may be called at DPC level.

Arguments:

    Data - Pointer to the filter callbackData that is passed to us.

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance, its associated volume and
        file object.

    CompletionContext - The completion context set in the pre-operation routine.

    Flags - Denotes whether the completion is successful or is being drained.

Return Value:

    The return value is the status of the operation.

--*/
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter: DriveFilterPostOperation: Entered\n") );

    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS
DriveFilterPreOperationNoPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    )
/*++

Routine Description:

    This routine is a pre-operation dispatch routine for this miniFilter.

    This is non-pageable because it could be called on the paging path
--*/
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("DriveFilter!DriveFilterPreOperationNoPostOperation: Entered\n") );

    // This template code does not do anything with the callbackData, but
    // rather returns FLT_PREOP_SUCCESS_NO_CALLBACK.
    // This passes the request down to the next miniFilter in the chain.

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


BOOLEAN
DriveFilterDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    )
/*++

Routine Description:

    This identifies those operations we want the operation status for.  These
    are typically operations that return STATUS_PENDING as a normal completion
    status.
Return Value:

    TRUE - If we want the operation status
    FALSE - If we don't

--*/
{
    PFLT_IO_PARAMETER_BLOCK iopb = Data->Iopb;

    //
    //  return boolean state based on which operations we are interested in
    //

    return (BOOLEAN)

    //
    //  Check for oplock operations
    //

        (((iopb->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
        ((iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_FILTER_OPLOCK)  ||
        (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_BATCH_OPLOCK)   ||
        (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_1) ||
        (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_2)))

        ||

        //
        //    Check for directy change notification
        //

        ((iopb->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
        (iopb->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY))
        );
}


