#include "stdafx.h"

#include "Core/Allocator/Mallocator.h"
#include "Core/Allocator/ThreadLocalAllocator.h"

#include "Core/Container/Pair.h"
#include "Core/Container/RingBuffer.h"
#include "Core/Container/Stack.h"
#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/StringView.h"
#include "Core/Maths/MathHelpers.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Misc/TargetPlatform.h"
#include "Core/Time/TimedScope.h"
#include "Core/Thread/Task/TaskHelpers.h"
#include "Core/Thread/ThreadPool.h"

#include <algorithm>
#include <allocators>

#define USE_TESTALLOCATOR_MEMSET 0

namespace Core {
namespace Test {
LOG_CATEGORY(, Test_Allocators)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
static constexpr size_t GNumBlocks_ = 1000;
#else
static constexpr size_t GNumBlocks_ = 10000;
#endif
#ifdef WITH_CORE_ASSERT
static constexpr size_t GLoopCount_ = 10;
#else
static constexpr size_t GLoopCount_ = 100;
#endif
#ifdef WITH_CORE_ASSERT
static constexpr size_t GSlidingWindow_ = 50;
#else
static constexpr size_t GSlidingWindow_ = 150;
#endif
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_ST_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    BENCHMARK_SCOPE(category, name);

    forrange(loop, 0, GLoopCount_) {
        for (size_t sz : blockSizes) {
            auto* ptr = allocator.allocate(sz);
#if USE_TESTALLOCATOR_MEMSET
            ::memset(ptr, 0xFA, sizeof(value_type) * sz);
#endif
            allocator.deallocate(ptr, sz);
        }
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_MT_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    BENCHMARK_SCOPE(category, name);

    forrange(loop, 0, GLoopCount_) {
        ParallelForEach(blockSizes.begin(), blockSizes.end(), [&allocator](size_t sz) {
            _Alloc alloc(allocator);
            auto* ptr = alloc.allocate(sz);
#if USE_TESTALLOCATOR_MEMSET
            ::memset(ptr, 0xFB, sizeof(value_type) * sz);
#endif
            alloc.deallocate(ptr, sz);
        });
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_Sliding_(const FWStringView& category, const FWStringView& name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes, size_t window) {
    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

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
            ::memset(ptr, 0xFC, sizeof(value_type) * sz);
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

    BENCHMARK_SCOPE(category, name);

    using value_type = typename std::allocator_traits<_Alloc>::value_type;
    STACKLOCAL_POD_ARRAY(value_type*, blockAddrs, blockSizes.size());

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
    constexpr size_t BlockSizeMid = 32736;
    const size_t BlockSizeMax = FPlatformMisc::SystemInfo.AllocationGranularity;

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

    LOG(Test_Allocators, Info, L"Small blocks data set = {0} blocks / {1}", smallBlocks.size(), Fmt::FSizeInBytes{ smallBlocksSizeInBytes } );
    LOG(Test_Allocators, Info, L"Large blocks data set = {0} blocks / {1}", largeBlocks.size(), Fmt::FSizeInBytes{ largeBlocksSizeInBytes });
    LOG(Test_Allocators, Info, L"Mixed blocks data set = {0} blocks / {1}", mixedBlocks.size(), Fmt::FSizeInBytes{ mixedBlocksSizeInBytes });

    Test_Allocator_(L"TMallocator", TMallocator<value_type>{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());
    Test_Allocator_(L"std::allocator", std::allocator<value_type>{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());
    Test_Allocator_(L"TThreadLocalAllocator", TThreadLocalAllocator<value_type>{}, smallBlocks.MakeConstView(), largeBlocks.MakeConstView(), mixedBlocks.MakeConstView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core
