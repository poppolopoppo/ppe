#pragma once

#include "RTTI/Namespace.h"

#include "MetaNamespace.h"
#include "Memory/MemoryDomain.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
#   define RTTI_NAMESPACE_DEF(_Api, _Name, _Domain) \
    PPE::RTTI::FMetaNamespace& CONCAT(RTTI_, _Name)() NOEXCEPT { \
        ONE_TIME_INITIALIZE(PPE::RTTI::FMetaNamespace, GInstance, STRINGIZE(_Name), MEMORYDOMAIN_TRACKING_DATA(_Domain)); \
        return GInstance; \
    }
#else
#   define RTTI_NAMESPACE_DEF(_Api, _Name, _Domain) \
    PPE::RTTI::FMetaNamespace& CONCAT(RTTI_, _Name)() NOEXCEPT { \
        ONE_TIME_INITIALIZE(PPE::RTTI::FMetaNamespace, GInstance, STRINGIZE(_Name)); \
        return GInstance; \
    }
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
