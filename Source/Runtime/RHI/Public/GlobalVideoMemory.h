#pragma once

#include "Graphics.h"

#include "Meta/Singleton.h"
#include "Memory/MemoryTracking.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGlobalVideoMemory : Meta::TSingleton<FMemoryTracking, FGlobalVideoMemory> {
public:
    typedef Meta::TSingleton<FMemoryTracking, FGlobalVideoMemory> parent_type;

    using parent_type::Get;
#ifdef WITH_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() {
        parent_type::Create("VideoMemory");
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
