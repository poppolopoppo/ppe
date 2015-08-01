#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Meta/Singleton.h"
#include "Core/Memory/MemoryTracking.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GlobalVideoMemory : Meta::Singleton<MemoryTrackingData, GlobalVideoMemory> {
public:
    typedef Meta::Singleton<MemoryTrackingData, GlobalVideoMemory> parent_type;

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
