#include "minifilter.h"
#include "KdRegistry.h"

const static UNICODE_STRING uniNamedPipe = RTL_CONSTANT_STRING(L"\\Device\\NamedPipe");

typedef enum _COOM_STR_TYPE {
	AR_TargetFileType,
	AR_BackUpFileType,
}COOM_STR_TYPE;

typedef struct _COOM_UNICODE_STRING_CONTEXT {
	COOM_STR_TYPE type;
	USHORT Length;       
	WCHAR Buffer[260];   
} COOM_UNICODE_STRING_CONTEXT, *PCOOM_UNICODE_STRING_CONTEXT;

PUNICODE_STRING gUniTargetName = NULL; 
PUNICODE_STRING gUniBackUpName = NULL; 

namespace MINIFILTER {
	auto Init(PDRIVER_OBJECT pDriverObj) -> bool{

		PSECURITY_DESCRIPTOR pSecDescriptor = NULL;
		OBJECT_ATTRIBUTES objAttr = { 0 };
		UNICODE_STRING uniPortName = { 0 };
		RtlInitUnicodeString(&uniPortName, R3portName);
		bool isSuccess = false;



		auto status = FltRegisterFilter(pDriverObj, &filterRegistration, &filterHandle);
		if (NT_SUCCESS(status)) {

			do {
				status = FltBuildDefaultSecurityDescriptor(&pSecDescriptor, FLT_PORT_ALL_ACCESS);
				CHECK_STATUS_AND_BREAK(status);
				InitializeObjectAttributes(&objAttr, &uniPortName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, pSecDescriptor);
				status = FltCreateCommunicationPort(filterHandle, &commPortHandle, &objAttr, NULL, fltConnectNotifyCallback, fltDisConnectNotifyCallback, NULL, 2);
				CHECK_STATUS_AND_BREAK(status);

				DebugPrint("[R0]: CreateCommunicationPort Success!\n");

				status = FltStartFiltering(filterHandle);
				CHECK_STATUS_AND_BREAK(status);
				isSuccess = true;

			} while (false);

			if (pSecDescriptor) {
				FltFreeSecurityDescriptor(pSecDescriptor);
			}



			if (!isSuccess) UnloadMinifilter();
		}

		return NT_SUCCESS(status);

	}

	auto fltConnectNotifyCallback (
	_In_ PFLT_PORT ClientPort,
		_In_opt_ PVOID ServerPortCookie,
		_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
		_In_ ULONG SizeOfContext,
		_Outptr_result_maybenull_ PVOID* ConnectionPortCookie
		)->NTSTATUS{

		clientHandle = ClientPort;
		DebugPrint("Client connected\n");

		NTSTATUS status = STATUS_UNSUCCESSFUL;
		UNICODE_STRING uniName = { 0 };

		if (ConnectionContext && SizeOfContext > 0) {
			PCOOM_UNICODE_STRING_CONTEXT context = (PCOOM_UNICODE_STRING_CONTEXT)ConnectionContext;
			if (context->Length <= sizeof(context->Buffer)) {

				switch (context->type) {
					case AR_TargetFileType: {
						if (!gUniTargetName) gUniTargetName = (PUNICODE_STRING)ExAllocatePool(PagedPool, sizeof(UNICODE_STRING));
						
						if (!gUniTargetName) {
							DebugPrint("Failed to allocate gUniTargetName\n");
							return STATUS_INSUFFICIENT_RESOURCES;
						}

						memset(gUniTargetName, 0, sizeof(UNICODE_STRING));

						gUniTargetName->Length = context->Length;
						gUniTargetName->MaximumLength = context->Length;
						if (gUniTargetName->Buffer) {
							ExFreePool(gUniTargetName->Buffer);
						}
						gUniTargetName->Buffer = (PWCH)ExAllocatePool(PagedPool, gUniTargetName->Length);
						if (gUniTargetName->Buffer) {
							memcpy(gUniTargetName->Buffer, context->Buffer, gUniTargetName->Length);
							DebugPrint("gUniTargetName: %wZ\n", gUniTargetName);
							status = STATUS_SUCCESS;
						}
						
						break;
					}
					case AR_BackUpFileType: {
						if (!gUniBackUpName) gUniBackUpName = (PUNICODE_STRING)ExAllocatePool(PagedPool, sizeof(UNICODE_STRING));
						if (!gUniBackUpName) {
							DebugPrint("Failed to allocate gUniBackUpName\n");
							return STATUS_INSUFFICIENT_RESOURCES;
						}

						memset(gUniBackUpName, 0, sizeof(UNICODE_STRING));

						gUniBackUpName->Length = context->Length;
						gUniBackUpName->MaximumLength = context->Length;
						if (gUniBackUpName->Buffer) {
							ExFreePool(gUniBackUpName->Buffer);
						}
						gUniBackUpName->Buffer = (PWCH)ExAllocatePool(PagedPool, gUniBackUpName->Length);
						if (gUniBackUpName->Buffer) {
							memcpy(gUniBackUpName->Buffer, context->Buffer, gUniBackUpName->Length);
							DebugPrint("gUniBackUpName: %wZ\n", gUniBackUpName);
							status = STATUS_SUCCESS;
						}
						break;
					}
				}
			}
		}


		return status;
	}

	auto fltDisConnectNotifyCallback(
		_In_opt_ PVOID ConnectionCookie
	) -> void {
		DebugPrint("Client disconnected\n");
		FltCloseClientPort(filterHandle, &clientHandle);
	}

	auto PreCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
		-> FLT_PREOP_CALLBACK_STATUS {

		// auto createStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
		auto createStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
		PECP_LIST ecpList = NULL;
		do {

		if (NT_SUCCESS(FltGetEcpListFromCallbackData(filterHandle, data, &ecpList)) && ecpList != NULL) {

			if (HELPER::checkEspListHasKernelGuid(filterHandle, ecpList, &GUID_ECP_PREFETCH_OPEN_FIX_VS_SHIT)) break;
			if (HELPER::checkEspListHasKernelGuid(filterHandle, ecpList, &GUID_ECP_CSV_DOWN_LEVEL_OPEN_FIX_WIN7)) break;

		}

		
			if (FlagOn(data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE)) break; // open pageing_file
			
			const auto pid = PsGetCurrentProcessId();
			if ((ULONG)pid <= 4) break; // SYSTEM

			if (FLT_IS_FASTIO_OPERATION(data)) break; // FastIO

			if (data->Iopb->MajorFunction == IRP_MJ_CREATE_NAMED_PIPE) break; // namedPipe


		} while (false);

		return createStatus;
	}

	auto PostCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags)
	-> FLT_POSTOP_CALLBACK_STATUS{

		do {
			if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING)) break;
			if (data->IoStatus.Status == STATUS_REPARSE) break;
			if (KeGetCurrentIrql() >= DISPATCH_LEVEL) break;
			if (!FltSupportsStreamHandleContexts(FltObjects->FileObject)) break;
			_Minifilter_Stream_Context* streamCtx = createStreamCtx(FltObjects, data, false);

		} while (false);

		
		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	auto freeStreamCtx(_Minifilter_Stream_Context* streamCtx) -> void {
		if (streamCtx->rawPath) {
			HELPER::FreeCopiedString(streamCtx->rawPath);
			streamCtx->rawPath = NULL;
		}
	}

	auto createStreamCtx(PCFLT_RELATED_OBJECTS fltObjects, PFLT_CALLBACK_DATA data, bool isInPreCallbacks = false)
	-> _Minifilter_Stream_Context* {
		PFLT_CONTEXT fltCtxPtr = NULL;
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		_Minifilter_Stream_Context* streamCtx = NULL;
		PFLT_FILE_NAME_INFORMATION nameInfo = NULL;
		bool isSuccess = false;

		do{
			status = FltAllocateContext(fltObjects->Filter, FLT_STREAMHANDLE_CONTEXT, sizeof(_Minifilter_Stream_Context), NonPagedPoolNx, &fltCtxPtr);
			
			CHECK_STATUS_AND_BREAK(status);

			streamCtx = (_Minifilter_Stream_Context*)fltCtxPtr;
			memset(streamCtx, 0, sizeof(_Minifilter_Stream_Context));

			status = FltSetStreamHandleContext(fltObjects->Instance, fltObjects->FileObject, FLT_SET_CONTEXT_REPLACE_IF_EXISTS
				, fltCtxPtr, NULL);

			CHECK_STATUS_AND_BREAK(status);

			status = FltGetFileNameInformation(data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &nameInfo);

			CHECK_STATUS_AND_BREAK(status);
			

			streamCtx->pid = PsGetCurrentProcessId();
			status = HELPER::CopyUnicodeStringWithAllocPagedMem(&streamCtx->rawPath, &nameInfo->Name);
			CHECK_STATUS_AND_BREAK(status);

			isSuccess = true;


		} while (false);
		if (!isSuccess) {
			freeStreamCtx(streamCtx);
			streamCtx = NULL;
		}
		if (fltCtxPtr) {
			FltReleaseContext(fltCtxPtr);
		}
		if (nameInfo) {
			FltReleaseFileNameInformation(nameInfo);
		}
		

		return streamCtx;

	}


	auto PostWriteOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) -> FLT_POSTOP_CALLBACK_STATUS{
		_Minifilter_Stream_Context* streamCtx = NULL;

		do {
			if (KeGetCurrentIrql() >= DISPATCH_LEVEL) break;
			const auto pid = PsGetCurrentProcessId();
			if ((ULONG)pid <= 4) break;

			if (!NT_SUCCESS(FltGetStreamHandleContext(FltObjects->Instance, FltObjects->FileObject, (PFLT_CONTEXT*)&streamCtx))) break;
			if (!streamCtx) break;

			
			if (streamCtx->rawPath && RtlPrefixUnicodeString(&uniNamedPipe, streamCtx->rawPath, true)) streamCtx->type = _FileAccessType::kPipe;

			streamCtx->hasWrite = true;

			if (gUniTargetName) {

				if (streamCtx->rawPath && RtlSuffixUnicodeString(gUniTargetName, streamCtx->rawPath, true)) {
					DebugPrint("try write %wZ \n", streamCtx->rawPath);
					DebugPrint("try to backup \n");
					NTSTATUS status = STATUS_UNSUCCESSFUL;


					LARGE_INTEGER fileSize;
					status = FsRtlGetFileSize(FltObjects->FileObject, &fileSize);

					DebugPrint("file size: %llu\n", fileSize.QuadPart);
					if (fileSize.QuadPart == 0) {
						return FLT_POSTOP_FINISHED_PROCESSING;
					}


					HANDLE hReadFile = NULL;
					PFILE_OBJECT fReadObj = NULL;
					OBJECT_ATTRIBUTES objAttr = { 0 };
					InitializeObjectAttributes(&objAttr, streamCtx->rawPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
					IO_STATUS_BLOCK ioSb = { 0 };


					HANDLE hWriteFile = NULL;
					PFILE_OBJECT fWriteObj = NULL;
					OBJECT_ATTRIBUTES objAttr2 = { 0 };
					InitializeObjectAttributes(&objAttr2, gUniBackUpName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
					IO_STATUS_BLOCK ioSb2 = { 0 };
					HANDLE hSection = NULL;


					ULONG size = 1 << 20;
					ULONG out = 0;
					ULONG readOffset = 0;
					PVOID buf = NULL;


					do {

						ULONG64 remainBytes = fileSize.QuadPart;
						buf = ExAllocatePool(PagedPool, remainBytes);
						if (!buf) {
							status = STATUS_NO_MEMORY;
							break;
						}

						status = FltCreateFileEx(FltObjects->Filter, FltObjects->Instance,
							&hReadFile, &fReadObj, GENERIC_READ, &objAttr, &ioSb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
							FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY, NULL, 0, IO_IGNORE_SHARE_ACCESS_CHECK);
						CHECK_STATUS_AND_BREAK(status);

						status = FltCreateFileEx(FltObjects->Filter, FltObjects->Instance,
							&hWriteFile, &fWriteObj, GENERIC_WRITE, &objAttr2, &ioSb2, NULL, FILE_ATTRIBUTE_NORMAL, 0,
							FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY, NULL, 0, 0);
						CHECK_STATUS_AND_BREAK(status);


						while (remainBytes) {
							status = FltReadFile(FltObjects->Instance, fReadObj, 0, min(size, remainBytes), buf, 0, &out, NULL, NULL);
							CHECK_STATUS_AND_BREAK(status);

							status = FltWriteFile(FltObjects->Instance, fWriteObj, 0, out, buf, 0, NULL, NULL, NULL);
							CHECK_STATUS_AND_BREAK(status);

							remainBytes -= out;
						}


					} while (false);
					if (buf) {
						ExFreePool(buf);
					}if (hReadFile) {
						FltClose(hReadFile);
					}if (hWriteFile) {
						FltClose(hWriteFile);
					}if (fReadObj) {
						ObDereferenceObject(fReadObj);
					}if (fWriteObj) {
						ObDereferenceObject(fWriteObj);
					}

				}
			}

		} while (false);
		if (streamCtx) FltReleaseContext((PFLT_CONTEXT)streamCtx);

		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	auto MinifilterContextCleanUp(PFLT_CONTEXT Context,FLT_CONTEXT_TYPE ContextType) -> void{
		UNREFERENCED_PARAMETER(ContextType);
		freeStreamCtx((_Minifilter_Stream_Context*)Context);
	}

	auto UnloadMinifilter() -> void {
		if (clientHandle) {
			FltCloseClientPort(filterHandle, &clientHandle);
			clientHandle = NULL;
		}

		if (commPortHandle) {
			FltCloseCommunicationPort(commPortHandle);
			commPortHandle = NULL;
		}

		if (filterHandle) {
			FltUnregisterFilter(filterHandle);
			filterHandle = NULL;
		}
		if (gUniBackUpName) {
			HELPER::FreeCopiedString(gUniBackUpName);
			gUniBackUpName = NULL;
		}
		if (gUniTargetName) {
			HELPER::FreeCopiedString(gUniTargetName);
			gUniTargetName = NULL;
		}
	}

	auto MinifilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags) -> NTSTATUS{
		UnloadMinifilter();
		return STATUS_SUCCESS;
	}

	auto InitializeMinifilterRegedit(PUNICODE_STRING RegPath, PCWSTR Altitude) -> NTSTATUS {
		NTSTATUS status = STATUS_SUCCESS;
		WCHAR keyPath[255] = { 0 };
		HANDLE keyHandle = (HANDLE)-1;
		WCHAR Instance[] = L"MinifilterDemo";
		ULONG Flags = 0;

		RtlCopyMemory(keyPath, RegPath->Buffer, RegPath->Length);
		wcscat_s(keyPath, 255, L"\\Instances");
		status = KDREGISTRY::CreateValueKey(keyPath, (WCHAR*)L"DefaultInstance", REG_SZ, Instance, sizeof(Instance));

		if (NT_SUCCESS(status)) {
			wcscat_s(keyPath, 255, L"\\MinifilterDemo");
			status = KDREGISTRY::CreateValueKey(keyPath, (WCHAR*)L"Altitude", REG_SZ, (PVOID)Altitude, (ULONG)wcslen(Altitude)*2);
			
			if (NT_SUCCESS(status)) {
				status = KDREGISTRY::CreateValueKey(keyPath, (WCHAR*)L"Flags", REG_DWORD, &Flags, 4);

			}
		}
		return status;
	}


};