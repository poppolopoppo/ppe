#include "stdafx.h"

#include "Allocator/Mallocator.h"
#include "Allocator/StlAllocator.h"

#include "Container/Pair.h"
#include "Container/RingBuffer.h"
#include "Container/Stack.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/StringView.h"
#include "Maths/MathHelpers.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/Threefy.h"
#include "Memory/MemoryPool.h"
#include "Memory/MemoryView.h"
#include "Memory/UniqueView.h"
#include "Time/TimedScope.h"
#include "Thread/Task/TaskHelpers.h"
#include "Thread/ThreadPool.h"

#include <algorithm>
#include <random>


#include "Container/CompressedRadixTrie.h"
#include "Maths/RandomGenerator.h"
#include "Maths/VarianceEstimator.h"
#include "Memory/CachedMemoryPool.h"
#include "Memory/MemoryPool.h"
#include "Modular/ModularDomain.h"
#include "Thread/CriticalSection.h"

#define USE_TESTALLOCATOR_MEMSET 0

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Allocators)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_DEBUG && !USE_PPE_FASTDEBUG
static constexpr size_t GTotalAllocationSize_ = CODE3264( 32u,  64u) * 1024u * size_t(1024);
#else
static constexpr size_t GTotalAllocationSize_ = CODE3264( 64u, 128u) * 1024u * size_t(1024);
#endif
#if USE_PPE_ASSERT
static constexpr size_t GLoopCount_ = 10;
#else
static constexpr size_t GLoopCount_ = 100;
#endif
#if USE_PPE_ASSERT
static constexpr size_t GSlidingWindow_ = 50;
#else
static constexpr size_t GSlidingWindow_ = 150;
#endif
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_ST_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    UNUSED(category); UNUSED(name);
    BENCHMARK_SCOPE(category, name);

    using allocator_traits = TAllocatorTraits<_Alloc>;

    forrange(loop, 0, GLoopCount_) {
        for (size_t sz : blockSizes) {
            const FAllocatorBlock blk = allocator_traits::Allocate(allocator, sz);
#if USE_TESTALLOCATOR_MEMSET
            FPlatformMemory::Memset(blk.Data, 0xFA, blk.SizeInBytes);
#endif
            allocator_traits::Deallocate(allocator, blk);
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_MT_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    UNUSED(category); UNUSED(name);
    BENCHMARK_SCOPE(category, name);

    using allocator_traits = TAllocatorTraits<_Alloc>;

    forrange(loop, 0, GLoopCount_) {
        ParallelForEachValue(blockSizes.begin(), blockSizes.end(), [&allocator](size_t sz) {
            _Alloc alloc(allocator);
            const FAllocatorBlock blk = allocator_traits::Allocate(alloc, sz);
#if USE_TESTALLOCATOR_MEMSET
            FPlatformMemory::Memset(blk.Data, 0xFB, blk.SizeInBytes);
#endif
            allocator_traits::Deallocate(alloc, blk);
        });
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Sliding_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes, size_t window) {
    UNUSED(category); UNUSED(name);
    BENCHMARK_SCOPE(category, name);

    const size_t numDeallocs = Max(size_t(1), window / 10);

    using allocator_traits = TAllocatorTraits<_Alloc>;
    STACKLOCAL_RINGBUFFER(FAllocatorBlock, blockAddrs, window);

    forrange(loop, 0, GLoopCount_) {
        FAllocatorBlock prev;
        for(size_t sz : blockSizes) {
            FAllocatorBlock blk = allocator_traits::Allocate(allocator, sz);
#if USE_TESTALLOCATOR_MEMSET
            FPlatformMemory::Memset(blk.Data, 0xFC, blk.SizeInBytes);
#endif

            if (blockAddrs.size() == window) {
                forrange(i, 0, numDeallocs) {
                    if (blockAddrs.pop_front(&prev))
                        allocator_traits::Deallocate(allocator, prev);
                    else
                        AssertNotReached();
                }
            }

            blockAddrs.push_back(blk);
        }

        while (blockAddrs.pop_back(&prev))
            allocator_traits::Deallocate(allocator, prev);
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Trashing_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    UNUSED(category); UNUSED(name);
    BENCHMARK_SCOPE(category, name);

    using allocator_traits = TAllocatorTraits<_Alloc>;
    STACKLOCAL_POD_ARRAY(void*, blockAddrs, blockSizes.size());

    forrange(loop, 0, GLoopCount_) {
        forrange(i, 0, blockSizes.size()) {
            const size_t sz = blockSizes[i];
            FAllocatorBlock blk = allocator_traits::Allocate(allocator, sz);
            blockAddrs[i] = blk.Data;
        }

        forrange(i, 0, blockSizes.size())
            allocator_traits::Deallocate(allocator, FAllocatorBlock{ blockAddrs[i], blockSizes[i] });
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Dangling_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    FTaskManager& threadPool = FHighPriorityThreadPool::Get();

    const size_t numWorkers = threadPool.WorkerCount();
    const size_t allocsPerWorker = ((blockSizes.size() + numWorkers - 1) / numWorkers);
    Assert(numWorkers * allocsPerWorker >= blockSizes.size());

    using allocator_traits = TAllocatorTraits<_Alloc>;
    using blocks_type = TUniqueArray<FAllocatorBlock>;

    TVector<FAllocatorBlock> blocks;
    blocks.reserve(blockSizes.size());
    for (size_t sz : blockSizes)
        blocks.emplace_back_AssumeNoGrow(nullptr, sz);

    TVector<FTaskFunc> allocateTasks;
    allocateTasks.reserve_AssumeEmpty(numWorkers);
    TVector<FTaskFunc> deallocateTasks;
    deallocateTasks.reserve_AssumeEmpty(numWorkers);

    struct payload_t {
        _Alloc& Allocator;
        TMemoryView<FAllocatorBlock> Blocks;
    }   payload{ allocator, blocks.MakeView() };

    forrange(i, 0, numWorkers) {
        u32 bbegin = u32(i * allocsPerWorker);
        u32 bend = u32(Min(bbegin + allocsPerWorker, blocks.size()));

        if (bbegin >= bend)
            break;

        allocateTasks.emplace_back_AssumeNoGrow([bbegin, bend, &payload](ITaskContext&) {
            for (FAllocatorBlock& b : payload.Blocks.SubRange(bbegin, bend - bbegin))
                b = allocator_traits::Allocate(payload.Allocator, b.SizeInBytes);
        });
        deallocateTasks.emplace_back_AssumeNoGrow([bbegin, bend, &payload](ITaskContext&) {
            for (FAllocatorBlock& b : payload.Blocks.SubRange(bbegin, bend - bbegin))
                allocator_traits::Deallocate(payload.Allocator, b);
        });
    }

    std::random_device rdevice;
    std::mt19937 rand(rdevice());

    UNUSED(category); UNUSED(name);
    BENCHMARK_SCOPE(category, name);

    // tries to free blocks from another thread from which they were allocated initially
    // this is the worst case for many allocators and can be *VERY* slow
    forrange(loop, 0, GLoopCount_) {
        // allocate from each worker
        threadPool.RunAndWaitFor(allocateTasks.MakeConstView());
        // randomize all blocks, will release from any thread
        std::shuffle(blocks.begin(), blocks.end(), rand);
        // deallocate from (hopefully) another allocator
        threadPool.RunAndWaitFor(deallocateTasks.MakeConstView());
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Realloc_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    UNUSED(category); UNUSED(name);
    BENCHMARK_SCOPE(category, name);

    using allocator_traits = TAllocatorTraits<_Alloc>;

    forrange(loop, 0, GLoopCount_) {
        const size_t numWorkers = checked_cast<size_t>(std::thread::hardware_concurrency()) / 2;
        ParallelFor(0, numWorkers, [&allocator, blockSizes](size_t) {
            for (u32 b = 0; b + 3 < blockSizes.size(); b += 4) {
                FAllocatorBlock blk{};
                allocator_traits::Reallocate(allocator, blk, blockSizes[b + 0]);
                allocator_traits::Reallocate(allocator, blk, blockSizes[b + 1]);
                allocator_traits::Reallocate(allocator, blk, blockSizes[b + 2]);
                allocator_traits::Reallocate(allocator, blk, blockSizes[b + 3]);
                allocator_traits::Deallocate(allocator, blk);
            }
        });
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_(
    const FWStringView& name, _Alloc&& allocator,
    const TMemoryView<const size_t>& smallBlocks,
    const TMemoryView<const size_t>& largeBlocks,
    const TMemoryView<const size_t>& mixedBlocks ) {
    STATIC_ASSERT(is_allocator_v<_Alloc>);
    LOG(Test_Allocators, Emphasis, L"benchmarking <{0}>", name);

    BENCHMARK_SCOPE(name, L"Global");
    {
        BENCHMARK_SCOPE(name, L"Single-Thread");

        Test_Allocator_ST_(name, L"small blocks", std::forward<_Alloc>(allocator), smallBlocks);
        Test_Allocator_ST_(name, L"large blocks", std::forward<_Alloc>(allocator), largeBlocks);
        Test_Allocator_ST_(name, L"mixed blocks", std::forward<_Alloc>(allocator), mixedBlocks);
    }
    {
        BENCHMARK_SCOPE(name, L"Multi-Thread");

        Test_Allocator_MT_(name, L"small blocks", std::forward<_Alloc>(allocator), smallBlocks);
        Test_Allocator_MT_(name, L"large blocks", std::forward<_Alloc>(allocator), largeBlocks);
        Test_Allocator_MT_(name, L"mixed blocks", std::forward<_Alloc>(allocator), mixedBlocks);
    }
    {
        BENCHMARK_SCOPE(name, L"Sliding");

        Test_Allocator_Sliding_(name, L"small blocks", std::forward<_Alloc>(allocator), smallBlocks, GSlidingWindow_);
        Test_Allocator_Sliding_(name, L"large blocks", std::forward<_Alloc>(allocator), largeBlocks, GSlidingWindow_);
        Test_Allocator_Sliding_(name, L"mixed blocks", std::forward<_Alloc>(allocator), mixedBlocks, GSlidingWindow_);
    }
    {
        BENCHMARK_SCOPE(name, L"Trashing");

        Test_Allocator_Trashing_(name, L"small blocks", std::forward<_Alloc>(allocator), smallBlocks);
        Test_Allocator_Trashing_(name, L"large blocks", std::forward<_Alloc>(allocator), largeBlocks);
        Test_Allocator_Trashing_(name, L"mixed blocks", std::forward<_Alloc>(allocator), mixedBlocks);
    }
    {
        BENCHMARK_SCOPE(name, L"Dangling");

        Test_Allocator_Dangling_(name, L"small blocks", std::forward<_Alloc>(allocator), smallBlocks);
        Test_Allocator_Dangling_(name, L"large blocks", std::forward<_Alloc>(allocator), largeBlocks);
        Test_Allocator_Dangling_(name, L"mixed blocks", std::forward<_Alloc>(allocator), mixedBlocks);
    }
    {
        BENCHMARK_SCOPE(name, L"Realloc");

        Test_Allocator_Realloc_(name, L"small blocks", std::forward<_Alloc>(allocator), smallBlocks);
        Test_Allocator_Realloc_(name, L"large blocks", std::forward<_Alloc>(allocator), largeBlocks);
        Test_Allocator_Realloc_(name, L"mixed blocks", std::forward<_Alloc>(allocator), mixedBlocks);
    }
}
//----------------------------------------------------------------------------
static void Test_CompressedRadixTrie_() {
    LOG(Test_Allocators, Emphasis, L"testing FCompressedRadixTrie");

    ONLY_IF_MEMORYDOMAINS(FMemoryTracking dummyTracking("dummy", &MEMORYDOMAIN_TRACKING_DATA(ReservedMemory)));
    FReadWriteCompressedRadixTrie radixTrie{
#if USE_PPE_MEMORYDOMAINS
        dummyTracking
#endif
    };

    FRandomGenerator rng;
    VECTOR(Benchmark, TPair<uintptr_t COMMA uintptr_t>) blocks;

    forrange(loop, 0, 30) {
        {
            const size_t numBlocks = 2000;
            blocks.reserve(numBlocks);
            forrange(i, 0, numBlocks)
                blocks.emplace_back(uintptr_t(i) << 10, rng.NextU32(1, 8192) << 1);

            rng.Shuffle(blocks.MakeView());
        }

        for (const auto& it : blocks)
            radixTrie.Insert(it.first, it.second);

        auto blocksToDelete = blocks.MakeView().CutBefore(blocks.size() / 3);
        auto blocksToKeep = blocks.MakeView().CutStartingAt(blocks.size() / 3);

        rng.Shuffle(blocksToKeep);
        rng.Shuffle(blocksToDelete);

        radixTrie.Insert(1 << 8, 0xABC0);
        radixTrie.Insert(2 << 8, 0xBCE0);
        radixTrie.Insert(3 << 8, 0xECF0);

        u32 total = 0;
        radixTrie.Foreach([&total](uintptr_t, uintptr_t) {
            total++;
            });
        AssertRelease(total == blocks.size() + 3);

        AssertRelease(radixTrie.Lookup(1 << 8) == 0xABC0);
        AssertRelease(radixTrie.Lookup(2 << 8) == 0xBCE0);
        AssertRelease(radixTrie.Lookup(3 << 8) == 0xECF0);

        uintptr_t value;
        AssertRelease(radixTrie.Find(&value, 2 << 8));
        AssertRelease(0xBCE0 == value);
        AssertRelease(radixTrie.Find(&value, 3 << 8));
        AssertRelease(0xECF0 == value);

        radixTrie.Erase(1 << 8);

        AssertRelease(not radixTrie.Find(&value, 1 << 8));

        radixTrie.Erase(2 << 8);
        radixTrie.Erase(3 << 8);

        u32 hits = 0;
        total = 0;
        radixTrie.Foreach([=, &hits, &total](uintptr_t key, uintptr_t) {
            total++;
            hits += (std::find_if(blocksToDelete.begin(), blocksToDelete.end(), [=](auto x) {
                return x.first == key;
                }) != blocksToDelete.end()) ? 1 : 0;
            });
        AssertRelease(blocks.size() == total);
        AssertRelease(blocksToDelete.size() == hits);

#if 1
        radixTrie.DeleteIf([=](uintptr_t key, uintptr_t) {
            return std::find_if(blocksToDelete.begin(), blocksToDelete.end(), [=](auto x) {
                return x.first == key;
                }) != blocksToDelete.end();
            });
#else
        for (const auto& it : blocksToDelete)
            AssertRelease(radixTrie.Erase(it.first) == it.second);
#endif

        total = 0;
        radixTrie.Foreach([&total](uintptr_t, uintptr_t) {
            total++;
            });
        AssertRelease(total == blocksToKeep.size());

        ParallelForEachRef(blocksToKeep.begin(), blocksToKeep.end(),
            [&radixTrie](const TPair<uintptr_t COMMA uintptr_t>& it) {
            VerifyRelease(radixTrie.Erase(it.first) == it.second);
        },  ETaskPriority::Normal, &FHighPriorityThreadPool::Get() );

        Assert(radixTrie.empty());
        blocks.clear();
    }
}
//----------------------------------------------------------------------------
struct FDummyForPool_ {
    u8 Data0{ 0 };
    u16 Data1{ 0 };

    mutable std::atomic<u32> RefCount_{ 0 };

    FDummyForPool_() = default;
    FDummyForPool_(const FDummyForPool_& rhs) : Data0(rhs.Data0), Data1(rhs.Data1) {}
    FDummyForPool_(FRandomGenerator& rng) {
        rng.Randomize(Data0);
        rng.Randomize(Data1);
    }

    ~FDummyForPool_() {
        AssertRelease(0 == RefCount_);
    }

    bool AddRef() const { return RefCount_.fetch_add(1, std::memory_order_relaxed) == 0; }
    NODISCARD bool RemoveRef() const { return RefCount_.fetch_sub(1, std::memory_order_relaxed) == 1; }

    bool operator ==(const FDummyForPool_& rhs) const {
        return (Data0 == rhs.Data0 && Data1 == rhs.Data1);
    }
    bool operator !=(const FDummyForPool_& rhs) const {
        return (not operator ==(rhs));
    }

    friend hash_t hash_value(const FDummyForPool_& dummy) {
        return hash_tuple(dummy.Data0, dummy.Data1);
    }
};
//----------------------------------------------------------------------------
static void Test_MemoryPool_() {
    using pool_type = TTypedMemoryPool<FDummyForPool_, 8, 512, true, ALLOCATOR(Container)>;
    using index_type = pool_type::index_type;

    STATIC_CONST_INTEGRAL(size_t, ToDeallocate, (pool_type::MaxSize * 2) / 3);
    FRandomGenerator rng;
    TStaticArray<index_type, pool_type::MaxSize> allocs;

    pool_type pool;
    forrange(loop, 0, 10) {
        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, pool_type::MaxSize,
            [&allocs, &pool](size_t i) {
                allocs[i] = pool.Allocate();
            });

        Assert_NoAssume(pool.CheckInvariants());

        rng.Shuffle(allocs.MakeView());

        ParallelFor(0, ToDeallocate,
            [&allocs, &pool](size_t i) {
                pool.Deallocate(allocs[i]);
            });

        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, ToDeallocate,
            [&allocs, &pool](size_t i) {
                allocs[i] = pool.Allocate();
            });

        Assert_NoAssume(pool.CheckInvariants());

        rng.Shuffle(allocs.MakeView());

        ParallelFor(0, pool_type::MaxSize,
            [&allocs, &pool](size_t i) {
                pool.Deallocate(allocs[i]);
            });

        Assert_NoAssume(pool.CheckInvariants());

        pool.Clear_AssertCompletelyEmpty();
    }
}
//----------------------------------------------------------------------------
static void Test_CachedMemoryPool_() {
    using pool_type = TCachedMemoryPool<FDummyForPool_, FDummyForPool_, 8, 512, ALLOCATOR(Container)>;
    using block_type = pool_type::block_type;
    using index_type = pool_type::index_type;

    STATIC_CONST_INTEGRAL(u32, ToAllocate, u32(pool_type::MaxSize));
    STATIC_CONST_INTEGRAL(u32, ToDeallocate, u32((pool_type::MaxSize * 2) / 3));

    pool_type pool;

    FRandomGenerator rng;
    TFixedSizeStack<index_type, ToAllocate> allocs;

    forrange(loop, 0, 10) {
        allocs.clear();

        ParallelFor(0, ToAllocate,
            [&rng, &allocs, &pool](size_t) {
                FDummyForPool_ key{ rng };
                pool.FindOrAdd(key, [&allocs](const FDummyForPool_* pblock, index_type id, bool exist) {
                    VerifyRelease(exist != pblock->AddRef());
                    allocs.Push(id);
                });
            });

        Assert_NoAssume(pool.CheckInvariants());

        rng.Shuffle(allocs.MakeView());

        auto toReallocate = allocs.MakeView().LastNElements(Min(allocs.size(), ToDeallocate));
        ParallelForEachRef(toReallocate.begin(), toReallocate.end(),
            [&pool](index_type& id) {
                pool.RemoveIf(id, [&id](const FDummyForPool_* pblock) {
                    if (pblock->RemoveRef()) {
                        id = UMax;
                        return true;
                    }
                    return false;
                });
            });

        Assert_NoAssume(pool.CheckInvariants());

        ParallelForEachRef(toReallocate.begin(), toReallocate.end(),
            [&pool, &rng](index_type& id) {
                if (id == UMax) {
                    FDummyForPool_ key{ rng };
                    pool.FindOrAdd(key, [&id](const FDummyForPool_* pblock, index_type newId, bool exist) {
                        VerifyRelease(exist != pblock->AddRef());
                        id = newId;
                    });
                }
            });

        Assert_NoAssume(pool.CheckInvariants());

        ParallelForEachRef(allocs.begin(), allocs.end(),
            [&pool](index_type& id) {
                pool.RemoveIf(id, [&id](FDummyForPool_* pblock) {
                    if (pblock->RemoveRef()) {
                        id = UMax;
                        return true;
                    }
                    return false;
                });
            });

        Assert_NoAssume(pool.CheckInvariants());

#if USE_PPE_DEBUG
        for (index_type id : allocs)
            Assert_NoAssume(UMax == id);
#endif

        pool.Clear_AssertCompletelyEmpty();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Allocators() {
    PPE_DEBUG_NAMEDSCOPE("Test_Allocators");

    LOG(Test_Allocators, Emphasis, L"starting allocator tests ...");

    Test_MemoryPool_();
    Test_CachedMemoryPool_();

    typedef u8 value_type;

    constexpr size_t BlockSizeMin = 16;
    constexpr size_t BlockSizeMid = 32768;
    constexpr size_t BlockSizeLarge = 2*1024*1024;

    size_t smallBlocksSizeInBytes = 0;
    size_t largeBlocksSizeInBytes = 0;
    size_t mixedBlocksSizeInBytes = 0;

    using blocksizes_t = VECTORMINSIZE(Container, size_t, 1024);

    blocksizes_t smallBlocks;
    blocksizes_t largeBlocks;
    blocksizes_t mixedBlocks;
    {
        FThreefy_4x32 rnd;
        rnd.RandomSeed();

        auto generator = [&rnd](blocksizes_t* blks, u32 minSize, u32 maxSize, size_t alignment, size_t totalSize) NOEXCEPT -> size_t {
            u32 currentSize = 0;
            for (;;) {
                float4 f = rnd.UniformF(0.f, 1.f);

                const u324 sz4{
                    u32(Meta::RoundToNext(u32(minSize + (maxSize - minSize) * f.x), alignment)),
                    u32(Meta::RoundToNext(u32(minSize + (maxSize - minSize) * f.y), alignment)),
                    u32(Meta::RoundToNext(u32(minSize + (maxSize - minSize) * f.z), alignment)),
                    u32(Meta::RoundToNext(u32(minSize + (maxSize - minSize) * f.w), alignment)) };

                if (currentSize + sz4.x <= totalSize) { currentSize += sz4.x; blks->push_back(sz4.x); }
                if (currentSize + sz4.y <= totalSize) { currentSize += sz4.y; blks->push_back(sz4.y); }
                if (currentSize + sz4.z <= totalSize) { currentSize += sz4.z; blks->push_back(sz4.z); }
                if (currentSize + sz4.w <= totalSize) { currentSize += sz4.w; blks->push_back(sz4.w); }
                else break;
            }
            return currentSize;
        };

        const size_t totalSizePerWorker = GTotalAllocationSize_;

        smallBlocksSizeInBytes += generator(&smallBlocks, BlockSizeMin, BlockSizeMid, ALLOCATION_BOUNDARY, totalSizePerWorker);
        largeBlocksSizeInBytes += generator(&largeBlocks, BlockSizeMid, BlockSizeLarge, 128, totalSizePerWorker);
        mixedBlocksSizeInBytes += generator(&mixedBlocks, BlockSizeMin, BlockSizeMid, ALLOCATION_BOUNDARY, totalSizePerWorker / 2);
        mixedBlocksSizeInBytes += generator(&mixedBlocks, BlockSizeMid, BlockSizeLarge, 128, totalSizePerWorker / 2);
    }

    LOG(Test_Allocators, Info, L"Small blocks data set = {0} blocks / {1}", smallBlocks.size(), Fmt::SizeInBytes(smallBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Large blocks data set = {0} blocks / {1}", largeBlocks.size(), Fmt::SizeInBytes(largeBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Mixed blocks data set = {0} blocks / {1}", mixedBlocks.size(), Fmt::SizeInBytes(mixedBlocksSizeInBytes) );

    ReleaseMemoryInModules();

    Test_Allocator_(L"FMallocator", FMallocator{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());

    ReleaseMemoryInModules();

    Test_Allocator_(L"FStdMallocator", FStdMallocator{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());

    ReleaseMemoryInModules();

    Test_CompressedRadixTrie_();

    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
