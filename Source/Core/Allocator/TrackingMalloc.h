#pragma once

#include "Core/Core.h"

namespace Core {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API void* (tracking_malloc)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
CORE_API void  (tracking_free)(void *ptr);
//----------------------------------------------------------------------------
CORE_API void* (tracking_calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size);
//----------------------------------------------------------------------------
CORE_API void* (tracking_realloc)(FMemoryTracking& trackingData, void *ptr, size_t size);
//----------------------------------------------------------------------------
CORE_API void* (tracking_malloc_thread_local)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
CORE_API void  (tracking_free_thread_local)(void *ptr);
//----------------------------------------------------------------------------
CORE_API void* (tracking_calloc_thread_local)(FMemoryTracking& trackingData, size_t nmemb, size_t size);
//----------------------------------------------------------------------------
CORE_API void* (tracking_realloc_thread_local)(FMemoryTracking& trackingData, void *ptr, size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_malloc(size_t size);
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
void* tracking_calloc_thread_local(size_t nmemb, size_t size);
//----------------------------------------------------------------------------
template <typename _MemoryDomain>
void* tracking_realloc_thread_local(void *ptr, size_t size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "TrackingMalloc-inl.h"
