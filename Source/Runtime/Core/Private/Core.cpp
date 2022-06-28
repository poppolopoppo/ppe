#include "stdafx.h"

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Meta/ForRange.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(u8) == 1);
STATIC_ASSERT(sizeof(u16) == sizeof(u8) + sizeof(u8));
STATIC_ASSERT(sizeof(u32) == sizeof(u16) + sizeof(u16));
STATIC_ASSERT(sizeof(u64) == sizeof(u32) + sizeof(u32));
STATIC_ASSERT(sizeof(u96) == sizeof(u64) + sizeof(u32));
STATIC_ASSERT(sizeof(u128) == sizeof(u64) + sizeof(u64));
STATIC_ASSERT(sizeof(u160) == sizeof(u128) + sizeof(u32));
STATIC_ASSERT(sizeof(u192) == sizeof(u128) + sizeof(u64));
STATIC_ASSERT(sizeof(u224) == sizeof(u128) + sizeof(u96));
STATIC_ASSERT(sizeof(u256) == sizeof(u128) * 2);
//----------------------------------------------------------------------------
STATIC_ASSERT(has_stealallocatorblock_v<FMallocator, FMallocator>);
STATIC_ASSERT(has_stealallocatorblock_v<ALLOCATOR(Container), ALLOCATOR(Diagnostic)>);
#if USE_PPE_MEMORYDOMAINS
STATIC_ASSERT(not TAllocatorTraits<FMallocator>::has_memory_tracking::value);
STATIC_ASSERT(TAllocatorTraits<ALLOCATOR(Container)>::has_memory_tracking::value);
STATIC_ASSERT(TAllocatorTraits<ALLOCATOR_MINSIZE(Container, size_t, 4)>::has_memory_tracking::value);
STATIC_ASSERT(TAllocatorTraits<BATCH_ALLOCATOR(Container, size_t)>::has_memory_tracking::value);
STATIC_ASSERT(TAllocatorTraits<INLINE_ALLOCATOR(Container, size_t, 4)>::has_memory_tracking::value);
STATIC_ASSERT(TAllocatorTraits<INLINE_STACK_ALLOCATOR(Container, size_t, 4)>::has_memory_tracking::value);
STATIC_ASSERT(TAllocatorTraits<STACKLOCAL_ALLOCATOR()>::has_memory_tracking::value);
#endif
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::has_common_type_v<i32, i16>);
STATIC_ASSERT(Meta::has_common_type_v<i64, i32>);
STATIC_ASSERT(Meta::has_common_type_v<u64, i32>);
STATIC_ASSERT(Meta::has_common_type_v<float, i32>);
STATIC_ASSERT(Meta::has_common_type_v<float, double>);
STATIC_ASSERT(Meta::has_common_type_v<FMallocator, double> == false);
//----------------------------------------------------------------------------
static_assert(not PPE_HAS_CXX20 || PPE_VA_OPT_SUPPORTED, "C++20 compilers should support __VA_OPT__");
STATIC_ASSERT(PP_NUM_ARGS() == 0);
STATIC_ASSERT(PP_NUM_ARGS(a) == 1);
STATIC_ASSERT(PP_NUM_ARGS(a, b) == 2);
STATIC_ASSERT(PP_NUM_ARGS(a, b, c, d, e, f, g, h) == 8);
STATIC_ASSERT(PP_NUM_ARGS(a, b, c, d, e, f, g, h, i, j, k) == 11);
STATIC_ASSERT(PP_NUM_ARGS(a, b, c, d, e, f, g, h, i, j, k, l, m, n) == 14);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
