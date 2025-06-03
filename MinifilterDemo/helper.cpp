#include "helper.h"

auto HELPER::checkEspListHasKernelGuid(PFLT_FILTER pFilter, PECP_LIST pEcpList, LPCGUID pGUID) -> bool{
    
    PVOID pEcpContext = NULL;
    if(!NT_SUCCESS(FltFindExtraCreateParameter(pFilter,pEcpList,pGUID,&pEcpContext,nullptr)) || 
        FltIsEcpFromUserMode(pFilter, pEcpContext)) {
        return false;
    }
    return true;
}

auto HELPER::FreeCopiedString(PUNICODE_STRING p1) -> void {
    if (p1) {
        if (p1->Buffer) {
            ExFreePool(p1->Buffer);
        }
        ExFreePool(p1);
    }
}

auto HELPER::CopyUnicodeStringWithAllocPagedMem(PUNICODE_STRING* p1, PUNICODE_STRING p2) -> bool{
    
    const auto target = (PUNICODE_STRING)ExAllocatePool(PagedPool, sizeof(UNICODE_STRING));
    bool isSuccess = false;

    if (!target) {
        return false;
    }

    memset(target, 0, sizeof(UNICODE_STRING));
    do{
        target->Length = p2->Length;
        target->MaximumLength = p2->MaximumLength;
        target->Buffer = (PWCH)ExAllocatePool(PagedPool, target->MaximumLength);
        if (!target->Buffer) {
            ExFreePool(target); 
            return false;
        }

        memcpy(target->Buffer, p2->Buffer, p2->MaximumLength);
        *p1 = target;
        isSuccess = true;
    } while (false);
    if (!isSuccess){
        FreeCopiedString(target);
    }
    return isSuccess;

}
