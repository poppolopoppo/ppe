#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_SERIALIZE
#   define PPE_SERIALIZE_API DLL_EXPORT
#else
#   define PPE_SERIALIZE_API DLL_IMPORT
#endif

#include "Allocator/PoolAllocatorTag.h"

namespace PPE {
namespace Serialize {
POOL_TAG_DECL(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FSerializeModule is the entry and exit point encapsulating every call to Core::Serialize::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FSerializeModule {
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
} //!namespace PPE
