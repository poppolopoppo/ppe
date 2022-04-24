#pragma once

#include "Core.h"

#include "Container/BitTree.h"
#include "Container/CompressedRadixTrie.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformHash.h"
#include "HAL/PlatformMaths.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Meta/PointerWFlags.h"
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
    static void* PageReserve(size_t alignment, size_t sizeInBytes) = delete;
    static void PageRelease(void* ptr, size_t sizeInBytes) = delete;
    static void PageCommit(void* ptr, size_t sizeInBytes) = delete;
    static void PageDecommit(void* ptr, size_t sizeInBytes) = delete;
#if USE_PPE_MEMORYDOMAINS
    static void OnUserAlloc(void* ptr, size_t sizeInBytes) = delete;
    static void OnUserFree(void* ptr, size_t sizeInBytes) = delete;
#endif
};
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _MemoryDomainTag, u32 _Granularity>
#else
template <u32 _Granularity>
#endif
struct TBitmapCpuTraits {
    STATIC_CONST_INTEGRAL(u32, Granularity, _Granularity);
    static void* PageReserve(size_t alignment, size_t sizeInBytes) { return FVirtualMemory::PageReserve(alignment, sizeInBytes); }
    static void PageRelease(void* ptr, size_t sizeInBytes) { FVirtualMemory::PageRelease(ptr, sizeInBytes); }
#if USE_PPE_MEMORYDOMAINS
    using domain_tag = _MemoryDomainTag;
    static FMemoryTracking& TrackingData() NOEXCEPT { return domain_tag::TrackingData(); }
    static void PageCommit(void* ptr, size_t sizeInBytes) {
        FVirtualMemory::PageCommit(ptr, sizeInBytes, TrackingData());
        PPE_DEBUG_POISONMEMORY(ptr, sizeInBytes); // ASAN
    }
    static void PageDecommit(void* ptr, size_t sizeInBytes) {
        PPE_DEBUG_UNPOISONMEMORY(ptr, sizeInBytes); // ASAN
        FVirtualMemory::PageDecommit(ptr, sizeInBytes, TrackingData());
    }
    static void OnUserAlloc(void* ptr, size_t sizeInBytes) {
        Unused(ptr);
        PPE_DEBUG_UNPOISONMEMORY(ptr, sizeInBytes); // ASAN
        TrackingData().AllocateUser(sizeInBytes);
    }
    static void OnUserFree(void* ptr, size_t sizeInBytes) {
        Unused(ptr);
        PPE_DEBUG_POISONMEMORY(ptr, sizeInBytes); // ASAN
        TrackingData().DeallocateUser(sizeInBytes);
    }
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
struct CACHELINE_ALIGNED FBitmapBasicPage {
    using mask_t = CODE3264(u32, u64);

    std::atomic<mask_t> Pages;
    std::atomic<mask_t> Sizes;
    void* vAddressSpace;
    FBitmapBasicPage* NextPage;

    void Reset(void* vaddr) {
        Pages = mask_t(-1);
        Sizes = 0;
        vAddressSpace = vaddr;
        NextPage = nullptr;
    }

    static PPE_CORE_API void ReleaseCacheMemory() NOEXCEPT;

private:
    template <typename _Traits>
    friend struct TBitmapHeap;

    static PPE_CORE_API FBitmapBasicPage* Allocate();
    static PPE_CORE_API void Deallocate(FBitmapBasicPage* p) NOEXCEPT;
};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
template <u32 _PageSize>
struct TBitmapPage : FBitmapBasicPage {
    STATIC_ASSERT(Meta::IsPow2(_PageSize));

    STATIC_CONST_INTEGRAL(u32, MaxPages, sizeof(mask_t) << 3);
    STATIC_CONST_INTEGRAL(u32, PageSize, _PageSize);
    STATIC_CONST_INTEGRAL(u32, LargeBlockThreshold, 4/* pages */);

    u32 Available() const NOEXCEPT;
    u32 Capacity() const NOEXCEPT;
    bool Empty() const NOEXCEPT;
    u32 LargestBlockAvailable() const NOEXCEPT;
    float ExternalFragmentation() const NOEXCEPT;

    FBitmapMemoryStats MemoryStats() const NOEXCEPT;

    bool Aliases(const void* ptr) const NOEXCEPT;
    size_t RegionSize(const void* ptr) const NOEXCEPT;

    void* Allocate(u32 sizeInBytes, bool& exhausted) NOEXCEPT;
    bool Free_ReturnIfUnused(void* ptr) NOEXCEPT;

    void* Resize(void* ptr, const u32 sizeInBytes, bool& exhausted) NOEXCEPT; // try to shrink/grow block **INPLACE**

    void DebugInfo(FBitmapPageInfo* pinfo) const NOEXCEPT;

    static u32 SnapSize(u32 sizeInBytes) NOEXCEPT;
    static CONSTEXPR bool Segregate(u32 sizeInBytes) NOEXCEPT {
        return (sizeInBytes > LargeBlockThreshold * PageSize);
    }

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
    struct ALIGN(16) FStackMRU {
        struct lrupage_t {
            page_type* Page;
            size_t Revision;
        };

        STATIC_CONST_INTEGRAL(u32, CacheSize, 4);
        lrupage_t MRU[CacheSize] = {};

        void Push(page_type* page, size_t revision) NOEXCEPT {
            for (auto it = std::begin(MRU), end = std::end(MRU); it != end; ++it) {
                if (it->Page == page) {
                    it->Revision = revision;
                    std::rotate(std::begin(MRU), it, it + 1);
                    return;
                }
            }

            std::rotate(std::begin(MRU), std::begin(MRU) + (CacheSize - 1), std::end(MRU));
            MRU[0] = { page, revision };
        }

        static FStackMRU& Tls() NOEXCEPT {
            ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(FStackMRU, GInstance);
            return GInstance;
        }
    };

    // those helpers assumes barrier to be already locked (read or write)
    page_type* AliasingPage_AssumeLocked_(const void* ptr) const NOEXCEPT;
    page_type* FindFreePage_AssumeLocked_(u32 sizeInBytes) NOEXCEPT;
    void RegisterFreePage_Unlocked_(page_type* page) NOEXCEPT;
    void ReleaseUnusedPage_AssumeLocked_(page_type* page) NOEXCEPT;

    static inline page_type* const GDummyPage{ (page_type*)1 }; // avoid double registration

#if USE_PPE_MEMORYDOMAINS
    FMemoryTracking TrackingData;
#endif

    FReadWriteLock RWLock;
    size_t Revision{ 0 };
    std::atomic<page_type*> FreePage{ GDummyPage };
    FCompressedRadixTrie Pages;
};
//----------------------------------------------------------------------------
template <size_t _ReservedSize, typename _Traits = FBitmapGenericTraits>
struct TBitmapFixedSizeHeap : Meta::FNonCopyableNorMovable {
    using traits_type = _Traits;
    using page_type = TBitmapPage<traits_type::Granularity>;
    using mask_type = typename page_type::mask_t;

    STATIC_CONST_INTEGRAL(u32, Granularity, traits_type::Granularity);
    STATIC_CONST_INTEGRAL(u32, BitmapSize, Granularity* page_type::MaxPages);

    STATIC_CONST_INTEGRAL(u32, MinAllocSize, traits_type::Granularity);
    STATIC_CONST_INTEGRAL(u32, MaxAllocSize, BitmapSize / 2);

    STATIC_CONST_INTEGRAL(size_t, ReservedSize, _ReservedSize);
    STATIC_CONST_INTEGRAL(u32, NumReservedPages, u32(_ReservedSize / BitmapSize));
    STATIC_CONST_INTEGRAL(u32, MaxExhaustedPages, 1);

    TBitmapFixedSizeHeap() NOEXCEPT;
    ~TBitmapFixedSizeHeap() NOEXCEPT;

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
    struct ALIGN(16) FStackMRU {
        STATIC_CONST_INTEGRAL(u32, CacheSize, 4);
        u32 MRU[CacheSize] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };

        void Push(u32 pageIndex) NOEXCEPT {
            auto view = MakeView(MRU);
            const auto it = view.Find(pageIndex);
            if (view.end() != it) {
                std::rotate(view.begin(), it, it + 1);
            }
            else {
                std::rotate(view.begin(), view.end() + (CacheSize - 1), view.end());
                MRU[0] = pageIndex;
            }
        }

        static FStackMRU& Tls() NOEXCEPT {
            ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(FStackMRU, GInstance);
            return GInstance;
        }
    };

    FReadWriteLock RWLock;
    u8* vAddressSpace;
    page_type* PageTable;
    TAtomicBitTree<u64> CommittedPages;
    TAtomicBitTree<u64> ExhaustedPages;
    u32 NumExhaustedPages;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Allocator/BitmapHeap-inl.h"
