#include "callbacks.h"
#include "minifilter.h"

namespace CALLBACKS {
	bool MinifilterIsInstalled = FALSE;

	auto Uninstall() -> void {
		if (MinifilterIsInstalled) {
			MINIFILTER::UnloadMinifilter();
		}
	}

	auto Init(PDRIVER_OBJECT pDriverObj) -> bool {
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		do {
			status = MINIFILTER::Init(pDriverObj);
			CHECK_STATUS_AND_BREAK(status);
			MinifilterIsInstalled = true;


		} while (false);

		return NT_SUCCESS(status);
	}


};