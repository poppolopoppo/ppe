#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
namespace Engine {
POOL_TAG_DECL(Engine);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEngineStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FEngineStartup()  { Start(); }
    ~FEngineStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
