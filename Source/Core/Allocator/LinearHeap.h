#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryTracking.h"

#include <atomic>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FLinearHeap {
public:
    static FLinearHeap& GGlobalHeap;

    explicit FLinearHeap(const char* name);
    ~FLinearHeap();

    FLinearHeap(const FLinearHeap&) = delete;
    FLinearHeap& operator =(const FLinearHeap&) = delete;

    FLinearHeap(FLinearHeap&&) = delete;
    FLinearHeap& operator =(FLinearHeap&&) = delete;

#ifdef USE_MEMORY_DOMAINS
    const FMemoryTracking& TrackingData() const { return _trackingData; }
#endif

    void* Allocate(size_t sizeInBytes, size_t alignment = ALLOCATION_BOUNDARY);
    void* Relocate(void* ptr, size_t oldSize, size_t newSize, size_t alignment = ALLOCATION_BOUNDARY);
    void  Deallocate(void* ptr);

    void Freeze();
    void Unfreeze();
    void ReleaseAll();

    static size_t SnapSize(size_t size);

private:
    void* _smallBlocksTLS[CORE_MAX_CORES];

    void* _largeBlocks;
    std::atomic<size_t> _largeBlocksCount;

#ifdef WITH_CORE_ASSERT
    std::atomic_bool _frozen;
#endif
#ifdef USE_MEMORY_DOMAINS
    FMemoryTracking _trackingData;
#endif

    void* SmallAllocate_(size_t sizeInBytes, size_t alignment);
    void* LargeAllocate_(size_t sizeInBytes, size_t alignment);
    void  LargeDeallocate_(void* ptr);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
