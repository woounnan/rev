#include "ioctl.h"
#include "ssdt.h"
#include <string.h>
#include "irp.h"
#define TIME_SECOND(x) x*10000000*-1
#pragma warning(disable: 4047)
PDEVICE_OBJECT p_ctrlDev= NULL;
UNICODE_STRING u_linkName;
int irp_ctrlCount = 0;
CHAR flag_one = FALSE;

NTSTATUS myCtrlCreateLink(PDRIVER_OBJECT DriverObject, LPWSTR devName, LPWSTR linkName)
{
	NTSTATUS ntStat = STATUS_UNSUCCESSFUL;
	UNICODE_STRING u_devName;
	RtlInitUnicodeString(&u_devName, devName);
	RtlInitUnicodeString(&u_linkName, linkName);

	ntStat = IoCreateDevice(
		DriverObject,
		0,
		&u_devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&p_ctrlDev
	);
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "ctrlDev:%x\n", p_ctrlDev);
	if (!NT_SUCCESS(ntStat))
	{
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "IOCTL::Failed to IoCreateDevice #%x @@@\n", ntStat);
		return ntStat;
	}
	ntStat = IoCreateSymbolicLink(&u_linkName, &u_devName);
	if (!NT_SUCCESS(ntStat))
	{
		IoDeleteDevice(p_ctrlDev);
		DbgPrintEx(DPFLTR_ACPI_ID, 0, "IOCTL::Failed to IoCreateSymbolicLink #%x @@@\n", ntStat);
		return ntStat;
	}

	DbgPrintEx(DPFLTR_ACPI_ID, 0, "success createDeviceObj\n");
	return ntStat;
}


NTSTATUS myCtrlControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	PIO_STACK_LOCATION pStack;
	NTSTATUS returnStatus = STATUS_SUCCESS;
	ULONG ControlCode;
	//int i = 0;
	int outLen = 0;
	LPWSTR p_out;
	//PCHAR outBuf;
	//PCHAR data = "ah ssibal";
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "call myCtrlControl()\n");
	irpCount++;
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "irpCount:%x\n", irpCount);
	pStack = IoGetCurrentIrpStackLocation(Irp);
	ControlCode = pStack->Parameters.DeviceIoControl.IoControlCode;


	switch (ControlCode)
	{
		case IOCTL_SSDT_N:
			DbgPrintEx(DPFLTR_ACPI_ID, 0, " IOCTL_SSDT Call\n");
			p_out = (LPWSTR)Irp->UserBuffer;

			
			outLen = pStack->Parameters.DeviceIoControl.OutputBufferLength;

			if (outLen <= 0)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, 0, "There is no transmitted data.\n");
				break;
			}

			//후킹함수에서 wp_procName을 사용하기 위해 동적할당.
			wp_procName=(LPWSTR)ExAllocatePool(NonPagedPool, sizeof(WCHAR)*outLen);
			
			RtlZeroMemory(wp_procName, sizeof(WCHAR)*outLen);
			memcpy(wp_procName, p_out, outLen*2);
			
			if (statHook == FALSE)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, 0, "statHook == FALSE\n");
				CHAR buf[] = "Successful hooking";
				hookSSDT();
				statHook = TRUE;
				memcpy(pStack->Parameters.DeviceIoControl.Type3InputBuffer, buf, sizeof(buf));
				Irp->IoStatus.Information = sizeof(buf);
			}
			else
			{
				DbgPrintEx(DPFLTR_ACPI_ID, 0, "statHook == TRUE\n");
				CHAR buf[] = "It is in a state already hooked";
				memcpy(pStack->Parameters.DeviceIoControl.Type3InputBuffer, buf, sizeof(buf));
				Irp->IoStatus.Information = sizeof(buf);
			}
			
			break;

		case IOCTL_GETKEY_N:
			DbgPrintEx(DPFLTR_ACPI_ID, 0, "\n Calling IOCTL_GETKEY_N \n");
			PMYDEVICE_EXTENSION pMyDevExt = (PMYDEVICE_EXTENSION)DeviceObject->DriverObject->DriverExtension;
			PLIST_ENTRY pListNext = pMyDevExt->listHead->Blink;

			KTIMER kTimer;
			LARGE_INTEGER timeout;
			timeout.QuadPart = (LONGLONG)TIME_SECOND(0.1);

				
			while (pMyDevExt->listHead == NULL)
			{
				DbgPrintEx(DPFLTR_ACPI_ID, 0, "Data does not exist\n"); 
				KeInitializeTimer(&kTimer);

				KeSetTimer(&kTimer, timeout, NULL);
				KeWaitForSingleObject(&kTimer, Executive, KernelMode, FALSE, NULL);

			}
			if (pListNext == NULL)
				flag_one = TRUE;
			memcpy(pStack->Parameters.DeviceIoControl.Type3InputBuffer, ((PKEY_DATA)pMyDevExt->listHead)->keyCode, sizeof(((PKEY_DATA)pMyDevExt->listHead)->keyCode));
			Irp->IoStatus.Information = sizeof(((PKEY_DATA)pMyDevExt->listHead)->keyCode);
			ExFreePool(pMyDevExt->listHead);
			pMyDevExt->listHead = pListNext;
			flag_one = FALSE;
			break;
	}
	irpCount--;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	DbgPrintEx(DPFLTR_ACPI_ID, 0, "after myCtrlControl()\n");
	return returnStatus;
}

NTSTATUS myCtrlCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	irpCount++;
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "call myCtrlCreate()\n");
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "irpCount:%x\n", irpCount);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	irpCount--;
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "after myCtrlCreate()\n");
	return STATUS_SUCCESS;
}

void myCtrlDelete()
{
	IoDeleteSymbolicLink(&u_linkName);
	IoDeleteDevice(p_ctrlDev);
}