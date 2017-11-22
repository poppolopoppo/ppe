#pragma once

#include "Core/Core.h"

#if USE_CORE_MEMORY_DEBUGGING
#   define WITH_CORE_MALLOCSTOMP
#endif

#ifdef WITH_CORE_MALLOCSTOMP

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMallocStomp {
public:
    static void*    Malloc(size_t size);
    static void     Free(void* ptr);
    static void*    Realloc(void* ptr, size_t size);

    static void*    AlignedMalloc(size_t size, size_t alignment);
    static void     AlignedFree(void* ptr);
    static void*    AlignedRealloc(void* ptr, size_t size, size_t alignment);

    static size_t   RegionSize(void* ptr);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!WITH_CORE_MALLOCSTOMP
