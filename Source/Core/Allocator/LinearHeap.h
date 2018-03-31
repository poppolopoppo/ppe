#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"
#include "Core/Memory/MemoryTracking.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FLinearHeap {
public:
#ifdef USE_MEMORY_DOMAINS
    explicit FLinearHeap(FMemoryTracking* parent);
#else
    FLinearHeap();
#endif

    ~FLinearHeap();

    FLinearHeap(const FLinearHeap&) = delete;
    FLinearHeap& operator =(const FLinearHeap&) = delete;

    FLinearHeap(FLinearHeap&&) = delete;
    FLinearHeap& operator =(FLinearHeap&&) = delete;

#ifdef USE_MEMORY_DOMAINS
    const FMemoryTracking& TrackingData() const { return _trackingData; }
#endif

    void* Allocate(size_t size);
    void* Relocate(void* ptr, size_t newSize, size_t oldSize);
    void  Release(void* ptr, size_t size);

    void* Relocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize);
    void  Release_AssumeLast(void* ptr, size_t size);

    bool AliasesToHeap(void* ptr) const;

    void ReleaseAll();

    static const size_t MaxBlockSize;
    static void FlushVirtualMemoryCache();
    static constexpr size_t SnapSize(size_t sizeInBytes) { return ROUND_TO_NEXT_16(sizeInBytes); }

private:
    void* _blocks;
    void* _deleteds;

#ifdef USE_MEMORY_DOMAINS
    FMemoryTracking _trackingData;
#endif
};
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
