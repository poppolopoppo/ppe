#pragma once

#include "Core/Allocator/TrackingMalloc.h"
#include "Core/Memory/MemoryDomain.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
inline void* tracking_malloc(size_t size) {
#if USE_CORE_MEMORYDOMAINS
    return (tracking_malloc)(_MemoryDomain::TrackingData(), size);
#else
    return (malloc)(size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_calloc(size_t nmemb, size_t size) {
#if USE_CORE_MEMORYDOMAINS
    return (tracking_calloc)(_MemoryDomain::TrackingData(), nmemb, size);
#else
    return (calloc)(nmemb, size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_realloc(void *ptr, size_t size) {
#if USE_CORE_MEMORYDOMAINS
    return (tracking_realloc)(_MemoryDomain::TrackingData(), ptr, size);
#else
    return (realloc)(ptr, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
