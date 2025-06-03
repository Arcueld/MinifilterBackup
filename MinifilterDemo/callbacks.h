#pragma once

#include "helper.h"
#include "callbacks.h"

namespace CALLBACKS {
	auto Uninstall() -> void;
	auto Init(PDRIVER_OBJECT pDriverObj) -> bool;
};