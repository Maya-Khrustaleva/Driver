#include "moufiltr.h"
#include <moufiltr.tmh>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
//#pragma alloc_text (INIT, GetMaxValue)
#pragma alloc_text (PAGE, MouFilter_EvtDeviceAdd)
#pragma alloc_text (PAGE, MouFilter_EvtIoInternalDeviceControl)
#pragma alloc_text (PAGE, MouFiltr_EvtDriverContextCleanup)
#endif


#pragma warning(push)
#pragma warning(disable:4055) // type case from PVOID to PSERVICE_CALLBACK_ROUTINE
#pragma warning(disable:4152) // function/data pointer conversion in expression

typeCountPressButton maxValue;

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
/*++
Routine Description:

     Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

--*/
{
    WDF_DRIVER_CONFIG               config;
    NTSTATUS                        status;

    DebugPrint(("Mouse Filter Driver Sample - Driver Framework Edition.\n"));
    DebugPrint(("Built %s %s\n", __DATE__, __TIME__));

    WPP_INIT_TRACING(DriverObject, RegistryPath);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
        "MouFiltr Driver Sample — Driver Framework Edition.\n");

    GetMaxValue(&maxValue);

    WDF_DRIVER_CONFIG_INIT(
        &config,
        MouFilter_EvtDeviceAdd
    );

    WDF_OBJECT_ATTRIBUTES driverAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&driverAttributes);
    driverAttributes.EvtCleanupCallback = MouFiltr_EvtDriverContextCleanup;
    //
    // Create a framework driver object to represent our driver.
    //
    status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            &driverAttributes, 
                            &config,
                            WDF_NO_HANDLE); 

    if (!NT_SUCCESS(status)) {
        DebugPrint( ("WdfDriverCreate failed with status 0x%x\n", status));

        TraceEvents(TRACE_LEVEL_ERROR, TRACE_ERROR,
            "WdfDriverCreate failed with status Ox%X\n",
            status);
        WPP_CLEANUP(DriverObject);
    }

    return status; 

   
}

VOID GetMaxValue(typeCountPressButton* maxVal)
{
    (*maxVal) = 0;
    for (unsigned short i = 0; i < sizeof(typeCountPressButton) * 8; i++)
    {
        (*maxVal) += 2 ^ i;
    }
}

NTSTATUS
MouFilter_EvtDeviceAdd(
    IN WDFDRIVER        Driver,
    IN PWDFDEVICE_INIT  DeviceInit
    )
/*++
Routine Description:

    EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. Here you can query the device properties
    using WdfFdoInitWdmGetPhysicalDevice/IoGetDeviceProperty and based
    on that, decide to create a filter device object and attach to the
    function stack.

    If you are not interested in filtering this particular instance of the
    device, you can just return STATUS_SUCCESS without creating a framework
    device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/   
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    NTSTATUS                status;
    WDFDEVICE               hDevice;
    WDF_IO_QUEUE_CONFIG     ioQueueConfig;
    
    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    DebugPrint(("Enter FilterEvtDeviceAdd \n"));

    WdfFdoInitSetFilter(DeviceInit);
 
    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_MOUSE);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes,
        DEVICE_EXTENSION);

    //
    // Create a framework device object.  This call will in turn create
    // a WDM deviceobject, attach to the lower stack and set the
    // appropriate flags and attributes.
    //
    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &hDevice);
    if (!NT_SUCCESS(status)) {
        DebugPrint(("WdfDeviceCreate failed with status code 0x%x\n", status));
        return status;
    }


    //
    // Configure the default queue to be Parallel. 
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                             WdfIoQueueDispatchParallel);

    //
    // Framework by default creates non-power managed queues for
    // filter drivers.
    //
    //ioQueueConfig.PowerManaged = FALSE;//не управляемая энергопотреблением очередь вводв/вывода
    ioQueueConfig.EvtIoInternalDeviceControl = MouFilter_EvtIoInternalDeviceControl;
  
    status = WdfIoQueueCreate(hDevice,
                            &ioQueueConfig,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            WDF_NO_HANDLE // pointer to default queue
                            );
    if (!NT_SUCCESS(status)) {
        DebugPrint( ("WdfIoQueueCreate failed 0x%x\n", status));
        return status;
    }

    return status;
}


VOID Moufiltr_EvtWriteWorkItem(
    IN WDFWORKITEM WorkItem
)
/*++
Routine Description:

    Moufiltr_EvtWriteWorkItem is callback function for work item.
    Moufiltr_EvtWriteWorkItem writes a statistics of mouse clicks to the file 

    To transfer data to the EvtWorkItem callback function, 
    driver must use the context area of the work item object.

Arguments:

    WorkItem - work item object


--*/
{
    NTSTATUS          status = STATUS_SUCCESS;   
    HANDLE            fileHandle = NULL;
    IO_STATUS_BLOCK   iostatus;
    WDFDEVICE hDevice;
    PDEVICE_EXTENSION devExt;
    OBJECT_ATTRIBUTES objectAttributes;
    PWORKITEM_CONTEXT pItemContext;

    pItemContext = GetWorkItemContext(WorkItem);
   

    hDevice = pItemContext->Device;
    devExt = FilterGetData(hDevice);

    RtlInitUnicodeString(&devExt->fileName, L"\\??\\C:\\Output\\output.txt");
    InitializeObjectAttributes(&objectAttributes,
        &devExt->fileName,
        OBJ_KERNEL_HANDLE |     //indicates that the descriptor can only be accessed in kernel mode
        OBJ_CASE_INSENSITIVE,   //case insensitive name comparison
        NULL,                   //RootDirectory is NULL, If ObjectName is the fully qualified name of the object
        NULL                    //Drivers can specify NULL to accept object security by default.(optional parameter)
    );
      
    status = ZwCreateFile(&fileHandle,
        SYNCHRONIZE | GENERIC_WRITE,
        &objectAttributes,
        &iostatus,
        0,  // alloc size = none
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
        NULL,
        0);

    if (NT_SUCCESS(status))
    {
        FILE_STANDARD_INFORMATION fileInfo;

        status = ZwQueryInformationFile(fileHandle,
            &iostatus,
            &fileInfo,
            sizeof(FILE_STANDARD_INFORMATION),
            FileStandardInformation);
        
        if (NT_SUCCESS(status))
        {
            char buffer[20];// buffer for writing to file

            status = RtlStringCbPrintfA(buffer, sizeof(buffer), "%s, count=%d\n", devExt->Button.Buffer, devExt->count);
            if (NT_SUCCESS(status))
            {
                size_t byteCount;// quantity of bytes to write to file

                status = RtlStringCbLengthA(buffer, sizeof(buffer), &byteCount);
                if (NT_SUCCESS(status))
                {
                    LARGE_INTEGER ByteOffset = fileInfo.EndOfFile;

                    status = ZwWriteFile(fileHandle,
                        NULL,//null necessarily for intermediate drivers
                        NULL,//null necessarily for intermediate drivers
                        NULL,//null necessarily for intermediate drivers
                        &iostatus,
                        buffer,
                        byteCount, 
                        &ByteOffset, 
                        NULL);//null necessarily for intermediate drivers

                    //iostatus.Information if the transfer request succeeds, the number of bytes transferred is set
                    //otherwise - 0
                    if (!NT_SUCCESS(status) || iostatus.Information != byteCount)
                    {
                        DbgPrint("Moufiltr_EvtWriteWorkItem: ZwWriteFile failed with status 0x%x\n", status);
                    }
                }
                else 
                {
                    DbgPrint("Moufiltr_EvtWriteWorkItem: RtlStringCbLengthA failed with status 0x%x\n", status);
                }
                
            }
            else 
            { 
                DbgPrint("Moufiltr_EvtWriteWorkItem: RtlStringCbPrintfA failed with status 0x%x\n", status); 
            }

        }
        
        ZwClose(fileHandle);
    }

    WdfObjectDelete(WorkItem);

    return;

}




VOID
MouFilter_DispatchPassThrough(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target
    )
/*++
Routine Description:

    Passes a request on to the lower driver.

--*/
{
    //
    // Pass the IRP to the target
    //
 
    WDF_REQUEST_SEND_OPTIONS options;
    BOOLEAN ret;
    NTSTATUS status = STATUS_SUCCESS;

    WDF_REQUEST_SEND_OPTIONS_INIT(&options,
                                  WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);

    ret = WdfRequestSend(Request, Target, &options);

    if (ret == FALSE) {
        status = WdfRequestGetStatus (Request);
        DebugPrint( ("WdfRequestSend failed: 0x%x\n", status));
        WdfRequestComplete(Request, status);
    }

    return;
}           

VOID
MouFilter_EvtIoInternalDeviceControl(
    IN WDFQUEUE      Queue,
    IN WDFREQUEST    Request,
    IN size_t        OutputBufferLength,
    IN size_t        InputBufferLength,
    IN ULONG         IoControlCode
    )
/*++

Routine Description:

    This routine is the dispatch routine for internal device control requests.
    There are two specific control codes that are of interest:
    
    IOCTL_INTERNAL_MOUSE_CONNECT:
        Store the old context and function pointer and replace it with our own.
        This makes life much simpler than intercepting IRPs sent by the RIT and
        modifying them on the way back up.
                                                                            
--*/
{
    
    PDEVICE_EXTENSION           devExt;
    PCONNECT_DATA               connectData;
    NTSTATUS                   status = STATUS_SUCCESS;
    WDFDEVICE                 hDevice;
    size_t                           length; 

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    PAGED_CODE();//макрос используется для выявления случаев входа в страничный код с недопустимо высокими приоритетами

    hDevice = WdfIoQueueGetDevice(Queue);
    devExt = FilterGetData(hDevice);

    switch (IoControlCode) {

    //
    // Connect a mouse class device driver to the port driver.
    //
    case IOCTL_INTERNAL_MOUSE_CONNECT:
        //
        // Only allow one connection.
        //
        if (devExt->UpperConnectData.ClassService != NULL) {
            status = STATUS_SHARING_VIOLATION;
            //((NTSTATUS)0xC0000043L)
            // MessageId: STATUS_SHARING_VIOLATION
            //
            // MessageText:
            //
            // A file cannot be opened because the share access flags are incompatible.
            break;
        }
        
        //
        // Copy the connection parameters to the device extension.
        //
         status = WdfRequestRetrieveInputBuffer(Request,
                            sizeof(CONNECT_DATA),
                            &connectData,
                            &length);
            //
            // Define the port connection data structure.
            //
            /*    typedef struct _CONNECT_DATA {
                 IN PDEVICE_OBJECT ClassDeviceObject;
                 IN PVOID ClassService;
             } CONNECT_DATA, * PCONNECT_DATA;*/

        if(!NT_SUCCESS(status)){
            DebugPrint(("WdfRequestRetrieveInputBuffer failed 0x%x\n", status));
            break;
        }

        
        devExt->UpperConnectData = *connectData;

        //MouFilter_EvtIoInternalDeviceControl
        // Hook into the report chain.  Everytime a mouse packet is reported to
        // the system, MouFilter_ServiceCallback will be called
        //
        connectData->ClassDeviceObject = WdfDeviceWdmGetDeviceObject(hDevice);
        //The WdfDeviceWdmGetDeviceObject method returns the Windows Driver Model (WDM) device object 
        //that is associated with a specified framework device object.

        connectData->ClassService = MouFilter_ServiceCallback;

        break;

    //
    // Disconnect a mouse class device driver from the port driver.
    //
    case IOCTL_INTERNAL_MOUSE_DISCONNECT:

        //
        // Clear the connection parameters in the device extension.
        //
        
        devExt->UpperConnectData.ClassDeviceObject = NULL;
        devExt->UpperConnectData.ClassService = NULL;

        status = STATUS_NOT_IMPLEMENTED;
        break;
    
    //case IOCTL_INTERNAL_MOUSE_ENABLE:
    //case IOCTL_INTERNAL_MOUSE_DISABLE:

    //case IOCTL_MOUSE_QUERY_ATTRIBUTES:
    //case IOCTL_MOUSE_INSERT_DATA:

    default:
        break;
    }

    if (!NT_SUCCESS(status)) {
        WdfRequestComplete(Request, status);
        return ;
    }

    MouFilter_DispatchPassThrough(Request,WdfDeviceGetIoTarget(hDevice));
}


VOID
MouFiltr_EvtDriverContextCleanup(
    IN WDFOBJECT Driver
)
{
    //
    // EvtCleanupCallback for WDFDRIVER is always called at PASSIVE_LEVEL
    //
    _IRQL_limited_to_(PASSIVE_LEVEL);

    PAGED_CODE();
    //
    // For the case when WPP is not being used.
    //
    UNREFERENCED_PARAMETER(Driver);

    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_VERBOSE,
        "Entered Moufiltr_EvtDriverContextCleanup\n");

    DebugPrint(("Enter in MouFiltr_EvtDriverContextCleanup"));
    //
    // No need to free the controldevice object explicitly because it will
    // be deleted when the Driver object is deleted due to the default parent
    // child relationship between Driver and ControlDevice.
    //
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)Driver));

}

#pragma warning(disable:28118) // this callback will run at IRQL=PASSIVE_LEVEL
VOID
MouFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA InputDataStart,
    IN PMOUSE_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    )
/*++

Routine Description:

    Called when there are mouse packets to report to the RIT.
                    
Arguments:

    DeviceObject - Context passed during the connect IOCTL
    
    InputDataStart - First packet to be reported
    
    InputDataEnd - One past the last packet to be reported.  Total number of
                   packets is equal to InputDataEnd - InputDataStart
    
    InputDataConsumed - Set to the total number of packets consumed by the RIT
                        (via the function pointer we replaced in the connect
                        IOCTL)

Return Value:

    Status is returned.

--*/
{
    PDEVICE_EXTENSION   devExt;
    WDFDEVICE   hDevice;
    NTSTATUS status = STATUS_SUCCESS;
    
    hDevice = WdfWdmDeviceGetWdfDeviceHandle(DeviceObject);
    devExt = FilterGetData(hDevice);

    if ((InputDataStart->ButtonFlags & MOUSE_RIGHT_BUTTON_DOWN) || (InputDataStart->ButtonFlags & MOUSE_LEFT_BUTTON_DOWN))
    {
        PWORKITEM_CONTEXT context;
        WDF_OBJECT_ATTRIBUTES attributes;
        WDF_WORKITEM_CONFIG workitemConfig;
        WDFWORKITEM hWorkItem;
        static typeCountPressButton leftButton = 0, rigthButton = 0;
        typeCountPressButton current;
        ANSI_STRING pressButton;
        //TODO: добавить переопределение maxvalue

        if (InputDataStart->ButtonFlags & MOUSE_RIGHT_BUTTON_DOWN)
        {
            (rigthButton == maxValue) ? rigthButton = 0 : rigthButton++;
            DebugPrint(("\nRigth botton pressed, count is %i\n", rigthButton));
            current = rigthButton;
            RtlInitAnsiString(&pressButton, "Right");
        }
        else
        {
            (leftButton == maxValue) ? leftButton = 0 : leftButton++;
            DebugPrint(("\nLeft botton pressed, count is %i\n", leftButton));
            current = leftButton;
            RtlInitAnsiString(&pressButton, "Left");
        }

        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, WORKITEM_CONTEXT);
        attributes.ParentObject = hDevice;

        WDF_WORKITEM_CONFIG_INIT(&workitemConfig, Moufiltr_EvtWriteWorkItem);

        status = WdfWorkItemCreate(&workitemConfig,
            &attributes,
            &hWorkItem);

        if (!NT_SUCCESS(status)) {
            DebugPrint(("WdfWorkItemCreate failed with 0x%x status\n", status));
        }
        else
        {
            context = GetWorkItemContext(hWorkItem);
            context->Device = hDevice;
            devExt->count = current;
            devExt->Button = pressButton;

            WdfWorkItemEnqueue(hWorkItem);
        }
    }
    //
    // UpperConnectData must be called at DISPATCH
    //
    (*(PSERVICE_CALLBACK_ROUTINE) devExt->UpperConnectData.ClassService)(
        devExt->UpperConnectData.ClassDeviceObject,
        InputDataStart,
        InputDataEnd,
        InputDataConsumed
        );    
} 

#pragma warning(pop)


