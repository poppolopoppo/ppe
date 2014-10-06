#pragma once

#include "Core.h"

#define USE_MEMORY_DOMAINS

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MemoryTrackingData;
//----------------------------------------------------------------------------
#define MEMORY_DOMAIN_NAME(_Name) CONCAT(_, CONCAT(_Name, Tag))
#define MEMORY_DOMAIN_TAG(_Name) Core::Domain::MEMORY_DOMAIN_NAME(_Name)
#define MEMORY_DOMAIN_TRACKING_DATA(_Name) MEMORY_DOMAIN_TAG(_Name)::TrackingData
//----------------------------------------------------------------------------
namespace Domain {
    struct MEMORY_DOMAIN_NAME(Global) {
        static MemoryTrackingData& TrackingData;
    };
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) namespace Domain { \
    struct MEMORY_DOMAIN_NAME(_Name) { \
        static MemoryTrackingData& TrackingData; \
        }; }
#else
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) namespace Domain { \
    typedef MEMORY_DOMAIN_TAG(Global) MEMORY_DOMAIN_NAME(_Name); }
#endif
//----------------------------------------------------------------------------
#include "MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MemoryView;
//----------------------------------------------------------------------------
MemoryView<MemoryTrackingData*> EachDomainTrackingData();
//----------------------------------------------------------------------------
void ReportDomainTrackingData();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
