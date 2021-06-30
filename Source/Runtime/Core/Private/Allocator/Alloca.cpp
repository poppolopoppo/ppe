#include "stdafx.h"

#include "Allocator/Alloca.h"

#include "Allocator/SlabHeap.h"
#include "Allocator/SlabAllocator.h"
#include "Allocator/Malloc.h"

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

    STATIC_CONST_INTEGRAL(size_t, MaxBlockSize, DefaultSlabSize / 2);
};
//----------------------------------------------------------------------------
// Fall back on thread local heap when the block is too large :
struct FAllocaFallback_ {
    static void* Malloc(size_t size) {
        return TRACKING_MALLOC(Alloca, size);
    }
    static void* Realloc(void* ptr, size_t size) {
        return TRACKING_REALLOC(Alloca, ptr, size);
    }
    static void Free(void* ptr) {
        TRACKING_FREE(Alloca, ptr);
    }
    static size_t SnapSize(size_t size) {
        return PPE::malloc_snap_size(size);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
u32 AllocaDepth() {
#if USE_PPE_ALLOCA_SLABHEAP
    // used for detecting live alloca TLS blocks in debug
    return (FAllocaSlabHeapTLS_::Get().Tell().Origin > 0);
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
        p = FAllocaFallback_::Malloc(size);

    Assert(Meta::IsAligned(16, p));
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
            result = heap.Reallocate_AssumeLast(ptr, newSize, oldSize);
        }
        else {
            result = FAllocaFallback_::Malloc(newSize);

            if (keepData)
                FPlatformMemory::Memcpy(result, ptr, Min(oldSize, newSize));

            heap.Deallocate_AssumeLast(ptr, oldSize);
        }
    }
    else
#else
    UNUSED(keepData);
#endif
    {
#if USE_PPE_ALLOCA_SLABHEAP
        Assert_NoAssume(FAllocaSlabHeapTLS_::Get().AliasesToHeap(ptr) == false);
#endif

        result = FAllocaFallback_::Realloc(ptr, newSize);
    }

    Assert(Meta::IsAligned(16, result));
    return result;
}
//----------------------------------------------------------------------------
void FreeAlloca(void* ptr, size_t size) {
    if (nullptr == ptr) {
        Assert(0 == size);
        return;
    }

    Assert(Meta::IsAligned(16, ptr));
    Assert(size);

#if USE_PPE_ALLOCA_SLABHEAP
    if (size <= FAllocaSlabHeapTLS_::MaxBlockSize)
        FAllocaSlabHeapTLS_::Get().Deallocate_AssumeLast(ptr, size);
    else
#endif
    {
#if USE_PPE_ALLOCA_SLABHEAP
        Assert_NoAssume(FAllocaSlabHeapTLS_::Get().AliasesToHeap(ptr) == false);
#endif

        FAllocaFallback_::Free(ptr);
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
TThreadSafe<TPtrRef<FSlabHeap>, EThreadBarrier::ThreadLocal> AllocaHeap() {
    return MakeThreadSafe<EThreadBarrier::ThreadLocal>(
        MakePtrRef(static_cast<FSlabHeap&>(FAllocaSlabHeapTLS_::Get())) );
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
