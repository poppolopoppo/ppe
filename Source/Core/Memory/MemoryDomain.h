#pragma once

#include "Core/Core.h"

// Memory Domains ON/OFF
#if not (USE_CORE_FINAL_RELEASE || USE_CORE_PROFILING) // %_NOCOMMIT%
#   define USE_CORE_MEMORYDOMAINS 1
#else
#   define USE_CORE_MEMORYDOMAINS 0
#endif

// Memory domains collapsing (less codegen)
#define WITH_CORE_MEMORYDOMAINS_COLLAPSING (USE_CORE_MEMORYDOMAINS && USE_CORE_PROFILING) // %_NOCOMMIT%
#define WITH_CORE_MEMORYDOMAINS_FULL_COLLAPSING (WITH_CORE_MEMORYDOMAINS_COLLAPSING && 0) // turn to 1 to collapse all domains to Used/ReservedMemory %_NOCOMMIT%

namespace Core {
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMemoryTracking;
//----------------------------------------------------------------------------
#define MEMORYDOMAIN_NAME(_Name) CONCAT(F, _Name)
#define MEMORYDOMAIN_TAG(_Name) ::Core::MemoryDomain::MEMORYDOMAIN_NAME(_Name)
#define MEMORYDOMAIN_TRACKING_DATA(_Name) MEMORYDOMAIN_TAG(_Name)::TrackingData()
//----------------------------------------------------------------------------
namespace MemoryDomain {
    struct MEMORYDOMAIN_NAME(PooledMemory) { static CORE_API FMemoryTracking& TrackingData(); };
    struct MEMORYDOMAIN_NAME(UsedMemory) { static CORE_API FMemoryTracking& TrackingData(); };
    struct MEMORYDOMAIN_NAME(ReservedMemory) { static CORE_API FMemoryTracking& TrackingData(); };
}   // ^^^ don't use those directly ! always prefer explicit domains vvv
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !USE_CORE_MEMORYDOMAINS
#   define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#else
#   define MEMORYDOMAIN_GROUP_IMPL(_Name, _Parent) /* groups are not publicly visible */
#   if WITH_CORE_MEMORYDOMAINS_COLLAPSING
#       define MEMORYDOMAIN_COLLAPSABLE_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#   endif
#   if !WITH_CORE_MEMORYDOMAINS_FULL_COLLAPSING
#       define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        struct MEMORYDOMAIN_NAME(_Name) { \
            static CORE_API FMemoryTracking& TrackingData(); \
        }; \
    }
#   else
#       define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#   endif
#endif

#include "Core/Memory/MemoryDomain.Definitions-inl.h"

#undef MEMORYDOMAIN_COLLAPSABLE_IMPL
#undef MEMORYDOMAIN_GROUP_IMPL
#undef MEMORYDOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API void RegisterTrackingData(FMemoryTracking *pTrackingData);
CORE_API void UnregisterTrackingData(FMemoryTracking *pTrackingData);
//----------------------------------------------------------------------------
CORE_API void ReportAllocationHistogram(FWTextWriter& oss);
//----------------------------------------------------------------------------
CORE_API void ReportAllTrackingData(FWTextWriter* optional = nullptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
