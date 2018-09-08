#pragma once

#include "Core.h"

// Memory Domains ON/OFF
#if not (USE_PPE_FINAL_RELEASE || USE_PPE_PROFILING) // %_NOCOMMIT%
#   define USE_PPE_MEMORYDOMAINS 1
#else
#   define USE_PPE_MEMORYDOMAINS 0
#endif

// Memory domains collapsing (less codegen)
#define WITH_PPE_MEMORYDOMAINS_COLLAPSING (USE_PPE_MEMORYDOMAINS && USE_PPE_PROFILING) // %_NOCOMMIT%
#define WITH_PPE_MEMORYDOMAINS_FULL_COLLAPSING (WITH_PPE_MEMORYDOMAINS_COLLAPSING && 0) // turn to 1 to collapse all domains to Used/ReservedMemory %_NOCOMMIT%

namespace PPE {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryTracking;
//----------------------------------------------------------------------------
#define MEMORYDOMAIN_NAME(_Name) CONCAT(F, _Name)
#define MEMORYDOMAIN_TAG(_Name) ::PPE::MemoryDomain::MEMORYDOMAIN_NAME(_Name)
#define MEMORYDOMAIN_TRACKING_DATA(_Name) MEMORYDOMAIN_TAG(_Name)::TrackingData()
//----------------------------------------------------------------------------
namespace MemoryDomain {
struct MEMORYDOMAIN_NAME(PooledMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
struct MEMORYDOMAIN_NAME(UsedMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
struct MEMORYDOMAIN_NAME(ReservedMemory) { static PPE_CORE_API FMemoryTracking& TrackingData(); };
}   // ^^^ don't use those directly ! always prefer explicit domains vvv
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !USE_PPE_MEMORYDOMAINS
#   define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#else
#   define MEMORYDOMAIN_GROUP_IMPL(_Name, _Parent) /* groups are not publicly visible */
#   if WITH_PPE_MEMORYDOMAINS_COLLAPSING
#       define MEMORYDOMAIN_COLLAPSABLE_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#   endif
#   if !WITH_PPE_MEMORYDOMAINS_FULL_COLLAPSING
#       define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        struct MEMORYDOMAIN_NAME(_Name) { \
            static PPE_CORE_API FMemoryTracking& TrackingData(); \
        }; \
    }
#   else
#       define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#   endif
#endif

#include "Memory/MemoryDomain.Definitions-inl.h"

#undef MEMORYDOMAIN_COLLAPSABLE_IMPL
#undef MEMORYDOMAIN_GROUP_IMPL
#undef MEMORYDOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API void RegisterTrackingData(FMemoryTracking *pTrackingData);
PPE_CORE_API void UnregisterTrackingData(FMemoryTracking *pTrackingData);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportAllocationHistogram(FWTextWriter& oss);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportAllTrackingData(FWTextWriter* optional = nullptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
