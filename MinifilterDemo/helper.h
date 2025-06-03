#pragma once
#include <ntifs.h>
#include <fltKernel.h>
#include <intrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ndis.h>
#include <ntimage.h>
#include <bcrypt.h>


#define DebugPrint(...) \
    DbgPrintEx(77, 0, __VA_ARGS__)

#define CHECK_STATUS_AND_BREAK(s)                                      \
    if (!NT_SUCCESS(s)) {                                              \
        DbgPrintEx(77, 0, "[%s:%d] status: 0x%x\n",                   \
                   __FUNCTION__, __LINE__, (ULONGLONG)(s));            \
        break;                                                         \
    }

#define CHECK_STATUS_AND_RETURN(s, cleanup_expr, ...)          \
    if (!NT_SUCCESS(s)) {                                              \
        DbgPrintEx(77, 0, "[%s:%d] status: 0x%x\n",                     \
                   __FUNCTION__, __LINE__, (ULONGLONG)(s));            \
        cleanup_expr;                                                  \
        return __VA_ARGS__;                                            \
    }

typedef struct _LDR_DATA_TABLE_ENTRY
{
    struct _LIST_ENTRY InLoadOrderLinks;                                    //0x0
    struct _LIST_ENTRY InMemoryOrderLinks;                                  //0x10
    struct _LIST_ENTRY InInitializationOrderLinks;                          //0x20
    VOID* DllBase;                                                          //0x30
    VOID* EntryPoint;                                                       //0x38
    ULONG SizeOfImage;                                                      //0x40
    struct _UNICODE_STRING FullDllName;                                     //0x48
    struct _UNICODE_STRING BaseDllName;                                     //0x58
    union
    {
        UCHAR FlagGroup[4];                                                 //0x68
        ULONG Flags;                                                        //0x68
        struct
        {
            ULONG PackagedBinary : 1;                                         //0x68
            ULONG MarkedForRemoval : 1;                                       //0x68
            ULONG ImageDll : 1;                                               //0x68
            ULONG LoadNotificationsSent : 1;                                  //0x68
            ULONG TelemetryEntryProcessed : 1;                                //0x68
            ULONG ProcessStaticImport : 1;                                    //0x68
            ULONG InLegacyLists : 1;                                          //0x68
            ULONG InIndexes : 1;                                              //0x68
            ULONG ShimDll : 1;                                                //0x68
            ULONG InExceptionTable : 1;                                       //0x68
            ULONG ReservedFlags1 : 2;                                         //0x68
            ULONG LoadInProgress : 1;                                         //0x68
            ULONG LoadConfigProcessed : 1;                                    //0x68
            ULONG EntryProcessed : 1;                                         //0x68
            ULONG ProtectDelayLoad : 1;                                       //0x68
            ULONG ReservedFlags3 : 2;                                         //0x68
            ULONG DontCallForThreads : 1;                                     //0x68
            ULONG ProcessAttachCalled : 1;                                    //0x68
            ULONG ProcessAttachFailed : 1;                                    //0x68
            ULONG CorDeferredValidate : 1;                                    //0x68
            ULONG CorImage : 1;                                               //0x68
            ULONG DontRelocate : 1;                                           //0x68
            ULONG CorILOnly : 1;                                              //0x68
            ULONG ChpeImage : 1;                                              //0x68
            ULONG ReservedFlags5 : 2;                                         //0x68
            ULONG Redirected : 1;                                             //0x68
            ULONG ReservedFlags6 : 2;                                         //0x68
            ULONG CompatDatabaseProcessed : 1;                                //0x68
        };
    };
    USHORT ObsoleteLoadCount;                                               //0x6c
    USHORT TlsIndex;                                                        //0x6e
    struct _LIST_ENTRY HashLinks;                                           //0x70
    ULONG TimeDateStamp;                                                    //0x80
    struct _ACTIVATION_CONTEXT* EntryPointActivationContext;                //0x88
    VOID* Lock;                                                             //0x90
    struct _LDR_DDAG_NODE* DdagNode;                                        //0x98
    struct _LIST_ENTRY NodeModuleLink;                                      //0xa0
    struct _LDRP_LOAD_CONTEXT* LoadContext;                                 //0xb0
    VOID* ParentDllBase;                                                    //0xb8
    VOID* SwitchBackContext;                                                //0xc0
    struct _RTL_BALANCED_NODE BaseAddressIndexNode;                         //0xc8
    struct _RTL_BALANCED_NODE MappingInfoIndexNode;                         //0xe0
    ULONGLONG OriginalBase;                                                 //0xf8
    union _LARGE_INTEGER LoadTime;                                          //0x100
    ULONG BaseNameHashValue;                                                //0x108
    enum _LDR_DLL_LOAD_REASON LoadReason;                                   //0x10c
    ULONG ImplicitPathOptions;                                              //0x110
    ULONG ReferenceCount;                                                   //0x114
    ULONG DependentLoadFlags;                                               //0x118
    UCHAR SigningLevel;                                                     //0x11c
}LDR_DATA_TABLE_ENTRY,*PLDR_DATA_TABLE_ENTRY;

namespace HELPER {
    auto checkEspListHasKernelGuid(PFLT_FILTER pFilter, PECP_LIST pEcpList, LPCGUID pGUID) -> bool;
    auto FreeCopiedString(PUNICODE_STRING p1) -> void;
    auto CopyUnicodeStringWithAllocPagedMem(PUNICODE_STRING* p1, PUNICODE_STRING p2) -> bool;
};

DEFINE_GUID(GUID_ECP_CSV_DOWN_LEVEL_OPEN_FIX_WIN7, 0x4248be44, 0x647f, 0x488f, 0x8b, 0xe5, 0xa0, 0x8a, 0xaf, 0x70, 0xf0, 0x28);
DEFINE_GUID(GUID_ECP_PREFETCH_OPEN_FIX_VS_SHIT, 0xe1777b21, 0x847e, 0x4837, 0xaa, 0x45, 0x64, 0x16, 0x1d, 0x28, 0x06, 0x55);
