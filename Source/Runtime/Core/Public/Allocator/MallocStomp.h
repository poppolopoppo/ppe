#pragma once

#include "Core.h"

#if USE_PPE_MEMORY_DEBUGGING
#   define WITH_PPE_MALLOCSTOMP
#endif

#ifdef WITH_PPE_MALLOCSTOMP

namespace PPE {
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
} //!namespace PPE

#endif //!WITH_PPE_MALLOCSTOMP