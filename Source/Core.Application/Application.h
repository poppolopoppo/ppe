#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
namespace Application {
POOL_TAG_DECL(Application);
class ApplicationBase;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    ApplicationStartup() { Start(); }
    ~ApplicationStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationContext {
public:
    ApplicationContext();
    ~ApplicationContext();
};
//----------------------------------------------------------------------------
int LaunchApplication(const ApplicationContext& context, ApplicationBase* app);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
