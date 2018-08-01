#pragma once

#include "Core.h"

#define TRACKING_MALLOC(_DOMAIN, _SIZE) ::Core::tracking_malloc<MEMORYDOMAIN_TAG(_DOMAIN)>(_SIZE)

namespace PPE {
class FMemoryTracking;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API void* (tracking_malloc)(FMemoryTracking& trackingData, size_t size);
//----------------------------------------------------------------------------
PPE_API void  (tracking_free)(void *ptr);
//----------------------------------------------------------------------------
PPE_API void* (tracking_calloc)(FMemoryTracking& trackingData, size_t nmemb, size_t size);
//----------------------------------------------------------------------------
PPE_API void* (tracking_realloc)(FMemoryTracking& trackingData, void *ptr, size_t size);
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "TrackingMalloc-inl.h"
