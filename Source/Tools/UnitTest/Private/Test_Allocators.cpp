#include "stdafx.h"

#include "Allocator/Mallocator.h"
#include "Allocator/SlabHeap.h"
#include "Allocator/StlAllocator.h"
#include "Container/CompressedRadixTrie.h"
#include "Container/HashSet.h"
#include "Container/Pair.h"
#include "Container/RingBuffer.h"
#include "Container/Stack.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/StringView.h"
#include "Maths/MathHelpers.h"
#include "Maths/RandomGenerator.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/Threefy.h"
#include "Memory/CachedMemoryPool.h"
#include "Memory/MemoryPool.h"
#include "Memory/MemoryView.h"
#include "Memory/UniqueView.h"
#include "Modular/ModularDomain.h"
#include "Thread/AtomicPool.h"
#include "Thread/Task/TaskHelpers.h"
#include "Thread/ThreadPool.h"
#include "Time/TimedScope.h"

#include <algorithm>
#include <random>

#include "Allocator/SlabAllocator.h"
#include "Container/Map.h"

#define USE_TESTALLOCATOR_MEMSET (0) // we don't want to benchmark memset() performance

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Allocators)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_DEBUG && !USE_PPE_FASTDEBUG
static constexpr size_t GTotalAllocationSize_ = CODE3264( 16u,  32u) * 1024u * size_t(1024);
#else
static constexpr size_t GTotalAllocationSize_ = CODE3264( 64u, 128u) * 1024u * size_t(1024);
#endif
#if USE_PPE_ASSERT
static constexpr size_t GLoopCount_ = 5;
#else
static constexpr size_t GLoopCount_ = 100;
#endif
#if USE_PPE_ASSERT
static constexpr size_t GSlidingWindow_ = 30;
#else
static constexpr size_t GSlidingWindow_ = 150;
#endif
//----------------------------------------------------------------------------
template <typename _Alloc>
static NO_INLINE void Test_Allocator_ST_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    Unused(category); Unused(name);
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
static NO_INLINE void Test_Allocator_MT_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    Unused(category); Unused(name);
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
static NO_INLINE void Test_Allocator_Sliding_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes, size_t window) {
    Unused(category); Unused(name);
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
static NO_INLINE void Test_Allocator_Trashing_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    Unused(category); Unused(name);
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
static NO_INLINE void Test_Allocator_Dangling_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    FTaskManager& threadPool = FHighPriorityThreadPool::Get();

    const size_t numWorkers = threadPool.WorkerCount();
    const size_t allocsPerWorker = ((blockSizes.size() + numWorkers - 1) / numWorkers);
    Assert(numWorkers * allocsPerWorker >= blockSizes.size());

    using allocator_traits = TAllocatorTraits<_Alloc>;

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

    Unused(category); Unused(name);
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
static NO_INLINE void Test_Allocator_Realloc_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    Unused(category); Unused(name);
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
static NO_INLINE void Test_Allocator_(
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
static NO_INLINE void Test_CompressedRadixTrie_() {
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
        },  ETaskPriority::Normal, HighPriorityTaskContext() );

        Assert(radixTrie.empty());
        blocks.clear();
    }
}
//----------------------------------------------------------------------------
template <typename _Word>
static NO_INLINE void Test_BitTree_Impl_(FWStringView name, TMemoryView<const u32> sizes) {
    Unused(name);
    TAllocaBlock<_Word> alloc;
    TBitTree<_Word, false> tree;

    FRandomGenerator rnd;

    MALLOCA_POD(u32, tmpIds, sizes.Max());

    for (u32 capacity : sizes) {
        LOG(Test_Allocators, Emphasis, L"testing {0} with size {1}", name, capacity);

        tree.SetupMemoryRequirements(capacity);
        Assert_NoAssume(tree.DesiredSize == capacity);

        if (tree.TotalNumWords() != alloc.Count)
            alloc.Relocate(tree.TotalNumWords(), false);
        Assert_NoAssume(alloc.SizeInBytes() == tree.AllocationSize());
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(alloc.data(), alloc.SizeInBytes()));

        tree.Initialize(alloc.data(), false);

        u32 available = 0;
        for (;;) {
            const u32 bit = tree.NextAllocateBit();
            if (UMax == bit)
                break;

            tree.AllocateBit(bit);
            ++available;
        }

        AssertRelease_NoAssume(available == capacity);
        AssertRelease_NoAssume(tree.Full());
        AssertRelease_NoAssume(tree.CountOnes(capacity) == capacity);

        const auto ids = tmpIds.MakeView().CutBefore(capacity);
        MakeInterval(capacity).CopyTo(ids.begin());
        rnd.Shuffle(ids);

        for (auto bit : ids) {
            AssertRelease_NoAssume(tree.IsAllocated(bit));
            tree.Deallocate(bit);
        }

        AssertRelease_NoAssume(not tree.Full());
        AssertRelease_NoAssume(tree.CountOnes(capacity) == 0);
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_BitTrees_() {
    constexpr const u32 sizes[] = {
        32, 64, 256, 1024, 2048, 65536, // aligned
        2, 5, 11, 23, 47, 97, 197, 397, 797, 1597, 3203, 4519, 6421, 12853, 25717, 51437 // unaligned
    };
    Test_BitTree_Impl_<u32>(L"TBitTree<u32>", sizes);
    Test_BitTree_Impl_<u64>(L"TBitTree<u64>", sizes);
}
//----------------------------------------------------------------------------
struct FDummyForPool_ {
    u8 Data0{ 0 };
    u16 Data1{ 0 };

    mutable std::atomic<u32> RefCount_{ 0 };

    u64 Canary = 0xdeadbeefdeadbeefULL;

    FDummyForPool_() = default;
    FDummyForPool_(const FDummyForPool_& rhs) NOEXCEPT { operator =(rhs); }
    FDummyForPool_& operator =(const FDummyForPool_& rhs) NOEXCEPT {
        Data0 = rhs.Data0;
        Data1 = rhs.Data1;
        return (*this);
    }
    FDummyForPool_(FRandomGenerator& rng) {
        rng.Randomize(Data0);
        rng.Randomize(Data1);
    }

    ~FDummyForPool_() {
        AssertRelease(CheckInvariants());
    }

    bool CheckCanary() const NOEXCEPT {
        return (0xdeadbeefdeadbeefULL == Canary);
    }

    bool CheckInvariants() const NOEXCEPT {
        return (0 == RefCount_ && CheckCanary());
    }

    bool AddRef() const { return RefCount_.fetch_add(1, std::memory_order_relaxed) == 0; }
    bool RemoveRef() const { return RefCount_.fetch_sub(1, std::memory_order_relaxed) == 1; }

    bool operator ==(const FDummyForPool_& rhs) const {
        return (Data0 == rhs.Data0 && Data1 == rhs.Data1);
    }
    bool operator !=(const FDummyForPool_& rhs) const {
        return (not operator ==(rhs));
    }

    friend void swap(FDummyForPool_& lhs, FDummyForPool_& rhs) {
        std::swap(lhs.Data0, rhs.Data0);
        std::swap(lhs.Data1, rhs.Data1);
    }

    friend hash_t hash_value(const FDummyForPool_& dummy) {
        return hash_tuple(dummy.Data0, dummy.Data1);
    }
};
//----------------------------------------------------------------------------
static NO_INLINE void Test_AtomicPool_(ETaskPriority priority, ITaskContext* context) {
    LOG(Test_Allocators, Emphasis, L"testing TAtomicPool<>");

    BENCHMARK_SCOPE(L"Pool", L"TAtomicPool<>");

    using pool_type = TAtomicPool<FDummyForPool_, 32>;
    //using index_type = pool_type::index_type;

    STATIC_CONST_INTEGRAL(size_t, ToDeallocate, (pool_type::Capacity * 2) / 3);
    FRandomGenerator rng;
    TStaticArray<FDummyForPool_*, pool_type::Capacity> allocs;

#if USE_PPE_DEBUG
    constexpr int numLoops = 100;
#else
    constexpr int numLoops = 1000;
#endif

    pool_type pool;
    forrange(loop, 0, numLoops) {
        ParallelFor(0, pool_type::Capacity,
            [&allocs, &pool](size_t i) {
                allocs[i] = pool.Allocate();
                AssertRelease(allocs[i]->CheckInvariants());
                Verify( allocs[i]->AddRef() );
            }, priority, context);

        rng.Shuffle(allocs.MakeView());

        ParallelFor(0, ToDeallocate,
            [&allocs, &pool](size_t i) {
                Verify( allocs[i]->RemoveRef() );
                AssertRelease(allocs[i]->CheckInvariants());
                pool.Release(allocs[i]);
            }, priority, context);

        pool.Clear_ReleaseMemory();

        ParallelFor(0, ToDeallocate,
            [&allocs, &pool](size_t i) {
                allocs[i] = pool.Allocate();
                AssertRelease(allocs[i]->CheckInvariants());
                Verify( allocs[i]->AddRef() );
            }, priority, context);

        rng.Shuffle(allocs.MakeView());

        ParallelFor(0, pool_type::Capacity,
            [&allocs, &pool](size_t i) {
                Verify( allocs[i]->RemoveRef() );
                AssertRelease(allocs[i]->CheckInvariants());
                pool.Release(allocs[i]);
            }, priority, context);

        pool.Clear_ReleaseMemory();
    }
}
//----------------------------------------------------------------------------
template <typename _MemoryPool>
static NO_INLINE void Test_MemoryPool_Impl_(FWStringView name, ETaskPriority priority, ITaskContext* context) {
    Unused(name);
    LOG(Test_Allocators, Emphasis, L"testing {0}", name);

    BENCHMARK_SCOPE(L"Pool", name);

    using pool_type = _MemoryPool;
    using index_type = typename pool_type::index_type;

    STATIC_CONST_INTEGRAL(size_t, ToDeallocate, (static_cast<size_t>(pool_type::MaxSize) * 2) / 3);
    FRandomGenerator rng;
    TStaticArray<index_type, pool_type::MaxSize> allocs;

#if USE_PPE_DEBUG
    constexpr int numLoops = 100;
#else
    constexpr int numLoops = 1000;
#endif

    pool_type pool;
    forrange(loop, 0, numLoops) {
        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, pool_type::MaxSize,
            [&allocs, &pool](size_t i) {
                allocs[i] = pool.Allocate();
            }, priority, context);

        Assert_NoAssume(pool.CheckInvariants());

        rng.Shuffle(allocs.MakeView());

        ParallelFor(0, ToDeallocate,
            [&allocs, &pool](size_t i) {
                pool.Deallocate(allocs[i]);
            }, priority, context);

        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, ToDeallocate,
            [&allocs, &pool](size_t i) {
                allocs[i] = pool.Allocate();
            }, priority, context);

        Assert_NoAssume(pool.CheckInvariants());

        rng.Shuffle(allocs.MakeView());

        ParallelFor(0, pool_type::MaxSize,
            [&allocs, &pool](size_t i) {
                pool.Deallocate(allocs[i]);
            }, priority, context);

        Assert_NoAssume(pool.CheckInvariants());

        pool.Clear_AssertCompletelyEmpty();
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_MemoryPools_(ETaskPriority priority, ITaskContext* context) {
    constexpr auto ChunkSize = CODE3264(64, 128);
    constexpr auto MaxChunks = 32;

    using FMemoryPool = TTypedMemoryPool<FDummyForPool_, ChunkSize, MaxChunks, ALLOCATOR(Container)>;
    Test_MemoryPool_Impl_<FMemoryPool>(L"TMemoryPool<>", priority, context);
}
//----------------------------------------------------------------------------
template <typename _CachedMemoryPool>
static NO_INLINE void Test_CachedMemoryPool_Impl_(
    FWStringView name,
    const FRandomGenerator& seed,
    TMemoryView<const FDummyForPool_> uniq,
    TMemoryView<const FDummyForPool_> shuf,
    ETaskPriority priority, ITaskContext* context) {
    Unused(name);
    LOG(Test_Allocators, Emphasis, L"testing {0}", name);

#if USE_PPE_DEBUG
    constexpr int numLoops = 100;
#else
    constexpr int numLoops = 1000;
#endif

    using pool_type = _CachedMemoryPool;
    //using block_type = typename pool_type::block_type;
    using index_type = typename pool_type::index_type;

    STATIC_CONST_INTEGRAL(u32, ToDeallocate, u32((pool_type::MaxSize * 2) / 3));
    AssertRelease(uniq.size() == pool_type::MaxSize);
    AssertRelease(shuf.size() == pool_type::MaxSize);

    STACKLOCAL_POD_ARRAY(index_type, uniqAllocs, pool_type::MaxSize);
    STACKLOCAL_POD_ARRAY(index_type, shufAllocs, pool_type::MaxSize);

    FRandomGenerator rng{ seed };
    STACKLOCAL_POD_ARRAY(index_type, reorder, pool_type::MaxSize);
    MakeInterval<index_type>(pool_type::MaxSize).CopyTo(reorder.begin());

    pool_type pool;

    BENCHMARK_SCOPE(L"Pool", name);

    forrange(loop, 0, numLoops) {
        AssertRelease(0 == pool.NumCachedBlocks());

        rng.Shuffle(reorder);

        ParallelFor(0, uniq.size(),
            [&](size_t i) {
                pool.FindOrAdd(FDummyForPool_{ uniq[reorder[i]] }, [&](const FDummyForPool_* pblock, index_type id, bool exist) {
                    AssertRelease(not exist);
                    Assert(pblock->CheckCanary());
                    Assert(uniq[reorder[i]] == *pblock);
                    VerifyRelease(pblock->AddRef());
                    uniqAllocs[i] = id;
                });
            }, priority, context);

        AssertRelease(pool_type::MaxSize == pool.NumCachedBlocks());
        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, shuf.size(),
            [&](size_t i) {
                pool.FindOrAdd(FDummyForPool_{ shuf[reorder[i]] }, [&](const FDummyForPool_* pblock, index_type id, bool exist) {
                    AssertRelease(exist);
                    Assert(pblock->CheckCanary());
                    Assert(shuf[reorder[i]] == *pblock);
                    VerifyRelease(not pblock->AddRef());
                    shufAllocs[i] = id;
                });
            }, priority, context);

        AssertRelease(pool_type::MaxSize == pool.NumCachedBlocks());
        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, shuf.size(),
            [&](size_t i) {
                pool.RemoveIf(shufAllocs[i], [&](const FDummyForPool_* pblock) {
                    Assert(pblock->CheckCanary());
                    Assert(shuf[reorder[i]] == *pblock);
                    VerifyRelease(not pblock->RemoveRef());
                    shufAllocs[i] = UMax;
                    return false;
                });
            }, priority, context);

        AssertRelease(pool_type::MaxSize == pool.NumCachedBlocks());
        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, ToDeallocate,
            [&](size_t i) {
                pool.RemoveIf(uniqAllocs[i], [&](const FDummyForPool_* pblock) {
                    Assert(pblock->CheckCanary());
                    Assert(uniq[reorder[i]] == *pblock);
                    VerifyRelease(pblock->RemoveRef());
                    uniqAllocs[i] = UMax;
                    return true;
                });
            }, priority, context);

        AssertRelease(pool_type::MaxSize - ToDeallocate == pool.NumCachedBlocks());
        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, ToDeallocate,
            [&](size_t i) {
                Assert_NoAssume(UMax == uniqAllocs[i]);
                pool.FindOrAdd(FDummyForPool_{ uniq[reorder[i]] }, [&](const FDummyForPool_* pblock, index_type id, bool exist) {
                    AssertRelease(not exist);
                    Assert(pblock->CheckCanary());
                    Assert(uniq[reorder[i]] == *pblock);
                    VerifyRelease(pblock->AddRef());
                    uniqAllocs[i] = id;
                });
            }, priority, context);

        AssertRelease(pool_type::MaxSize == pool.NumCachedBlocks());
        Assert_NoAssume(pool.CheckInvariants());

        ParallelFor(0, uniq.size(),
            [&](size_t i) {
                pool.RemoveIf(uniqAllocs[i], [&](const FDummyForPool_* pblock) {
                    Assert(pblock->CheckCanary());
                    Assert(uniq[reorder[i]] == *pblock);
                    VerifyRelease(pblock->RemoveRef());
                    uniqAllocs[i] = UMax;
                    return true;
                });
            }, priority, context);

        AssertRelease(0 == pool.NumCachedBlocks());
        Assert_NoAssume(pool.CheckInvariants());

#if USE_PPE_DEBUG
        for (index_type id : uniqAllocs)
            Assert_NoAssume(UMax == id);
        for (index_type id : shufAllocs)
            Assert_NoAssume(UMax == id);
#endif

        pool.Clear_AssertCompletelyEmpty();
    }
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_CachedMemoryPools_(ETaskPriority priority, ITaskContext* context) {
    constexpr auto ChunkSize = CODE3264(64, 128);
    constexpr auto MaxChunks = 32;
    constexpr auto MaxSize = (MaxChunks * ChunkSize);

    FRandomGenerator rng;

    VECTOR(Container, FDummyForPool_) uniq;
    {
        HASHSET(Container, FDummyForPool_) tmp;
        tmp.reserve(MaxSize);

        while (tmp.size() < MaxSize) {
            FDummyForPool_ key{ rng };
            tmp.insert(key);
        }

        uniq.reserve(tmp.size());
        uniq.assign(tmp.begin(), tmp.end());
    }

    VECTOR(Container, FDummyForPool_) shuffled( uniq );
    rng.Shuffle(shuffled.MakeView());

    using FCachedMemoryPool = TCachedMemoryPool<FDummyForPool_, FDummyForPool_, ChunkSize, MaxChunks, ALLOCATOR(Container)>;
    Test_CachedMemoryPool_Impl_<FCachedMemoryPool>(L"TCachedMemoryPool<>", rng, uniq.MakeConstView(), shuffled.MakeConstView(), priority, context);
}
//----------------------------------------------------------------------------
static NO_INLINE void Test_Pools_() {
    const ETaskPriority priority = ETaskPriority::Normal;
    ITaskContext* const context = HighPriorityTaskContext();

    Test_AtomicPool_(priority, context);
    Test_MemoryPools_(priority, context);
    Test_CachedMemoryPools_(priority, context);
}
//----------------------------------------------------------------------------
NO_INLINE void Test_SlabHeap_() {
    SLABHEAP(Container) heap;
    heap.SetSlabSize(malloc_snap_size(20 * sizeof(hash_t))); // looking for troubles

    auto make_canary = [](TMemoryView<hash_t> canary, hash_t seed) {
        hash_t h{ PPE_HASH_VALUE_SEED };
        for (hash_t& value : canary) {
            hash_combine(h, seed);
            value = h;
        }
    };
    auto test_canary = [](TMemoryView<const hash_t> canary, hash_t seed) -> bool {
        hash_t h{ PPE_HASH_VALUE_SEED };
        for (const hash_t& value : canary) {
            hash_combine(h, seed);
            if (value != h)
                return false;
        }
        return true;
    };

    const hash_t seed0 = hash_value("canary0");
    const hash_t seed1 = hash_value("canary1");
    const hash_t seed2 = hash_value("canary2");

    const auto canary0 = heap.AllocateT<hash_t>(10);

    make_canary(canary0, seed0);

    const auto canary1 = heap.AllocateT<hash_t>(13);
    AssertRelease(test_canary(canary0, seed0));

    make_canary(canary1, seed1);

    const auto canary2 = heap.AllocateT<hash_t>(16);
    AssertRelease(test_canary(canary0, seed0));
    AssertRelease(test_canary(canary1, seed1));

    make_canary(canary2, seed2);

    AssertRelease(test_canary(canary0, seed0));
    AssertRelease(test_canary(canary1, seed1));
    AssertRelease(test_canary(canary2, seed2));

    PP_FOREACH_ARGS(UNUSED, canary0, canary1, canary2);

    heap.DiscardAll();
}
//----------------------------------------------------------------------------
NO_INLINE void Test_SlabHeapPooled_() {
    const hash_t seed0 = hash_value("canary0");
    const hash_t seed1 = hash_value("canary1");
    const hash_t seed2 = hash_value("canary2");
    const hash_t seed3 = hash_value("canary3");

    SLABHEAP_POOLED(Container) heap;
    heap.SetSlabSize(16_KiB);
    auto mainAllocator = TSlabAllocator{ heap };

    forrange(i, 0, 100) {
        TPoolingSlabHeap subHeap{ mainAllocator };
        subHeap.SetSlabSize(4_KiB);

        auto subAllocator = TSlabAllocator{ subHeap };
        using suballocator_type = Meta::TDecay<decltype(subAllocator)>;

        TVector<i64, suballocator_type> ints{ subAllocator };
        ints.insert(ints.end(), { 1, 2, 3 });
        ints.insert(ints.end(), { 4, 5, 6 });
        ints.insert(ints.end(), { 7, 8, 9 });

        TMap<hash_t, double, Meta::TLess<hash_t>, suballocator_type> map{ TStlAllocator<TPair<const hash_t, double>, suballocator_type>(subAllocator) };
        map.insert({ seed0, 0.0f });
        map.insert({ seed1, 1.0f });
        map.insert({ seed2, 2.0f });
        map.insert({ seed3, 3.0f });

        ints.insert(ints.end(), { 1, 2, 3 });
        ints.insert(ints.end(), { 4, 5, 6 });
        ints.insert(ints.end(), { 7, 8, 9 });

        THashMap<hash_t, double, Meta::THash<hash_t>, Meta::TEqualTo<hash_t>, suballocator_type> table{ subAllocator };
        table.insert({ seed0, 0.0f });
        table.insert({ seed1, 1.0f });
        table.insert({ seed2, 2.0f });
        table.insert({ seed3, 3.0f });

        ints.insert(ints.end(), { 1, 2, 3 });
        ints.insert(ints.end(), { 4, 5, 6 });
        ints.insert(ints.end(), { 7, 8, 9 });
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Allocators() {
    PPE_DEBUG_NAMEDSCOPE("Test_Allocators");

    Test_CompressedRadixTrie_();
    Test_BitTrees_();
    Test_Pools_();
    Test_SlabHeap_();
    Test_SlabHeapPooled_();

    ReleaseMemoryInModules();

    LOG(Test_Allocators, Emphasis, L"starting allocator tests ...");

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

        auto generator = [&rnd](blocksizes_t* blks, u32 minSize, u32 maxSize, u32 alignment, size_t totalSize) NOEXCEPT -> size_t {
            u32 currentSize = 0;
            for (;;) {
                float4 f = rnd.UniformF(0.f, 1.f);

                const u324 sz4{
                    u32(Meta::RoundToNextPow2(u32(minSize + (maxSize - minSize) * f.x), alignment)),
                    u32(Meta::RoundToNextPow2(u32(minSize + (maxSize - minSize) * f.y), alignment)),
                    u32(Meta::RoundToNextPow2(u32(minSize + (maxSize - minSize) * f.z), alignment)),
                    u32(Meta::RoundToNextPow2(u32(minSize + (maxSize - minSize) * f.w), alignment)) };

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

    Unused(smallBlocksSizeInBytes);
    Unused(mixedBlocksSizeInBytes);
    Unused(largeBlocksSizeInBytes);

    LOG(Test_Allocators, Info, L"Small blocks data set = {0} blocks / {1}", smallBlocks.size(), Fmt::SizeInBytes(smallBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Large blocks data set = {0} blocks / {1}", largeBlocks.size(), Fmt::SizeInBytes(largeBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Mixed blocks data set = {0} blocks / {1}", mixedBlocks.size(), Fmt::SizeInBytes(mixedBlocksSizeInBytes) );

    ReleaseMemoryInModules();

    Test_Allocator_(L"FMallocator", FMallocator{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());

    ReleaseMemoryInModules();

    Test_Allocator_(L"FStdMallocator", FStdMallocator{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());

    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
