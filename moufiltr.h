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
#include "Trace.h" // contains macros for WPP tracing

#if DBG

#define TRAP()          DbgBreakPoint()

#define DebugPrint(_x_) DbgPrint _x_

#else   // DBG

#define TRAP()

#define DebugPrint(_x_)

#endif

#define typeCountPressButton int

 
typedef struct _DEVICE_EXTENSION
{     
    //
    // The real connect data that this driver reports to
    //
    CONNECT_DATA UpperConnectData;

    UNICODE_STRING fileName;

    //
    //Button counter
    //
    typeCountPressButton count;

    //
    //The field contains information about which button is pressed
    //
    ANSI_STRING Button;
    

  
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

typedef struct _WORKITEM_CONTEXT
{
    WDFDEVICE Device;

}WORKITEM_CONTEXT, * PWORKITEM_CONTEXT;




WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, FilterGetData)
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WORKITEM_CONTEXT, GetWorkItemContext)

//
// Prototypes
//
DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD MouFilter_EvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL MouFilter_EvtIoInternalDeviceControl;
EVT_WDF_OBJECT_CONTEXT_CLEANUP MouFiltr_EvtDriverContextCleanup;
EVT_WDF_WORKITEM Moufiltr_EvtWriteWorkItem;

VOID GetMaxValue(typeCountPressButton*);

VOID
MouFilter_DispatchPassThrough(
     _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target
    );

VOID
MouFilter_ServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA InputDataStart,
    IN PMOUSE_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    );

VOID
MouFiltr_EvtDriverContextCleanup(
    IN WDFOBJECT Driver
);

#endif  // MOUFILTER_H


