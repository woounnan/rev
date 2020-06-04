#include <ntddk.h>
#include <stdio.h>
#define _ssdh_h_
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

#define HOOK_SYSCALL(_Function, _Hook, _Orig )  \
       _Orig = (PVOID) InterlockedExchange( (PLONG) &KeServiceDescriptorTable.ServiceTableBase[SYSCALL_INDEX(_Function)], (LONG) _Hook)

#define UNHOOK_SYSCALL(_Function, _Orig )  \
       InterlockedExchange( (PLONG) &KeServiceDescriptorTable.ServiceTableBase[SYSCALL_INDEX(_Function)], (LONG) _Orig)
//#define TARGET_PROCESS L"notepad.exe"
#pragma warning(disable: 4213)
#pragma warning(disable: 4054)
#pragma warning(disable: 4152)

typedef struct _IO_COUNTERS2 {
	ULONGLONG  ReadOperationCount;
	ULONGLONG  WriteOperationCount;
	ULONGLONG  OtherOperationCount;
	ULONGLONG ReadTransferCount;
	ULONGLONG WriteTransferCount;
	ULONGLONG OtherTransferCount;
} IO_COUNTERS2;

typedef struct _VM_COUNTERS2 {
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} VM_COUNTERS2;

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
	VM_COUNTERS2                     VmCounters;
	IO_COUNTERS2                     IoCounters; //windows 2000 only
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

LARGE_INTEGER m_UserTime;
LARGE_INTEGER m_KernelTime;

//extern LPWSTR cp_procName;
extern LPSTR p_procName;
extern LPWSTR wp_procName;

extern BOOLEAN statHook;

NTSTATUS NewZwQuerySystemInformation(
	IN ULONG SystemInformationClass,
	IN PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength);


VOID WP_OFF();
VOID WP_ON();

VOID unHookSSDT();
NTSTATUS hookSSDT();