#pragma once

#include "Application.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/Application/targetmodule.h can't be included first !"
#endif

using FApplicationModule = PPE::Application::FApplicationModule;
PPE_STATICMODULE_STARTUP_DEF(Application);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(Application)
