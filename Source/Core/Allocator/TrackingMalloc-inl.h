#pragma once

#include "Core/Allocator/TrackingMalloc.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
inline void* tracking_malloc(size_t size) {
#ifdef USE_MEMORY_DOMAINS
    return (malloc)(_MemoryDomain::TrackingData, size);
#else
    return (malloc)(size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
inline void tracking_free(void *ptr)  {
#ifdef USE_MEMORY_DOMAINS
    (free)(_MemoryDomain::TrackingData, ptr);
#else
    (free)(ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_calloc(size_t nmemb, size_t size) {
#ifdef USE_MEMORY_DOMAINS
    return (calloc)(_MemoryDomain::TrackingData, nmemb, size);
#else
    return (calloc)(nmemb, size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_realloc(void *ptr, size_t size) {
#ifdef USE_MEMORY_DOMAINS
    return (realloc)(_MemoryDomain::TrackingData, ptr, size);
#else
    return (realloc)(ptr, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
