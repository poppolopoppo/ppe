#pragma once

#include "Core/Core.h"

#ifdef EXPORT_CORE_APPLICATION
#   define CORE_APPLICATION_API DLL_EXPORT
#else
#   define CORE_APPLICATION_API DLL_IMPORT
#endif

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
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
} //!namespace Core
