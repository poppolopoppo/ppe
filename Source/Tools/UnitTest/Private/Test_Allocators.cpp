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
#include "HAL/PlatformMemory.h"
#include "Maths/MathHelpers.h"
#include "Maths/Threefy.h"
#include "Memory/MemoryView.h"
#include "Memory/UniqueView.h"
#include "Time/TimedScope.h"
#include "Thread/Task/TaskHelpers.h"
#include "Thread/ThreadPool.h"

#include <algorithm>
#include <random>

#include "Modular/ModularDomain.h"

#define USE_TESTALLOCATOR_MEMSET 0

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Allocators)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
static constexpr size_t GTotalAllocationSize_ = CODE3264( 64u, 128u) * 1024u * size_t(1024);
#else
static constexpr size_t GTotalAllocationSize_ = CODE3264(128u, 256u) * 1024u * size_t(1024);
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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Allocators() {
    PPE_DEBUG_NAMEDSCOPE("Test_Allocators");

    LOG(Test_Allocators, Emphasis, L"starting allocator tests ...");

    typedef u8 value_type;

    constexpr size_t BlockSizeMin = 16;
    constexpr size_t BlockSizeMid = 32768;
    constexpr size_t BlockSizeLarge = 8*1024*1024;

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
                auto sz4 = rnd.UniformI(minSize, maxSize);

                sz4.x = u32(Meta::RoundToNext(sz4.x, alignment));
                sz4.y = u32(Meta::RoundToNext(sz4.y, alignment));
                sz4.z = u32(Meta::RoundToNext(sz4.z, alignment));
                sz4.w = u32(Meta::RoundToNext(sz4.w, alignment));

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
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
