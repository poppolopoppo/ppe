#include "stdafx.h"

#include "Allocator/MipMapAllocator.h"

#include "Allocator/InitSegAllocator.h"
#include "Container/IntrusiveList.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct ALIGN(64) FMipmapPool {
    STATIC_CONST_INTEGRAL(u32, MaxPages, ALLOCATION_GRANULARITY / sizeof(FMipmapPage));

    POD_STORAGE(FMipmapPage) Pages[MaxPages];

    static CONSTEXPR const size_t HeaderSize{
        sizeof(u32) * 2 + // atomic counters
        sizeof(FMipmapPage*) + // linked list
        sizeof(FMipmapPage*) * 2 // prev/next
#if USE_PPE_ASSERT
        + sizeof(size_t) // canary
#endif
    };
    Meta::TAlignedStorage<ALLOCATION_GRANULARITY - sizeof(Pages) - HeaderSize, alignof(void*)> Padding_;

    std::atomic<u32> NumBusy{ 0 };
    std::atomic<u32> NumRegistered{ 0 };

    std::atomic<FMipmapPage*> FreePages{ 0 };

    TIntrusiveListNode<FMipmapPool> Node{ nullptr, nullptr };

#if USE_PPE_ASSERT
    const size_t Canary{ CODE3264(0xdeadbeefLU, 0xdeadbeefabadcafeLLU) };
    bool CheckCanary() const NOEXCEPT { return (CODE3264(0xdeadbeefLU, 0xdeadbeefabadcafeLLU) == Canary); }
#endif

    bool AliasedToPool(const FMipmapPage* page) const NOEXCEPT {
        return (uintptr_t(page) >= uintptr_t(Pages) &&
                uintptr_t(page) < uintptr_t(&Pages[MaxPages]));
    }

    static FMipmapPool* PoolFromPage(const FMipmapPage* page) NOEXCEPT {
        STATIC_ASSERT(Meta::IsPow2(sizeof(FMipmapPool)));
        STATIC_ASSERT(sizeof(FMipmapPool) == ALLOCATION_GRANULARITY);
        STATIC_ASSERT(Meta::TCheckSameSize<FMipmapPool, Meta::TArray<u8, ALLOCATION_GRANULARITY>>::value);
        return reinterpret_cast<FMipmapPool*>(uintptr_t(page) & ~(sizeof(FMipmapPool) - 1));
    }

    FMipmapPage* GrabFreePage() NOEXCEPT {
        FMipmapPage* freePage = FreePages.load(std::memory_order_relaxed);
        for (; freePage;) {
            if (FreePages.compare_exchange_weak(freePage, freePage->NextPage,
                    std::memory_order_release, std::memory_order_relaxed)) {
                Verify(++NumBusy <= MaxPages);
                ONLY_IF_ASSERT(freePage->NextPage = nullptr);
                Assert_NoAssume(PoolFromPage(freePage) == this);
                return freePage;
            }
        }

        u32 registered = NumRegistered.load(std::memory_order_relaxed);
        for (; registered < MaxPages; ) {
            if (NumRegistered.compare_exchange_weak(registered, registered+1,
                    std::memory_order_release, std::memory_order_relaxed)) {
                Verify(++NumBusy <= MaxPages);
                return INPLACE_NEW(&Pages[registered], FMipmapPage);
            }
        }

        return nullptr;
    }

    bool ReleaseBusyPage(FMipmapPage* busyPage) NOEXCEPT { // returns true if pool is completely unused
        Assert(busyPage);
        Assert_NoAssume(AliasedToPool(busyPage));
        Assert_NoAssume(NumBusy > 0);
        Assert_NoAssume(NumRegistered > 0);
        Assert_NoAssume(busyPage->NumBlocks == 0);
        Assert_NoAssume(busyPage->NumUnused == 0);
        Assert_NoAssume(busyPage->LiveSet == 0);
        Assert_NoAssume(busyPage->vAddressSpace == nullptr);

        busyPage->NextPage = FreePages.load(std::memory_order_relaxed);
        for (;;) {
            if (FreePages.compare_exchange_weak(busyPage->NextPage, busyPage,
                    std::memory_order_release, std::memory_order_relaxed)) {
                const u32 busy = --NumBusy;
                Verify(busy < MaxPages);
                return (0 == busy);
            }
        }
    }
};
//----------------------------------------------------------------------------
struct FMipmapGlobalCache {
    FAtomicSpinLock Barrier;
    TIntrusiveList<FMipmapPool, &FMipmapPool::Node> Pools;

    static FMipmapGlobalCache& Get() NOEXCEPT {
        ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FMipmapGlobalCache>, GlobalCache);
        return GlobalCache;
    }

    FMipmapGlobalCache() = default;

    ~FMipmapGlobalCache() {
        const FAtomicSpinLock::FScope scopeLock(Barrier);
        while (FMipmapPool* pool = Pools.PopHead()) {
            AssertRelease(0 == pool->NumBusy);
            STATIC_ASSERT(sizeof(FMipmapPool) == ALLOCATION_GRANULARITY);
            FVirtualMemory::InternalFree(pool, sizeof(FMipmapPool)
            #if USE_PPE_MEMORYDOMAINS
                , MEMORYDOMAIN_TRACKING_DATA(Bookkeeping)
            #endif
                );
        }
    }

    FMipmapPage* GrabFreePage() NOEXCEPT {
#if USE_PPE_MEMORYDOMAINS
        ON_SCOPE_EXIT([]() { // after AllocateSystem() call
            MEMORYDOMAIN_TRACKING_DATA(Bookkeeping).AllocateUser(sizeof(FMipmapPage));
        });
#endif
        for (FMipmapPool* pool = Pools.Head(); pool; pool = pool->Node.Next) {
            Assert_NoAssume(pool->CheckCanary());

            if (FMipmapPage* page = pool->GrabFreePage()) {
                if (Unlikely(Pools.Head() != pool && pool->FreePages.load(std::memory_order_relaxed))) {
                    const FAtomicSpinLock::FScope scopeLock(Barrier);
                    if (pool->NumBusy < Pools.Head()->NumBusy)
                        Pools.PokeHead(pool);
                }
                return page;
            }
        }
        return GrabPageFromNewPool();
    }

    NO_INLINE FMipmapPage* GrabPageFromNewPool() {
        const FAtomicSpinLock::FScope scopeLock(Barrier);

        FMipmapPool* const pool = Pools.Head();
        if ((pool == nullptr) || (pool->NumBusy == FMipmapPool::MaxPages) ) {
            void* const alloc = FVirtualMemory::InternalAlloc(sizeof(FMipmapPool)
#if USE_PPE_MEMORYDOMAINS
                , MEMORYDOMAIN_TRACKING_DATA(Bookkeeping)
#endif
                );
            Assert(alloc);
            Pools.PushHead(INPLACE_NEW(alloc, FMipmapPool));
        }

        FMipmapPage* const page = Pools.Head()->GrabFreePage();
        Assert(page);

        return page;
    }

    void ReleaseBusyPage(FMipmapPage* busyPage) {
        Assert(busyPage);
        Assert(Pools.Head());
        Assert_NoAssume(busyPage->NumUnused == 0);

#if USE_PPE_MEMORYDOMAINS
        MEMORYDOMAIN_TRACKING_DATA(Bookkeeping).DeallocateUser(sizeof(FMipmapPage));
#endif

        FMipmapPool* const pool = FMipmapPool::PoolFromPage(busyPage);
        Assert_NoAssume(pool->CheckCanary());

        if (pool->ReleaseBusyPage(busyPage))
            ReleaseUnusedPool(pool);
    }

    NO_INLINE void ReleaseUnusedPool(FMipmapPool* pool) {
        Assert(pool);

        const FAtomicSpinLock::FScope scopeLock(Barrier);

        if (Pools.Head() == pool)
            return; // always keep one empty pool

        Pools.Erase(pool);

        if (Likely(0 == pool->NumBusy)) {
            FVirtualMemory::InternalFree(pool, sizeof(FMipmapPool)
#if USE_PPE_MEMORYDOMAINS
                , MEMORYDOMAIN_TRACKING_DATA(Bookkeeping)
#endif
                );
        }
        else { // the block was reused concurrently
            Pools.PushHead(pool); // push back the pool
        }
    }
};
//----------------------------------------------------------------------------
FMipmapPage* FMipmapPage::GrabFreePage() {
    return FMipmapGlobalCache::Get().GrabFreePage();
}
//----------------------------------------------------------------------------
void FMipmapPage::ReleaseBusyPage(FMipmapPage* page) {
    FMipmapGlobalCache::Get().ReleaseBusyPage(page);
}
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
bool FMipmapPage::CheckSanity() const {
    // /!\ time for some sanity checks
    u32 totalAvail = 0;
    forrange(blk, 0, NumBlocks)
        totalAvail += AvailableMips(blk);

    const u32 totalCmmit = NumBlocks * NumMips;
    const float percentAvail = (100.0f * totalAvail) / totalCmmit;
    const bool wtf = (0 != totalCmmit && percentAvail > 60.f && NumBlocks > 10);

    if (Likely(not wtf))
        return true;

    // /!\ time for some sanity checks
    // convert the mip mask to a 32 bits mask (ie lowest mip)
    wchar_t tok[] = L"#";
    wchar_t chr[] = L"AB";
    const u32 n = NumBlocks;
    forrange(mi, 0, n) {
        const FBlock& block = Blocks[mi];
        FPlatformDebug::OutputDebug(L"\n -0- ");
        {
            const u32 mips = ~u32(block.Mips.load() >> u64(31));
            const u32 size = u32(block.Size.load() >> u64(31));
            forrange(i, 0, 32) {
                tok[0] = chr[i & 1];
                FPlatformDebug::OutputDebug(!(size & (1 << i))
                    ? (mips & (1 << i)) ? tok : L"."
                    : L"!" );
            }
        }
        FPlatformDebug::OutputDebug(L"\n -1- ");
        {
            const u32 mips = ~u32(block.Mips.load() >> u64(15));
            const u32 size = u32(block.Size.load() >> u64(15));
            forrange(i, 0, 16) {
                tok[0] = chr[i & 1];
                forrange(j, 0, 2)
                    FPlatformDebug::OutputDebug(!(size & (1 << i))
                        ? (mips & (1 << i)) ? tok : L"."
                        : L"!" );
            }
        }
        FPlatformDebug::OutputDebug(L"\n -2- ");
        {
            const u32 mips = ~u32(block.Mips.load() >> u64(7));
            const u32 size = u32(block.Size.load() >> u64(7));
            forrange(i, 0, 8) {
                tok[0] = chr[i & 1];
                forrange(j, 0, 4)
                    FPlatformDebug::OutputDebug(!(size & (1 << i))
                        ? (mips & (1 << i)) ? tok : L"."
                        : L"!" );
            }
        }
        FPlatformDebug::OutputDebug(L"\n -3- ");
        {
            const u32 mips = ~u32(block.Mips.load() >> u64(3));
            const u32 size = u32(block.Size.load() >> u64(3));
            forrange(i, 0, 4) {
                tok[0] = chr[i & 1];
                forrange(j, 0, 8)
                    FPlatformDebug::OutputDebug(!(size & (1 << i))
                        ? (mips & (1 << i)) ? tok : L"."
                        : L"!" );
            }
        }
        FPlatformDebug::OutputDebug(L"\n -4- ");
        {
            const u32 mips = ~u32(block.Mips.load() >> u64(1));
            const u32 size = u32(block.Size.load() >> u64(1));
            forrange(i, 0, 2) {
                tok[0] = chr[i & 1];
                forrange(j, 0, 16)
                    FPlatformDebug::OutputDebug(!(size & (1 << i))
                        ? (mips & (1 << i)) ? tok : L"."
                        : L"!" );
            }
        }
        FPlatformDebug::OutputDebug(L"\n -5- ");
        {
            const u32 mips = ~u32(block.Mips.load() >> u64(0));
            const u32 size = u32(block.Size.load() >> u64(0));
            forrange(i, 0, 1) {
                tok[0] = chr[i & 1];
                forrange(j, 0, 32)
                    FPlatformDebug::OutputDebug(!(size & (1 << i))
                        ? (mips & (1 << i)) ? tok : L"."
                        : L"!" );
            }
        }
        FPlatformDebug::OutputDebug(L"\n");
    }

    FPlatformDebug::OutputDebug("\nstop\n");
    return false;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
