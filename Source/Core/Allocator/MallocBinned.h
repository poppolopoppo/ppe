#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMallocBinned {
public:
    static CORE_API void* Malloc(size_t size);
    static CORE_API void  Free(void* ptr);
    static CORE_API void* Realloc(void* ptr, size_t size);

    static CORE_API void* AlignedMalloc(size_t size, size_t alignment);
    static CORE_API void  AlignedFree(void* ptr);
    static CORE_API void* AlignedRealloc(void* ptr, size_t size, size_t alignment);

    static CORE_API void  ReleasePendingBlocks();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
