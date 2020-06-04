
#include <ntddk.h>
#include "Header.h"

#define IOCTL_SSDT_N	CTL_CODE(FILE_DEVICE_UNKNOWN,0x4001,METHOD_NEITHER,FILE_ANY_ACCESS)
#define IOCTL_GETKEY_N	CTL_CODE(FILE_DEVICE_UNKNOWN,0x4002,METHOD_NEITHER,FILE_ANY_ACCESS)
#define IOCTL_TEST_N	CTL_CODE(FILE_DEVICE_UNKNOWN,0x4003,METHOD_NEITHER,FILE_ANY_ACCESS)

#define IOCTL_TEST_B	CTL_CODE(FILE_DEVICE_UNKNOWN,0x4003,METHOD_BUFFERED,FILE_ANY_ACCESS)
//user
#define DRV_NAME L"\\\\.\\wooulog"
#define DWORD unsigned long

//kernel
#define DEV_LINK L"\\DosDevices\\wooulog"
#define DEV_NAME L"\\Device\\wooulog"

#define IO_BUF_SIZE 100

extern int irp_ctrlCount;

NTSTATUS myCtrlCreateLink(PDRIVER_OBJECT DriverObject, LPWSTR devName, LPWSTR linkName);
NTSTATUS myCtrlControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS myCtrlCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
void myCtrlDelete();
