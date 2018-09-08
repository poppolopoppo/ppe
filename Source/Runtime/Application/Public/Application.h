#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_APPLICATION
#   define PPE_APPLICATION_API DLL_EXPORT
#else
#   define PPE_APPLICATION_API DLL_IMPORT
#endif

#include "Allocator/PoolAllocatorTag.h"

namespace PPE {
namespace Application {
POOL_TAG_DECL(Application);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationModule : public FModule {
public:
    FApplicationModule();
    virtual ~FApplicationModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATION_API FApplicationContext {
public:
    FApplicationContext();
    ~FApplicationContext();
};
//----------------------------------------------------------------------------
PPE_APPLICATION_API int LaunchApplication(const FApplicationContext& context, class FApplicationBase* app);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
