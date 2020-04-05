#include "stdafx.h"

#include "Allocator/Alloca.h"

#include "Allocator/LinearHeap.h"
#include "Allocator/LinearAllocator.h"
#include "Allocator/Malloc.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Meta/Singleton.h"

// skip linear heap when using memory debugging
#define USE_PPE_ALLOCA_LINEARHEAP (!USE_PPE_MEMORY_DEBUGGING) // %_NOCOMMIT%

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_ALLOCA_LINEARHEAP
class FAllocaLinearHeapTLS_
    : LINEARHEAP(Alloca)
    , Meta::TThreadLocalSingleton<FAllocaLinearHeapTLS_> {
    typedef Meta::TThreadLocalSingleton<FAllocaLinearHeapTLS_> parent_type;
public:
    using parent_type::Get;
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }

    STATIC_CONST_INTEGRAL(size_t, MaxBlockSize, 32 << 10); // 32 kb

    using LINEARHEAP(Alloca)::Reallocate_AssumeLast;
    using LINEARHEAP(Alloca)::SnapSize;

#if !USE_PPE_FINAL_RELEASE
    using LINEARHEAP(Alloca)::AliasesToHeap;
#endif


#if !USE_PPE_ASSERT
    static void* Malloc(size_t sz) {
        return Get().Allocate(sz);
    }

    void Free(void* ptr, size_t sz) {
        Deallocate_AssumeLast(ptr, sz);
    }

#else
    u32 Depth{ 0 };

    static void* Malloc(size_t sz) {
        auto& heap = Get();
        heap.Depth++;
        return heap.Allocate(sz);
    }

    void Free(void* ptr, size_t sz) {
        Assert(Depth);
        Depth--;
        Deallocate_AssumeLast(ptr, sz);
    }

#endif //!USE_PPE_ASSERT
};
#endif //!USE_PPE_ALLOCA_LINEARHEAP
//----------------------------------------------------------------------------
// Fall back on thread local heap when the block is too large :
struct FAllocaFallback_ {
    static void* Malloc(size_t size) {
        return PPE::malloc(size);
    }
    static void* Realloc(void* ptr, size_t size) {
        return PPE::realloc(ptr, size);
    }
    static void Free(void* ptr) {
        PPE::free(ptr);
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
#if USE_PPE_ALLOCA_LINEARHEAP
    // used for detecting live alloca TLS blocks in debug
    return FAllocaLinearHeapTLS_::Get().Depth;
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
#if USE_PPE_ALLOCA_LINEARHEAP
    if (size <= FAllocaLinearHeapTLS_::MaxBlockSize)
        p = FAllocaLinearHeapTLS_::Malloc(size);
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
    else if (0 == newSize) {
        FreeAlloca(ptr, oldSize);
        return nullptr;
    }

    Assert(ptr);
    Assert(oldSize);

    void* result;
#if USE_PPE_ALLOCA_LINEARHEAP
    if (oldSize <= FAllocaLinearHeapTLS_::MaxBlockSize) {
        auto& heap = FAllocaLinearHeapTLS_::Get();
        Assert_NoAssume(heap.AliasesToHeap(ptr));

        if (newSize <= FAllocaLinearHeapTLS_::MaxBlockSize) {
            result = heap.Reallocate_AssumeLast(ptr, newSize, oldSize);
        }
        else {
            result = FAllocaFallback_::Malloc(newSize);

            if (keepData)
                FPlatformMemory::Memcpy(result, ptr, Min(oldSize, newSize));

            heap.Free(ptr, oldSize);
        }
    }
    else
#else
    UNUSED(keepData);
#endif
    {
#if USE_PPE_ALLOCA_LINEARHEAP
        Assert_NoAssume(FAllocaLinearHeapTLS_::Get().AliasesToHeap(ptr) == false);
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

#if USE_PPE_ALLOCA_LINEARHEAP
    if (size <= FAllocaLinearHeapTLS_::MaxBlockSize)
        FAllocaLinearHeapTLS_::Get().Free(ptr, size);
    else
#endif
    {
#if USE_PPE_ALLOCA_LINEARHEAP
        Assert_NoAssume(FAllocaLinearHeapTLS_::Get().AliasesToHeap(ptr) == false);
#endif

        FAllocaFallback_::Free(ptr);
    }
}
//----------------------------------------------------------------------------
size_t AllocaSnapSize(size_t size) {
#if USE_PPE_ALLOCA_LINEARHEAP
    if (size <= FAllocaLinearHeapTLS_::MaxBlockSize)
        return FAllocaLinearHeapTLS_::SnapSize(size);
    else
#endif
        return FAllocaFallback_::SnapSize(size);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAllocaStartup::Start(bool/* mainThread */) {
#if USE_PPE_ALLOCA_LINEARHEAP
    FAllocaLinearHeapTLS_::Create();
#endif
}
//----------------------------------------------------------------------------
void FAllocaStartup::Shutdown() {
#if USE_PPE_ALLOCA_LINEARHEAP
    FAllocaLinearHeapTLS_::Destroy();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
