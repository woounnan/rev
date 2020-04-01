
#include <Header.h>
#include "getCode.h"
#pragma once

#define KEYBOARD_DEVICE_NAME	L"\\Device\\KeyboardClass0"
#define BYTE unsigned char


#pragma warning(disable:4113)
#pragma warning(disable:4047)
//#pragma warning(disable:4242)
#pragma once

extern int irpCount;
extern USHORT scanCode;

NTSTATUS myRead(IN PDEVICE_OBJECT MyDevObj, IN PIRP	Irp);
NTSTATUS getDriverObject(PCWSTR deviceName, PDRIVER_OBJECT *pdrv_obj);
NTSTATUS myAddDevice(PDRIVER_OBJECT DriverObject, PCWCHAR TargetDeviceName);
NTSTATUS passDown(IN PDEVICE_OBJECT MyDevObj, IN PIRP Irp);
NTSTATUS readCompletion(PDEVICE_OBJECT DeviceObject, PIRP irp, PVOID Context);
void myIrpDelete(PDRIVER_OBJECT DriverObject);
