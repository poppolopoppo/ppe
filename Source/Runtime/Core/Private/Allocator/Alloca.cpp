// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/Alloca.h"

#include "Allocator/Allocation.h"
#include "Allocator/SlabHeap.h"
#include "Allocator/SlabAllocator.h"
#include "Allocator/Malloc.h"
#include "Allocator/StaticAllocator.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Meta/Singleton.h"
#include "Thread/ThreadSafe.h"

// skip linear heap when using memory debugging
#define USE_PPE_ALLOCA_SLABHEAP (!USE_PPE_MEMORY_DEBUGGING) // %_NOCOMMIT%

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FAllocaSlabHeapTLS_
:   public SLABHEAP(Alloca)
,   Meta::TThreadLocalSingleton<FAllocaSlabHeapTLS_> {
    typedef Meta::TThreadLocalSingleton<FAllocaSlabHeapTLS_> parent_type;
public:
    using slab_type = SLABHEAP(Alloca);

    using parent_type::Get;
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() {
        parent_type::Create();
    }

    STATIC_CONST_INTEGRAL(size_t, MaxBlockSize, 48_KiB);
};
//----------------------------------------------------------------------------
// Fall back on thread local heap when the block is too large :
using FAllocaFallback_ = TStaticAllocator<ALLOCATOR(Alloca)>;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
u32 AllocaDepth() {
#if USE_PPE_ALLOCA_SLABHEAP
    // used for detecting live alloca TLS blocks in debug
    return static_cast<u32>(FAllocaSlabHeapTLS_::Get().HasLiveBlocks_ForDebugOnly());
#else
    return 0;
#endif
}
#endif //!USE_PPE_ASSERT
//----------------------------------------------------------------------------
void* Alloca(size_t size) {
    if (0 == size)
        return nullptr;

    void* p;
#if USE_PPE_ALLOCA_SLABHEAP
    if (size <= FAllocaSlabHeapTLS_::MaxBlockSize)
        p = FAllocaSlabHeapTLS_::Get().Allocate(size);
    else
#endif
        p = FAllocaFallback_::Allocate(size).Data;

    Assert(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, p));
    return p;
}
//----------------------------------------------------------------------------
void* RelocateAlloca(void* ptr, size_t newSize, size_t oldSize, bool keepData) {
    if (nullptr == ptr) {
        Assert(0 == oldSize);
        return Alloca(newSize);
    }
    if (0 == newSize) {
        FreeAlloca(ptr, oldSize);
        return nullptr;
    }

    Assert(ptr);
    Assert(oldSize);

    void* result;
#if USE_PPE_ALLOCA_SLABHEAP
    if (oldSize <= FAllocaSlabHeapTLS_::MaxBlockSize) {
        auto& heap = FAllocaSlabHeapTLS_::Get();
        Assert_NoAssume(heap.AliasesToHeap(ptr));

        if (newSize <= FAllocaSlabHeapTLS_::MaxBlockSize) {
            result = heap.Reallocate(ptr, newSize, oldSize);
        }
        else {
            result = FAllocaFallback_::Allocate(newSize).Data;

            if (keepData)
                FPlatformMemory::Memcpy(result, ptr, Min(oldSize, newSize));

            heap.Deallocate(ptr, oldSize);
        }
    }
    else
#else
    Unused(keepData);
#endif
    {
#if USE_PPE_ALLOCA_SLABHEAP
        Assert_NoAssume(FAllocaSlabHeapTLS_::Get().AliasesToHeap(ptr) == false);
#endif

        FAllocatorBlock blk{ ptr, oldSize };
        FAllocaFallback_::Reallocate(blk, newSize);
        result = blk.Data;
    }

    Assert(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, result));
    return result;
}
//----------------------------------------------------------------------------
void FreeAlloca(void* ptr, size_t size) {
    if (nullptr == ptr) {
        Assert(0 == size);
        return;
    }

    Assert(Meta::IsAlignedPow2(ALLOCATION_BOUNDARY, ptr));
    Assert(size);

#if USE_PPE_ALLOCA_SLABHEAP
    if (size <= FAllocaSlabHeapTLS_::MaxBlockSize) {
        FAllocaSlabHeapTLS_::Get().Deallocate(ptr, size);
    }
    else
#endif
    {
#if USE_PPE_ALLOCA_SLABHEAP
        Assert_NoAssume(FAllocaSlabHeapTLS_::Get().AliasesToHeap(ptr) == false);
#endif

        FAllocaFallback_::Deallocate({ ptr, size });
    }
}
//----------------------------------------------------------------------------
size_t AllocaSnapSize(size_t size) {
#if USE_PPE_ALLOCA_SLABHEAP
    if (size <= FAllocaSlabHeapTLS_::MaxBlockSize)
        return FAllocaSlabHeapTLS_::SnapSize(size);
    else
#endif
        return FAllocaFallback_::SnapSize(size);
}
//----------------------------------------------------------------------------
FAllocaHeap& AllocaHeap() {
    return FAllocaSlabHeapTLS_::Get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAllocaStartup::Start(bool/* mainThread */) {
#if USE_PPE_ALLOCA_SLABHEAP
    FAllocaSlabHeapTLS_::Create();
#endif
}
//----------------------------------------------------------------------------
void FAllocaStartup::Shutdown() {
#if USE_PPE_ALLOCA_SLABHEAP
    FAllocaSlabHeapTLS_::Destroy();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
