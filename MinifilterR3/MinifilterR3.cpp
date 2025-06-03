#include "MinifilterR3.h"

typedef enum _COOM_STR_TYPE {
    AR_TargetFileType,
    AR_BackUpFileType,
}COOM_STR_TYPE;

typedef struct _COOM_UNICODE_STRING_CONTEXT {
    COOM_STR_TYPE type;
    USHORT Length;       
    WCHAR Buffer[260];   
} COOM_UNICODE_STRING_CONTEXT, * PCOOM_UNICODE_STRING_CONTEXT;

auto connectDriverBackUp(std::wstring inputStr, COOM_STR_TYPE type) -> bool{
    bool isConnect = false;
    HRESULT apiResult = S_OK;
    HANDLE minifilterHandle = INVALID_HANDLE_VALUE;

    do{

        
        PCOOM_UNICODE_STRING_CONTEXT context = (PCOOM_UNICODE_STRING_CONTEXT)malloc(sizeof(COOM_UNICODE_STRING_CONTEXT));

        size_t lenInBytes = inputStr.length() * sizeof(wchar_t);

        context->type = type;
        context->Length = lenInBytes;
        memset(context->Buffer, 0, sizeof(context->Buffer));
        memcpy(context->Buffer, inputStr.c_str(), lenInBytes);

        apiResult = FilterConnectCommunicationPort(R3portName, 0, context, sizeof(COOM_UNICODE_STRING_CONTEXT), NULL, &minifilterHandle);
        
        if (IS_ERROR(apiResult)) break;
        CloseHandle(minifilterHandle);
        isConnect = true;
    } while (false);
    if (!isConnect) {
        printf("[R3]:connect failed\n");
    }


    return isConnect;
}

auto BackUp(std::wstring backUpPath, std::wstring targetPath) -> bool {
    bool bRet = FALSE;
    if (targetPath.empty() || backUpPath.empty()) {
        std::wcerr << L"[R3] Path is empty.\n";
        return false;
    }
    bRet |= connectDriverBackUp(targetPath, AR_TargetFileType);
    bRet |= connectDriverBackUp(backUpPath, AR_BackUpFileType);

    return bRet;

}

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 3) {
        std::wcerr << L"Usage: x.exe <TargetPath> <BackupPath>\n";
        return 1;
    }

    std::wstring targetPath = argv[1];
    std::wstring backUpPath = argv[2];

    if (BackUp(backUpPath, targetPath)) {
        std::wcout << L"[R3] Backup path sent to driver successfully.\n";
    }
    else {
        std::wcout << L"[R3] Failed to send backup info to driver.\n";
    }
    return 0;
}

