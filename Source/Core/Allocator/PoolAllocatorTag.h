#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"

#ifdef USE_MEMORY_DOMAINS
#   define WITH_CORE_POOL_ALLACATOR_TAGNAME //%__NOCOMMIT%
#endif

namespace Core {
class FMemoryPoolBase;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _POOL_TAG_DECL_IMPL(_NameId, _NameStr) \
    namespace PoolTag { \
        struct _NameId { \
        public: \
            static const char* Name() { return (_NameStr); } \
            \
            static void Register(FMemoryPoolBase* ppool); \
            static void Unregister(FMemoryPoolBase* ppool); \
            \
            static void Start(); \
            static void Shutdown(); \
            \
            static void ClearAll_AssertCompletelyFree(); \
            static void ClearAll_IgnoreLeaks(); \
            static void ClearAll_UnusedMemory(); \
        }; \
    }
//----------------------------------------------------------------------------
#ifdef WITH_CORE_POOL_ALLACATOR_TAGNAME
#   define POOL_TAG_DECL(_Name) _POOL_TAG_DECL_IMPL(_Name, STRINGIZE(_Name))
#else
#   define POOL_TAG_DECL(_Name) _POOL_TAG_DECL_IMPL(_Name, "")
#endif
//----------------------------------------------------------------------------
#define POOL_TAG_FWD(_Name) namespace PoolTag { struct _Name; }
//----------------------------------------------------------------------------
#define POOL_TAG(_Name) PoolTag::_Name
//----------------------------------------------------------------------------
POOL_TAG_DECL(Default) // Default tag for Pool segregation
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
