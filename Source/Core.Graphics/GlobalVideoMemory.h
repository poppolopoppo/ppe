#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Meta/Singleton.h"
#include "Core/Memory/MemoryTracking.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGlobalVideoMemory : Meta::TSingleton<FMemoryTracking, FGlobalVideoMemory> {
public:
    typedef Meta::TSingleton<FMemoryTracking, FGlobalVideoMemory> parent_type;

    using parent_type::Get;
#ifdef WITH_CORE_ASSERT
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
} //!namespace Core
