#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Meta/Singleton.h"
#include "Core/Memory/MemoryTracking.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGlobalVideoMemory : Meta::TSingleton<FMemoryTrackingData, FGlobalVideoMemory> {
public:
    typedef Meta::TSingleton<FMemoryTrackingData, FGlobalVideoMemory> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create() {
        parent_type::Create("VideoMemory");
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
