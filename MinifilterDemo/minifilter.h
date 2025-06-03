#pragma once
#include "helper.h"

#define MINIFILTER_TAG 'TFLT'
#define R3portName L"\\TestPort"


static PFLT_FILTER filterHandle;
static PFLT_PORT clientHandle;
static PFLT_PORT commPortHandle;

namespace MINIFILTER {
	static const auto RegistryPath = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\MinifilterDemo";
	static const auto Altitude = L"370020";

	enum class _FileAccessType {
		kNomal,
		KRawAccess,
		kPipe
	};

	struct _Minifilter_Stream_Context{
		PUNICODE_STRING rawPath;
		PUNICODE_STRING fixPath;
		PUNICODE_STRING dosVolumePath;
		PUNICODE_STRING VolumePath;
		HANDLE pid;
		HANDLE requestorPid;
		bool isPreStream;
		bool hasRead;
		bool hasWrite;
		_FileAccessType type;
	};

	auto PreCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS FltObjects,
		PVOID* CompletionContext) -> FLT_PREOP_CALLBACK_STATUS;

	auto PostCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags)
		-> FLT_POSTOP_CALLBACK_STATUS;
	
	auto createStreamCtx(PCFLT_RELATED_OBJECTS fltObjects, PFLT_CALLBACK_DATA data, bool isInPreCallbacks)
		-> _Minifilter_Stream_Context* ;

	
	auto PostWriteOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags)
		-> FLT_POSTOP_CALLBACK_STATUS;


	auto MinifilterContextCleanUp(PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType) -> void;
	auto MinifilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags) -> NTSTATUS;
	
	auto Init(PDRIVER_OBJECT pDriverObj) -> bool;

	auto fltConnectNotifyCallback(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext, PVOID* ConnectionPortCookie) -> NTSTATUS;

	auto fltDisConnectNotifyCallback(PVOID ConnectionCookie) -> void;
	
	auto UnloadMinifilter() -> void;

	auto InitializeMinifilterRegedit(PUNICODE_STRING RegPath, PCWSTR Altitude) -> NTSTATUS;

	static const FLT_OPERATION_REGISTRATION callbacks[] = {
		{IRP_MJ_CREATE,0,(PFLT_PRE_OPERATION_CALLBACK)PreCreateOperation,(PFLT_POST_OPERATION_CALLBACK)PostCreateOperation},
		{IRP_MJ_WRITE,0,NULL,(PFLT_POST_OPERATION_CALLBACK)PostWriteOperation},
		{IRP_MJ_OPERATION_END}
	};

	const FLT_CONTEXT_REGISTRATION contextRegistration[] = {
		{FLT_STREAMHANDLE_CONTEXT,0,MinifilterContextCleanUp,sizeof(_Minifilter_Stream_Context),MINIFILTER_TAG},
		{FLT_CONTEXT_END}
	};



	const FLT_REGISTRATION filterRegistration = {
		sizeof(FLT_REGISTRATION),
		FLT_REGISTRATION_VERSION,
		FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS,
		contextRegistration,
		callbacks,
		MinifilterUnload,
		NULL,NULL,NULL,NULL,NULL,NULL,NULL
	};
};