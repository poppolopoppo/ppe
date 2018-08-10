#pragma once

#include "Allocator/PoolAllocator.h"

#include "Memory/MemoryPool.h"
#include "Meta/Singleton.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define POOL_TAG_DEF(_Name) \
    namespace PoolTag { \
        typedef PPE::Meta::TSingleton< PPE::FMemoryPoolList, _Name > CONCAT(_Name, _type); \
        \
        void _Name::Register(IMemoryPool* ppool) { CONCAT(_Name, _type)::Get().Insert(ppool); } \
        void _Name::Unregister(IMemoryPool* ppool) { CONCAT(_Name, _type)::Get().Remove(ppool); } \
        \
        void _Name::Start() { CONCAT(_Name, _type)::Create(); } \
        void _Name::Shutdown() { CONCAT(_Name, _type)::Destroy(); } \
        \
        void _Name::ClearAll_AssertCompletelyFree() { CONCAT(_Name, _type)::Get().ClearAll_AssertCompletelyFree(); } \
        void _Name::ClearAll_IgnoreLeaks() { CONCAT(_Name, _type)::Get().ClearAll_IgnoreLeaks(); } \
        void _Name::ClearAll_UnusedMemory() { CONCAT(_Name, _type)::Get().ClearAll_UnusedMemory(); } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
