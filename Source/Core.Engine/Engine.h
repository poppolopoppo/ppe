#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
namespace Engine {
POOL_TAG_DECL(Engine);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EngineStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    EngineStartup()  { Start(); }
    ~EngineStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
