#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"
#include "Core/Meta/Singleton.h"

#ifdef _DEBUG
#   define USE_HEAP_VALIDATION
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Heap {
public:
    struct current_process_t{};

    explicit Heap(const char* nameForDebug, bool locked, size_t maximumSize = 0);
    explicit Heap(current_process_t);
    Heap(Heap&& rvalue);
    ~Heap();

    Heap(const Heap&) = delete;
    Heap& operator =(const Heap&) = delete;

    void*   Handle() const { return _handle; }
    SizeInBytes Size() const { return TrackingData().TotalSizeInBytes(); }

    void*   malloc(size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global());
    void    free(void *ptr, MemoryTrackingData& trackingData = MemoryTrackingData::Global());
    void*   calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global());
    void*   realloc(void *ptr, size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global());

    void*   aligned_malloc(size_t size, size_t alignment, MemoryTrackingData& trackingData = MemoryTrackingData::Global());
    void    aligned_free(void *ptr, MemoryTrackingData& trackingData = MemoryTrackingData::Global());
    void*   aligned_calloc(size_t nmemb, size_t size, size_t alignment, MemoryTrackingData& trackingData = MemoryTrackingData::Global());
    void*   aligned_realloc(void *ptr, size_t size, size_t alignment, MemoryTrackingData& trackingData = MemoryTrackingData::Global());

    template <size_t _Alignment>
    void*   malloc(size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void    free(void *ptr, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   realloc(void *ptr, size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type* = 0);

    template <size_t _Alignment>
    void*   malloc(size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void    free(void *ptr, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   realloc(void *ptr, size_t size, MemoryTrackingData& trackingData = MemoryTrackingData::Global(), typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type* = 0);

    void    Swap(Heap& other);

private:
    void*   _handle;

public:
#ifdef USE_MEMORY_DOMAINS
    const MemoryTrackingData& TrackingData() const { return _trackingData; }
private:
    MemoryTrackingData _trackingData;
#else
    const MemoryTrackingData& TrackingData() const { return MemoryTrackingData::Global(); }
#endif
};
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::malloc(size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const ptr = this->malloc(size, trackingData);
    Assert(IS_ALIGNED(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void Heap::free(void *ptr, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    this->free(ptr, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const ptr = this->calloc(nmemb, size, trackingData);
    Assert(IS_ALIGNED(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::realloc(void *ptr, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< IsNaturalyAligned<_Alignment>::value >::type*) {
    void* const ptr = this->realloc(ptr, size, trackingData);
    Assert(IS_ALIGNED(_Alignment, ptr));
    return ptr;
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::malloc(size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return aligned_malloc(size, _Alignment, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void Heap::free(void *ptr, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    aligned_free(ptr, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::calloc(size_t nmemb, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return aligned_calloc(nmemb, size, _Alignment, trackingData);
}
//----------------------------------------------------------------------------
template <size_t _Alignment>
FORCE_INLINE
void* Heap::realloc(void *ptr, size_t size, MemoryTrackingData& trackingData, typename std::enable_if< !IsNaturalyAligned<_Alignment>::value >::type*) {
    return aligned_realloc(ptr, size, trackingData);
}
//----------------------------------------------------------------------------
inline void Heap::Swap(Heap& other) {
    std::swap(other._handle, _handle);
}
//----------------------------------------------------------------------------
inline void swap(Heap& lhs, Heap& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag>
class HeapSingleton : Meta::Singleton<Heap, _Tag> {
    typedef Meta::Singleton<Heap, _Tag> parent_type;
public:
    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;
    using parent_type::Create;
};
//----------------------------------------------------------------------------
#define HEAP_SINGLETON(_NAME) namespace Heaps { \
    struct _NAME : public Core::HeapSingleton<_NAME> {}; \
    }
//----------------------------------------------------------------------------
HEAP_SINGLETON(Process);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
