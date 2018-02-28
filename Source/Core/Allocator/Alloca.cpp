#include "stdafx.h"

#include "Alloca.h"

#include "Diagnostic/Logger.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"
#include "Meta/Singleton.h"
#include "Misc/TargetPlatform.h"

#include "ThreadLocalHeap.h"

#ifdef USE_DEBUG_LOGGER
#   include "IO/FormatHelpers.h"
#endif

#if USE_CORE_MEMORY_DEBUGGING
#   define WITH_CORE_ALLOCA_FALLBACK_TO_MALLOC
#endif

#ifdef WITH_CORE_ASSERT
#   define WITH_CORE_ALLOCA_CANARY
#endif

namespace Core {
LOG_CATEGORY(CORE_API, Alloca);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void* AllocaFallback_Malloc_(size_t sizeInBytes);
static void AllocaFallback_Free_(void* p);
static void* AllocaFallback_Realloc_(void* p, size_t newSizeInBytes, bool keepData);
static size_t AllocaFallback_SnapSize_(size_t sz);
//----------------------------------------------------------------------------
static void* AllocaLocalStorage_Alloc_();
static void AllocaLocalStorage_Deallocate_(void* p);
//----------------------------------------------------------------------------
class FAllocaStorage {
public:
    enum : u32 {
    // memory reserved per thread for alloca
#if defined(ARCH_X64)
        Capacity        =128<<10    /* 128 kb   */
#elif defined(ARCH_X86)
        Capacity        = 64<<10    /*  64 kb   */
#endif

    // each allocation are guaranteed to be aligned on :
    ,   Boundary        = 16        /* 16 b     */

    // each allocation fallback on TLH when bigger than :
#if defined(ARCH_X64)
    ,   MaxBlockSize    = 32<<10    /* 32 kb    */
#else
    ,   MaxBlockSize    = 16<<10    /* 16 kb    */
#endif

    // internal configuration :

    ,   HeaderSize      = Boundary  // keeps blocks aligned on boundary

#ifdef WITH_CORE_ALLOCA_CANARY
    ,   HeaderCanary    = 0xABADCAFE

    ,   FooterCanary    = 0xDEADBABE
    ,   FooterSize      = Boundary // keeps blocks aligned on boundary
#else
    ,   FooterSize      = 0
#endif
    ,   PayloadSize     = HeaderSize + FooterSize
    };
    STATIC_ASSERT(Boundary == ALLOCATION_BOUNDARY);
    STATIC_ASSERT(ROUND_TO_NEXT_16(PayloadSize) == PayloadSize);

    FAllocaStorage();
    ~FAllocaStorage();

    bool AliasesToLocalStorage(const void* ptr) const {
        return (size_t(ptr) > size_t(_storage) &&
                size_t(ptr) < size_t(_storage) + _offset);
    }

    void* Push(size_t sizeInBytes);
    void Pop(void* ptr);

    void* Relocate(void* ptr, size_t newSizeInBytes, bool keepData);

private:
    u32 _offset;
    void* _storage;
};
//----------------------------------------------------------------------------
FAllocaStorage::FAllocaStorage()
:   _offset(0)
,   _storage(nullptr) {
}
//----------------------------------------------------------------------------
FAllocaStorage::~FAllocaStorage() {
    Assert(0 == _offset); // Check that all used memory has been released

    if (_storage)
        AllocaLocalStorage_Deallocate_(_storage);
}
//----------------------------------------------------------------------------
void* FAllocaStorage::Push(size_t sizeInBytes) {
    Assert(sizeInBytes > 0);

    const size_t alignedSizeInBytes = ROUND_TO_NEXT_16(sizeInBytes);
    Assert(alignedSizeInBytes <= MaxBlockSize);

    if (alignedSizeInBytes + PayloadSize + _offset <= Capacity) {

        if (nullptr == _storage) {
            Assert(0  == _offset);
            _storage = AllocaLocalStorage_Alloc_();
        }

        Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, _offset));
        void* const block = reinterpret_cast<u8*>(_storage) + _offset;

        const u32 blockSize = checked_cast<u32>(alignedSizeInBytes + PayloadSize);

        u32* const header = reinterpret_cast<u32*>(block);
        void* const userBlock = reinterpret_cast<u8*>(block) + HeaderSize;

        _offset += (*header = blockSize);

#ifdef WITH_CORE_ALLOCA_CANARY
        header[1] = checked_cast<u32>(sizeInBytes);
        header[2] = HeaderCanary;
        header[3] = HeaderCanary;

        u32* const footer = reinterpret_cast<u32*>((u8*)&header[4] + sizeInBytes);
        footer[0] = FooterCanary;
        footer[1] = FooterCanary;
        footer[2] = FooterCanary;
        footer[3] = header[1];
#endif

        Assert(Meta::IsAligned(Boundary, userBlock));
        return userBlock;
    }
    else {
        // Fallback on thread local heap allocations if :
        // - asked block size is too large (> MaxBlockSize)
        // - no more space available in _storage (should not happen ...)
        return AllocaFallback_Malloc_(sizeInBytes);
    }
}
//----------------------------------------------------------------------------
void FAllocaStorage::Pop(void* ptr) {
    Assert(uintptr_t(ptr) > HeaderSize);

    if (AliasesToLocalStorage(ptr)) {
        Assert(_storage);

        void* const block = reinterpret_cast<u8*>(ptr) - HeaderSize;
        const u32* header = reinterpret_cast<u32*>(block);
        const u32 blockSize = *header;

        // Can't delete any other block than the last one allocated
        AssertRelease(uintptr_t(block) + blockSize == uintptr_t(_storage) + _offset);

        Assert(blockSize <= _offset);
        Assert(blockSize <= checked_cast<u32>(ROUND_TO_NEXT_16(MaxBlockSize) + PayloadSize));

#ifdef WITH_CORE_ALLOCA_CANARY
        const u32 userSizeInBytes = header[1];
        Assert(header[2] == HeaderCanary);
        Assert(header[3] == HeaderCanary);

        const u32* footer = reinterpret_cast<const u32*>((const u8*)&header[4] + userSizeInBytes);
        Assert(footer[0] == FooterCanary);
        Assert(footer[1] == FooterCanary);
        Assert(footer[2] == FooterCanary);
        Assert(footer[3] == userSizeInBytes);
#endif

#ifdef WITH_CORE_ASSERT
        ::memset(block, 0xDD, blockSize);
#endif

        _offset -= blockSize;
    }
    else {
        AllocaFallback_Free_(ptr);
    }
}
//----------------------------------------------------------------------------
void* FAllocaStorage::Relocate(void* ptr, size_t newSizeInBytes, bool keepData) {
    Assert(uintptr_t(ptr) > HeaderSize);
    Assert(newSizeInBytes);

    const size_t alignedSizeInBytes = ROUND_TO_NEXT_16(newSizeInBytes);

    if (AliasesToLocalStorage(ptr)) {
        Assert(_storage);

        void* const block = reinterpret_cast<u8*>(ptr) - HeaderSize;
        u32* const header = reinterpret_cast<u32*>(block);
        u32 blockSize = *header;

        // Can't relocate any other block than the last one allocated
        AssertRelease(uintptr_t(block) + blockSize == uintptr_t(_storage) + _offset);

        Assert(blockSize <= _offset);
        Assert(blockSize <= checked_cast<u32>(ROUND_TO_NEXT_16(MaxBlockSize) + PayloadSize));

#ifdef WITH_CORE_ALLOCA_CANARY
        const u32 userSizeInBytes = header[1];
        Assert(header[2] == HeaderCanary);
        Assert(header[3] == HeaderCanary);

        const u32* footer = reinterpret_cast<const u32*>((const u8*)&header[4] + userSizeInBytes);
        Assert(footer[0] == FooterCanary);
        Assert(footer[1] == FooterCanary);
        Assert(footer[2] == FooterCanary);
        Assert(footer[3] == userSizeInBytes);
#endif

        if (uintptr_t(block) + alignedSizeInBytes + PayloadSize <= uintptr_t(_storage) + Capacity) {
            _offset -= blockSize;
            blockSize = checked_cast<u32>(alignedSizeInBytes + PayloadSize);
            _offset += blockSize;
            *header = blockSize;

            Assert(uintptr_t(_storage) + _offset == uintptr_t(block) + blockSize);

#ifdef WITH_CORE_ALLOCA_CANARY
            header[1] = checked_cast<u32>(newSizeInBytes);
            header[2] = HeaderCanary;
            header[3] = HeaderCanary;

            u32* const newFooter = reinterpret_cast<u32*>((u8*)&header[4] + newSizeInBytes);
            newFooter[0] = FooterCanary;
            newFooter[1] = FooterCanary;
            newFooter[2] = FooterCanary;
            newFooter[3] = header[1];
#endif
        }
        else { // not enough place in the stack local space, fallback to thread local allocation
            void* newPtr = AllocaFallback_Malloc_(newSizeInBytes);

            if (keepData)
                ::memcpy(newPtr, ptr, Min(newSizeInBytes, blockSize));

            Pop(ptr);

            ptr = newPtr;
        }

        return ptr;
    }
    else {
        return AllocaFallback_Realloc_(ptr, newSizeInBytes, keepData);
    }
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
constexpr size_t GAllocaFallbackPayload = 4 * sizeof(u32); // keep memory aligned on 16
constexpr u32 GAllocaFallbackAliveCanary = 0x0A110CA;
constexpr u32 GAllocaFallbackDeadCanary = 0xDEADBEEF;
#endif
//----------------------------------------------------------------------------
static void* AllocaFallback_Malloc_(size_t sizeInBytes) {
#ifdef USE_MEMORY_DOMAINS
    // Need a payload to track memory allocated for Alloca()
    if (void* p = GetThreadLocalHeap().Malloc(sizeInBytes + GAllocaFallbackPayload)) {
        u32* const payload = (u32*)p;
        payload[0] = checked_cast<u32>(sizeInBytes);
        ONLY_IF_ASSERT(payload[1] = GAllocaFallbackAliveCanary);
        ONLY_IF_ASSERT(payload[2] = GAllocaFallbackAliveCanary);
        ONLY_IF_ASSERT(payload[3] = GAllocaFallbackAliveCanary);

        MEMORY_DOMAIN_TRACKING_DATA(Alloca).Allocate(1, sizeInBytes);

        p = (payload + 4);
        Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, p));

        return p;
    }
    return nullptr;
#else
    return GetThreadLocalHeap().Malloc(sizeInBytes);
#endif
}
//----------------------------------------------------------------------------
static void AllocaFallback_Free_(void* p) {
#ifdef USE_MEMORY_DOMAINS
    u32* const payload = (u32*)p - 4;
    Assert(GAllocaFallbackAliveCanary == payload[1]);
    Assert(GAllocaFallbackAliveCanary == payload[2]);
    Assert(GAllocaFallbackAliveCanary == payload[3]);

    // Without payload we wouldn't know how much memory we just freed
    const size_t sizeInBytes = payload[0];
    MEMORY_DOMAIN_TRACKING_DATA(Alloca).Deallocate(1, sizeInBytes);

    ONLY_IF_ASSERT(payload[0] = GAllocaFallbackDeadCanary);
    ONLY_IF_ASSERT(payload[1] = GAllocaFallbackDeadCanary);
    ONLY_IF_ASSERT(payload[2] = GAllocaFallbackDeadCanary);
    ONLY_IF_ASSERT(payload[3] = GAllocaFallbackDeadCanary);

    GetThreadLocalHeap().Free(payload);
#else
    GetThreadLocalHeap().Free(p);
#endif
}
//----------------------------------------------------------------------------
static void* AllocaFallback_Realloc_(void* p, size_t newSizeInBytes, bool keepData) {
#ifdef USE_MEMORY_DOMAINS
    if (p) {
        u32* const oldPayload = (u32*)p - 4;
        Assert(GAllocaFallbackAliveCanary == oldPayload[1]);
        Assert(GAllocaFallbackAliveCanary == oldPayload[2]);
        Assert(GAllocaFallbackAliveCanary == oldPayload[3]);

        const size_t oldSizeInBytes = oldPayload[0];
        MEMORY_DOMAIN_TRACKING_DATA(Alloca).Deallocate(1, oldSizeInBytes);

        if (oldSizeInBytes == newSizeInBytes)
            return p;

        void* newP = nullptr;
        if (newSizeInBytes) {
            u32* const newPayload = (u32*)GetThreadLocalHeap().Malloc(newSizeInBytes);

            if (newPayload)
            {
                MEMORY_DOMAIN_TRACKING_DATA(Alloca).Allocate(1, newSizeInBytes);

                newPayload[0] = checked_cast<u32>(newSizeInBytes);
                newPayload[1] = GAllocaFallbackAliveCanary;
                newPayload[2] = GAllocaFallbackAliveCanary;
                newPayload[3] = GAllocaFallbackAliveCanary;

                newP = (newPayload + 4);
                Assert(Meta::IsAligned(ALLOCATION_BOUNDARY, newP));

                ::memcpy(newP, p, Min(newSizeInBytes, oldSizeInBytes));
            }
        }

        ONLY_IF_ASSERT(oldPayload[0] = GAllocaFallbackDeadCanary);
        ONLY_IF_ASSERT(oldPayload[1] = GAllocaFallbackDeadCanary);
        ONLY_IF_ASSERT(oldPayload[2] = GAllocaFallbackDeadCanary);
        ONLY_IF_ASSERT(oldPayload[3] = GAllocaFallbackDeadCanary);

        GetThreadLocalHeap().Free(oldPayload);

        return newP;
    }
    else {
        return AllocaFallback_Malloc_(newSizeInBytes);
    }
#else
    return GetThreadLocalHeap().Realloc(p, newSizeInBytes);
#endif
}
//----------------------------------------------------------------------------
static size_t AllocaFallback_SnapSize_(size_t sz) {
    return GetThreadLocalHeap().SnapSize(sz);
}
//----------------------------------------------------------------------------
static void* AllocaLocalStorage_Alloc_() {
    void* const p = FVirtualMemory::AlignedAlloc(FPlatformMisc::SystemInfo.AllocationGranularity, FAllocaStorage::Capacity);
    AssertRelease(p);
    Assert(Meta::IsAligned(FAllocaStorage::Boundary, p));

#ifdef USE_MEMORY_DOMAINS
    MEMORY_DOMAIN_TRACKING_DATA(Alloca).Allocate(1, FAllocaStorage::Capacity);
#endif

    LOG(Alloca, Debug, L"allocate storage {0} [{1}]",
        Fmt::Pointer(p),
        Fmt::SizeInBytes(size_t(FAllocaStorage::Capacity)) );

    return p;
}
//----------------------------------------------------------------------------
static void AllocaLocalStorage_Deallocate_(void* p) {
    Assert(p);
    Assert(Meta::IsAligned(FPlatformMisc::SystemInfo.AllocationGranularity, p));

    LOG(Alloca, Debug, L"release storage {0} [{1}]",
        Fmt::Pointer(p),
        Fmt::SizeInBytes(size_t(FAllocaStorage::Capacity)) );

#ifdef USE_MEMORY_DOMAINS
    MEMORY_DOMAIN_TRACKING_DATA(Alloca).Deallocate(1, FAllocaStorage::Capacity);
#endif

    FVirtualMemory::AlignedFree(p, FAllocaStorage::Capacity);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class FThreadLocalAllocaStorage : Meta::TThreadLocalSingleton<FAllocaStorage, FThreadLocalAllocaStorage> {
    typedef Meta::TThreadLocalSingleton<FAllocaStorage, FThreadLocalAllocaStorage> parent_type;
public:
    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* Alloca(size_t sizeInBytes) {
#ifndef WITH_CORE_ALLOCA_FALLBACK_TO_MALLOC
    if (0 == sizeInBytes)
        return nullptr;

    return (sizeInBytes <= FAllocaStorage::MaxBlockSize)
        ? FThreadLocalAllocaStorage::Instance().Push(sizeInBytes)
        : AllocaFallback_Malloc_(sizeInBytes);

#else
    return Core::malloc(sizeInBytes);

#endif
}
//----------------------------------------------------------------------------
void* RelocateAlloca(void* ptr, size_t newSizeInBytes, bool keepData) {
#ifndef WITH_CORE_ALLOCA_FALLBACK_TO_MALLOC
    if (ptr) {
        if (0 == newSizeInBytes) {
            FreeAlloca(ptr);
            return nullptr;
        }
        else {
            return FThreadLocalAllocaStorage::Instance().Relocate(ptr, newSizeInBytes, keepData);
        }
    }
    else if (newSizeInBytes) {
        return (newSizeInBytes <= FAllocaStorage::MaxBlockSize)
            ? FThreadLocalAllocaStorage::Instance().Push(newSizeInBytes)
            : AllocaFallback_Malloc_(newSizeInBytes);
    }
    else {
        return nullptr;
    }

#else
    return Core::realloc(ptr, newSizeInBytes);

#endif
}
//----------------------------------------------------------------------------
void FreeAlloca(void* ptr) {
#ifndef WITH_CORE_ALLOCA_FALLBACK_TO_MALLOC
    if (!ptr)
        return;

    FThreadLocalAllocaStorage::Instance().Pop(ptr);

#else
    Core::free(ptr);

#endif
}
//----------------------------------------------------------------------------
size_t AllocaSnapSize(size_t sz) {
    return (sz <= FAllocaStorage::MaxBlockSize
        ? ROUND_TO_NEXT_16(sz)
        : AllocaFallback_SnapSize_(sz) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FAllocaStartup::Start(bool/* mainThread */) {
#ifndef WITH_CORE_ALLOCA_FALLBACK_TO_MALLOC
    FThreadLocalAllocaStorage::Create();
#endif
}
//----------------------------------------------------------------------------
void FAllocaStartup::Shutdown() {
#ifndef WITH_CORE_ALLOCA_FALLBACK_TO_MALLOC
    FThreadLocalAllocaStorage::Destroy();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
