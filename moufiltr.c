#include "moufiltr.h"

//#include "moufiltr.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, MouFilter_EvtDeviceAdd)
#pragma alloc_text (PAGE, MouFilter_EvtIoInternalDeviceControl)
//#pragma alloc_text (PAGE, MouFiltr_EvtIoRead)
#endif


#pragma warning(push)
#pragma warning(disable:4055) // type case from PVOID to PSERVICE_CALLBACK_ROUTINE
#pragma warning(disable:4152) // function/data pointer conversion in expression

#define maxValue 4294967295 

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
    
    //***
    //WDF_OBJECT_ATTRIBUTES          attributes;
    //UNREFERENCED_PARAMETER(attributes);

    DebugPrint(("Mouse Filter Driver Sample - Driver Framework Edition.\n"));
    DebugPrint(("Built %s %s\n", __DATE__, __TIME__));
    
    // Initialize driver config to control the attributes that
    // are global to the driver. Note that framework by default
    // provides a driver unload routine. If you create any resources
    // in the DriverEntry and want to be cleaned in driver unload,
    // you can override that by manually setting the EvtDriverUnload in the
    // config structure. In general xxx_CONFIG_INIT macros are provided to
    // initialize most commonly used members.

    WDF_DRIVER_CONFIG_INIT(
        &config,
        MouFilter_EvtDeviceAdd
    );/* Первым делом функция DriverEntry инициализирует структуру конфигурации объекта драй -
        вера указателем на обратный вызов EvtDriverDeviceAdd драйвера. Для этого она вызывает
        функцию WDF_DRIVER_CONFIG_INIT.*/

    //
    // Create a framework driver object to represent our driver.
    //

    //***
    //WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    //attributes.EvtCleanupCallback = MouFilterEvtDriverContextCleanup;
    //attributes.ExecutionLevel=
    
    /*После инициализации структур конфигурации и атрибутов драйвер вызывает метод
        WdfDriverCreate, чтобы создать инфраструктурный объект драйвера. Метод WdfDriverCreate
        принимает следующие параметры:*/
    status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            WDF_NO_OBJECT_ATTRIBUTES, //при создании функции очистки - attributes
                            &config,
                            WDF_NO_HANDLE); // hDriver optional
                            /*адрес для дескриптора созданного объекта WDFDRIVER. Это необязательный параметр и
                            может быть WDF_NO_HANDLE (определен как NULL), если драйвер не нуждается в этом деск-
                            рипторе.
                            Большинство драйверов не сохраняет дескриптор объекта драйвера, т. к. он редко используется,
                            и драйвер всегда может получить его с помощью метода WdfGetDriver.*/
    

    if (!NT_SUCCESS(status)) {
        DebugPrint( ("WdfDriverCreate failed with status 0x%x\n", status));
    }


    

    //инициализация трассировки WPP
    //WPP_INIT_TRACINC(DriverObject, RegistryPath);
    //TraceEvents(TRACE_LEVEL_INFORMATION, DBC_INIT,
    //   "MouFiltr Driver Sample — Driver Framework Edition.\n")

    return status; 

   
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

    //WDF_OBJECT_ATTRIBUTES queueArributes;
    
    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    DebugPrint(("Enter FilterEvtDeviceAdd \n"));

    //
    // Tell the framework that you are filter driver. Framework
    // takes care of inherting all the device flags & characterstics
    // from the lower device you are attaching to.
    //
    WdfFdoInitSetFilter(DeviceInit);
    
    
    
    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_MOUSE);


    /*Устанавливает тип ввода/вывода.
    Драйвер указывает тип ввода/вывода для запросов на чтение и запись — буферизирован-
    ный, прямой или и непрямой, и небуферизированный. Устанавливается стандартный тип
    WdfDeviceIoBuffered.*/
    //WdfDeviceInitSetIoType(&DeviceInit, WdfDeviceIoBuffered);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes,
        DEVICE_EXTENSION);

    /*Драйвер записывает тип области контекста в структуру атрибутов объекта с помощью мак-
    роса WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE или макроса WDF_OBJECT_ ATTRIBUTES_SET_
    CONTEXT_TYPE.
    Макрос WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE записывает информацию об области кон-
    текста в ранее инициализированной структуре атрибутов, которую драйвер предоставляет
    в дальнейшем при создании объекта.
    Макрос WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE совмещает действия макросов WDF_OBJECT_
    ATTRIBUTES_INIT и WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE.
    То есть, кроме информации о контексте, он инициализирует структуру атрибутов установ-
    ками для других атрибутов.*/

    //deviceAttributes.EvtCleanupCallback = MouFiltr_EvtDriverContextCleanup;


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
    // Configure the default queue to be Parallel. Do not use sequential queue
    // if this driver is going to be filtering PS2 ports because it can lead to
    // deadlock. The PS2 port driver sends a request to the top of the stack when it
    // receives an ioctl request and waits for it to be completed. If you use a
    // a sequential queue, this request will be stuck in the queue because of the 
    // outstanding ioctl request sent earlier to the port driver.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                             WdfIoQueueDispatchParallel);

    //
    // Framework by default creates non-power managed queues for
    // filter drivers.
    //

    //ioQueueConfig.PowerManaged = FALSE;//не управляемая энергопотреблением очередь вводв/вывода
    ioQueueConfig.EvtIoInternalDeviceControl = MouFilter_EvtIoInternalDeviceControl;
    //ioQueueConfig.EvtIoRead = MouFiltr_EvtIoRead;


    //WDF_OBJECT_ATTRIBUTES_INIT(&queueArributes);
    //queueArributes.ExecutionLevel = WdfExecutionLevelPassive;

    status = WdfIoQueueCreate(hDevice,
                            &ioQueueConfig,
                            //&queueArributes,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            WDF_NO_HANDLE // pointer to default queue
                            );
    if (!NT_SUCCESS(status)) {
        DebugPrint( ("WdfIoQueueCreate failed 0x%x\n", status));
        return status;
    }

    return status;
}

//VOID
//MouFiltr_EvtIoRead(
//    _In_
//    WDFQUEUE Queue,
//    _In_
//    WDFREQUEST Request,
//    _In_
//    size_t Length)
//{
//    PDEVICE_EXTENSION devExt;
//    WDFDEVICE hDevice;
//    NTSTATUS status=STATUS_SUCCESS;
//    CONNECT_DATA connectData;
//    PMOUSE_INPUT_DATA pMouseData;
//    PINTERNAL_I8042_HOOK_MOUSE hookMouse;
//    size_t length;
//
//
//    hDevice = WdfIoQueueGetDevice(Queue);
//    devExt = FilterGetData(hDevice);
//
//    //#define IOCTL_INTERNAL_I8042_HOOK_MOUSE     CTL_CODE(FILE_DEVICE_MOUSE, 0x0FF0, METHOD_NEITHER, FILE_ANY_ACCESS)
//
//
//    //у UpperConnectdata тип ConnectData
//            status = WdfRequestRetrieveInputBuffer(Request,
//                sizeof(INTERNAL_I8042_HOOK_MOUSE),
//                hookMouse,
//                &length);
//
//            if (!NT_SUCCESS(status))
//            {
//                DebugPrint(("MouFiltr_EvtIoRead, WdfRequestRetrieveInputBuffer failed %x\n", status));
//            }
//
//
//       
//        status = STATUS_SUCCESS;
//     
//}


//VOID
//Moufiltr_EvtFileClose(
//    IN WDFFILEOBJECT    FileObject
//)
//{
//    PDEVICE_EXTENSION devExt;
//
//    PAGED_CODE();
//
//
//    //TraceEvents(TRACE_LEVEL_VERBOSE, DBG_INIT, "Moufiltr_EvtFileClose\n");
//
//    devExt = ControlGetData(WdfFileObjectGetDevice(FileObject));
//
//    if (devExt->FileHandle) {
//        //TraceEvents(TRACE_LEVEL_VERBOSE, DBG_INIT,
//        //    "Closing File Handle %p", devExt->FileHandle);
//        ZwClose(devExt->FileHandle);
//    }
//
//    return;
//}



VOID Moufiltr_EvtWriteWorkItem(
    IN WDFWORKITEM WorkItem
)
{
    NTSTATUS          status = STATUS_SUCCESS;   
    HANDLE            fileHandle = NULL;//дескриптор объекта устройства, который будет посылать запросы ввода/вывода получателю
    IO_STATUS_BLOCK   iostatus;
    UNICODE_STRING     fileName;
    OBJECT_ATTRIBUTES objectAttributes;
    //UNICODE_STRING              absFileName, directory;
   // PDEVICE_EXTENSION devExt;
    //USHORT length = 0;

    //UNREFERENCED_PARAMETER(data);
    
    //_IRQL_limited_to_(PASSIVE_LEVEL);//либо вместо этих макросов использовать disable:28118
    //PAGED_CODE();
    

    //devExt = FilterGetData(Device);

    PAGED_CODE();
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    RtlInitUnicodeString(&fileName, L"\\??\\C:\\Output\\test.txt");
    
    //fileName = WdfFileObjectGetFileName(FileObject);

    //TraceEvents(TRACE_LEVEL_VERBOSE, DBG_INIT, "Moufiltr_EvtDeviceFileCreate %wZ%wZ",
    //    &directory, fileName);


    //
    // Find the total length of the directory + filename
    //
    //length = directory.Length + fileName->Length;
    
    //ExAllocatePoolWithTag выделяет пул памяти указанного типа и возвращает указатель на выделенный блок.

    /*Paged pool, which is pageable system memory. 
    Paged pool can only be allocated and accessed at IRQL < DISPATCH_LEVEL.*/


    /*Тег пула для использования в выделенной памяти. 
    Укажите тег пула как ненулевой символьный литерал от одного до четырех символов, 
    разделенных одинарными кавычками (например, «Tag1»). 
    Строка обычно указывается в обратном порядке (например, «1gaT»). 
    Каждый символ ASCII в теге должен иметь значение в диапазоне от 0x20 (пробел) до 0x7E (тильда).
    Каждый путь кода выделения должен использовать уникальный тег пула, чтобы помочь отладчикам 
    и верификаторам идентифицировать путь кода.*/

    //ExAllocatePoolWithTag возвращает NULL, если в свободном пуле недостаточно памяти для удовлетворения запроса. 
    //В противном случае процедура возвращает указатель на выделенную память.


    InitializeObjectAttributes(&objectAttributes,
        &fileName,
        OBJ_KERNEL_HANDLE |     //указывает, что дескриптор может быть доступен только в режиме ядра
        OBJ_CASE_INSENSITIVE,  //сравнение имен без учета регистра
        //OBJ_EXCLUSIVE |         //только один дескриптор может быть открыт для данного объекта
        //OBJ_OPENIF,             //если этот флаг указан в функции, которая создает объект и он уже существует,
        //то он должен быть открыт, иначе функция возвращает NTSTATUS code of STATUS_OBJECT_NAME_COLLISION.
        
        NULL,       //Дескриптор каталога корневого объекта для пути, 
                    //указанного в параметре ObjectName. 
                    //Если ObjectName является полностью определенным именем объекта, 
                    //RootDirectory имеет значение NULL.
        NULL        //Определяет дескриптор безопасности для применения к объекту при его создании. 
                    //Этот параметр не является обязательным. Драйверы могут указать NULL, 
                    //чтобы принять безопасность объекта по умолчанию. 
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

    DebugPrint(("\nStatus is %x\n",status));
    if (status == STATUS_OBJECT_NAME_COLLISION)
    {
        DebugPrint(("Hmmm"));
    }

    switch (iostatus.Information)
    {
    case FILE_CREATED:
        DebugPrint(("FILE_CREATED\n"));
        break;
    case FILE_OPENED:
        DebugPrint(("FILE_OPENED\n"));
        break;
    case FILE_OVERWRITTEN:
        DebugPrint(("FILE_OVERWRITTEN\n"));
        break;
    case FILE_SUPERSEDED:
        DebugPrint(("FILE_SUPERSEDED\n"));
        break;
    case FILE_EXISTS:
        DebugPrint(("FILE_EXISTS\n"));
        break;
    case FILE_DOES_NOT_EXIST:
        DebugPrint(("FILE_DOES_NOT_EXIST\n"));
        break;
    default:
        DebugPrint(("unexpected status file %x\n", iostatus.Information));
        break;
    }

    if (NT_SUCCESS(status))
    {
        // Структура, которая поможет определить длину файла:
        FILE_STANDARD_INFORMATION fileInfo;
        
        status =        // Получаем информацию о файле
            ZwQueryInformationFile(fileHandle,
                &iostatus,
                &fileInfo,
                sizeof(FILE_STANDARD_INFORMATION),
                FileStandardInformation
            );
        char cur[] = "test\n";
        ULONG len = strlen(cur);
        if (NT_SUCCESS(status))
        {
            LARGE_INTEGER ByteOffset = fileInfo.EndOfFile;
            
            status = ZwWriteFile(fileHandle,
                NULL,//null обязательно для intermediate drivers
                NULL,//null обязательно для intermediate drivers
                NULL,//null обязательно для intermediate drivers
                &iostatus,
                cur, len,   // Записываемый буфер и размер буфера в байтах
                &ByteOffset,     // a если NULL? см. ниже
                NULL);//null обязательно для intermediate drivers
            //iostatus.Information при успешном завершении запроса на  передачу устанавливается
            //число переданных байтов, иначе - 0
            if (!NT_SUCCESS(status) || iostatus.Information != len)
            {
                DbgPrint("Error on writing. Status = %x.", status);
            }
        }
        ZwClose(fileHandle);
    }
    return status;

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

    //
    // We are not interested in post processing the IRP so 
    // fire and forget.
    //
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
                                      
    IOCTL_INTERNAL_I8042_HOOK_MOUSE:
        Add in the necessary function pointers and context values so that we can
        alter how the ps/2 mouse is initialized.
                                            
    NOTE:  Handling IOCTL_INTERNAL_I8042_HOOK_MOUSE is *NOT* necessary if 
           all you want to do is filter MOUSE_INPUT_DATAs.  You can remove
           the handling code and all related device extension fields and 
           functions to conserve space.
                                         

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
            DebugPrint(("WdfRequestRetrieveInputBuffer failed %x\n", status));
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


//#define MOUSE_HID_HARDWARE          0x0080
//#define WHEELMOUSE_HID_HARDWARE     0x0100
//#define HORIZONTAL_WHEEL_PRESENT    0x8000


//BOOLEAN
//MouFilter_IsrHook(
//    PVOID         DeviceExtension, 
//    PMOUSE_INPUT_DATA       CurrentInput, 
//    POUTPUT_PACKET          CurrentOutput,
//    UCHAR                   StatusByte,
//    PUCHAR                  DataByte,
//    PBOOLEAN                ContinueProcessing,
//    PMOUSE_STATE            MouseState,
//    PMOUSE_RESET_SUBSTATE   ResetSubState
//)
///*++
//
//Remarks:
//    i8042prt specific code, if you are writing a packet only filter driver, you
//    can remove this function
//
//Arguments:
//
//    DeviceExtension - Our context passed during IOCTL_INTERNAL_I8042_HOOK_MOUSE
//    
//    CurrentInput - Current input packet being formulated by processing all the
//                    interrupts
//
//    CurrentOutput - Current list of bytes being written to the mouse or the
//                    i8042 port.
//                    
//    StatusByte    - Byte read from I/O port 60 when the interrupt occurred                                            
//    
//    DataByte      - Byte read from I/O port 64 when the interrupt occurred. 
//                    This value can be modified and i8042prt will use this value
//                    if ContinueProcessing is TRUE
//
//    ContinueProcessing - If TRUE, i8042prt will proceed with normal processing of
//                         the interrupt.  If FALSE, i8042prt will return from the
//                         interrupt after this function returns.  Also, if FALSE,
//                         it is this functions responsibilityt to report the input
//                         packet via the function provided in the hook IOCTL or via
//                         queueing a DPC within this driver and calling the
//                         service callback function acquired from the connect IOCTL
//                                             
//Return Value:
//
//    Status is returned.
//
//  --+*/
//{
//    PDEVICE_EXTENSION   devExt;
//    BOOLEAN             retVal = TRUE;
//
//    devExt = DeviceExtension;
//    
//    if (devExt->UpperIsrHook) {
//        retVal = (*devExt->UpperIsrHook) (devExt->UpperContext,
//                            CurrentInput,
//                            CurrentOutput,
//                            StatusByte,
//                            DataByte,
//                            ContinueProcessing,
//                            MouseState,
//                            ResetSubState
//            );
//
//        if (!retVal || !(*ContinueProcessing)) {
//            return retVal;
//        }
//    }
//    //unsigned int leftButton = 0, rigthButton = 0;
//
//   
//
//    //if (CurrentInput->ButtonFlags & MOUSE_RIGHT_BUTTON_DOWN)
//    //{
//    //    //rigthButton++;
//    //    (rigthButton == maxValue) ? rigthButton = 0 : rigthButton++;
//    //    DebugPrint(("Rigth botton %i", rigthButton));
//    //}
//    //else if (CurrentInput->ButtonFlags & MOUSE_LEFT_BUTTON_DOWN)
//    //{
//    //    //leftButton++;
//    //    (leftButton == maxValue) ? leftButton = 0: leftButton++;
//    //    DebugPrint(("Left botton %i", leftButton));
//    //}
//    
//
//    *ContinueProcessing = TRUE;
//    return retVal;
//}

    
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

    Called when there are mouse packets to report to the RIT.  You can do 
    anything you like to the packets.  For instance:
    
    o Drop a packet altogether
    o Mutate the contents of a packet 
    o Insert packets into the stream 
                    
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
    PWORKITEM_CONTEXT context;
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_WORKITEM_CONFIG workitemConfig;
    WDFWORKITEM hWorkItem;

    



    hDevice = WdfWdmDeviceGetWdfDeviceHandle(DeviceObject);

    devExt = FilterGetData(hDevice);
    static unsigned int leftButton = 0, rigthButton = 0;
    if (InputDataStart->ButtonFlags & MOUSE_RIGHT_BUTTON_DOWN)
    {
        //rigthButton++;
        (rigthButton == maxValue) ? rigthButton = 0 : rigthButton++;
        DebugPrint(("\n\nRigth botton %u\n\n", rigthButton));
        //const char* data = strcat("Rigth botton pressed, count= ", (char*)(rigthButton));

        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, WORKITEM_CONTEXT);
        attributes.ParentObject = hDevice;
        WDF_WORKITEM_CONFIG_INIT(&workitemConfig, ReadWriteWorkItem);
        status = WdfWorkItemCreate(&workitemConfig,
            &attributes,
            &hWorkItem);
        if (!NT_SUCCESS(status)) {
            return status;
        }
        context = GetWorkItemContext(hWorkItem);
        context->Device = hDevice;

        WdfWorkItemEnqueue(hWorkItem);


        status = MouFiltr_WriteToFile();
        if (NT_SUCCESS(status))
        {
            DbgPrint("\nYEAAP\n");
        }
           
    }
    else if (InputDataStart->ButtonFlags & MOUSE_LEFT_BUTTON_DOWN)
    {
        //leftButton++;
        (leftButton == maxValue) ? leftButton = 0 : leftButton++;
        DebugPrint(("Left botton %i", leftButton));
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
    /*for (PMOUSE_INPUT_DATA curData = InputDataStart; curData < InputDataEnd; curData++)
    {
        DebugPrint(("curData"));
        if (curData->ButtonFlags & MOUSE_RIGHT_BUTTON_DOWN)
        {
            DebugPrint(("Rigth button pressed"));
            DebugPrint(("Content of curData: \n X - %d; \n Y - %d;", curData->LastX, curData->LastY));
        }
        else if (curData->ButtonFlags & MOUSE_LEFT_BUTTON_DOWN)
            DebugPrint(("Left button pressed"));

    }*/
    
} 

#pragma warning(pop)



//VOID
//MouFiltr_EvtDriverContextCleanup(
//    IN WDFOBJECT Driver
//)
//{
//    //TraceEvents(TRACE_LEVEL_VERBOSE, DBG_INIT,
//    //   "Entered NonPnpEvtDriverContextCleanup\n");
//
//    PAGED_CODE();
//    KbPrint(("Enter in MouFiltr_EvtDriverContextCleanup"));
//    //
//    // No need to free the controldevice object explicitly because it will
//    // be deleted when the Driver object is deleted due to the default parent
//    // child relationship between Driver and ControlDevice.
//    //
//    //WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)Driver));
//
//}