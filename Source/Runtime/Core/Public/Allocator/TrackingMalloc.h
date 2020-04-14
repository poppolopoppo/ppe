#pragma once

#include "Core_fwd.h"

#include "Allocator/Malloc.h"
#include "Memory/MemoryDomain.h"

#define TRACKING_MALLOC(_DOMAIN, _SIZE) ::PPE::tracking_malloc<MEMORYDOMAIN_TAG(_DOMAIN)>(_SIZE)
#define TRACKING_CALLOC(_DOMAIN, _NMEMB, _SIZE) ::PPE::tracking_calloc<MEMORYDOMAIN_TAG(_DOMAIN)>(_NMEMB, _SIZE)
#define TRACKING_REALLOC(_DOMAIN, _PTR, _SIZE) ::PPE::tracking_realloc<MEMORYDOMAIN_TAG(_DOMAIN)>(_PTR, _SIZE)
#define TRACKING_FREE(_DOMAIN, _PTR) ::PPE::tracking_free(_PTR)

#define TRACKING_NEW(_DOMAIN, ...) INPLACE_NEW(TRACKING_MALLOC(_DOMAIN, sizeof(__VA_ARGS__)), __VA_ARGS__)
#define TRACKING_DELETE(_DOMAIN, _PTR) ::PPE::tracking_delete(_PTR)

namespace PPE {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API void* (tracking_malloc)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API void  (tracking_free)(void *ptr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void* (tracking_calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size);
//----------------------------------------------------------------------------
PPE_CORE_API void* (tracking_realloc)(FMemoryTracking& trackingData, void *ptr, size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_malloc(size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_malloc)(_MemoryDomain::TrackingData(), size);
#else
    return (PPE::malloc)(size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_calloc(size_t nmemb, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_calloc)(_MemoryDomain::TrackingData(), nmemb, size);
#else
    return (PPE::calloc)(nmemb, size);
#endif
}
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_realloc(void *ptr, size_t size) {
#if USE_PPE_MEMORYDOMAINS
    return (tracking_realloc)(_MemoryDomain::TrackingData(), ptr, size);
#else
    return (PPE::realloc)(ptr, size);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain, typename T, typename... _Args>
T* tracking_new(_Args&&... args) {
    return INPLACE_NEW(tracking_malloc<_MemoryDomain>(sizeof(T)), T) {
        std::forward<_Args>(args)...
    };
}
//----------------------------------------------------------------------------
template <typename T>
void tracking_delete(T* ptr) NOEXCEPT {
    Assert(ptr);
    Meta::Destroy(ptr);
    tracking_free(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
