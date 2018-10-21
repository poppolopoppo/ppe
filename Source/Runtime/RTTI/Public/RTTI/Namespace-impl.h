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
    _Api PPE::RTTI::FMetaNamespace RTTI_ ## _Name (STRINGIZE(_Name), MEMORYDOMAIN_TRACKING_DATA(_Domain))
#else
#   define RTTI_NAMESPACE_DEF(_Api, _Name, _Domain) \
    _Api PPE::RTTI::FMetaNamespace RTTI_ ## _Name (STRINGIZE(_Name))
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
