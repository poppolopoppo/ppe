#pragma once

#include "Network.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/Network/targetmodule.h can't be included first !"
#endif

using FNetworkModule = PPE::Network::FNetworkModule;
PPE_STATICMODULE_STARTUP_DEF(Network);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(Network)
