#pragma once

#include "RTTI.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/RTTI/targetmodule.h can't be included first !"
#endif

using FRTTIModule = PPE::RTTI::FRTTIModule;
PPE_STATICMODULE_STARTUP_DEF(RTTI);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(RTTI)
