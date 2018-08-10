#pragma once

#include "Core.h"

#include "Allocator/PoolAllocatorTag.h"
#include "Allocator/SingletonPoolAllocator.h"

namespace PPE {
POOL_TAG_DECL(NodeBasedContainer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define NODEBASED_CONTAINER_ALLOCATOR(_Domain, T) \
    SINGLETON_POOL_ALLOCATOR(_Domain, COMMA_PROTECT(T), PPE::PoolTag::NodeBasedContainer/* hard coded to work in other namespaces */)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_Domain, T) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATOR(_Domain, COMMA_PROTECT(T), PPE::PoolTag::NodeBasedContainer/* hard coded to work in other namespaces */)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
