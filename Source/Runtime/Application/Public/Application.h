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
class FApplicationBase;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FApplicationModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FApplicationModule() { Start(); }
    ~FApplicationModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FApplicationContext {
public:
    FApplicationContext();
    ~FApplicationContext();
};
//----------------------------------------------------------------------------
int LaunchApplication(const FApplicationContext& context, FApplicationBase* app);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
