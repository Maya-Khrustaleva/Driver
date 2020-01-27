#pragma once
#include <evntrace.h>                      
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID( \
        MoufiltrTraceGuid, (4D36E96F,E325,11CE,BFC1,08002BE10318),   \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)   /* bit 0 = 0x00000001 */ \
        WPP_DEFINE_BIT(TRACE_DRIVER)        /* bit 1 = 0x00000002 */ \
        WPP_DEFINE_BIT(TRACE_DEVICE)        /* bit 2 = 0x00000004 */ \
        WPP_DEFINE_BIT(TRACE_QUEUE)         /* bit 3 = 0x00000008 */ \
        WPP_DEFINE_BIT(TRACE_ERROR)         /* bit 4 = 0x00000010 */ \
        WPP_DEFINE_BIT(TRACE_INFORMATION)   /* bit 5 = 0x00000020 */ \
        WPP_DEFINE_BIT(TRACE_VERBOSE)       /* bit 6 = 0x00000040 */ \
        )

/*FunctionName(Condition1, Condition2,..., "Message", MessageVariables...)
�������� ���������� ����������� ������� ��������� ����������� ���������:
 - Condition1, Condition2 � ���� ��� ��������� �������, ����������� ��������. ��� ����-
�����, ������ ����� ����������, ��������������� ��� �������. ��������� ���������-
�� ������������, ������ ���� ����������� ��� �������;
 - Message � ��������� ���������, ������������ ������ ��������� �����������. �����-
�������� ������� ����� ��, ��� � ��� ������� DoTraceMessage;
 - MessageVariables � ������ ����������� �������� ������������ ��������� �������-
���, ��� �������� ����������� � ��������� � ������������ � ��������, ��������� �
��������� Message.*/


/*// FUNC TraceError{LEVEL=TRACE_ERROR,FLAGS=Error}(MSG,...);
// FUNC TraceData{LEVEL=TRACE_VERBOSE,FLAGS=DataFlow}(MSG,...);
// FUNC TraceDriverStatus{LEVEL=TRACE_INFORMATION,FLAGS=DriverStatus}(MSG,...);*/

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level  >= lvl)


//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
//
// end_wpp
//

