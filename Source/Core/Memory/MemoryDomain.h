#pragma once

#include "Core/Core.h"

// Memory Domains ON/OFF
#if not defined(FINAL_RELEASE) && not defined(PROFILING_ENABLED) // %_NOCOMMIT%
#   define USE_MEMORY_DOMAINS
#endif

// Memory domains collapsing (less codegen)
#if defined(USE_MEMORY_DOMAINS) && defined(PROFILING_ENABLED) // %_NOCOMMIT%
#   define COLLAPSE_MEMORY_DOMAINS
#endif

namespace Core {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryTracking;
//----------------------------------------------------------------------------
#define MEMORY_DOMAIN_NAME(_Name) CONCAT(_, _Name)
#define MEMORY_DOMAIN_TAG(_Name) Core::Domain::MEMORY_DOMAIN_NAME(_Name)
#define MEMORY_DOMAIN_TRACKING_DATA(_Name) MEMORY_DOMAIN_TAG(_Name)::TrackingData()
//----------------------------------------------------------------------------
namespace Domain {
    struct MEMORY_DOMAIN_NAME(Global) {
        static FMemoryTracking& TrackingData();
    };
}
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        struct MEMORY_DOMAIN_NAME(_Name) { \
            static FMemoryTracking& TrackingData(); \
        }; \
    }
#else
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent) \
    namespace Domain { \
        typedef MEMORY_DOMAIN_TAG(Global) MEMORY_DOMAIN_NAME(_Name); \
    }
#endif
//----------------------------------------------------------------------------
#ifdef COLLAPSE_MEMORY_DOMAINS
#   define MEMORY_DOMAIN_COLLAPSABLE_IMPL(_Name, _Parent) \
    namespace Domain { \
        typedef MEMORY_DOMAIN_TAG(_Parent) MEMORY_DOMAIN_NAME(_Name); \
    }
#endif
//----------------------------------------------------------------------------
#include "Core/Memory/MemoryDomain.Definitions-inl.h"
//----------------------------------------------------------------------------
#undef MEMORY_DOMAIN_COLLAPSABLE_IMPL
#undef MEMORY_DOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
class TMemoryTracking {
public:
    TMemoryTracking();
    ~TMemoryTracking();
};
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
TMemoryTracking<T, _Domain>::TMemoryTracking() {
    _Domain::TrackingData.Allocate(1, sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T, typename _Domain>
TMemoryTracking<T, _Domain>::~TMemoryTracking() {
    _Domain::TrackingData.Deallocate(1, sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API TMemoryView<FMemoryTracking *> EachDomainTrackingData();
//----------------------------------------------------------------------------
CORE_API void RegisterAdditionalTrackingData(FMemoryTracking *pTrackingData);
CORE_API void UnregisterAdditionalTrackingData(FMemoryTracking *pTrackingData);
//----------------------------------------------------------------------------
CORE_API void ReportDomainTrackingData(FWTextWriter& oss);
CORE_API void ReportAdditionalTrackingData(FWTextWriter& oss);
CORE_API void ReportAllocationHistogram(FWTextWriter& oss);
//----------------------------------------------------------------------------
CORE_API void ReportAllTrackingData(FWTextWriter* optional = nullptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
