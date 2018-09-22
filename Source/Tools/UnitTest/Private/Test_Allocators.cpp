#include "stdafx.h"

#include "Allocator/Mallocator.h"

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

#define USE_TESTALLOCATOR_MEMSET 0

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Allocators)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef WITH_PPE_ASSERT
static constexpr size_t GNumBlocks_ = 1000;
#else
static constexpr size_t GNumBlocks_ = 10000;
#endif
#ifdef WITH_PPE_ASSERT
static constexpr size_t GLoopCount_ = 10;
#else
static constexpr size_t GLoopCount_ = 100;
#endif
#ifdef WITH_PPE_ASSERT
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
        ParallelForEach(blockSizes.begin(), blockSizes.end(), [&allocator](size_t sz) {
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
    const size_t allocsPerWorker = (blockSizes.size() / numWorkers);

    using pointer = typename std::allocator_traits<_Alloc>::pointer;
    using blocks_type = TUniqueArray<pointer>;

    TVector<blocks_type> perWorkerIndex;
    perWorkerIndex.reserve_AssumeEmpty(numWorkers);
    forrange(i, 0, numWorkers) {
        const auto workerSizes = blockSizes.Slice(i, allocsPerWorker);
        perWorkerIndex.emplace_back_AssumeNoGrow(NewArray<pointer>(workerSizes.size()));
    }

    TVector<FTaskFunc> allocateTasks;
    allocateTasks.reserve_AssumeEmpty(numWorkers);
    TVector<FTaskFunc> deallocateTasks;
    deallocateTasks.reserve_AssumeEmpty(numWorkers);

    const struct payload_t {
        _Alloc& Allocator;
        const TMemoryView<const size_t> BlockSize;
        const TVector<blocks_type>& PerWorkerIndex;
    }   payload{ allocator, blockSizes, perWorkerIndex };

    forrange(i, 0, numWorkers) {
        allocateTasks.emplace_back_AssumeNoGrow([i, allocsPerWorker, &payload](ITaskContext&) {
            const size_t* sz = payload.BlockSize.data() + i * allocsPerWorker;
            for (pointer& p : payload.PerWorkerIndex[i])
                p = payload.Allocator.allocate(*sz++);
        });
        deallocateTasks.emplace_back_AssumeNoGrow([i, allocsPerWorker, &payload](ITaskContext&) {
            const size_t* sz = payload.BlockSize.data() + i * allocsPerWorker;
            for (pointer p : payload.PerWorkerIndex[i])
                payload.Allocator.deallocate(p, *sz++);
        });
    }

    std::random_device rdevice;
    std::mt19937 rand(rdevice());

    NOOP(category, name);
    BENCHMARK_SCOPE(category, name);

    // tries to free blocks from another thread from which they were allocated
    // this is the worst case for many allocators
    forrange(loop, 0, GLoopCount_) {
        // allocate from each worker
        threadPool.RunAndWaitFor(allocateTasks.MakeConstView());
        // randomize per allocator blocks
        std::shuffle(perWorkerIndex.begin(), perWorkerIndex.end(), rand);
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
    LOG(Test_Allocators, Emphasis, L"starting allocator tests ...");

    typedef u8 value_type;

    constexpr size_t BlockSizeMin = 16;
    constexpr size_t BlockSizeMid = 32768;
    const size_t BlockSizeMax = checked_cast<size_t>(FPlatformMemory::Constants().AllocationGranularity);

    size_t smallBlocksSizeInBytes = 0;
    size_t largeBlocksSizeInBytes = 0;
    size_t mixedBlocksSizeInBytes = 0;

    VECTOR(Container, size_t) smallBlocks;
    VECTOR(Container, size_t) largeBlocks;
    VECTOR(Container, size_t) mixedBlocks;
    {
        FRandomGenerator rand;

        smallBlocks.resize_Uninitialized(GNumBlocks_);
        for (size_t& blockSize : smallBlocks) {
            blockSize = Lerp(BlockSizeMin, BlockSizeMid, rand.NextFloat01());
            smallBlocksSizeInBytes += blockSize * sizeof(value_type);
        }

        largeBlocks.resize_Uninitialized(GNumBlocks_);
        for (size_t& blockSize : largeBlocks) {
            blockSize = Lerp(BlockSizeMid+1, BlockSizeMax, rand.NextFloat01());
            largeBlocksSizeInBytes += blockSize * sizeof(value_type);
        }

        mixedBlocks.resize_Uninitialized(GNumBlocks_);
        for (size_t& blockSize : mixedBlocks) {
            blockSize = Lerp(BlockSizeMin, BlockSizeMax, Sqr(rand.NextFloat01()));
            mixedBlocksSizeInBytes += blockSize * sizeof(value_type);
        }
    }

    LOG(Test_Allocators, Info, L"Small blocks data set = {0} blocks / {1}", smallBlocks.size(), Fmt::SizeInBytes(smallBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Large blocks data set = {0} blocks / {1}", largeBlocks.size(), Fmt::SizeInBytes(largeBlocksSizeInBytes) );
    LOG(Test_Allocators, Info, L"Mixed blocks data set = {0} blocks / {1}", mixedBlocks.size(), Fmt::SizeInBytes(mixedBlocksSizeInBytes) );

    Test_Allocator_(L"TMallocator", TMallocator<value_type>{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());
    Test_Allocator_(L"std::allocator", std::allocator<value_type>{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());

    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
