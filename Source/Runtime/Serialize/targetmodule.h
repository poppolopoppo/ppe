#pragma once

#include "Serialize.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/Serialize/targetmodule.h can't be included first !"
#endif

using FSerializeModule = PPE::Serialize::FSerializeModule;
PPE_STATICMODULE_STARTUP_DEF(Serialize);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(Serialize)
