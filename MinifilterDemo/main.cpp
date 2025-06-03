#include "minifilter.h"
#include "callbacks.h"



NTSTATUS UnloadDriver(PDRIVER_OBJECT DriverObject)
{
    DbgPrintEx(77, 0, "Unload!\n");
    CALLBACKS::Uninstall();

    return STATUS_SUCCESS;
}

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    DriverObject->DriverUnload = (PDRIVER_UNLOAD)UnloadDriver;

    PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)((ULONG_PTR)DriverObject->DriverSection);
    ldr->Flags |= 0x20;

    UNICODE_STRING uniRegPath = { 0 };
    RtlInitUnicodeString(&uniRegPath, MINIFILTER::RegistryPath);
    
    MINIFILTER::InitializeMinifilterRegedit(&uniRegPath, MINIFILTER::Altitude);


    if (!CALLBACKS::Init(DriverObject)) return STATUS_UNSUCCESSFUL;

    DebugPrint("Success\n");
    return STATUS_SUCCESS;
}