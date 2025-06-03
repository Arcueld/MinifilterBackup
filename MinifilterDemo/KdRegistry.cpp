#include "KdRegistry.h"

namespace KDREGISTRY {
	NTSTATUS NTAPI SetValueKey(WCHAR* RegKey, WCHAR* KeyName, ULONG ValueClass, PVOID Value, int vlen){
		PAGED_CODE();

		NTSTATUS status = STATUS_UNSUCCESSFUL;

		UNICODE_STRING uniKey = { 0 };
		RtlInitUnicodeString(&uniKey, RegKey);

		OBJECT_ATTRIBUTES objAttr = { 0 };
		InitializeObjectAttributes(&objAttr, &uniKey, OBJ_CASE_INSENSITIVE, NULL, NULL);

		HANDLE hKey = NULL;
		status = ZwOpenKey(&hKey, GENERIC_ALL, &objAttr);

		CHECK_STATUS_AND_RETURN(status,NOTHING,status);

		UNICODE_STRING uniValueName = { 0 };
		RtlInitUnicodeString(&uniValueName, KeyName);

		status = ZwSetValueKey(hKey, &uniValueName, NULL, ValueClass, Value, vlen);

		CHECK_STATUS_AND_RETURN(status, ZwClose(hKey), status);
		
		ZwClose(hKey);

		return status;
	}

	NTSTATUS NTAPI CreateValueKey(WCHAR* RegKey, WCHAR* KeyName, ULONG ValueClass, PVOID Value, int vlen) {
		PAGED_CODE();
		NTSTATUS status = STATUS_UNSUCCESSFUL;

		UNICODE_STRING uniReg = { 0 };
		RtlInitUnicodeString(&uniReg, RegKey);

		OBJECT_ATTRIBUTES objAttr = { 0 };
		InitializeObjectAttributes(&objAttr, &uniReg, OBJ_CASE_INSENSITIVE, NULL, NULL);

		ULONG uRes = NULL;
		HANDLE hReg = NULL;

		status = ZwCreateKey(&hReg, KEY_ALL_ACCESS, &objAttr, 0, NULL, REG_OPTION_NON_VOLATILE, &uRes);

		if (NT_SUCCESS(status)) {
			ZwClose(hReg);
			status = SetValueKey(RegKey, KeyName, ValueClass, Value, vlen);
		}
		
		return status;

	}
};