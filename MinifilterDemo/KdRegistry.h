#pragma once
#include "helper.h"

namespace KDREGISTRY {
	NTSTATUS NTAPI SetValueKey(WCHAR* RegKey, WCHAR* KeyName, ULONG ValueClass, PVOID Value, int vlen);
	NTSTATUS NTAPI CreateValueKey(WCHAR* RegKey, WCHAR* KeyName, ULONG ValueClass, PVOID Value, int vlen);
};