#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_VFS
#   define PPE_VFS_API DLL_EXPORT
#else
#   define PPE_VFS_API DLL_IMPORT
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
