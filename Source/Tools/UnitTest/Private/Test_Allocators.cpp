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
#include "Maths/RandomGenerator.h"
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
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    NOOP(category, name);
    BENCHMARK_SCOPE(category, name);

    forrange(loop, 0, GLoopCount_) {
        for (size_t sz : blockSizes) {
            auto* ptr = allocator.allocate(sz);
#if USE_TESTALLOCATOR_MEMSET
            FPlatformMemory::Memset(ptr, 0xFA, sizeof(value_type) * sz);
#endif
            allocator.deallocate(ptr, sz);
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_MT_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    NOOP(category, name);
    BENCHMARK_SCOPE(category, name);

    forrange(loop, 0, GLoopCount_) {
        ParallelForEachValue(blockSizes.begin(), blockSizes.end(), [&allocator](size_t sz) {
            _Alloc alloc(allocator);
            auto* ptr = alloc.allocate(sz);
#if USE_TESTALLOCATOR_MEMSET
            FPlatformMemory::Memset(ptr, 0xFB, sizeof(value_type) * sz);
#endif
            alloc.deallocate(ptr, sz);
        });
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Sliding_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes, size_t window) {
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    NOOP(category, name);
    BENCHMARK_SCOPE(category, name);

    const size_t numDeallocs = Max(size_t(1), window / 10);

    using pointer = typename std::allocator_traits<_Alloc>::pointer;
    using block_type = TPair<pointer, size_t>;
    STACKLOCAL_RINGBUFFER(block_type, blockAddrs, window);

    block_type block;

    forrange(loop, 0, GLoopCount_) {
        for(size_t sz : blockSizes) {
            pointer ptr = allocator.allocate(sz);
#if USE_TESTALLOCATOR_MEMSET
            FPlatformMemory::Memset(ptr, 0xFC, sizeof(value_type) * sz);
#endif

            if (blockAddrs.size() == window) {
                forrange(i, 0, numDeallocs) {
                    if (not blockAddrs.pop_front(&block))
                        AssertNotReached();

                    allocator.deallocate(block.first, block.second);
                }
            }

            blockAddrs.push_back(ptr, sz);
        }

        while (blockAddrs.pop_back(&block))
            allocator.deallocate(block.first, block.second);
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Trashing_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    NOOP(category, name);
    BENCHMARK_SCOPE(category, name);

    using pointer = typename std::allocator_traits<_Alloc>::pointer;
    STACKLOCAL_POD_ARRAY(pointer, blockAddrs, blockSizes.size());

    forrange(loop, 0, GLoopCount_) {
        forrange(i, 0, blockSizes.size()) {
            const size_t sz = blockSizes[i];
            auto* ptr = allocator.allocate(sz);
            blockAddrs[i] = ptr;
        }

        forrange(i, 0, blockSizes.size()) {
            allocator.deallocate(blockAddrs[i], blockSizes[i]);
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Dangling_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    FTaskManager& threadPool = FHighPriorityThreadPool::Get();

    const size_t numWorkers = threadPool.WorkerCount();
    const size_t allocsPerWorker = ((blockSizes.size() + numWorkers - 1) / numWorkers);
    Assert(numWorkers * allocsPerWorker >= blockSizes.size());

    using alloc_traits = std::allocator_traits<_Alloc>;
    struct FBlock {
        typename alloc_traits::pointer Ptr;
        size_t SizeInBytes;
    };

    using pointer = typename std::allocator_traits<_Alloc>::pointer;
    using blocks_type = TUniqueArray<FBlock>;

    TVector<FBlock> blocks;
    blocks.reserve(blockSizes.size());
    for (size_t sz : blockSizes)
        blocks.emplace_back_AssumeNoGrow(nullptr, sz);

    TVector<FTaskFunc> allocateTasks;
    allocateTasks.reserve_AssumeEmpty(numWorkers);
    TVector<FTaskFunc> deallocateTasks;
    deallocateTasks.reserve_AssumeEmpty(numWorkers);

    struct payload_t {
        _Alloc& Allocator;
        TMemoryView<FBlock> Blocks;
    }   payload{ allocator, blocks.MakeView() };

    forrange(i, 0, numWorkers) {
        u32 bbegin = u32(i * allocsPerWorker);
        u32 bend = u32(Min(bbegin + allocsPerWorker, blocks.size()));

        if (bbegin >= bend)
            break;

        allocateTasks.emplace_back_AssumeNoGrow([bbegin, bend, &payload](ITaskContext&) {
            for (FBlock& b : payload.Blocks.SubRange(bbegin, bend - bbegin))
                b.Ptr = payload.Allocator.allocate(b.SizeInBytes);
        });
        deallocateTasks.emplace_back_AssumeNoGrow([bbegin, bend, &payload](ITaskContext&) {
            for (FBlock& b : payload.Blocks.SubRange(bbegin, bend - bbegin))
                payload.Allocator.deallocate(b.Ptr, b.SizeInBytes);
        });
    }

    std::random_device rdevice;
    std::mt19937 rand(rdevice());

    NOOP(category, name);
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
static void Test_Allocator_(
    const FWStringView& name, _Alloc&& allocator,
    const TMemoryView<const size_t>& smallBlocks,
    const TMemoryView<const size_t>& largeBlocks,
    const TMemoryView<const size_t>& mixedBlocks ) {

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
        FRandomGenerator rand;

        auto generator = [&rand](blocksizes_t* blks, size_t minSize, size_t maxSize, size_t totalSize) NOEXCEPT -> size_t {
            size_t currentSize = 0;
            for (;;) {
                size_t sz = Lerp(minSize, maxSize, rand.NextFloat01());
                if (currentSize + sz > totalSize)
                    break;
                blks->push_back(sz);
                currentSize += sz;
            }
            return currentSize;
        };

        const size_t totalSizePerWorker = GTotalAllocationSize_;

        smallBlocksSizeInBytes += generator(&smallBlocks, BlockSizeMin, BlockSizeMid, totalSizePerWorker);
        largeBlocksSizeInBytes += generator(&largeBlocks, BlockSizeMid, BlockSizeLarge, totalSizePerWorker);
        mixedBlocksSizeInBytes += generator(&mixedBlocks, BlockSizeMin, BlockSizeMid, totalSizePerWorker / 2);
        mixedBlocksSizeInBytes += generator(&mixedBlocks, BlockSizeMid, BlockSizeLarge, totalSizePerWorker / 2);
    }

    LOG(Test_Allocators, Info, L"Small blocks data set = {0} blocks / {1}", smallBlocks.size(), Fmt::SizeInBytes(smallBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Large blocks data set = {0} blocks / {1}", largeBlocks.size(), Fmt::SizeInBytes(largeBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Mixed blocks data set = {0} blocks / {1}", mixedBlocks.size(), Fmt::SizeInBytes(mixedBlocksSizeInBytes) );

    ReleaseMemoryInModules();

    Test_Allocator_(L"TMallocator", TStlAllocator<value_type, FMallocator>{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());

    ReleaseMemoryInModules();

    Test_Allocator_(L"std::allocator", std::allocator<value_type>{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());

    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
