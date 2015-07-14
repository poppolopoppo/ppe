#pragma once

#include "Core/Memory/SegregatedMemoryPool.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
template <typename _Tag, typename T, bool _ThreadLocal >
MemoryTrackingData *TypedSegregatedMemoryPool<_Tag, T, _ThreadLocal>::TrackingData = nullptr;
#endif
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
void *TypedSegregatedMemoryPool<_Tag, T, _ThreadLocal>::Allocate() {
#ifdef USE_MEMORY_DOMAINS
    ONE_TIME_INITIALIZE_TPL(PoolTracking<T COMMA _ThreadLocal>, sTracking, typeid(T).name(), segregatedpool_type::Instance().TrackingData(), &TrackingData);
    return segregatedpool_type::Instance().Allocate(&sTracking.TrackingData);
#else
    return segregatedpool_type::Instance().Allocate();
#endif
}
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
void TypedSegregatedMemoryPool<_Tag, T, _ThreadLocal>::Deallocate(void *ptr) {
#ifdef USE_MEMORY_DOMAINS
    segregatedpool_type::Instance().Deallocate(ptr, TrackingData);
#else
    segregatedpool_type::Instance().Deallocate(ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename _Tag, typename T, bool _ThreadLocal >
void TypedSegregatedMemoryPool<_Tag, T, _ThreadLocal>::Clear_UnusedMemory() {
    segregatedpool_type::Instance().Clear_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
