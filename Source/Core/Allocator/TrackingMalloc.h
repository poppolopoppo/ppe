#pragma once

#include "Core/Core.h"

namespace Core {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API void* (malloc)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
CORE_API void  (free)(FMemoryTracking& trackingData, void *ptr);
//----------------------------------------------------------------------------
CORE_API void* (calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size);
//----------------------------------------------------------------------------
CORE_API void* (realloc)(FMemoryTracking& trackingData, void *ptr, size_t size);
//----------------------------------------------------------------------------
CORE_API void* (malloc_thread_local)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
CORE_API void  (free_thread_local)(FMemoryTracking& trackingData, void *ptr);
//----------------------------------------------------------------------------
CORE_API void* (calloc_thread_local)(FMemoryTracking& trackingData, size_t nmemb, size_t size);
//----------------------------------------------------------------------------
CORE_API void* (realloc_thread_local)(FMemoryTracking& trackingData, void *ptr, size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_malloc(size_t size);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void  tracking_free(void *ptr);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_calloc(size_t nmemb, size_t size);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_realloc(void *ptr, size_t size);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_malloc_thread_local(size_t size);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void  tracking_free_thread_local(void *ptr);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_calloc_thread_local(size_t nmemb, size_t size);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_realloc_thread_local(void *ptr, size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
#define CLASS_MEMORY_TRACKING_DEF(T, _Domain) \
    static Core::FMemoryTracking& Class_TrackingData() { \
        return MEMORY_DOMAIN_TRACKING_DATA(_Domain); \
    } \
    \
    void* operator new(size_t size) { \
        Assert(sizeof(T) == size); \
        Class_TrackingData().Allocate(1, sizeof(T)); \
        return Core::malloc(size); \
    } \
    void operator delete(void* ptr) { \
        Class_TrackingData().Deallocate(1, sizeof(T)); \
        Core::free(ptr); \
    } \
    \
    void* operator new(size_t, void* ptr) { \
        Assert(ptr); \
        Likely(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define CLASS_MEMORY_TRACKING_DEF(T, _Domain) \
    static FMemoryTracking& Class_TrackingData() { \
        return MEMORY_DOMAIN_TRACKING_DATA(_Domain); \
    }
//----------------------------------------------------------------------------
#endif //!USE_MEMORY_DOMAINS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "TrackingMalloc-inl.h"
