#pragma once

#include "Core/Core.h"

#ifdef EXPORT_CORE_SERIALIZE
#   define CORE_SERIALIZE_API DLL_EXPORT
#else
#   define CORE_SERIALIZE_API DLL_IMPORT
#endif

#include "Core/Allocator/PoolAllocatorTag.h"

namespace Core {
namespace Serialize {
POOL_TAG_DECL(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FSerializeModule is the entry and exit point encapsulating every call to Core::Serialize::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class CORE_SERIALIZE_API FSerializeModule {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FSerializeModule()  { Start(); }
    ~FSerializeModule() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
