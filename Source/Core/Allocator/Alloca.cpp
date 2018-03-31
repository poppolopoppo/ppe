#include "stdafx.h"

#include "Alloca.h"

#include "Allocator/LinearHeap.h"
#include "Allocator/Malloc.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Meta/Singleton.h"

#include "ThreadLocalHeap.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FAllocaLinearHeapTLS_ : Meta::TThreadLocalSingleton<FLinearHeap, FAllocaLinearHeapTLS_> {
    typedef Meta::TThreadLocalSingleton<FLinearHeap, FAllocaLinearHeapTLS_> parent_type;
public:
    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

#ifdef USE_MEMORY_DOMAINS
    static void Create() { parent_type::Create(&MEMORY_DOMAIN_TRACKING_DATA(Alloca)); }
#else
    static void Create() { parent_type::Create(); }
#endif

#if defined(ARCH_X64)
    STATIC_CONST_INTEGRAL(size_t, GMaxBlockSize, 32 << 10); // 32 kb
#else
    STATIC_CONST_INTEGRAL(size_t, GMaxBlockSize, 16 << 10); // 16 kb
#endif
};
//----------------------------------------------------------------------------
// Fallback on thread local heap when the block is too large :
struct FAllocaFallback_ {
    static void* Malloc(size_t size) {
        return GetThreadLocalHeap().Malloc(size);
    }
    static void* Realloc(void* ptr, size_t size) {
        return GetThreadLocalHeap().Realloc(ptr, size);
    }
    static void Free(void* ptr) {
        GetThreadLocalHeap().Free(ptr);
    }
    static size_t SnapSize(size_t size) {
        return GetThreadLocalHeap().SnapSize(size);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* Alloca(size_t size) {
    if (0 == size)
        return nullptr;

    void* const p = ((size <= FAllocaLinearHeapTLS_::GMaxBlockSize)
        ? FAllocaLinearHeapTLS_::Instance().Allocate(size)
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

    void* result;
    auto& heap = FAllocaLinearHeapTLS_::Instance();
    if (heap.AliasesToHeap(ptr)) {
        if (newSize <= FAllocaLinearHeapTLS_::GMaxBlockSize) {
            result = heap.Relocate_AssumeLast(ptr, newSize, oldSize);
        }
        else if (keepData) {
            void* const dst = FAllocaFallback_::Malloc(newSize);
            ::memcpy(dst, ptr, Min(oldSize, newSize));
            result = dst;
        }
        else {
            heap.Release_AssumeLast(ptr, oldSize);
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

    auto& heap = FAllocaLinearHeapTLS_::Instance();
    if (heap.AliasesToHeap(ptr))
        heap.Release_AssumeLast(ptr, size);
    else
        FAllocaFallback_::Free(ptr);
}
//----------------------------------------------------------------------------
size_t AllocaSnapSize(size_t size) {
    return ((size <= FAllocaLinearHeapTLS_::GMaxBlockSize)
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
} //!namespace Core
