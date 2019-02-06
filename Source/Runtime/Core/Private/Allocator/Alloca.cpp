#include "stdafx.h"

#include "Allocator/Alloca.h"

#include "Allocator/LinearHeap.h"
#include "Allocator/LinearHeapAllocator.h"
#include "Allocator/Malloc.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Meta/Singleton.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FAllocaLinearHeapTLS_
    : public LINEARHEAP(Alloca)
    , Meta::TThreadLocalSingleton<FAllocaLinearHeapTLS_> {
    typedef Meta::TThreadLocalSingleton<FAllocaLinearHeapTLS_> parent_type;
public:
    using parent_type::Get;
#ifdef WITH_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }

    STATIC_CONST_INTEGRAL(size_t, MaxBlockSize, 32 << 10); // 32 kb

#ifndef WITH_PPE_ASSERT
    static void* Malloc(size_t sz) {
        return Get().Allocate(sz);
    }

    void Free(void* ptr, size_t sz) {
        return Release_AssumeLast(ptr, sz);
    }

#else
    u32 Depth{ 0 };

    static void* Malloc(size_t sz) {
        auto& heap = Get();
        heap.Depth++;
        return heap.Allocate(sz);
    }

    void Free(void* ptr, size_t sz) {
        auto& heap = Get();
        Assert(heap.Depth);
        heap.Depth--;
        heap.Release_AssumeLast(ptr, sz);
    }

#endif //!WITH_PPE_ASSERT
};
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
#ifdef WITH_PPE_ASSERT
u32 AllocaDepth() {
    // used for detecting live alloca TLS blocks in debug
    return FAllocaLinearHeapTLS_::Get().Depth;
}
#endif //!WITH_PPE_ASSERT
//----------------------------------------------------------------------------
void* Alloca(size_t size) {
    if (0 == size)
        return nullptr;

    void* const p = ((size <= FAllocaLinearHeapTLS_::MaxBlockSize)
        ? FAllocaLinearHeapTLS_::Malloc(size)
        : FAllocaFallback_::Malloc(size) );

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
    auto& heap = FAllocaLinearHeapTLS_::Get();
    if (heap.AliasesToHeap(ptr)) {
        if (newSize <= FAllocaLinearHeapTLS_::MaxBlockSize) {
            result = heap.Relocate_AssumeLast(ptr, newSize, oldSize);
        }
        else if (keepData) {
            void* const dst = FAllocaFallback_::Malloc(newSize);
            FPlatformMemory::Memcpy(dst, ptr, Min(oldSize, newSize));
            result = dst;
        }
        else {
            heap.Free(ptr, oldSize);
            result = FAllocaFallback_::Malloc(newSize);
        }
    }
    else {
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

    auto& heap = FAllocaLinearHeapTLS_::Get();
    if (heap.AliasesToHeap(ptr))
        heap.Free(ptr, size);
    else
        FAllocaFallback_::Free(ptr);
}
//----------------------------------------------------------------------------
size_t AllocaSnapSize(size_t size) {
    return ((size <= FAllocaLinearHeapTLS_::MaxBlockSize)
        ? ROUND_TO_NEXT_16(size)
        : FAllocaFallback_::SnapSize(size) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAllocaStartup::Start(bool/* mainThread */) {
    FAllocaLinearHeapTLS_::Create();
}
//----------------------------------------------------------------------------
void FAllocaStartup::Shutdown() {
    FAllocaLinearHeapTLS_::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
