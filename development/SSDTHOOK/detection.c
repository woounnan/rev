void FindSSDTHook(void)

{

 unsigned int i;

 DbgPrint("start FindSSDTHook()\n");

 for (i = 0; i < KeServiceDescriptorTable.NumberOfServices; i++)

 {

  if ((KeServiceDescriptorTable.ServiceTableBase[i] < G_ntoskrnl.Base)

   || (KeServiceDescriptorTable.ServiceTableBase[i] > G_ntoskrnl.End))

  {

   DbgPrint("시스템 콜:[%d]이 [%x]로 후킹되었습니다. \n", i, KeServiceDescriptorTable.ServiceTableBase[i]);

  }

 }

}



PMODULE_LIST GetListOfModules(PNTSTATUS pns)

{

 ULONG moduleTotalSize;

 ULONG *ModuleListAddr = NULL;

 NTSTATUS ns;

 PMODULE_LIST pml = NULL;



 //모듈 정보 크기 구함.

 ZwQuerySystemInformation(SystemModuleInformation,

  &moduleTotalSize,

  0,

  &moduleTotalSize

 );



 //모듈 정보 담을 메모리 공간 할당.

 ModuleListAddr = (ULONG *)ExAllocatePool(PagedPool,
  moduleTotalSize);



 if (!ModuleListAddr) // ExAllocatePool 실패 

 {

  if (pns != NULL)

   *pns = STATUS_INSUFFICIENT_RESOURCES;



  return (PMODULE_LIST)ModuleListAddr;

 }



 ns = ZwQuerySystemInformation(

  SystemModuleInformation,

  ModuleListAddr,

  moduleTotalSize,

  0

 );



 if (ns != STATUS_SUCCESS)// ZwQuerySysternlnformation 실패
 {
  // 할당한 메모리 해제
  ExFreePool((PVOID)ModuleListAddr);

  if (pns != NULL)
   *pns = ns;
  return NULL;

 }

 pml = (PMODULE_LIST)ModuleListAddr;



 if (pns != NULL)
  *pns = ns;

 return pml;

}




NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)

{

 int count;

 G_PModuleList = NULL;

 G_ntoskrnl.Base = 0;

 G_ntoskrnl.End = 0;

 G_PModuleList = GetListOfModules(NULL);



 UNREFERENCED_PARAMETER(RegistryPath);



 DriverObject->DriverUnload = OnUnload;

 if (!G_PModuleList)

  return STATUS_UNSUCCESSFUL;



 for (count = 0; count < G_PModuleList->ModuleCount; count++)
 {
  //find ntoskrnl.exe
  if (_stricmp("ntoskrnl.exe", (char*)G_PModuleList->ModuleInfo[count].ModulePath + G_PModuleList->ModuleInfo[count].NameOffset) == 0)
  {

   DbgPrint("Wow, find ntoskrnl.exe!!\n");

   DbgPrint("ntoskrnl.exe\n");

   G_ntoskrnl.Base = (DWORD)G_PModuleList->ModuleInfo[count].Base;

   G_ntoskrnl.End = ((DWORD)G_PModuleList->ModuleInfo[count].Base + G_PModuleList->ModuleInfo[count].Size);

   DbgPrint("start Addr:%x\n", G_ntoskrnl.Base);

   DbgPrint("end Addr:%x\n", G_ntoskrnl.End);

  }

 }

 ExFreePool(G_PModuleList);



 if (G_ntoskrnl.Base != 0)

 {

  FindSSDTHook();

  return STATUS_SUCCESS;

 }

 else

 {

  return STATUS_UNSUCCESSFUL;

 }

}
