#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"

#ifdef USE_MEMORY_DOMAINS
#   define WITH_CORE_POOL_ALLACATOR_TAGNAME //%__NOCOMMIT%
#endif

namespace Core {
class MemoryPoolBase;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _POOLTAG_DECL_IMPL(_NameId, _NameStr) \
    namespace PoolTag { \
        struct _NameId { \
        public: \
            static char *Name() { return (_NameStr); } \
            \
            static void Register(MemoryPoolBase* ppool); \
            static void Unregister(MemoryPoolBase* ppool); \
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
#   define POOLTAG_DECL(_Name) _POOLTAG_DECL_IMPL(_Name, STRINGIZE(_Name))
#else
#   define POOLTAG_DECL(_Name) _POOLTAG_DECL_IMPL(_Name, "")
#endif
//----------------------------------------------------------------------------
#define POOLTAG(_Name) PoolTag::_Name
//----------------------------------------------------------------------------
POOLTAG_DECL(Default) // Default tag for Pool segregation
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
