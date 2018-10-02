#pragma once

#include "VFS.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/VFS/targetmodule.h can't be included first !"
#endif

using FVirtualFileSystemModule = PPE::FVirtualFileSystemModule;
PPE_STATICMODULE_STARTUP_DEF(VirtualFileSystem);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(VirtualFileSystem)
