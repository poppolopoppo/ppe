#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
POOLTAG_DEF(Engine);

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EngineStartup {
public:
    static void Start();
    static void Shutdown();

    EngineStartup()  { Start(); }
    ~EngineStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
