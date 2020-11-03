#pragma once

#include "Core.h"

#include "Container/CompressedRadixTrie.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformHash.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Meta/Utility.h"
#include "Thread/ReadWriteLock.h"

#if USE_PPE_ASSERT
#   include "HAL/PlatformDebug.h"
#   include "IO/TextWriter.h"
#endif

#include <atomic>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBitmapGenericTraits {
    STATIC_CONST_INTEGRAL(size_t, Granularity, 0);
    static void* PageReserve(size_t sizeInBytes) = delete;
    static void PageRelease(void* ptr, size_t sizeInBytes) = delete;
    static void PageCommit(void* ptr, size_t sizeInBytes) = delete;
    static void PageDecommit(void* ptr, size_t sizeInBytes) = delete;
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _MemoryDomainTag, u32 _Granularity>
#else
template <u32 _Granularity>
#endif
struct TBitmapCpuTraits {
    STATIC_CONST_INTEGRAL(u32, Granularity, _Granularity);
    static void* PageReserve(size_t sizeInBytes) { return FVirtualMemory::PageReserve(sizeInBytes, sizeInBytes); }
    static void PageRelease(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageRelease(ptr, sizeInBytes); }
#if USE_PPE_MEMORYDOMAINS
    using domain_tag = _MemoryDomainTag;
    static FMemoryTracking& TrackingData() NOEXCEPT { return domain_tag::TrackingData(); }
    static void PageCommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageCommit(ptr, sizeInBytes, TrackingData()); }
    static void PageDecommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageDecommit(ptr, sizeInBytes, TrackingData()); }
#else
    static void PageCommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageCommit(ptr, sizeInBytes); }
    static void PageDecommit(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageDecommit(ptr, sizeInBytes); }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBitmapMemoryStats {
    u32 NumAllocations;
    u32 LargestFreeBlock;

    size_t TotalSizeAllocated;
    size_t TotalSizeAvailable;
    size_t TotalSizeCommitted;

    CONSTEXPR friend FBitmapMemoryStats operator +(const FBitmapMemoryStats& lhs, const FBitmapMemoryStats& rhs) NOEXCEPT {
        return FBitmapMemoryStats{
            lhs.NumAllocations + rhs.NumAllocations,
            Max(lhs.LargestFreeBlock, rhs.LargestFreeBlock),
            lhs.TotalSizeAllocated + rhs.TotalSizeAllocated,
            lhs.TotalSizeAvailable + rhs.TotalSizeAvailable,
            lhs.TotalSizeCommitted + rhs.TotalSizeCommitted
        };
    }
};
//----------------------------------------------------------------------------
struct FBitmapPageInfo {
    void* vAddressSpace;
    u64 Pages;
    u64 Sizes;
    FBitmapMemoryStats Stats;
};
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // 'XXX' structure was padded due to alignment
template <u32 _PageSize>
struct TBitmapPage {
    STATIC_ASSERT(Meta::IsPow2(_PageSize));

    using mask_t = CODE3264(u32, u64);
    STATIC_CONST_INTEGRAL(u32, MaxPages, sizeof(mask_t) << 3);
    STATIC_CONST_INTEGRAL(u32, PageSize, _PageSize);
    STATIC_CONST_INTEGRAL(u32, LargeBlockThreshold, 4/* pages */);

    std::atomic<mask_t> Pages{ mask_t(-1) };
    std::atomic<mask_t> Sizes{ 0 };
    void* vAddressSpace{ nullptr };
    TBitmapPage* NextPage{ nullptr };

    u32 Available() const NOEXCEPT;
    u32 Capacity() const NOEXCEPT;
    bool Empty() const NOEXCEPT;
    u32 LargestBlockAvailable() const NOEXCEPT;
    float ExternalFragmentation() const NOEXCEPT;

    FBitmapMemoryStats MemoryStats() const NOEXCEPT;

    bool Aliases(const void* ptr) const NOEXCEPT;
    size_t RegionSize(const void* ptr) const NOEXCEPT;

    void* Allocate(u32 sizeInBytes) NOEXCEPT;
    bool Free_ReturnIfUnused(void* ptr) NOEXCEPT;

    void* Resize(void* ptr, const u32 sizeInBytes) NOEXCEPT; // try to shrink/grow block **INPLACE**

    void DebugInfo(FBitmapPageInfo* pinfo) const NOEXCEPT;

    static u32 SnapSize(u32 sizeInBytes) NOEXCEPT;
    static CONSTEXPR bool Segregate(u32 sizeInBytes) NOEXCEPT {
        return (sizeInBytes > LargeBlockThreshold * PageSize);
    }

};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
struct FBitmapPageCache {
    template <u32 _PageSize>
    static TBitmapPage<_PageSize>* GrabPage() {
        return INPLACE_NEW(GragPage_(sizeof(TBitmapPage<_PageSize>)), TBitmapPage<_PageSize>);
    }

    template <u32 _PageSize>
    static void ReleasePage(TBitmapPage<_PageSize>* page) {
        Meta::Destroy(page);
        ReleasePage_(page, sizeof(TBitmapPage<_PageSize>));
    }

private:
    static PPE_CORE_API void* GragPage_(size_t sz) NOEXCEPT;
    static PPE_CORE_API void ReleasePage_(void* page, size_t sz) NOEXCEPT;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBitmapHeapInfo {
    u32 BitmapSize;
    u32 Granularity;
    u32 PagesPerBlock;
    u32 MinAllocSize;
    u32 MaxAllocSize;
    FBitmapMemoryStats Stats;
    TMemoryView<FBitmapPageInfo> Pages;
};
//----------------------------------------------------------------------------
template <typename _Traits = FBitmapGenericTraits>
struct TBitmapHeap : Meta::FNonCopyableNorMovable {
    using traits_type = _Traits;
    using page_type = TBitmapPage<traits_type::Granularity>;
    using mask_type = typename page_type::mask_t;

    STATIC_CONST_INTEGRAL(u32, Granularity, traits_type::Granularity);
    STATIC_CONST_INTEGRAL(u32, BitmapSize, Granularity * page_type::MaxPages);

    STATIC_CONST_INTEGRAL(u32, MinAllocSize, traits_type::Granularity);
    STATIC_CONST_INTEGRAL(u32, MaxAllocSize, BitmapSize / 2);

#if USE_PPE_MEMORYDOMAINS
    TBitmapHeap() NOEXCEPT;
#else
    TBitmapHeap() = default;
#endif
    ~TBitmapHeap();

    page_type* Aliases(const void* ptr) const NOEXCEPT;
    size_t AllocationSize(const void* ptr) const NOEXCEPT;

    FBitmapMemoryStats MemoryStats() const NOEXCEPT;

    void* Allocate(size_t sizeInBytes);
    void Free(void* ptr, page_type* page = nullptr);

    // try to shrink/grow block **INPLACE**
    void* Resize(void* ptr, size_t sizeInBytes, page_type* page = nullptr) NOEXCEPT;

    void GarbageCollect(); // try to lock
    void ForceGarbageCollect(); // force lock

    template <typename _Each>
    size_t EachPage(_Each&& each) const NOEXCEPT;

    // call twice to know how much to reserve for pinfo->Pages
    size_t DebugInfo(FBitmapHeapInfo* pinfo) const NOEXCEPT;

    static size_t SnapSize(size_t sz) NOEXCEPT {
        return page_type::SnapSize(checked_cast<u32>(sz));
    }

private:
    struct ALIGN(16) FHintTLS {
        struct lrupage_t {
            page_type* Page;
            size_t Revision;
        };

        STATIC_CONST_INTEGRAL(u32, CacheSize, 4);
        lrupage_t LRU[CacheSize] = {};

        void Push(page_type* page, size_t revision) NOEXCEPT {
            forrange(i, 0, CacheSize) {
                if (LRU[i].Page == page) {
                    LRU[i].Revision = revision;
                    if (i > 0)
                        std::swap(LRU[i], LRU[0]);
                    return;
                }
            }
            reverseforrange(i, 1, CacheSize)
                LRU[i] = LRU[i - 1];
            LRU[0] = { page, revision };
        }

        static FHintTLS& Get() NOEXCEPT {
            ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(FHintTLS, GInstance);
            return GInstance;
        }
    };

    // those helpers assumes barrier to be already locked (read or write)
    page_type* AliasingPage_AssumeLocked_(const void* ptr) const NOEXCEPT;
    page_type* FreePage_AssumeLocked_(u32 sizeInBytes) NOEXCEPT;
    void ReleaseUnusedPage_AssumeLocked_(page_type* page) NOEXCEPT;

    static CONSTEXPR page_type* const GDummyPage{ (page_type*)1 };

    FReadWriteLock RWLock;
    size_t Revision{ 0 };
    std::atomic<page_type*> FreePage{ GDummyPage };
    FCompressedRadixTrie Pages;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Allocator/BitmapHeap-inl.h"
