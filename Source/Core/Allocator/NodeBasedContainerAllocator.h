#pragma once

#include "Core/Core.h"

#include "Core/Allocator/SingletonPoolAllocator.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
POOLTAG_DEF(NodeBasedContainer);
//----------------------------------------------------------------------------
#define NODEBASED_CONTAINER_ALLOCATOR(_Domain, T) \
    SINGLETON_POOL_ALLOCATOR(_Domain, T, NodeBasedContainer)
//----------------------------------------------------------------------------
#define THREAD_LOCAL_NODEBASED_CONTAINER_ALLOCATOR(_Domain, T) \
    THREAD_LOCAL_SINGLETON_POOL_ALLOCATOR(_Domain, T, NodeBasedContainer)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
