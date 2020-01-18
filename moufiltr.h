/*++
Copyright (c) 2008  Microsoft Corporation

Module Name:

    moufiltr.h

Abstract:

    This module contains the common private declarations for the mouse
    packet filter

Environment:

    kernel mode only

Notes:


Revision History:


--*/

#ifndef MOUFILTER_H
#define MOUFILTER_H

#include <ntddk.h>
#include <kbdmou.h>
#include <ntddmou.h>
#include <ntdd8042.h>
#include <wdf.h>
#include"Ntstrsafe.h"
//#include "Trace.h" // contains macros for WPP tracing

#if DBG

#define TRAP()                      DbgBreakPoint()

#define DebugPrint(_x_) DbgPrint _x_

#else   // DBG

#define TRAP()

#define DebugPrint(_x_)

#endif

#define typeCountPressButton int
#define maxValue 255 
 
typedef struct _DEVICE_EXTENSION
{
 
     //
    // Previous hook routine and context
    //                               
    PVOID UpperContext;
     
    //
    // Context for IsrWritePort, QueueMousePacket
    //
    IN PVOID CallContext;

    //
    // The real connect data that this driver reports to
    //
    CONNECT_DATA UpperConnectData;

    UNICODE_STRING fileName;
    typeCountPressButton count;
    //char Button[6];
    ANSI_STRING Button;
    //TODO: добавить передачу с использованием буфура

  
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _WORKITEM_CONTEXT
{
    WDFDEVICE Device;

}WORKITEM_CONTEXT, * PWORKITEM_CONTEXT;




WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION,
    FilterGetData)//объявление типа области контекста
/*Макросы для объявления типа контекста ассоциируют тип с областью контекста и создают
именованный метод доступа, который возвращает указатель на область контекста.
Макрос WDF_DECLARE_CONTEXT_TYPE_WITH_NAME назначает указанное драйвером имя методу дос-
тупа.*/

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WORKITEM_CONTEXT, GetWorkItemContext)

//
// Prototypes
//
DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD MouFilter_EvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL MouFilter_EvtIoInternalDeviceControl;
//EVT_WDF_IO_QUEUE_IO_READ MouFiltr_EvtIoRead;
EVT_WDF_WORKITEM Moufiltr_EvtWriteWorkItem;

VOID
MouFilter_DispatchPassThrough(
     _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target
    );

//_IRQL_requires_max_(PASSIVE_LEVEL);
//NTSTATUS MouFiltr_WriteToFile();

VOID
MouFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA InputDataStart,
    IN PMOUSE_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

#endif  // MOUFILTER_H


