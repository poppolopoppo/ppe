#pragma once

#include "Allocator/BitmapHeap.h"

#if USE_PPE_MEMORYDOMAINS
#   include "Meta/TypeInfo.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline bool TBitmapPage<_PageSize>::Aliases(const void* ptr) const NOEXCEPT {
    Assert(ptr);
    return FPlatformMemory::Memaliases(vAddressSpace, MaxPages * PageSize, ptr);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::Available() const NOEXCEPT {
    return u32(FPlatformMaths::popcnt(Pages.load(std::memory_order_relaxed)) * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::Capacity() const NOEXCEPT {
    return (MaxPages * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline bool TBitmapPage<_PageSize>::Empty() const NOEXCEPT {
    const bool emptySizes = (Sizes.load(std::memory_order_relaxed) == 0);
#if USE_PPE_ASSERT
    const bool emptyPages = (Pages.load(std::memory_order_relaxed) == mask_t(-1));
    Assert_NoAssume(emptyPages == emptySizes);
#endif
    return emptySizes;
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::LargestBlockAvailable() const NOEXCEPT {
    return (FPlatformMaths::ContiguousBits(Pages.load(std::memory_order_relaxed)) * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline float TBitmapPage<_PageSize>::ExternalFragmentation() const NOEXCEPT {
    const u32 avail = Available();
    return (avail ? (100.f - (LargestBlockAvailable() * 100.f) / avail) : 0.f);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline size_t TBitmapPage<_PageSize>::RegionSize(const void* ptr) const NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(Aliases(ptr));

    const u32 off = checked_cast<u32>(((u8*)ptr - (u8*)vAddressSpace) / PageSize);
    Assert(off < MaxPages);

    const u32 numPages = (u32(FPlatformMaths::tzcnt(Sizes >> off)) + 1);
    Assert(numPages < MaxPages);
    Assert_NoAssume((Pages.load(std::memory_order_relaxed) & ((mask_t(-1) >> (MaxPages - numPages))) << off) == 0);

    return (numPages * PageSize);
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline auto TBitmapPage<_PageSize>::MemoryStats() const NOEXCEPT -> FBitmapMemoryStats {
    Assert(vAddressSpace);

    const u64 pages = Pages.load(std::memory_order_relaxed);
    const u64 sizes = Sizes.load(std::memory_order_relaxed);
    const u32 numPagesAvailable = u32(FPlatformMaths::popcnt(pages));

    return FBitmapMemoryStats{
        u32(FPlatformMaths::popcnt(sizes)),
        FPlatformMaths::ContiguousBits(pages) * PageSize,
        (MaxPages - numPagesAvailable) * PageSize,
        numPagesAvailable * PageSize,
        MaxPages * PageSize
    };
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline void* TBitmapPage<_PageSize>::Allocate(u32 sizeInBytes, bool& exhausted) NOEXCEPT {
    Assert(sizeInBytes);
    Assert_NoAssume(sizeInBytes < Capacity());
    Assert_NoAssume(Meta::IsAlignedPow2(PageSize, sizeInBytes));

    const bool ate = Segregate(sizeInBytes); // reverse search order for larger blocks
    const u32 numPages = ((sizeInBytes + PageSize - 1) / PageSize);
    Assert(numPages <= MaxPages);

    mask_t pages = Pages.load(std::memory_order_relaxed);
    for (;;) {
    RETRY_CAS:
        const mask_t blk = (mask_t(-1) >> (MaxPages - numPages));
        mask_t lookup = pages;
        if (ate) {
            lookup = FPlatformMaths::ReverseBits(lookup);
            Assert_NoAssume(FPlatformMaths::ReverseBits(lookup) == pages);
        }

        for (u32 off = checked_cast<u32>(FPlatformMaths::tzcnt(lookup)); (off != MaxPages) & !!(lookup >> off); ++off) {
            if ((blk & (lookup >> off)) == blk) {
                if (ate)
                    off = MaxPages - off - numPages;
                Assert(off + numPages <= MaxPages);

                const mask_t insert = (pages & ~(blk << off));
                if (Pages.compare_exchange_weak(pages, insert, std::memory_order_release, std::memory_order_relaxed)) {
                    Assert(off + numPages - 1 < MaxPages);
                    const mask_t backup = (Sizes |= (mask_t(1) << (off + numPages - 1)));
                    Unused(backup);
                    void* const result = (reinterpret_cast<u8*>(vAddressSpace) + PageSize * off);
                    Assert_NoAssume(backup & (mask_t(1) << (off + numPages - 1)));
                    Assert_NoAssume(RegionSize(result) == SnapSize(sizeInBytes));
                    exhausted = (insert == 0);
                    return result;
                }

                FPlatformAtomics::ShortSyncWait();
                goto RETRY_CAS;
            }
        }

        Assert_NoAssume(FPlatformMaths::ContiguousBits(pages) < numPages);
        return nullptr;
    }
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline bool TBitmapPage<_PageSize>::Free_ReturnIfUnused(void* ptr) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(Aliases(ptr));

    const u32 off = checked_cast<u32>(((u8*)ptr - (u8*)vAddressSpace) / PageSize);
    Assert(off < MaxPages);

    const u32 numPages = u32(FPlatformMaths::tzcnt(Sizes >> off) + 1);
    Assert(off + numPages <= MaxPages);

    // remove size tag before reseting allocation mask,
    // because we can't guarantee thread safety after the next compare_exchange_weak()
    Sizes &= ~(mask_t(1) << (off + numPages - 1));

    mask_t pages = Pages.load(std::memory_order_relaxed);
    for (;;) {
        Assert_NoAssume((pages & ((mask_t(-1) >> (MaxPages - numPages)) << off)) == 0);
        const mask_t remove = (pages | ((mask_t(-1) >> (MaxPages - numPages)) << off));
        Assert(pages != remove);

        if (Pages.compare_exchange_weak(pages, remove, std::memory_order_release, std::memory_order_relaxed))
            return (remove == mask_t(-1));

        FPlatformAtomics::ShortSyncWait();
    }
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline void* TBitmapPage<_PageSize>::Resize(void* ptr, u32 sizeInBytes, bool& exhausted) NOEXCEPT {
    Assert(ptr);
    Assert(sizeInBytes);
    Assert_NoAssume(Aliases(ptr));
    Assert_NoAssume(sizeInBytes < Capacity());
    Assert_NoAssume(Meta::IsAlignedPow2(PageSize, sizeInBytes));

    const u32 off = checked_cast<u32>(((u8*)ptr - (u8*)vAddressSpace) / PageSize);
    Assert(off < MaxPages);

    const u32 newNumPages = (sizeInBytes / PageSize);
    Assert(newNumPages < MaxPages);
    Assert_NoAssume(newNumPages * PageSize == sizeInBytes);
    const u32 oldNumPages = checked_cast<u32>(FPlatformMaths::tzcnt(Sizes.load(std::memory_order_relaxed) >> off) + 1);
    Assert(oldNumPages < MaxPages);
    Assert(off + oldNumPages <= MaxPages);

    AssertRelease(oldNumPages != newNumPages);
    if (off + newNumPages > MaxPages)
        return nullptr;

    const mask_t oblk = (mask_t(-1) >> (MaxPages - oldNumPages)) << off;
    const mask_t nblk = (mask_t(-1) >> (MaxPages - newNumPages)) << off;

    Sizes &= ~(mask_t(1) << (off + oldNumPages - 1));

    mask_t pages = Pages.load(std::memory_order_relaxed);
    for (;;) {
        Assert_NoAssume((pages & oblk) == 0);
        mask_t resize = (pages | oblk);

        if ((nblk & resize) == nblk) {
            resize &= ~nblk;

            if (Pages.compare_exchange_weak(pages, resize, std::memory_order_release, std::memory_order_relaxed)) {
                Sizes |= (mask_t(1) << (off + newNumPages - 1));
                Assert_NoAssume(RegionSize(ptr) == SnapSize(sizeInBytes));
                exhausted = (resize == 0);
                return ptr;
            }

            FPlatformAtomics::ShortSyncWait();
        }
        else {
            break;
        }
    }

    Sizes |= (mask_t(1) << (off + oldNumPages - 1));
    return nullptr;
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline u32 TBitmapPage<_PageSize>::SnapSize(u32 sz) NOEXCEPT {
    Assert(sz);
    STATIC_ASSERT(Meta::IsPow2(PageSize));
    return ((0 == sz) ? u32(0) : (sz + PageSize - u32(1)) & ~(PageSize - u32(1)));
}
//----------------------------------------------------------------------------
template <u32 _PageSize>
inline void TBitmapPage<_PageSize>::DebugInfo(FBitmapPageInfo* pinfo) const NOEXCEPT {
    Assert(pinfo);

    pinfo->vAddressSpace = vAddressSpace;
    pinfo->Pages = Pages.load(std::memory_order_relaxed);
    pinfo->Sizes = Sizes.load(std::memory_order_relaxed);
    pinfo->Stats = MemoryStats();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Traits>
TBitmapHeap<_Traits>::TBitmapHeap() NOEXCEPT
:   TrackingData(Meta::type_info<TBitmapHeap>.name, &MEMORYDOMAIN_TRACKING_DATA(BitmapHeap))
,   Pages(TrackingData) {
    RegisterTrackingData(&TrackingData);
}
#endif
//----------------------------------------------------------------------------
template <typename _Traits>
TBitmapHeap<_Traits>::~TBitmapHeap() {
    const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);
    Pages.DeleteIf([this](uintptr_t key, uintptr_t value) {
        Unused(key);

        auto* const page = reinterpret_cast<page_type*>(value);
        Assert_NoAssume(uintptr_t(page->vAddressSpace) == key); // check for leaks

        ReleaseUnusedPage_AssumeLocked_(page);
        return true;
    });

    FBitmapBasicPage::ReleaseCacheMemory();

#if USE_PPE_MEMORYDOMAINS
    UnregisterTrackingData(&TrackingData);
#endif
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::Aliases(const void* ptr) const NOEXCEPT -> page_type* {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    return AliasingPage_AssumeLocked_(ptr);
}
//----------------------------------------------------------------------------
template <typename _Traits>
size_t TBitmapHeap<_Traits>::AllocationSize(const void* ptr) const NOEXCEPT {
    Assert(ptr);

    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    page_type* const page = AliasingPage_AssumeLocked_(ptr);
    AssertRelease(page);

    return page->RegionSize(ptr);
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::MemoryStats() const NOEXCEPT -> FBitmapMemoryStats {
    FBitmapMemoryStats mem{};

    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);
    Pages.Foreach([&mem](uintptr_t, uintptr_t value) {
        auto* const page = reinterpret_cast<page_type*>(value);
        mem = mem + page->MemoryStats();
    });

    return mem;
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TBitmapHeap<_Traits>::Allocate(size_t sizeInBytes) {
    Assert(sizeInBytes >= MinAllocSize);
    Assert(sizeInBytes <= MaxAllocSize);
    Assert_NoAssume(Meta::IsAlignedPow2(Granularity, sizeInBytes));

    FStackMRU& hint = FStackMRU::Tls();

    void* result;

    // first try to fetch from hint or from a page already allocated
    {
        const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

        // use thread local least-recently-used hint cache
#if 0 // trivial: first-fit
        for (const auto& MRU : hint.MRU) {
            if (Likely((MRU.Revision == Revision) & (!!MRU.Page))) {
                result = MRU.Page->Allocate(checked_cast<u32>(sizeInBytes));
                if (Likely(result)) {
                    ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
                    return result;
                }
            }
        }
#elif 0 // segregate: try to isolate small blocks from large blocks
        const bool ate = page_type::Segregate(checked_cast<u32>(sizeInBytes));
        for (u32 c = ate ? FStackMRU::CacheSize : 1; ate ? c : c <= FStackMRU::CacheSize; c = ate ? c - 1 : c + 1) {
            const auto& MRU = hint.MRU[c - 1];
            if (Likely((MRU.Revision == Revision) & (!!MRU.Page))) {
                result = MRU.Page->Allocate(checked_cast<u32>(sizeInBytes));
                if (Likely(result)) {
                    ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
                    return result;
                }
            }
        }
#elif 0 // address-first-fit: try to allocate from the lowest block first
        page_type* firstFit = nullptr;
        for (const auto& MRU : hint.MRU) {
            if (Likely((MRU.Revision == Revision) & (!!MRU.Page))) {
                if ((MRU.Page->LargestBlockAvailable() >= sizeInBytes) & ((!firstFit) | (firstFit > MRU.Page)))
                    firstFit = MRU.Page;
            }
        }
        if (Likely(firstFit)) {
            result = firstFit->Allocate(checked_cast<u32>(sizeInBytes));
            if (Likely(result)) {
                ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
                return result;
            }
        }
#elif 1 // address-first-fit+segregate: try to allocate from the lowest block first + reverse order for larger blocks
        const bool ate = page_type::Segregate(checked_cast<u32>(sizeInBytes));
        page_type* firstFit = nullptr;
        for (const auto& MRU : hint.MRU) {
            if (Likely((MRU.Revision == Revision) & (!!MRU.Page))) {
                if ((MRU.Page->LargestBlockAvailable() >= sizeInBytes) & ((!firstFit) | (ate ? firstFit < MRU.Page : firstFit > MRU.Page)))
                    firstFit = MRU.Page;
            }
        }
        if (Likely(firstFit)) {
            bool exhausted = false;
            result = firstFit->Allocate(checked_cast<u32>(sizeInBytes), exhausted);
            if (Likely(result)) {
                ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
                return result;
            }
        }
#else // cost: try to minimize fragmentation across MRU cache
        u32 bestCost = UINT32_MAX;
        page_type* bestFit = nullptr;
        for (const auto& MRU : hint.MRU) {
            if (Likely((MRU.Revision == Revision) & (!!MRU.Page))) {
                u32 cost = FPlatformMaths::Abs(i32(sizeInBytes) - i32(MRU.Page->LargestBlockAvailable()));
                if (cost < bestCost) {
                    bestCost = cost;
                    bestFit = MRU.Page;
                }
            }
        }
        if (Likely(bestFit)) {
            result = bestFit->Allocate(checked_cast<u32>(sizeInBytes));
            if (Likely(result)) {
                ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
                return result;
            }
        }
#endif

        // then look for a page with free space if the hint didn't work
        page_type* const page = FindFreePage_AssumeLocked_(checked_cast<u32>(sizeInBytes));
        if (Likely(page)) {
            bool exhausted = false;
            result = page->Allocate(checked_cast<u32>(sizeInBytes), exhausted);

            if (Likely(result)) {
                hint.Push(page, Revision);
                ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
                return result;
            }
        }
    }

    // otherwise we might need to commit a new page
    return Meta::unlikely([this, sizeInBytes, &hint]() -> void* {
        const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

        // first look again for free space now that we have exclusive access
        page_type* freePage = FindFreePage_AssumeLocked_(checked_cast<u32>(sizeInBytes));

        // then we try to reserve a new page if not enough memory is available
        if (Unlikely(nullptr == freePage)) {
#if USE_PPE_ASSERT && 0 //%_NOCOMMIT%
            FBitmapMemoryStats mem{};
            Pages.Foreach([&mem](uintptr_t, uintptr_t value) {
                auto* const page = reinterpret_cast<page_type*>(value);
                mem = mem + page->MemoryStats();
            });
            Pages.Foreach([&mem](uintptr_t, uintptr_t value) {
                auto* const page = reinterpret_cast<page_type*>(value);
                char buffer[256];
                FFixedSizeTextWriter oss(buffer);
                using mask_t = typename page_type::mask_t;
                const mask_t pages = page->Pages.load(std::memory_order_relaxed);
                const mask_t sizes = page->Sizes.load(std::memory_order_relaxed);
                char tag = 0;
                forrange(b, 0, mask_t(page_type::MaxPages)) {
                    oss << ((pages & (mask_t(1) << b)) ? '.' : char('A' + tag));
                    if (sizes & (mask_t(1) << b)) tag = (tag + 1) & 1;
                }
                oss << "  " << page->LargestBlockAvailable() << Eol << Eos;
                FPlatformDebug::OutputDebug(buffer);
            });
            const float externalFragmentation = (100.f - mem.LargestFreeBlock * 100.f / mem.TotalSizeAvailable);
            char buffer[256];
            FFixedSizeTextWriter oss(buffer);
            oss << "Ask = " << sizeInBytes << " -> " << (sizeInBytes / Granularity) << " pages" << Eol
                << "External fragmentation = " << externalFragmentation << Eol
                << "Largest block = " << mem.LargestFreeBlock << Eol
                << "Total size available = " << mem.TotalSizeAvailable << Eol
                << "Total size committed = " << mem.TotalSizeCommitted << Eol
                << Eos;
            FPlatformDebug::OutputDebug(buffer);
            //Assert_NoAssume(externalFragmentation < 90.f);
            Assert_NoAssume(mem.TotalSizeCommitted < 400*1024*1024);
#endif

            freePage = static_cast<TBitmapPage<Granularity>*>(FBitmapBasicPage::Allocate());
            Assert(freePage);

            freePage->Reset(traits_type::PageReserve(BitmapSize, BitmapSize));

            if (Likely(freePage->vAddressSpace)) {
                Assert_NoAssume(Meta::IsAlignedPow2(BitmapSize, freePage->vAddressSpace));
                traits_type::PageCommit(freePage->vAddressSpace, BitmapSize);
                Pages.Insert(freePage->vAddressSpace, freePage);
            }
            else {
                FBitmapBasicPage::Deallocate(freePage);
                freePage = nullptr; // OOM
            }
        }

        if (Likely(freePage)) {
            // at this point we should have a new page, without any older allocations
            bool exhausted = false;
            void* const ptr = freePage->Allocate(checked_cast<u32>(sizeInBytes), exhausted);
            AssertRelease(ptr);

            hint.Push(freePage, Revision);
            ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(ptr, SnapSize(sizeInBytes)));
            return ptr;
        }

        return nullptr;
    });
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::Free(void* ptr, page_type* page/* = nullptr */) {
    Assert(ptr);

    // first release block and check if we need to run GC
    {
        const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

        if (nullptr == page)
            page = AliasingPage_AssumeLocked_(ptr);
        Assert(page);

        ONLY_IF_MEMORYDOMAINS(traits_type::OnUserFree(ptr, page->RegionSize(ptr)));

        if (Unlikely(page->Free_ReturnIfUnused(ptr) && nullptr == page->NextPage))
            RegisterFreePage_Unlocked_(page);
    }

    // then check if we have too much unused pages (always keep 1 ready)
    page_type* freePage = FreePage.load(std::memory_order_relaxed);
    if (Unlikely(freePage != GDummyPage && freePage->NextPage != GDummyPage))
        GarbageCollect();
}
//----------------------------------------------------------------------------
template <typename _Traits>
void* TBitmapHeap<_Traits>::Resize(void* ptr, size_t sizeInBytes, page_type* page/* = nullptr */) NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    if (nullptr == page)
        page = AliasingPage_AssumeLocked_(ptr);
    Assert(page);

    const size_t prevSizeInBytes = page->RegionSize(ptr);
    if (prevSizeInBytes == SnapSize(sizeInBytes))
        return ptr;

    bool exhausted = false;
    void* const result = page->Resize(ptr, checked_cast<u32>(sizeInBytes), exhausted);

    if (result) {
        ONLY_IF_MEMORYDOMAINS(traits_type::OnUserFree(result, prevSizeInBytes));
        ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, sizeInBytes));
    }

     return result;
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::GarbageCollect() {
    if (Likely(RWLock.TryLockWrite())) {
        for (page_type* page = FreePage.load(std::memory_order_relaxed); page != GDummyPage;) {
            page_type* const pnext = static_cast<page_type*>(page->NextPage);
            Assert(pnext);

            if (page->Empty()) {
                Pages.Erase(page->vAddressSpace);
                ReleaseUnusedPage_AssumeLocked_(page);
            }

            page->NextPage = nullptr;
            page = pnext;
        }

        FreePage = GDummyPage;
        RWLock.UnlockWrite();
    }
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::ForceGarbageCollect() {
    { // release every unused chunk
        const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

        // run GC on every chunk to trim unused committed memory
        Pages.DeleteIf([this](uintptr_t, uintptr_t value) NOEXCEPT -> bool{
            auto* const page = reinterpret_cast<page_type*>(value);
            page->NextPage = nullptr;

            if (page->Empty()) {
                ReleaseUnusedPage_AssumeLocked_(page);
                return true;
            }

            return false;
        });

        FreePage = GDummyPage;
    }
    { // trigger page cache cleanup
        FBitmapBasicPage::ReleaseCacheMemory();
    }
}
//----------------------------------------------------------------------------
template <typename _Traits>
template <typename _Each>
size_t TBitmapHeap<_Traits>::EachPage(_Each&& each) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    size_t numBlocks = 0;
    Pages.Foreach([&each, &numBlocks](uintptr_t, uintptr_t value) NOEXCEPT {
        numBlocks++;
        auto* const page = reinterpret_cast<page_type*>(value);
        each(   page->vAddressSpace,
                page->Pages.load(std::memory_order_relaxed),
                page->Sizes.load(std::memory_order_relaxed) );
    });

    return numBlocks;
}
//----------------------------------------------------------------------------
template <typename _Traits>
size_t TBitmapHeap<_Traits>::DebugInfo(FBitmapHeapInfo* pinfo) const NOEXCEPT {
    const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

    if (pinfo) {
        pinfo->BitmapSize = BitmapSize;
        pinfo->Granularity = Granularity;
        pinfo->PagesPerBlock = page_type::MaxPages;
        pinfo->Stats = FBitmapMemoryStats{};
    }

    size_t numPages = 0;
    Pages.Foreach([&pinfo, &numPages](uintptr_t, uintptr_t value) {
        auto* const page = reinterpret_cast<page_type*>(value);

        if (pinfo && numPages < pinfo->Pages.size()) {
            FBitmapPageInfo& pageInfo = pinfo->Pages[numPages];
            page->DebugInfo(&pageInfo);
            pinfo->Stats = pinfo->Stats + pageInfo.Stats;
        }

        numPages++;
    });

    return numPages;
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::AliasingPage_AssumeLocked_(const void* ptr) const NOEXCEPT -> page_type* {
    const void* const vAddressSpace = Meta::RoundToPrevPow2(ptr, BitmapSize);

    FStackMRU& hint = FStackMRU::Tls();
    for (const auto& MRU : hint.MRU) {
        if ((MRU.Revision == Revision) & (!!MRU.Page)) {
            if (MRU.Page->vAddressSpace == vAddressSpace)
                return MRU.Page;
        }
    }

    page_type* page;
    if (Pages.Find(reinterpret_cast<void**>(&page), vAddressSpace)) {
        Assert(page);
        Assert_NoAssume(page->Aliases(ptr));
        hint.Push(page, Revision);
        return page;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
template <typename _Traits>
auto TBitmapHeap<_Traits>::FindFreePage_AssumeLocked_(u32 sizeInBytes) NOEXCEPT -> page_type* {
    Assert(sizeInBytes);
    Assert_NoAssume(Meta::IsAlignedPow2(Granularity, sizeInBytes));

    page_type* freePage = FreePage.load(std::memory_order_relaxed);
    for (; freePage != GDummyPage; freePage = static_cast<page_type*>(freePage->NextPage)) {
        if (freePage->LargestBlockAvailable() >= sizeInBytes)
            return freePage;
    }

    freePage = nullptr;
    Pages.Where([sizeInBytes, &freePage](uintptr_t, uintptr_t value) NOEXCEPT -> bool {
        auto* const page = reinterpret_cast<page_type*>(value);
        if (page->LargestBlockAvailable() >= sizeInBytes) {
            freePage = page;
            return true;
        }
        return false;
    });

    Assert_NoAssume(GDummyPage != freePage);
    return freePage;
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::RegisterFreePage_Unlocked_(page_type* page) NOEXCEPT {
    // cmpxch needed to avoid double registration in the queue
    if (FPlatformAtomics::CompareExchangePtr<page_type>((volatile page_type**)&page->NextPage, GDummyPage, nullptr) == nullptr) {
        page_type* freePage = FreePage.load(std::memory_order_relaxed);
        for (;;) {
            page->NextPage = freePage;
            if (FreePage.compare_exchange_weak(freePage, page, std::memory_order_release, std::memory_order_relaxed))
                return;

            FPlatformAtomics::ShortSyncWait();
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Traits>
void TBitmapHeap<_Traits>::ReleaseUnusedPage_AssumeLocked_(page_type* page) NOEXCEPT {
    Assert(page);
    Assert(page->vAddressSpace);
    AssertRelease(page->Empty());
    Assert_NoAssume(page->Sizes.load(std::memory_order_relaxed) == 0);
    Assert_NoAssume(page->Pages.load(std::memory_order_relaxed) == mask_type(-1));

    ++Revision; // invalidate each TLS cache

    traits_type::PageDecommit(page->vAddressSpace, BitmapSize);
    traits_type::PageRelease(page->vAddressSpace, BitmapSize);

    ONLY_IF_ASSERT(page->vAddressSpace = nullptr);

    FBitmapBasicPage::Deallocate(page);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBitmapMetadata {
    static void* Allocate(size_t sz) NOEXCEPT;
    static void Free(void* ptr, size_t sz) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <size_t _ReservedSize, typename _Traits>
TBitmapFixedSizeHeap<_ReservedSize, _Traits>::TBitmapFixedSizeHeap() NOEXCEPT
:   NumExhaustedPages(0) {
    STATIC_ASSERT(Meta::IsAlignedPow2(BitmapSize, _ReservedSize));

    const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

    vAddressSpace = reinterpret_cast<u8*>(traits_type::PageReserve(BitmapSize, _ReservedSize));
    AssertRelease(vAddressSpace);

    CommittedPages.SetupMemoryRequirements(NumReservedPages);
    ExhaustedPages.SetupMemoryRequirements(NumReservedPages);

    const size_t szPageTable = Meta::RoundToNextPow2(NumReservedPages * sizeof(page_type*), CACHELINE_SIZE);
    const size_t szCommitted = Meta::RoundToNextPow2(CommittedPages.AllocationSize(), CACHELINE_SIZE);
    const size_t szExhausted = Meta::RoundToNextPow2(ExhaustedPages.AllocationSize(), CACHELINE_SIZE);

    u8* Metadata = reinterpret_cast<u8*>(FBitmapMetadata::Allocate(szPageTable + szCommitted + szExhausted));

    PageTable = reinterpret_cast<page_type*>(Metadata);
    Metadata += szPageTable;

    using word_t = typename decltype(CommittedPages)::word_t;

    CommittedPages.Initialize(reinterpret_cast<word_t*>(Metadata), szCommitted / sizeof(word_t), false);
    Metadata += szCommitted;

    ExhaustedPages.Initialize(reinterpret_cast<word_t*>(Metadata), szExhausted / sizeof(word_t), true );
}
//----------------------------------------------------------------------------
template <size_t _ReservedSize, typename _Traits>
TBitmapFixedSizeHeap<_ReservedSize, _Traits>::~TBitmapFixedSizeHeap() NOEXCEPT {
    Assert(vAddressSpace);

    ForceGarbageCollect();

    const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

    AssertRelease(CommittedPages.Empty_ForAssert()); // are we leaking ?
    AssertRelease(ExhaustedPages.Full());

    const size_t szPageTable = Meta::RoundToNextPow2(NumReservedPages * sizeof(page_type*), CACHELINE_SIZE);
    const size_t szCommitted = Meta::RoundToNextPow2(CommittedPages.AllocationSize(), CACHELINE_SIZE);
    const size_t szExhausted = Meta::RoundToNextPow2(ExhaustedPages.AllocationSize(), CACHELINE_SIZE);

    void* const Metadata = PageTable;
    FBitmapMetadata::Free(Metadata, szPageTable + szCommitted + szExhausted);

    traits_type::PageRelease(vAddressSpace, _ReservedSize);

    ONLY_IF_ASSERT(vAddressSpace = nullptr);
}
//----------------------------------------------------------------------------
template <size_t _ReservedSize, typename _Traits>
auto TBitmapFixedSizeHeap<_ReservedSize, _Traits>::Aliases(const void* ptr) const NOEXCEPT -> page_type* {
    Assert(vAddressSpace);

    if (FPlatformMemory::Memaliases(vAddressSpace, _ReservedSize, ptr)) {
        const u32 pageIndex = checked_cast<u32>((uintptr_t(ptr) - uintptr_t(vAddressSpace)) / BitmapSize);
        Assert_NoAssume(pageIndex < NumReservedPages);
        page_type* const page = (PageTable + pageIndex);
        Assert_NoAssume(page->Aliases(ptr));
        return page;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
template <size_t _ReservedSize, typename _Traits>
size_t TBitmapFixedSizeHeap<_ReservedSize, _Traits>::AllocationSize(const void* ptr) const NOEXCEPT {
    page_type* const page = Aliases(ptr);
    return page->RegionSize(ptr);
}
//----------------------------------------------------------------------------
template <size_t _ReservedSize, typename _Traits>
void* TBitmapFixedSizeHeap<_ReservedSize, _Traits>::Allocate(size_t sizeInBytes) {
    Assert(sizeInBytes);
    Assert(Meta::IsAlignedPow2(page_type::PageSize, sizeInBytes));

    auto& hint = FStackMRU::Tls();

    const bool ate = page_type::Segregate(checked_cast<u32>(sizeInBytes));
    {
        const FReadWriteLock::FScopeLockRead scopeRead(RWLock);

        for (;;) {
            u32 allocIndex = UINT32_MAX;
            for (u32 pageIndex : hint.MRU) {
                if (pageIndex < NumReservedPages) {
                    page_type* const page = (PageTable + pageIndex);
                    if ((page->LargestBlockAvailable() >= sizeInBytes) &
                        ((allocIndex == UINT32_MAX) | (ate ? allocIndex < pageIndex : allocIndex > pageIndex)) )
                        allocIndex = pageIndex;
                }
            }

            if (Unlikely(UINT32_MAX == allocIndex)) {
                u32 exhaustedIndex = ExhaustedPages.NextAllocateBit(); // look for a page with remaining space
                while (Likely(exhaustedIndex < NumReservedPages)) {
                    page_type* const page = (PageTable + exhaustedIndex);
                    if (page->LargestBlockAvailable() >= sizeInBytes) {
                        allocIndex = exhaustedIndex;
                        goto TRY_ALLOCATE;
                    }
                    exhaustedIndex = ExhaustedPages.NextAllocateBit(exhaustedIndex + 1);
                }

                break; // need a new page ?
            }

        TRY_ALLOCATE:
            Assert(allocIndex < NumReservedPages);
            bool exhausted = false;
            void* const result = PageTable[allocIndex].Allocate(checked_cast<u32>(sizeInBytes), exhausted);
            if (Likely(result)) {
                if (Likely(not exhausted))
                    hint.Push(allocIndex);
                else
                    ExhaustedPages.AllocateBit(allocIndex);

                ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
                return result;
            }
        }
    }

    return Meta::unlikely([this](FStackMRU& hint, size_t sizeInBytes) -> void* {
        const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

        u32 allocIndex = UINT32_MAX;
        u32 exhaustedIndex = ExhaustedPages.NextAllocateBit(); // look for a page with remaining space
        while (Likely(exhaustedIndex < NumReservedPages)) {
            page_type* const page = (PageTable + exhaustedIndex);
            if (page->LargestBlockAvailable() >= sizeInBytes) {
                allocIndex = exhaustedIndex;
                break;
            }
            exhaustedIndex = ExhaustedPages.NextAllocateBit(exhaustedIndex + 1);
        }

        if (UINT32_MAX == allocIndex) {
            allocIndex = CommittedPages.Allocate();
            if (Unlikely(UINT32_MAX == allocIndex))
                return nullptr; // OOM

            ExhaustedPages.Deallocate(allocIndex);

            Assert(allocIndex < NumReservedPages);
            page_type& page = PageTable[allocIndex];
            page.Reset(vAddressSpace + allocIndex * BitmapSize);
            page_type::PageCommit(page.vAddressSpace, BitmapSize);
        }

        Assert(allocIndex < NumReservedPages);
        Assert_NoAssume(PageTable[allocIndex].LargestBlockAvailable() >= sizeInBytes);

        bool exhausted = false;
        void* const result = PageTable[allocIndex].Allocate(sizeInBytes, exhausted);
        if (Likely(result)) {
            if (Likely(not exhausted))
                hint.Push(allocIndex);
            else
                ExhaustedPages.AllocateBit(allocIndex);

            ONLY_IF_MEMORYDOMAINS(traits_type::OnUserAlloc(result, SnapSize(sizeInBytes)));
            return result;
        }

        return nullptr;

    },  hint, sizeInBytes );
}
//----------------------------------------------------------------------------
template <size_t _ReservedSize, typename _Traits>
void TBitmapFixedSizeHeap<_ReservedSize, _Traits>::Free(void* ptr, page_type* page/* = nullptr */) {
    if (Unlikely(nullptr == page))
        page = Aliases(page);

    ONLY_IF_MEMORYDOMAINS(traits_type::OnUserFree(page->RegionSize(ptr)));

    if (Unlikely(not page->Free_ReturnIfUnused(ptr))) {
        Meta::unlikely([this](page_type* page) {
            const u32 pageIndex = checked_cast<u32>(page - PageTable);
            Assert(pageIndex < NumReservedPages);

            const FReadWriteLock::FScopeLockWrite scopeWrite(RWLock);

            if (page->Empty() && page->vAddressSpace) {

                if (NumExhaustedPages == MaxExhaustedPages) {
                    CommittedPages.Deallocate(pageIndex);
                    ExhaustedPages.AllocateBit(pageIndex);

                    traits_type::PageDecommit(page->vAddressSpace, BitmapSize);
                    page->vAddressSpace = nullptr;
                    page->Pages.store(0, std::memory_order_relaxed);
                    Assert(page->LargestBlockAvailable() == 0); // no more committed -> no more space available
                }
                else {
                    NumExhaustedPages++;
                    ExhaustedPages.Deallocate(pageIndex);
                }

                Assert_NoAssume(NumExhaustedPages <= MaxExhaustedPages);
            }
        },  page );
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
