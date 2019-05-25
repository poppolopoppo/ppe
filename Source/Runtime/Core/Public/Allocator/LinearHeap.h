#pragma once

#include "Core_fwd.h"

#include "Allocator/MallocBinned.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

#if USE_PPE_MEMORYDOMAINS
#   define LINEARHEAP(_DOMAIN) ::PPE::TLinearHeap<MEMORYDOMAIN_TAG(_DOMAIN)>
#   define LINEARHEAP_POOLED(_DOMAIN) ::PPE::TPooledLinearHeap<MEMORYDOMAIN_TAG(_DOMAIN)>
#else
#   define LINEARHEAP(_DOMAIN) ::PPE::FLinearHeap
#   define LINEARHEAP_POOLED(_DOMAIN) ::PPE::FPooledLinearHeap
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FLinearHeap : Meta::FNonCopyableNorMovable {
#if USE_PPE_MEMORYDOMAINS
protected:
    friend class FPooledLinearHeap;
    explicit FLinearHeap(FMemoryTracking& parent) NOEXCEPT;
#else
public:
    FLinearHeap() NOEXCEPT;
#endif
public:

    ~FLinearHeap();

    void* Allocate(size_t size);
    void* Reallocate(void* ptr, size_t newSize, size_t oldSize);
    void  Deallocate(void* ptr, size_t size);

    void* Reallocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize);
    void  Deallocate_AssumeLast(void* ptr, size_t size);

    void* Reallocate_AssumeSlow(void* ptr, size_t newSize, size_t oldSize);

    void ReleaseAll();

    bool IsLastBlock(void* ptr, size_t size) const NOEXCEPT;

    static const size_t MinBlockSize;
    static const size_t MaxBlockSize;

    static size_t SnapSize(size_t sizeInBytes) NOEXCEPT;

    static void FlushVirtualMemoryCache();

#if !USE_PPE_FINAL_RELEASE
    bool AliasesToHeap(void* ptr) const NOEXCEPT;
#endif
#if USE_PPE_MEMORYDOMAINS
    const FMemoryTracking& TrackingData() const NOEXCEPT { return _trackingData; }
#endif

private:
    size_t _offset;
    void* _blocks;

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Domain>
class TLinearHeap : public FLinearHeap {
public:
    TLinearHeap() NOEXCEPT
        : FLinearHeap(_Domain::TrackingData()) {}
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FPooledLinearHeap : Meta::FNonCopyableNorMovable {
#if USE_PPE_MEMORYDOMAINS
protected:
    explicit FPooledLinearHeap(FMemoryTracking& parent) NOEXCEPT;
#else
public:
    FPooledLinearHeap() NOEXCEPT;
#endif
public:

    void* Allocate(size_t size);
    void* Reallocate(void* ptr, size_t newSize, size_t oldSize);
    void  Deallocate(void* ptr, size_t size);

    void ReleaseAll();
    void TrimPools();

    static size_t SnapSize(size_t sizeInBytes) NOEXCEPT;

#if !USE_PPE_FINAL_RELEASE
    bool AliasesToHeap(void* ptr) const NOEXCEPT { return _heap.AliasesToHeap(ptr); }
#endif

private:
    FLinearHeap _heap;
    void* _pools[FMallocBinned::NumSizeClasses];
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Domain>
class TPooledLinearHeap : public FPooledLinearHeap {
public:
    TPooledLinearHeap() NOEXCEPT
        : FPooledLinearHeap(_Domain::TrackingData()) {}
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE


// Using FLinearHeap as an allocator
inline void* operator new(size_t sizeInBytes, PPE::FLinearHeap& heap) {
    return heap.Allocate(sizeInBytes);
}
inline void operator delete(void* ptr, PPE::FLinearHeap& heap) {
    Assert_NoAssume(heap.AliasesToHeap(ptr));
    AssertNotImplemented(); // can't delete since we don't know the allocation size
    UNUSED(ptr);
    UNUSED(heap);
}

// Using FPooledLinearHeap as an allocator
inline void* operator new(size_t sizeInBytes, PPE::FPooledLinearHeap& heap) {
    return heap.Allocate(sizeInBytes);
}
inline void operator delete(void* ptr, PPE::FPooledLinearHeap& heap) {
    Assert_NoAssume(heap.AliasesToHeap(ptr));
    AssertNotImplemented(); // can't delete since we don't know the allocation size
    UNUSED(ptr);
    UNUSED(heap);
}
