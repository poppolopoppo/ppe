#include "stdafx.h"

#include "Core.h"

#include "Allocator/Allocation.h"

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
STATIC_ASSERT(has_stealallocatorblock_v<ALLOCATOR(Container), ALLOCATOR(Internal)>);
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::has_common_type_v<i32, i16>);
STATIC_ASSERT(Meta::has_common_type_v<i64, i32>);
STATIC_ASSERT(Meta::has_common_type_v<u64, i32>);
STATIC_ASSERT(Meta::has_common_type_v<float, i32>);
STATIC_ASSERT(Meta::has_common_type_v<float, double>);
STATIC_ASSERT(Meta::has_common_type_v<FMallocator, double> == false);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
