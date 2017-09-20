#include "stdafx.h"

#include "Core/Allocator/Mallocator.h"
#include "Core/Allocator/ThreadLocalAllocator.h"

#include "Core/Container/Vector.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/Maths/RandomGenerator.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Time/TimedScope.h"
#include "Core/Thread/Task/TaskHelpers.h"
#include "Core/Thread/ThreadPool.h"

#include <algorithm>
#include <allocators>

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_ST_(const wchar_t* name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    const FBenchmarkScope bench(name, L"single thread");

    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    for (size_t sz : blockSizes) {
        auto* ptr = allocator.allocate(sz);
        memset(ptr, 0xFA, sizeof(value_type) * sz);
        allocator.deallocate(ptr, sz);
    }
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_MT_(const wchar_t* name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    const FBenchmarkScope bench(name, L"multi thread");

    typedef typename std::allocator_traits<_Alloc>::value_type value_type;

    parallel_for(blockSizes.begin(), blockSizes.end(), [&allocator](size_t sz) {
        auto* ptr = allocator.allocate(sz);
        memset(ptr, 0xFA, sizeof(value_type) * sz);
        allocator.deallocate(ptr, sz);
    });
}
//----------------------------------------------------------------------------
template <typename _Alloc>
static void Test_Allocator_(const wchar_t* name, _Alloc&& allocator, const TMemoryView<const size_t>& blockSizes) {
    LOG(Info, L"{0}", Repeat(L"-*=*", 20));
    const FBenchmarkScope bench(name, L"global");

    Test_Allocator_ST_(name, std::forward<_Alloc>(allocator), blockSizes);
    Test_Allocator_MT_(name, std::forward<_Alloc>(allocator), blockSizes);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Allocators() {
#ifdef WITH_CORE_ASSERT
    static constexpr size_t NumBlocks = 10000;
#else
    static constexpr size_t NumBlocks = 1000000;
#endif

    constexpr size_t BlockSizeMin = 16;
    constexpr size_t BlockSizeMax = 64 * 1024;

    VECTOR(Container, size_t) blockSizes;
    {
        FRandomGenerator rand;

        blockSizes.resize_Uninitialized(NumBlocks);
        for (size_t& blockSize : blockSizes)
            blockSize = rand.Next(BlockSizeMin, BlockSizeMax);

        std::random_shuffle(blockSizes.begin(), blockSizes.end());
    }

    Test_Allocator_(L"std::allocator", std::allocator<u8>{}, blockSizes.MakeConstView());
    Test_Allocator_(L"Core::TMallocator", Core::TMallocator<u8>{}, blockSizes.MakeConstView());
    Test_Allocator_(L"Core::TThreadLocalAllocator", Core::TThreadLocalAllocator<u8>{}, blockSizes.MakeConstView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
