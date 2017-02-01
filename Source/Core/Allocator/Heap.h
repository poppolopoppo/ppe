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
class FHeap {
public:
    struct current_process_t{};

    explicit FHeap(const char* nameForDebug, bool locked, size_t maximumSize = 0);
    explicit FHeap(current_process_t);
    FHeap(FHeap&& rvalue);
    ~FHeap();

    FHeap(const FHeap&) = delete;
    FHeap& operator =(const FHeap&) = delete;

    void*   Handle() const { return _handle; }
    FSizeInBytes Size() const { return TrackingData().TotalSizeInBytes(); }

    void*   Malloc(size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());
    void    Free(void *ptr, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());
    void*   Calloc(size_t nmemb, size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());
    void*   Realloc(void *ptr, size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());

    void*   AlignedMalloc(size_t size, size_t alignment, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());
    void    AlignedFree(void *ptr, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());
    void*   AlignedCalloc(size_t nmemb, size_t size, size_t alignment, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());
    void*   AlignedRealloc(void *ptr, size_t size, size_t alignment, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global());

    template <size_t _Alignment>
    void*   Malloc(size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void    Free(void *ptr, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Calloc(size_t nmemb, size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Realloc(void *ptr, size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);

    template <size_t _Alignment>
    void*   Malloc(size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void    Free(void *ptr, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Calloc(size_t nmemb, size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Realloc(void *ptr, size_t size, FMemoryTrackingData& trackingData = FMemoryTrackingData::Global(), typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);

    void    Swap(FHeap& other);

private:
    void*   _handle;

public:
#ifdef USE_MEMORY_DOMAINS
    const FMemoryTrackingData& TrackingData() const { return _trackingData; }
private:
    FMemoryTrackingData _trackingData;
#else
    const FMemoryTrackingData& TrackingData() const { return FMemoryTrackingData::Global(); }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag>
class THeapSingleton : Meta::TSingleton<FHeap, _Tag> {
    typedef Meta::TSingleton<FHeap, _Tag> parent_type;
public:
    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;
    using parent_type::Create;
};
//----------------------------------------------------------------------------
#define HEAP_SINGLETON_DEF(_NAME) namespace Heaps { \
    struct _NAME : public Core::THeapSingleton<_NAME> {}; \
    }
//----------------------------------------------------------------------------
HEAP_SINGLETON_DEF(Process);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Allocator/Heap-inl.h"
