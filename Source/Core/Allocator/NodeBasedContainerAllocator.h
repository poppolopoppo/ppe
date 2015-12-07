#pragma once

#include "Core/Core.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Allocator/SingletonPoolAllocator.h"

namespace Core {
POOLTAG_DECL(NodeBasedContainer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define NODEBASED_CONTAINER_ALLOCATOR(_Domain, T) \
    SINGLETON_POOL_ALLOCATOR(_Domain, COMMA_PROTECT(T), Core::PoolTag::NodeBasedContainer/* hard coded to work in other namespaces */)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_Domain, T) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATOR(_Domain, COMMA_PROTECT(T), Core::PoolTag::NodeBasedContainer/* hard coded to work in other namespaces */)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
