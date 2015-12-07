#pragma once

#include "Core/Allocator/PoolAllocator.h"

#include "Core/Memory/MemoryPool.h"
#include "Core/Meta/Singleton.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define POOLTAG_DEF(_Name) \
    namespace PoolTag { \
        typedef Core::Meta::Singleton< Core::MemoryPoolBaseList, _Name > CONCAT(_Name, _type); \
        \
        void _Name::Register(MemoryPoolBase* ppool) { CONCAT(_Name, _type)::Instance().Insert(ppool); } \
        void _Name::Unregister(MemoryPoolBase* ppool) { CONCAT(_Name, _type)::Instance().Remove(ppool); } \
        \
        void _Name::Start() { CONCAT(_Name, _type)::Create(); } \
        void _Name::Shutdown() { CONCAT(_Name, _type)::Destroy(); } \
        \
        void _Name::ClearAll_AssertCompletelyFree() { CONCAT(_Name, _type)::Instance().ClearAll_AssertCompletelyFree(); } \
        void _Name::ClearAll_IgnoreLeaks() { CONCAT(_Name, _type)::Instance().ClearAll_IgnoreLeaks(); } \
        void _Name::ClearAll_UnusedMemory() { CONCAT(_Name, _type)::Instance().ClearAll_UnusedMemory(); } \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
