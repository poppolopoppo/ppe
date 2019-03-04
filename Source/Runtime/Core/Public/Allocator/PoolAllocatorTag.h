#pragma once

#include "Core.h"

#include "Memory/MemoryDomain.h"

#if USE_PPE_MEMORYDOMAINS
#   define WITH_PPE_POOL_ALLACATOR_TAGNAME //%__NOCOMMIT%
#endif

namespace PPE {
class IMemoryPool;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define _POOL_TAG_DECL_IMPL(_Api, _NameId, _NameStr) \
    namespace PoolTag { \
        struct _Api _NameId { \
        public: \
            static const char* Name() { return (_NameStr); } \
            \
            static void Register(IMemoryPool* ppool); \
            static void Unregister(IMemoryPool* ppool); \
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
#ifdef WITH_PPE_POOL_ALLACATOR_TAGNAME
#   define POOL_TAG_DECL(_Api, _Name) _POOL_TAG_DECL_IMPL(_Api, _Name, STRINGIZE(_Name))
#else
#   define POOL_TAG_DECL(_Api, _Name) _POOL_TAG_DECL_IMPL(_Api, _Name, "")
#endif
//----------------------------------------------------------------------------
#define POOL_TAG_FWD(_Api, _Name) namespace PoolTag { struct _Api _Name; }
//----------------------------------------------------------------------------
#define POOL_TAG(_Name) PoolTag::_Name
//----------------------------------------------------------------------------
POOL_TAG_DECL(PPE_CORE_API, Default) // Default tag for Pool segregation
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
