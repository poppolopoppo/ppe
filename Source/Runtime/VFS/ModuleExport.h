#pragma once

#include "VirtualFileSystem_fwd.h"

#include "Module.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "Runtime/VFS/ModuleExport.h can't be included first !"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_VFS_API FVirtualFileSystemModule : public FModule {
public:
    FVirtualFileSystemModule();
    virtual ~FVirtualFileSystemModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

using FVirtualFileSystemModule = PPE::FVirtualFileSystemModule;
PPE_STATICMODULE_STARTUP_DEF(VirtualFileSystem);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(VirtualFileSystem)
