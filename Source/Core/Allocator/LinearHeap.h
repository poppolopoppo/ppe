#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"

#if USE_CORE_MEMORYDOMAINS
#   define LINEARHEAP(_DOMAIN) ::Core::TLinearHeap<MEMORYDOMAIN_TAG(_DOMAIN)>
#else
#   define LINEARHEAP(_DOMAIN) ::Core::FLinearHeap
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FLinearHeap {
#if USE_CORE_MEMORYDOMAINS
protected:
    explicit FLinearHeap(FMemoryTracking& parent);
#else
public:
    FLinearHeap();
#endif
public:

    ~FLinearHeap();

    FLinearHeap(const FLinearHeap&) = delete;
    FLinearHeap& operator =(const FLinearHeap&) = delete;

    FLinearHeap(FLinearHeap&&) = delete;
    FLinearHeap& operator =(FLinearHeap&&) = delete;

    u32 NumAllocs() const { return _numAllocs; }

#if USE_CORE_MEMORYDOMAINS
    const FMemoryTracking& TrackingData() const { return _trackingData; }
#endif

    void* Allocate(size_t size);
    void* Relocate(void* ptr, size_t newSize, size_t oldSize);
    void  Release(void* ptr, size_t size);

    void* Relocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize);
    void  Release_AssumeLast(void* ptr, size_t size);

    bool AliasesToHeap(void* ptr) const;

    void ReleaseAll();

#if !USE_CORE_FINAL_RELEASE
    void DumpMemoryStats(const wchar_t* title);
#endif

    static const size_t MinBlockSize;
    static const size_t MinBlockSizeForRecycling;
    static const size_t MaxBlockSize;

    static CONSTEXPR size_t SnapSize(size_t sizeInBytes) { return ROUND_TO_NEXT_16(sizeInBytes); }
    static CONSTEXPR size_t SnapSizeForRecycling(size_t sizeInBytes) { return Max(MinBlockSizeForRecycling, SnapSize(sizeInBytes)); }

    static void FlushVirtualMemoryCache();

private:
    u32 _numAllocs;
    void* _blocks;
    void* _deleteds;

    void FlushDeleteds_AssumeEmpty();

#if USE_CORE_MEMORYDOMAINS
    FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
#if USE_CORE_MEMORYDOMAINS
template <typename _Domain>
class TLinearHeap : public FLinearHeap {
public:
    TLinearHeap() : FLinearHeap(_Domain::TrackingData()) {}
};
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

//----------------------------------------------------------------------------
// Use FLinearHeap as an inplace allocator :
template <typename T>
void* operator new(size_t sizeInBytes, Core::FLinearHeap& heap) {
    Assert(sizeInBytes == sizeof(T));
    return heap.Allocate(sizeInBytes, std::alignment_of_v<T>);
}
