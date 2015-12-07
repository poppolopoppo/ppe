#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
namespace Serialize {
POOLTAG_DECL(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// SerializeStartup is the entry and exit point encapsulating every call to Core::Serialize::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class SerializeStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    SerializeStartup()  { Start(); }
    ~SerializeStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
